#pragma once

#include "nocopy.h"

namespace synthux {
namespace touchb {

class Application {
  public:
    Application()  = default;
    ~Application() = default;

    void init();
    void loop();

  private:
    NOCOPY(Application)
};
};
};
