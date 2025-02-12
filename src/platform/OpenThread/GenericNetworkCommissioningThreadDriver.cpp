/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <lib/support/CodeUtils.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <platform/NetworkCommissioning.h>
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/ThreadStackManager.h>

#include <limits>

using namespace chip;
using namespace chip::Thread;
using namespace chip::DeviceLayer::PersistedStorage;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

// NOTE: For GenericThreadDriver, we assume that the network configuration is persisted by
// the OpenThread stack after ConnectNetwork command is called, and before that, all the changes
// are made to a local copy of the dataset stored in mStagingNetwork.
// Also, in order to support the fail-safe mechanism, the configuration is backed up in a temporary
// KVS slot upon any changes and restored when the fail-safe timeout is triggered or the device
// reboots without completing all the changes.

CHIP_ERROR GenericThreadDriver::Init(Internal::BaseDriver::NetworkStatusChangeCallback * statusChangeCallback)
{
    ThreadStackMgrImpl().SetNetworkStatusChangeCallback(statusChangeCallback);
    ThreadStackMgrImpl().GetThreadProvision(mStagingNetwork);
    ReturnErrorOnFailure(PlatformMgr().AddEventHandler(OnThreadStateChangeHandler, reinterpret_cast<intptr_t>(this)));

    // If the network configuration backup exists, it means that the device has been rebooted with
    // the fail-safe armed. Since OpenThread persists all operational dataset changes, the backup
    // must be restored on the boot. If there's no backup, the below function is a no-op.
    RevertConfiguration();

    CheckInterfaceEnabled();

    return CHIP_NO_ERROR;
}

void GenericThreadDriver::OnThreadStateChangeHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    if ((event->Type == DeviceEventType::kThreadStateChange) &&
        (event->ThreadStateChange.OpenThread.Flags & OT_CHANGED_THREAD_PANID))
    {
        // Update the mStagingNetwork when thread panid changed
        ThreadStackMgrImpl().GetThreadProvision(reinterpret_cast<GenericThreadDriver *>(arg)->mStagingNetwork);
    }
}

void GenericThreadDriver::Shutdown()
{
    ThreadStackMgrImpl().SetNetworkStatusChangeCallback(nullptr);
}

CHIP_ERROR GenericThreadDriver::CommitConfiguration()
{
    // OpenThread persists the network configuration on AttachToThreadNetwork, so simply remove
    // the backup, so that it cannot be restored. If no backup could be found, it means that the
    // configuration has not been modified since the fail-safe was armed, so return with no error.

    CHIP_ERROR error = KeyValueStoreMgr().Delete(DefaultStorageKeyAllocator::FailSafeNetworkConfig().KeyName());

    return error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND ? CHIP_NO_ERROR : error;
}

CHIP_ERROR GenericThreadDriver::RevertConfiguration()
{
    uint8_t datasetBytes[Thread::kSizeOperationalDataset];
    size_t datasetLength;

    CHIP_ERROR error = KeyValueStoreMgr().Get(DefaultStorageKeyAllocator::FailSafeNetworkConfig().KeyName(), datasetBytes,
                                              sizeof(datasetBytes), &datasetLength);

    // If no backup could be found, it means that the network configuration has not been modified
    // since the fail-safe was armed, so return with no error.
    ReturnErrorCodeIf(error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND, CHIP_NO_ERROR);

    if (!GetEnabled())
    {
        // When reverting configuration, set InterfaceEnabled to default value (true).
        // From the spec:
        // If InterfaceEnabled is written to false on the same interface as that which is used to write the value, the Administrator
        // could await the recovery of network configuration to prior safe values, before being able to communicate with the
        // node again.
        ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Delete(kInterfaceEnabled));
    }

    ChipLogProgress(NetworkProvisioning, "Reverting Thread operational dataset");

    if (error == CHIP_NO_ERROR)
    {
        error = mStagingNetwork.Init(ByteSpan(datasetBytes, datasetLength));
    }

    if (error == CHIP_NO_ERROR)
    {
        error = DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(mStagingNetwork, /* callback */ nullptr);
    }

    // Always remove the backup, regardless if it can be successfully restored.
    KeyValueStoreMgr().Delete(DefaultStorageKeyAllocator::FailSafeNetworkConfig().KeyName());

    return error;
}

