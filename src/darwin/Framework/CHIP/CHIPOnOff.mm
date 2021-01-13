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

#import <Foundation/Foundation.h>

#import "CHIPCallbackBridge.h"
#import "CHIPDevice.h"
#import "CHIPDevice_Internal.h"
#import "CHIPOnOff.h"

#include <controller/CHIPClusters.h>

@interface CHIPOnOff ()

@property (readonly) chip::Controller::OnOffCluster cppCluster;
@property (readonly, nonatomic) dispatch_queue_t callbackQueue;
@end

@implementation CHIPOnOff

- (instancetype)initWithDevice:(CHIPDevice *)device endpoint:(chip::EndpointId)endpoint queue:(dispatch_queue_t)queue
{
    CHIP_ERROR err = _cppCluster.Associate([device internalDevice], endpoint);

    if (err != CHIP_NO_ERROR) {
        return nil;
    }

    if (self = [super init]) {
        _callbackQueue = queue;
    }
    return self;
}

- (BOOL)lightOn:(CHIPDeviceCallback)onCompletion
{
    CHIPCallbackBridge * callback = new CHIPCallbackBridge(onCompletion, _callbackQueue);
    if (!callback) {
        return NO;
    }

    CHIP_ERROR err = self.cppCluster.On(callback);
    if (err != CHIP_NO_ERROR) {
        callback->Cancel();
        delete callback;
        return NO;
    }
    return YES;
}

- (BOOL)lightOff:(CHIPDeviceCallback)onCompletion
{
    CHIPCallbackBridge * callback = new CHIPCallbackBridge(onCompletion, _callbackQueue);
    if (!callback) {
        return NO;
    }

    CHIP_ERROR err = self.cppCluster.Off(callback);
    if (err != CHIP_NO_ERROR) {
        callback->Cancel();
        delete callback;
        return NO;
    }
    return YES;
}

- (BOOL)toggleLight:(CHIPDeviceCallback)onCompletion
{
    CHIPCallbackBridge * callback = new CHIPCallbackBridge(onCompletion, _callbackQueue);
    if (!callback) {
        return NO;
    }

    CHIP_ERROR err = self.cppCluster.Toggle(callback);
    if (err != CHIP_NO_ERROR) {
        callback->Cancel();
        delete callback;
        return NO;
    }
    return YES;
}

@end
