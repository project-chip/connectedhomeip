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

#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/config.h>

#include "laundry-washer-controls-delegate.h"
#include "laundry-washer-controls-server.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/server/Server.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::LaundryWasherControls::Attributes;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kLaundryWasherControlsDelegateTableSize =
    MATTER_DM_LAUNDRY_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation
//
namespace {
Delegate * gDelegateTable[kLaundryWasherControlsDelegateTableSize] = { nullptr };
}

namespace {
Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, LaundryWasherControls::Id,
                                                       MATTER_DM_LAUNDRY_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kLaundryWasherControlsDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

} // namespace

LaundryWasherControlsServer LaundryWasherControlsServer::sInstance;

/**********************************************************
 * LaundryWasherControlsServer public methods
 *********************************************************/
void LaundryWasherControlsServer::SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, LaundryWasherControls::Id,
                                                       MATTER_DM_LAUNDRY_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kLaundryWasherControlsDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

LaundryWasherControlsServer & LaundryWasherControlsServer::Instance()
{
    return sInstance;
}

Status LaundryWasherControlsServer::SetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> spinSpeedCurrent)
{
    DataModel::Nullable<uint8_t> spinSpeedCurrentNow;
    Status res = SpinSpeedCurrent::Get(endpointId, spinSpeedCurrentNow);
    if ((res == Status::Success) && (spinSpeedCurrentNow != spinSpeedCurrent))
    {
        res = SpinSpeedCurrent::Set(endpointId, spinSpeedCurrent);
    }

    return res;
}

Status LaundryWasherControlsServer::GetSpinSpeedCurrent(EndpointId endpointId, DataModel::Nullable<uint8_t> & spinSpeedCurrent)
{
    return SpinSpeedCurrent::Get(endpointId, spinSpeedCurrent);
}

Status LaundryWasherControlsServer::SetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum newNumberOfRinses)
{
    NumberOfRinsesEnum numberOfRinses;
    Status res = NumberOfRinses::Get(endpointId, &numberOfRinses);

    if ((res == Status::Success) && (numberOfRinses != newNumberOfRinses))
    {
        res = NumberOfRinses::Set(endpointId, newNumberOfRinses);
    }

    return res;
}

Status LaundryWasherControlsServer::GetNumberOfRinses(EndpointId endpointId, NumberOfRinsesEnum & numberOfRinses)
{
    return NumberOfRinses::Get(endpointId, &numberOfRinses);
}

/**********************************************************
 * LaundryWasherControlsServer private methods
 *********************************************************/
CHIP_ERROR LaundryWasherControlsServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    if (aPath.mClusterId != LaundryWasherControls::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    switch (aPath.mAttributeId)
    {
    case Attributes::SpinSpeeds::Id:
        return ReadSpinSpeeds(aPath, aEncoder);
    case Attributes::SupportedRinses::Id:
        return ReadSupportedRinses(aPath, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR LaundryWasherControlsServer::ReadSpinSpeeds(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    Delegate * delegate = GetDelegate(aPath.mEndpointId);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));

    return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            char buffer[kMaxSpinSpeedLength];
            MutableCharSpan spinSpeed(buffer);
            auto err = delegate->GetSpinSpeedAtIndex(i, spinSpeed);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(spinSpeed));
        }
    });
}

CHIP_ERROR LaundryWasherControlsServer::ReadSupportedRinses(const ConcreteReadAttributePath & aPath,
                                                            AttributeValueEncoder & aEncoder)
{
    Delegate * delegate = GetDelegate(aPath.mEndpointId);
    VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));

    return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            NumberOfRinsesEnum supportedRinse;
            auto err = delegate->GetSupportedRinseAtIndex(i, supportedRinse);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(supportedRinse));
        }
    });
}

/**********************************************************
 * Register LaundryWasherControlsServer
 *********************************************************/

void MatterLaundryWasherControlsPluginServerInitCallback()
{
    LaundryWasherControlsServer & laundryWasherControlsServer = LaundryWasherControlsServer::Instance();
    registerAttributeAccessOverride(&laundryWasherControlsServer);
}

Status MatterLaundryWasherControlsClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                                           EmberAfAttributeType attributeType, uint16_t size,
                                                                           uint8_t * value)
{
    Delegate * delegate = GetDelegate(attributePath.mEndpointId);
    VerifyOrDie((delegate != nullptr) && "Washer Controls implementation requires a registered delegate for validation.");
    switch (attributePath.mAttributeId)
    {
    case Attributes::SpinSpeedCurrent::Id: {
        if (NumericAttributeTraits<uint8_t>::IsNullValue(*value))
        {
            return Status::Success;
        }
        char buffer[LaundryWasherControlsServer::kMaxSpinSpeedLength];
        MutableCharSpan spinSpeed(buffer);
        uint8_t spinSpeedIndex = *value;
        auto err               = delegate->GetSpinSpeedAtIndex(spinSpeedIndex, spinSpeed);
        if (err == CHIP_NO_ERROR)
        {
            return Status::Success;
        }
        return Status::ConstraintError;
    }
    case Attributes::NumberOfRinses::Id: {
        uint8_t supportedRinseIdx = 0;
        while (true)
        {
            NumberOfRinsesEnum supportedRinse;
            auto err = delegate->GetSupportedRinseAtIndex(supportedRinseIdx, supportedRinse);
            if (err != CHIP_NO_ERROR)
            {
                // Can't find the attribute to be written in the supported list (CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
                // Or can't get the correct supported list
                return Status::InvalidInState;
            }
            if (supportedRinse == static_cast<NumberOfRinsesEnum>(*value))
            {
                // The written attribute is one of the supported item
                return Status::Success;
            }
            supportedRinseIdx++;
        }
    }
    }
    return Status::Success;
}
