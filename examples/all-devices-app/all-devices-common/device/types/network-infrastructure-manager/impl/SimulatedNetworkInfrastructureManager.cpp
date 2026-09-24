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

#include <device/types/network-infrastructure-manager/impl/SimulatedNetworkInfrastructureManager.h>

#include <clusters/ThreadBorderRouterManagement/Attributes.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

namespace {
constexpr uint16_t kThreadVersionForThread_1_3_1 = 5;
} // namespace

SimulatedNetworkInfrastructureManager::SimulatedNetworkInfrastructureManager(const Context & context) :
    NetworkInfrastructureManager(NetworkInfrastructureManager::Context{
        .delegate            = *this,
        .failSafeContext     = context.failSafeContext,
        .platformManager     = context.platformManager,
        .storage             = context.storage,
        .breadcrumbTracker   = *this,
        .diagnosticsProvider = *this,
    }),
    mTimerDelegate(context.timerDelegate), mBorderRouterName(context.nodeLabel)
{}

SimulatedNetworkInfrastructureManager::~SimulatedNetworkInfrastructureManager()
{
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    mTimerDelegate.CancelTimer(&mPendingDatasetTimerContext);
}

void SimulatedNetworkInfrastructureManager::Unregister(CodeDrivenDataModelProvider & provider)
{
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    mTimerDelegate.CancelTimer(&mPendingDatasetTimerContext);
    CompleteActivation(CHIP_ERROR_CANCELLED);
    mActive                  = NoActiveDataset{};
    mAttributeChangeCallback = nullptr;
    mPendingDataset.Clear();
    NetworkInfrastructureManager::Unregister(provider);
}

CHIP_ERROR SimulatedNetworkInfrastructureManager::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                           EndpointComposition composition)
{
    ReturnErrorOnFailure(NetworkInfrastructureManager::Register(endpoint, provider, composition));

    ReturnErrorOnFailure(
        SetWiFiNetworkCredentials(ByteSpan::fromCharSpan("MatterAP"_span), ByteSpan::fromCharSpan("Setec Astronomy"_span)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedNetworkInfrastructureManager::Init(AttributeChangeCallback * attributeChangeCallback)
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::Init called");
    mAttributeChangeCallback = attributeChangeCallback;
    return CHIP_NO_ERROR;
}

bool SimulatedNetworkInfrastructureManager::GetPanChangeSupported()
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::GetPanChangeSupported called");
    return true;
}

void SimulatedNetworkInfrastructureManager::GetBorderRouterName(MutableCharSpan & borderRouterName)
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::GetBorderRouterName called");
    if (!mBorderRouterName.empty())
    {
        CopyCharSpanToMutableCharSpanWithTruncation(CharSpan::fromCharString(mBorderRouterName.c_str()), borderRouterName);
    }
    else
    {
        CopyCharSpanToMutableCharSpanWithTruncation("all-devices-br"_span, borderRouterName);
    }
}

CHIP_ERROR SimulatedNetworkInfrastructureManager::GetBorderAgentId(MutableByteSpan & borderAgentId)
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::GetBorderAgentId called");
    static constexpr uint8_t kBorderAgentId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                  0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
    VerifyOrReturnError(borderAgentId.size() == Clusters::ThreadBorderRouterManagementDelegate::kBorderAgentIdLength,
                        CHIP_ERROR_INVALID_ARGUMENT);
    return CopySpanToMutableSpan(ByteSpan(kBorderAgentId), borderAgentId);
}

uint16_t SimulatedNetworkInfrastructureManager::GetThreadVersion()
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::GetThreadVersion called");
    return kThreadVersionForThread_1_3_1;
}

bool SimulatedNetworkInfrastructureManager::GetInterfaceEnabled()
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::GetInterfaceEnabled called");
    return ActiveDataset() != nullptr;
}

CHIP_ERROR SimulatedNetworkInfrastructureManager::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::GetDataset called (type: %d)", static_cast<int>(type));
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

void SimulatedNetworkInfrastructureManager::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                   ActivateDatasetCallback * callback)
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::SetActiveDataset called (seq: %" PRIu32 ")", sequenceNum);
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

CHIP_ERROR SimulatedNetworkInfrastructureManager::CommitActiveDataset()
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::CommitActiveDataset called");
    if (auto * uncommitted = std::get_if<ActiveUncommitted>(&mActive))
    {
        mActive = ActiveCommitted{ uncommitted->dataset };
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedNetworkInfrastructureManager::RevertActiveDataset()
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::RevertActiveDataset called");
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

CHIP_ERROR SimulatedNetworkInfrastructureManager::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    ChipLogProgress(AppServer, "SimulatedNetworkInfrastructureManager::SetPendingDataset called");
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

void SimulatedNetworkInfrastructureManager::OnActiveDatasetTimerFired()
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

void SimulatedNetworkInfrastructureManager::OnPendingDatasetTimerFired()
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

const Thread::OperationalDataset * SimulatedNetworkInfrastructureManager::ActiveDataset() const
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

void SimulatedNetworkInfrastructureManager::CompleteActivation(CHIP_ERROR error)
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

void SimulatedNetworkInfrastructureManager::ReportAttributeChange(AttributeId attributeId)
{
    if (mAttributeChangeCallback != nullptr)
    {
        mAttributeChangeCallback->ReportAttributeChanged(attributeId);
    }
}

} // namespace app
} // namespace chip
