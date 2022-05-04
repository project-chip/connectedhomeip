/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <platform/ThreadStackManager.h>
#include <platform/Tizen/NetworkCommissioningDriver.h>
#include <platform/Tizen/ThreadStackManagerImpl.h>

#include <limits>
#include <string>
#include <vector>

using namespace chip;
using namespace chip::Thread;

namespace chip {
namespace DeviceLayer {
namespace NetworkCommissioning {

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD

CHIP_ERROR TizenThreadDriver::Init(BaseDriver::NetworkStatusChangeCallback * networkStatusChangeCallback)
{
    VerifyOrReturnError(ConnectivityMgrImpl().IsThreadAttached(), CHIP_NO_ERROR);
    VerifyOrReturnError(ThreadStackMgrImpl().GetThreadProvision(mStagingNetwork) == CHIP_NO_ERROR, CHIP_NO_ERROR);

    mSavedNetwork.Init(mStagingNetwork.AsByteSpan());

    return CHIP_NO_ERROR;
}

CHIP_ERROR TizenThreadDriver::CommitConfiguration()
{
    mSavedNetwork = mStagingNetwork;
    return CHIP_NO_ERROR;
}

CHIP_ERROR TizenThreadDriver::RevertConfiguration()
{
    mStagingNetwork = mSavedNetwork;
    return CHIP_NO_ERROR;
}

Status TizenThreadDriver::AddOrUpdateNetwork(ByteSpan operationalDataset, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
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

Status TizenThreadDriver::RemoveNetwork(ByteSpan networkId, MutableCharSpan & outDebugText, uint8_t & outNetworkIndex)
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

Status TizenThreadDriver::ReorderNetwork(ByteSpan networkId, uint8_t index, MutableCharSpan & outDebugText)
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

void TizenThreadDriver::ConnectNetwork(ByteSpan networkId, ConnectCallback * callback)
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

void TizenThreadDriver::ScanNetworks(ThreadDriver::ScanCallback * callback)
{
    CHIP_ERROR err = DeviceLayer::ThreadStackMgrImpl().StartThreadScan(callback);
    if (err != CHIP_NO_ERROR)
    {
        callback->OnFinished(Status::kUnknownError, CharSpan(), nullptr);
    }
}

size_t TizenThreadDriver::ThreadNetworkIterator::Count()
{
    ChipLogError(NetworkProvisioning, "Not implemented");
    return 0;
}

bool TizenThreadDriver::ThreadNetworkIterator::Next(Network & item)
{
    ChipLogError(NetworkProvisioning, "Not implemented");
    return false;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

} // namespace NetworkCommissioning
} // namespace DeviceLayer
} // namespace chip
