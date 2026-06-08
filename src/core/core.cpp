#include "core.h"

#include <functional>
#include <daisysp.h>
#include "hw/buffer.sdram.h"
#include "behavior.h"
#include "common.h"
#include "expose.h"

using namespace synthux::touchb;
using namespace daisysp;

Core::Core():
_increment          { 1.f },
_target_increment   { 1.f },
_in_mult            { 1.f },
_behavior_ptr       { -1 }
{
    _bus.fill(0);
    _behavior.fill(0xff);
};
  
void Core::init(const float sample_rate, const float cb_buffer_size) {
    auto& pool = SDRAMBuffer::pool();
    _buffer.init(pool.sourceBuffer(), pool.sourceBufferSize());
    
    // Voxs
    auto vox_idx = 0;
    for (auto& v: _vox) v.init(&_buffer, vox_idx++);

    // Start overdubbing
    _buffer.set_rec_size(_buffer.size());
    _buffer.set_recording(true);

    // Tape
    _rnd_imp.init(sample_rate);
    _rnd_imp.set_freq_hz(4.f);
    _smooth.init(sample_rate, 0.06f);
    for (auto& f: _tape_filter) {
        f.Init(sample_rate);
        f.SetDrive(.4f);
        f.SetRes(.2f);
    }

    _in_buf_switch.init(sample_rate);

    //Fx
    _fx.init(sample_rate);

    // Filter
    for (auto& f: _filter) {
        f.Init(sample_rate);
        f.SetDrive(0.f);
    }

    // Reverb
    _reverb = infrasonic::SDRAM::allocate<ReverbSc>();
    _reverb->Init(sample_rate);
    _reverb->SetFeedback(kReverbFeedback);
    _reverb->SetLpFreq(kReverLPFreq);

    // Controls
    set_start(0.f);
    set_size(1.f);
    set_mix(1.f);
    set_filter(1.f);
};

void Core::process(const float* const* in, float** out, size_t size) 
{
    float in0, in1;
    for (size_t i = 0; i < size; i++) {
        auto flutter = _rnd_imp.process() * _tape_mod;
        auto smooth_tape = _smooth.process(std::abs(flutter));
        volatile auto tape_freq = 16000.f * (1.f - std::clamp(8.f * smooth_tape, .0f, 8.f));
        auto target = _target_increment * (1.f + flutter);
        auto set_increment = false;
        if (std::fabs(target  - _increment) > .002f) {
            _increment += (target - _increment) * 0.002083333333f; //20ms    0.0002083333333f; //100ms
            set_increment = true;
        }
        else {
            _increment = target;
        }

        in0 = in[0][i] * _in_mult;
        in1 = in[1][i] * _in_mult;

        _buffer.write(in0, in1);
        _bus.fill(0);
        auto vout0 = 0.f, vout1 = 0.f;
        for (auto& v: _vox) {
            if (set_increment) v.set_playhead_increment(_increment);
            
            _is_active.set(v.idx(), v.is_playing());
            if (v.is_playing()) {
                v.process(vout0, vout1);
                _bus[0] += vout0;
                _bus[1] += vout1;
                if (_rec_cued) {
                    _buffer.set_recording(true);
                    _rec_cued = false;
                }

                if (!v.is_playing()) {
                    _is_active.reset(v.idx());
                    if (_has_behavior()) _trigger_vox((v.idx() + 1) % kVoxCount);
                }
            }
        }
        
        _in_buf_mix.set_stage(_in_buf_switch.process());
        _in_buf_mix.process(in0, in1, _bus[0], _bus[1], _in_buf_mix_bus[0], _in_buf_mix_bus[1]);

        _pre_fx_mix.process(_in_buf_mix_bus[0], _in_buf_mix_bus[1], _bus[0], _bus[1], _bus[0], _bus[1]);

        for (auto k = 0; k < 2; k++) {
            if (_tape_mod > 0) {
                _tape_filter[k].SetFreq(tape_freq);
                _tape_filter[k].Process(_bus[k]);
                _bus[k] = _tape_filter[k].Low();
            }

            _filter[k].Process(_bus[k]);
            _bus[k] = _fltr_lp ? _filter[k].Low() : _filter[k].High();
        }

        _fx.process(_bus[0], _bus[1]);

        _reverb_send.process(0, 0, _bus[0], _bus[1], _reverb_in[0], _reverb_in[1]);
        _reverb->Process(_reverb_in[0], _reverb_in[1], &(_reverb_out[0]), &(_reverb_out[1]));        
        _bus[0] = (_bus[0] + _reverb_out[0]) * .75f;
        _bus[1] = (_bus[1] + _reverb_out[1]) * .75f;
        
        if (!_has_behavior()) {
            _post_fx_mix.process(_in_buf_mix_bus[0], _in_buf_mix_bus[1], _bus[0], _bus[1], _bus[0], _bus[1]);
        }

        out[0][i] = SoftLimit(_bus[0]);
        out[1][i] = SoftLimit(_bus[1]);
    }
};

