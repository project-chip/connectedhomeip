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

SimulatedThreadBorderRouter::SimulatedThreadBorderRouter(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage,
                                                         DeviceLayer::PlatformManager & platformManager,
                                                         FailSafeContext & failSafeContext,
                                                         Clusters::BreadCrumbTracker * breadcrumbTracker) :
    ThreadBorderRouter(ThreadBorderRouter::Context{
        .delegate          = *this,
        .failSafeContext   = failSafeContext,
        .platformManager   = platformManager,
        .storage           = storage,
        .breadcrumbTracker = breadcrumbTracker,
    }),
    mTimerDelegate(timerDelegate)
{}

SimulatedThreadBorderRouter::SimulatedThreadBorderRouter(const Context & context) :
    SimulatedThreadBorderRouter(context.timerDelegate, context.storage, context.platformManager, context.failSafeContext,
                                context.breadcrumbTracker)
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
    mActivateDatasetCallback = nullptr;
    mAttributeChangeCallback = nullptr;

    ThreadBorderRouter::Unregister(provider);
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
    CopyCharSpanToMutableCharSpanWithTruncation("all-devices-tbr"_span, borderRouterName);
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
    return !mActiveDataset.IsEmpty();
}

CHIP_ERROR SimulatedThreadBorderRouter::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::GetDataset called (type: %d)", static_cast<int>(type));
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

void SimulatedThreadBorderRouter::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                   ActivateDatasetCallback * callback)
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::SetActiveDataset called (seq: %" PRIu32 ")", sequenceNum);
    if (mActivateDatasetCallback != nullptr)
    {
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    Thread::OperationalDataset tempDataset;
    CHIP_ERROR err = tempDataset.Init(activeDataset.AsByteSpan());
    if (err != CHIP_NO_ERROR)
    {
        callback->OnActivateDatasetComplete(sequenceNum, err);
        return;
    }

    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    err = mTimerDelegate.StartTimer(&mActiveDatasetTimerContext, System::Clock::Seconds32(1));
    if (err != CHIP_NO_ERROR)
    {
        callback->OnActivateDatasetComplete(sequenceNum, err);
        return;
    }

    mStagedActiveDataset     = tempDataset;
    mActivateDatasetCallback = callback;
    mActivateDatasetSequence = sequenceNum;
}

CHIP_ERROR SimulatedThreadBorderRouter::CommitActiveDataset()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::CommitActiveDataset called");
    mStagedActiveDataset.Clear();
    return CHIP_NO_ERROR;
}

CHIP_ERROR SimulatedThreadBorderRouter::RevertActiveDataset()
{
    ChipLogProgress(AppServer, "SimulatedThreadBorderRouter::RevertActiveDataset called");
    mTimerDelegate.CancelTimer(&mActiveDatasetTimerContext);
    mActivateDatasetCallback = nullptr;
    mStagedActiveDataset.Clear();
    mActiveDataset.Clear();

    if (mAttributeChangeCallback != nullptr)
    {
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id);
    }
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
    ReturnErrorOnFailure(mTimerDelegate.StartTimer(&mPendingDatasetTimerContext, System::Clock::Milliseconds32(delayTimerMillis)));

    mPendingDataset = tempDataset;
    if (mAttributeChangeCallback != nullptr)
    {
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    }
    return CHIP_NO_ERROR;
}

void SimulatedThreadBorderRouter::OnActiveDatasetTimerFired()
{
    auto * callback          = mActivateDatasetCallback;
    auto sequenceNum         = mActivateDatasetSequence;
    mActivateDatasetCallback = nullptr;

    mActiveDataset = mStagedActiveDataset;
    if (mAttributeChangeCallback != nullptr)
    {
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id);
    }

    if (callback != nullptr)
    {
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_NO_ERROR);
    }
}

void SimulatedThreadBorderRouter::OnPendingDatasetTimerFired()
{
    TEMPORARY_RETURN_IGNORED mActiveDataset.Init(mPendingDataset.AsByteSpan());
    mPendingDataset.Clear();
    if (mAttributeChangeCallback != nullptr)
    {
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
        mAttributeChangeCallback->ReportAttributeChanged(ThreadBorderRouterManagement::Attributes::InterfaceEnabled::Id);
    }
}

} // namespace app
} // namespace chip
