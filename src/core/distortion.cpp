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
    _fold_gain.init(sample_rate);

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

/* Signal path ........................................................

                dirve_bus
      |-- _drive -- _drive_gain --|
in >--|                           d/f-- _reduce -- _reduce_gain --|
      |-- _fold --- _fold_gain ---|                              wet
      |         fold_bus                                       _bypas --> out
      |                                                          dry
      |-----------------------------------------------------------|

Note: bypass is after the processing chain to feed rms gains constantly.
....................................................................... */

void Distortion::process(float& inout0, float& inout1)
{
    float bus[2] = { inout0, inout1 };
    int i;

    // Fold
    float fold_bus[2] = { inout0, inout1 };
    for (i = 0; i < 2; i++) _fold[i].process(fold_bus[i]);
    _fold_gain.process(fold_bus[0], fold_bus[1]);

    // Drive
    float drive_bus[2] = { inout0, inout1 };
    for (i = 0; i < 2; i++) drive_bus[i] = _drive[i].Process(drive_bus[i]);
    _drive_gain.process(drive_bus[0], drive_bus[1]);
    
    auto drive = _drive_fold_switch.process();
    auto fold = (1.f - drive);

    for (i = 0; i < 2; i++) {
        // Switch fold / drive
        bus[i] = fold_bus[i] * fold + drive_bus[i] * drive;

        // Reduce
        bus[i] = _decimator[i].Process(bus[i]);
    }
    _reduce_gain.process(bus[0], bus[1]);
    
    auto dry = _bypass.process();
    auto wet = infrasonic::unitclamp(1.f - dry) * comp;

    inout0 = inout0 * dry + bus[0] * wet;
    inout1 = inout1 * dry + bus[1] * wet;
}

void Distortion::_validate()
{
    auto fold_thresh = infrasonic::map(_drive_amnt * _flavor, 0.f, 1.f, .5f, .01f);
    auto fold_gain = infrasonic::map(_drive_amnt * _flavor, 0.f, 1.f, 1.f, 5.f);
    auto drive = infrasonic::map(_drive_amnt * _flavor, 0.f, 1.f, .3f, .9f);
    auto downsample = infrasonic::map(_downsample_amnt * (1.f - _flavor), 0.f, .7f, 0.f, .9f);
    auto bits_to_crush = std::round(_bits_reduce_amnt * (1.f - _flavor));
    for (auto i = 0; i < 2; i++) {
        _fold[i].set_gain_mult(fold_gain);
        _fold[i].set_threshold_norm(fold_thresh);
        
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
        _drive_fold_switch.set_on(false);
        for (auto t: p.types) {
            if (t == Distortion::Type::drive) {
                _drive_fold_switch.set_on(true);
                break;
            }
        }
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
