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
#import <os/log.h>

#import "CHIPEchoClient.h"
#import "CHIPDeviceController.h"

static UInt32 const kDefaultFrequencySeconds = 10;
static NSString * const kEchoString = @"Hello from darwin!";

@implementation CHIPEchoClient {
    dispatch_queue_t _echoCallbackQueue;
    dispatch_source_t _send_timer;
    CHIPDeviceController* _chipController;
}

- (instancetype)initWithServerAddress:(NSString *)ipAddress port:(UInt16)port {
    if (self = [super init]) {
        NSError * error;
        _echoCallbackQueue = dispatch_queue_create("com.zigbee.chip.echo", NULL);
        _chipController = [[CHIPDeviceController alloc] init:_echoCallbackQueue];

        BOOL didInit = [_chipController connect:ipAddress port:port error:&error onMessage:^(NSData *  _Nonnull message, NSString * _Nonnull ipAddress, UInt16 port) {
            NSData* sentMessage = [kEchoString dataUsingEncoding:NSUTF8StringEncoding];
            if ([sentMessage isEqualToData:message]) {
                os_log(OS_LOG_DEFAULT, "Received expected echo response...");
            } else {
                os_log(OS_LOG_DEFAULT, "Unexpected echo response. \nExpected: %@ \nReceived: %@", sentMessage, message);
            }
        } onError:^(NSError * _Nonnull error) {
            os_log(OS_LOG_DEFAULT, "Received error %@. Stopping...", error);
            [self stop];
        }];

        if (!didInit) {
            os_log(OS_LOG_DEFAULT, "Failed to init with error %@", error);
            return nil;
        }
        _send_timer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _echoCallbackQueue);
        if (!_send_timer) {
            os_log(OS_LOG_DEFAULT, "Could not create dispatch source");
            return nil;
        }

    }
    return self;
}

- (BOOL)start:(NSError * __autoreleasing *)error frequency:(UInt32)frequency {
    dispatch_source_set_timer(_send_timer, dispatch_walltime(NULL, 0), frequency * NSEC_PER_SEC, 1 * NSEC_PER_SEC);
    dispatch_source_set_event_handler(_send_timer, ^{
        os_log(OS_LOG_DEFAULT, "Sending echo...");
        NSError * error;
        if (![self->_chipController sendMessage:[kEchoString dataUsingEncoding:NSUTF8StringEncoding] error:&error]) {
            os_log(OS_LOG_DEFAULT, "Failed to send with error %@", error);
        }

    });
    dispatch_activate(_send_timer);
    return YES;
}

- (BOOL)start:(NSError * __autoreleasing *)error {
    return [self start:error frequency:kDefaultFrequencySeconds];
}

- (BOOL)start:(NSError * __autoreleasing *)error frequency:(UInt32)frequency stopAfter:(UInt32)stopAfter {
    BOOL result = [self start:error frequency:frequency];
    dispatch_after(dispatch_walltime(nil, stopAfter * NSEC_PER_SEC), _echoCallbackQueue, ^{
        [self stop];
    });
    return result;
}

- (void)stop {
    dispatch_suspend(self->_send_timer);
    [self->_chipController disconnect:nil];
    os_log(OS_LOG_DEFAULT, "Echo client stopped...");
}

@end
