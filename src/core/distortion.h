#pragma once

#include <daisysp.h>

#include "synthux.h"
#include "nocopy.h"
#include "folder.h"
#include "level.h"

namespace synthux {
namespace touchb {

class Distortion {
public:
    enum class Type: uint8_t {
        wah,
        fold,
        reduce,
        Count,
        None
    };

    struct Params {
        std::array<Type, (size_t)Type::Count> types;
        uint8_t type_count;

        //Fold
        float fold;
        float fold_gain_kof;
        float fold_mix;

        //Reduce
        float downsample;
        uint8_t bits;
        float reduce_mix;

        //Wah
        float wah;
        float wah_mix;

        //Output
        float level;
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

    Level _fold_level;
    std::array<synthux::Folder, 2> _fold;
    std::array<daisysp::Decimator, 2> _decimator;
    std::array<daisysp::Autowah, 2> _wah;

    SoftSwitch _wah_fold_switch;
    SoftSwitch _bypass;

    float _flavor;

    float _fold_amnt;
    float _fold_gain_kof;
    float _fold_mix;
    OnePoleSmoother _fold_amnt_smooth;

    float _wah_amnt;
    float _wah_mix;
    OnePoleSmoother _wah_amnt_smooth;

    float _downsample_amnt;
    uint8_t _bits_reduce_amnt;

    float _level;

};

};
};
