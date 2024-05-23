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

#import "WiFiViewController.h"
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"

@interface WiFiViewController ()
@property (strong, nonatomic) UITextField * networkSSID;
@property (strong, nonatomic) UITextField * networkPassword;
@property (strong, nonatomic) UIButton * saveButton;
@property (strong, nonatomic) UIButton * clearButton;
@end

@implementation WiFiViewController

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
    [self.networkSSID resignFirstResponder];
    [self.networkPassword resignFirstResponder];
}

// MARK: UI helpers

- (void)setupUI
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Network Configuration" toView:self.view];

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
    informationLabel.text = @"Please provide the home network your phone is connected to.";
    informationLabel.numberOfLines = 0;
    [stackView addArrangedSubview:informationLabel];
    informationLabel.translatesAutoresizingMaskIntoConstraints = false;

    // wifi entry
    UILabel * ssidLabel = [UILabel new];
    ssidLabel.text = @"SSID";
    _networkSSID = [UITextField new];
    UIView * ssidView = [CHIPUIViewUtils viewWithLabel:ssidLabel textField:_networkSSID];
    [stackView addArrangedSubview:ssidView];
    ssidView.translatesAutoresizingMaskIntoConstraints = false;
    [ssidView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // password entry
    UILabel * passwordLabel = [UILabel new];
    passwordLabel.text = @"Password";
    _networkPassword = [UITextField new];
    [_networkPassword setSecureTextEntry:YES];
    UIView * passwordView = [CHIPUIViewUtils viewWithLabel:passwordLabel textField:_networkPassword];
    [stackView addArrangedSubview:passwordView];
    [passwordView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

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
    NSString * networkSSID = MTRGetDomainValueForKey(MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey);
    if ([networkSSID length] > 0) {
        self.networkSSID.text = networkSSID;
    }

    NSString * networkPassword = MTRGetDomainValueForKey(MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey);
    if ([networkPassword length] > 0) {
        self.networkPassword.text = networkPassword;
    }
}

// MARK: Button methods

- (IBAction)saveCredientials:(id)sender
{
    if ([self.networkSSID.text length] > 0) {
        MTRSetDomainValueForKey(MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey, self.networkSSID.text);
    }

    if ([self.networkPassword.text length] > 0) {
        MTRSetDomainValueForKey(MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey, self.networkPassword.text);
    }

    [self.navigationController popViewControllerAnimated:YES];
}

- (IBAction)clearCredientials:(id)sender
{
    if ([self.networkSSID.text length] > 0) {
        MTRSetDomainValueForKey(MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey, nil);
    }

    if ([self.networkPassword.text length] > 0) {
        MTRSetDomainValueForKey(MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey, nil);
    }

    self.networkSSID.text = @"";
    self.networkPassword.text = @"";
}
@end
