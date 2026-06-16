#pragma once

#include <daisysp.h>

#include "synthux.h"
#include "nocopy.h"

namespace synthux {
namespace touchb {

class Fx {
public:
    enum Type: uint8_t {
        drive,
        reduce,
        Count,
        None = 0xff
    };

    struct Params {
        std::array<Type, Type::Count> types;    
        uint8_t type_count;

        //Drive
        float drive;

        //Reduce
        float downsample;
        uint8_t bits;
    };

    Fx();
    ~Fx() = default;

    void init(const float sample_rate);
    void process(float& inout0, float& inout1);

    void set_flavor_norm(const float);

    void engage(const Params);
    void disengage();
    
private:
    NOCOPY(Fx)

    void _validate();

    std::array<daisysp::Overdrive, 2> _drive;
    std::array<daisysp::Decimator, 2> _decimator;

    RMSGain _drive_gain;
    RMSGain _reduce_gain;

    SoftSwitch _bypass;

    float _flavor;
    float _drive_amnt;
    float _downsample_amnt;
    uint8_t _bits_reduce_amnt;
    
};

};
};
