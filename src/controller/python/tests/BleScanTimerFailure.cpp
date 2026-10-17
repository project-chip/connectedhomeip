#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayerImplSelect.h>

#include <atomic>
#include <dlfcn.h>

namespace {

std::atomic<bool> sFailNextTimer{ false };
std::atomic<uint32_t> sConnectionErrors{ 0 };

}

extern "C" void SetBleScanTimerFailure(bool enabled)
{
    sFailNextTimer.store(enabled);
}

extern "C" uint32_t StartBleScanConnectionForTest()
{
    sConnectionErrors.store(0);
    auto & manager   = chip::DeviceLayer::Internal::BLEMgrImpl();
    CHIP_ERROR error = manager.ConfigureBle(0, true);
    if (error != CHIP_NO_ERROR)
    {
        return error.AsInteger();
    }
    chip::SetupDiscriminator discriminator;
    discriminator.SetLongValue(3840);
    return chip::DeviceLayer::Internal::BLEMgr()
        .GetBleLayer()
        ->NewBleConnectionByDiscriminator(discriminator, nullptr, nullptr,
                                          [](void *, CHIP_ERROR) { sConnectionErrors.fetch_add(1); })
        .AsInteger();
}

extern "C" uint32_t CancelBleScanConnectionForTest()
{
    return chip::DeviceLayer::Internal::BLEMgr().GetBleLayer()->CancelBleIncompleteConnection().AsInteger();
}

extern "C" uint32_t BleScanConnectionErrorsForTest()
{
    return sConnectionErrors.load();
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
