#include <array>
#include <stdint.h>
#include "vox.h"

namespace synthux {
namespace touchb {
    
    struct VoxBehavior {
        std::array<uint8_t, 7>* pads;
        uint8_t lead_ptr;
        uint32_t buf_size;
    };

    struct VoxParams {
        float norm_start;
        float size_mult;
        Vox::SpeedMode speed_mode;
    };

    VoxParams vox_parms_for_behavior(VoxBehavior vb)
    {
        VoxParams vp;

        using SM = Vox::SpeedMode;

        auto lead = (*vb.pads)[vb.lead_ptr];
        switch (lead) {
            case 0: {
                vp.speed_mode = SM::Tape;
                vp.norm_start = .2f;
                vp.size_mult = .33f;
                break;
            }
            case 1: {
                vp.speed_mode = SM::Tape;
                vp.norm_start = .5f;
                vp.size_mult = .5f;
                break;
            }
            case 5: {
                vp.speed_mode = SM::Tape;
                vp.norm_start = .7f;
                vp.size_mult = .25f;
                break;
            }
            case 2: {
                vp.speed_mode = SM::Digital;
                vp.norm_start = .5f;
                vp.size_mult = 1.f;
                break;
            }
            case 3: {
                vp.speed_mode = SM::Digital;
                vp.norm_start = .3f;
                vp.size_mult = 1.1f;
                break;
            }
            case 4: {
                vp.speed_mode = SM::Digital;
                vp.norm_start = .6f;
                vp.size_mult = 1.5f;
                break;
            }
            case 6: {
                vp.speed_mode = SM::Digital;
                vp.norm_start = .8f;
                vp.size_mult = 1.3f;
                break;
            }
        }
        
        return vp;
    }   
};
};
