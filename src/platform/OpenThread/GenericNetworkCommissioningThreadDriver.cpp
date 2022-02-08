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
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/NetworkCommissioning.h>
#include <platform/OpenThread/GenericNetworkCommissioningThreadDriver.h>
#include <platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.h>
#include <platform/ThreadStackManager.h>

#include <limits>

using namespace chip;
using namespace chip::Thread;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {
// NOTE: For ThreadDriver, we use two network configs, one is mSavedNetwork, and another is mStagingNetwork, during init, it will
// load the network config from thread persistent info, and loads it into both mSavedNetwork and mStagingNetwork. When updating the
// networks, all changes are made on the staging network. When validated we can commit it and save it to the persistent info

CHIP_ERROR GenericThreadDriver::Init()
{
    ByteSpan currentProvision;
    VerifyOrReturnError(ThreadStackMgrImpl().IsThreadAttached(), CHIP_NO_ERROR);
    VerifyOrReturnError(ThreadStackMgrImpl().GetThreadProvision(currentProvision) == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.Init(currentProvision);
    mStagingNetwork.Init(currentProvision);

    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericThreadDriver::CommitConfiguration()
{
    // Note: on AttachToThreadNetwork OpenThread will persist the networks on its own,
    // we don't have much to do for saving the networks (see Init() above,
    // we just load the saved dataset from ot instance.)
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR GenericThreadDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

Status GenericThreadDriver::AddOrUpdateNetwork(ByteSpan operationalDataset)
{
    uint8_t extpanid[kSizeExtendedPanId];
    uint8_t newExtpanid[kSizeExtendedPanId];
    Thread::OperationalDataset newDataset;

    newDataset.Init(operationalDataset);
    VerifyOrReturnError(newDataset.IsCommissioned(), Status::kOutOfRange);

    newDataset.GetExtendedPanId(newExtpanid);
    mStagingNetwork.GetExtendedPanId(extpanid);

    // We only support one active operational dataset. Add/Update based on either:
    // Staging network not commissioned yet (active) or we are updating the dataset with same Extended Pan ID.
    VerifyOrReturnError(!mStagingNetwork.IsCommissioned() || memcmp(extpanid, newExtpanid, kSizeExtendedPanId) == 0,
                        Status::kBoundsExceeded);

    mStagingNetwork = newDataset;
    return Status::kSuccess;
}

Status GenericThreadDriver::RemoveNetwork(ByteSpan networkId)
{
    uint8_t extpanid[kSizeExtendedPanId];
    if (!mStagingNetwork.IsCommissioned())
    {
        return Status::kNetworkNotFound;
    }
    else if (mStagingNetwork.GetExtendedPanId(extpanid) != CHIP_NO_ERROR)
    {
        return Status::kUnknownError;
    }

    VerifyOrReturnError(networkId.size() == kSizeExtendedPanId && memcmp(networkId.data(), extpanid, kSizeExtendedPanId) == 0,
                        Status::kNetworkNotFound);
    mStagingNetwork.Clear();
    return Status::kSuccess;
}

Status GenericThreadDriver::ReorderNetwork(ByteSpan networkId, uint8_t index)
{
    uint8_t extpanid[kSizeExtendedPanId];
    if (!mStagingNetwork.IsCommissioned())
    {
        return Status::kNetworkNotFound;
    }
    else if (mStagingNetwork.GetExtendedPanId(extpanid) != CHIP_NO_ERROR)
    {
        return Status::kUnknownError;
    }

    VerifyOrReturnError(networkId.size() == kSizeExtendedPanId && memcmp(networkId.data(), extpanid, kSizeExtendedPanId) == 0,
                        Status::kNetworkNotFound);

    return Status::kSuccess;
}

void GenericThreadDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
{
    NetworkCommissioning::Status status = Status::kSuccess;
    uint8_t extpanid[kSizeExtendedPanId];
    if (!mStagingNetwork.IsCommissioned())
    {
        ExitNow(status = Status::kNetworkNotFound);
    }
    else if (mStagingNetwork.GetExtendedPanId(extpanid) != CHIP_NO_ERROR)
    {
        ExitNow(status = Status::kUnknownError);
    }

    VerifyOrExit((networkId.size() == kSizeExtendedPanId && memcmp(networkId.data(), extpanid, kSizeExtendedPanId) == 0),
                 status = Status::kNetworkNotFound);

    VerifyOrExit(DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(mStagingNetwork.AsByteSpan(), callback) == CHIP_NO_ERROR,
                 status = Status::kUnknownError);

exit:
    if (status != Status::kSuccess)
    {
        callback->OnResult(status, CharSpan(), 0);
    }
}

void GenericThreadDriver::ScanNetworks(ThreadDriver::ScanCallback * callback)
{
    CHIP_ERROR err = DeviceLayer::ThreadStackMgrImpl().StartThreadScan(callback);
    if (err != CHIP_NO_ERROR)
    {
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

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

    ByteSpan currentProvision;
    Thread::OperationalDataset currentDataset;
    uint8_t enabledExtPanId[Thread::kSizeExtendedPanId];

    // The Thread network is not actually enabled.
    VerifyOrReturnError(ConnectivityMgrImpl().IsThreadAttached(), true);
    VerifyOrReturnError(ThreadStackMgrImpl().GetThreadProvision(currentProvision) == CHIP_NO_ERROR, true);
    VerifyOrReturnError(currentDataset.Init(currentProvision) == CHIP_NO_ERROR, true);
    // The Thread network is not enabled, but has a different extended pan id.
    VerifyOrReturnError(currentDataset.GetExtendedPanId(enabledExtPanId) == CHIP_NO_ERROR, true);
    VerifyOrReturnError(memcmp(extpanid, enabledExtPanId, kSizeExtendedPanId) == 0, true);
    // The Thread network is enabled and has the same extended pan id as the one in our record.
    item.connected = true;

    return true;
}

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
