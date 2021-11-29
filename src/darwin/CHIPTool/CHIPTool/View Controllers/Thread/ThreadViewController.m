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

#import "ThreadViewController.h"
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"

@interface ThreadViewController ()
@property (strong, nonatomic) UITextField * networkChannel;
@property (strong, nonatomic) UITextField * networkKey;
@property (strong, nonatomic) UITextField * networkPANID;
@property (strong, nonatomic) UITextField * networkExtendedPANID;
@property (strong, nonatomic) UIButton * saveButton;
@property (strong, nonatomic) UIButton * clearButton;
@end

@implementation ThreadViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self fillNetworkConfigWithDefaults];
}

- (void)dismissKeyboard
{
    [self.networkChannel resignFirstResponder];
    [self.networkKey resignFirstResponder];
    [self.networkPANID resignFirstResponder];
    [self.networkExtendedPANID resignFirstResponder];
}

// MARK: UI helpers

- (void)setupUI
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Thread Network Configuration" toView:self.view];

    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionFill;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 30;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:30].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // info label
    UILabel * informationLabel = [UILabel new];
    informationLabel.font = [UIFont systemFontOfSize:17 weight:UIFontWeightThin];
    informationLabel.text = @"Please provide the Thread network operational dataset";
    informationLabel.numberOfLines = 0;
    [stackView addArrangedSubview:informationLabel];
    informationLabel.translatesAutoresizingMaskIntoConstraints = false;

    // Channel entry
    UILabel * channelLabel = [UILabel new];
    channelLabel.text = @"Channel";
    _networkChannel = [UITextField new];
    UIView * channelView = [CHIPUIViewUtils viewWithLabel:channelLabel textField:_networkChannel];
    [stackView addArrangedSubview:channelView];
    channelView.translatesAutoresizingMaskIntoConstraints = false;
    [channelView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // PAN-ID entry
    UILabel * panidLabel = [UILabel new];
    panidLabel.text = @"PAN ID";
    _networkPANID = [UITextField new];
    UIView * panidView = [CHIPUIViewUtils viewWithLabel:panidLabel textField:_networkPANID];
    [stackView addArrangedSubview:panidView];
    panidView.translatesAutoresizingMaskIntoConstraints = false;
    [panidView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Extended PAN-ID entry
    UILabel * exPanidLabel = [UILabel new];
    exPanidLabel.text = @"Ext. PAN ID";
    _networkExtendedPANID = [UITextField new];
    UIView * exPanidView = [CHIPUIViewUtils viewWithLabel:exPanidLabel textField:_networkExtendedPANID];
    [stackView addArrangedSubview:exPanidView];
    exPanidView.translatesAutoresizingMaskIntoConstraints = false;
    [exPanidView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Network Key entry
    UILabel * networkKeyLabel = [UILabel new];
    networkKeyLabel.text = @"Network Key";
    _networkKey = [UITextField new];
    UIView * networkKeyView = [CHIPUIViewUtils viewWithLabel:networkKeyLabel textField:_networkKey];
    [stackView addArrangedSubview:networkKeyView];
    networkKeyView.translatesAutoresizingMaskIntoConstraints = false;
    [networkKeyView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Button stack view
    _clearButton = [UIButton new];
    [_clearButton setTitle:@"Clear" forState:UIControlStateNormal];
    _saveButton = [UIButton new];
    [_saveButton setTitle:@"Save" forState:UIControlStateNormal];
    [_clearButton addTarget:self action:@selector(clearCredientials:) forControlEvents:UIControlEventTouchUpInside];
    [_saveButton addTarget:self action:@selector(saveCredientials:) forControlEvents:UIControlEventTouchUpInside];

    UIStackView * stackViewButtons = [CHIPUIViewUtils stackViewWithButtons:@[ _clearButton, _saveButton ]];
    [stackView addArrangedSubview:stackViewButtons];
    [stackViewButtons.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
}

- (void)fillNetworkConfigWithDefaults
{
    NSString * threadPANID = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkPanIDDefaultsKey);
    if ([threadPANID length] > 0) {
        self.networkPANID.text = threadPANID;
    }
    NSString * threadExtPANID = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkExtPanIDDefaultsKey);
    if ([threadExtPANID length] > 0) {
        self.networkExtendedPANID.text = threadExtPANID;
    }
    NSString * threadChannel = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkChannelDefaultsKey);
    if ([threadChannel length] > 0) {
        self.networkChannel.text = threadChannel;
    }
    NSString * threadMasterKey = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkKeyDefaultsKey);
    if ([threadMasterKey length] > 0) {
        self.networkKey.text = threadMasterKey;
    }
}

// MARK: Button methods

- (IBAction)saveCredientials:(id)sender
{
    if ([self.networkPANID.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkPanIDDefaultsKey, self.networkPANID.text);
    }
    if ([self.networkExtendedPANID.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkExtPanIDDefaultsKey, self.networkExtendedPANID.text);
    }
    if ([self.networkChannel.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkChannelDefaultsKey, self.networkChannel.text);
    }
    if ([self.networkKey.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkKeyDefaultsKey, self.networkKey.text);
    }

    [self.navigationController popViewControllerAnimated:YES];
}

- (IBAction)clearCredientials:(id)sender
{
    if ([self.networkPANID.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkPanIDDefaultsKey, nil);
    }
    if ([self.networkExtendedPANID.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkExtPanIDDefaultsKey, nil);
    }
    if ([self.networkChannel.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkChannelDefaultsKey, nil);
    }
    if ([self.networkKey.text length] > 0) {
        CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kThreadNetworkKeyDefaultsKey, nil);
    }

    self.networkChannel.text = @"";
    self.networkPANID.text = @"";
    self.networkKey.text = @"";
    self.networkExtendedPANID.text = @"";
}
@end
