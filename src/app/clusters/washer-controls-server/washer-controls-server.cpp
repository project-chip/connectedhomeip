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
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteAttributePath.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>
#include <app/clusters/washer-controls-server/washer-controls-delegate.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WasherControls;
using namespace chip::app::Clusters::WasherControls::Attributes;
using chip::Protocols::InteractionModel::Status;

static constexpr size_t kWasherControlsDelegateTableSize =
     EMBER_AF_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;


// -----------------------------------------------------------------------------
// Delegate Implementation
//
using chip::app::Clusters::WasherControls::Delegate;
namespace {
Delegate * gDelegateTable[kWasherControlsDelegateTableSize] = { nullptr };
}

namespace chip {
namespace app {
namespace Clusters {
namespace WasherControls {

void SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ContentLauncher::Id,
                                                       EMBER_AF_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kWasherControlsDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
    else
    {
    }
}
} // namespace WasherControls
} // namespace Clusters
} // namespace app
} // namespace chip

namespace {
Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, ContentLauncher::Id,
                                                       EMBER_AF_WASHER_CONTROLS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kWasherControlsDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

bool isDelegateNull(Delegate * delegate, EndpointId endpoint)
{
    if (delegate == nullptr)
    {
        ChipLogProgress(Zcl, "Washer Control has no delegate set for endpoint:%u", endpoint);
        return true;
    }
    return false;
}



class WasherControlsAttrAccess : public AttributeAccessInterface
{
public:
    WasherControlsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), WasherControls::Id) {}
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadSpinSpeeds(AttributeValueEncoder & aEncoder, Delegate *delegate);
    CHIP_ERROR ReadSpinSpeedCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadNumberOfRinses(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadMaxRinses(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR WasherControlsAttrAccess::ReadSpinSpeeds(AttributeValueEncoder & aEncoder, Delegate *delegate)
{
    return delegate->HandleGetSpinSpeedsList(aEncoder);
}

CHIP_ERROR WasherControlsAttrAccess::ReadSpinSpeedCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder)
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

CHIP_ERROR WasherControlsAttrAccess::ReadNumberOfRinses(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    DataModel::Nullable<uint8_t> numOfRinses;
    NumberOfRinses::Get(endpoint, numOfRinses);
    uint8_t ret = 0;
    if (!numOfRinses.IsNull())
    {
        ret = numOfRinses.Value();
    }

    return aEncoder.Encode(ret);
}

CHIP_ERROR WasherControlsAttrAccess::ReadMaxRinses(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    uint8_t maxRinses=0;
    MaxRinses::Get(endpoint, &maxRinses);
    return aEncoder.Encode(maxRinses);
}

CHIP_ERROR WasherControlsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    EndpointId endpoint = aPath.mEndpointId;
    Delegate *delegate = GetDelegate(endpoint);

    if (aPath.mClusterId != WasherControls::Id)
    {
        // We shouldn't have been called at all.
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    switch (aPath.mAttributeId)
    {
        case Attributes::SpinSpeeds::Id:
	    if (isDelegateNull(delegate, endpoint)) {
                return aEncoder.EncodeEmptyList();
	    }
            return ReadSpinSpeeds(aEncoder, delegate);
        case Attributes::SpinSpeedCurrent::Id:
            return ReadSpinSpeedCurrent(endpoint, aEncoder);
        case Attributes::NumberOfRinses::Id:
            return ReadNumberOfRinses(endpoint, aEncoder);
        case Attributes::MaxRinses::Id:
            return ReadMaxRinses(endpoint, aEncoder);
        default:
            break;
    }
    return CHIP_NO_ERROR;
}
}

void emberAfWasherControlsClusterInitCallback(chip::EndpointId endpoint)
{
    return;
}

using imcode = Protocols::InteractionModel::Status;

void MatterWasherControlsClusterServerAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath)
{
    // ToDo
    return;
}

chip::Protocols::InteractionModel::Status
MatterWasherControlsClusterServerPreAttributeChangedCallback(const chip::app::ConcreteAttributePath & attributePath,
                                                             EmberAfAttributeType attributeType, uint16_t size, uint8_t * value)
{
    // ToDo
    return imcode::Success;
}

WasherControlsAttrAccess gAttrAccess;

void MatterWasherControlsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}

