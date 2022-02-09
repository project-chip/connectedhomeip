/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/* This file contains the glue code for passing the incoming OTA Requestor cluster commands
 * to the OTA Requestor object that handles them
 */

#include <app/AttributeAccessInterface.h>
#include <app/EventLogging.h>
#include <app/clusters/ota-requestor/ota-requestor-server.h>
#include <app/util/attribute-storage.h>
#include <platform/OTARequestorInterface.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor;
using namespace chip::app::Clusters::OtaSoftwareUpdateRequestor::Attributes;

namespace {

class OtaSoftwareUpdateRequestorAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the OTA Requestor Cluster on all endpoints.
    OtaSoftwareUpdateRequestorAttrAccess() :
        AttributeAccessInterface(Optional<EndpointId>::Missing(), OtaSoftwareUpdateRequestor::Id)
    {}

    // TODO: Implement Read/Write for OtaSoftwareUpdateRequestorAttrAccess
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    CHIP_ERROR Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override;
};

OtaSoftwareUpdateRequestorAttrAccess gAttrAccess;

CHIP_ERROR OtaSoftwareUpdateRequestorAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::DefaultOtaProviders::Id:
        return aEncoder.Encode(DataModel::List<uint8_t>());
    default:
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OtaSoftwareUpdateRequestorAttrAccess::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case Attributes::DefaultOtaProviders::Id: {
        DataModel::DecodableList<OtaSoftwareUpdateRequestor::Structs::ProviderLocation::DecodableType> list;
        ReturnErrorOnFailure(aDecoder.Decode(list));
        // Ignore the list for now
        break;
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

} // namespace

// -----------------------------------------------------------------------------
// Global functions
EmberAfStatus OtaRequestorServerSetUpdateState(OTAUpdateStateEnum value)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    // Find all endpoints that have OtaSoftwareUpdateRequestor implemented
    for (auto endpoint : EnabledEndpointsWithServerCluster(OtaSoftwareUpdateRequestor::Id))
    {
        OTAUpdateStateEnum currentValue;
        status = Attributes::UpdateState::Get(endpoint, &currentValue);
        VerifyOrDie(EMBER_ZCL_STATUS_SUCCESS == status);

        if (currentValue != value)
        {
            status = Attributes::UpdateState::Set(endpoint, value);
            VerifyOrDie(EMBER_ZCL_STATUS_SUCCESS == status);
        }
    }

    return status;
}

EmberAfStatus OtaRequestorServerGetUpdateState(chip::EndpointId endpointId, OTAUpdateStateEnum & value)
{
    return Attributes::UpdateState::Get(endpointId, &value);
}

EmberAfStatus OtaRequestorServerSetUpdateStateProgress(app::DataModel::Nullable<uint8_t> value)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;

    // Find all endpoints that have OtaSoftwareUpdateRequestor implemented
    for (auto endpoint : EnabledEndpointsWithServerCluster(OtaSoftwareUpdateRequestor::Id))
    {
        app::DataModel::Nullable<uint8_t> currentValue;
        status = Attributes::UpdateStateProgress::Get(endpoint, currentValue);
        VerifyOrDie(EMBER_ZCL_STATUS_SUCCESS == status);

        if (currentValue != value)
        {
            status = Attributes::UpdateStateProgress::Set(endpoint, value);
            VerifyOrDie(EMBER_ZCL_STATUS_SUCCESS == status);
        }
    }

    return status;
}

EmberAfStatus OtaRequestorServerGetUpdateStateProgress(chip::EndpointId endpointId, DataModel::Nullable<uint8_t> & value)
{
    return Attributes::UpdateStateProgress::Get(endpointId, value);
}

void OtaRequestorServerOnStateTransition(DataModel::Nullable<OTAUpdateStateEnum> previousState, OTAUpdateStateEnum newState,
                                         OTAChangeReasonEnum reason, DataModel::Nullable<uint32_t> const & targetSoftwareVersion)
{
    if (!previousState.IsNull() && previousState.Value() == newState)
    {
        ChipLogError(Zcl, "Previous state and new state are the same, no event to log");
        return;
    }

    // Find all endpoints that have OtaSoftwareUpdateRequestor implemented
    for (auto endpoint : EnabledEndpointsWithServerCluster(OtaSoftwareUpdateRequestor::Id))
    {
        Events::StateTransition::Type event{ previousState, newState, reason, targetSoftwareVersion };
        EventNumber eventNumber;

        CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(Zcl, "Failed to record StateTransition event: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

void OtaRequestorServerOnVersionApplied(uint32_t softwareVersion, uint16_t productId)
{
    // Find all endpoints that have OtaSoftwareUpdateRequestor implemented
    for (auto endpoint : EnabledEndpointsWithServerCluster(OtaSoftwareUpdateRequestor::Id))
    {
        Events::VersionApplied::Type event{ softwareVersion, productId };
        EventNumber eventNumber;

        CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(Zcl, "Failed to record VersionApplied event: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

void OtaRequestorServerOnDownloadError(uint32_t softwareVersion, uint64_t bytesDownloaded,
                                       DataModel::Nullable<uint8_t> progressPercent, DataModel::Nullable<int64_t> platformCode)
{
    // Find all endpoints that have OtaSoftwareUpdateRequestor implemented
    for (auto endpoint : EnabledEndpointsWithServerCluster(OtaSoftwareUpdateRequestor::Id))
    {
        Events::DownloadError::Type event{ softwareVersion, bytesDownloaded, progressPercent, platformCode };
        EventNumber eventNumber;

        CHIP_ERROR err = LogEvent(event, endpoint, eventNumber);
        if (CHIP_NO_ERROR != err)
        {
            ChipLogError(Zcl, "Failed to record DownloadError event: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
}

// -----------------------------------------------------------------------------
// Callbacks implementation

bool emberAfOtaSoftwareUpdateRequestorClusterAnnounceOtaProviderCallback(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    EmberAfStatus status;
    chip::OTARequestorInterface * requestor = chip::GetRequestorInstance();

    if (requestor != nullptr)
    {
        status = requestor->HandleAnnounceOTAProvider(commandObj, commandPath, commandData);
    }
    else
    {
        status = EMBER_ZCL_STATUS_FAILURE;
    }

    emberAfSendImmediateDefaultResponse(status);
    return true;
}

// -----------------------------------------------------------------------------
// Plugin initialization

void MatterOtaSoftwareUpdateRequestorPluginServerInitCallback(void)
{
    registerAttributeAccessOverride(&gAttrAccess);
}
