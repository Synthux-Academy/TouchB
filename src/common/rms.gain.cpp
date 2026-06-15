#include "rms.gain.h"
#include <arm_math.h>

using namespace bleeptools;
    
RMSGain::RMSGain():
_rms    { 0.f },
_gain   { 1.f }
{}

void RMSGain::init(const float sample_rate) {
    _attack_kof     = expf(-1.f / (sample_rate * 0.02f)); // 50ms
    _release_kof    = expf(-1.f / (sample_rate * 0.05f)); // 100ms
}

void RMSGain::process(float& inout0, float& inout1) {
    // M/S weighted power: perceptually accurate for stereo loudness
    // Mid carries core loudness, Side adds width but contributes less
    auto mid   = 0.5f * (inout0 + inout1);
    auto side  = 0.5f * (inout0 - inout1);
    auto power = 0.7f * mid * mid + 0.3f * side * side;

    auto rms_sq = _rms * _rms;
    auto kof = (power > rms_sq) ? _attack_kof : _release_kof;
    arm_sqrt_f32(kof * rms_sq + (1.f - kof) * power, &_rms);

    auto target_gain = (_rms > 1e-6f) ? kTargetRMS / _rms : 1.f;
    _gain += .001f * (target_gain - _gain);

    inout0 *= _gain;
    inout1 *= _gain;
};
