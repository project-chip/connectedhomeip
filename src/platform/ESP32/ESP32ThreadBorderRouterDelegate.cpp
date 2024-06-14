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

#include "ESP32ThreadBorderRouterDelegate.h"

#include <esp_openthread.h>
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

CHIP_ERROR ESP32ThreadBorderRouterDelegate::Init()
{
    mCallback = nullptr;
    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(OnPlatformEventHandler, reinterpret_cast<intptr_t>(this)));
    ReturnErrorOnFailure(RevertActiveDataset());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetBorderAgentId(MutableByteSpan & borderAgentIdSpan)
{
    otBorderAgentId borderAgentId;
    if (borderAgentIdSpan.size() < sizeof(borderAgentId.mId))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    ScopedThreadLock threadLock;
    otError err = otBorderAgentGetId(esp_openthread_get_instance(), &borderAgentId);
    if (err == OT_ERROR_NONE)
    {
        memcpy(borderAgentIdSpan.data(), borderAgentId.mId, sizeof(borderAgentId.mId));
        borderAgentIdSpan.reduce_size(sizeof(borderAgentId.mId));
        return CHIP_NO_ERROR;
    }
    return CHIP_ERROR_INTERNAL;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetThreadVersion(uint16_t & threadVersion)
{
    threadVersion = otThreadGetVersion();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetInterfaceEnabled(bool & interfaceEnabled)
{
    ScopedThreadLock threadLock;
    interfaceEnabled = otIp6IsEnabled(esp_openthread_get_instance());
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::GetDataset(Thread::OperationalDataset & dataset, DatasetType type)
{
    ScopedThreadLock threadLock;
    otError otErr = OT_ERROR_NONE;
    otOperationalDatasetTlvs datasetTlvs;
    if (type == DatasetType::kActive)
    {
        otErr = otDatasetGetActiveTlvs(esp_openthread_get_instance(), &datasetTlvs);
    }
    else
    {
        otErr = otDatasetGetPendingTlvs(esp_openthread_get_instance(), &datasetTlvs);
    }
    if (otErr == OT_ERROR_NONE)
    {
        return dataset.Init(ByteSpan(datasetTlvs.mTlvs, datasetTlvs.mLength));
    }
    return CHIP_ERROR_NOT_FOUND;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::SetActiveDataset(const Thread::OperationalDataset & activeDataset,
                                                             ActivateDatasetCallback * callback)
{
    VerifyOrReturnError(callback, CHIP_ERROR_INVALID_ARGUMENT);
    otOperationalDatasetTlvs datasetTlvs;
    memcpy(datasetTlvs.mTlvs, activeDataset.AsByteSpan().data(), activeDataset.AsByteSpan().size());
    datasetTlvs.mLength = activeDataset.AsByteSpan().size();

    ScopedThreadLock threadLock;
    // Save the previous thread state and dataset for reverting
    bool threadIsEnabled = GetThreadEnabled();
    ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kFailsafeThreadEnabledKey, threadIsEnabled));
    if (threadIsEnabled)
    {
        otOperationalDatasetTlvs stagingDataset;
        ReturnErrorCodeIf(otDatasetGetActiveTlvs(esp_openthread_get_instance(), &stagingDataset) != OT_ERROR_NONE,
                          CHIP_ERROR_INTERNAL);
        if (activeDataset.AsByteSpan().data_equal(ByteSpan(stagingDataset.mTlvs, stagingDataset.mLength)))
        {
            callback->OnActivateDatasetComplete(CHIP_NO_ERROR);
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgr().Put(kFailsafeThreadDatasetTlvsKey,
                                                                                   stagingDataset.mTlvs, stagingDataset.mLength));
    }
    SetThreadEnabled(false);
    ReturnErrorCodeIf(otDatasetSetActiveTlvs(esp_openthread_get_instance(), &datasetTlvs) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    SetThreadEnabled(true);
    mCallback = callback;
    return CHIP_NO_ERROR;
}

void ESP32ThreadBorderRouterDelegate::OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    ESP32ThreadBorderRouterDelegate * delegate = reinterpret_cast<ESP32ThreadBorderRouterDelegate *>(arg);
    if (delegate && delegate->mCallback)
    {
        if (event->Type == DeviceLayer::DeviceEventType::kThreadConnectivityChange &&
            event->ThreadConnectivityChange.Result == DeviceLayer::kConnectivity_Established)
        {
            delegate->mCallback->OnActivateDatasetComplete(CHIP_NO_ERROR);
            // Delete Failsafe Keys after activating dataset is completed
            DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadDatasetTlvsKey);
            DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadEnabledKey);
            delegate->mCallback = nullptr;
        }
    }
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::RevertActiveDataset()
{
    bool threadIsEnabled = false;
    CHIP_ERROR err       = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kFailsafeThreadEnabledKey, &threadIsEnabled);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    ScopedThreadLock threadLock;
    if (threadIsEnabled)
    {
        otOperationalDatasetTlvs stagingDataset;
        size_t datasetTlvslen = 0;
        err = DeviceLayer::PersistedStorage::KeyValueStoreMgr().Get(kFailsafeThreadDatasetTlvsKey, stagingDataset.mTlvs,
                                                                    sizeof(stagingDataset.mTlvs), &datasetTlvslen);
        if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
        {
            return CHIP_NO_ERROR;
        }
        ReturnErrorOnFailure(err);
        stagingDataset.mLength = datasetTlvslen;
        ReturnErrorOnFailure(SetThreadEnabled(false));
        ReturnErrorCodeIf(otDatasetSetActiveTlvs(esp_openthread_get_instance(), &stagingDataset) != OT_ERROR_NONE,
                          CHIP_ERROR_INTERNAL);
    }
    ReturnErrorOnFailure(SetThreadEnabled(threadIsEnabled));
    // Delete Failsafe Keys after reverting.
    DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadDatasetTlvsKey);
    DeviceLayer::PersistedStorage::KeyValueStoreMgr().Delete(kFailsafeThreadEnabledKey);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::SetPendingDataset(const Thread::OperationalDataset & pendingDataset)
{
    ScopedThreadLock threadLock;
    otOperationalDatasetTlvs datasetTlvs;
    memcpy(datasetTlvs.mTlvs, pendingDataset.AsByteSpan().data(), pendingDataset.AsByteSpan().size());
    datasetTlvs.mLength = pendingDataset.AsByteSpan().size();
    ReturnErrorCodeIf(otDatasetSetPendingTlvs(esp_openthread_get_instance(), &datasetTlvs) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32ThreadBorderRouterDelegate::SetThreadEnabled(bool enabled)
{
    otInstance * instance = esp_openthread_get_instance();
    bool isEnabled        = (otThreadGetDeviceRole(instance) != OT_DEVICE_ROLE_DISABLED);
    bool isIp6Enabled     = otIp6IsEnabled(instance);
    if (enabled && !isIp6Enabled)
    {
        ReturnErrorCodeIf(otIp6SetEnabled(instance, enabled) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    if (enabled != isEnabled)
    {
        ReturnErrorCodeIf(otThreadSetEnabled(instance, enabled) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    if (!enabled && isIp6Enabled)
    {
        ReturnErrorCodeIf(otIp6SetEnabled(instance, enabled) != OT_ERROR_NONE, CHIP_ERROR_INTERNAL);
    }
    return CHIP_NO_ERROR;
}

bool ESP32ThreadBorderRouterDelegate::GetThreadEnabled()
{
    otInstance * instance = esp_openthread_get_instance();
    return otIp6IsEnabled(instance) && (otThreadGetDeviceRole(instance) != OT_DEVICE_ROLE_DISABLED);
}

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
