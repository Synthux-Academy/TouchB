#include <array>
#include <stdint.h>
#include "vox.h"

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
        
        using DT = Distortion::Type;

        auto lead = (*c.idxs)[c.lead_ptr];
        switch (lead) {
            case 0: {
                vp.distortion.types[vp.distortion.type_count++] = DT::fold;
                vp.distortion.types[vp.distortion.type_count++] = DT::reduce;
                vp.distortion.drive = 0.53;
                vp.distortion.bits = 4;
                vp.distortion.downsample = 0.3;
                break;
            }
            case 1: {
                vp.distortion.types[vp.distortion.type_count++] = DT::fold;
                vp.distortion.types[vp.distortion.type_count++] = DT::reduce;
                vp.distortion.drive = 0.47;
                vp.distortion.bits = 4;
                vp.distortion.downsample = 0.3;
                break;
            }
            case 5: {
                vp.distortion.types[vp.distortion.type_count++] = DT::fold;
                vp.distortion.types[vp.distortion.type_count++] = DT::reduce;
                vp.distortion.drive = 0.7;
                vp.distortion.bits = 10;
                vp.distortion.downsample = 0.6;
                break;
            }
            case 2: {
                //bypass
                break;
            }
            case 3: {
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::drive;
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::reduce;
                vp.distortion.bits = 10;
                vp.distortion.downsample = 0.3;
                vp.distortion.drive = 0.5;
                break;
            }
            case 4: {
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::drive;
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::reduce;
                vp.distortion.drive = 0.5;
                vp.distortion.bits = 12;
                vp.distortion.downsample = 0.2;
                break;
            }
            case 6: {
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::drive;
                vp.distortion.types[vp.distortion.type_count++] = Distortion::Type::reduce;
                vp.distortion.bits = 10;
                vp.distortion.downsample = 0.4;
                vp.distortion.drive = 0.7;
                break;
            }
        }
        return vp;
    }   
};
};
