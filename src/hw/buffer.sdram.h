#pragma once

#include <assert.h>
#include <daisy_seed.h>

#include "nocopy.h"
#include "core/buffer.h"

namespace synthux {
namespace touchb {

static constexpr uint32_t kSampleRate       { 48000 };
static constexpr uint8_t kSourceMaxSeconds  { 5 };

class SDRAMBuffer {
public:
    static SDRAMBuffer& pool() {
        static SDRAMBuffer instance;
        return instance;
    }

    Buffer::Frame* sourceBuffer();
    uint32_t sourceBufferSize();

private:
    NOCOPY(SDRAMBuffer)

    SDRAMBuffer();
    ~SDRAMBuffer() = default;
};
};
};