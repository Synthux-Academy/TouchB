#include "app.h"

#include <functional>

#include "ui/core.ui.h"
#include "expose.h"

#if DEBUG

// #define METER
#ifdef METER
#include "meter.h"
#endif

#endif

using namespace daisy;
using namespace synthux;
using namespace synthux::touchb;
using namespace infrasonic;

namespace synthux {
namespace touchb {

class AppImpl {
  public:
    AppImpl():
    _ui     { CoreUI(_touch, _core) }
    {}

    ~AppImpl() = default;

    void init();
    void loop();
    CoreUI& ui() { return _ui; }

    void ProcessAudio(AudioHandle::InputBuffer  in,
                      AudioHandle::OutputBuffer out,
                      size_t                    size);

  private:
    NOCOPY(AppImpl)

    #if DEBUG
    StopwatchTimer _log_timer;
    void logDebugInfo();
    #endif

    Core        _core;
    CoreUI      _ui;
    Touch       _touch;
};
};
};

static AppImpl impl;

void T5Callback(void* data) 
{
    
};

//According to GetPClk2Freq docs, timers run at the frequency twice faster
//as their peripheral frequency. So call_freq_hz should be twise smaller 
//then synclock period.
TimerHandle tim5_handle;
void StartT5Callback(TimerHandle::PeriodElapsedCallback cb, uint32_t call_freq_hz) {
    TimerHandle::Config timcfg;
    timcfg.periph = TimerHandle::Config::Peripheral::TIM_5;
    timcfg.dir = TimerHandle::Config::CounterDir::UP;
    timcfg.period = System::GetPClk2Freq() / call_freq_hz;
    timcfg.enable_irq = true;
    tim5_handle.Init(timcfg);
    tim5_handle.SetCallback(cb);  
    tim5_handle.Start();
};

static void AudioCallback(AudioHandle::InputBuffer  in,
                          AudioHandle::OutputBuffer out,
                          size_t                    size)
{
    impl.ProcessAudio(in, out, size);
};

void AppImpl::init() 
{
    _touch.init();

    auto seed = _touch.seed();
    seed.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    seed.SetAudioBlockSize(96);

    _core.init(seed.AudioSampleRate(), seed.AudioCallbackRate());
    _ui.init();

    seed.StartAudio(AudioCallback);

    // StartT5Callback(T5Callback, 250);

    #if DEBUG
    Log::StartLog(false);
    _log_timer.Init();
    #endif

    #ifdef METER
    Meter::cpu().load.Init(seed.AudioSampleRate(), seed.AudioBlockSize());
    #endif
}

void AppImpl::loop()
{
    while(true) {
        _ui.process();
        
        #if DEBUG
        if(_log_timer.HasPassedMs(100))
        {
            logDebugInfo();
            _log_timer.Restart();

            #ifdef METER
            auto& loadMeter = Meter::cpu().load;
            const float avgLoad = loadMeter.GetAvgCpuLoad();
            const float maxLoad = loadMeter.GetMaxCpuLoad();
            const float minLoad = loadMeter.GetMinCpuLoad();
            Log::PrintLine("Processing Load %:");
            Log::PrintLine("Max: " FLT_FMT3, FLT_VAR3(maxLoad * 100.0f));
            Log::PrintLine("Avg: " FLT_FMT3, FLT_VAR3(avgLoad * 100.0f));
            Log::PrintLine("Min: " FLT_FMT3, FLT_VAR3(minLoad * 100.0f));
            #endif
        }
        #endif
    }
}

void AppImpl::ProcessAudio(AudioHandle::InputBuffer  in,
                           AudioHandle::OutputBuffer out,
                           size_t                    size)
{
    #ifdef METER
    Meter::cpu().load.OnBlockStart();
    #endif
    
    _touch.knobs().process();
    _ui.tick();
    _core.process(in, out, size);

    #ifdef METER
    Meter::cpu().load.OnBlockEnd();
    #endif
}

#if DEBUG
void AppImpl::logDebugInfo()
{
    Expose::values().print();
}
#endif

void Application::init() 
{
    impl.init();
}

void Application::loop() 
{
    impl.loop();
}
