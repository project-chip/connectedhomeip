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
#include <app/CommandHandlerInterface.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <platform/DiagnosticDataProvider.h>
#include <zap-generated/gen_config.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics::Attributes;
using chip::DeviceLayer::DiagnosticDataProvider;

namespace {

class EthernetDiagosticsGlobalInstance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    // Register for the EthernetNetworkDiagnostics cluster on all endpoints.
    EthernetDiagosticsGlobalInstance(DiagnosticDataProvider & diagnosticProvider) :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), EthernetNetworkDiagnostics::Id),
        CommandHandlerInterface(Optional<EndpointId>::Missing(), EthernetNetworkDiagnostics::Id),
        mDiagnosticProvider(diagnosticProvider)
    {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    template <typename T>
    CHIP_ERROR ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &), AttributeValueEncoder & aEncoder);

    CHIP_ERROR ReadPHYRate(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadFullDuplex(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadCarrierDetect(AttributeValueEncoder & aEncoder);

    void InvokeCommand(HandlerContext & ctx) override;

#ifdef ETHERNET_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
    void HandleResetCounts(HandlerContext & ctx, const Commands::ResetCounts::DecodableType & commandData);
#endif

    DiagnosticDataProvider & mDiagnosticProvider;
};

template <typename T>
CHIP_ERROR EthernetDiagosticsGlobalInstance::ReadIfSupported(CHIP_ERROR (DiagnosticDataProvider::*getter)(T &),
                                                             AttributeValueEncoder & aEncoder)
{
    T data;
    CHIP_ERROR err = (mDiagnosticProvider.*getter)(data);
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

CHIP_ERROR EthernetDiagosticsGlobalInstance::ReadPHYRate(AttributeValueEncoder & aEncoder)
{
    Attributes::PHYRate::TypeInfo::Type pHYRate;
    auto value = app::Clusters::EthernetNetworkDiagnostics::PHYRateEnum::kRate10M;

    if (mDiagnosticProvider.GetEthPHYRate(value) == CHIP_NO_ERROR)
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

CHIP_ERROR EthernetDiagosticsGlobalInstance::ReadFullDuplex(AttributeValueEncoder & aEncoder)
{
    Attributes::FullDuplex::TypeInfo::Type fullDuplex;
    bool value = false;

    if (mDiagnosticProvider.GetEthFullDuplex(value) == CHIP_NO_ERROR)
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

CHIP_ERROR EthernetDiagosticsGlobalInstance::ReadCarrierDetect(AttributeValueEncoder & aEncoder)
{
    Attributes::CarrierDetect::TypeInfo::Type carrierDetect;
    bool value = false;

    if (mDiagnosticProvider.GetEthCarrierDetect(value) == CHIP_NO_ERROR)
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

CHIP_ERROR EthernetDiagosticsGlobalInstance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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

void EthernetDiagosticsGlobalInstance::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::ResetCounts::Id:
#ifdef ETHERNET_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
        CommandHandlerInterface::HandleCommand<Commands::ResetCounts::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResetCounts(ctx, commandData); });
        break;
#endif
    default:
        break; // Make the switch statement syntactically correct if ETHERNET_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD is not
               // defined.
    }
}

#ifdef ETHERNET_NETWORK_DIAGNOSTICS_ENABLE_RESET_COUNTS_CMD
void EthernetDiagosticsGlobalInstance::HandleResetCounts(HandlerContext & ctx,
                                                         const Commands::ResetCounts::DecodableType & commandData)
{
    mDiagnosticProvider.ResetEthNetworkDiagnosticsCounts();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Protocols::InteractionModel::Status::Success);
}
#endif

EthernetDiagosticsGlobalInstance gEthernetDiagosticsInstance(DeviceLayer::GetDiagnosticDataProvider());

} // anonymous namespace

void MatterEthernetNetworkDiagnosticsPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&gEthernetDiagosticsInstance);
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&gEthernetDiagosticsInstance);
}
