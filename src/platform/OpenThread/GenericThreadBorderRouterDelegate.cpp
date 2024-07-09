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

#include "GenericThreadBorderRouterDelegate.h"

#include <openthread/border_agent.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/instance.h>
#include <openthread/ip6.h>
#include <openthread/link.h>
#include <openthread/netdiag.h>
#include <openthread/thread.h>
#include <openthread/thread_ftd.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/thread-border-router-management-server/thread-br-delegate.h>
#include <inet/IPAddress.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>
#include <platform/CHIPDeviceEvent.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/PlatformManager.h>
#include <platform/ThreadStackManager.h>

namespace chip {
namespace app {
namespace Clusters {

namespace ThreadBorderRouterManagement {

class ScopedThreadLock
{
public:
    ScopedThreadLock() { DeviceLayer::ThreadStackMgr().LockThreadStack(); }
    ~ScopedThreadLock() { DeviceLayer::ThreadStackMgr().UnlockThreadStack(); }
};

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::Init()
{
    mCallback = nullptr;
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    // When the Thread Border Router is reboot during SetActiveDataset, we need to revert the active dateset.
    ReturnErrorOnFailure(RevertActiveDataset());
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::GetBorderAgentId(MutableByteSpan & borderAgentIdSpan)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);
    otBorderAgentId borderAgentId;
    if (borderAgentIdSpan.size() < sizeof(borderAgentId.mId))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ScopedThreadLock threadLock;
    otError err = otBorderAgentGetId(otInst, &borderAgentId);
    if (err == OT_ERROR_NONE)
    {
        memcpy(borderAgentIdSpan.data(), borderAgentId.mId, sizeof(borderAgentId.mId));
        borderAgentIdSpan.reduce_size(sizeof(borderAgentId.mId));
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::GetThreadVersion(uint16_t & threadVersion)
{
    threadVersion = otThreadGetVersion();
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::GetInterfaceEnabled(bool & interfaceEnabled)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);
    ScopedThreadLock threadLock;
    interfaceEnabled = otIp6IsEnabled(otInst);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);

    ScopedThreadLock threadLock;
    otError otErr = OT_ERROR_NONE;
    otOperationalDatasetTlvs datasetTlvs;
    if (type == DatasetType::kActive)
    {
        otErr = otDatasetGetActiveTlvs(otInst, &datasetTlvs);
    }
    else
    {
        otErr = otDatasetGetPendingTlvs(otInst, &datasetTlvs);
    }
    if (otErr == OT_ERROR_NONE)
    {
        return dataset.Init(ByteSpan(datasetTlvs.mTlvs, datasetTlvs.mLength));
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::SetActiveDataset(const Thread::OperationalDataset & activeDataset,
                                                                   uint32_t sequenceNum, ActivateDatasetCallback * callback)
{
    // This function will never be invoked when there is an Active Dataset already configured.
    CHIP_ERROR err = SaveThreadBorderRouterCommissioned(false);
    if (err == CHIP_NO_ERROR)
    {
        err = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(activeDataset, nullptr);
    }
    if (err == CHIP_NO_ERROR)
    {
        // We can change the stored sequence number because OnActivateDatasetComplete executed later must be called for this
        // SetActiveDataset request
        mSequenceNum = sequenceNum;
        mCallback    = callback;
    }
    return err;
}

void GenericOpenThreadBorderRouterDelegate::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    GenericOpenThreadBorderRouterDelegate * delegate = reinterpret_cast<GenericOpenThreadBorderRouterDelegate *>(arg);
    if (delegate && delegate->mCallback)
    {
        if (event->Type == DeviceLayer::DeviceEventType::kThreadConnectivityChange &&
            event->ThreadConnectivityChange.Result == DeviceLayer::kConnectivity_Established)
        {
            delegate->mCallback->OnActivateDatasetComplete(delegate->mSequenceNum, CHIP_NO_ERROR);
            SaveThreadBorderRouterCommissioned(true);
            delegate->mCallback = nullptr;
        }
    }
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::SaveThreadBorderRouterCommissioned(bool commissioned)
{
    return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kFailsafeThreadBorderRouterCommissioned, commissioned);
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::RevertActiveDataset()
{
    // The FailSafe Timer is triggered and the previous command request should be handled, so reset the callback.
    mCallback               = nullptr;
    bool threadCommissioned = true;
    DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kFailsafeThreadBorderRouterCommissioned, &threadCommissioned);
    if (!threadCommissioned)
    {
        // If Thread is not commissioned, we will try to attach an empty Thread dataset and that will clear the one stored in the
        // Thread stack since the SetActiveDataset operation fails and FailSafe timer is triggered.
        Thread::OperationalDataset emptyDataset = {};
        return DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(emptyDataset, nullptr);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);

    ScopedThreadLock threadLock;
    otOperationalDatasetTlvs datasetTlvs;
    memcpy(datasetTlvs.mTlvs, pendingDataset.AsByteSpan().data(), pendingDataset.AsByteSpan().size());
    datasetTlvs.mLength = pendingDataset.AsByteSpan().size();
    ReturnErrorCodeIf(otDatasetSetPendingTlvs(otInst, &datasetTlvs) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
