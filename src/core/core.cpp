#include "core.h"

#include <functional>
#include <daisysp.h>
#include "hw/buffer.sdram.h"
#include "behavior.h"
#include "common/common.h"

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
    // memcpy(out, in, sizeof(float) * 2 * size);
    // return;

    float in0, in1;
    for (size_t i = 0; i < size; i++) {
        auto set_increment = false;
        if (std::fabs(_target_increment - _increment) > .002f) {
            _increment += (_target_increment - _increment) * 0.0002083333333f; //100ms
            set_increment = true;
        }
        else {
            _increment = _target_increment;
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
                if (!v.is_playing()) {
                    _is_active.reset(v.idx());
                    if (_has_behavior()) _trigger_vox((v.idx() + 1) % kVoxCount);
                }
            }
        }

        _mix.process(in0, in1, _bus[0], _bus[1], _bus[0], _bus[1]);

        _reverb_send.process(0, 0, _bus[0], _bus[1], _reverb_in[0], _reverb_in[1]);
        _reverb->Process(_reverb_in[0], _reverb_in[1], &(_reverb_out[0]), &(_reverb_out[1]));
        _bus[0] = (_bus[0] + _reverb_out[0]) * .75f;
        _bus[1] = (_bus[1] + _reverb_out[1]) * .75f;

        _filter[0].Process(_bus[0]);
        _filter[1].Process(_bus[1]);

        out[0][i] = SoftLimit(_fltr_lp ? _filter[0].Low() : _filter[0].High());
        out[1][i] = SoftLimit(_fltr_lp ? _filter[1].Low() : _filter[1].High());
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
        _buffer.set_recording(false);
        _trigger_vox();
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
        _buffer.set_recording(true);
    }
    else {
        _apply_behavior();
    }
}

void Core::_apply_behavior()
{
    // VoxBehavior vb;
    // vb.buf_size = _buffer.size();
    // vb.lead_ptr = _behavior_ptr;
    // vb.pads = &_behavior;
    // auto p = vox_parms_for_behavior(vb);
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
    auto size = infrasonic::unitclamp(_norm_size) * _buffer.size();
    for (auto& v: _vox) v.set_size(size);
}

void Core::set_mix(const float norm)
{
    _mix.set_stage(norm);
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

void Core::set_reverb(const float norm)
{
    _reverb_send.set_stage(norm);
    if (norm > .5f) {
        auto fb = kReverbFeedback + (1.f - kReverbFeedback) * (2.f * norm - 1.f);
        _reverb->SetFeedback(infrasonic::unitclamp(fb));
    }
    else {
        _reverb->SetFeedback(kReverbFeedback);
    }
}

void Core::set_filter(const float norm)
{
    _fltr_lp = norm < 0.5;
    auto val = _fltr_lp ? 2.f * norm : 2.f * (norm - .5f);
    auto clamped = infrasonic::unitclamp(val * val);
    auto freq = infrasonic::map(clamped, 0.f, 1.f, 30.f, 10000.f);
    auto res = infrasonic::map(clamped, 0.f, 1.f, 0.5f, 0.f);
    for (auto& f: _filter) {
        f.SetFreq(freq);
        f.SetRes(res);
    }
}

void Core::set_play_direction(const PlayDirection direction)
{
    _direction = direction;
}

void Core::set_input_level(const float norm)
{
    auto db = infrasonic::map(norm, 0.f, 1.f, -90.f, 0.f);
    _in_mult = infrasonic::dbfs2lin(db);
}
