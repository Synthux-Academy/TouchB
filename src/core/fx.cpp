#include "fx.h"
#include "common/common.h"

using namespace touchb;
using namespace daisysp;
using namespace infrasonic;

void Fx::init(const float sample_rate)
{
    _drive.Init();

    _decimator.Init();
    _decimator.SetSmoothCrushing(false);
}

void Fx::process(float& inout0, float& inout1)
{
    float out[2] = { inout0, inout1 };

    //
    inout0 = out[0];
    inout1 = out[1];
}

void Fx::engage(const Params p)
{
    disengage();

    
    
    for (auto t: p.types) {
        switch (t) {
            case Type::drive:
                _drive_on.set_on(true);
                _drive.SetDrive(p.drive);
                auto dbfs_comp = map(unitclamp(p.vol_comp), 0.f, 1.f, -40.f, -10.f);
                _drive_comp = dbfs2lin(dbfs_comp);
                break;

            case Type::reduce:
                _reduce_on.set_on(true);
                _decimator.SetBitsToCrush(p.bits);
                _decimator.SetDownsampleFactor(p.downsample);
        }
    }
}

void Fx::disengage()
{
    _drive_on.set_on(false);
    _reduce_on.set_on(false);
}