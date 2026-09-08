/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <controller/ControllerOperation.h>

#include <controller/CHIPDeviceController.h>

#include <utility>

namespace chip {
namespace Controller {

void ControllerOperationBase::Start(DeviceController & controller, NodeId nodeId, Callback::Cancelable::Owned onCompletion)
{
    Callback::CancelableOperationBase::Start(std::move(onCompletion));
    CHIP_ERROR err = controller.GetConnectedDevice(nodeId, &mDeviceConnected, &mDeviceConnectionFailure);
    if (err != CHIP_NO_ERROR)
    {
        return OnConnectionFailure(err);
    }
}

void ControllerOperationBase::OnFinished(bool cancelled)
{
    mDeviceConnected.Cancel();
    mDeviceConnectionFailure.Cancel();
    Callback::CancelableOperationBase::OnFinished(cancelled);
}

void ControllerInvokeOperationBase::Start(DeviceController & controller, NodeId nodeId, EndpointId endpoint,
                                          Callback::Cancelable::Owned onCompletion)
{
    // Before starting: a connection failure completes the operation from within the call below.
    mEndpoint = endpoint;
    ControllerOperationBase::Start(controller, nodeId, std::move(onCompletion));
}

void ControllerInvokeOperationBase::OnFinished(bool cancelled)
{
    // Tear down an invocation we are no longer interested in. Reaching this from within one of the
    // handlers installed by Invoke() is not a concern: they clear mCancelInvoke first.
    if (mCancelInvoke)
    {
        mCancelInvoke();
        mCancelInvoke = nullptr;
    }
    ControllerOperationBase::OnFinished(cancelled);
}

} // namespace Controller
} // namespace chip
