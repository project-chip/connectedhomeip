/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "platform/internal/CHIPDeviceLayerInternal.h"
#include <assert.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <memory>

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
    ChipLogProgress(DeviceLayer, "RunEventLoop completed");
    chip::Platform::MemoryShutdown();
    return 0;
}

CHIP_REGISTER_TEST_SUITE(TestCHIPoBLEStackManager);
