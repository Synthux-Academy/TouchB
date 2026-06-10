#include "fx.h"
#include "common/common.h"

using namespace synthux::touchb;
using namespace daisysp;
using namespace infrasonic;

void Fx::init(const float sample_rate)
{
    // Drive
    _drive_on.init(sample_rate);
    for (auto& d: _drive) d.Init();
    
    // Reduce
    _reduce_on.init(sample_rate);
    for (auto& d: _decimator) {
        d.Init();
        d.SetSmoothCrushing(false);
    }
}

void Fx::process(float& inout0, float& inout1)
{
    float out[2] = { inout0, inout1 };
    int i;

    // Drive
    _drive_on.process();
    if (!_drive_on.is_idle()) {
        for (i = 0; i < 2; i++) out[i] = _drive[i].Process(out[i]) * _drive_comp;
    }
    
    // Reduce
    _reduce_on.process();
    if (!_reduce_on.is_idle()) {
        for (i = 0; i < 2; i++) out[i] = _decimator[i].Process(out[i]) * _reduce_comp;
    }
    
    inout0 = out[0];
    inout1 = out[1];
}

void Fx::engage(const Params p)
{   
    _drive_on.set_on(false);
    _reduce_on.set_on(false);
    auto t = p.types.front();
    switch (t) {
        case Type::drive: {
            _drive_on.set_on(true);
            _drive_comp = dbfs2lin(p.drive_vol_comp_dbfs);
            for (auto& d: _drive) d.SetDrive(p.drive);
            break;
        }
        case Type::reduce: {
            _reduce_on.set_on(true);
            _reduce_comp = dbfs2lin(p.drive_vol_comp_dbfs);
            for (auto& d: _decimator) {
                d.SetBitsToCrush(p.bits);
                d.SetDownsampleFactor(p.downsample);
            }
            break;

        default: break;
        }
    }
    
}

void Fx::disengage()
{
    _drive_on.set_on(false);
    _reduce_on.set_on(false);
}