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

#include <device/types/thread-border-router/impl/SimulatedThreadBorderRouter.h>

#include <clusters/ThreadBorderRouterManagement/Attributes.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
constexpr uint16_t kThreadVersionForThread_1_3_1 = 5;
} // namespace

SimulatedThreadBorderRouter::SimulatedThreadBorderRouter(const Context & context) :

    ThreadBorderRouter(ThreadBorderRouter::Context{
        .delegate            = *this,
        .failSafeContext     = context.failSafeContext,
        .platformManager     = context.platformManager,
        .breadcrumbTracker   = *this,
        .diagnosticsProvider = *this,
    }),
    mTimerDelegate(context.timerDelegate), mThreadNetworkDirectoryStorage(context.storage), mBorderRouterName(context.nodeLabel)
{}

SimulatedThreadBorderRouter::~SimulatedThreadBorderRouter()
{
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    mTimerDelegate.CancelTimer(&mPendingDatasetTimerContext);
}

void SimulatedThreadBorderRouter::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    mTimerDelegate.CancelTimer(&mPendingDatasetTimerContext);
    CompleteActivation(CHIP_ERROR_CANCELLED);
    mActive                  = NoActiveDataset{};
    mAttributeChangeCallback = nullptr;
    mPendingDataset.Clear();
    ThreadBorderRouter::Unregister(provider);
}

CHIP_ERROR SimulatedThreadBorderRouter::RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
{
    mThreadNetworkDirectoryCluster.Create(endpoint, mThreadNetworkDirectoryStorage);
    return provider.AddCluster(mThreadNetworkDirectoryCluster.Registration());
}

void SimulatedThreadBorderRouter::UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider)
{
    if (mThreadNetworkDirectoryCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDirectoryCluster.Cluster()));
        mThreadNetworkDirectoryCluster.Destroy();
    }
}

CHIP_ERROR SimulatedThreadBorderRouter::Init(AttributeChangeCallback * attributeChangeCallback)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::Init called");
    mAttributeChangeCallback = attributeChangeCallback;
    return CHIP_NO_ERROR;
}

bool SimulatedThreadBorderRouter::GetPanChangeSupported()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetPanChangeSupported called");
    return true;
}

void SimulatedThreadBorderRouter::GetBorderRouterName(MutableCharSpan & borderRouterName)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetBorderRouterName called");
    if (!mBorderRouterName.empty())
    {
        CopyCharSpanToMutableCharSpanWithTruncation(CharSpan::fromCharString(mBorderRouterName.c_str()), borderRouterName);
    }
    else
    {
        CopyCharSpanToMutableCharSpanWithTruncation("all-devices-tbr"_span, borderRouterName);
    }
}

CHIP_ERROR SimulatedThreadBorderRouter::GetBorderAgentId(MutableByteSpan & borderAgentId)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetBorderAgentId called");
    static constexpr uint8_t kBorderAgentId[] = { 0x10, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    VerifyOrReturnError(borderAgentId.size() == Clusters::ThreadBorderRouterManagementDelegate::kBorderAgentIdLength,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(kBorderAgentId), borderAgentId);
}

uint16_t SimulatedThreadBorderRouter::GetThreadVersion()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetThreadVersion called");
    return kThreadVersionForThread_1_3_1;
}

bool SimulatedThreadBorderRouter::GetInterfaceEnabled()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetInterfaceEnabled called");
    return ActiveDataset() != nullptr;
}

CHIP_ERROR SimulatedThreadBorderRouter::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetDataset called (type: %d)", static_cast<int>(type));
    const Thread::OperationalDataset * source = nullptr;
    switch (type)
    {
    case DatasetType::kActive:
        source = ActiveDataset();
        break;
    case DatasetType::kPending:
        source = mPendingDataset.IsEmpty() ? nullptr : &mPendingDataset;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    VerifyOrReturnError(source != nullptr, CHIP_ERROR_NOT_FOUND);
    return dataset.Init(source->AsByteSpan());
}

