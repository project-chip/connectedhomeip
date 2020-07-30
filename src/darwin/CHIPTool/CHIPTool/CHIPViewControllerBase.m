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
        onConnected:^(void) {
            typeof(self) strongSelf = weakSelf;
            [strongSelf postConnected];
        }
        onMessage:^(NSData * _Nonnull message) {
            typeof(self) strongSelf = weakSelf;
            if ([CHIPDeviceController isDataModelCommand:message] == YES) {
                NSString * strMessage = [CHIPDeviceController commandToString:message];
                [strongSelf postResult:strMessage];
            } else {
                NSString * strMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
                [strongSelf postResult:strMessage];
            }
        }
        onError:^(NSError * _Nonnull error) {
            typeof(self) strongSelf = weakSelf;
            [strongSelf postError:error.localizedDescription];
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
    self.useIncorrectKey = NO;
    self.useIncorrectKeyStateChanged = NO;
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

    const unsigned char local_key_bytes[] = { 0x00, 0xd1, 0x90, 0xd9, 0xb3, 0x95, 0x1c, 0x5f, 0xa4, 0xe7, 0x47, 0x92, 0x5b, 0x0a,
        0xa9, 0xa7, 0xc1, 0x1c, 0xe7, 0x06, 0x10, 0xe2, 0xdd, 0x16, 0x41, 0x52, 0x55, 0xb7, 0xb8, 0x80, 0x8d, 0x87, 0xa1 };

    const unsigned char peer_key_bytes[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4, 0x3c,
        0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7, 0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24,
        0xc5, 0xd4, 0x5c, 0x60, 0x79, 0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf, 0x9f, 0xdc,
        0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

    const unsigned char local_incorrect_key_bytes[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f, 0x11,
        0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65, 0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

    NSData * peer_key = [NSData dataWithBytes:peer_key_bytes length:sizeof(peer_key_bytes)];
    NSData * local_key = NULL;
    if (!self.useIncorrectKey) {
        NSLog(@"Using correct key");
        local_key = [NSData dataWithBytes:local_key_bytes length:sizeof(local_key_bytes)];
    } else {
        NSLog(@"Using incorrect key");
        local_key = [NSData dataWithBytes:local_incorrect_key_bytes length:sizeof(local_incorrect_key_bytes)];
    }

    if ([self.chipController isConnected]) {
        [self.chipController disconnect:nil];
    }

    BOOL didConnect = [self.chipController connect:inputIPAddress local_key:local_key peer_key:peer_key error:&error];
    if (!didConnect) {
        [self postResult:[@"Error: " stringByAppendingString:error.localizedDescription]];
    }
}

- (IBAction)encryptionKey:(id)sender
{
    if ([self.encryptionKeySwitch isOn]) {
        self.useIncorrectKey = YES;
        [self postResult:@"App will use incorrect key"];
    } else {
        self.useIncorrectKey = NO;
        [self postResult:@"App will use correct key"];
    }
    self.useIncorrectKeyStateChanged = YES;
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

    if (!addrInfo || needsReconnect || self.useIncorrectKeyStateChanged) {
        [self _connect];
        self.useIncorrectKeyStateChanged = NO;
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

@end
