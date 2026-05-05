/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
#include <devices/Types.h>
#include <devices/proximity-ranger/ProximityRangerDevice.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

ProximityRangerDevice::ProximityRangerDevice(const Context & context) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kProximityRanger, 1)), mContext(context)
{}

CHIP_ERROR ProximityRangerDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mContext.timerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // Compute optional attributes from driver feature map (code-driven path, not codegen)
    auto featureFlags = mContext.driver.GetFeatureMap();
    AttributeSet attrs;
    if (featureFlags.Has(ProximityRanging::Feature::kWiFiUsdProximityDetection))
    {
        attrs.ForceSet<ProximityRanging::Attributes::WiFiDevIK::Id>();
    }
    if (featureFlags.Has(ProximityRanging::Feature::kBleBeaconRssi))
    {
        attrs.ForceSet<ProximityRanging::Attributes::BLEDeviceID::Id>();
    }
    if (featureFlags.Has(ProximityRanging::Feature::kBluetoothChannelSounding))
    {
        attrs.ForceSet<ProximityRanging::Attributes::BLTDevIK::Id>();
        attrs.ForceSet<ProximityRanging::Attributes::BLTCSSecurityLevel::Id>();
        attrs.ForceSet<ProximityRanging::Attributes::BLTCSModeCapability::Id>();
    }

    mProximityRangingCluster.Create(endpoint, ProximityRanging::ProximityRangingCluster::OptionalAttributes(attrs));
    mProximityRangingCluster.Cluster().SetDriver(&mContext.driver);
    ReturnErrorOnFailure(provider.AddCluster(mProximityRangingCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void ProximityRangerDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

    if (mProximityRangingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mProximityRangingCluster.Cluster()));
        mProximityRangingCluster.Destroy();
    }

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
