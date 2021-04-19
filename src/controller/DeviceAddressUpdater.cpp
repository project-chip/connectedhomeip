/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include "DeviceAddressUpdater.h"

#include <controller/CHIPDeviceController.h>

namespace chip {
namespace Controller {

CHIP_ERROR DeviceAddressUpdater::Init(DeviceController * controller, DeviceAddressUpdateDelegate * delegate)
{
    VerifyOrReturnError(mController == nullptr, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mDelegate == nullptr, CHIP_ERROR_INCORRECT_STATE);

    mController = controller;
    mDelegate   = delegate;

    return CHIP_NO_ERROR;
}

void DeviceAddressUpdater::OnNodeIdResolved(NodeId nodeId, const Mdns::ResolvedNodeData & nodeData)
{
    CHIP_ERROR error = CHIP_NO_ERROR;
    Device * device  = nullptr;

    VerifyOrExit(nodeData.mAddress.Type() != Inet::kIPAddressType_Any, error = CHIP_ERROR_INVALID_ADDRESS);
    VerifyOrExit(mController != nullptr, error = CHIP_ERROR_INCORRECT_STATE);
    SuccessOrExit(error = mController->GetDevice(nodeId, &device));

    device->UpdateAddress(Transport::PeerAddress::UDP(nodeData.mAddress, nodeData.mPort, nodeData.mInterfaceId));
    mController->PersistDevice(device);

exit:
    if (mDelegate != nullptr)
    {
        mDelegate->OnAddressUpdateComplete(nodeId, error);
    }
}

void DeviceAddressUpdater::OnNodeIdResolutionFailed(NodeId nodeId, CHIP_ERROR error)
{
    if (mDelegate != nullptr)
    {
        mDelegate->OnAddressUpdateComplete(nodeId, error);
    }
}

} // namespace Controller
} // namespace chip
