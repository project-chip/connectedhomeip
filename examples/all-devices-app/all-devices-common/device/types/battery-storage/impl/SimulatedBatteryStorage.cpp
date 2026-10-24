/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/battery-storage/impl/SimulatedBatteryStorage.h>

#include <lib/support/logging/CHIPLogging.h>

namespace chip::app {

SimulatedBatteryStorage::SimulatedBatteryStorage(const Context & context) :
    BatteryStorage(context), mTimerDelegate(context.timerDelegate)
{}

CHIP_ERROR SimulatedBatteryStorage::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mTimerDelegate));
    return provider.AddCluster(mIdentifyCluster.Registration());
}

void SimulatedBatteryStorage::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app
