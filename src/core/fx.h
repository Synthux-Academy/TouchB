#pragma once

#include "nocopy.h"

namespace touchb {

class Fx {
public:
    struct Params {
        float sample_rate;
    };

    Fx() = default;
    ~Fx() = default;

    void init(const Params);
    void process(float& inout0, float& inout1);
    
private:
    NOCOPY(Fx)
};

};
