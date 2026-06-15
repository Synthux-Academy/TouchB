#include "core.ui.h"

#include <daisysp.h>
#include "common/common.h"
#include "expose.h"

using namespace synthux::touchb;
using namespace daisy;
using namespace daisysp;



static constexpr std::array<float, 7> kSpeedSteps = { 
    .125f,          // -24
    .25f,           // -12
    .33371f,        // -7
    .5f,            // 0
    .5830516667f,   // 7 
    .666666667f,    // 12
    1.f             // 24
};
static float snapped_speed(const float speed)
{
    auto s = static_cast<float>(kSpeedSteps.size() - 1);
    auto idx = static_cast<int>(std::clamp(std::round(speed * s), 0.f, s));
    return kSpeedSteps[idx];
}

CoreUI::CoreUI(Touch& touch, Core& core):
_touch       { touch },
_core        { core },
_latched_pad { kNoLatch },
_is_latched  { false }
{}

void CoreUI::init() 
{
    // Listen to knobs /////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    _pot_monitor.Init(_ui_queue, _touch.knobs(), 500, 0.005f, 0.002f);

    // Assign callbacks ////////////////////////////////////////
    ////////////////////////////////////////////////////////////
    using namespace std::placeholders;
    auto on_touch = std::bind(&CoreUI::_on_pad_touch, this, _1);
    auto on_release = std::bind(&CoreUI::_on_pad_release, this, _1);
    _touch.pads().set_on_touch(on_touch);
    _touch.pads().set_on_release(on_release);

    _init_timer.Init();

    _fltr_val.set(.5f);
    _inp_val.set(1.f);

    _verb_send.set(0.f);
    _verb_fb.set(.7f);
};

void CoreUI::process() 
{
    _touch.process();
    _process_ui_queue();
    
    auto switch_a = _touch.switches().A();
    // Latch //////////////////////////
    static auto was_latched = false;
    _is_latched = switch_a == 1;
    if (was_latched && !_is_latched && !_touched.any()) {
        _release_latched();
    }
    was_latched = _is_latched;

    // Env ////////////////////////////
    static auto was_env_on = false;
    auto env_on = switch_a == 2;
    if (!was_env_on && env_on) {
        _core.set_envelope_on(true);
    }
    else if (was_env_on && !env_on) {
        _core.set_envelope_on(false);
    }
    was_env_on = env_on;
    
    // Direction ///////////////////////
    auto switch_b = _touch.switches().B();
    _core.set_play_direction(static_cast<Core::PlayDirection>(switch_b));

    // Filter
    if (_apply.test(Knobs::s31)) {
        _core.set_filter(_fltr_val.value());
        _core.set_input_level(_inp_val.value());
    }

    //Reverb
    if (_apply.test(Knobs::s30)) {
        _core.set_reverb_send(_verb_send.value());
        _core.set_reverb_fb(_verb_fb.value());
    }

    _apply.reset();
};

void CoreUI::_process_ui_queue()
{
    if (!_is_init && _init_timer.HasPassedMs(100)) {
        auto& knobs = _touch.knobs();
        _apply.set(Knobs::s30);
        _verb_send.set(knobs.GetPotValue(Knobs::s30));
        
        _apply.set(Knobs::s31);
        _fltr_val.set(knobs.GetPotValue(Knobs::s31));

        _core.set_mix(knobs.GetPotValue(Knobs::s36));

        _is_init = true;
    }

    _pot_monitor.Process();
    
    auto is_alt_touched = _touched.test(Pads::Tou) || _touched.test(Pads::Ch);

    while(!_ui_queue.IsQueueEmpty()) {
        auto event = _ui_queue.GetAndRemoveNextEvent();
        if (event.type == UiEventQueue::Event::EventType::potMoved) {
            auto id = event.asPotMoved.id;
            auto val = event.asPotMoved.newPosition;
            val = infrasonic::map(val, .02f, .95f, 0.f, 1.f);
            _apply.set(id);
            switch (id) {
                case Knobs::s30: {
                    _verb_send.process(val, !is_alt_touched);
                    _verb_fb.process(val, is_alt_touched);
                    break;
                }
                case Knobs::s31: {
                    _fltr_val.process(val, !is_alt_touched);
                    _inp_val.process(val, is_alt_touched);
                    break;
                }    
                case Knobs::s32: _core.set_pitch(snapped_speed(val)); break;
                case Knobs::s33: _core.set_tape_mod(val);   break;
                case Knobs::s34: _core.set_blur(val);       break;
                case Knobs::s35: _core.set_size(val);       break;
                case Knobs::s36: _core.set_mix(val);        break;
                case Knobs::s37: _core.set_start(val);      break;
            }
        }
    }
}

