#include "WindowCovering.h"
#include <devices/Types.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip::app {

WindowCovering::WindowCovering(TimerDelegate & timerDelegate, Clusters::IdentifyDelegate & identifyDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kMySensor, 1)), mTimerDelegate(timerDelegate),
    mIdentifyDelegate(identifyDelegate)
{}

CHIP_ERROR WindowCovering::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                    EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // Wire up the mandatory identify delegate
    mIdentifyCluster.Create(IdentifyCluster::Config(endpoint, mTimerDelegate).WithDelegate(&mIdentifyDelegate));
    ReturnErrorOnFailure(provider.AddCluster(mIdentifyCluster.Registration()));

    mMySensorCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mMySensorCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void WindowCovering::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mMySensorCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mMySensorCluster.Cluster()));
        mMySensorCluster.Destroy();
    }
    if (mIdentifyCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mIdentifyCluster.Cluster()));
        mIdentifyCluster.Destroy();
    }
}

} // namespace chip::app
