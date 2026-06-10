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
                vp.fx.drive = 0.53;
                vp.fx.drive_vol_comp_dbfs = -18;
                break;
            }
            case 1: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.drive = 0.47;
                vp.fx.drive_vol_comp_dbfs = -16;
                break;
            }
            case 5: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::drive;
                vp.fx.drive = 0.7;
                vp.fx.drive_vol_comp_dbfs = -25;
                break;
            }
            case 2: {
                //CLEAN
                break;
            }
            case 3: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.bits = 10;
                vp.fx.downsample = 0.3;
                vp.fx.reduce_vol_comp_dbfs = 1.5;
                break;
            }
            case 4: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.bits = 12;
                vp.fx.downsample = 0.2;
                vp.fx.reduce_vol_comp_dbfs = -2;
                break;
            }
            case 6: {
                vp.fx.types[vp.fx.type_count++] = Fx::Type::reduce;
                vp.fx.bits = 10;
                vp.fx.downsample = 0.4;
                break;
            }
        }
        return vp;
    }   
};
};
