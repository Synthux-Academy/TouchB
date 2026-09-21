#pragma once

namespace synthux {

class Folder {
public:
    Folder();
    ~Folder() = default;

    void process(float&);

    void set_gain_mult(const float);
    void set_offset_norm(const float);
    void set_threshold_norm(const float);


private:
    float _gain;
    float _offset;
    float _threshold;
};

};
