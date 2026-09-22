#include "distortion.h"
#include "common/common.h"

using namespace synthux::touchb;
using namespace daisysp;
using namespace infrasonic;

Distortion::Distortion():
_fold_amnt          { .5f },
_downsample_amnt    { .5f },
_bits_reduce_amnt   { 4 },
_level              { 1.f }
{}

void Distortion::init(const float sample_rate)
{
    //On/Off
    _bypass.init(sample_rate);

    _fold_level.init(sample_rate);

    for (auto& w: _wah) {
        w.Init(sample_rate);
        w.SetDryWet(100.f);
        w.SetLevel(1.f);
    }

    // Reduce
    for (auto& d: _decimator) {
        d.Init();
        d.SetSmoothCrushing(false);
    }

    _wah_amnt_smooth.init(sample_rate, .003f);
    _fold_amnt_smooth.init(sample_rate, .003f);
}

void Distortion::set_flavor_norm(const float norm)
{
    _flavor = norm;
    _validate();
}

/* Signal path ........................................................

                fold_bus
      |-- _fold --- _fold_gain ---|
in >--|                           w/f-- _reduce -- _reduce_gain --|
      |-- _wah ---- _wah_mix -----|                              wet
      |         wah_bus                                        _bypas --> out
      |                                                          dry
      |-----------------------------------------------------------|

Note: bypass is after the processing chain to feed rms gains constantly.
....................................................................... */

void Distortion::process(float& inout0, float& inout1)
{
    float bus[2] = { inout0, inout1 };
    int i;

    // Smooth wah/fold amounts at audio rate
    auto wah_amnt = _wah_amnt_smooth.process(_wah_amnt);
    auto fold_amnt = _fold_amnt_smooth.process(_fold_amnt);
    auto wah = infrasonic::map(wah_amnt * _flavor, 0.f, 1.f, .3f, 1.f);
    // Lower threshold == more folding == heavier distortion
    auto threshold = infrasonic::map(fold_amnt * _flavor, 0.f, 1.f, .8f, .5f);
    for (i = 0; i < 2; i++) {
        _wah[i].SetWah(wah);
        _fold[i].set_threshold_norm(threshold);
        // Pre-gain compensates the shrinking threshold so folding density
        // increases with the amount, rather than just clipping harder.
        _fold[i].set_gain_mult(_fold_gain_kof / threshold);
    }

    // Wah
    float wah_bus[2] = { inout0, inout1 };
    for (i = 0; i < 2; i++) {
        wah_bus[i] = _wah[i].Process(wah_bus[i]) * _wah_mix;
    }

    // Fold
    float fold_bus[2] = { inout0, inout1 };
    _fold_level.pre_gain(fold_bus[0], fold_bus[1]);
    for (i = 0; i < 2; i++) {
        _fold[i].process(fold_bus[i]);
        // Post-gain scales back down by the same threshold so the folded
        // output keeps roughly the same volume across light/medium/heavy.
        fold_bus[i] *= threshold * _fold_mix;
    }
    _fold_level.post_gain(fold_bus[0], fold_bus[1]);

    auto fold = _wah_fold_switch.process();
    auto wah_amt = (1.f - fold);

    for (i = 0; i < 2; i++) {
        // Switch wah / fold
        bus[i] = wah_bus[i] * wah_amt + fold_bus[i] * fold;

        // Reduce
        bus[i] = _decimator[i].Process(bus[i]);
        if (_wah_fold_switch.is_on()) {
            bus[i] *= 1.f / (_fold_mix + (1 - _fold_mix) * _flavor);
        }
    }
    
    auto dry = _bypass.process();
    auto wet = infrasonic::unitclamp(1.f - dry) * _level;

    inout0 = inout0 * dry + bus[0] * wet;
    inout1 = inout1 * dry + bus[1] * wet;
}

void Distortion::_validate()
{
    auto downsample = infrasonic::map(_downsample_amnt * (1.f - _flavor), 0.f, .7f, 0.f, .9f);
    auto bits_to_crush = std::round(_bits_reduce_amnt * (1.f - _flavor));
    for (auto i = 0; i < 2; i++) {
        _decimator[i].SetBitsToCrush(bits_to_crush);
        _decimator[i].SetDownsampleFactor(downsample);
    }
}

void Distortion::engage(const Params p)
{   
    if (p.type_count > 0) {
        _fold_amnt = p.fold;
        _fold_gain_kof = p.fold_gain_kof;
        _fold_mix = p.fold_mix;

        _wah_amnt = p.wah;
        _wah_mix = p.wah_mix;

        _downsample_amnt = p.downsample;
        _bits_reduce_amnt = p.bits;

        _level = p.level;

        _bypass.set_on(false);
        _wah_fold_switch.set_on(false);
        for (auto t: p.types) {
            if (t == Distortion::Type::fold) {
                _wah_fold_switch.set_on(true);
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
