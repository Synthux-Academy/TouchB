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
#include "nocopy.h"

namespace bleeptools {

class MValue {
public:
  MValue();
  ~MValue() {}

int id() const { return _id; }

float process(const float value, const bool active, int* id);

bool is_tracking() const { return _is_tracking; }

float in_value() const { return _in_value; }

float value() const { return _value; }

void set(const float value) {
  _is_tracking = false;
  _is_active = false;
  _value = value;
}

private:
  NOCOPY(MValue)

  bool _set_active(const bool active);

  static constexpr float kTreshold = 0.02;

  int _id;

  float _in_value;
  float _value;

  bool _is_active;
  bool _is_tracking;
};

};
