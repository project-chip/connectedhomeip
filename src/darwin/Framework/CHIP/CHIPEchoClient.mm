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

@interface CHIPEchoClient()  {
    dispatch_queue_t _echoCallbackQueue;
    dispatch_source_t _sendTimer;
    CHIPDeviceController* _chipController;
}

@property (readonly) dispatch_queue_t echoCallbackQueue;
@property (readonly) dispatch_source_t sendTimer;
@property (readonly) CHIPDeviceController* chipController;

@end

@implementation CHIPEchoClient

- (instancetype)initWithServerAddress:(NSString *)ipAddress port:(UInt16)port {
    if (self = [super init]) {
        NSError * error;
        _echoCallbackQueue = dispatch_queue_create("com.zigbee.chip.echo", NULL);
        _chipController = [[CHIPDeviceController alloc] initWithCallbackQueue:_echoCallbackQueue];

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
        _sendTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, _echoCallbackQueue);
        if (!_sendTimer) {
            os_log(OS_LOG_DEFAULT, "Could not create dispatch source");
            return nil;
        }

    }
    return self;
}

- (BOOL)startWithFrequency:(UInt32)frequency withError:(NSError * __autoreleasing *)error  {
    dispatch_source_set_timer(self.sendTimer, dispatch_walltime(NULL, 0), frequency * NSEC_PER_SEC, 1 * NSEC_PER_SEC);
    dispatch_source_set_event_handler(self.sendTimer, ^{
        os_log(OS_LOG_DEFAULT, "Sending echo...");
        NSError * error;
        if (![self.chipController sendMessage:[kEchoString dataUsingEncoding:NSUTF8StringEncoding] error:&error]) {
            os_log(OS_LOG_DEFAULT, "Failed to send with error %@", error);
        }

    });
    dispatch_activate(self.sendTimer);
    return YES;
}

- (BOOL)startWithError:(NSError * __autoreleasing *)error {
    return [self startWithFrequency:kDefaultFrequencySeconds withError:error];
}

- (BOOL)startWithFrequency:(UInt32)frequency stopAfter:(UInt32)stopAfter withError:(NSError * __autoreleasing *)error {
    BOOL result = [self startWithFrequency:frequency withError:error];
    dispatch_after(dispatch_walltime(nil, stopAfter * NSEC_PER_SEC), self.echoCallbackQueue, ^{
        [self stop];
    });
    return result;
}

- (void)stop {
    dispatch_suspend(self->_sendTimer);
    [self.chipController disconnect:nil];
    os_log(OS_LOG_DEFAULT, "Echo client stopped...");
}

- (void)dealloc {
    [self stop];
}

@end
