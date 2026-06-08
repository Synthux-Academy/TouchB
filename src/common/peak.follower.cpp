#include "peak.follower.h"

using namespace bleeptools;


PeakFollower::PeakFollower():
_out { 0.f }
{
    set_release_time_s(.2f);
}

void PeakFollower::init(const float sample_rate)
{
    _sample_rate = sample_rate;
}

void PeakFollower::set_release_time_s(const float sec) 
{
    _release_kof = std::exp(-1.0f / (sec * _sample_rate));
}

float PeakFollower::process(float input) 
{
    auto diff = _out - input;
    if (input < _out && std::abs(diff) > .002f) {
        _out = input + _release_kof * diff;
    }
    else {
        reset(input);
    }
    return _out;
}

void PeakFollower::reset(const float value) 
{
    _out = value;
}