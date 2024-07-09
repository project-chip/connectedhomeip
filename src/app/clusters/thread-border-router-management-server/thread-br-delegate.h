/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-enums.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/core/Optional.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/Span.h>
#include <lib/support/ThreadOperationalDataset.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

constexpr size_t kBorderRouterNameMaxLength = 63;
constexpr size_t kBorderAgentIdLength       = 16;

class Delegate
{
public:
    Delegate()          = default;
    virtual ~Delegate() = default;

    class ActivateDatasetCallback
    {
    public:
        ActivateDatasetCallback()          = default;
        virtual ~ActivateDatasetCallback() = default;
        // If the dataset is set successfully, OnActivateDatasetComplete should be called with CHIP_NO_ERROR when the
        // Border Router is attached to the Thread network.
        // If an error occurs while setting the active dataset, this callback should be called with the error.
        virtual void OnActivateDatasetComplete(uint32_t sequenceNum, CHIP_ERROR error) = 0;
    };

    enum class DatasetType : uint8_t
    {
        kActive,
        kPending,
    };

    virtual CHIP_ERROR Init() = 0;

    virtual CHIP_ERROR GetPanChangeSupported(bool & panChangeSupported) = 0;

    virtual CHIP_ERROR GetBorderRouterName(MutableCharSpan & borderRouterName) = 0;

    virtual CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) = 0;

    virtual CHIP_ERROR GetThreadVersion(uint16_t & threadVersion) = 0;

    virtual CHIP_ERROR GetInterfaceEnabled(bool & interfaceEnabled) = 0;

    virtual CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) = 0;

    // There should be no active dataset configured when calling this API, otherwise we should use SetPendingDataset.
    // The Delegate implementation must store the sequence number and pass it to OnActivateDatasetComplete.
    virtual CHIP_ERROR SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                        ActivateDatasetCallback * callback) = 0;

    // The function is called when Failsafe timer is triggered and the delegate implementation should clear the active dataset in
    // this function to allow trying again a new SetActiveDataset operation.
    // The delegate is allowed to call OnActivateDatasetComplete for the previous SetActiveDataset request even after this function
    // is called as the sequence number passed to OnActivateDatasetComplete will be different.
    virtual CHIP_ERROR RevertActiveDataset() = 0;

    virtual CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) = 0;
};

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
