/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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
#include "power-topology-server.h"

#include <protocols/interaction_model/StatusCode.h>

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::DataModel;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerTopology;
using namespace chip::app::Clusters::PowerTopology::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace PowerTopology {

CHIP_ERROR Instance::Init()
{
    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

bool Instance::SupportsOptAttr(OptionalAttributes aOptionalAttrs) const
{
    return mOptionalAttrs.Has(aOptionalAttrs);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeature));
        break;
    case AvailableEndpoints::Id:
        return ReadAvailableEndpoints(aEncoder);
    case ActiveEndpoints::Id:
        return ReadActiveEndpoints(aEncoder);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::ReadAvailableEndpoints(AttributeValueEncoder & aEncoder)
{
    if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeAvailableEndpoints))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
    VerifyOrReturnError(HasFeature(Feature::kSetTopology), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                        ChipLogError(Zcl, "Power Topology: can not get AvailableEndpoints, feature is not supported"));

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            EndpointId endpointId;
            auto err = mDelegate.GetAvailableEndpointAtIndex(i, endpointId);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(endpointId));
        }
    });
}

CHIP_ERROR Instance::ReadActiveEndpoints(AttributeValueEncoder & aEncoder)
{
    if (!SupportsOptAttr(OptionalAttributes::kOptionalAttributeActiveEndpoints))
    {
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
    VerifyOrReturnError(HasFeature(Feature::kDynamicPowerFlow), CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE,
                        ChipLogError(Zcl, "Power Topology: can not get ActiveEndpoints, feature is not supported"));

    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (uint8_t i = 0; true; i++)
        {
            EndpointId endpointId;
            auto err = mDelegate.GetActiveEndpointAtIndex(i, endpointId);
            if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
            {
                return CHIP_NO_ERROR;
            }
            ReturnErrorOnFailure(err);
            ReturnErrorOnFailure(encoder.Encode(endpointId));
        }
    });
}

} // namespace PowerTopology
} // namespace Clusters
} // namespace app
} // namespace chip
