#include "fx.h"
#include "common/common.h"

using namespace touchb;
using namespace infrasonic;
using namespace daisysp;

void Fx::init(const Params p)
{
}

void Fx::process(float& inout0, float& inout1)
{
    float out[2] = { inout0, inout1 };

    //

    inout0 = out[0];
    inout1 = out[1];
}
