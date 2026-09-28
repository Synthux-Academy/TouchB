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
_core        { core }
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

    auto on_latch_on = std::bind(&CoreUI::_on_latch_on, this, _1);
    auto on_latch_off = std::bind(&CoreUI::_on_latch_off, this, _1);
    _latch.set_on_note_on(on_latch_on);
    _latch.set_on_note_off(on_latch_off);

    _init_timer.Init();

    _out_val.set(1.f);
    _mix_val.set(1.f);

    _verb_send.set(0.f);
    _verb_fb.set(.8f);

    _blur.set(0.f);
    _flutter.set(0.f);

    _filter_val.set(.5f);
    _in_val.set(0.6896551724f); //maps to 0db on a scale -60...+12
};

void CoreUI::process() 
{
    _touch.process();
    _process_ui_queue();
    
    // Latch //////////////////////////
    auto switch_a = _touch.switches().A();
    _latch.set_on(switch_a == 1);

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
    
    // Direction .....................................
    auto switch_b = _touch.switches().B();
    _core.set_play_direction(static_cast<Core::PlayDirection>(switch_b));

    // Modulation ....................................
    if (_apply.test(Knobs::s33)) {
        _core.set_blur(_blur.value());
        _core.set_flutter(_flutter.value());
    }

    //Reverb .........................................
    if (_apply.test(Knobs::s34)) {
        _core.set_reverb_send(_verb_send.value());
        _core.set_reverb_fb(_verb_fb.value());
    }

    // Mix / out
    if (_apply.test(Knobs::s35)) {
        _core.set_mix(_mix_val.value());
        _core.set_output_level(_out_val.value());
    }

    // Filter / in
    if (_apply.test(Knobs::s37)) {
        _core.set_filter(_filter_val.value());
        _core.set_input_level(_in_val.value());
    }

    _apply.reset();
};

void CoreUI::_process_ui_queue()
{
    // Init mvalues from knobs ......................
    if (!_is_init && _init_timer.HasPassedMs(100)) {
        auto& knobs = _touch.knobs();
        _apply.set(Knobs::s33);
        _blur.set(knobs.GetPotValue(Knobs::s33));
            
        _apply.set(Knobs::s34);
        _verb_send.set(knobs.GetPotValue(Knobs::s34));

        _apply.set(Knobs::s35);
        _mix_val.set(knobs.GetPotValue(Knobs::s35));

        _apply.set(Knobs::s37);
        _filter_val.set(knobs.GetPotValue(Knobs::s37));

        _is_init = true;
    }

    auto is_alt_touched = _touched.test(Pads::Tou) || _touched.test(Pads::Ch);
    _pot_monitor.Process();
    while(!_ui_queue.IsQueueEmpty()) {
        auto event = _ui_queue.GetAndRemoveNextEvent();
        if (event.type == UiEventQueue::Event::EventType::potMoved) {
            auto id = event.asPotMoved.id;
            auto val = event.asPotMoved.newPosition;
            val = infrasonic::map(val, .02f, .95f, 0.f, 1.f);
            _apply.set(id);
            switch (id) {
                case Knobs::s30: 
                    _core.set_size(val);
                    break;

                case Knobs::s31: 
                    _core.set_start(val);
                    break;

                case Knobs::s32: 
                    _core.set_pitch(snapped_speed(val)); 
                    break;

                case Knobs::s33: {
                    _blur.process(val, !is_alt_touched);
                    _flutter.process(val, is_alt_touched);
                    break;
                }
                case Knobs::s34: {
                    _verb_send.process(val, !is_alt_touched);
                    _verb_fb.process(val, is_alt_touched);
                    break;
                }
                case Knobs::s35:
                    _mix_val.process(val, !is_alt_touched);
                    _out_val.process(val, is_alt_touched);
                    break;

                case Knobs::s36:
                    _core.set_distortion_flavor(val);
                    break;

                case Knobs::s37: {
                    _filter_val.process(val, !is_alt_touched);
                    _in_val.process(val, is_alt_touched);
                    break;
                }
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
            _latch.note_on(pad-3);
        }
    }  
};

void CoreUI::_on_pad_release(Pads::Pad pad)
{
    _touched.reset(pad);
    switch (pad) {
        case Pads::TopLeft:
        case Pads::TopCenter:
        case Pads::TopRight:
        case Pads::Tou:
        case Pads::Ch: break;
        default: _latch.note_off(pad - 3);
    }
};

void CoreUI::_on_latch_on(const uint8_t pad)
{
    _core.add_behavior(pad);
}

void CoreUI::_on_latch_off(const uint8_t pad)
{
    _core.remove_behavior(pad);
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
