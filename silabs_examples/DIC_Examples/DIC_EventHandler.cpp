/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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


#include "DIC_EventHandler.h"

namespace chip {

namespace DICManager {

    void AppSpecificConnectivityEventCallback(const ChipDeviceEvent * event, intptr_t arg)
    {
        SILABS_LOG("AppSpecificConnectivityEventCallback: call back for IPV4");
        if ((event->Type == DeviceEventType::kInternetConnectivityChange) &&
            (event->InternetConnectivityChange.IPv4 == kConnectivity_Established))
        {
            SILABS_LOG("Got IPv4 Address! Starting DIC module\n");
            if (DIC_OK != DIC_Init())
                SILABS_LOG("Failed to initialize DIC module\n");
        }
    }

    void Init(){
        PlatformMgr().AddEventHandler(AppSpecificConnectivityEventCallback, reinterpret_cast<intptr_t>(nullptr));
    }
}
}
