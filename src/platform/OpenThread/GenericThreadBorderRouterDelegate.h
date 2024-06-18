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
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ThreadBorderRouterManagement {

class GenericThreadBorderRouterDelegate : public Delegate
{
public:
    static constexpr char kThreadBorderRourterName[]      = "Espressif-ThreadBR";
    static constexpr char kFailsafeThreadDatasetTlvsKey[] = "g/fs/td";
    static constexpr char kFailsafeThreadEnabledKey[]     = "g/fs/te";

    GenericThreadBorderRouterDelegate()  = default;
    ~GenericThreadBorderRouterDelegate() = default;

    CHIP_ERROR Init() override;

    CHIP_ERROR GetPanChangeSupported(bool & panChangeSupported) override
    {
        panChangeSupported = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetBorderRouterName(MutableCharSpan & borderRouterName) override
    {
        if (borderRouterName.size() < strlen(kThreadBorderRourterName))
        {
            return CHIP_ERROR_NO_MEMORY;
        }
        strcpy(borderRouterName.data(), kThreadBorderRourterName);
        borderRouterName.reduce_size(strlen(kThreadBorderRourterName));
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override;

    CHIP_ERROR GetThreadVersion(uint16_t & threadVersion) override;

    CHIP_ERROR GetInterfaceEnabled(bool & interfaceEnabled) override;

    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override;

    CHIP_ERROR SetActiveDataset(const Thread::OperationalDataset & activeDataset, ActivateDatasetCallback * callback) override;

    CHIP_ERROR RevertActiveDataset() override;

    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override;

    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);

private:
    CHIP_ERROR SetThreadEnabled(bool enabled);
    bool GetThreadEnabled();
    ActivateDatasetCallback * mCallback = nullptr;
};
} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
