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
#import "DeviceSelector.h"
#import <CHIP/CHIP.h>

NSString * const kCHIPNumLightOnOffCluster = @"OnOffViewController_NumLights";

@interface OnOffViewController ()
@property (nonatomic, strong) UITextField * numLightsTextField;

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UILabel * titleLabel;
@property (nonatomic, strong) UIStackView * stackView;

@property (nonatomic, strong) DeviceSelector * deviceSelector;

@end

@implementation OnOffViewController {
    dispatch_queue_t _callbackQueue;
    NSArray * _numLightsOptions;
}

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
    [_numLightsTextField resignFirstResponder];
    [_deviceSelector resignFirstResponder];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    _titleLabel = [CHIPUIViewUtils addTitle:@"On Off Cluster" toView:self.view];
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

    // Device List and picker
    _deviceSelector = [DeviceSelector new];

    UILabel * deviceIDLabel = [UILabel new];
    deviceIDLabel.text = @"Device ID:";
    UIView * deviceIDView = [CHIPUIViewUtils viewWithLabel:deviceIDLabel textField:_deviceSelector];
    [stackView addArrangedSubview:deviceIDView];

    deviceIDView.translatesAutoresizingMaskIntoConstraints = false;
    [deviceIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = true;

    // Num lights to show
    UILabel * numLightsLabel = [UILabel new];
    numLightsLabel.text = @"# of light endpoints:";
    _numLightsTextField = [UITextField new];
    _numLightsOptions = @[ @"1", @"2", @"3", @"4", @"5", @"6", @"7", @"8", @"9", @"10" ];
    _numLightsTextField.text
        = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kCHIPNumLightOnOffCluster) ?: [_numLightsOptions objectAtIndex:0];
    UIPickerView * numLightsPicker = [[UIPickerView alloc] initWithFrame:CGRectMake(0, 100, 0, 0)];
    _numLightsTextField.inputView = numLightsPicker;
    [numLightsPicker setDataSource:self];
    [numLightsPicker setDelegate:self];
    _numLightsTextField.delegate = self;
    UIView * numLightsView = [CHIPUIViewUtils viewWithLabel:numLightsLabel textField:_numLightsTextField];
    numLightsLabel.font = [UIFont systemFontOfSize:17 weight:UIFontWeightSemibold];

    UIToolbar * keyboardDoneButtonView = [[UIToolbar alloc] init];
    [keyboardDoneButtonView sizeToFit];
    UIBarButtonItem * doneButton = [[UIBarButtonItem alloc] initWithTitle:@"Done"
                                                                    style:UIBarButtonItemStylePlain
                                                                   target:self
                                                                   action:@selector(pickerDoneClicked:)];
    UIBarButtonItem * flexible = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                                                               target:self
                                                                               action:nil];
    [keyboardDoneButtonView setItems:[NSArray arrayWithObjects:flexible, doneButton, nil]];
    _numLightsTextField.inputAccessoryView = keyboardDoneButtonView;

    [stackView addArrangedSubview:numLightsView];
    numLightsView.translatesAutoresizingMaskIntoConstraints = false;
    [numLightsView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = true;

    // Create buttons
    [self addButtons:[self numLightClustersToShow] toStackView:stackView];

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

- (void)addButtons:(NSInteger)numButtons toStackView:(UIStackView *)stackView
{
    for (int i = 1; i <= numButtons; i++) {
        // Create buttons
        UILabel * labelLight = [UILabel new];
        labelLight.text = [NSString stringWithFormat:@"Light (endpoint %@): ", @(i)];
        UIButton * onButton = [UIButton new];
        onButton.tag = i;
        [onButton setTitle:@"On" forState:UIControlStateNormal];
        [onButton addTarget:self action:@selector(onButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
        UIButton * offButton = [UIButton new];
        offButton.tag = i;
        [offButton setTitle:@"Off" forState:UIControlStateNormal];
        [offButton addTarget:self action:@selector(offButtonTapped:) forControlEvents:UIControlEventTouchUpInside];
        UIButton * toggleButton = [UIButton new];
        toggleButton.tag = i;
        [toggleButton setTitle:@"Toggle" forState:UIControlStateNormal];
        [toggleButton addTarget:self action:@selector(toggleButtonTapped:) forControlEvents:UIControlEventTouchUpInside];

        UIStackView * stackViewButtons = [CHIPUIViewUtils stackViewWithLabel:labelLight
                                                                     buttons:@[ onButton, offButton, toggleButton ]];
        stackViewButtons.axis = UILayoutConstraintAxisHorizontal;
        stackViewButtons.distribution = UIStackViewDistributionEqualSpacing;
        stackViewButtons.alignment = UIStackViewAlignmentLeading;
        stackViewButtons.spacing = 10;
        [stackView addArrangedSubview:stackViewButtons];
    }
}

- (void)updateResult:(NSString *)result
{
    _resultLabel.hidden = NO;
    _resultLabel.text = result;
}

// MARK: UIPickerView

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
    NSLog(@"%@", [_numLightsOptions objectAtIndex:row]);
    _numLightsTextField.text = [NSString stringWithFormat:@"%@", [_numLightsOptions objectAtIndex:row]];
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component
{
    return [_numLightsOptions count];
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView
{
    return 1;
}

- (NSString *)pickerView:(UIPickerView *)pickerView titleForRow:(NSInteger)row forComponent:(NSInteger)component
{
    return [_numLightsOptions objectAtIndex:row];
}

// tell the picker the width of each row for a given component
- (CGFloat)pickerView:(UIPickerView *)pickerView widthForComponent:(NSInteger)component
{
    return 200;
}

- (IBAction)pickerDoneClicked:(id)sender
{
    CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kCHIPNumLightOnOffCluster, _numLightsTextField.text);
    [_numLightsTextField resignFirstResponder];
    [self setupStackView];
}

// MARK: Cluster Setup

- (int)numLightClustersToShow
{
    NSString * numClusters = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kCHIPNumLightOnOffCluster);
    int numberOfLights = 1;

    if (numClusters) {
        numberOfLights = [numClusters intValue];
    }
    return numberOfLights;
}

// MARK: UIButton actions

- (IBAction)onButtonTapped:(id)sender
{
    UIButton * button = (UIButton *) sender;
    NSInteger endpoint = button.tag;
    [self updateResult:[NSString stringWithFormat:@"On command sent on endpoint %@", @(endpoint)]];

    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        if (CHIPGetConnectedDeviceWithID(deviceId, ^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                if (chipDevice) {
                    CHIPOnOff * onOff = [[CHIPOnOff alloc] initWithDevice:chipDevice
                                                                 endpoint:endpoint
                                                                    queue:dispatch_get_main_queue()];
                    [onOff on:^(NSError * error, NSDictionary * values) {
                        NSString * resultString = (error != nil)
                            ? [NSString stringWithFormat:@"An error occured: 0x%02lx", error.code]
                            : @"On command success";
                        [self updateResult:resultString];
                    }];
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

- (IBAction)offButtonTapped:(id)sender
{
    UIButton * button = (UIButton *) sender;
    NSInteger endpoint = button.tag;
    [self updateResult:[NSString stringWithFormat:@"Off command sent on endpoint %@", @(endpoint)]];

    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        if (CHIPGetConnectedDeviceWithID(deviceId, ^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                if (chipDevice) {
                    CHIPOnOff * onOff = [[CHIPOnOff alloc] initWithDevice:chipDevice
                                                                 endpoint:endpoint
                                                                    queue:dispatch_get_main_queue()];
                    [onOff off:^(NSError * error, NSDictionary * values) {
                        NSString * resultString = (error != nil)
                            ? [NSString stringWithFormat:@"An error occured: 0x%02lx", error.code]
                            : @"Off command success";
                        [self updateResult:resultString];
                    }];
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

- (IBAction)toggleButtonTapped:(id)sender
{
    UIButton * button = (UIButton *) sender;
    NSInteger endpoint = button.tag;
    [self updateResult:[NSString stringWithFormat:@"Toggle command sent on endpoint %@", @(endpoint)]];

    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        if (CHIPGetConnectedDeviceWithID(deviceId, ^(CHIPDevice * _Nullable chipDevice, NSError * _Nullable error) {
                if (chipDevice) {
                    CHIPOnOff * onOff = [[CHIPOnOff alloc] initWithDevice:chipDevice
                                                                 endpoint:endpoint
                                                                    queue:dispatch_get_main_queue()];
                    [onOff toggle:^(NSError * error, NSDictionary * values) {
                        NSString * resultString = (error != nil)
                            ? [NSString stringWithFormat:@"An error occured: 0x%02lx", error.code]
                            : @"Toggle command success";
                        [self updateResult:resultString];
                    }];
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
