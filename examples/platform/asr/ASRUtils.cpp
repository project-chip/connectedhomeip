#include "ASRUtils.h"
#include "AppConfig.h"
#include <lega_rtos_api.h>

using namespace ::chip;
using namespace ::chip::Credentials;
// using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

extern "C" void asr_matter_reset(int type);
extern "C" void matter_ble_start_adv(void);
extern "C" void ShutdownChip();

void asr_matter_reset(int type)
{
    if (type == 0) {
        ConnectivityMgr().ClearWiFiStationProvision();
        chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
        chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    } else if (type == 1) {
        chip::Server::GetInstance().ScheduleFactoryReset();
    } else if (type == 2) {
        chip::Server::GetInstance().GetFabricTable().DeleteAllFabrics();
        auto & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        commissionMgr.OpenBasicCommissioningWindow(commissionMgr.MaxCommissioningTimeout(),
                                                    CommissioningWindowAdvertisement::kDnssdOnly);
    }
}

void ShutdownChip()
{
    Server::GetInstance().Shutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();
}

volatile int apperror_cnt;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    ASR_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    lega_rtos_declare_critical();
    lega_rtos_enter_critical();
    while (1)
        ;
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}

// ================================================================================
// FreeRTOS Callbacks
// ================================================================================
extern "C" void vApplicationIdleHook(void)
{
    // FreeRTOS Idle callback
}


extern "C" void __attribute__((weak)) asr_matter_onoff(int cmd)
{
}

extern "C" void __attribute__((weak)) asr_matter_sensors(bool enable, int temp, int humi, int pressure)
{
}

extern "C" void __attribute__((weak)) asr_matter_ota(uint32_t timeout)
{
}