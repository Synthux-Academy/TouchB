#pragma once

#include <sys/system.h>

namespace synthux {
/// Derived from daisy StopwatchTimer

template<size_t seconds>
class TimeInterval {
  public:
    TimeInterval()   = default;
    ~TimeInterval()  = default;

    inline bool is_passed()
    {
      if (_scheduled) {
        const auto now = daisy::System::GetTick();
        if (daisy::System::GetUsBetweenTicks(now, _last) >= seconds * 1000) {
          _scheduled = false;
          return true;
        }
      }
      return false;
    }

    inline void start() { 
      _scheduled = true;
      _last = daisy::System::GetTick(); 
    }

  private:
    uint32_t _last;
    bool _scheduled;
};

};
