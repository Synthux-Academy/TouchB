#pragma once
#include "nocopy.h"

namespace synthux {

class MValue {
public:
  MValue();
  ~MValue() {}

int id() const { return _id; }

float process(const float value, const bool active, int* id = nullptr);

bool is_tracking() const { return _is_tracking; }

float in_value() const { return _in_value; }

float value() const { return _value; }

void set(const float value) {
  _is_tracking = false;
  _is_active = false;
  _value = value;
}

private:
  NOCOPY(MValue)

  bool _set_active(const bool active);

  static constexpr float kTreshold = 0.02;

  int _id;

  float _in_value;
  float _value;

  bool _is_active;
  bool _is_tracking;
};

};
