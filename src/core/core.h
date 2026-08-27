#pragma once

#include <array>
#include <random>
#include <daisysp.h>

#include "synthux.h"
#include "bleeptools.h"
#include "sdram_alloc.h"
#include "nocopy.h"
#include "config.h"
#include "vox.h"
#include "buffer.h"
#include "distortion.h"
#include "detector.h"

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

  void set_start(const float);
  void set_size(const float);
  void set_pitch(const float);

  void set_distortion_flavor(const float);
  void set_filter(const float);

  void set_reverb_send(const float);
  void set_reverb_fb(const float);
  
  void set_blur(const float);
  void set_flutter(const float);
  
  void set_mix(const float);

  void set_output_level(const float);

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
  static constexpr uint8_t kNoVox = 0xff;

  std::array<float, 2> _reverb_in;
  std::array<float, 2> _reverb_out;
  
  std::array<float, 2> _in_bus;
  std::array<float, 2> _loop_bus;
  std::array<float, 2> _mix_bus;

  std::array<Vox, kVoxCount> _vox;
  std::bitset<kVoxCount> _is_active;
  std::array<uint8_t, 7> _behavior;
  Buffer _buffer;

  Detector _in_detector;
  SoftSwitch _master_switch;

  SoftSwitch _dist_feed_switch;
  SoftSwitch _in_loop_switch;
  XFade _in_loop_mix;
  XFade _dist_feed_mix;
  XFade _dry_wet_mix;
  XFade _reverb_send;
  
  
  OnePoleSmoother _tape_smooth;
  bleeptools::RandomImpulse _rnd_imp;
  std::array<daisysp::Svf, 2>  _tape_filter;
  std::array<daisysp::Svf, 2>  _in_filter;
  std::array<daisysp::Svf, 2>  _loop_filter;
  SoftSwitch _filter_switch;
  OnePoleSmoother _filter_smooth;

  Distortion _distortion;

  daisysp::ReverbSc* _reverb;
  std::array<daisysp::Limiter, 2>  _limiter;

  std::default_random_engine _rand;
  std::normal_distribution<float> _dice;

  float _norm_start;
  float _norm_size;

  float _increment;
  float _target_increment;

  float _out_mult;

  float _flutter;

  float _fltr_freq;

  float _rev_fb;

  PlayDirection _direction;
  int8_t _behavior_ptr;
  bool _env_on;
  bool _rec_cued;
};

};
};
