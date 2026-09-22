#include <array>
#include <stdint.h>
#include "vox.h"
#include "common/common.h"

namespace synthux {
namespace touchb {
    
    struct Combo {
        std::array<uint8_t, 7>* idxs;
        uint8_t lead_ptr;
    };

    enum distortionType: uint8_t {
        drive,
        bitcrusher
    };

    struct Behavior {
        Distortion::Params distortion;
    };

    Behavior behavior4combo(Combo c)
    {
        Behavior vp;
        memset(&vp, 0, sizeof(vp));

        vp.distortion.types.fill(Distortion::Type::None);
        vp.distortion.level = 1.f;
        vp.distortion.fold_gain_kof = 1.f;
        
        using DT = Distortion::Type;

        auto lead = (*c.idxs)[c.lead_ptr];
        switch (lead) {
            case 0: {
                vp.distortion.types[vp.distortion.type_count++] = DT::wah;
                vp.distortion.types[vp.distortion.type_count++] = DT::reduce;
                vp.distortion.bits = 4;
                vp.distortion.downsample = 0.3;
                vp.distortion.wah = 0.2;
                vp.distortion.wah_mix = 1.f;
                break;
            }
            case 1: {
                vp.distortion.types[vp.distortion.type_count++] = DT::wah;
                vp.distortion.types[vp.distortion.type_count++] = DT::reduce;
                vp.distortion.bits = 4;
                vp.distortion.downsample = .3f;
                vp.distortion.wah = .6f;
                vp.distortion.wah_mix = 1.f;
                break;
            }
            case 5: {
                vp.distortion.types[vp.distortion.type_count++] = DT::wah;
                vp.distortion.types[vp.distortion.type_count++] = DT::reduce;
                vp.distortion.bits = 10;
                vp.distortion.downsample = 0.6;
                vp.distortion.wah = 1.f;
                vp.distortion.wah_mix = 1.f;
                break;
            }
            case 2: {
                //bypass
                break;
            }
            case 3: {
                // Fold - light
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::fold;
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::reduce;
                vp.distortion.bits = 10;
                vp.distortion.downsample = 0.3;
                vp.distortion.fold = 0.01;
                vp.distortion.fold_gain_kof = 0.9;
                vp.distortion.fold_mix = 1.f;
                break;
            }
            case 4: {
                // Fold - medium
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::fold;
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::reduce;
                vp.distortion.bits = 12;
                vp.distortion.downsample = 0.2;
                vp.distortion.fold = 0.5;
                vp.distortion.fold_gain_kof = 0.7;
                vp.distortion.fold_mix = 1.f;
                break;
            }
            case 6: {
                // Fold - heavy
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::fold;
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::reduce;
                vp.distortion.bits = 10;
                vp.distortion.downsample = 0.4;
                vp.distortion.fold = 1.f;
                vp.distortion.fold_gain_kof = 0.5;
                vp.distortion.fold_mix = 1.f;
                break;
            }
        }
        
        return vp;
    }   
};
};
