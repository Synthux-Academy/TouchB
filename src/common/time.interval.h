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

#include <sys/system.h>

namespace bleeptools
{
/// Derived from daisy StopwatchTimer

template<size_t seconds>
class TimeInterval
{
  public:
    TimeInterval()   = default;
    ~TimeInterval()  = default;

    inline bool is_passed()
    {
      if (_scheduled) {
        const auto now = daisy::System::GetTick();
        if (daisy::System::GetUsBetweenTicks(now, _last) >= seconds * 1000) {
          _scheduled = false;
          return true;
        }
      }
      return false;
    }

    inline void start() { 
      _scheduled = true;
      _last = daisy::System::GetTick(); 
    }

  private:
    uint32_t _last;
    bool _scheduled;
};

};
