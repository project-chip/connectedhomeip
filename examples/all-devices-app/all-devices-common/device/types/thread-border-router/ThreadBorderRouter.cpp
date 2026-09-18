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

#include <device/types/thread-border-router/ThreadBorderRouter.h>
#include <devices/Types.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
constexpr uint16_t kThreadVersionForThread_1_3_1 = 5;
} // namespace

ThreadBorderRouter::ThreadBorderRouter(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage,
                                       DeviceLayer::PlatformManager & platformManager, FailSafeContext & failSafeContext) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kThreadBorderRouter, 1)),
    mThreadNetworkDirectoryStorage(storage), mTimerDelegate(timerDelegate), mPlatformManager(platformManager),
    mFailSafeContext(failSafeContext)
{}

ThreadBorderRouter::~ThreadBorderRouter()
{
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    mTimerDelegate.CancelTimer(&mPendingDatasetTimerContext);
}

CHIP_ERROR ThreadBorderRouter::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                        EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // 1. Thread Border Router Management
    ThreadBorderRouterManagementCluster::Config tbrConfig(*this, mFailSafeContext, mBreadCrumbTracker, mPlatformManager);
    mThreadBorderRouterManagementCluster.Create(endpoint, tbrConfig);
    ReturnErrorOnFailure(provider.AddCluster(mThreadBorderRouterManagementCluster.Registration()));

    // 2. Thread Network Directory (optional on device type; included for cluster testing)
    mThreadNetworkDirectoryCluster.Create(endpoint, mThreadNetworkDirectoryStorage);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDirectoryCluster.Registration()));

    // 3. Thread Network Diagnostics
    mThreadNetworkDiagnosticsCluster.Create(endpoint, ThreadNetworkDiagnosticsCluster::ClusterType::kFull,
                                            mThreadDiagnosticsProvider);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDiagnosticsCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ThreadBorderRouter::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mThreadNetworkDiagnosticsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDiagnosticsCluster.Cluster()));
        mThreadNetworkDiagnosticsCluster.Destroy();
    }
    if (mThreadNetworkDirectoryCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDirectoryCluster.Cluster()));
        mThreadNetworkDirectoryCluster.Destroy();
    }
    if (mThreadBorderRouterManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadBorderRouterManagementCluster.Cluster()));
        mThreadBorderRouterManagementCluster.Destroy();
    }
}

CHIP_ERROR ThreadBorderRouter::Init(AttributeChangeCallback * attributeChangeCallback)
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::Init called");
    mAttributeChangeCallback = attributeChangeCallback;
    return CHIP_NO_ERROR;
}

bool ThreadBorderRouter::GetPanChangeSupported()
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::GetPanChangeSupported called");
    return true;
}

void ThreadBorderRouter::GetBorderRouterName(MutableCharSpan & borderRouterName)
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::GetBorderRouterName called");
    CopyCharSpanToMutableCharSpanWithTruncation("all-devices-tbr"_span, borderRouterName);
}

CHIP_ERROR ThreadBorderRouter::GetBorderAgentId(MutableByteSpan & borderAgentId)
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::GetBorderAgentId called");
    static constexpr uint8_t kBorderAgentId[] = { 0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    VerifyOrReturnError(borderAgentId.size() == Clusters::ThreadBorderRouterManagementDelegate::kBorderAgentIdLength,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(kBorderAgentId), borderAgentId);
}

uint16_t ThreadBorderRouter::GetThreadVersion()
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::GetThreadVersion called");
    return kThreadVersionForThread_1_3_1;
}

bool ThreadBorderRouter::GetInterfaceEnabled()
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::GetInterfaceEnabled called");
    return !mActiveDataset.IsEmpty();
}

CHIP_ERROR ThreadBorderRouter::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::GetDataset called (type: %d)", static_cast<int>(type));
    Thread::OperationalDataset * source;
    switch (type)
    {
    case DatasetType::kActive:
        source = &mActiveDataset;
        break;
    case DatasetType::kPending:
        source = &mPendingDataset;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    VerifyOrReturnError(!source->IsEmpty(), CHIP_ERROR_NOT_FOUND);
    return dataset.Init(source->AsByteSpan());
}

void ThreadBorderRouter::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                          ActivateDatasetCallback * callback)
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::SetActiveDataset called (seq: %" PRIu32 ")", sequenceNum);
    if (mActivateDatasetCallback != nullptr)
    {
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    CHIP_ERROR err = mActiveDataset.Init(activeDataset.AsByteSpan());
    if (err != CHIP_NO_ERROR)
    {
        callback->OnActivateDatasetComplete(sequenceNum, err);
        return;
    }

    mActivateDatasetCallback = callback;
    mActivateDatasetSequence = sequenceNum;
    err                      = mTimerDelegate.StartTimer(&mActiveDatasetTimerContext, System::Clock::Seconds32(1));
    if (err != CHIP_NO_ERROR)
    {
        mActivateDatasetCallback = nullptr;
        callback->OnActivateDatasetComplete(sequenceNum, err);
    }
}

CHIP_ERROR ThreadBorderRouter::CommitActiveDataset()
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::CommitActiveDataset called");
    return CHIP_NO_ERROR;
}

CHIP_ERROR ThreadBorderRouter::RevertActiveDataset()
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::RevertActiveDataset called");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR ThreadBorderRouter::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    ChipLogProgress(AppServer, "ThreadBorderRouter::SetPendingDataset called");
    ReturnErrorOnFailure(mPendingDataset.Init(pendingDataset.AsByteSpan()));
    uint32_t delayTimerMillis;
    ReturnErrorOnFailure(mPendingDataset.GetDelayTimer(delayTimerMillis));
    return mTimerDelegate.StartTimer(&mPendingDatasetTimerContext, System::Clock::Milliseconds32(delayTimerMillis));
}

void ThreadBorderRouter::OnActiveDatasetTimerFired()
{
    auto * callback          = mActivateDatasetCallback;
    auto sequenceNum         = mActivateDatasetSequence;
    mActivateDatasetCallback = nullptr;
    if (callback)
    {
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_NO_ERROR);
    }
}

void ThreadBorderRouter::OnPendingDatasetTimerFired()
{
    TEMPORARY_RETURN_IGNORED mActiveDataset.Init(mPendingDataset.AsByteSpan());
    mPendingDataset.Clear();
    if (mAttributeChangeCallback)
    {
        mAttributeChangeCallback->ReportAttributeChanged(
            Clusters::ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        mAttributeChangeCallback->ReportAttributeChanged(
            Clusters::ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    }
}

} // namespace app
} // namespace chip
