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
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <cstdint>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ThreadBorderRouterManagement {

class GenericOpenThreadBorderRouterDelegate : public Delegate
{
public:
    static constexpr char kFailsafeActiveDatasetConfigured[] = "g/fs/tbradc";
    GenericOpenThreadBorderRouterDelegate(PersistentStorageDelegate * storage) : mStorage(storage) {}
    ~GenericOpenThreadBorderRouterDelegate() = default;

    CHIP_ERROR Init(AttributeChangeCallback * callback) override;

    bool GetPanChangeSupported() override { return true; }

    void GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        CopyCharSpanToMutableCharSpan(CharSpan(mThreadBorderRouterName, strlen(mThreadBorderRouterName)), borderRouterName);
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override;

    uint16_t GetThreadVersion() override;

    bool GetInterfaceEnabled() override;

    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override;

    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                          ActivateDatasetCallback * callback) override;

    CHIP_ERROR CommitActiveDataset() override;

    CHIP_ERROR RevertActiveDataset() override;

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override;

    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

    void SetThreadBorderRouterName(const CharSpan & name)
    {
        MutableCharSpan borderRouterName(mThreadBorderRouterName);
        CopyCharSpanToMutableCharSpan(name, borderRouterName);
        if (mpAttributeChangeCallback)
        {
            DeviceLayer::SystemLayer().ScheduleLambda(
                [this]() { mpAttributeChangeCallback->ReportAttributeChanged(Attributes::BorderRouterName::Id); });
        }
    }

    void NotifyBorderAgentIdChanged()
    {
        if (mpAttributeChangeCallback)
        {
            // OpenThread doesn't have callback or event for BorderAgentId change, we can only change the BorderAgentId with
            // otBorderAgentSetId(). Please call this function with otBorderAgentSetId().
            DeviceLayer::SystemLayer().ScheduleLambda(
                [this]() { mpAttributeChangeCallback->ReportAttributeChanged(Attributes::BorderAgentID::Id); });
        }
    }

private:
    CHIP_ERROR SaveActiveDatasetConfigured(bool configured);
    ActivateDatasetCallback * mpActivateDatasetCallback = nullptr;
    uint32_t mSequenceNum                               = 0;
    char mThreadBorderRouterName[kBorderRouterNameMaxLength + 1];
    PersistentStorageDelegate * mStorage;
    AttributeChangeCallback * mpAttributeChangeCallback = nullptr;
};
} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
