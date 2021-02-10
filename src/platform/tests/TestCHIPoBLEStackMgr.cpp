/* See Project CHIP LICENSE file for licensing information. */

#include "platform/internal/CHIPDeviceLayerInternal.h"
#include <assert.h>
#include <memory>
#include <support/CHIPMem.h>
#include <support/CodeUtils.h>
#include <support/UnitTestRegistration.h>

#include "platform/PlatformManager.h"
#include "platform/internal/BLEManager.h"

void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
        // exit(0);
    }
}

int TestCHIPoBLEStackManager()
{
    chip::Platform::MemoryInit();

    chip::DeviceLayer::PlatformMgrImpl().InitChipStack();

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("CHIP0001");

    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(1, false);

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

    ChipLogProgress(DeviceLayer, "Start Chip Over Ble stack Done");

    chip::DeviceLayer::PlatformMgrImpl().RunEventLoop();
    ChipLogProgress(DeviceLayer, "Start EventLoop");

    return 0;
}

CHIP_REGISTER_TEST_SUITE(TestCHIPoBLEStackManager);
