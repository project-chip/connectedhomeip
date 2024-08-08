/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/thread-border-router-management-server/thread-border-router-management-server.h>
#include <app/server/Server.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>

#include <optional>

using namespace chip;
using namespace chip::literals;
using namespace chip::app;
using namespace chip::app::Clusters;

namespace {
class FakeBorderRouterDelegate final : public ThreadBorderRouterManagement::Delegate
{
    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override
    {
        mAttributeChangeCallback = attributeChangeCallback;
        return CHIP_NO_ERROR;
    }

    bool GetPanChangeSupported() override { return true; }

    void GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        CopyCharSpanToMutableCharSpan("netman-br"_span, borderRouterName);
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
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(1000), ActivateActiveDataset, this);
    }

    CHIP_ERROR CommitActiveDataset() override { return CHIP_NO_ERROR; }
    CHIP_ERROR RevertActiveDataset() override { return CHIP_ERROR_NOT_IMPLEMENTED; }

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override
    {
        ReturnErrorOnFailure(mPendingDataset.Init(pendingDataset.AsByteSpan()));
        uint32_t delayTimerMillis;
        ReturnErrorOnFailure(mPendingDataset.GetDelayTimer(delayTimerMillis));
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(delayTimerMillis), ActivatePendingDataset, this);
        return CHIP_NO_ERROR;
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
        self->mActiveDataset.Init(self->mPendingDataset.AsByteSpan());
        self->mPendingDataset.Clear();
        // This could just call MatterReportingAttributeChangeCallback directly
        self->mAttributeChangeCallback->ReportAttributeChanged(
            ThreadBorderRouterManagement::Attributes::ActiveDatasetTimestamp::Id);
        self->mAttributeChangeCallback->ReportAttributeChanged(
            ThreadBorderRouterManagement::Attributes::PendingDatasetTimestamp::Id);
    }

    AttributeChangeCallback * mAttributeChangeCallback;
    Thread::OperationalDataset mActiveDataset;
    Thread::OperationalDataset mPendingDataset;

    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence;
};

FakeBorderRouterDelegate gBorderRouterDelegate{};
} // namespace

std::optional<ThreadBorderRouterManagement::ServerInstance> gThreadBorderRouterManagementServer;
void emberAfThreadBorderRouterManagementClusterInitCallback(EndpointId endpoint)
{
    VerifyOrDie(!gThreadBorderRouterManagementServer);
    gThreadBorderRouterManagementServer.emplace(endpoint, &gBorderRouterDelegate, Server::GetInstance().GetFailSafeContext())
        .Init();
}
