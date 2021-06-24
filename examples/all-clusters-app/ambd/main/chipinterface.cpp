#include <platform_stdlib.h>

#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Server.h"

#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

extern "C"{ void * __dso_handle = 0 ;}

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

namespace
{
    void SetupPretendDevices(void)
    {
        //TODO
    }

    class AppCallbacks : public AppDelegate
    {
    public:
        void OnReceiveError() override { };
        void OnRendezvousStarted() override {  };
        void OnRendezvousStopped() override {};
        void OnPairingWindowOpened() override { };
        void OnPairingWindowClosed() override { };
    };

} // namespace

static DeviceCallbacks EchoCallbacks;

extern "C" void ChipTest(void)
{
    printf("In ChipTest()\r\n");
    CHIP_ERROR err = CHIP_NO_ERROR;


    CHIPDeviceManager &deviceMgr = CHIPDeviceManager::GetInstance();


    err = deviceMgr.Init(&EchoCallbacks);

    if (err != CHIP_NO_ERROR)
    {
        printf("DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        printf("DeviceManagerInit() - OK\r\n");
    }

    SetupPretendDevices();

    AppCallbacks callbacks;
    InitServer(&callbacks);

    while(true)
        vTaskDelay( pdMS_TO_TICKS(50) );
}
