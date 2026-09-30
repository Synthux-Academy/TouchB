#pragma once

#include "knobs.h"
#include "pads.h"
#include "switches.h"
#include "nocopy.h"

namespace synthux {

static constexpr daisy::Pin kMidiUartRxPin = daisy::seed::D14;
static constexpr daisy::Pin kMidiUartTxPin = daisy::seed::D13;

class Touch {
public:
    Touch(): _state { 0 } {}
    ~Touch() {}

    void init() {
      _seed.Init();
      _pads.init(_seed);
      _knobs.init(_seed);
      _switches.init();

      // ADC
      _seed.adc.Start();

      // MIDI
      #if USB_MIDI
      daisy::MidiUsbHandler::Config midi_cfg;
      _usb_midi.Init(midi_cfg);
      #endif
    }

    void process() {
      _pads.process();
    }

    void set_led(const bool on) {
      _seed.SetLed(on);
    }

    Pads& pads() { return _pads; }
    Knobs& knobs() { return _knobs; }
    Switches& switches() { return _switches; }
    daisy::DaisySeed& seed() { return _seed; }
    daisy::MidiUsbHandler& usb_midi() { return _usb_midi; }

private:
    NOCOPY(Touch)

    daisy::DaisySeed _seed;

    uint16_t _state;
    daisy::Mpr121I2C _mpr;
    Knobs _knobs;
    Pads _pads;
    Switches _switches;
    daisy::MidiUsbHandler _usb_midi;
};

};
