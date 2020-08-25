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

@interface CHIPViewControllerBase () <CHIPDeviceControllerDelegate>

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
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.example.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];

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

    if ([self.chipController isConnected]) {
        [self.chipController disconnect:nil];
    }

    BOOL didConnect = [self.chipController connect:inputIPAddress error:&error];
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

- (void)postResult:(NSString *)resultMsg
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self.resultLabel.text = [@"Echo Response: " stringByAppendingFormat:@"%@", resultMsg];
        self.resultLabel.hidden = NO;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, RESULT_DISPLAY_DURATION), dispatch_get_main_queue(), ^{
            self.resultLabel.hidden = YES;
        });

        if (self.onMessageBlock) {
            self.onMessageBlock(resultMsg);
        }
    });
}

- (void)postError:(NSString *)errorMsg
{
    dispatch_async(dispatch_get_main_queue(), ^{
        self.resultLabel.text = [@"Error: " stringByAppendingString:errorMsg];
        self.resultLabel.hidden = NO;
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, RESULT_DISPLAY_DURATION), dispatch_get_main_queue(), ^{
            self.resultLabel.hidden = YES;
        });

        if (self.onErrorBlock) {
            self.onErrorBlock(errorMsg);
        }
    });
}

- (void)postConnected
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.onConnectedBlock) {
            self.onConnectedBlock();
        }
    });
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    NSError * error = nil;
    // This disconnect is needed to make sure the connection goes away.
    // The VC deallocation doesnt sometimes happen right away.
    // So if one goes back out of the VC and back in, and send an echo msg right away, then the first reponse at times gets dropped
    // as the first VC was not deallocated on time.
    [self.chipController disconnect:&error];
    if (error) {
        NSLog(@"Error disconnecting on view disappearing %@", error);
    }
}

#if 0
#pragma mark -
#pragma mark == CHIPDeviceControllerDelegate Methods ==
#endif

- (void)deviceControllerOnConnected
{
    [self postConnected];
}

- (void)deviceControllerOnMessage:(NSData *)message
{
    if ([CHIPDeviceController isDataModelCommand:message] == YES) {
        NSString * strMessage = [CHIPDeviceController commandToString:message];
        [self postResult:strMessage];
    } else {
        NSString * strMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
        [self postResult:strMessage];
    }
}

- (void)deviceControllerOnError:(NSError *)error
{
    [self postError:error.localizedDescription];
}

@end
