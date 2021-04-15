/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "CHIPDeviceStatusDelegateBridge.h"
#import <Foundation/Foundation.h>

CHIPDeviceStatusDelegateBridge::CHIPDeviceStatusDelegateBridge(void)
    : mDelegate(nil)
{
}

CHIPDeviceStatusDelegateBridge::~CHIPDeviceStatusDelegateBridge() {}

void CHIPDeviceStatusDelegateBridge::setDelegate(id<CHIPDeviceStatusDelegate> delegate, dispatch_queue_t queue)
{
    if (delegate && queue) {
        mDelegate = delegate;
        mQueue = queue;
    } else {
        mDelegate = nil;
        mQueue = nil;
    }
}

void CHIPDeviceStatusDelegateBridge::OnMessage(chip::System::PacketBufferHandle message)
{
    NSLog(@"DeviceStatusDelegate received message from the device");

    size_t data_len = message->DataLength();
    // convert to NSData
    NSMutableData * dataBuffer = [[NSMutableData alloc] initWithBytes:message->Start() length:data_len];
    message.FreeHead();

    while (!message.IsNull()) {
        data_len = message->DataLength();
        [dataBuffer appendBytes:message->Start() length:data_len];
        message.FreeHead();
    }

    id<CHIPDeviceStatusDelegate> strongDelegate = mDelegate;
    if (strongDelegate && mQueue) {
        dispatch_async(mQueue, ^{
            [strongDelegate onMessageReceived:dataBuffer];
        });
    }
}

void CHIPDeviceStatusDelegateBridge::OnStatusChange() { NSLog(@"CHIPDeviceStatusDelegateBridge device status changed"); }
