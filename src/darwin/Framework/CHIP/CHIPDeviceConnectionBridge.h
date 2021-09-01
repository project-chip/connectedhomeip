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

#import <CHIPDeviceController.h>
#import <Foundation/Foundation.h>

#include <controller/CHIPDeviceController.h>
#include <lib/core/ReferenceCounted.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPDeviceConnectionBridge : public chip::ReferenceCounted<CHIPDeviceConnectionBridge>
{
public:
    CHIPDeviceConnectionBridge(CHIPDeviceConnectionCallback completionHandler, dispatch_queue_t queue) :
        mCompletionHandler(completionHandler), mQueue(queue), mOnConnected(OnConnected, this),
        mOnConnectFailed(OnConnectionFailure, this)
    {}

    ~CHIPDeviceConnectionBridge()
    {
        mOnConnected.Cancel();
        mOnConnectFailed.Cancel();
    }

    CHIP_ERROR connect(chip::Controller::DeviceController * controller, chip::NodeId deviceID)
    {
        return controller->GetConnectedDevice(deviceID, &mOnConnected, &mOnConnectFailed);
    }

private:
    CHIPDeviceConnectionCallback mCompletionHandler;
    dispatch_queue_t mQueue;
    chip::Callback::Callback<chip::Controller::OnDeviceConnected> mOnConnected;
    chip::Callback::Callback<chip::Controller::OnDeviceConnectionFailure> mOnConnectFailed;

    static void OnConnected(void * context, chip::Controller::Device * device);
    static void OnConnectionFailure(void * context, chip::NodeId deviceId, CHIP_ERROR error);
};

NS_ASSUME_NONNULL_END
