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

#import "EchoViewController.h"

#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"

#define RESULT_DISPLAY_DURATION 5.0 * NSEC_PER_SEC

@interface EchoViewController ()

@property (strong, nonatomic) UITextField * messageTextField;
@property (strong, nonatomic) UIButton * sendButton;

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UIStackView * stackView;

@property (readwrite) CHIPDeviceController * chipController;
@property (readwrite) CHIPDevice * chipDevice;

@property (readonly) CHIPToolPersistentStorageDelegate * persistentStorage;

@end

@implementation EchoViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUIElements];

    _persistentStorage = [[CHIPToolPersistentStorageDelegate alloc] init];

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    // initialize the device controller
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.echovc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];
    [self.chipController setPersistentStorageDelegate:_persistentStorage queue:callbackQueue];

    uint64_t deviceID = CHIPGetNextAvailableDeviceID();
    if (deviceID > 1) {
        // Let's use the last device that was paired
        deviceID--;
        NSError * error;
        self.chipDevice = [self.chipController getPairedDevice:deviceID error:&error];
    }
}

- (void)dismissKeyboard
{
    [self.messageTextField resignFirstResponder];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Echo client" toView:self.view];

    // stack view
    _stackView = [UIStackView new];
    _stackView.axis = UILayoutConstraintAxisVertical;
    _stackView.distribution = UIStackViewDistributionEqualSpacing;
    _stackView.alignment = UIStackViewAlignmentLeading;
    _stackView.spacing = 30;
    [self.view addSubview:_stackView];

    _stackView.translatesAutoresizingMaskIntoConstraints = false;
    [_stackView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:30].active = YES;
    [_stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // Send message
    _messageTextField = [UITextField new];
    _messageTextField.placeholder = @"Hello from iOS!";
    _sendButton = [UIButton new];
    [_sendButton setTitle:@"Send" forState:UIControlStateNormal];
    [_sendButton addTarget:self action:@selector(sendMessage:) forControlEvents:UIControlEventTouchUpInside];
    UIView * sendMessageView = [CHIPUIViewUtils viewWithUITextField:_messageTextField button:_sendButton];
    [_stackView addArrangedSubview:sendMessageView];
    sendMessageView.translatesAutoresizingMaskIntoConstraints = false;
    [sendMessageView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;

    // Result message
    _resultLabel = [UILabel new];
    _resultLabel.hidden = YES;
    _resultLabel.font = [UIFont systemFontOfSize:17];
    _resultLabel.textColor = UIColor.systemBlueColor;
    _resultLabel.lineBreakMode = NSLineBreakByWordWrapping;
    _resultLabel.numberOfLines = 0;
    [_stackView addArrangedSubview:_resultLabel];

    _resultLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_resultLabel.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    _resultLabel.adjustsFontSizeToFitWidth = YES;
}

- (void)updateResult:(NSString *)result
{
    _resultLabel.hidden = NO;
    _resultLabel.text = result;
}

// MARK: UIButton actions

- (IBAction)sendMessage:(id)sender
{
    NSString * msg = [self.messageTextField text];
    if (msg.length == 0) {
        msg = [self.messageTextField placeholder];
    }

    // send message
    if ([self.chipDevice isActive]) {
        NSError * error;
        BOOL didSend = [self.chipDevice sendMessage:[msg dataUsingEncoding:NSUTF8StringEncoding] error:&error];
        if (!didSend) {
            NSString * errorString = [@"Error: " stringByAppendingString:error.localizedDescription];
            [self updateResult:errorString];
        } else {
            [self updateResult:@"Message Sent"];
        }
    } else {
        [self updateResult:@"Controller not connected"];
    }
}

// MARK: CHIPDeviceControllerDelegate
- (void)deviceControllerOnConnected
{
    NSLog(@"Status: Device connected");
}

- (void)deviceControllerOnError:(nonnull NSError *)error
{
    NSLog(@"Status: Device Controller error %@", [error description]);
    if (error) {
        NSString * stringError = [@"Error: " stringByAppendingString:error.description];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5.0), dispatch_get_main_queue(), ^{
            [self updateResult:stringError];
        });
    }
}

- (void)deviceControllerOnMessage:(nonnull NSData *)message
{
    NSString * stringMessage;
    if ([CHIPDevice isDataModelCommand:message] == YES) {
        stringMessage = [CHIPDevice commandToString:message];
    } else {
        stringMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
    }
    NSString * resultMessage = [@"Echo Response: " stringByAppendingFormat:@"%@", stringMessage];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5.0), dispatch_get_main_queue(), ^{
        [self updateResult:resultMessage];
    });
}

@end
