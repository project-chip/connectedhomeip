/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "JFAManager.h"
#include "rpc/RpcServer.h"
#include <AppMain.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>

#include <string>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{}

void EventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;

    if (event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete)
    {
        JFAMgr().HandleCommissioningCompleteEvent();
    }
}

void ApplicationInit()
{
    JFAMgr().Init(Server::GetInstance());
    DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);
}

void ApplicationShutdown() {}

#ifdef __NuttX__
// NuttX requires the main function to be defined with C-linkage. However, marking
// the main as extern "C" is not strictly conformant with the C++ standard. Since
// clang >= 20 such code triggers -Wmain warning.
extern "C" {
#endif

int main(int argc, char * argv[])
{
    LinuxDeviceOptions::GetInstance().rpcServerPort = RPC_SERVER_PORT;

    if (ChipLinuxAppInit(argc, argv) != 0)
    {
        return -1;
    }

    ChipLinuxAppMainLoop();

    return 0;
}

#ifdef __NuttX__
}
#endif
