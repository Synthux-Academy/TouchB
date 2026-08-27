#pragma once

namespace synthux::touchb {

class Detector {
public:
    Detector();
    ~Detector() = default;

    void process(const float in);
    bool is_open() const;

private:
    static constexpr auto ln367 = -0.99967234081320612357829304641019f;
    static constexpr auto kof = ln367 / 48.f; //ln(36,7) / (samplerate * 0.001)

    float _attack_ms;
    float _release_ms;
    float _avg;
};
};
