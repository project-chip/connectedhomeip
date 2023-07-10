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
#include <app/clusters/laundry-washer-controls-server/laundry-washer-controls-server.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::LaundryWasherControls;
using namespace chip::app::Clusters::LaundryWasherControls::Attributes;
using chip::Protocols::InteractionModel::Status;

namespace {
class LaundryWasherControlsAttrAccess : public AttributeAccessInterface
{
public:
    LaundryWasherControlsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), LaundryWasherControls::Id) {}
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadSpinSpeeds(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSupportedRinses(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR LaundryWasherControlsAttrAccess::ReadSpinSpeeds(const ConcreteReadAttributePath & aPath,
                                                           AttributeValueEncoder & aEncoder)
{
    const LaundryWasherControls::LaundryWasherManager * gLaundryWasherManager = LaundryWasherControls::getLaundryWasherManager();
    const LaundryWasherControls::LaundryWasherManager::AttributeProvider<CharSpan> attrProvider =
        gLaundryWasherManager->getSpinSpeedProvider(aPath.mEndpointId);
    if (attrProvider.begin() == nullptr)
    {
        aEncoder.EncodeEmptyList();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR err;
    err = aEncoder.EncodeList([attrProvider](const auto & encoder) -> CHIP_ERROR {
        const auto * end = attrProvider.end();
        for (auto * it = attrProvider.begin(); it != end; ++it)
        {
            auto & spinSpeed = *it;
            ReturnErrorOnFailure(encoder.Encode(spinSpeed));
        }
        return CHIP_NO_ERROR;
    });
    return err;
}

CHIP_ERROR LaundryWasherControlsAttrAccess::ReadSupportedRinses(const ConcreteReadAttributePath & aPath,
                                                                AttributeValueEncoder & aEncoder)
{
    const LaundryWasherControls::LaundryWasherManager * gLaundryWasherManager = LaundryWasherControls::getLaundryWasherManager();
    const LaundryWasherControls::LaundryWasherManager::AttributeProvider<NumberOfRinsesEnum> attrProvider =
        gLaundryWasherManager->getSupportedRinseProvider(aPath.mEndpointId);
    if (attrProvider.begin() == nullptr)
    {
        aEncoder.EncodeEmptyList();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR err;
    err = aEncoder.EncodeList([attrProvider](const auto & encoder) -> CHIP_ERROR {
        const auto * end = attrProvider.end();
        for (auto * it = attrProvider.begin(); it != end; ++it)
        {
            auto & rinse = *it;
            ReturnErrorOnFailure(encoder.Encode(rinse));
        }
        return CHIP_NO_ERROR;
    });
    return err;
}

CHIP_ERROR LaundryWasherControlsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
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
} // namespace

void emberAfLaundryWasherControlsClusterInitCallback(chip::EndpointId endpoint)
{
    return;
}

LaundryWasherControlsAttrAccess gAttrAccess;

void MatterLaundryWasherControlsPluginServerInitCallback()
{
    registerAttributeAccessOverride(&gAttrAccess);
}
