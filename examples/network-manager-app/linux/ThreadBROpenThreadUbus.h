/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include "UbusManager.h"
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>

struct blob_attr;

namespace chip {

class OpenThreadUbusBorderRouterDelegate final : public app::Clusters::ThreadBorderRouterManagement::Delegate
{
public:
    OpenThreadUbusBorderRouterDelegate(ubus::UbusManager & ubusManager) : mUbusManager(ubusManager) {}

    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override;

    void GetBorderRouterName(MutableCharSpan & borderRouterName) override;
    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override;
    uint16_t GetThreadVersion() override;
    bool GetInterfaceEnabled() override;
    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override;
    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                          ActivateDatasetCallback * callback) override;

    // otbr implements MGMT_PENDING_SET via its set_pending method, so a running
    // network can be migrated rather than only formed.
    bool GetPanChangeSupported() override { return true; }
    CHIP_ERROR CommitActiveDataset() override
    {
        mActivationPending = false;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR RevertActiveDataset() override;
    CHIP_ERROR SetPendingDataset(const chip::Thread::OperationalDataset & pendingDataset) override;

private:
    void OnDataReceived(blob_attr * msg, bool notification);
    CHIP_ERROR SubmitDeprovision();
    void ResyncFromOtbr();

    // Invokes an otbr method that takes a hex encoded dataset argument.
    CHIP_ERROR InvokeWithDataset(const char * method, const Thread::OperationalDataset & dataset);

    AttributeChangeCallback * mAttributeChangeCallback;

    ubus::UbusManager & mUbusManager;
    ubus::UbusWatch mOtbr{ "otbr", this };

    bool mBorderAgentIDValid = false;
    uint8_t mBorderAgentID[app::Clusters::ThreadBorderRouterManagement::kBorderAgentIdLength];

    Thread::OperationalDataset mActiveDataset;
    Thread::OperationalDataset mPendingDataset;
    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence;
    // An activation that has not been committed yet. Only such an activation
    // may be reverted: the fail-safe expiry handler reverts unconditionally,
    // including for fail-safes that never touched the dataset, and reverting
    // then would wipe a network provisioned outside Matter.
    bool mActivationPending = false;
    // A revert whose deprovision could not be delivered. The fail-safe fires
    // once, so without this the dataset would stay on the router forever if
    // otbr happened to be away at that moment; retried when it comes back.
    bool mRevertPending = false;
};

} // namespace chip
