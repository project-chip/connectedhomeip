/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <AppConfig.h>
#include <MiniApp.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/window-covering-server/window-covering-server.h>
#include <app/server/Server.h>
#include <app/util/af.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CHIPDeviceLayer.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

namespace example {

CHIP_ERROR MiniApp::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    EFR32_LOG("Init CHIP Stack");
    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();
    chip::DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().InitChipStack() failed");
        appError(ret);
    }
    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("EFR32_MINI");
    
#if CHIP_ENABLE_OPENTHREAD
    EFR32_LOG("Initializing OpenThread stack");
    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().InitThreadStack() failed");
        appError(ret);
    }

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ConnectivityMgr().SetThreadDeviceType() failed");
        appError(ret);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    EFR32_LOG("Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }

#if CHIP_ENABLE_OPENTHREAD
    EFR32_LOG("Starting OpenThread task");

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        EFR32_LOG("ThreadStackMgr().StartThreadTask() failed");
        appError(ret);
    }
#endif // CHIP_ENABLE_OPENTHREAD

    mTask = CreateTask(APP_TASK_NAME, *this);
    VerifyOrReturnError(mTask, CHIP_ERROR_NO_MEMORY);

    mQueue = CreateQueue(*this);
    VerifyOrReturnError(mTask, CHIP_ERROR_NO_MEMORY);

    err = mQueue->Init();
    VerifyOrReturnError(CHIP_NO_ERROR == err, err);

    // Init ZCL Data Model
    chip::Server::GetInstance().Init();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    ConfigurationMgr().LogDeviceConfig();

    return CHIP_NO_ERROR;
}

CHIP_ERROR MiniApp::Start()
{
    return mTask->Start();
}

void MiniApp::Finish()
{
    mQueue->Finish();
    delete (mQueue);

    mTask->Finish();
    delete (mTask);

    chip::Platform::MemoryShutdown();

    // Should never get here.
    EFR32_LOG("vTaskStartScheduler() failed");
}

CHIP_ERROR MiniApp::PostEvent(const Event & event)
{
    VerifyOrReturnError(mQueue, CHIP_ERROR_INTERNAL);
    return mQueue->Post(event);
}

CHIP_ERROR MiniApp::Main()
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR MiniApp::OnTaskRun(const Task & task)
{
    EFR32_LOG("---- MiniApp::OnTaskRun()");
    CHIP_ERROR err = CHIP_NO_ERROR;
    while (CHIP_NO_ERROR == err)
    {
        mQueue->DispatchPending();
        err = Main();
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR MiniApp::OnEvent(const Event & event)
{
    // EFR32_LOG("---- MiniApp::OnEvent(%u)", event.mId);
    switch (event.mId)
    {
    case Event::Id::Button0Pressed:
        EFR32_LOG("---- Button0: PRESSED");
        break;
    case Event::Id::Button0Released:
        EFR32_LOG("---- Button0: Released");
        ConfigurationMgr().InitiateFactoryReset();
        break;
    case Event::Id::Button1Pressed:
        EFR32_LOG("---- Button1: PRESSED");
        break;
    case Event::Id::Button1Released:
        EFR32_LOG("---- Button1: Released");
        break;
    default:
        EFR32_LOG("---- Event %u", event.mId);
    }
    return CHIP_NO_ERROR;
}

} // namespace example
