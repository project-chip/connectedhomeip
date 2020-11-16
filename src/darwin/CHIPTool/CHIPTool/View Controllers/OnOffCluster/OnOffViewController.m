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

@property (nonatomic, strong) UILabel * resultLabel;

@property (readwrite) CHIPDeviceController * chipController;

@property (readonly) CHIPToolPersistentStorageDelegate * persistentStorage;

@end

@implementation OnOffViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUIElements];

    _persistentStorage = [[CHIPToolPersistentStorageDelegate alloc] init];

    // initialize the device controller
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.onoffvc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];
    [self.chipController setPersistentStorageDelegate:_persistentStorage queue:callbackQueue];
    self.onOff = [[CHIPOnOff alloc] initWithDeviceController:self.chipController];
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

    // Create buttons
    [self addButtons:4 toStackView:stackView];

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
        labelLight.text = [NSString stringWithFormat:@"Light %@: ", @(i)];
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
                                                                     button1:onButton
                                                                     button2:offButton
                                                                     button3:toggleButton];
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

// MARK: UIButton actions

- (IBAction)onButtonTapped:(id)sender
{
    UIButton * button = (UIButton *) sender;
    NSInteger lightNumber = button.tag;
    NSLog(@"Light %@ on button pressed.", @(lightNumber));
    // TODO: Do something based on which light is selected
    [self.onOff lightOn];
}

- (IBAction)offButtonTapped:(id)sender
{
    UIButton * button = (UIButton *) sender;
    NSInteger lightNumber = button.tag;
    NSLog(@"Light %@ off button pressed.", @(lightNumber));
    // TODO: Do something based on which light is selected
    [self.onOff lightOff];
}

- (IBAction)toggleButtonTapped:(id)sender
{
    UIButton * button = (UIButton *) sender;
    NSInteger lightNumber = button.tag;
    NSLog(@"Light %@ toggle button pressed.", @(lightNumber));
    // TODO: Do something based on which light is selected
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

@end
