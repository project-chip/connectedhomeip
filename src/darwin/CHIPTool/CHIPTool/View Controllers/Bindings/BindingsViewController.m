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

#import "BindingsViewController.h"
#import "CHIPUIViewUtils.h"

@interface BindingsViewController ()
@property (nonatomic, strong) UITextField * nodeIDTextField;
@property (nonatomic, strong) UITextField * groupIDTextField;
@property (nonatomic, strong) UITextField * endpointIDTextField;
@property (nonatomic, strong) UITextField * clusterIDTextField;
@end

@implementation BindingsViewController

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];
}

- (void)dismissKeyboard
{
    [_nodeIDTextField resignFirstResponder];
    [_endpointIDTextField resignFirstResponder];
    [_groupIDTextField resignFirstResponder];
    [_clusterIDTextField resignFirstResponder];
}

// MARK: UI helpers

- (void)setupUI
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Bindings" toView:self.view];

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

    // nodeID entry
    UILabel * nodeIDLabel = [UILabel new];
    nodeIDLabel.text = @"Node ID";
    _nodeIDTextField = [UITextField new];
    UIView * nodeIDView = [CHIPUIViewUtils viewWithLabel:nodeIDLabel textField:_nodeIDTextField];
    [stackView addArrangedSubview:nodeIDView];
    nodeIDView.translatesAutoresizingMaskIntoConstraints = false;
    [nodeIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // groupID entry
    UILabel * groupIDLabel = [UILabel new];
    groupIDLabel.text = @"Group ID";
    _groupIDTextField = [UITextField new];
    UIView * groupIDView = [CHIPUIViewUtils viewWithLabel:groupIDLabel textField:_groupIDTextField];
    [stackView addArrangedSubview:groupIDView];
    [groupIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // endpointID entry
    UILabel * endpointIDLabel = [UILabel new];
    endpointIDLabel.text = @"Endpoint ID";
    _endpointIDTextField = [UITextField new];
    UIView * endpointIDView = [CHIPUIViewUtils viewWithLabel:endpointIDLabel textField:_endpointIDTextField];
    [stackView addArrangedSubview:endpointIDView];
    endpointIDView.translatesAutoresizingMaskIntoConstraints = false;
    [endpointIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // clusterID entry
    UILabel * clusterIDLabel = [UILabel new];
    clusterIDLabel.text = @"Cluster ID";
    _clusterIDTextField = [UITextField new];
    UIView * clusterIDView = [CHIPUIViewUtils viewWithLabel:clusterIDLabel textField:_clusterIDTextField];
    [stackView addArrangedSubview:clusterIDView];
    [clusterIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Buttons
    UIButton * bindButton = [UIButton new];
    [bindButton setTitle:@"Bind" forState:UIControlStateNormal];
    UIButton * unbindButton = [UIButton new];
    [unbindButton setTitle:@"Unbind" forState:UIControlStateNormal];
    [bindButton addTarget:self action:@selector(bind:) forControlEvents:UIControlEventTouchUpInside];
    [unbindButton addTarget:self action:@selector(unbind:) forControlEvents:UIControlEventTouchUpInside];
    UIStackView * stackViewButtons = [CHIPUIViewUtils stackViewWithButtons:@[ bindButton, unbindButton ]];

    [stackView addArrangedSubview:stackViewButtons];
    [stackViewButtons.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
}

- (void)_clearTextFields
{
    _nodeIDTextField.text = @"";
    _endpointIDTextField.text = @"";
    _groupIDTextField.text = @"";
    _clusterIDTextField.text = @"";
}

// MARK: Button methods

- (IBAction)bind:(id)sender
{
    [self _clearTextFields];
    // TODO: Call binding API
}

- (IBAction)unbind:(id)sender
{
    [self _clearTextFields];
    // TODO: Call unbinding API
}

@end
