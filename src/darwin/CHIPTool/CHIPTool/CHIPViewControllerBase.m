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

#import "CHIPViewControllerBase.h"

#import <CHIP/CHIP.h>
#import <UIKit/UIKit.h>

#define RESULT_DISPLAY_DURATION 5.0 * NSEC_PER_SEC

@interface CHIPViewControllerBase ()

@property (readwrite) dispatch_queue_t chipCallbackQueue;
@property (readwrite) BOOL reconnectOnForeground;

@property (weak, nonatomic) IBOutlet UITextField * serverIPTextField;
@property (weak, nonatomic) IBOutlet UITextField * serverPortTextField;

@end

@implementation CHIPViewControllerBase

- (void)viewDidLoad
{
    [super viewDidLoad];

    // auto resize the results on screen
    self.resultLabel.adjustsFontSizeToFitWidth = YES;

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    self.chipCallbackQueue = dispatch_queue_create("com.zigbee.chip.example.callback", DISPATCH_QUEUE_SERIAL);
    // initialize the device controller
    self.chipController = [[CHIPDeviceController alloc] initWithCallbackQueue:self.chipCallbackQueue];

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

- (void)_appEnteredBackground:(NSNotification *)notification
{
    if ([self.chipController isConnected]) {
        [self.chipController disconnect:nil];
        self.reconnectOnForeground = YES;
    }
}

- (void)_appEnteringForeground:(NSNotification *)notification
{
    if (self.reconnectOnForeground) {
        [self _connect];
    }
}

- (void)_connect
{
    NSError * error;
    NSString * inputIPAddress = self.serverIPTextField.text;
    UInt16 inputPort = [self.serverPortTextField.text intValue];
    BOOL didConnect = [self.chipController connect:inputIPAddress
        port:inputPort
        error:&error
        onMessage:^(NSData * _Nonnull message, NSString * _Nonnull ipAddress, UInt16 port) {
            NSString * strMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
            [self postResult:[@"Echo Response: " stringByAppendingFormat:@"%@\nFrom: %@:%d", strMessage, ipAddress, port]];
        }
        onError:^(NSError * _Nonnull error) {
            [self postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
        }];
    if (!didConnect) {
        [self postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
    }
}

- (void)dismissKeyboard
{
    [self.serverIPTextField resignFirstResponder];
    [self.serverPortTextField resignFirstResponder];
}

- (void)reconnectIfNeeded
{
    // collect fields
    NSString * inputIPAddress = self.serverIPTextField.text;
    UInt16 inputPort = [self.serverPortTextField.text intValue];
    BOOL needsReconnect = NO;

    // check the addr of the connected device
    AddressInfo * addrInfo = [self.chipController getAddressInfo];
    if (addrInfo) {
        // check if the addr changed
        if (![addrInfo.ip isEqualToString:inputIPAddress] || addrInfo.port != inputPort) {
            NSError * error;
            // stop current connection
            if (![self.chipController disconnect:&error]) {
                // post error
                [self postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
            }
            needsReconnect = YES;
        }
    }

    if (!addrInfo || needsReconnect) {
        [self _connect];
    }
}

- (void)postResult:(NSString *)result
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self.resultLabel.text = result;
        self.resultLabel.hidden = NO;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, RESULT_DISPLAY_DURATION), dispatch_get_main_queue(), ^{
            self.resultLabel.hidden = YES;
        });
    });
}

@end
