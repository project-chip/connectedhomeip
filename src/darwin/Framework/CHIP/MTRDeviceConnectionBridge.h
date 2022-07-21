/**
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

#import "MTRDeviceController.h"

#import <Foundation/Foundation.h>

#include <controller/CHIPDeviceController.h>
#include <lib/core/ReferenceCounted.h>

NS_ASSUME_NONNULL_BEGIN

class MTRDeviceConnectionBridge : public chip::ReferenceCounted<MTRDeviceConnectionBridge>
{
public:
    MTRDeviceConnectionBridge(MTRDeviceConnectionCallback completionHandler, dispatch_queue_t queue) :
        mCompletionHandler(completionHandler), mQueue(queue), mOnConnected(OnConnected, this),
        mOnConnectFailed(OnConnectionFailure, this)
    {}

    ~MTRDeviceConnectionBridge()
    {
        mOnConnected.Cancel();
        mOnConnectFailed.Cancel();
    }

    CHIP_ERROR connect(chip::Controller::DeviceController * controller, chip::NodeId deviceID)
    {
        return controller->GetConnectedDevice(deviceID, &mOnConnected, &mOnConnectFailed);
    }

private:
    MTRDeviceConnectionCallback mCompletionHandler;
    dispatch_queue_t mQueue;
    chip::Callback::Callback<chip::OnDeviceConnected> mOnConnected;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnConnectFailed;

    static void OnConnected(void * context, chip::OperationalDeviceProxy * device);
    static void OnConnectionFailure(void * context, chip::PeerId peerId, CHIP_ERROR error);
};

NS_ASSUME_NONNULL_END
