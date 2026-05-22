#include "pads.h"

using namespace synthux;
using namespace daisy;

void Pads::init(DaisySeed& hw) {
    Mpr121I2C::Config cfg;
    _mpr.Init(cfg);
};

void Pads::process() {
    uint16_t pad;
    bool is_touched;
    bool was_touched;
    auto state = _mpr.Touched();
    for (uint16_t i = 0; i < 12; i++) {
        pad = 1 << i;
        is_touched = state & pad;
        was_touched = _state & pad;
        if (_on_touch != nullptr && is_touched && !was_touched) {
            _on_touch(static_cast<Pad>(i));
        }
        else if (_on_release != nullptr && was_touched && !is_touched) {
            _on_release(static_cast<Pad>(i));
        }
    }
    _state = state;
};
