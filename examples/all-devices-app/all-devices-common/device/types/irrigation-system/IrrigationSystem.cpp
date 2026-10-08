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

#include "IrrigationSystem.h" // IWYU pragma: keep.
#include "lib/support/CodeUtils.h"
namespace chip {
namespace app {

CHIP_ERROR IrrigationSystem::Register(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                                      EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);
    mEndpointId = allocator.Allocate();
    ReturnErrorOnFailure(RegisterDescriptor(mEndpointId, provider, composition));

    Clusters::IdentifyCluster::Config Iconfig(mEndpointId, mTimerDelegate);
    Iconfig.WithDelegate(mIdentifyDelegate);
    mIdentifyCluster.Create(Iconfig);

    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mOperationalStateCluster.Create(mEndpointId, *mOperationalStateDelegate);
    ReturnErrorOnFailure(provider.AddCluster(mOperationalStateCluster.Registration()));

    mFlowMeasurementCluster.Create(mEndpointId);
    ReturnErrorOnFailure(provider.AddCluster(mFlowMeasurementCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));

    ReturnErrorOnFailure(RegisterParts(allocator, provider, composition));
    transaction.Commit();

    return CHIP_NO_ERROR;
}

void IrrigationSystem::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterParts(provider);
    UnregisterDescriptor(mEndpointId, provider);
    mEndpointId = kInvalidEndpointId;

    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
    if (mFlowMeasurementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mFlowMeasurementCluster.Cluster()));
        mFlowMeasurementCluster.Destroy();
    }
    if (mOperationalStateCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mOperationalStateCluster.Cluster()));
        mOperationalStateCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
