#pragma once

#include <daisy_seed.h>

namespace synthux {

class Switches {
public:
    Switches() {};
    ~Switches() {};

    void init();

    int A();
    int B();

private:
    daisy::Switch3 _switch_7_8;
    daisy::Switch3 _switch_9_10;
};

};
