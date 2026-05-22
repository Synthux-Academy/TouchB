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
#include <cmath>

namespace bleeptools {

class OnePoleSmoother {
public:
    OnePoleSmoother(): 
    _kof            { 1.f }, 
    _value          { 0.f },
    _is_smoothing   { false }
    {}
    ~OnePoleSmoother() = default;

    void init(const float sample_rate, const float time_s = 0.001f) {
        if (time_s <= 0.f || sample_rate <= 0.0f) {
            _kof = 1.f;
        } 
        else {
            _kof = 1.0f / (time_s * sample_rate);
        }
    }

    float process(const float target_value) {
        auto diff = target_value - _value;
        if (!_is_smoothing && std::abs(diff) < .002f) return _value;
        
        _is_smoothing = true;
        _value += _kof * diff;
        
        if (std::abs(target_value - _value) < .002f) {
            _value = target_value;
            _is_smoothing = false;
        }
        
        return _value;
    }

    void reset(const float value = 0.0f) {
        _value = value;
        _is_smoothing = false;
    }

private:
    float _kof;
    float _value;
    bool _is_smoothing;
};

};