void CoreUI::_on_pad_touch(Pads::Pad pad) 
{
    _touched.set(pad);
    switch (pad) {
        case Pads::TopLeft:
        case Pads::TopCenter:
        case Pads::TopRight:
        case Pads::Tou:
        case Pads::Ch: break;
        default: {
            _latched_pad = pad;
            _core.add_behavior(pad - 3);     
        }
    }
};

void CoreUI::_on_pad_release(Pads::Pad pad)
{
    _touched.reset(pad);

    if (_is_latched) return;
    _release_pad(pad);
};

void CoreUI::_release_pad(Pads::Pad pad)
{
    switch (pad) {
        case Pads::TopLeft:
        case Pads::TopCenter:
        case Pads::TopRight:
        case Pads::Tou:
        case Pads::Ch: break;
        default: _core.remove_behavior(pad - 3);
    }
}

void CoreUI::_release_latched()
{
    if (_latched_pad != kNoLatch) {
        _release_pad(_latched_pad);
    }   
}

#ifdef USB_MIDI
float norm(const uint8_t value) {
    return static_cast<float>(value) / 127.f;
};

void BassUI::_process_midi() 
{
    _bass.ProcessClockIn(false);
    _midi.Listen();
    while(_midi.HasEvents()) {
        auto msg = _midi.PopEvent();
        switch(msg.type) {
            case SystemRealTime: {
                switch (msg.srt_type) {
                    case TimingClock: {
                        _bass.ProcessClockIn(true);
                    }
                    break;
                    default: break;
                }
            }
            break;
            case NoteOn: {
                auto note_msg = msg.AsNoteOn();
                _bass.NoteOn(note_msg.note);
            }
            break;
            case NoteOff: {
                auto note_msg = msg.AsNoteOff();
                _bass.NoteOff(note_msg.note);
            }
            case ControlChange: {
                auto ctrl_msg = msg.AsControlChange();
                auto num = ctrl_msg.control_number;
                auto norm_value = norm(ctrl_msg.value);
                if (num == 71) { _flt_reso.Set(norm_value); }              // resonance
                else if (num == 72) { _env_value.Set(norm_value); }             // envelope
                else if (num == 74) { _flt_freq.Set(norm_value); }         // cut off
                else if (num == 75) { _osc_1_freq.Set(norm_value); }       // osc 1 freq
                else if (num == 76) { _osc_1_shape.Set(norm_value); }      // osc 1 shape
                else if (num == 77) { _osc_2_freq.Set(norm_value); }       // osc 2 freq
                else if (num == 78) { _osc_2_amount.Set(norm_value); }     // osc 2 amount
                else if (num == 85) { _pattern_value.Set(norm_value); }    // pattern
                else if (num == 86) { _human_note_value.Set(norm_value); } // human note
                else if (num == 87) { _human_env_value.Set(norm_value); }  // human envelope
                else if (num == 91) { _verb_value.Set(norm_value); }       // reverb
                else if (num == 123) { _bass.Reset(); }
                else if (num == 126) { _bass.SetMono(); }
                else if (num == 127) { _bass.SetPoly(); }
            }
            break;
            
            default: break;
        }
    }
};
#endif
