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
#include "nocopy.h"

namespace bleeptools {

// The square law crossfade
// Adopted from Will. C. Pirkle "Designing Software Synthesizer Plugins in C++".
class XFade {
public:
  XFade():
    _stage  { 0.f },
    _lhs    { 1.f },
    _rhs    { 0.f } 
    {}

  void process(const float lhs0, const float lhs1, const float rhs0, const float rhs1, float& out0, float& out1) 
  {
    out0 = lhs0 * _lhs + rhs0 * _rhs;
    out1 = lhs1 * _lhs + rhs1 * _rhs;
  }

  void set_stage(const float value) 
  {
    _stage = value;
    _calculate_multipliers();
  }

  void set_inverted(const bool invert) 
  {
    _is_inverted = invert;
    _calculate_multipliers();
  }

  float stage() const 
  {
    return _stage;
  }

private:
  NOCOPY(XFade)

  void _calculate_multipliers()
  {
    auto stage = std::clamp(_is_inverted ? 1.f - _stage : _stage, 0.f, 1.f);
    auto sq = _stage * _stage;
    _lhs = 1.f - sq;
    _rhs = 2.f * _stage - sq;
  }

  float _stage;
  float _lhs;
  float _rhs;
  bool _is_inverted;
};

};
