/*
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <clusters/ThreadBorderRouterManagement/Attributes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {

class FakeBorderRouterDelegate final : public app::Clusters::ThreadBorderRouterManagement::Delegate
{
    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override
    {
        mAttributeChangeCallback = attributeChangeCallback;
        return CHIP_NO_ERROR;
    }

    bool GetPanChangeSupported() override { return true; }

    void GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        TEMPORARY_RETURN_IGNORED CopyCharSpanToMutableCharSpan("netman-br"_span, borderRouterName);
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override
    {
        static constexpr uint8_t kBorderAgentId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
        VerifyOrReturnError(borderAgentId.size() == 16, CHIP_ERROR_INVALID_ARGUMENT);
        return CopySpanToMutableSpan(ByteSpan(kBorderAgentId), borderAgentId);
    }

    uint16_t GetThreadVersion() override { return /* Thread 1.3.1 */ 5; }

    bool GetInterfaceEnabled() override { return !mActiveDataset.IsEmpty(); }

    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override
    {
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

    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                          ActivateDatasetCallback * callback) override
    {
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
        mActivationPending       = true;
        VerifyOrReturn(DeviceLayer::SystemLayer()
                           .StartTimer(System::Clock::Milliseconds32(1000), ActivateActiveDataset, this)
                           .Handle([&](CHIP_ERROR error) {
                               // Without the timer nothing would ever complete this activation;
                               // undo the state so the next attempt is not refused as Busy.
                               mActivateDatasetCallback = nullptr;
                               mActivationPending       = false;
                               mActiveDataset.Clear();
                               callback->OnActivateDatasetComplete(sequenceNum, error);
                           }));
    }

    CHIP_ERROR CommitActiveDataset() override
    {
        mActivationPending = false;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR RevertActiveDataset() override
    {
        // Parity with the ubus delegate: the fail-safe handler calls this for
        // every expiry, and only an activation that was not committed reverts.
        VerifyOrReturnError(mActivationPending, CHIP_NO_ERROR);
        mActivationPending = false;

        // The activation timer may still be armed. A reverted activation must
        // neither report success nor block the next attempt.
        DeviceLayer::SystemLayer().CancelTimer(ActivateActiveDataset, this);
        mActivateDatasetCallback = nullptr;

        // SetActiveDataset is only accepted when no dataset is configured, so
        // reverting it means returning to the unconfigured state.
        mActiveDataset.Clear();
        mAttributeChangeCallback->ReportAttributeChanged(
            app::Clusters::ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override
    {
        ReturnErrorOnFailure(mPendingDataset.Init(pendingDataset.AsByteSpan()));
        uint32_t delayTimerMillis;
        ReturnErrorOnFailure(mPendingDataset.GetDelayTimer(delayTimerMillis));
        return DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(delayTimerMillis), ActivatePendingDataset, this);
    }

private:
    static void ActivateActiveDataset(System::Layer *, void * context)
    {
        auto * self                    = static_cast<FakeBorderRouterDelegate *>(context);
        auto * callback                = self->mActivateDatasetCallback;
        auto sequenceNum               = self->mActivateDatasetSequence;
        self->mActivateDatasetCallback = nullptr;
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_NO_ERROR);
    }

    static void ActivatePendingDataset(System::Layer *, void * context)
    {
        auto * self = static_cast<FakeBorderRouterDelegate *>(context);
        TEMPORARY_RETURN_IGNORED self->mActiveDataset.Init(self->mPendingDataset.AsByteSpan());
        TEMPORARY_RETURN_IGNORED self->mPendingDataset.Clear();
        // This could just call MatterReportingAttributeChangeCallback directly
        self->mAttributeChangeCallback->ReportAttributeChanged(
            app::Clusters::ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        self->mAttributeChangeCallback->ReportAttributeChanged(
            app::Clusters::ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    }

    AttributeChangeCallback * mAttributeChangeCallback;
    Thread::OperationalDataset mActiveDataset;
    Thread::OperationalDataset mPendingDataset;

    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence;
    bool mActivationPending = false;
};

} // namespace chip
