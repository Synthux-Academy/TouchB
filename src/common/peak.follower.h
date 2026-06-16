#pragma once

#include <cmath>
#include "smooth.h"
#include "nocopy.h"

namespace synthux {

class PeakFollower {
public:
    PeakFollower();
    ~PeakFollower() = default;

    void init(const float sample_rate);
    float process(float input);
    
    void set_release_time_s(const float sec);
    void reset(const float value = 0.0f);
    
    float value();

private:
    OnePoleSmoother _smooth;

    float _sample_rate;
    float _release_kof;
    float _out;
};
};
