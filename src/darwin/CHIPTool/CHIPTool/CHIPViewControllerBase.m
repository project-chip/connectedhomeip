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

static NSString * const ipKey = @"ipk";

@interface CHIPViewControllerBase ()

@property (readwrite) dispatch_queue_t chipCallbackQueue;
@property (readwrite) BOOL reconnectOnForeground;

@property (weak, nonatomic) IBOutlet UITextField * serverIPTextField;
@property (weak, nonatomic) IBOutlet UILabel * IPLabel;

@end

@implementation CHIPViewControllerBase

- (NSString *)_getScannedIP
{
    return [[NSUserDefaults standardUserDefaults] stringForKey:ipKey];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    // auto resize the results on screen
    self.resultLabel.adjustsFontSizeToFitWidth = YES;

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    // initialize the device controller
    __weak typeof(self) weakSelf = self;
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.example.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController registerCallbacks:callbackQueue
        onMessage:^(NSData * _Nonnull message, NSString * _Nonnull ipAddress, UInt16 port) {
            typeof(self) strongSelf = weakSelf;
            NSString * strMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
            [strongSelf postResult:[@"Echo Response: " stringByAppendingFormat:@"%@", strMessage]];
        }
        onError:^(NSError * _Nonnull error) {
            typeof(self) strongSelf = weakSelf;
            [strongSelf postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
        }];

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

    BOOL shouldHide = NO;
    if ([[self _getScannedIP] length] > 0) {
        shouldHide = YES;
    }
    [self.serverIPTextField setHidden:shouldHide];
    [self.IPLabel setHidden:shouldHide];
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

    NSString * inputIPAddress = [[self _getScannedIP] length] > 0 ? [self _getScannedIP] : self.serverIPTextField.text;

    const unsigned char local_key_bytes[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f, 0x11, 0x0e, 0x34,
        0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65, 0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    const unsigned char peer_key_bytes[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd, 0xfb, 0x1f, 0xcc,
        0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3, 0xab, 0xe4, 0x33, 0xb6, 0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34, 0x92,
        0x73, 0x14, 0x59, 0x0b, 0xbd, 0x44, 0x72, 0x1b, 0xcd, 0xb9, 0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae, 0xe8, 0x87,
        0x2e, 0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

    NSData * peer_key = [NSData dataWithBytes:peer_key_bytes length:sizeof(peer_key_bytes)];
    NSData * local_key = [NSData dataWithBytes:local_key_bytes length:sizeof(local_key_bytes)];

    BOOL didConnect = [self.chipController connect:inputIPAddress local_key:local_key peer_key:peer_key error:&error];
    if (!didConnect) {
        [self postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
    }
}

- (void)dismissKeyboard
{
    [self.serverIPTextField resignFirstResponder];
}

- (void)reconnectIfNeeded
{
    // collect fields
    NSString * inputIPAddress = self.serverIPTextField.text;
    BOOL needsReconnect = NO;
    // Don't rely on the text fields if we have scanned connection info from a QRCode
    BOOL hasScannedConnectionInfo = ([[self _getScannedIP] length] > 0);

    // check the addr of the connected device
    AddressInfo * addrInfo = [self.chipController getAddressInfo];
    if (addrInfo && !hasScannedConnectionInfo) {
        // check if the addr changed
        if (![addrInfo.ip isEqualToString:inputIPAddress]) {
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
