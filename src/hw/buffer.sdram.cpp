#include "buffer.sdram.h"

using namespace synthux::touchb;

#define ALIGN32K __attribute__((aligned(32768)))
inline constexpr size_t aligned(size_t in)
{
    return in + (32768 - in % 32768);
}

static constexpr size_t kSourceBufferLength = kSourceMaxSeconds * kSampleRate;
static Buffer::Frame DSY_SDRAM_BSS ALIGN32K _srcBuf[aligned(kSourceBufferLength)];

SDRAMBuffer::SDRAMBuffer()
{
    auto srcbs = kSourceBufferLength * sizeof(Buffer::Frame);
    std::memset(_srcBuf, 0, srcbs);
};

Buffer::Frame* SDRAMBuffer::sourceBuffer() 
{
    return _srcBuf;
};

uint32_t SDRAMBuffer::sourceBufferSize() 
{
    return kSourceBufferLength;
}
