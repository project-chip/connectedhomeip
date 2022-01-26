/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#include "PairingDelegateBridge.h"

using namespace ::chip;
using namespace ::chip::Controller;

const uint8_t kEndpoint = 0;

CHIPDevice * CHIPGetConnectedDevice(NodeId nodeID)
{
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];
    CHIPDevice * __block connectedDevice;
    [controller getConnectedDevice:nodeID
                             queue:dispatch_get_main_queue()
                 completionHandler:^(CHIPDevice * _Nullable device, NSError * _Nullable error) {
                     connectedDevice = device;
                     if (error) {
                         NSLog(@"Error on getting device: %@", error);
                     }
                 }];
    return connectedDevice;
}

@interface CHIPToolPairingDelegate ()
@property (nonatomic, strong) CHIPBasic * cluster;
@property (nonatomic, strong) ResponseHandler responseHandler;
@end
@implementation CHIPToolPairingDelegate
- (void)onPairingComplete:(NSError *)error
{
    NSLog(@"onPairingComplete: %@", error);
    // dispatch_queue_t callbackQueue = dispatch_get_main_queue();
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.chip.callbacks", DISPATCH_QUEUE_SERIAL);
    _cluster = [[CHIPBasic alloc] initWithDevice:CHIPGetConnectedDevice(_deviceID) endpoint:kEndpoint queue:callbackQueue];
    __block uint32_t responseCount = 100000;
    __weak typeof(self) weakSelf = self;
    _responseHandler = ^(NSString * value, NSError * err) {
        NSLog(@"Result: err: %@, values: %@", err, value);
        if (err != nil) {
            NSLog(@"Something went wrong: Stopping the app");
            exit(EXIT_FAILURE);
        }
        responseCount--;
        typeof(self) strongSelf = weakSelf;
        if (strongSelf) {
            if (responseCount) {
                [strongSelf emitCommand];
                NSLog(@"Still expecting %d responses", responseCount);
            } else {
                NSLog(@"Will shutdown the stack");
                CHIPDeviceController * controller = [CHIPDeviceController sharedController];
                [controller shutdown];
                weakSelf.commandBridge->SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE);
            }
        }
    };
    [self emitCommand];
    _commandBridge->SetCommandExitStatus(CHIP_ERROR_INCORRECT_STATE);
}
- (void)onPairingDeleted:(NSError *)error
{
    NSLog(@"Pairing Deleted: %@", error);
    _commandBridge->SetCommandExitStatus(CHIP_NO_ERROR);
}

- (void)onCommissioningComplete:(NSError *)error
{
    NSLog(@"Pairing Complete: %@", error);
    _commandBridge->SetCommandExitStatus(CHIP_NO_ERROR);
}

- (void)emitCommand
{
    [_cluster readAttributeSoftwareVersionStringWithCompletionHandler:_responseHandler];
}

@end
