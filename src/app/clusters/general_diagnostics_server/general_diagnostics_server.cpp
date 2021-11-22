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
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::GeneralDiagnostics::Attributes;
using namespace chip::DeviceLayer;
using chip::DeviceLayer::ConnectivityMgr;
using chip::DeviceLayer::DiagnosticDataProvider;
using chip::DeviceLayer::GetDiagnosticDataProvider;

namespace {

class GeneralDiagosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the GeneralDiagnostics cluster on all endpoints.
    GeneralDiagosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), GeneralDiagnostics::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    template <typename T>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &), AttributeValueEncoder & aEncoder);

    template <typename T>
    CHIP_ERROR ReadListIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &), AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadNetworkInterfaces(AttributeValueEncoder & aEncoder);
};

template <typename T>
CHIP_ERROR GeneralDiagosticsAttrAccess::ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &),
                                                        AttributeValueEncoder & aEncoder)
{
    T data;
    CHIP_ERROR err = (GetDiagnosticDataProvider().*getter)(data);
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

template <typename T>
CHIP_ERROR GeneralDiagosticsAttrAccess::ReadListIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &),
                                                            AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    T faultList;

    if ((GetDiagnosticDataProvider().*getter)(faultList) == CHIP_NO_ERROR)
    {
        err = aEncoder.EncodeList([&faultList](const TagBoundEncoder & encoder) -> CHIP_ERROR {
            for (auto fault : faultList)
            {
                ReturnErrorOnFailure(encoder.Encode(fault));
            }

            return CHIP_NO_ERROR;
        });
    }
    else
    {
        err = aEncoder.Encode(DataModel::List<uint8_t>());
    }

    return err;
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

CHIP_ERROR GeneralDiagosticsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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
    case ActiveHardwareFaults::Id: {
        return ReadListIfSupported(&DiagnosticDataProvider::GetActiveHardwareFaults, aEncoder);
    }
    case ActiveRadioFaults::Id: {
        return ReadListIfSupported(&DiagnosticDataProvider::GetActiveRadioFaults, aEncoder);
    }
    case ActiveNetworkFaults::Id: {
        return ReadListIfSupported(&DiagnosticDataProvider::GetActiveNetworkFaults, aEncoder);
    }
    case RebootCount::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetRebootCount, aEncoder);
    }
    case UpTime::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetUpTime, aEncoder);
    }
    case TotalOperationalHours::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetTotalOperationalHours, aEncoder);
    }
    case BootReasons::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetBootReason, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}

class GeneralDiagnosticDelegate : public DeviceLayer::ConnectivityManagerDelegate, public DeviceLayer::DiagnosticsDelegate
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
                    MatterReportingAttributeChangeCallback(endpointId, GeneralDiagnostics::Id,
                                                           GeneralDiagnostics::Attributes::NetworkInterfaces::Id);
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

    GetDiagnosticDataProvider().SetDelegate(&gDiagnosticDelegate);
    ConnectivityMgr().SetDelegate(&gDiagnosticDelegate);
}
