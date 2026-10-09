/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "JFAInit.h"

#include "JFADatastoreSync.h"
#include "JFAManager.h"

#include <app/server/Server.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>

using namespace chip;
using namespace chip::DeviceLayer;

namespace chip {
namespace NXP {
namespace App {
namespace JFA {

namespace {

// Wraps the platform DeviceInstanceInfoProvider and overrides GetJointFabricMode
// so the JointFabricAdministrator cluster can report the local JF mode. All
// other queries are forwarded to the underlying platform provider.
class JFADeviceInstanceInfoProvider : public DeviceInstanceInfoProvider
{
public:
    void Init(DeviceInstanceInfoProvider * defaultProvider) { mDefaultProvider = defaultProvider; }

    CHIP_ERROR GetVendorName(char * buf, size_t bufSize) override { return mDefaultProvider->GetVendorName(buf, bufSize); }
    CHIP_ERROR GetVendorId(uint16_t & vendorId) override { return mDefaultProvider->GetVendorId(vendorId); }
    CHIP_ERROR GetProductName(char * buf, size_t bufSize) override { return mDefaultProvider->GetProductName(buf, bufSize); }
    CHIP_ERROR GetProductId(uint16_t & productId) override { return mDefaultProvider->GetProductId(productId); }
    CHIP_ERROR GetPartNumber(char * buf, size_t bufSize) override { return mDefaultProvider->GetPartNumber(buf, bufSize); }
    CHIP_ERROR GetProductURL(char * buf, size_t bufSize) override { return mDefaultProvider->GetProductURL(buf, bufSize); }
    CHIP_ERROR GetProductLabel(char * buf, size_t bufSize) override { return mDefaultProvider->GetProductLabel(buf, bufSize); }
    CHIP_ERROR GetSerialNumber(char * buf, size_t bufSize) override { return mDefaultProvider->GetSerialNumber(buf, bufSize); }
    CHIP_ERROR GetManufacturingDate(uint16_t & year, uint8_t & month, uint8_t & day) override
    {
        return mDefaultProvider->GetManufacturingDate(year, month, day);
    }
    CHIP_ERROR GetHardwareVersion(uint16_t & hardwareVersion) override
    {
        return mDefaultProvider->GetHardwareVersion(hardwareVersion);
    }
    CHIP_ERROR GetHardwareVersionString(char * buf, size_t bufSize) override
    {
        return mDefaultProvider->GetHardwareVersionString(buf, bufSize);
    }
    CHIP_ERROR GetRotatingDeviceIdUniqueId(MutableByteSpan & uniqueIdSpan) override
    {
        return mDefaultProvider->GetRotatingDeviceIdUniqueId(uniqueIdSpan);
    }
    CHIP_ERROR GetJointFabricMode(uint8_t & jointFabricMode) override { return JFAMgr().GetJointFabricMode(jointFabricMode); }

private:
    DeviceInstanceInfoProvider * mDefaultProvider = nullptr;
};

JFADeviceInstanceInfoProvider gJFADeviceInstanceInfoProvider;

void JFAEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == DeviceEventType::kCommissioningComplete)
    {
        JFAMgr().HandleCommissioningCompleteEvent();
    }
}

} // namespace

CHIP_ERROR Init()
{
    Server & server = Server::GetInstance();

    ReturnErrorOnFailure(JFAMgr().Init(server));
    ReturnErrorOnFailure(JFADSync().Init(server));

    TEMPORARY_RETURN_IGNORED server.GetJointFabricAdministrator().SetDelegate(&JFAMgr());
    TEMPORARY_RETURN_IGNORED server.GetJointFabricDatastore().SetDelegate(&JFADSync());

    auto * defaultProvider = GetDeviceInstanceInfoProvider();
    if (defaultProvider != &gJFADeviceInstanceInfoProvider)
    {
        gJFADeviceInstanceInfoProvider.Init(defaultProvider);
        SetDeviceInstanceInfoProvider(&gJFADeviceInstanceInfoProvider);
    }

    ReturnErrorOnFailure(PlatformMgrImpl().AddEventHandler(JFAEventHandler, 0));

    ChipLogProgress(JointFabric, "Joint Fabric Administrator initialized");
    return CHIP_NO_ERROR;
}

} // namespace JFA
} // namespace App
} // namespace NXP
} // namespace chip
