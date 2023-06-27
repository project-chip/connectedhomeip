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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-delegate.h>
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::LaundryWasherControls::Attributes;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kLaundryWasherControlsDelegateTableSize =
    EMBER_AF_LAUNDRY_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

// -----------------------------------------------------------------------------
// Delegate Implementation
//
using chip::app::Clusters::LaundryWasherControls::Delegate;
namespace {
Delegate * gDelegateTable[kLaundryWasherControlsDelegateTableSize] = { nullptr };
}

namespace chip {
namespace app {
namespace Clusters {
namespace LaundryWasherControls {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, LaundryWasherControls::Id, EMBER_AF_LAUNDRY_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kLaundryWasherControlsDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}
} // namespace LaundryWasherControls
} // namespace Clusters
} // namespace app
} // namespace chip

namespace {
Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep =
        emberAfGetClusterServerEndpointIndex(endpoint, LaundryWasherControls::Id, EMBER_AF_LAUNDRY_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kLaundryWasherControlsDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Laundry Washer Control has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}

class LaundryWasherControlsAttrAccess : public AttributeAccessInterface
{
public:
    LaundryWasherControlsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), LaundryWasherControls::Id) {}
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadSpinSpeeds(AttributeValueEncoder & aEncoder, Delegate * delegate);
    CHIP_ERROR ReadSpinSpeedCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNumberOfRinses(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedRinses(AttributeValueEncoder & aEncoder, Delegate * delegate);
};

CHIP_ERROR LaundryWasherControlsAttrAccess::ReadSpinSpeeds(AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetSpinSpeedsList(aEncoder);
}

CHIP_ERROR LaundryWasherControlsAttrAccess::ReadSpinSpeedCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::Nullable<uint8_t> speedSetting;
    SpinSpeedCurrent::Get(endpoint, speedSetting);
    uint8_t ret = 0;
    if (!speedSetting.IsNull())
    {
        ret = speedSetting.Value();
    }

    return aEncoder.Encode(ret);
}

CHIP_ERROR LaundryWasherControlsAttrAccess::ReadNumberOfRinses(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    LaundryWasherControls::NumberOfRinsesEnum numOfRinses;
    NumberOfRinses::Get(endpoint, &numOfRinses);

    return aEncoder.Encode(numOfRinses);
}

CHIP_ERROR LaundryWasherControlsAttrAccess::ReadSupportedRinses(AttributeValueEncoder & aEncoder, Delegate * delegate)
{
    return delegate->HandleGetSupportedRinses(aEncoder);
}

CHIP_ERROR LaundryWasherControlsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate * delegate = GetDelegate(endpoint);

    if (aPath.mClusterId != LaundryWasherControls::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    switch (aPath.mAttributeId)
    {
    case Attributes::SpinSpeeds::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }
        return ReadSpinSpeeds(aEncoder, delegate);
    case Attributes::SpinSpeedCurrent::Id:
        return ReadSpinSpeedCurrent(endpoint, aEncoder);
    case Attributes::NumberOfRinses::Id:
        return ReadNumberOfRinses(endpoint, aEncoder);
    case Attributes::SupportedRinses::Id:
        if (isDelegateNull(delegate, endpoint))
        {
            return aEncoder.EncodeEmptyList();
        }
        return ReadSupportedRinses(aEncoder, delegate);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
} // namespace

void emberAfLaundryWasherControlsClusterInitCallback(chip::EndpointId endpoint)
{
    return;
}

using imcode = Protocols::InteractionModel::Status;

void MatterLaundryWasherControlsClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    // ToDo
    return;
}

chip::Protocols::InteractionModel::Status
MatterLaundryWasherControlsClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                             EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    // ToDo
    return imcode::Success;
}

LaundryWasherControlsAttrAccess gAttrAccess;

void MatterLaundryWasherControlsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
