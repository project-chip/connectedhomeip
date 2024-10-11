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

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
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
        // The error input of this function could be SDK-range error for CHIP error or OpenThread-range error for Thread error.
        virtual void OnActivateDatasetComplete(uint32_t sequenceNum, CHIP_ERROR error) = 0;
    };

    class AttributeChangeCallback
    {
    public:
        AttributeChangeCallback()          = default;
        virtual ~AttributeChangeCallback() = default;
        // If the attributes of the Thread Border Router Management is changed, ReportAttributeChanged should be called.
        virtual void ReportAttributeChanged(AttributeId attributeId) = 0;
    };

    enum class DatasetType : uint8_t
    {
        kActive,
        kPending,
    };

    virtual CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) = 0;

    // Get whether PanChange feature is supported for the Thread BR.
    virtual bool GetPanChangeSupported() = 0;

    // Get the BorderRouterName of the Thread BR, which will also be the service name of Thread BR's MeshCOP service.
    virtual void GetBorderRouterName(MutableCharSpan & borderRouterName) = 0;

    // Get the BorderAgentId of the Thread BR.
    // @return
    //   -IncorrectState When Thread stack is not initialized.
    //   -InvalidArgument When the size of borderAgentId is not 16 bytes.
    //   -ThreadErrors When failing to get BorderAgentId.
    virtual CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) = 0;

    // Get the Thread version which matches the value mapping defined in the "Version TLV" section of the Thread specification.
    virtual uint16_t GetThreadVersion() = 0;

    // Get whether the associated IEEE 802.15.4 Thread interface is enabled or disabled.
    virtual bool GetInterfaceEnabled() = 0;

    // Get the active dataset or the pending dataset.
    // @return
    //   -IncorrectState When Thread stack is not initialized.
    //   -NotFound when failing to get the dataset.
    virtual CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) = 0;

    // There should be no active dataset configured when calling this API, otherwise we should use SetPendingDataset.
    // The Delegate implementation must store the sequence number and pass it to OnActivateDatasetComplete.
    virtual void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                  ActivateDatasetCallback * callback) = 0;

    // This function will check save whether there is active dataset configured.
    virtual CHIP_ERROR CommitActiveDataset() = 0;

    // The function is called when Failsafe timer is triggered or when the Border Router reboots with a previous Failsafe timer
    // started but not disarmed before reboot. The delegate implementation should check whether there is a previous SetActiveDataset
    // request and revert the active dataset set by the previous SetActiveDataset. Since there should be no configured dataset when
    // calling SetActiveDataset, this function will clear the active dataset to allow trying again a new SetActiveDataset operation.
    // The delegate is allowed to call OnActivateDatasetComplete for the previous SetActiveDataset request even after this function
    // is called as the sequence number passed to OnActivateDatasetComplete will be different.
    virtual CHIP_ERROR RevertActiveDataset() = 0;

    virtual CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) = 0;
};

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
