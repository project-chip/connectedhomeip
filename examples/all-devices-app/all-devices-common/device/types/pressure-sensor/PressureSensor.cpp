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
#include <device/types/pressure-sensor/PressureSensor.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

PressureSensor::PressureSensor(TimerDelegate & timerDelegate, PressureMeasurementCluster::Config pressureConfig) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kPressureSensor, 1)), mTimerDelegate(timerDelegate),
    mPressureConfig(pressureConfig)
{}

CHIP_ERROR PressureSensor::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mPressureMeasurementCluster.Create(endpoint, mPressureConfig);
    ReturnErrorOnFailure(provider.AddCluster(mPressureMeasurementCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void PressureSensor::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mPressureMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mPressureMeasurementCluster.Cluster()));
        mPressureMeasurementCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
