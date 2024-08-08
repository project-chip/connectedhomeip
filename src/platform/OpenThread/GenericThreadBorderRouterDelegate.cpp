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
#include <platform/OpenThread/OpenThreadUtils.h>
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

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::Init(AttributeChangeCallback * callback)
{
    mpActivateDatasetCallback = nullptr;
    mpAttributeChangeCallback = callback;
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    // When the Thread Border Router is reboot during SetActiveDataset, we need to revert the active dateset.
    RevertActiveDataset();
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::GetBorderAgentId(MutableByteSpan & borderAgentIdSpan)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);
    otBorderAgentId borderAgentId;
    if (borderAgentIdSpan.size() != sizeof(borderAgentId.mId))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    otError otErr = OT_ERROR_NONE;
    {
        ScopedThreadLock threadLock;
        otErr = otBorderAgentGetId(otInst, &borderAgentId);
    }
    if (otErr == OT_ERROR_NONE)
    {
        CopySpanToMutableSpan(ByteSpan(borderAgentId.mId), borderAgentIdSpan);
        return CHIP_NO_ERROR;
    }
    return DeviceLayer::Internal::MapOpenThreadError(otErr);
}

uint16_t GenericOpenThreadBorderRouterDelegate::GetThreadVersion()
{
    return otThreadGetVersion();
}

bool GenericOpenThreadBorderRouterDelegate::GetInterfaceEnabled()
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnValue(otInst, false);
    ScopedThreadLock threadLock;
    return otIp6IsEnabled(otInst);
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);

    otError otErr = OT_ERROR_NONE;
    otOperationalDatasetTlvs datasetTlvs;
    {
        ScopedThreadLock threadLock;
        if (type == DatasetType::kActive)
        {
            otErr = otDatasetGetActiveTlvs(otInst, &datasetTlvs);
        }
        else
        {
            otErr = otDatasetGetPendingTlvs(otInst, &datasetTlvs);
        }
    }
    if (otErr == OT_ERROR_NONE)
    {
        return dataset.Init(ByteSpan(datasetTlvs.mTlvs, datasetTlvs.mLength));
    }
    return DeviceLayer::Internal::MapOpenThreadError(otErr);
}

void GenericOpenThreadBorderRouterDelegate::SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                                                             ActivateDatasetCallback * callback)
{
    // This function will never be invoked when there is an Active Dataset already configured.
    CHIP_ERROR err = SaveActiveDatasetConfigured(false);
    if (err == CHIP_NO_ERROR)
    {
        err = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(activeDataset, nullptr);
    }
    if (err != CHIP_NO_ERROR)
    {
        callback->OnActivateDatasetComplete(sequenceNum, err);
        return;
    }
    mSequenceNum              = sequenceNum;
    mpActivateDatasetCallback = callback;
}

void GenericOpenThreadBorderRouterDelegate::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    GenericOpenThreadBorderRouterDelegate * delegate = reinterpret_cast<GenericOpenThreadBorderRouterDelegate *>(arg);
    if (delegate)
    {
        if ((event->Type == DeviceLayer::DeviceEventType::kThreadConnectivityChange) &&
            (event->ThreadConnectivityChange.Result == DeviceLayer::kConnectivity_Established) &&
            delegate->mpActivateDatasetCallback)
        {
            delegate->mpActivateDatasetCallback->OnActivateDatasetComplete(delegate->mSequenceNum, CHIP_NO_ERROR);
            delegate->mpActivateDatasetCallback = nullptr;
        }
    }
    if (event->Type == DeviceLayer::DeviceEventType::kThreadStateChange)
    {
        if (event->ThreadStateChange.OpenThread.Flags & OT_CHANGED_THREAD_NETIF_STATE)
        {
            DeviceLayer::SystemLayer().ScheduleLambda(
                [delegate]() { delegate->mpAttributeChangeCallback->ReportAttributeChanged(Attributes::InterfaceEnabled::Id); });
        }
        if (event->ThreadStateChange.OpenThread.Flags & OT_CHANGED_ACTIVE_DATASET)
        {
            DeviceLayer::SystemLayer().ScheduleLambda([delegate]() {
                delegate->mpAttributeChangeCallback->ReportAttributeChanged(Attributes::ActiveDatasetTimestamp::Id);
            });
        }
        if (event->ThreadStateChange.OpenThread.Flags & OT_CHANGED_PENDING_DATASET)
        {
            DeviceLayer::SystemLayer().ScheduleLambda([delegate]() {
                delegate->mpAttributeChangeCallback->ReportAttributeChanged(Attributes::PendingDatasetTimestamp::Id);
            });
        }
    }
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::SaveActiveDatasetConfigured(bool configured)
{
    VerifyOrReturnError(mStorage, CHIP_ERROR_INTERNAL);
    return mStorage->SyncSetKeyValue(kFailsafeActiveDatasetConfigured, &configured, sizeof(bool));
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::CommitActiveDataset()
{
    return SaveActiveDatasetConfigured(DeviceLayer::ThreadStackMgrImpl().IsThreadAttached());
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::RevertActiveDataset()
{
    // The FailSafe Timer is triggered and the previous command request should be handled, so reset the callback.
    mpActivateDatasetCallback           = nullptr;
    bool activeDatasetConfigured        = true;
    uint16_t activeDatasetConfiguredLen = sizeof(bool);
    VerifyOrReturnError(mStorage, CHIP_ERROR_INTERNAL);
    mStorage->SyncGetKeyValue(kFailsafeActiveDatasetConfigured, &activeDatasetConfigured, activeDatasetConfiguredLen);
    VerifyOrDie(activeDatasetConfiguredLen == sizeof(bool));
    if (!activeDatasetConfigured)
    {
        // The active dataset should be no configured after calling this function, so we will try to attach an empty Thread dataset
        // and that will clear the one stored in the Thread stack since the SetActiveDataset operation fails and FailSafe timer is
        // triggered.
        Thread::OperationalDataset emptyDataset = {};
        CHIP_ERROR err                          = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(emptyDataset, nullptr);
        SaveActiveDatasetConfigured(false);
        return err;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericOpenThreadBorderRouterDelegate::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    otInstance * otInst = DeviceLayer::ThreadStackMgrImpl().OTInstance();
    VerifyOrReturnError(otInst, CHIP_ERROR_INCORRECT_STATE);

    otOperationalDatasetTlvs datasetTlvs;
    memcpy(datasetTlvs.mTlvs, pendingDataset.AsByteSpan().data(), pendingDataset.AsByteSpan().size());
    datasetTlvs.mLength = pendingDataset.AsByteSpan().size();
    {
        ScopedThreadLock threadLock;
        ReturnErrorCodeIf(otDatasetSetPendingTlvs(otInst, &datasetTlvs) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    return CHIP_NO_ERROR;
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
