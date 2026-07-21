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

#include "BatteryPowerSource.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

BatteryPowerSource::BatteryPowerSource(CharSpan description, Clusters::PowerSource::BatReplaceabilityEnum replaceability,
                                       TimerDelegate & timerDelegate) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kPowerSource, 1)),
    mTimerDelegate(timerDelegate), mDescription(description), mReplaceability(replaceability)
{}

CHIP_ERROR BatteryPowerSource::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                        EndpointComposition composition)
{
    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Power Source (some arbitrary configuration)
    SimpleBatteryPowerSourceCluster::Config config(mDescription, mReplaceability, mTimerDelegate);
    config.usedOptionalAttributes.Set<BatPercentRemainingId>();
    config.status = Clusters::PowerSource::PowerSourceStatusEnum::kActive;
    config.order  = 0;
    config.batPercentRemaining.SetNonNull(200); // 100% (doubled percentage)
    mEndpointList[0] = endpoint;

    mBatteryPowerSourceCluster.Create(endpoint, config);
    ReturnErrorOnFailure(mBatteryPowerSourceCluster.Cluster().SetEndpointList(Span<const EndpointId>(mEndpointList)));
    ReturnErrorOnFailure(provider.AddCluster(mBatteryPowerSourceCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void BatteryPowerSource::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBatteryPowerSourceCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBatteryPowerSourceCluster.Cluster()));
        mBatteryPowerSourceCluster.Destroy();
    }
}

BatteryPowerSource::SimpleBatteryPowerSourceCluster & BatteryPowerSource::BatteryPowerSourceCluster()
{
    VerifyOrDie(mBatteryPowerSourceCluster.IsConstructed());
    return mBatteryPowerSourceCluster.Cluster();
}

} // namespace chip::app
