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

#import "UnpairDevicesViewController.h"
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import "DeviceSelector.h"
#import <Matter/Matter.h>

@interface UnpairDevicesViewController ()
@property (strong, nonatomic) UISwitch * unpairAllDevicesSwitch;
@property (nonatomic, strong) UILabel * unpairLabel;
@property (nonatomic, strong) UILabel * titleLabel;
@property (nonatomic, strong) UIStackView * stackView;
@property (nonatomic, strong) DeviceSelector * deviceSelector;
@property (strong, nonatomic) UIButton * unpairButton;
@end

@implementation UnpairDevicesViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self setupUIElements];
}

- (void)dismissKeyboard
{
    [_deviceSelector resignFirstResponder];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    _titleLabel = [CHIPUIViewUtils addTitle:@"Unpair Devices" toView:self.view];
    [self setupStackView];
}

- (void)setupStackView
{
    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionEqualSpacing;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 30;
    [_stackView removeFromSuperview];
    _stackView = stackView;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:_titleLabel.bottomAnchor constant:30].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // Unpair All Devices
    UILabel * unpairAllDevices = [UILabel new];
    unpairAllDevices.text = @"Unpair all devices";
    _unpairAllDevicesSwitch = [UISwitch new];
    [_unpairAllDevicesSwitch setOn:YES];
    UIView * openPairingOnAllDevicesView = [CHIPUIViewUtils viewWithLabel:unpairAllDevices toggle:_unpairAllDevicesSwitch];
    [_unpairAllDevicesSwitch addTarget:self action:@selector(unpairAllDevicesButton:) forControlEvents:UIControlEventTouchUpInside];
    [stackView addArrangedSubview:openPairingOnAllDevicesView];
    openPairingOnAllDevicesView.translatesAutoresizingMaskIntoConstraints = false;
    [openPairingOnAllDevicesView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Device List and picker
    _deviceSelector = [DeviceSelector new];
    [_deviceSelector setEnabled:NO];

    UILabel * deviceIDLabel = [UILabel new];
    deviceIDLabel.text = @"Device ID:";
    UIView * deviceIDView = [CHIPUIViewUtils viewWithLabel:deviceIDLabel textField:_deviceSelector];
    [stackView addArrangedSubview:deviceIDView];

    deviceIDView.translatesAutoresizingMaskIntoConstraints = false;
    [deviceIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = true;

    // Unpair devices button
    _unpairButton = [UIButton new];
    [_unpairButton setTitle:@"Unpair Devices" forState:UIControlStateNormal];
    [_unpairButton addTarget:self action:@selector(unpairSelectedDevices:) forControlEvents:UIControlEventTouchUpInside];
    _unpairButton.backgroundColor = UIColor.systemBlueColor;
    _unpairButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _unpairButton.titleLabel.textColor = [UIColor whiteColor];
    _unpairButton.layer.cornerRadius = 5;
    _unpairButton.clipsToBounds = YES;
    [stackView addArrangedSubview:_unpairButton];

    _unpairButton.translatesAutoresizingMaskIntoConstraints = false;
    [_unpairButton.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
}

// MARK: UIButton actions

- (IBAction)unpairAllDevicesButton:(id)sender
{
    if ([_unpairAllDevicesSwitch isOn]) {
        [_deviceSelector setEnabled:NO];
    } else {
        [_deviceSelector setEnabled:YES];
    }
}

- (IBAction)unpairSelectedDevices:(id)sender
{
    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        MTRUnpairDeviceWithID(deviceId);
    }];

    [_deviceSelector refreshDeviceList];
    //[_deviceSelector setEnabled:NO];
}

@end
