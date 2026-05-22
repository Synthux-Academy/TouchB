#pragma once

#include <array>
#include <random>
#include <daisysp.h>

#include "common/xfade.h"
#include "common/smooth.h"
#include "common/sdram_alloc.h"
#include "nocopy.h"
#include "config.h"
#include "vox.h"
#include "buffer.h"

namespace synthux {
namespace touchb {

class Core {
public:
  enum PlayDirection: uint8_t
  {
    Rnd,
    Fwd,
    Rev
  };

  Core();
  ~Core() = default;
  
  void init(const float sample_rate, const float cb_buffer_size);
  void process(const float* const* in, float** out, size_t size);

  void add_behavior(const uint8_t idx);
  void remove_behavior(const uint8_t idx);

  void set_mix(const float);
  void set_start(const float);
  void set_size(const float);
  void set_filter(const float);
  void set_reverb(const float);
  void set_start_mod(const float);
  void set_pitch(const float);
  void set_blur(const float);

  void set_input_level(const float);

  void set_envelope_on(const bool);
  
  void set_play_direction(const PlayDirection);

private:
  NOCOPY(Core)

  void _trigger_vox(const uint8_t idx = 0);
  bool _has_behavior() const { return _behavior_ptr >= 0; };
  void _apply_behavior();
  void _set_start();
  void _set_size();

  static constexpr uint8_t kVoxCount = 2;

  std::array<float, 2> _reverb_in;
  std::array<float, 2> _reverb_out;
  std::array<float, 2> _bus;

  std::array<Vox, kVoxCount> _vox;
  std::bitset<kVoxCount> _is_active;
  std::array<uint8_t, 7> _behavior;
  Buffer _buffer;
  bleeptools::XFade _mix;  
  bleeptools::XFade _reverb_send;
  std::array<daisysp::Svf, 2>  _filter;
  daisysp::ReverbSc* _reverb;

  std::default_random_engine _rand;
    std::normal_distribution<float> _dice;

  float _norm_start;
  float _norm_size;

  float _start_mod;
  float _size_mult;

  float _increment;
  float _target_increment;

  float _in_mult;

  PlayDirection _direction;
  int8_t _behavior_ptr;
  bool _fltr_lp;
  bool _fade_in;
};

};
};
