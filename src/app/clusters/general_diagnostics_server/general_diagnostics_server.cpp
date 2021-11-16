/**
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <platform/ConnectivityManager.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using chip::DeviceLayer::ConnectivityMgr;
using chip::DeviceLayer::PlatformManager;
using chip::DeviceLayer::PlatformMgr;

namespace {

class GeneralDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the GeneralDiagnostics cluster on all endpoints.
    GeneralDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), GeneralDiagnostics::Id) {}

    CHIP_ERROR Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    template <typename T>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (PlatformManager::*getter)(T &), AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNetworkInterfaces(AttributeValueEncoder & aEncoder);
};

template <typename T>
CHIP_ERROR GeneralDiagosticsAttrAccess::ReadIfSupported(CHIP_ERROR (PlatformManager::*getter)(T &),
                                                        AttributeValueEncoder & aEncoder)
{
    T data;
    CHIP_ERROR err = (PlatformMgr().*getter)(data);
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        data = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return aEncoder.Encode(data);
}

CHIP_ERROR GeneralDiagosticsAttrAccess::ReadNetworkInterfaces(AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    DeviceLayer::NetworkInterface * netifs;

    if (ConnectivityMgr().GetNetworkInterfaces(&netifs) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&netifs](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            for (DeviceLayer::NetworkInterface * ifp = netifs; ifp != nullptr; ifp = ifp->Next)
            {
                ReturnErrorOnFailure(encoder.Encode(*ifp));
            }

            return CHIP_NO_ERROR;
        });

        ConnectivityMgr().ReleaseNetworkInterfaces(netifs);
    }
    else
    {
        err = aEncoder.Encode(DataModel::List<EndpointId>());
    }

    return err;
}

GeneralDiagosticsAttrAccess gAttrAccess;

CHIP_ERROR GeneralDiagosticsAttrAccess::Read(const ConcreteAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != GeneralDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case NetworkInterfaces::Id: {
        return ReadNetworkInterfaces(aEncoder);
    }
    case RebootCount::Id: {
        return ReadIfSupported(&PlatformManager::GetRebootCount, aEncoder);
    }
    case UpTime::Id: {
        return ReadIfSupported(&PlatformManager::GetUpTime, aEncoder);
    }
    case TotalOperationalHours::Id: {
        return ReadIfSupported(&PlatformManager::GetTotalOperationalHours, aEncoder);
    }
    case BootReasons::Id: {
        return ReadIfSupported(&PlatformManager::GetBootReasons, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

class GeneralDiagnosticDelegate : public DeviceLayer::ConnectivityManagerDelegate, public DeviceLayer::PlatformManagerDelegate
{

    // Gets called when any network interface on the Node is updated.
    void OnNetworkInfoChanged() override
    {
        ChipLogProgress(Zcl, "GeneralDiagnosticDelegate: OnNetworkInfoChanged");

        for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
        {
            if (emberAfEndpointIndexIsEnabled(index))
            {
                EndpointId endpointId = emberAfEndpointFromIndex(index);

                if (emberAfContainsServer(endpointId, GeneralDiagnostics::Id))
                {
                    // If General Diagnostics cluster is implemented on this endpoint
                    MatterReportingListAttributeChangeCallback(endpointId, GeneralDiagnostics::Id,
                                                               GeneralDiagnostics::Attributes::NetworkInterfaces::Id,
                                                               ListOperation::ReplaceAll);
                }
            }
        }
    }

    // Gets called when the device has been rebooted.
    void OnDeviceRebooted() override
    {
        ChipLogProgress(Zcl, "GeneralDiagnosticDelegate: OnDeviceRebooted");

        for (uint16_t index = 0; index < emberAfEndpointCount(); index++)
        {
            if (emberAfEndpointIndexIsEnabled(index))
            {
                EndpointId endpointId = emberAfEndpointFromIndex(index);

                if (emberAfContainsServer(endpointId, GeneralDiagnostics::Id))
                {
                    // If General Diagnostics cluster is implemented on this endpoint
                    MatterReportingAttributeChangeCallback(endpointId, GeneralDiagnostics::Id,
                                                           GeneralDiagnostics::Attributes::RebootCount::Id);
                    MatterReportingAttributeChangeCallback(endpointId, GeneralDiagnostics::Id,
                                                           GeneralDiagnostics::Attributes::BootReasons::Id);
                }
            }
        }
    }
};

GeneralDiagnosticDelegate gDiagnosticDelegate;

} // anonymous namespace

void MatterGeneralDiagnosticsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);

    PlatformMgr().SetDelegate(&gDiagnosticDelegate);
    ConnectivityMgr().SetDelegate(&gDiagnosticDelegate);
}
