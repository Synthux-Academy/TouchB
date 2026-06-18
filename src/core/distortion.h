#pragma once

#include <daisysp.h>

#include "synthux.h"
#include "nocopy.h"
#include "folder.h"

namespace synthux {
namespace touchb {

class Distortion {
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

    Distortion();
    ~Distortion() = default;

    void init(const float sample_rate);
    void process(float& inout0, float& inout1);

    void set_flavor_norm(const float);

    void engage(const Params);
    void disengage();
    
private:
    NOCOPY(Distortion)

    void _validate();

    std::array<daisysp::Overdrive, 2> _drive;
    std::array<daisysp::Decimator, 2> _decimator;
    std::array<synthux::Folder, 2> _folder;

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
