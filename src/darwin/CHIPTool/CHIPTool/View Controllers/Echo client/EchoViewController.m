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

@property (strong, nonatomic) UITextField * serverIPTextField;
@property (strong, nonatomic) UIButton * serverIPDoneButton;

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UIStackView * stackView;

@property (readwrite) CHIPDeviceController * chipController;
@property (readwrite) CHIPConnectivityManager * connectivityManager;

@end

@implementation EchoViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUIElements];

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    // initialize the device controller
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.echovc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];
    self.connectivityManager = [CHIPConnectivityManager sharedManager];
    [self.connectivityManager setDelegate:self queue:callbackQueue];
}

- (void)dismissKeyboard
{
    [self.messageTextField resignFirstResponder];
    [self.serverIPTextField resignFirstResponder];
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

    // IP
    UILabel * serverIPLabel = [UILabel new];
    serverIPLabel.text = @"Server IP";
    _serverIPTextField = [UITextField new];
    _serverIPTextField.placeholder = @"Server IP";
    NSString * savedServerIP = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey);
    if ([savedServerIP length] > 0) {
        _serverIPTextField.text = savedServerIP;
    }
    _serverIPDoneButton = [UIButton new];
    [_serverIPDoneButton setTitle:@"Save" forState:UIControlStateNormal];
    [_serverIPDoneButton addTarget:self action:@selector(saveServerID:) forControlEvents:UIControlEventTouchUpInside];
    UIView * serverIPView = [CHIPUIViewUtils viewWithUITextField:_serverIPTextField button:_serverIPDoneButton];
    [_stackView addArrangedSubview:serverIPView];
    serverIPView.translatesAutoresizingMaskIntoConstraints = false;
    [serverIPView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;

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

- (IBAction)saveServerID:(id)sender
{
    NSString * serverIP = [self.serverIPTextField text];
    if ([serverIP length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey, serverIP);
    }
}

- (IBAction)sendMessage:(id)sender
{
    NSString * msg = [self.messageTextField text];
    if (msg.length == 0) {
        msg = [self.messageTextField placeholder];
    }

    [self.connectivityManager reconnectIfNeeded];

    // send message
    if ([self.chipController isConnected]) {
        NSError * error;
        BOOL didSend = [self.chipController sendMessage:[msg dataUsingEncoding:NSUTF8StringEncoding] error:&error];
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
    if ([CHIPDeviceController isDataModelCommand:message] == YES) {
        stringMessage = [CHIPDeviceController commandToString:message];
    } else {
        stringMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
    }
    NSString * resultMessage = [@"Echo Response: " stringByAppendingFormat:@"%@", stringMessage];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5.0), dispatch_get_main_queue(), ^{
        [self updateResult:resultMessage];
    });
}

// MARK: CHIPConnectivityManagerDelegate

- (void)didReceiveConnectionError:(nonnull NSError *)error
{
    NSLog(@"Status: Connection error %@", [error description]);
    if (error) {
        NSString * stringError = [@"Error: " stringByAppendingString:error.description];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5.0), dispatch_get_main_queue(), ^{
            [self updateResult:stringError];
        });
    }
}

- (void)didReceiveDisconnectionError:(nonnull NSError *)error
{
    NSLog(@"Status: Disconnection error %@", [error description]);
    if (error) {
        NSString * stringError = [@"Error: " stringByAppendingString:error.description];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5.0), dispatch_get_main_queue(), ^{
            [self updateResult:stringError];
        });
    }
}

@end
