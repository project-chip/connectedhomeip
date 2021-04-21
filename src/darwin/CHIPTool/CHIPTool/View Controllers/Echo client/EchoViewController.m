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

@interface EchoViewController ()

@property (strong, nonatomic) UITextField * messageTextField;
@property (strong, nonatomic) UIButton * sendButton;

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UIStackView * stackView;

@property (readwrite) CHIPBasic * cluster;

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

    self.cluster = [[CHIPBasic alloc] initWithDevice:CHIPGetPairedDevice() endpoint:0 queue:dispatch_get_main_queue()];
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
    if (!self.cluster) {
        [self updateResult:@"Something went wrong. Cluster is not initialized."];
    }

    NSString * msg = [self.messageTextField text];
    if (msg.length == 0) {
        msg = [self.messageTextField placeholder];
    }

    [self updateResult:@"MfgSpecificPing command sent..."];

    [self.cluster mfgSpecificPing:^(NSError * error, NSDictionary * values) {
        NSString * resultString = (error == nil) ? @"MfgSpecificPing command: success!"
                                                 : [NSString stringWithFormat:@"An error occured: 0x%02lx", error.code];
        [self updateResult:resultString];
    }];
}

@end