void SimulatedThreadBorderRouter::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                   ActivateDatasetCallback * callback)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::SetActiveDataset called (seq: %" PRIu32 ")", sequenceNum);
    if (!std::holds_alternative<NoActiveDataset>(mActive))
    {
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    Activating activating;
    activating.callback = callback;
    activating.sequence = sequenceNum;
    CHIP_ERROR err      = activating.dataset.Init(activeDataset.AsByteSpan());
    if (err == CHIP_NO_ERROR)
    {
        err = mTimerDelegate.StartTimer(&mActiveDatasetTimerContext, System::Clock::Seconds32(1));
    }
    if (err != CHIP_NO_ERROR)
    {
        callback->OnActivateDatasetComplete(sequenceNum, err);
        return;
    }

    mActive = activating;
}

CHIP_ERROR SimulatedThreadBorderRouter::CommitActiveDataset()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::CommitActiveDataset called");
    if (auto * uncommitted = std::get_if<ActiveUncommitted>(&mActive))
    {
        mActive = ActiveCommitted{ uncommitted->dataset };
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedThreadBorderRouter::RevertActiveDataset()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::RevertActiveDataset called");
    if (std::holds_alternative<Activating>(mActive))
    {
        // The dataset was never visible, so no attribute changes.
        mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
        CompleteActivation(CHIP_ERROR_CANCELLED);
    }
    else if (std::holds_alternative<ActiveUncommitted>(mActive))
    {
        mActive = NoActiveDataset{};
        ReportAttributeChange(ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        ReportAttributeChange(ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id);
    }
    // ActiveCommitted and NoActiveDataset: nothing to revert.
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedThreadBorderRouter::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::SetPendingDataset called");
    Thread::OperationalDataset tempDataset;
    ReturnErrorOnFailure(tempDataset.Init(pendingDataset.AsByteSpan()));
    uint32_t delayTimerMillis;
    ReturnErrorOnFailure(tempDataset.GetDelayTimer(delayTimerMillis));

    mTimerDelegate.CancelTimer(&mPendingDatasetTimerContext);
    CHIP_ERROR err = mTimerDelegate.StartTimer(&mPendingDatasetTimerContext, System::Clock::Milliseconds32(delayTimerMillis));
    if (err != CHIP_NO_ERROR)
    {
        mPendingDataset.Clear();
        ReportAttributeChange(ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
        return err;
    }

    mPendingDataset = tempDataset;
    ReportAttributeChange(ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    return CHIP_NO_ERROR;
}

void SimulatedThreadBorderRouter::OnActiveDatasetTimerFired()
{
    auto * activating = std::get_if<Activating>(&mActive);
    VerifyOrReturn(activating != nullptr);

    auto * callback = activating->callback;
    auto sequence   = activating->sequence;
    mActive         = ActiveUncommitted{ activating->dataset };
    ReportAttributeChange(ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
    ReportAttributeChange(ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id);

    if (callback != nullptr)
    {
        callback->OnActivateDatasetComplete(sequence, CHIP_NO_ERROR);
    }
}

void SimulatedThreadBorderRouter::OnPendingDatasetTimerFired()
{
    ActiveCommitted committed;
    CHIP_ERROR err = committed.dataset.Init(mPendingDataset.AsByteSpan());
    mPendingDataset.Clear();
    ReportAttributeChange(ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    if (err != CHIP_NO_ERROR)
    {
        LogErrorOnFailure(err);
        return;
    }

    // A pending dataset replaces any activation still in progress.
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    CompleteActivation(CHIP_ERROR_CANCELLED);

    mActive = committed;
    ReportAttributeChange(ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
    ReportAttributeChange(ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id);
}

const Thread::OperationalDataset * SimulatedThreadBorderRouter::ActiveDataset() const
{
    if (auto * uncommitted = std::get_if<ActiveUncommitted>(&mActive))
    {
        return &uncommitted->dataset;
    }
    if (auto * committed = std::get_if<ActiveCommitted>(&mActive))
    {
        return &committed->dataset;
    }
    return nullptr;
}

void SimulatedThreadBorderRouter::CompleteActivation(CHIP_ERROR error)
{
    auto * activating = std::get_if<Activating>(&mActive);
    VerifyOrReturn(activating != nullptr);

    auto * callback = activating->callback;
    auto sequence   = activating->sequence;
    mActive         = NoActiveDataset{};
    if (callback != nullptr)
    {
        callback->OnActivateDatasetComplete(sequence, error);
    }
}

void SimulatedThreadBorderRouter::ReportAttributeChange(AttributeId attributeId)
{
    if (mAttributeChangeCallback != nullptr)
    {
        mAttributeChangeCallback->ReportAttributeChanged(attributeId);
    }
}

} // namespace app
} // namespace chip
