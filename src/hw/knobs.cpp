#include "knobs.h"

using namespace synthux;
using namespace daisy;
using namespace seed;

void Knobs::init(DaisySeed& hw) {
    AdcChannelConfig cfg[Count];
    Pin pins[Count] = { A0, A1, A2, A3, A4, A5, A6, A7 };
    for (auto i = 0; i < Count; i++) {
        cfg[i].InitSingle(pins[i]);    
    }
	hw.adc.Init(cfg, Count);
    for (auto i = 0; i < Count; i++)  {
        _knobs[i].Init(hw.adc.GetPtr(i), hw.AudioCallbackRate());
    }
};

void Knobs::process()
{
    for (auto& k: _knobs) k.Process();
}
