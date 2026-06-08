#pragma once

#include <string.h>
#include <stdint.h>

namespace synthux {
namespace touchb {

// Clock ........................................
static constexpr uint8_t kPPQNIntern = 48;

// Buffer
static constexpr int32_t kRecordFade = 192; // 4ms

// Grain ........................................
static constexpr size_t kWindowSlope = 960; //20ms @ 48K 1x
static constexpr size_t kMinimumWindowSize = 2 * kWindowSlope; //40ms @ 48k 1x
static constexpr size_t kDefaultWindowSize = 2880; //60ms @ 48k 1x

// Slice ........................................
static constexpr size_t kSliceSlope = 192; //4ms
static constexpr size_t kSliceMinSize = 2 * kSliceSlope + 2016; //+42ms sustain = 50ms @ 48K 1x 

// Reverb .......................................
static constexpr float kReverbFeedback = .6f;
static constexpr float kReverLPFreq = 10000.f; //Hz

};
};
