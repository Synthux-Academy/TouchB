#pragma once

#include <daisysp.h>

#include "synthux.h"
#include "nocopy.h"

namespace synthux {
namespace touchb {

class Distortion {
public:
    enum class Type: uint8_t {
        wah,
        drive,
        reduce,
        Count,
        None
    };

    struct Params {
        std::array<Type, (size_t)Type::Count> types;
        uint8_t type_count;

        //Drive
        float drive;
        float drive_mix;

        //Reduce
        float downsample;
        uint8_t bits;
        float reduce_mix;

        //Wah
        float wah;
        float wah_mix;
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
    std::array<daisysp::Autowah, 2> _wah;
    
    SoftSwitch _drive_fold_switch;
    SoftSwitch _bypass;

    float _flavor;

    float _drive_amnt;
    float _drive_mix;
    OnePoleSmoother _drive_amnt_smooth;

    float _wah_amnt;
    float _wah_mix;
    OnePoleSmoother _wah_amnt_smooth;

    float _downsample_amnt;
    uint8_t _bits_reduce_amnt;

};

};
};
