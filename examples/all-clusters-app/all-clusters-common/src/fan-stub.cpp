/*
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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::FanControl::Attributes;

namespace {

/*
 * TODO: This is a stop-gap solution to allow the existing fan control cluster tests to run after changes to
 * the cluster objects for TE1. This should be removed once #6496 is resolved as it will likely result in a
 * FanControl delegate added to the SDK.
 *
 * FYI... The previous implementation of the FanControl cluster set the speedCurrent/percentCurrent when it received
 * speedSetting/percentSetting. The new implementation of the FanControl cluster does not do this as this should
 * really be done by the application.
 */

class FanAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the FanControl cluster on all endpoints.
    FanAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), FanControl::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    CHIP_ERROR ReadPercentCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSpeedCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

CHIP_ERROR FanAttrAccess::ReadPercentCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    // Return PercentSetting attribute value for now
    DataModel::Nullable<uint8_t> percentSetting;
    PercentSetting::Get(endpoint, percentSetting);
    uint8_t ret = 0;
    if (!percentSetting.IsNull())
    {
        ret = percentSetting.Value();
    }

    return aEncoder.Encode(ret);
}

CHIP_ERROR FanAttrAccess::ReadSpeedCurrent(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    // Return SpeedCurrent attribute value for now
    DataModel::Nullable<uint8_t> speedSetting;
    SpeedSetting::Get(endpoint, speedSetting);
    uint8_t ret = 0;
    if (!speedSetting.IsNull())
    {
        ret = speedSetting.Value();
    }

    return aEncoder.Encode(ret);
}

FanAttrAccess gAttrAccess;

CHIP_ERROR FanAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == FanControl::Id);

    switch (aPath.mAttributeId)
    {
    case SpeedCurrent::Id:
        return ReadSpeedCurrent(aPath.mEndpointId, aEncoder);
    case PercentCurrent::Id:
        return ReadPercentCurrent(aPath.mEndpointId, aEncoder);
    default:
        break;
    }
    return CHIP_NO_ERROR;
}
} // anonymous namespace

void emberAfFanControlClusterInitCallback(EndpointId endpoint)
{
    uint32_t featureMap = 0;

    featureMap |= to_underlying(FanControl::Feature::kMultiSpeed);
    featureMap |= to_underlying(FanControl::Feature::kMultiSpeed);
    featureMap |= to_underlying(FanControl::Feature::kAuto);

    FeatureMap::Set(endpoint, featureMap);

    registerAttributeAccessOverride(&gAttrAccess);
}
