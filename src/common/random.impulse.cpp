#include "random.impulse.h"

#include <algorithm>
#include "expose.h"

using namespace bleeptools;

RandomImpulse::RandomImpulse():
_freq_hz        { 1.f },
_imp_sec        { .05f },
_prob           { .6f },
_value          { 0.f },
_iterator       { 0 },
_period_samp    { 0 },
_imp_samp       { 0 },
_value_dist     { URD{  0.f, 1.f } },
_prob_dist      { URD{  0.f, 1.f } },
_period_dist    { URD{ 0.5f, 1.5f } }
{};

void RandomImpulse::init(const float sample_rate)
{
    _sample_rate = sample_rate;
    _period_samp = _new_period_samp();
    _imp_samp = _new_imp_samp();
}

void RandomImpulse::set_freq_hz(const float hz)
{
    _freq_hz = hz;
    _imp_samp = _new_imp_samp();
}

void RandomImpulse::set_imp_sec(const float seconds)
{
    _imp_sec = seconds;
    _imp_samp = _new_imp_samp();
}

void RandomImpulse::set_prob(const float val)
{
    _prob = std::clamp(val, 0.f, 1.f);
}

float RandomImpulse::process()
{
    if (_iterator >= _period_samp) {
        _iterator = 0;
        _period_samp = _new_period_samp();

        if (_prob_dist(_rnd) < _prob) {
            _value = _value_dist(_rnd);
            _imp_samp = _new_imp_samp();
        }
        else {
            _value = 0.f;
        }
    }
    auto output = (_iterator < _imp_samp) ? _value : 0.f;
    _iterator++;
    return output;
}

uint32_t RandomImpulse::_new_period_samp()
{
    auto mean = _sample_rate / _freq_hz;

    auto jitter = _period_dist(_rnd);
    auto period = static_cast<uint32_t>(mean * jitter);

    return std::max(period, 2ul);
}

uint32_t RandomImpulse::_new_imp_samp() const
{
    auto mean = _sample_rate / _freq_hz;
    auto max_imp_sec  = (mean * .95f) / _sample_rate;  // 95 % of mean period
    auto clamp_sec = std::clamp(_imp_sec, 1.f / _sample_rate, max_imp_sec);
    auto imp_sec = static_cast<uint32_t>(clamp_sec * _sample_rate);
    return std::max(imp_sec, 1ul);
}
