#include "rms.gain.h"
#include <arm_math.h>

using namespace bleeptools;
    
RMSGain::RMSGain():
_rms    { 0.f },
_gain   { 1.f }
{}

void RMSGain::init(const float sample_rate) {
    float32_t in[1];
    float32_t out[1];

    in[0] = -1.f / (sample_rate * 0.1f);
    
    _attack_kof     = expf(-1.f / (sample_rate * 0.1f)); // 100ms attack
    _release_kof    = expf(-1.f / (sample_rate * 0.4f)); // 400ms release
}

float RMSGain::process(const float in0, const float in1, float& out0, float& out1) {
    // M/S weighted power: perceptually accurate for stereo loudness
    // Mid carries core loudness, Side adds width but contributes less
    auto mid   = 0.5f * (in0 + in1);
    auto side  = 0.5f * (in0 - in1);
    auto power = 0.7f * mid * mid + 0.3f * side * side;

    auto rms_sq = _rms * _rms;
    auto kof = (power > rms_sq) ? _attack_kof : _release_kof;
    arm_sqrt_f32(kof * rms_sq + (1.f - kof) * power, &_rms);

    auto target_gain = (_rms > 1e-6f) ? kTargetRMS / _rms : 1.f;
    _gain += .001f * (target_gain - _gain);

    out0 = in0 * _gain;
    out1 = in1 * _gain;
};
