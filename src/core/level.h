//
// A slow, program-dependent gain rider.
//
// Approach:
//   1. Track a *long-term* loudness estimate (hundreds of ms to seconds).
//   2. Compute a target gain from that slow estimate, but also RATE-LIMIT
//      how fast the gain itself is allowed to change (dB/sec). This means
//      the gain drifts to match the overall level of a passage, but cannot
//      chase an individual transient.
//   3. Apply that gain BEFORE your nonlinear stage (wavefolder), so the
//      folder always sees a roughly consistent drive level -> consistent
//      fold/tone character regardless of source.
//   4. Apply the INVERSE of that same gain AFTER the nonlinear stage. This
//      restores the original macro-dynamics (loud sections come back loud,
//      quiet sections come back quiet), i.e. classic compander behaviour
//      wrapped around the nonlinearity, instead of a leveler flattening
//      everything to one output loudness.
//
// Usage:
//   Level level;
//   level.init(sample_rate);
//   ...
//   float bus0, bus1;
//
//   level.pre_gain(bus0, bus1);
//   ....do processing....
//   level.post_gain(bus0, bus1);
//
// Tuning:
//   set_target_level(lvl)          // desired RMS amplitude feeding the folder (0..1)
//   set_loudness_window(sec)       // how long a "phrase" is, e.g. 0.3 - 1.0s
//   set_max_gain_rate_db(dbps)     // how fast gain is allowed to move, e.g. 6-12 dB/s
//   set_gain_range_db(minDb,maxDb) // clamp, e.g. -24 .. +24 dB
//   set_hold_threshold(lvl)        // below this input level, freeze gain (avoid
//                                  // riding noise floor / silence between hits)

#pragma once

#include <cmath>
#include <algorithm>
#include "common.h"

namespace synthux {
 namespace touchb {

class Level {
public:
    Level() = default;
    ~Level() = default;

    void init(const float sample_rate)
    {
        sample_rate_ = sample_rate;
        set_loudness_window(.5f);          // 500ms long-term loudness estimate
        set_max_gain_rate_db(8.f);         // gentle: 8 dB/sec max gain change
        set_target_level(.3f);             // target RMS amplitude into the folder
        set_gain_range_db(-24.f, 24.f);
        set_hold_threshold(.001f);         // ~ -60 dBFS: freeze gain in silence

        env_    = 0.0f;
        gain_db_ = 0.0f;
    }

    // --- Configuration -----------------------------------------------

    void set_target_level(const float lvl) 
    { 
        target_ = std::max(lvl, 1e-6f); 
    }

    void set_loudness_window(const float seconds)
    {
        // one-pole coefficient for the long-term RMS-ish envelope
        auto limited = std::max(seconds, 0.01f);
        env_coeff_   = std::exp(-1.0f / (limited * sample_rate_));
    }

    void set_max_gain_rate_db(const float db_per_sec)
    {
        max_gain_step_db_ = std::max(db_per_sec, 0.01f) / sample_rate_; // per-sample cap
    }

    void set_gain_range_db(const float min_db, const float max_db)
    {
        mingain_db_ = min_db;
        maxgain_db_ = max_db;
    }

    void set_hold_threshold(const float lvl) 
    { 
        holdThresh_ = lvl; 
    }

    void pre_gain(float& inout0, float& inout1)
    {
        // slow, squared-envelope ("RMS-ish") long term loudness estimate
        auto sq = std::max(inout0 * inout0, inout1 * inout1);
        env_ = env_coeff_ * env_ + (1.0f - env_coeff_) * sq;
        auto rms = std::sqrt(env_);

        // desired gain to bring that long-term level to target, in dB
        float desired_db;
        if (rms < holdThresh_) {
            // signal has dropped into near-silence: don't move gain,
            // just hold last value so we don't ride up noise / decay tails
            desired_db = gain_db_;
        }
        else {
            auto g   = target_ / rms;
            desired_db = infrasonic::lin2dbfs(std::max(g, 1e-6f));
            desired_db = std::clamp(desired_db, mingain_db_, maxgain_db_);
        }

        // rate-limit the gain movement so it can't chase individual
        // transients -- this is what keeps drum hits from "pumping"
        auto diff = desired_db - gain_db_;
        auto step = std::clamp(diff, -max_gain_step_db_, max_gain_step_db_);
        gain_db_ += step;

        last_linear_gain_ = infrasonic::dbfs2lin(gain_db_);
        inout0 *= last_linear_gain_;
        inout1 *= last_linear_gain_;
    }

    // Call once per sample, AFTER your nonlinear stage, on the same
    // sample index as the matching ProcessPre() call. Applies the
    // complementary inverse gain to restore original macro-dynamics.
    void post_gain(float& inout0, float& inout1)
    {
        // inverse of the pre-gain currently in effect
        auto inv_gain = 1.0f / std::max(last_linear_gain_, 1e-6f);
        inout0 *= inv_gain;
        inout1 *= inv_gain;
    }

    float get_current_gain_db() const { return gain_db_; }

private:
    float sample_rate_;

    float target_;
    float env_coeff_;
    float max_gain_step_db_;
    float mingain_db_;
    float maxgain_db_;
    float holdThresh_;

    float env_;
    float gain_db_;
    float last_linear_gain_;
};

}
}
