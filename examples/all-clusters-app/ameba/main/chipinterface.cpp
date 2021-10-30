#include <platform_stdlib.h>

#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "Server.h"

#include <app/clusters/identify-server/identify-server.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CHIPMem.h>

extern "C" {
void * __dso_handle = 0;
}

using namespace ::chip;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;

Identify gIdentify0 = {
    chip::EndpointId{ 0 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

Identify gIdentify1 = {
    chip::EndpointId{ 1 },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "onIdentifyStop"); },
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
};

static DeviceCallbacks EchoCallbacks;

extern "C" void ChipTest(void)
{
    printf("In ChipTest()\r\n");
    CHIP_ERROR err = CHIP_NO_ERROR;

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    err = deviceMgr.Init(&EchoCallbacks);

    if (err != CHIP_NO_ERROR)
    {
        printf("DeviceManagerInit() - ERROR!\r\n");
    }
    else
    {
        printf("DeviceManagerInit() - OK\r\n");
    }

    chip::Server::GetInstance().Init();

    while (true)
        vTaskDelay(pdMS_TO_TICKS(50));
}

bool lowPowerClusterSleep()
{
    return true;
}
