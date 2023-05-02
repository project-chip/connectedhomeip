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
#ifndef __DIC_EventHandler_H
#define __DIC_EventHandler_H

#include "dic.h"
#ifdef SIWX_917
#include "siwx917_utils.h"
#else
#include "efr32_utils.h"
#endif
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {

namespace DICManager {
    void Init();
    void AppSpecificConnectivityEventCallback(const ChipDeviceEvent * event, intptr_t arg);
}

}
#endif
