#include <system/SystemLayerImplSelect.h>

#include <atomic>
#include <dlfcn.h>

namespace {

std::atomic<bool> sFailNextTimer{ false };

}

extern "C" void SetBleScanTimerFailure(bool enabled)
{
    sFailNextTimer.store(enabled);
}

namespace chip::System {

CriticalFailure LayerImplSelect::StartTimer(Clock::Timeout delay, TimerCompleteCallback onComplete, void * appState)
{
    if (sFailNextTimer.exchange(false))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    using StartTimerFunction = CriticalFailure (*)(LayerImplSelect *, Clock::Timeout, TimerCompleteCallback, void *);
    static auto startTimer   = reinterpret_cast<StartTimerFunction>(
        dlsym(RTLD_NEXT,
                "_ZN4chip6System15LayerImplSelect10StartTimerENSt6chrono8durationIjSt5ratioILl1ELl1000EEEEPFvPNS0_5LayerEPvES9_"));
    VerifyOrDie(startTimer != nullptr);
    return startTimer(this, delay, onComplete, appState);
}

} // namespace chip::System
