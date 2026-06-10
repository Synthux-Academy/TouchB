#pragma once

#include <daisysp.h>

#include "bleeptools.h"
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
        float drive_vol_comp_dbfs;

        //Reduce
        float downsample;
        uint8_t bits;        
        float reduce_vol_comp_dbfs;

        
    };

    Fx() = default;
    ~Fx() = default;

    void init(const float sample_rate);
    void process(float& inout0, float& inout1);

    void engage(const Params);
    void disengage();
    
private:
    NOCOPY(Fx)

    std::array<daisysp::Overdrive, 2> _drive;
    std::array<daisysp::Decimator, 2> _decimator;

    bleeptools::SoftSwitch _drive_on;
    bleeptools::SoftSwitch _reduce_on;

    float _drive_comp;
    float _reduce_comp;
};

};
};
