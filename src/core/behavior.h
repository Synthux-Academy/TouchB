#include <array>
#include <stdint.h>
#include "vox.h"

namespace synthux {
namespace touchb {
    
    struct Combo {
        std::array<uint8_t, 7>* idxs;
        uint8_t lead_ptr;
    };

    enum FXType: uint8_t {
        drive,
        bitcrusher
    };

    struct Behavior {
        Fx::Params fx;
    };

    Behavior behavior4combo(Combo c)
    {
        Behavior vp;
        memset(&vp, 0, sizeof(vp));
        vp.fx.types.fill(Fx::Type::None);
        
        auto lead = (*c.idxs)[c.lead_ptr];
        switch (lead) {
            case 0: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.drive = 0.53;
                vp.fx.bits = 4;
                vp.fx.downsample = 0.3;
                break;
            }
            case 1: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.drive = 0.47;
                vp.fx.bits = 4;
                vp.fx.downsample = 0.3;
                break;
            }
            case 5: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.drive = 0.7;
                vp.fx.bits = 10;
                vp.fx.downsample = 0.6;
                break;
            }
            case 2: {
                //bypass
                break;
            }
            case 3: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.bits = 10;
                vp.fx.downsample = 0.3;
                vp.fx.drive = 0.5;
                break;
            }
            case 4: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.drive = 0.5;
                vp.fx.bits = 12;
                vp.fx.downsample = 0.2;
                break;
            }
            case 6: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.bits = 10;
                vp.fx.downsample = 0.4;
                vp.fx.drive = 0.7;
                break;
            }
        }
        return vp;
    }   
};
};
