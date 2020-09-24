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

#import "CHIPConnectivityManager.h"
#import <UIKit/UIKit.h>
#import <CHIP/CHIP.h>
#import "DefaultsUtils.h"

@interface CHIPConnectivityManager()
@property (readwrite) CHIPDeviceController * chipController;
@property (nonatomic, weak) id <CHIPConnectivityManagerDelegate> delegate;
@property (readonly, nonatomic) dispatch_queue_t delegateQueue;
@end

@implementation CHIPConnectivityManager {
    BOOL reconnectOnForeground;
}


+ (CHIPConnectivityManager *)sharedManager
{
    static CHIPConnectivityManager * manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        // initialize the device controller
        manager = [[CHIPConnectivityManager alloc] init];
    });
    return manager;
}

- (void)setDelegate:(id<CHIPConnectivityManagerDelegate>)delegate queue:(dispatch_queue_t)queue
{
    if (delegate && queue) {
        _delegate = delegate;
        _delegateQueue = queue;
    } else {
        _delegate = nil;
        _delegateQueue = NULL;
    }
}

- (instancetype)init
{
    if (self = [super init]) {
        self.chipController = [CHIPDeviceController sharedController];

        // need to restart connections on background/foreground transitions otherwise the socket can be closed without CHIP knowing
        // about it.
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_appEnteredBackground:)
                                                     name:UISceneDidEnterBackgroundNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_appEnteringForeground:)
                                                     name:UISceneWillEnterForegroundNotification
                                                   object:nil];
    }
    return self;
}

- (NSString *)_getScannedIP
{
    return CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey);
}

- (void)_appEnteredBackground:(NSNotification *)notification
{
    if ([self.chipController isConnected]) {
        [self.chipController disconnect:nil];
        reconnectOnForeground = YES;
    }
}

- (void)_appEnteringForeground:(NSNotification *)notification
{
    if (reconnectOnForeground) {
        [self _connect];
    }
}

- (void)_connect
{
    NSError * error;

    NSString * inputIPAddress = [self _getScannedIP];

    if ([self.chipController isConnected]) {
        [self.chipController disconnect:nil];
    }

    BOOL didConnect = [self.chipController connect:inputIPAddress error:&error];
    if (!didConnect) {
        id<CHIPConnectivityManagerDelegate> strongDelegate = [self delegate];
        if (strongDelegate && [self delegateQueue]) {
            dispatch_async(self.delegateQueue, ^{
                [strongDelegate didReceiveConnectionError:error];
            });
        }
    }
}

- (void)reconnectIfNeeded
{
    // collect fields
    NSString * inputIPAddress = [self _getScannedIP];
    BOOL needsReconnect = NO;

    // check the addr of the connected device
    AddressInfo * addrInfo = [self.chipController getAddressInfo];
    // check if the addr changed
    if (![addrInfo.ip isEqualToString:inputIPAddress]) {
        NSError * error;
        // stop current connection
        if (![self.chipController disconnect:&error]) {
            // post error
            id<CHIPConnectivityManagerDelegate> strongDelegate = [self delegate];
            if (strongDelegate && [self delegateQueue]) {
                dispatch_async(self.delegateQueue, ^{
                    [strongDelegate didReceiveDisconnectionError:error];
                });
            }
        }
        needsReconnect = YES;
    }

    if (!addrInfo || needsReconnect) {
        [self _connect];
    }
}
@end