Status GenericThreadDriver::AddOrUpdateNetwork(ByteSpan operationalDataset, MutableCharSpan & outDebugText,
                                               uint8_t & outNetworkIndex)
{
    ByteSpan newExtpanid;
    Thread::OperationalDataset newDataset;

    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    VerifyOrReturnError(newDataset.Init(operationalDataset) == CHIP_NO_ERROR && newDataset.IsCommissioned(), Status::kOutOfRange);
    newDataset.GetExtendedPanIdAsByteSpan(newExtpanid);

    // We only support one active operational dataset. Add/Update based on either:
    // Staging network not commissioned yet (active) or we are updating the dataset with same Extended Pan ID.
    VerifyOrReturnError(!mStagingNetwork.IsCommissioned() || MatchesNetworkId(mStagingNetwork, newExtpanid) == Status::kSuccess,
                        Status::kBoundsExceeded);
    VerifyOrReturnError(BackupConfiguration() == CHIP_NO_ERROR, Status::kUnknownError);

    mStagingNetwork = newDataset;
    return Status::kSuccess;
}

Status GenericThreadDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;

    NetworkCommissioning::Status status = MatchesNetworkId(mStagingNetwork, networkId);

    VerifyOrReturnError(status == Status::kSuccess, status);
    VerifyOrReturnError(BackupConfiguration() == CHIP_NO_ERROR, Status::kUnknownError);

    mStagingNetwork.Clear();

    return Status::kSuccess;
}

Status GenericThreadDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);

    NetworkCommissioning::Status status = MatchesNetworkId(mStagingNetwork, networkId);

    VerifyOrReturnError(status == Status::kSuccess, status);
    VerifyOrReturnError(index == 0, Status::kOutOfRange);

    return Status::kSuccess;
}

void GenericThreadDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    NetworkCommissioning::Status status = MatchesNetworkId(mStagingNetwork, networkId);

    if (!GetEnabled())
    {
        // Set InterfaceEnabled to default value (true).
        ReturnOnFailure(PersistedStorage::KeyValueStoreMgr().Delete(kInterfaceEnabled));
    }

    if (status == Status::kSuccess && BackupConfiguration() != CHIP_NO_ERROR)
    {
        status = Status::kUnknownError;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT
    if (status == Status::kSuccess && ThreadStackMgrImpl().IsThreadAttached())
    {
        Thread::OperationalDataset currentDataset;
        if (ThreadStackMgrImpl().GetThreadProvision(currentDataset) == CHIP_NO_ERROR)
        {
            // Clear the previous srp host and services
            if (!currentDataset.AsByteSpan().data_equal(mStagingNetwork.AsByteSpan()) &&
                ThreadStackMgrImpl().ClearAllSrpHostAndServices() != CHIP_NO_ERROR)
            {
                status = Status::kUnknownError;
            }
        }
        else
        {
            status = Status::kUnknownError;
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD_SRP_CLIENT

    if (status == Status::kSuccess &&
        DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(mStagingNetwork, callback) != CHIP_NO_ERROR)
    {
        status = Status::kUnknownError;
    }

    if (status != Status::kSuccess)
    {
        callback->OnResult(status, CharSpan(), 0);
    }
}

CHIP_ERROR GenericThreadDriver::SetEnabled(bool enabled)
{
    if (enabled == GetEnabled())
    {
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kInterfaceEnabled, &enabled, sizeof(enabled)));

    if ((!enabled && ThreadStackMgrImpl().IsThreadEnabled()) || (enabled && ThreadStackMgrImpl().IsThreadProvisioned()))
    {
        ReturnErrorOnFailure(ThreadStackMgrImpl().SetThreadEnabled(enabled));
    }
    return CHIP_NO_ERROR;
}

bool GenericThreadDriver::GetEnabled()
{
    bool value;
    // InterfaceEnabled default value is true.
    VerifyOrReturnValue(PersistedStorage::KeyValueStoreMgr().Get(kInterfaceEnabled, &value, sizeof(value)) == CHIP_NO_ERROR, true);
    return value;
}

void GenericThreadDriver::ScanNetworks(ThreadDriver::ScanCallback * callback)
{
    if (DeviceLayer::ThreadStackMgrImpl().StartThreadScan(callback) != CHIP_NO_ERROR)
    {
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

Status GenericThreadDriver::MatchesNetworkId(const Thread::OperationalDataset & dataset, const ByteSpan & networkId) const
{
    ByteSpan extpanid;

    if (!dataset.IsCommissioned())
    {
        return Status::kNetworkIDNotFound;
    }

    if (dataset.GetExtendedPanIdAsByteSpan(extpanid) != CHIP_NO_ERROR)
    {
        return Status::kUnknownError;
    }

    return networkId.data_equal(extpanid) ? Status::kSuccess : Status::kNetworkIDNotFound;
}

CHIP_ERROR GenericThreadDriver::BackupConfiguration()
{
    // If configuration is already backed up, return with no error
    CHIP_ERROR err = KeyValueStoreMgr().Get(DefaultStorageKeyAllocator::FailSafeNetworkConfig().KeyName(), nullptr, 0);

    if (err == CHIP_NO_ERROR || err == CHIP_ERROR_BUFFER_TOO_SMALL)
    {
        return CHIP_NO_ERROR;
    }

    ByteSpan dataset = mStagingNetwork.AsByteSpan();

    return KeyValueStoreMgr().Put(DefaultStorageKeyAllocator::FailSafeNetworkConfig().KeyName(), dataset.data(), dataset.size());
}

void GenericThreadDriver::CheckInterfaceEnabled()
{
#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD_AUTOSTART
    // If the Thread interface is enabled and stack has been provisioned, but is not currently enabled, enable it now.
    if (GetEnabled() && ThreadStackMgrImpl().IsThreadProvisioned() && !ThreadStackMgrImpl().IsThreadEnabled())
    {
        ReturnOnFailure(ThreadStackMgrImpl().SetThreadEnabled(true));
        ChipLogProgress(DeviceLayer, "OpenThread ifconfig up and thread start");
    }
#endif
}

#if CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
CHIP_ERROR GenericThreadDriver::DisconnectFromNetwork()
{
    if (ThreadStackMgrImpl().IsThreadProvisioned())
    {
        Thread::OperationalDataset emptyNetwork = {};
        // Attach to an empty network will disconnect the driver.
        ReturnErrorOnFailure(ThreadStackMgrImpl().AttachToThreadNetwork(emptyNetwork, nullptr));
    }
    return CHIP_NO_ERROR;
}
#endif

size_t GenericThreadDriver::ThreadNetworkIterator::Count()
{
    return driver->mStagingNetwork.IsCommissioned() ? 1 : 0;
}

bool GenericThreadDriver::ThreadNetworkIterator::Next(Network & item)
{
    if (exhausted || !driver->mStagingNetwork.IsCommissioned())
    {
        return false;
    }
    uint8_t extpanid[kSizeExtendedPanId];
    VerifyOrReturnError(driver->mStagingNetwork.GetExtendedPanId(extpanid) == CHIP_NO_ERROR, false);
    memcpy(item.networkID, extpanid, kSizeExtendedPanId);
    item.networkIDLen = kSizeExtendedPanId;
    item.connected    = false;
    exhausted         = true;

    Thread::OperationalDataset currentDataset;
    uint8_t enabledExtPanId[Thread::kSizeExtendedPanId];

    // The Thread network is not actually enabled.
    VerifyOrReturnError(ConnectivityMgrImpl().IsThreadAttached(), true);
    VerifyOrReturnError(ThreadStackMgrImpl().GetThreadProvision(currentDataset) == CHIP_NO_ERROR, true);
    // The Thread network is not enabled, but has a different extended pan id.
    VerifyOrReturnError(currentDataset.GetExtendedPanId(enabledExtPanId) == CHIP_NO_ERROR, true);
    VerifyOrReturnError(memcmp(extpanid, enabledExtPanId, kSizeExtendedPanId) == 0, true);
    // The Thread network is enabled and has the same extended pan id as the one in our record.
    item.connected = true;

    return true;
}

ThreadCapabilities GenericThreadDriver::GetSupportedThreadFeatures()
{
    BitMask<ThreadCapabilities> capabilites = 0;
    capabilites.SetField(ThreadCapabilities::kIsBorderRouterCapable,
                         CHIP_DEVICE_CONFIG_THREAD_BORDER_ROUTER /*OPENTHREAD_CONFIG_BORDER_ROUTER_ENABLE*/);
    capabilites.SetField(ThreadCapabilities::kIsRouterCapable, CHIP_DEVICE_CONFIG_THREAD_FTD);
    capabilites.SetField(ThreadCapabilities::kIsSleepyEndDeviceCapable, !CHIP_DEVICE_CONFIG_THREAD_FTD);
    capabilites.SetField(ThreadCapabilities::kIsFullThreadDevice, CHIP_DEVICE_CONFIG_THREAD_FTD);
    capabilites.SetField(ThreadCapabilities::kIsSynchronizedSleepyEndDeviceCapable,
                         (!CHIP_DEVICE_CONFIG_THREAD_FTD && CHIP_DEVICE_CONFIG_THREAD_SSED));
    return capabilites;
}

uint16_t GenericThreadDriver::GetThreadVersion()
{
    uint16_t version = 0;
    ThreadStackMgrImpl().GetThreadVersion(version);
    return version;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
