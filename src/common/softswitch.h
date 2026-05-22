// Copyright 2026 bleeptools
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------

#pragma once

#include <algorithm>
#include "hann.h"
#include "nocopy.h"

namespace bleeptools {

/**
 * @brief
 * A four milliseconds micro ASR envelope. 
 * Usable for things like muting the sound preventing a click.
 */
class SoftSwitch {
public:
  SoftSwitch(): 
  _out { 0.f },
  _stage { Stage::idle } 
  {}
  
  ~SoftSwitch() {}

  void init(const float sample_rate) {
    _kof = 1.f / (.004f * sample_rate); //4 ms
  }

  void set_on(const bool on, const bool immediate = false) {
    _on = on;
    if (immediate) {
      if (_on) _stage = Stage::hold;
      else _stage = Stage::idle;
    }
  }

  bool is_on() const {
    return _on;
  }

  bool is_idle() const 
  {
    return _stage == Stage::idle;
  }

  float process(const bool inverse = false) 
  {
    switch (_stage) {
      case Stage::idle:
        _out = 0;
        _iterator = 0;
        if (_on) _stage = Stage::rise;
        break;

        case Stage::rise:
          if (!_on) _stage = Stage::fall;
          else _out = Hann_Value_At(_iterator * _kof);
          if (++_iterator >= 191) _stage = Stage::hold;
          break;

        case Stage::hold:
          _out = 1.f;
          _iterator = 191;
          if (!_on) _stage = Stage::fall;
          break;

        case Stage::fall:
          if (_on) _stage = Stage::rise;
          else _out = Hann_Value_At(_iterator * _kof);
          if (--_iterator <= 0) _stage = Stage::idle;
          break;
        }
        return std::clamp(inverse ? 1.f - _out : _out, 0.f, 1.f);
  }

private:
  NOCOPY(SoftSwitch)

  enum class Stage {
    idle,
    rise,
    hold,
    fall
  };

    int32_t _iterator;
    float _kof;
    float _out;
    Stage _stage;
    bool _on;
};

};
