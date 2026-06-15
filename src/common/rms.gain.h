#pragma once 

#include "nocopy.h"

namespace bleeptools {
    
class RMSGain {
public:
    RMSGain();
    ~RMSGain() = default;

    void init(const float sample_rate);
    void process(float& inout0, float& inout1);

private:
    NOCOPY(RMSGain)

    static constexpr auto kTargetRMS = .2f;

    float _rms;
    float _gain;
    float _attack_kof;
    float _release_kof;

};
};
