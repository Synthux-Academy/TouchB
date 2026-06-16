#pragma once

#include <cstdlib>
#include <array>
#include <stdint.h>

#include "common/xfade.h"
#include "common/softswitch.h"
#include "config.h"
#include "nocopy.h"

namespace synthux {
namespace touchb {

class Buffer {
public:
    struct Frame {
        float l;
        float r;
    };  

    Buffer();
    ~Buffer() {};

    void init(Frame* buf, size_t length);

    void read_linear(float frame, float& out0, float& out1);
    
    void set_recording(const bool is_rec_on);
    bool is_recording() const { return _state != State::idle; }
    bool is_overdubbing() const { return _cut_switch.is_on() && is_recording(); };
    void set_feedback(const float val); 
    void write(const float in0, const float in1);
    void cut();
    void clear();

    float norm_rec_size() const { return _size * _buffer_size_kof; }
    size_t rec_size() const { return _size; }
    void set_rec_size(const size_t);
    bool is_empty() const { return _size == 0; }
    
    size_t read_head() const { return _read_head; }
    size_t write_head() const { return _write_head; }

    Frame* raw() const { return _buffer; }
    size_t size() const { return _buffer_size; }

private:
    NOCOPY(Buffer)

    static constexpr auto kFadeCurveKof = 1.f / kRecordFade;
    void _read(size_t frame, float& out0, float& out1);
    void _start_recording();

    enum class State: uint8_t {
        idle,
        fadein,
        sustain,
        fadeout
    };

    SoftSwitch _cut_switch;

    Frame*  _buffer;
    size_t  _buffer_size;
    float   _buffer_size_kof;
    float   _feedback;
    size_t  _size;
    size_t  _target_length;
    size_t  _write_head;
    size_t  _write_counter;
    size_t  _read_head;
    int32_t _fade_counter;
    State   _state;
    bool    _did_cut;
    bool    _is_pending;
};

};
};