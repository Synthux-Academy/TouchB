#pragma once

#include <functional>
#include <bitset>
#include <daisy_seed.h>

#include "hw/touch.h"
#include "core/core.h"

#include "common/mvalue.h"

namespace synthux { 
namespace touchb {

class CoreUI {
public:
    CoreUI(Touch&, Core&);
    ~CoreUI() = default;

    void init();
    void process();

    void tick();

private:
    static constexpr uint16_t kNoLatch = 0xffff;

    Touch& _touch;
    Core& _core;

    MValue _fltr_val;
    MValue _inp_val;

    MValue _verb_send;
    MValue _verb_fb;

    MValue _blur;
    MValue _flutter;

    std::bitset<Knobs::Count> _apply;

    daisy::UiEventQueue _ui_queue;
    daisy::PotMonitor<Knobs, Knobs::Count> _pot_monitor;
    void _process_ui_queue();

    daisy::StopwatchTimer _init_timer;

    std::bitset<Pads::Count> _touched;
    void _on_pad_touch(Pads::Pad);
    void _on_pad_release(Pads::Pad);
    void _release_pad(Pads::Pad);
    void _release_latched();

    daisy::MidiUsbHandler _midi;
    bool _process_midi();
    bool _process_realtime(daisy::MidiEvent&);
    void _process_note_on(daisy::NoteOnEvent&);

    Pads::Pad _latched_pad;
    bool _is_latched;
    bool _is_init;
};

}; // namespace touchb
}; // namespace synthux
