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

#include <daisy_seed.h>

namespace bleeptools {

template<uint32_t duration_ms>
class Hold {
public:
    void init() {
        _timer.Init();
    }
    void begin() {
        if (!_is_holding) {
            _timer.Restart();
            _is_holding = true;
        }
    }
    bool process() {
        if (_passed) {
            return true;
        }
        else {
            _passed = _passed_threshold();
            return _passed;
        }
    }
    bool inidcate() {
        return _indicate;
    }
    bool is_holding() { 
        return _is_holding; 
    }
    bool passed() {
        return _passed;
    }
    void end() {
        _is_holding = false;
        _indicate = false;
        _passed = false;
    }

private:
    bool _passed_threshold() {
        if (!_is_holding) return false;
        if (_timer.HasPassedMs(50)) {
            _indicate = true;
        }
        if (_timer.HasPassedMs(duration_ms)) {
            _indicate = false;
            return true;
        }
        return false;
    }
    daisy::StopwatchTimer _timer;
    bool _is_holding;
    bool _indicate;
    bool _passed;
};

};
