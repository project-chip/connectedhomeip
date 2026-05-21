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

#pragma once

/**
 * @file LoggingBorderRouterDelegate.h
 * @brief Reference/mock implementation of ThreadBorderRouterManagementDelegate.
 *
 * This file implements a mock Thread Border Router Management delegate designed for
 * reference and testing purposes inside the all-devices-app. It simulates typical border router
 * behaviors (active/pending datasets in-memory storage, fake border agent ID, name, version) and
 * prints verbose progress logging for every interface function called.
 *
 * To keep the NIM device common implementation lightweight and simple, this reference delegate
 * is implemented header-only (inline). Platforms (like POSIX) can specialize it by inheriting from
 * this class and overriding specific virtual methods, or they can inject their own real platform
 * delegate from scratch when constructing the NimDevice.
 */

#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <clusters/ThreadBorderRouterManagement/Attributes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {

class LoggingBorderRouterDelegate : public Clusters::ThreadBorderRouterManagementDelegate
{
public:
    ~LoggingBorderRouterDelegate() override
    {
        DeviceLayer::SystemLayer().CancelTimer(ActivateActiveDataset, this);
        DeviceLayer::SystemLayer().CancelTimer(ActivatePendingDataset, this);
    }

    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::Init called");
        mAttributeChangeCallback = attributeChangeCallback;
        return CHIP_NO_ERROR;
    }

    bool GetPanChangeSupported() override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::GetPanChangeSupported called");
        return true;
    }

    void GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::GetBorderRouterName called");
        TEMPORARY_RETURN_IGNORED CopyCharSpanToMutableCharSpan("all-devices-br"_span, borderRouterName);
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::GetBorderAgentId called");
        static constexpr uint8_t kBorderAgentId[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                      0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
        VerifyOrReturnError(borderAgentId.size() == Clusters::ThreadBorderRouterManagementDelegate::kBorderAgentIdLength,
                            CHIP_ERROR_INVALID_ARGUMENT);
        return CopySpanToMutableSpan(ByteSpan(kBorderAgentId), borderAgentId);
    }

    uint16_t GetThreadVersion() override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::GetThreadVersion called");
        return /* Thread 1.3.1 */ 5;
    }

    bool GetInterfaceEnabled() override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::GetInterfaceEnabled called");
        return !mActiveDataset.IsEmpty();
    }

    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::GetDataset called (type: %d)", static_cast<int>(type));
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
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::SetActiveDataset called (seq: %" PRIu32 ")", sequenceNum);
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
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(1000), ActivateActiveDataset, this);
    }

    CHIP_ERROR CommitActiveDataset() override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::CommitActiveDataset called");
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR RevertActiveDataset() override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::RevertActiveDataset called");
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override
    {
        ChipLogProgress(AppServer, "LoggingBorderRouterDelegate::SetPendingDataset called");
        ReturnErrorOnFailure(mPendingDataset.Init(pendingDataset.AsByteSpan()));
        uint32_t delayTimerMillis;
        ReturnErrorOnFailure(mPendingDataset.GetDelayTimer(delayTimerMillis));
        return DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(delayTimerMillis), ActivatePendingDataset, this);
    }

private:
    static void ActivateActiveDataset(System::Layer *, void * context)
    {
        auto * self                    = static_cast<LoggingBorderRouterDelegate *>(context);
        auto * callback                = self->mActivateDatasetCallback;
        auto sequenceNum               = self->mActivateDatasetSequence;
        self->mActivateDatasetCallback = nullptr;
        callback->OnActivateDatasetComplete(sequenceNum, CHIP_NO_ERROR);
    }

    static void ActivatePendingDataset(System::Layer *, void * context)
    {
        auto * self = static_cast<LoggingBorderRouterDelegate *>(context);
        TEMPORARY_RETURN_IGNORED self->mActiveDataset.Init(self->mPendingDataset.AsByteSpan());
        self->mPendingDataset.Clear();
        if (self->mAttributeChangeCallback)
        {
            self->mAttributeChangeCallback->ReportAttributeChanged(
                Clusters::ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
            self->mAttributeChangeCallback->ReportAttributeChanged(
                Clusters::ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
        }
    }

    AttributeChangeCallback * mAttributeChangeCallback = nullptr;
    Thread::OperationalDataset mActiveDataset;
    Thread::OperationalDataset mPendingDataset;

    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence;
};

} // namespace app
} // namespace chip
