#include <daisy_seed.h>
#include "app.h"

using namespace synthux::touchb;
static Application app;

extern "C" int main(void)
{
    app.init();
    app.loop();
}
