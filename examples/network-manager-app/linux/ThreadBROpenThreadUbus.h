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

    bool GetPanChangeSupported() override { return false; }
    CHIP_ERROR CommitActiveDataset() override { return CHIP_NO_ERROR; }
    CHIP_ERROR RevertActiveDataset() override { return CHIP_ERROR_NOT_IMPLEMENTED; }
    CHIP_ERROR SetPendingDataset(const chip::Thread::OperationalDataset &) override { return CHIP_ERROR_NOT_IMPLEMENTED; }

private:
    void OnDataReceived(blob_attr * msg, bool notification);

    AttributeChangeCallback * mAttributeChangeCallback;

    ubus::UbusManager & mUbusManager;
    ubus::UbusWatch mOtbr{ "otbr", this };

    bool mBorderAgentIDValid = false;
    uint8_t mBorderAgentID[app::Clusters::ThreadBorderRouterManagement::kBorderAgentIdLength];

    Thread::OperationalDataset mActiveDataset;
    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence;
};

} // namespace chip
