#pragma once

#include <array>

#include <daisy_seed.h>
#include "nocopy.h"

namespace synthux {

class Knobs {
public:
    enum Knob: uint16_t {
        s30,
        s31,
        s32,
        s33,
        s34,
        s35,
        s36,
        s37,
        Count
    };

    Knobs() = default;
    ~Knobs() = default;

    void init(daisy::DaisySeed& hw);
    void process();

    // Implements PotMonitor Backend ////////////////
    /////////////////////////////////////////////////
    inline float GetPotValue(uint16_t pot_id)
    {
        return _knobs[pot_id].Value();
    }

private:
    NOCOPY(Knobs)

    std::array<daisy::AnalogControl, Count> _knobs;
};
};
