/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>

#include "laundry-dryer-controls-delegate.h"
#include "laundry-dryer-controls-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/Server.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryDryerControls;
using namespace chip::app::Clusters::LaundryDryerControls::Attributes;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kLaundryDryerControlsDelegateTableSize =
    MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation
//
namespace {
Delegate * gDelegateTable[kLaundryDryerControlsDelegateTableSize] = { nullptr };
}

namespace {
Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, LaundryDryerControls::Id,
                                                       MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kLaundryDryerControlsDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

} // namespace

LaundryDryerControlsServer LaundryDryerControlsServer::sInstance;

/**********************************************************
 * LaundryDryerControlsServer public methods
 *********************************************************/
void LaundryDryerControlsServer::SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, LaundryDryerControls::Id,
                                                       MATTER_DM_LAUNDRY_DRYER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kLaundryDryerControlsDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

LaundryDryerControlsServer & LaundryDryerControlsServer::Instance()
{
    return sInstance;
}

Status LaundryDryerControlsServer::SetSelectedDrynessLevel(EndpointId endpointId, DrynessLevelEnum newSelectedDrynessLevel)
{
    DataModel::Nullable<DrynessLevelEnum> selectedDrynessLevel;
    Status res = SelectedDrynessLevel::Get(endpointId, selectedDrynessLevel);

    if ((res == Status::Success) && (selectedDrynessLevel != newSelectedDrynessLevel))
    {
        res = SelectedDrynessLevel::Set(endpointId, newSelectedDrynessLevel);
    }

    return res;
}

Status LaundryDryerControlsServer::GetSelectedDrynessLevel(EndpointId endpointId,
                                                           DataModel::Nullable<DrynessLevelEnum> & selectedDrynessLevel)
{
    return SelectedDrynessLevel::Get(endpointId, selectedDrynessLevel);
}

/**********************************************************
 * LaundryDryerControlsServer private methods
 *********************************************************/
CHIP_ERROR LaundryDryerControlsServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != LaundryDryerControls::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    switch (aPath.mAttributeId)
    {
    case Attributes::SupportedDrynessLevels::Id:
        return ReadSupportedDrynessLevels(aPath, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryDryerControlsServer::ReadSupportedDrynessLevels(const ConcreteReadAttributePath & aPath,
                                                                  AttributeValueEncoder & aEncoder)
{
    Delegate * delegate = GetDelegate(aPath.mEndpointId);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));

    return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            DrynessLevelEnum supportedDrynessLevel;
            auto err = delegate->GetSupportedDrynessLevelAtIndex(i, supportedDrynessLevel);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(supportedDrynessLevel));
        }
    });
}

/**********************************************************
 * Register LaundryDryerControlsServer
 *********************************************************/

void MatterLaundryDryerControlsPluginServerInitCallback()
{
    LaundryDryerControlsServer & laundryDryerControlsServer = LaundryDryerControlsServer::Instance();
    AttributeAccessInterfaceRegistry::Instance().Register(&laundryDryerControlsServer);
}

Status MatterLaundryDryerControlsClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                                          EmberAfAttributeType attributeType, uint16_t size,
                                                                          uint8_t * value)
{
    Delegate * delegate = GetDelegate(attributePath.mEndpointId);
    VerifyOrDie((delegate != nullptr) && "Dryer Controls implementation requires a registered delegate for validation.");
    switch (attributePath.mAttributeId)
    {
    case Attributes::SelectedDrynessLevel::Id: {
        uint8_t drynessLevelIdx = 0;
        if (NumericAttributeTraits<uint8_t>::IsNullValue(*value))
        {
            return Status::Success;
        }
        while (true)
        {
            DrynessLevelEnum supportedDryness;
            auto err = delegate->GetSupportedDrynessLevelAtIndex(drynessLevelIdx, supportedDryness);
            if (err != CHIP_NO_ERROR)
            {
                // Can't find the attribute to be written in the supported list (CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                // Or can't get the correct supported list
                return Status::ConstraintError;
            }
            static_assert(sizeof(DrynessLevelEnum) == sizeof(*value), "Enum size doesn't match parameter size");
            if (supportedDryness == static_cast<DrynessLevelEnum>(*value))
            {
                // The written attribute is one of the supported item
                return Status::Success;
            }
            drynessLevelIdx++;
        }
    }
    }
    return Status::Success;
}
