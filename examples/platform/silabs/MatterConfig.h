/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2022 Silabs
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

class SilabsMatterConfig
{
public:
    static CHIP_ERROR InitMatter(const char * appName);

private:
    static CHIP_ERROR InitOpenThread(void);
    static CHIP_ERROR InitWiFi(void);
    static void ConnectivityEventCallback(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    static void InitOTARequestorHandler(chip::System::Layer * systemLayer, void * appState);
};
