#pragma once

#include <functional>
#include <daisy_seed.h>
#include "dev/mpr121.h"
#include "nocopy.h"

namespace synthux {

class Pads {
public:
    enum Pad: uint16_t {
        TopLeft,
        TopCenter,
        TopRight,
        Left,
        CenterLeft,
        Center,
        CenterRight,
        Right,
        BottomLeft,
        BottomRight,
        Tou,
        Ch,
        Count
    };

    Pads(): _state { 0 } {}
    ~Pads() {}

    void init(daisy::DaisySeed& hw);
    void process();

    void set_on_touch(std::function<void(Pad)> on_touch) {
        _on_touch = on_touch;
    };
    void set_on_release(std::function<void(Pad)> on_release) {
        _on_release = on_release;
    };

private:
    NOCOPY(Pads)

    uint16_t _state;
    daisy::Mpr121I2C _mpr;

    std::function<void(Pad)> _on_touch;
    std::function<void(Pad)> _on_release;
};

};
