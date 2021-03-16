/*
 *   Copyright (c) 2021 Project CHIP Authors
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

#import <CHIP/CHIPDevice.h>
#import <CHIP/CHIPDeviceController.h>
#import <CHIP/CHIPDevicePairingDelegate.h>
#import <CHIP/CHIPError.h>
#import <CHIP/CHIPManualSetupPayloadParser.h>
#import <CHIP/CHIPPersistentStorageDelegate.h>
#import <CHIP/CHIPQRCodeSetupPayloadParser.h>
#import <CHIP/CHIPSetupPayload.h>
#import <CHIP/gen/CHIPClustersObjc.h>
#import <Foundation/Foundation.h>

@interface CHIPToolPersistentStorageDelegate : NSObject <CHIPPersistentStorageDelegate>
@end

@implementation CHIPToolPersistentStorageDelegate
- (void)CHIPGetKeyValue:(NSString *)key handler:(SendKeyValue)completionHandler
{
}

- (NSString *)CHIPGetKeyValue:(NSString *)key
{
    return @"";
}

- (void)CHIPSetKeyValue:(NSString *)key value:(NSString *)value handler:(CHIPSendSetStatus)completionHandler
{
}

- (void)CHIPDeleteKeyValue:(NSString *)key handler:(CHIPSendDeleteStatus)completionHandler
{
}
@end

int main(int argc, const char * argv[])
{
    static dispatch_queue_t callbackQueue;
    static CHIPToolPersistentStorageDelegate * storage = nil;
    static dispatch_once_t onceToken;
    CHIPDeviceController * controller = [CHIPDeviceController sharedController];

    dispatch_once(&onceToken, ^{
        storage = [[CHIPToolPersistentStorageDelegate alloc] init];
        callbackQueue = dispatch_queue_create("com.chip.persistentstorage.callback", DISPATCH_QUEUE_SERIAL);
        [controller startup:storage queue:callbackQueue];
    });

    NSError * error;
    CHIPDevice * device = [controller getPairedDevice:1 error:&error];

    CHIPBasic * basicCluster = [[CHIPBasic alloc] initWithDevice:device endpoint:1 queue:dispatch_get_main_queue()];

    [basicCluster mfgSpecificPing:^(NSError * err, NSDictionary * values) {
        NSString * resultString = (err == nil) ? @"MfgSpecificPing command: success!"
                                               : [NSString stringWithFormat:@"An error occured: 0x%02lx", err.code];
        (void) resultString;
    }];

    return 0;
}
