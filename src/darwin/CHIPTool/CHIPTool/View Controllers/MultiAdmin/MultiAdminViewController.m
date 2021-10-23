/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#import "MultiAdminViewController.h"

#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import "DeviceSelector.h"

static NSString * const DEFAULT_TIMEOUT = @"900";
static NSString * const DEFAULT_DISCRIMINATOR = @"3840";

@interface MultiAdminViewController ()

@property (strong, nonatomic) UISwitch * openPairingOnAllDevices;
@property (strong, nonatomic) UISwitch * useOnboardingTokenSwitch;
@property (strong, nonatomic) UITextField * discriminatorField;
@property (strong, nonatomic) UITextField * timeoutField;
@property (strong, nonatomic) UIButton * openPairingWindowButton;

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UIStackView * stackView;

@property (nonatomic, strong) DeviceSelector * deviceSelector;
@end

@implementation MultiAdminViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUIElements];

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];
}

- (void)dismissKeyboard
{
    [self.discriminatorField resignFirstResponder];
    [self.timeoutField resignFirstResponder];
    [self.deviceSelector resignFirstResponder];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Open Pairing Window" toView:self.view];

    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionFill;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 30;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:40].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // Open pairing on all devices
    UILabel * pairAllDevices = [UILabel new];
    pairAllDevices.text = @"Enable pairing on all devices";
    _openPairingOnAllDevices = [UISwitch new];
    [_openPairingOnAllDevices setOn:YES];
    UIView * openPairingOnAllDevicesView = [CHIPUIViewUtils viewWithLabel:pairAllDevices toggle:_openPairingOnAllDevices];
    [_openPairingOnAllDevices addTarget:self action:@selector(pairAllDevicesButton:) forControlEvents:UIControlEventTouchUpInside];
    [stackView addArrangedSubview:openPairingOnAllDevicesView];
    openPairingOnAllDevicesView.translatesAutoresizingMaskIntoConstraints = false;
    [openPairingOnAllDevicesView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Device List and selector
    _deviceSelector = [DeviceSelector new];
    [_deviceSelector setEnabled:NO];

    UILabel * deviceIDLabel = [UILabel new];
    deviceIDLabel.text = @"Device ID:";
    UIView * deviceIDView = [CHIPUIViewUtils viewWithLabel:deviceIDLabel textField:_deviceSelector];
    [stackView addArrangedSubview:deviceIDView];

    deviceIDView.translatesAutoresizingMaskIntoConstraints = false;
    [deviceIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = true;

    // Open Pairing Window
    UILabel * useOnboardingToken = [UILabel new];
    useOnboardingToken.text = @"Use Onboarding Token";
    _useOnboardingTokenSwitch = [UISwitch new];
    [_useOnboardingTokenSwitch setOn:YES];
    UIView * useOnboardingTokenView = [CHIPUIViewUtils viewWithLabel:useOnboardingToken toggle:_useOnboardingTokenSwitch];
    [_useOnboardingTokenSwitch addTarget:self action:@selector(overrideControls:) forControlEvents:UIControlEventTouchUpInside];
    [stackView addArrangedSubview:useOnboardingTokenView];
    useOnboardingTokenView.translatesAutoresizingMaskIntoConstraints = false;
    [useOnboardingTokenView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Discriminator
    _discriminatorField = [UITextField new];
    _discriminatorField.keyboardType = UIKeyboardTypeNumberPad;
    _discriminatorField.placeholder = DEFAULT_DISCRIMINATOR;
    UILabel * discriminatorLabel = [UILabel new];
    [discriminatorLabel setText:@"Discriminator"];
    UIView * discriminatorView = [CHIPUIViewUtils viewWithLabel:discriminatorLabel textField:_discriminatorField];
    [stackView addArrangedSubview:discriminatorView];

    discriminatorView.translatesAutoresizingMaskIntoConstraints = false;
    [discriminatorView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Pairing Timeout
    _timeoutField = [UITextField new];
    _timeoutField.keyboardType = UIKeyboardTypeNumberPad;
    _timeoutField.placeholder = DEFAULT_TIMEOUT;
    UILabel * timeoutLabel = [UILabel new];
    [timeoutLabel setText:@"Timeout after (seconds)"];
    UIView * timeoutView = [CHIPUIViewUtils viewWithLabel:timeoutLabel textField:_timeoutField];
    [stackView addArrangedSubview:timeoutView];

    timeoutView.translatesAutoresizingMaskIntoConstraints = false;
    [timeoutView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Open Pairing Window button
    _openPairingWindowButton = [UIButton new];
    [_openPairingWindowButton setTitle:@"Open Pairing Window" forState:UIControlStateNormal];
    [_openPairingWindowButton addTarget:self action:@selector(openPairingWindow:) forControlEvents:UIControlEventTouchUpInside];
    _openPairingWindowButton.backgroundColor = UIColor.systemBlueColor;
    _openPairingWindowButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _openPairingWindowButton.titleLabel.textColor = [UIColor whiteColor];
    _openPairingWindowButton.layer.cornerRadius = 5;
    _openPairingWindowButton.clipsToBounds = YES;
    [stackView addArrangedSubview:_openPairingWindowButton];

    _openPairingWindowButton.translatesAutoresizingMaskIntoConstraints = false;
    [_openPairingWindowButton.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

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

- (IBAction)pairAllDevicesButton:(id)sender
{
    if ([_openPairingOnAllDevices isOn]) {
        [_deviceSelector setEnabled:NO];
    } else {
        [_deviceSelector setEnabled:YES];
    }
}

- (IBAction)overrideControls:(id)sender
{
    if ([_useOnboardingTokenSwitch isOn]) {
        _discriminatorField.placeholder = DEFAULT_DISCRIMINATOR;
        _timeoutField.placeholder = DEFAULT_TIMEOUT;
        [_discriminatorField setEnabled:YES];
    } else {
        _discriminatorField.text = @"";
        _discriminatorField.placeholder = @"Original discriminator";
        [_discriminatorField setEnabled:NO];
    }
}

- (IBAction)openPairingWindow:(id)sender
{
    uint32_t setupPIN = arc4random();
    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        if (CHIPGetConnectedDeviceWithID(deviceId, ^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                if (chipDevice) {
                    NSString * timeoutStr = [self.timeoutField text];
                    if (timeoutStr.length == 0) {
                        timeoutStr = [self.timeoutField placeholder];
                    }
                    int timeout = [timeoutStr intValue];

                    NSString * output;
                    NSError * error;
                    if ([self.useOnboardingTokenSwitch isOn]) {
                        NSString * discriminatorStr = [self.discriminatorField text];
                        if (discriminatorStr.length == 0) {
                            discriminatorStr = [self.discriminatorField placeholder];
                        }
                        NSInteger discriminator = [discriminatorStr intValue];

                        output = [chipDevice openPairingWindowWithPIN:timeout
                                                        discriminator:discriminator
                                                             setupPIN:setupPIN
                                                                error:&error];

                        if (output != nil) {
                            NSString * result = [@"Use Manual Code: " stringByAppendingString:output];
                            [self updateResult:result];
                        } else {
                            [self updateResult:@"Failed in opening the pairing window"];
                        }
                    } else {
                        BOOL didSend = [chipDevice openPairingWindow:timeout error:&error];
                        if (didSend) {
                            [self updateResult:@"Scan the QR code on the device"];
                        } else {
                            NSString * errorString = [@"Error: " stringByAppendingString:error.localizedDescription];
                            [self updateResult:errorString];
                        }
                    }
                } else {
                    [self updateResult:[NSString stringWithFormat:@"Failed to establish a connection with the device"]];
                }
            })) {
            [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"]];
        } else {
            [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"]];
        }
    }];
}

@end
