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
#include <device/types/window-covering/WindowCovering.h>
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

WindowCovering::WindowCovering(Clusters::WindowCovering::WindowCoveringDelegate & delegate,
                               Clusters::IdentifyDelegate & identifyDelegate, const Context & context,
                               BitFlags<Clusters::WindowCovering::Feature> features,
                               Clusters::WindowCovering::OptionalAttributeSet optionalAttributes) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kWindowCovering, 1)),
    mTimerDelegate(context.timerDelegate), mOptionalAttributes(optionalAttributes), mContext(context), mFeatures(features),
    mWindowCoveringDelegate(delegate), mIdentifyDelegate(identifyDelegate)
{
    // Sanity check: the Window Covering device type specification mandates that either Lift or Tilt, or both, be enabled.
    VerifyOrDie(mFeatures.Has(Clusters::WindowCovering::Feature::kLift) || mFeatures.Has(Clusters::WindowCovering::Feature::kTilt));
}

CHIP_ERROR WindowCovering::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                    EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Wire up the mandatory identify delegate
    mIdentifyCluster.Create(Clusters::IdentifyCluster::Config(endpoint, mContext.timerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    // The WindowCoveringDelegate is mandatory and must be supplied at construction time.
    mWindowCoveringDelegate.SetEndpoint(endpoint);

    Clusters::WindowCovering::WindowCoveringCluster::Config config(mWindowCoveringDelegate);
    config.WithFeatures(mFeatures).WithOptionalAttributes(mOptionalAttributes);
    mWindowCoveringCluster.Create(endpoint, config);

    ReturnErrorOnFailure(provider.AddCluster(mWindowCoveringCluster.Registration()));

    // Call hook to register optional clusters (like Groups on the simulated subclass)
    ReturnErrorOnFailure(RegisterOptionalClusters(endpoint, provider));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WindowCovering::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    UnregisterOptionalClusters(provider);

    if (mWindowCoveringCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWindowCoveringCluster.Cluster()));
        mWindowCoveringCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
