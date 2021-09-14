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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterceptor.h>
#include <app/CommandHandler.h>
#include <app/MessageDef/AttributeDataElement.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <lib/core/Optional.h>
#include <platform/ConnectivityManager.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::EthernetNetworkDiagnostics::Attributes;
using chip::DeviceLayer::ConnectivityManager;

namespace {

class EthernetDiagosticsAttrInterceptor : public AttributeAccessInterceptor
{
public:
    // Register for the EthernetNetworkDiagnostics cluster on all endpoints.
    EthernetDiagosticsAttrInterceptor() :
        AttributeAccessInterceptor(Optional<EndpointId>::Missing(), EthernetNetworkDiagnostics::Id)
    {}

    CHIP_ERROR ReadAttributeData(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter, bool * aDataRead) override;

private:
    CHIP_ERROR ReadDataIfSupported(CHIP_ERROR (ConnectivityManager::*getter)(uint64_t &), TLV::TLVWriter * aWriter);
};

EthernetDiagosticsAttrInterceptor gAttrInterceptor;

CHIP_ERROR EthernetDiagosticsAttrInterceptor::ReadAttributeData(ClusterInfo & aClusterInfo, TLV::TLVWriter * aWriter,
                                                                bool * aDataRead)
{
    if (aClusterInfo.mClusterId != EthernetNetworkDiagnostics::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    *aDataRead = true;
    switch (aClusterInfo.mFieldId)
    {
    case Ids::PacketRxCount: {
        return ReadDataIfSupported(&ConnectivityManager::GetEthPacketRxCount, aWriter);
    }
    case Ids::PacketTxCount: {
        return ReadDataIfSupported(&ConnectivityManager::GetEthPacketTxCount, aWriter);
    }
    case Ids::TxErrCount: {
        return ReadDataIfSupported(&ConnectivityManager::GetEthTxErrCount, aWriter);
    }
    case Ids::CollisionCount: {
        return ReadDataIfSupported(&ConnectivityManager::GetEthCollisionCount, aWriter);
    }
    case Ids::OverrunCount: {
        return ReadDataIfSupported(&ConnectivityManager::GetEthOverrunCount, aWriter);
    }
    default: {
        *aDataRead = false;
        break;
    }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR EthernetDiagosticsAttrInterceptor::ReadDataIfSupported(CHIP_ERROR (ConnectivityManager::*getter)(uint64_t &),
                                                                  TLV::TLVWriter * aWriter)
{
    uint64_t data;
    CHIP_ERROR err = (DeviceLayer::ConnectivityMgr().*getter)(data);
    if (err == CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE)
    {
        data = 0;
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    return aWriter->Put(TLV::ContextTag(AttributeDataElement::kCsTag_Data), data);
}
} // anonymous namespace

bool emberAfEthernetNetworkDiagnosticsClusterResetCountsCallback(EndpointId endpoint, app::CommandHandler * commandObj)
{
    EmberAfStatus status = EthernetNetworkDiagnostics::Attributes::SetPacketRxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketRxCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetPacketTxCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset PacketTxCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetTxErrCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset TxErrCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetCollisionCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset CollisionCount attribute"));

    status = EthernetNetworkDiagnostics::Attributes::SetOverrunCount(endpoint, 0);
    VerifyOrExit(status == EMBER_ZCL_STATUS_SUCCESS, ChipLogError(Zcl, "Failed to reset OverrunCount attribute"));

exit:
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

void emberAfEthernetNetworkDiagnosticsClusterServerInitCallback(EndpointId endpoint)
{
    static bool interceptorRegistered = false;
    if (!interceptorRegistered)
    {
        registerAttributeAccessInterceptor(&gAttrInterceptor);
        interceptorRegistered = true;
    }
}
