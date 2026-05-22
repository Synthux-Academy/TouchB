#include "core.ui.h"

#include <daisy.h>
#include "core/config.h"

using namespace synthux::touchb;
using namespace daisy;

// CLOCK ///////////////////////////////////
static bool clock_state = false;
void CoreUI::tick()
{
    // auto& d = _core.driver();
    auto new_state = false;
    // auto midi_state = _process_midi();
    // switch (d.source()) {
    //     case Driver::Source::ts4: new_state = _hw.GetClockInputState(); break;
    //     case Driver::Source::midi: new_state = midi_state; break;
    //     default: break;
    // }
    // d.tick(new_state && !clock_state);
    clock_state = new_state;

    // Modified libDaisy MIDI handlers require explicit call to transmit
    // enqueued messages instead of blocking every time a message is sent
    // _touch.usb_midi().TransmitEnqueuedMessages();
}

// MIDI /////////////////////////////////////
bool CoreUI::_process_midi()
{
    auto& midi = _touch.usb_midi();
    midi.Listen();
    bool has_clock = false;
    while(midi.HasEvents())
    {
        auto event = midi.PopEvent();
        switch(event.type)
        {
            case MidiMessageType::SystemRealTime: {
                has_clock = _process_realtime(event) || has_clock; 
            }
            break;
            
            case MidiMessageType::NoteOn: {
                auto e = event.AsNoteOn();
                _process_note_on(e);
            }
            break;

            default: break;
        }
    }
    return has_clock;
}
void CoreUI::_process_note_on(daisy::NoteOnEvent& note_on)
{
    //
}
bool CoreUI::_process_realtime(daisy::MidiEvent& event)
{
    return event.srt_type == SystemRealTimeType::TimingClock;
}
