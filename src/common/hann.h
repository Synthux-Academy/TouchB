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

#include <array>

namespace bleeptools {

static constexpr auto kHannCurveSize = 192;
static constexpr auto PI2 = 1.570796370506287f;

static std::array<float, kHannCurveSize> hannCurve() {
  std::array<float, kHannCurveSize> slope { 0 };
  for (size_t i = 0; i < kHannCurveSize; i++) {
    auto sin = std::sin(PI2 * static_cast<float>(i) / static_cast<float>(kHannCurveSize - 1));
    slope[i] = std::clamp(sin * sin, 0.f, 1.f);
  }
  return slope;
}

static auto HannCurve = hannCurve();

static float Hann_Value_At(const float norm_pos)
{
  auto pos = (kHannCurveSize - 1) * norm_pos;
  auto int_pos = static_cast<size_t>(pos);
  auto frac = pos - static_cast<float>(int_pos);
  auto n_pos = int_pos + 1;
  if (n_pos >= kHannCurveSize) n_pos = kHannCurveSize - 1;
  auto v = HannCurve[int_pos];
  auto n = HannCurve[n_pos];
  return std::clamp(v + frac * (n - v), 0.f, 1.f);
};

};
