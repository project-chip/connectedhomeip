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
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/Linux/ThreadStackManagerImpl.h>
#include <platform/ThreadStackManager.h>

#include <limits>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Thread;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

// NOTE: For ThreadDriver, we uses two network configs, one is mSavedNetwork, and another is mStagingNetwork, during init, it will
// load the network config from otbr-agent, and loads it into both mSavedNetwork and mStagingNetwork. When updating the networks,
// all changed are made on the staging network.
// TODO: The otbr-posix does not actually maintains its own networking states, it will always persist the last network connected.
// This should not be an issue for most cases, but we should implement the code for maintaining the states by ourselves.

CHIP_ERROR LinuxThreadDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    VerifyOrReturnError(ConnectivityMgrImpl().IsThreadAttached(), CHIP_NO_ERROR);
    VerifyOrReturnError(ThreadStackMgrImpl().GetThreadProvision(mStagingNetwork) == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.Init(mStagingNetwork.AsByteSpan());

    ThreadStackMgrImpl().SetNetworkStatusChangeCallback(networkStatusChangeCallback);

    return CHIP_NO_ERROR;
}

void LinuxThreadDriver::Shutdown()
{
    ThreadStackMgrImpl().SetNetworkStatusChangeCallback(nullptr);
}

CHIP_ERROR LinuxThreadDriver::CommitConfiguration()
{
    // Note: otbr-agent will persist the networks by their own, we don't have much to do for saving the networks (see Init() above,
    // we just loads the saved dataset from otbr-agent.)
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR LinuxThreadDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

Status LinuxThreadDriver::AddOrUpdateNetwork(ByteSpan operationalDataset, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    uint8_t extpanid[kSizeExtendedPanId];
    uint8_t newExtpanid[kSizeExtendedPanId];
    Thread::OperationalDataset newDataset;
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    newDataset.Init(operationalDataset);
    VerifyOrReturnError(newDataset.IsCommissioned(), Status::kOutOfRange);

    VerifyOrReturnError(!mStagingNetwork.IsCommissioned() || memcmp(extpanid, newExtpanid, kSizeExtendedPanId) == 0,
                        Status::kBoundsExceeded);

    mStagingNetwork = newDataset;
    return Status::kSuccess;
}

Status LinuxThreadDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
{
    outDebugText.reduce_size(0);
    outNetworkIndex = 0;
    uint8_t extpanid[kSizeExtendedPanId];
    if (!mStagingNetwork.IsCommissioned())
    {
        return Status::kNetworkNotFound;
    }
    if (mStagingNetwork.GetExtendedPanId(extpanid) != CHIP_NO_ERROR)
    {
        return Status::kUnknownError;
    }

    VerifyOrReturnError(networkId.size() == kSizeExtendedPanId && memcmp(networkId.data(), extpanid, kSizeExtendedPanId) == 0,
                        Status::kNetworkNotFound);
    mStagingNetwork.Clear();
    return Status::kSuccess;
}

Status LinuxThreadDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
{
    outDebugText.reduce_size(0);
    uint8_t extpanid[kSizeExtendedPanId];
    if (!mStagingNetwork.IsCommissioned())
    {
        return Status::kNetworkNotFound;
    }
    if (mStagingNetwork.GetExtendedPanId(extpanid) != CHIP_NO_ERROR)
    {
        return Status::kUnknownError;
    }

    VerifyOrReturnError(networkId.size() == kSizeExtendedPanId && memcmp(networkId.data(), extpanid, kSizeExtendedPanId) == 0,
                        Status::kNetworkNotFound);

    return Status::kSuccess;
}

void LinuxThreadDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
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

    VerifyOrExit(DeviceLayer::ThreadStackMgrImpl().AttachToThreadNetwork(mStagingNetwork, callback) == CHIP_NO_ERROR,
                 status = Status::kUnknownError);

exit:
    if (status != Status::kSuccess)
    {
        callback->OnResult(status, CharSpan(), 0);
    }
}

void LinuxThreadDriver::ScanNetworks(ThreadDriver::ScanCallback * callback)
{
    CHIP_ERROR err = DeviceLayer::ThreadStackMgrImpl().StartThreadScan(callback);
    // The ThreadScan callback will always be invoked in CHIP mainloop, which is strictly after this function
    if (err != CHIP_NO_ERROR)
    {
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

size_t LinuxThreadDriver::ThreadNetworkIterator::Count()
{
    return driver->mStagingNetwork.IsCommissioned() ? 1 : 0;
}

bool LinuxThreadDriver::ThreadNetworkIterator::Next(Network & item)
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

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
