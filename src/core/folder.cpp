#include "folder.h"
#include <algorithm>

using namespace synthux;

Folder::Folder():
_gain       { 1.f },
_offset     { 0.f },
_threshold  { .8f }
{}

void Folder::process(float& inout)
{
    auto val = inout * _gain + _offset;
    auto t2 = 2.f * _threshold;
    auto o2 = 2.f * _offset;
    
    // Upper half
    auto over_thresh = val > _threshold;
    auto folds = over_thresh ? 5 : 0;
    while ((over_thresh || val < _offset) && folds--) {
        if (over_thresh) val = t2 - val;
        if (val < _offset) val = o2 - val;
        over_thresh = val > _threshold;
    }
    
    // Lower half
    over_thresh = val < -_threshold;
    folds = over_thresh ? 5 : 0;
    while ((over_thresh || val > _offset) && folds--) {
        if (over_thresh) val = -t2 - val;
        if (val > _offset) val = o2 - val;
        over_thresh = val < -_threshold;
    }
    inout = val;
}

void Folder::set_threshold_norm(float norm) 
{ 
    norm = std::abs(norm);
    norm = std::clamp(norm, 0.f, 1.f);
    auto delta = .1f;
    if (_offset > 0 && norm <= _offset) _threshold = _offset + delta;
    else if (_offset < 0 && norm >= -_offset) _threshold = _offset - delta;
    else _threshold = norm;
}

void Folder::set_offset_norm(float norm)
{
    norm = std::clamp(norm, 0.f, 1.f);
    auto delta = .1f;
    if (norm >= _threshold) _offset = _threshold - delta;
    else if (norm <= -_threshold) _offset = delta - _threshold;
    else _offset = norm;
}

void Folder::set_gain_mult(const float norm) 
{ 
    _gain = std::abs(norm);
}