void Core::_trigger_vox(const uint8_t idx)
{
    auto& v = _vox[idx];
    bool reverse = false;
    switch (_direction) {
        case PlayDirection::Rev: reverse = true; break;
        case PlayDirection::Rnd: reverse = _dice(_rand) > .5f; break;
        default: break;
    };
    v.set_reverse(reverse);
    v.set_shape(_fade_in ? 1.f : 0.f);
    v.trigger();
}

void Core::add_behavior(const uint8_t idx)
{
    if (idx >= 7) return;
    for (auto b: _behavior) {
        if (b == idx) return;
    }

    _behavior[++_behavior_ptr] = idx;
    _apply_behavior();
    if (_behavior_ptr == 0) {
        _trigger_vox();
        _in_buf_switch.set_on(true);
        _rec_cued = true;
    }
}

void Core::remove_behavior(const uint8_t idx)
{
    if (idx >= 7) return;
    // Remove behavior
    auto found_match = false;
    for (size_t i = 0; i < _behavior.size(); i++) {
        if (found_match) {
            _behavior[i - 1] = _behavior[i];
        }
        else {
            if (_behavior[i] == idx) {
                found_match = true;
                _behavior_ptr --;
            }
        }
    }

    if (!_has_behavior()) {
        for (auto& v: _vox) v.stop();
        _fx.disengage();
        _in_buf_switch.set_on(false);
        _rec_cued = false;
    }
    else {
        _apply_behavior();
    }
}

void Core::_apply_behavior()
{
    Combo c;
    c.lead_ptr = _behavior_ptr;
    c.idxs = &_behavior;
    auto b = behavior4combo(c);
    _fx.engage(b.fx);
}

void Core::set_start(const float norm) 
{
    _norm_start = std::clamp(norm, 0.f, 1.f);
    _set_start();
}
void Core::_set_start()
{
    auto start = _norm_start * _buffer.size();
    for (auto& v: _vox) v.set_start(start);
}

void Core::set_size(const float norm) 
{
    _norm_size = norm * norm;
    _set_size();
}
void Core::_set_size()
{
    auto size = static_cast<size_t>(infrasonic::unitclamp(_norm_size) * _buffer.size());
    size = std::max(size, kSliceMinSize);
    for (auto& v: _vox) v.set_size(size);
}

void Core::set_mix(const float norm)
{
    _pre_fx_mix.set_stage(norm);
    _post_fx_mix.set_stage(1.f - norm);
}

float mapped_speed(const float val) 
{
    return val < .5f ? 2.f * val : 1.f + (val - .5f) * 6.f;
}
void Core::set_pitch(const float norm)
{
    auto speed = infrasonic::unitclamp(norm);
    _target_increment = mapped_speed(speed);
}

void Core::set_tape_mod(const float norm)
{
    _tape_mod = infrasonic::unitclamp(norm * .25f);
}

void Core::set_blur(const float norm)
{
    auto blur = infrasonic::unitclamp(norm) * 5760; //120ms
    for (auto& v: _vox) v.set_blur(blur);
}

void Core::set_envelope_on(const bool on)
{
    _fade_in = on;
}

void Core::set_reverb_send(const float norm)
{
    _reverb_send.set_stage(norm);
}

void Core::set_reverb_fb(const float norm)
{
    _reverb->SetFeedback(norm);
}

void Core::set_filter(const float norm)
{
    _fltr_lp = norm < 0.5;
    auto val = _fltr_lp ? 2.f * norm : 2.f * (norm - .5f);
    auto clamped = infrasonic::unitclamp(val * val);
    if (_fltr_lp) {
        _fltr_freq = infrasonic::map(clamped, 0.f, 1.f, 500.f, 10000.f);
    }
    else {
        _fltr_freq = infrasonic::map(clamped, 0.f, 1.f, 50.f, 2000.f);
    }

    FP3(_fltr_freq);
    
    auto res = infrasonic::map(clamped, 0.f, 1.f, 0.2f, 0.f);
    for (auto& f: _filter) {
        f.SetFreq(_fltr_freq);
        f.SetRes(res);
    }
}

void Core::set_play_direction(const PlayDirection direction)
{
    _direction = direction;
}

void Core::set_input_level(const float norm)
{
    auto db = infrasonic::map(norm, 0.f, 1.f, -40.f, 0.f);
    _in_mult = infrasonic::dbfs2lin(db);
}
