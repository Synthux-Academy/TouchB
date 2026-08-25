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
    _tape_smooth.init(sample_rate, 0.06f);
    for (auto& f: _tape_filter) {
        f.Init(sample_rate);
        f.SetDrive(.4f);
        f.SetRes(.2f);
    }
    _filter_smooth.init(sample_rate, 0.004f);

    _in_loop_switch.init(sample_rate);
    _dist_feed_switch.init(sample_rate);
    _filter_switch.init(sample_rate);

    //Distortion
    _distortion.init(sample_rate);

    // Filter
    for (auto i = 0; i < 2; i++) {
        _loop_filter[i].Init(sample_rate);
        _loop_filter[i].SetDrive(0.f);
        _in_filter[i].Init(sample_rate);
        _in_filter[i].SetDrive(0.f);
    }

    // Reverb
    _reverb = infrasonic::SDRAM::allocate<ReverbSc>();
    _reverb->Init(sample_rate);
    _reverb->SetFeedback(kReverbFeedback);
    _reverb->SetLpFreq(kReverLPFreq);

    // Limiter
    for (auto& l: _limiter) l.Init();

    // Controls
    set_start(0.f);
    set_size(1.f);
    set_mix(1.f);
    set_filter(.5f);
};

/* Signal path ............................................................. 
            Constantly feeding distortion with either input or looped
            signal to maintain RMS gain. The swithing between signal 
            is by _dist_feed_mix (l/in on the scheme).
            Note: _dist_feed_mix and _in_loop_mix work in opposite directions.
                 _loop_bus         
           |-- Loop -- Distort -- l/in -- Filter ---|
           |                       |                |        _mix_bus
       |---|                       |          _in_loop_mix -- Reverb --|
       |   |                       |                |                  |
       |   |----------- Filter ---------------------|                 wet 
in >---|         _in_bus                                        _dry_wet_mix --> out
       |                                                             dry 
       |                                                              | 
       |--------------------------------------------------------------|
                            Clean in0, in1
*/
void Core::process(const float* const* in, float** out, size_t size) 
{
    float in0, in1;
    for (size_t i = 0; i < size; i++) {
        //Write
        in0 = in[0][i] * _in_mult;
        in1 = in[1][i] * _in_mult;
        _buffer.write(in0, in1);

        // Init bus
        _in_bus[0] = in0;
        _in_bus[1] = in1;
        _loop_bus.fill(0);
        _mix_bus.fill(0);
        
        // Tape flutter
        auto flutter = _rnd_imp.process() * _flutter;
        auto smooth_tape = _tape_smooth.process(std::abs(flutter));
        volatile auto tape_freq = 16000.f * (1.f - std::clamp(8.f * smooth_tape, .0f, 8.f));
        auto target = _target_increment * (1.f + flutter);
        auto set_increment = false;
        if (std::fabs(target  - _increment) > .002f) {
            _increment += (target - _increment) * 0.002083333333f; //20ms
            set_increment = true;
        }
        else {
            _increment = target;
        }

        // Loop
        auto vout0 = 0.f, vout1 = 0.f;
        for (auto& v: _vox) {
            if (set_increment) v.set_playhead_increment(_increment);
            
            _is_active.set(v.idx(), v.is_playing());
            if (v.is_playing()) {
                v.process(vout0, vout1);
                _loop_bus[0] += vout0;
                _loop_bus[1] += vout1;
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

        // Distort
        // Constantly feed the distortion RMS gain either with input or loop.
        _dist_feed_mix.set_stage(_dist_feed_switch.process());
        _dist_feed_mix.process(_in_bus[0], _in_bus[1], _loop_bus[0], _loop_bus[1], _loop_bus[0], _loop_bus[1]);
        _distortion.process(_loop_bus[0], _loop_bus[1]);

        // Filter
        auto lpf_mix = _filter_switch.process();
        auto hpf_mix = std::clamp(1.f - lpf_mix, 0.f, 1.f);
        auto fltr_freq = _filter_smooth.process(_fltr_freq);
        for (auto k = 0; k < 2; k++) {
            if (_flutter > 0) {
                _tape_filter[k].SetFreq(tape_freq);
                _tape_filter[k].Process(_loop_bus[k]);
                _loop_bus[k] = _tape_filter[k].Low();
            }
       
            _loop_filter[k].SetFreq(fltr_freq);
            _loop_filter[k].Process(_loop_bus[k]);
            _loop_bus[k] = lpf_mix * _loop_filter[k].Low() + hpf_mix * _loop_filter[k].High();

            _in_filter[k].SetFreq(fltr_freq);
            _in_filter[k].Process(_in_bus[k]);
            _in_bus[k] = lpf_mix * _in_filter[k].Low() +  hpf_mix * _in_filter[k].High();
        }

        // Loop/In switch
        _in_loop_mix.set_stage(_in_loop_switch.process());
        _in_loop_mix.process(_in_bus[0], _in_bus[1], _loop_bus[0], _loop_bus[1], _mix_bus[0], _mix_bus[1]);

        // Reverb
        _reverb_send.process(0, 0, _mix_bus[0], _mix_bus[1], _reverb_in[0], _reverb_in[1]);
        _reverb->Process(_reverb_in[0], _reverb_in[1], &(_reverb_out[0]), &(_reverb_out[1]));
        _mix_bus[0] = (_mix_bus[0] + _reverb_out[0]) * .75f;
        _mix_bus[1] = (_mix_bus[1] + _reverb_out[1]) * .75f;

        // Dry / wet mix
        _dry_wet_mix.process(in0, in1, _mix_bus[0], _mix_bus[1], out[0][i], out[1][i]);
    }
    
    // Limiter
    for (auto i = 0; i < 2; i++) _limiter[i].ProcessBlock(out[i], size, 1);
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
    auto has_behavior = _has_behavior();
    _behavior_ptr = 0;
    _behavior[_behavior_ptr] = idx;
    _apply_behavior();
    if (!has_behavior) {
        _trigger_vox();
        _in_loop_switch.set_on(true);
        _dist_feed_switch.set_on(true);
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
        _distortion.disengage();
        _in_loop_switch.set_on(false);
        _dist_feed_switch.set_on(false);
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
    _distortion.engage(b.distortion);
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

void Core::set_distortion_flavor(const float norm)
{
    _distortion.set_flavor_norm(norm);
}

void Core::set_mix(const float norm)
{
    _dry_wet_mix.set_stage(norm);
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

void Core::set_blur(const float norm)
{
    auto blur = infrasonic::unitclamp(norm) * 5760; //120ms
    for (auto& v: _vox) v.set_blur(blur);
}
void Core::set_flutter(const float norm)
{
    _flutter = infrasonic::unitclamp(norm * .25f);
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
    auto is_lpf = norm < 0.5;
    _filter_switch.set_on(is_lpf);
    auto val = is_lpf ? 2.f * norm : 2.f * (norm - .5f);
    auto clamped = infrasonic::unitclamp(val * val);
    if (is_lpf) {
        _fltr_freq = infrasonic::map(clamped, 0.f, 1.f, 500.f, 10000.f);
    }
    else {
        _fltr_freq = infrasonic::map(clamped, 0.f, 1.f, 50.f, 2000.f);
    }
    auto res = infrasonic::map(clamped, 0.f, 1.f, 0.2f, 0.f);
    for (auto i = 0; i < 2; i++) {
        _in_filter[i].SetRes(res);
        _loop_filter[i].SetRes(res);
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
    _distortion.set_level_norm(_in_mult);
}
