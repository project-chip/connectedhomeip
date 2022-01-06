/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#include <FreeRTOS.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>

#include <AppTask.h>

#include "AppConfig.h"
#include "init_p6Platform.h"
#include <app/server/Server.h>

#ifdef HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#include "app/clusters/ota-requestor/BDXDownloader.h"
#include "app/clusters/ota-requestor/OTARequestor.h"
#include "platform/GenericOTARequestorDriver.h"
#include "platform/P6/OTAImageProcessorImpl.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

using chip::BDXDownloader;
using chip::CharSpan;
using chip::FabricIndex;
using chip::GetRequestorInstance;
using chip::NodeId;
using chip::OTADownloader;
using chip::OTAImageProcessorImpl;
using chip::OTAImageProcessorParams;
using chip::OTARequestor;
using chip::System::Layer;

void OnStartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState);

OTARequestor gRequestorCore;
chip::DeviceLayer::GenericOTARequestorDriver gRequestorUser;
BDXDownloader gDownloader;
OTAImageProcessorImpl gImageProcessor;

uint16_t delayQueryTimeInSec    = 0;
// TODO: Shouldn't these come from a cluster command?
NodeId providerNodeId           = 0x0;
FabricIndex providerFabricIndex = 1;

volatile int apperror_cnt;
// ================================================================================
// App Error
//=================================================================================
void appError(int err)
{
    P6_LOG("!!!!!!!!!!!! App Critical Error: %d !!!!!!!!!!!", err);
    portDISABLE_INTERRUPTS();
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

// ================================================================================
// Main Code
// ================================================================================
int main(void)
{
    init_p6Platform();

#ifdef HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    P6_LOG("==================================================\r\n");
    P6_LOG("chip-p6-ota-requestor-example starting\r\n");
    P6_LOG("==================================================\r\n");

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    CHIP_ERROR ret = chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("PersistedStorage::KeyValueStoreMgrImpl().Init() failed");
        appError(ret);
    }

    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }

    ret = chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("P6_OTA_REQUESTER");
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("ConnectivityMgr().SetBLEDeviceName() failed");
        appError(ret);
    }

    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    // Set server instance used for session establishment
    chip::Server * server = &(chip::Server::GetInstance());
    gRequestorCore.SetServerInstance(server);

    // Connect the Requestor and Requestor Driver objects
    gRequestorCore.SetOtaRequestorDriver(&gRequestorUser);
    gRequestorUser.Init(&gRequestorCore, &gImageProcessor);

    OTAImageProcessorParams ipParams;
    ipParams.imageFile = CharSpan("test.txt");
    gImageProcessor.SetOTAImageProcessorParams(ipParams);
    gImageProcessor.SetOTADownloader(&gDownloader);

    // Connect the Downloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&gImageProcessor);

    gRequestorCore.SetBDXDownloader(&gDownloader);

    // Test Mode operation: If a delay is provided, QueryImage after the timer expires
    if (delayQueryTimeInSec > 0)
    {
        // In this mode Provider node ID and fabric idx must be supplied explicitly from program args
        gRequestorCore.TestModeSetProviderParameters(providerNodeId, providerFabricIndex, chip::kRootEndpointId);

        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(delayQueryTimeInSec * 1000),
                                                    OnStartDelayTimerHandler, nullptr);
    }

    P6_LOG("Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }
    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        P6_LOG("GetAppTask().Init() failed");
        appError(ret);
    }
    /* Start the FreeRTOS scheduler */
    vTaskStartScheduler();

    chip::Platform::MemoryShutdown();
    PlatformMgr().StopEventLoopTask();
    PlatformMgr().Shutdown();

    // Should never get here.
    P6_LOG("vTaskStartScheduler() failed");
    appError(ret);
}

void OnStartDelayTimerHandler(chip::System::Layer * systemLayer, void * appState)
{
    static_cast<OTARequestor *>(GetRequestorInstance())->TriggerImmediateQuery();
}
