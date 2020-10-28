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

#include "Publisher.h"

#include <inttypes.h>

#include "lib/protocols/mdns/platform/Mdns.h"
#include "lib/support/logging/CHIPLogging.h"
#include "platform/CHIPDeviceBuildConfig.h"
#include "platform/CHIPDeviceLayer.h"
#include "support/CodeUtils.h"
#include "support/ErrorStr.h"

namespace chip {
namespace Protocols {
namespace Mdns {

CHIP_ERROR Publisher::Init()
{
    return ChipMdnsInit(HandleMdnsInit, HandleMdnsError, this);
}

void Publisher::HandleMdnsInit(void * context, CHIP_ERROR initError)
{
    Publisher * publisher = static_cast<Publisher *>(context);

    if (initError == CHIP_NO_ERROR)
    {
        publisher->mInitialized = true;
    }
    else
    {
        ChipLogError(Discovery, "mDNS initialization failed with %s", chip::ErrorStr(initError));
    }
}

void Publisher::HandleMdnsError(void * context, CHIP_ERROR initError)
{
    ChipLogError(Discovery, "mDNS error: %s", chip::ErrorStr(initError));
}

CHIP_ERROR Publisher::StartPublishDevice(chip::Inet::InterfaceId interface)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    MdnsService service;
    char key[] = "device_id";
    char deviceIdVal[20];
    uint16_t discriminator;
    uint64_t deviceId;
    TextEntry entry;

    VerifyOrExit(mInitialized, error = CHIP_ERROR_INCORRECT_STATE);
    ChipLogProgress(Discovery, "setup mdns service");
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(discriminator));
    sprintf(service.mName, "CHIP-%04d", discriminator);
    strcpy(service.mType, "_chip");
    service.mProtocol = MdnsServiceProtocol::kMdnsProtocolUdp;
    SuccessOrExit(error = chip::DeviceLayer::ConfigurationMgr().GetDeviceId(deviceId));
    entry.mKey = key;
    sprintf(deviceIdVal, "%" PRIu64, deviceId);
    entry.mData            = reinterpret_cast<const uint8_t *>(deviceIdVal);
    entry.mDataSize        = strlen(reinterpret_cast<const char *>(entry.mData));
    service.mTextEntryies  = &entry;
    service.mTextEntrySize = 1;
    service.mPort          = CHIP_PORT;

    error = ChipMdnsPublishService(&service);

exit:
    return error;
}

CHIP_ERROR Publisher::StopPublishDevice()
{
    return ChipMdnsStopPublish();
}

} // namespace Mdns
} // namespace Protocols
} // namespace chip
