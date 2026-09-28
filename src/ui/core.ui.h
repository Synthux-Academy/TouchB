#pragma once

#include <functional>
#include <bitset>
#include <daisy_seed.h>

#include "hw/touch.h"
#include "core/core.h"

#include "common/mvalue.h"
#include "latch.h"

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

    MValue _mix_val;
    MValue _out_val;

    MValue _verb_send;
    MValue _verb_fb;

    MValue _blur;
    MValue _flutter;

    MValue _filter_val;
    MValue _in_val;

    Latch<7> _latch;

    std::bitset<Knobs::Count> _apply;
    

    daisy::UiEventQueue _ui_queue;
    daisy::PotMonitor<Knobs, Knobs::Count> _pot_monitor;
    void _process_ui_queue();

    daisy::StopwatchTimer _init_timer;

    std::bitset<Pads::Count> _touched;
    void _on_pad_touch(Pads::Pad);
    void _on_pad_release(Pads::Pad);
    void _on_latch_on(const uint8_t);
    void _on_latch_off(const uint8_t);

    daisy::MidiUsbHandler _midi;
    bool _process_midi();
    bool _process_realtime(daisy::MidiEvent&);
    void _process_note_on(daisy::NoteOnEvent&);

    bool _is_init;
};

}; // namespace touchb
}; // namespace synthux
