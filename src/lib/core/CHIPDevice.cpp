/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *      This file implements the CHIP Device Interface that is used by
 *      applications to interact with the CHIP stack
 *
 */

#include <stdlib.h>

#if 1
#include <core/CHIPDevice.h>
#include <support/ErrorStr.h>
#include <lib/support/CodeUtils.h>

namespace chip {

namespace DeviceLayer {
/**
 *
 */
CHIP_ERROR Device::Init(PlatformManager::EventHandlerFunct handler)
{
    CHIP_ERROR err;
    
    // Initialize the LwIP core lock.  This must be done before the ESP
    // tcpip_adapter layer is initialized.
    err = PlatformMgrImpl().InitLwIPCoreLock();
    SuccessOrExit(err);

    // Initialize the CHIP stack.
    err = PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    // Configure the CHIP Connectivity Manager to always enable the AP. The Station interface
    // will be enabled automatically if the required configuration is set.
    ConnectivityMgr().SetWiFiAPMode(ConnectivityManager::kWiFiAPMode_Enabled);

    // Register a function to receive events from the CHIP device layer.  Note that calls to
    // this function will happen on the CHIP event loop thread, not the app_main thread.
    PlatformMgr().AddEventHandler(handler, 0);

    // Start a task to run the CHIP Device event loop.
    err = PlatformMgr().StartEventLoopTask();
    SuccessOrExit(err);

 exit:
    return err;
}
} // namespace DeviceLayer
} // namespace chip
#endif /* #if 0 */
