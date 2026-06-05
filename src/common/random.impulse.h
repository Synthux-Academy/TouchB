#pragma once

#include <random>
#include <cstdint>

/**
 * RandomImpulse
 *
 * Generates random short impulses with configurable frequency, impulse length,
 * and probability. Similar to sample-and-hold, but the hold time is shorter
 * than the generation period, producing discrete bursts rather than a
 * continuous stepped signal.
 *
 * Signal flow per cycle:
 *   |<----------- period (randomised) ----------->|
 *   |< impulse length >|__ silence _______________|
 *   ^                  ^
 *   random value drawn  output falls to 0
 *
 * Parameters (all settable at any time):
 *   frequency      [Hz]  – controls the mean rate of new cycles (> 0)
 *   impulseLength  [s]   – how long each impulse lasts (clamped < period)
 *   probability    [0-1] – chance that a new cycle actually fires a pulse
 *
 * Usage:
 *   RandomImpulse gen;
 *   gen.init(44100.f);
 *   gen.set_freq_hz(2.f);
 *   gen.set_imp_sec(0.05f);
 *   gen.set_prob(0.75f);
 *
 *   for (int i = 0; i < numSamples; ++i)
 *       outputBuffer[i] = gen.process();
 */
namespace bleeptools {

class RandomImpulse
{
public:
    RandomImpulse();
    ~RandomImpulse() = default;

    void init(const float sample_rate);
    float process();
    
    /** Mean impulse frequency in Hz. Must be > 0. */
    void set_freq_hz(const float hz);

    /**
     * Duration of each impulse in seconds.
     * Internally clamped to [1 sample … 95 % of the current mean period]
     * so there is always a gap between impulses.
     */
    void set_imp_sec(const float);

    /**
     * Probability that a cycle trigger actually fires a pulse.
     * 0.0 = never fires, 1.0 = always fires.
     */
    void set_prob(const float);

private:

    uint32_t _new_period_samp();
    uint32_t _new_imp_samp() const;

    float    _sample_rate;
    float    _freq_hz;
    float    _imp_sec;
    float    _prob;
    float    _value;

    uint32_t _iterator;
    uint32_t _period_samp;
    uint32_t _imp_samp;

    using URD = std::uniform_real_distribution<float>;

    std::random_device _rnd;
    URD _value_dist;
    URD _prob_dist;
    URD _period_dist;
};
};
