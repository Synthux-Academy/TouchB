#include "distortion.h"
#include "common/common.h"

using namespace synthux::touchb;
using namespace daisysp;
using namespace infrasonic;

const auto comp = dbfs2lin(-10);

Distortion::Distortion():
_drive_amnt         { .5f },
_downsample_amnt    { .5f },
_bits_reduce_amnt   { 4 }
{}

void Distortion::init(const float sample_rate)
{
    //On/Off
    _bypass.init(sample_rate);

    // Drive
    for (auto& d: _drive) {
        d.Init();
    }
    _drive_gain.init(sample_rate);
    
    // Reduce
    for (auto& d: _decimator) {
        d.Init();
        d.SetSmoothCrushing(false);
    }
    _reduce_gain.init(sample_rate);
}

void Distortion::set_flavor_norm(const float norm)
{
    _flavor = norm;
    _validate();
}

void Distortion::process(float& inout0, float& inout1)
{
    float bus[2] = { inout0, inout1 };
    int i;
    // Drive
    for (i = 0; i < 2; i++) bus[i] = _drive[i].Process(bus[i]);
    _drive_gain.process(bus[0], bus[1]);
    
    // Reduce
    for (i = 0; i < 2; i++) bus[i] = _decimator[i].Process(bus[i]);
    _reduce_gain.process(bus[0], bus[1]);
    
    auto dry = _bypass.process();
    auto wet = infrasonic::unitclamp(1.f - dry) * comp;

    inout0 = inout0 * dry + bus[0] * wet;
    inout1 = inout1 * dry + bus[1] * wet;
}

void Distortion::_validate()
{
    auto drive = infrasonic::map(_drive_amnt * _flavor, 0.f, 1.f, .3f, .9f);
    auto downsample = infrasonic::map(_downsample_amnt * (1.f - _flavor), 0.f, .7f, 0.f, .9f);
    auto bits_to_crush = std::round(_bits_reduce_amnt * (1.f - _flavor));
    for (auto i = 0; i < 2; i++) {
        _drive[i].SetDrive(drive);
        _decimator[i].SetBitsToCrush(bits_to_crush);
        _decimator[i].SetDownsampleFactor(downsample);
    }   
}

void Distortion::engage(const Params p)
{   
    if (p.type_count > 0) {
        _drive_amnt = p.drive;
        _downsample_amnt = p.downsample;
        _bits_reduce_amnt = p.bits;
        _bypass.set_on(false);
    }
    else {
        _bypass.set_on(true);
    }
    _validate();
    
}

void Distortion::disengage()
{
    _bypass.set_on(true);
}
