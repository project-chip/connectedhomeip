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
                                                                   uint32_t randomNumber, ActivateDatasetCallback * callback)
{
    CHIP_ERROR err = BackupActiveDataset();
    if (err == CHIP_NO_ERROR)
    {
        err = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(activeDataset, nullptr);
    }
    if (err == CHIP_NO_ERROR)
    {
        mRandomNumber = randomNumber;
        mCallback     = callback;
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
            delegate->mCallback->OnActivateDatasetComplete(delegate->mRandomNumber, CHIP_NO_ERROR);
            // Delete Failsafe Keys after activating dataset is completed
            DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadDatasetTlvsKey);
            delegate->mCallback = nullptr;
        }
    }
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::BackupActiveDataset()
{
    // If active dataset is already backed up, return with no error
    CHIP_ERROR err = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kFailsafeThreadDatasetTlvsKey, nullptr, 0);
    if (err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        return CHIP_NO_ERROR;
    }
    GetDataset(mStagingDataset, DatasetType::kActive);
    ByteSpan dataset = mStagingDataset.AsByteSpan();
    return DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kFailsafeThreadDatasetTlvsKey, dataset.data(), dataset.size());
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::CommitActiveDataset()
{
    // Delete Failsafe Key when committing.
    DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadDatasetTlvsKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::RevertActiveDataset()
{
    // The FailSafe Timer is triggered and the previous command request should be handled, so reset the callback.
    mCallback = nullptr;
    uint8_t datasetBytes[Thread::kSizeOperationalDataset];
    size_t datasetLength;
    CHIP_ERROR err = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kFailsafeThreadDatasetTlvsKey, datasetBytes,
                                                                           sizeof(datasetBytes), &datasetLength);
    // If no backup could be found, it means the active datset has not been modified since the fail-safe was armed,
    // so return with no error.
    ReturnErrorCodeIf(err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);
    if (err == CHIP_NO_ERROR)
    {
        err = mStagingDataset.Init(ByteSpan(datasetBytes, datasetLength));
    }
    if (err == CHIP_NO_ERROR)
    {
        err = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(mStagingDataset, nullptr);
    }

    // Always delete the backup, regardless if it can be successfully restored.
    DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadDatasetTlvsKey);
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
