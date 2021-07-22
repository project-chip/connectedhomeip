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

#import "CHIPDeviceConnectionBridge.h"
#import "CHIPDevice_Internal.h"
#import "CHIPError_Internal.h"

void CHIPDeviceConnectionBridge::OnConnected(void * context, chip::Controller::Device * device)
{
    auto * object = static_cast<CHIPDeviceConnectionBridge *>(context);
    CHIPDevice * chipDevice = [[CHIPDevice alloc] initWithDevice:device];
    dispatch_async(object->mQueue, ^{
        object->mCompletionHandler(chipDevice, nil);
        object->Release();
    });
}

void CHIPDeviceConnectionBridge::OnConnectionFailure(void * context, chip::NodeId deviceId, CHIP_ERROR error)
{
    auto * object = static_cast<CHIPDeviceConnectionBridge *>(context);
    dispatch_async(object->mQueue, ^{
        object->mCompletionHandler(nil, [CHIPError errorForCHIPErrorCode:error]);
        object->Release();
    });
}
