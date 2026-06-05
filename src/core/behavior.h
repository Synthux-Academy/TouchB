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

    enum FXType: uint8_t {
        drive,
        bitcrusher
    };

    struct VoxParams {
        float drive;
        float volume;
        FXType fx_type;
    };

    VoxParams vox_parms_for_behavior(VoxBehavior vb)
    {
        VoxParams vp;
        
        auto lead = (*vb.pads)[vb.lead_ptr];
        switch (lead) {
            case 0: {
                vp.fx_type = FXType::drive;
                
                break;
            }
            case 1: {
                
                break;
            }
            case 5: {
                
                break;
            }
            case 2: {
                
                break;
            }
            case 3: {
                
                break;
            }
            case 4: {
                
                break;
            }
            case 6: {
                
                break;
            }
        }
        
        return vp;
    }   
};
};
