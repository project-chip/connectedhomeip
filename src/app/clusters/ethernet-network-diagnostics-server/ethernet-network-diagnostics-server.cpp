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
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <platform/DiagnosticDataProvider.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics::Attributes;
using chip::DeviceLayer::DiagnosticDataProvider;

namespace {

class EthernetDiagnosticsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the EthernetNetworkDiagnostics cluster on all endpoints.
    EthernetDiagnosticsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), EthernetNetworkDiagnostics::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    template <typename T>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &), AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadPHYRate(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadFullDuplex(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadCarrierDetect(AttributeValueEncoder & aEncoder);
};

template <typename T>
CHIP_ERROR EthernetDiagnosticsAttrAccess::ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &),
                                                          AttributeValueEncoder & aEncoder)
{
    T data;
    CHIP_ERROR err = (DeviceLayer::GetDiagnosticDataProvider().*getter)(data);
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

CHIP_ERROR EthernetDiagnosticsAttrAccess::ReadPHYRate(AttributeValueEncoder & aEncoder)
{
    Attributes::PHYRate::TypeInfo::Type pHYRate;
    auto value = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;

    if (DeviceLayer::GetDiagnosticDataProvider().GetEthPHYRate(value) == CHIP_NO_ERROR)
    {
        pHYRate.SetNonNull(value);
        ChipLogProgress(Zcl, "The current nominal, usable speed at the top of the physical layer of the Node: %d",
                        chip::to_underlying(value));
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return aEncoder.Encode(pHYRate);
}

CHIP_ERROR EthernetDiagnosticsAttrAccess::ReadFullDuplex(AttributeValueEncoder & aEncoder)
{
    Attributes::FullDuplex::TypeInfo::Type fullDuplex;
    bool value = false;

    if (DeviceLayer::GetDiagnosticDataProvider().GetEthFullDuplex(value) == CHIP_NO_ERROR)
    {
        fullDuplex.SetNonNull(value);
        ChipLogProgress(Zcl, "The full-duplex operating status of Node: %d", value);
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return aEncoder.Encode(fullDuplex);
}

CHIP_ERROR EthernetDiagnosticsAttrAccess::ReadCarrierDetect(AttributeValueEncoder & aEncoder)
{
    Attributes::CarrierDetect::TypeInfo::Type carrierDetect;
    bool value = false;

    if (DeviceLayer::GetDiagnosticDataProvider().GetEthCarrierDetect(value) == CHIP_NO_ERROR)
    {
        carrierDetect.SetNonNull(value);
        ChipLogProgress(Zcl, "The status of the Carrier Detect control signal present on the ethernet network interface: %d",
                        value);
    }
    else
    {
        ChipLogProgress(Zcl, "The Ethernet interface is not currently configured or operational");
    }

    return aEncoder.Encode(carrierDetect);
}

EthernetDiagnosticsAttrAccess gAttrAccess;

CHIP_ERROR EthernetDiagnosticsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != EthernetNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    switch (aPath.mAttributeId)
    {
    case Attributes::PHYRate::Id: {
        return ReadPHYRate(aEncoder);
    }
    case FullDuplex::Id: {
        return ReadFullDuplex(aEncoder);
    }
    case CarrierDetect::Id: {
        return ReadCarrierDetect(aEncoder);
    }
    case TimeSinceReset::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetEthTimeSinceReset, aEncoder);
    }
    case PacketRxCount::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetEthPacketRxCount, aEncoder);
    }
    case PacketTxCount::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetEthPacketTxCount, aEncoder);
    }
    case TxErrCount::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetEthTxErrCount, aEncoder);
    }
    case CollisionCount::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetEthCollisionCount, aEncoder);
    }
    case OverrunCount::Id: {
        return ReadIfSupported(&DiagnosticDataProvider::GetEthOverrunCount, aEncoder);
    }
    default: {
        break;
    }
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

bool emberAfEthernetNetworkDiagnosticsClusterResetCountsCallback(app::CommandHandler * commandObj,
                                                                 const app::ConcreteCommandPath & commandPath,
                                                                 const Commands::ResetCounts::DecodableType & commandData)
{
    DeviceLayer::GetDiagnosticDataProvider().ResetEthNetworkDiagnosticsCounts();
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::Success);

    return true;
}

void MatterEthernetNetworkDiagnosticsPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gAttrAccess);
}
