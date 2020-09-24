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

#import "OnOffViewController.h"
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import <CHIP/CHIP.h>

@interface OnOffViewController ()
@property (weak, nonatomic) IBOutlet UIButton * onButton;
@property (weak, nonatomic) IBOutlet UIButton * offButton;
@property (weak, nonatomic) IBOutlet UIButton * toggleButton;
@property (readwrite) CHIPOnOff * onOff;

@property (strong, nonatomic) UITextField * serverIPTextField;
@property (strong, nonatomic) UIButton * serverIPDoneButton;

@property (nonatomic, strong) UILabel * resultLabel;

@property (readwrite) CHIPDeviceController * chipController;
@property (readwrite) CHIPConnectivityManager * connectivityManager;

@end

@implementation OnOffViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUIElements];

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    // initialize the device controller
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.onoffvc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];
    self.connectivityManager = [CHIPConnectivityManager sharedManager];
    [self.connectivityManager setDelegate:self queue:callbackQueue];

    self.onOff = [[CHIPOnOff alloc] initWithDeviceController:self.chipController];
}

- (void)dismissKeyboard
{
    [self.serverIPTextField resignFirstResponder];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"On Off Cluster" toView:self.view];

    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionEqualSpacing;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 30;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:30].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

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
    [stackView addArrangedSubview:serverIPView];
    serverIPView.translatesAutoresizingMaskIntoConstraints = false;
    [serverIPView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Button stack view
    UIStackView * stackViewButtons = [UIStackView new];
    stackViewButtons.axis = UILayoutConstraintAxisHorizontal;
    stackViewButtons.distribution = UIStackViewDistributionEqualSpacing;
    stackViewButtons.alignment = UIStackViewAlignmentLeading;
    stackViewButtons.spacing = 10;
    [stackView addArrangedSubview:stackViewButtons];

    // Create buttons
    UIButton * onButton = [UIButton new];
    [onButton setTitle:@"On" forState:UIControlStateNormal];
    [onButton addTarget:self action:@selector(onButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
    UIButton * offButton = [UIButton new];
    [offButton setTitle:@"Off" forState:UIControlStateNormal];
    [offButton addTarget:self action:@selector(offButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
    UIButton * toggleButton = [UIButton new];
    [toggleButton setTitle:@"Toggle" forState:UIControlStateNormal];
    [toggleButton addTarget:self action:@selector(toggleButtonTapped:) forControlEvents:UIControlEventTouchUpInside];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    NSArray<UIButton *> * buttons = @[ onButton, offButton, toggleButton ];
    for (int i = 0; i < buttons.count; i++) {
        UIButton * buttonForStack = [buttons objectAtIndex:i];
        buttonForStack.backgroundColor = UIColor.systemBlueColor;
        buttonForStack.titleLabel.font = [UIFont systemFontOfSize:17];
        buttonForStack.titleLabel.textColor = [UIColor whiteColor];
        buttonForStack.layer.cornerRadius = 5;
        buttonForStack.clipsToBounds = YES;
        buttonForStack.translatesAutoresizingMaskIntoConstraints = false;
        [buttonForStack.widthAnchor constraintGreaterThanOrEqualToConstant:60].active = YES;
        [stackViewButtons addArrangedSubview:buttonForStack];
    }

    // Result message
    _resultLabel = [UILabel new];
    _resultLabel.hidden = YES;
    _resultLabel.font = [UIFont systemFontOfSize:17];
    _resultLabel.textColor = UIColor.systemBlueColor;
    _resultLabel.lineBreakMode = NSLineBreakByWordWrapping;
    _resultLabel.numberOfLines = 0;
    [stackView addArrangedSubview:_resultLabel];

    _resultLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_resultLabel.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
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

- (IBAction)onButtonTapped:(id)sender
{
    [self.connectivityManager reconnectIfNeeded];

    [self.onOff lightOn];
}

- (IBAction)offButtonTapped:(id)sender
{
    [self.connectivityManager reconnectIfNeeded];

    [self.onOff lightOff];
}

- (IBAction)toggleButtonTapped:(id)sender
{
    [self.connectivityManager reconnectIfNeeded];

    [self.onOff toggleLight];
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
