#include "detector.h"
#include "daisysp.h"
#include <math.h>
#include "expose.h"

using namespace synthux::touchb;

Detector::Detector()
{
    _attack_ms = std::exp(kof / 8 /* ms */);
    _release_ms = std::exp(kof / 16 /* ms */);
}

bool Detector::is_open() const 
{ 
	return 20.f * daisysp::fastlog10f(_avg) > -60.f;
}

void Detector::process(const float in)
{
	//Get current value 
	auto mult = in > _avg ? _attack_ms : _release_ms;
	_avg = mult * (_avg - in) + in;
}
