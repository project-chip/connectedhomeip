/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "DispatchQueueNames.h"

#include <dispatch/dispatch.h>

namespace {
constexpr const char * kMainQueue              = "com.apple.main-thread";
constexpr const char * kChipQueue              = "org.csa-iot.matter.workqueue";
constexpr const char * kBleQueue               = "org.csa-iot.matter.framework.ble.workqueue";
constexpr const char * kXPCQueue               = "org.csa-iot.matter.framework.xpc.workqueue";
constexpr const char * kDeviceQueue            = "org.csa-iot.matter.framework.device.workqueue";
constexpr const char * kOTAProviderQueue       = "org.csa-iot.matter.framework.otaprovider.workqueue";
constexpr const char * kDeviceAttestationQueue = "org.csa-iot.matter.framework.device_attestation.workqueue";

constexpr const char * kMainQueueShort              = "main";
constexpr const char * kChipQueueShort              = "chip";
constexpr const char * kBleQueueShort               = "ble";
constexpr const char * kXPCQueueShort               = "xpc";
constexpr const char * kDeviceQueueShort            = "device";
constexpr const char * kOTAProviderQueueShort       = "ota-provider";
constexpr const char * kDeviceAttestationQueueShort = "device-attestation";
} // namespace

namespace chip {
namespace darwin {
namespace queues {

const char * CurrentLabel()
{
    const char * label = dispatch_queue_get_label(DISPATCH_CURRENT_QUEUE_LABEL);

    if (strcmp(label, kMainQueue) == 0)
    {
        label = kMainQueueShort;
    }
    else if (strcmp(label, kChipQueue) == 0)
    {
        label = kChipQueueShort;
    }
    else if (strcmp(label, kBleQueue) == 0)
    {
        label = kBleQueueShort;
    }
    else if (strcmp(label, kXPCQueue) == 0)
    {
        label = kXPCQueueShort;
    }
    else if (strcmp(label, kDeviceQueue) == 0)
    {
        label = kDeviceQueueShort;
    }
    else if (strcmp(label, kOTAProviderQueue) == 0)
    {
        label = kOTAProviderQueueShort;
    }
    else if (strcmp(label, kDeviceAttestationQueue) == 0)
    {
        label = kDeviceAttestationQueueShort;
    }

    return label;
}

} // namespace queues
} // namespace darwin
} // namespace chip
