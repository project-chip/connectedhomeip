//
//  TemperatureSensorViewController.m
//  CHIPTool
//
//  Created by Shana Azria on 07/10/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "TemperatureSensorViewController.h"
#import "CHIPUIViewUtils.h"
#import <CHIP/CHIP.h>

@interface TemperatureSensorViewController ()
@property (readwrite) CHIPDeviceController * chipController;
@property (nonatomic, strong) UITextField *minIntervalInSecondsTextField;
@property (nonatomic, strong) UITextField *maxIntervalInSecondsTextField;
@property (nonatomic, strong) UITextField *deltaInFahrenheitTextField;
@property (nonatomic, strong) UIButton *sendReportingSetup;

@end

@implementation TemperatureSensorViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];

    // initialize the device controller
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.tempsensorvc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];
    
    //TODO: Get current temperature
}

// MARK: UI helpers

- (void)setupUI
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Temperature Sensor" toView:self.view];
    
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
    
    // Temperature label
    UILabel *temperatureLabel = [UILabel new];
    temperatureLabel.text = @"150°F";
    temperatureLabel.textColor = UIColor.blackColor;
    temperatureLabel.textAlignment = NSTextAlignmentCenter;
    temperatureLabel.font = [UIFont systemFontOfSize:50 weight:UIFontWeightThin];
    [stackView addArrangedSubview:temperatureLabel];
    temperatureLabel.translatesAutoresizingMaskIntoConstraints = false;
    [temperatureLabel.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Reporting settings
    UILabel * reportingLabel = [UILabel new];
    reportingLabel.text = @"Reporting Setup";
    reportingLabel.textColor = UIColor.blackColor;
    reportingLabel.textAlignment = NSTextAlignmentLeft;
    reportingLabel.font = [UIFont systemFontOfSize:18 weight:UIFontWeightSemibold];
    [stackView addArrangedSubview:reportingLabel];

    reportingLabel.translatesAutoresizingMaskIntoConstraints = false;
    [reportingLabel.centerXAnchor constraintEqualToAnchor:stackView.centerXAnchor].active = YES;
    
    // Min interval in seconds
    _minIntervalInSecondsTextField = [UITextField new];
    _minIntervalInSecondsTextField.keyboardType = UIKeyboardTypeNumberPad;
    UILabel *minIntervalInSecondsLabel = [UILabel new];
    [minIntervalInSecondsLabel setText:@"Min. interval (sec):"];
    UIView *minIntervalInSecondsView = [CHIPUIViewUtils viewWithLabel:minIntervalInSecondsLabel textField:_minIntervalInSecondsTextField];
    [stackView addArrangedSubview:minIntervalInSecondsView];
    
    minIntervalInSecondsView.translatesAutoresizingMaskIntoConstraints = false;
    [minIntervalInSecondsView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Min interval in seconds
    _maxIntervalInSecondsTextField = [UITextField new];
    _maxIntervalInSecondsTextField.keyboardType = UIKeyboardTypeNumberPad;
    UILabel *maxIntervalInSecondsLabel = [UILabel new];
    [maxIntervalInSecondsLabel setText:@"Max. interval (sec):"];
    UIView *maxIntervalInSecondsView = [CHIPUIViewUtils viewWithLabel:maxIntervalInSecondsLabel textField:_maxIntervalInSecondsTextField];
    [stackView addArrangedSubview:maxIntervalInSecondsView];
    
    maxIntervalInSecondsView.translatesAutoresizingMaskIntoConstraints = false;
    [maxIntervalInSecondsView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    //Delta
    _deltaInFahrenheitTextField = [UITextField new];
    _deltaInFahrenheitTextField.keyboardType = UIKeyboardTypeNumberPad;
    UILabel *deltaInFahrenheitLabel = [UILabel new];
    [deltaInFahrenheitLabel setText:@"Delta (F):"];
    UIView *deltaInFahrenheitView = [CHIPUIViewUtils viewWithLabel:deltaInFahrenheitLabel textField:_deltaInFahrenheitTextField];
    [stackView addArrangedSubview:deltaInFahrenheitView];
    
    deltaInFahrenheitView.translatesAutoresizingMaskIntoConstraints = false;
    [deltaInFahrenheitView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Reporting button
    _sendReportingSetup = [UIButton new];
    [_sendReportingSetup setTitle:@"Send reporting settings" forState:UIControlStateNormal];
    [_sendReportingSetup addTarget:self action:@selector(sendReportingSetup:) forControlEvents:UIControlEventTouchUpInside];
    _sendReportingSetup.backgroundColor = UIColor.systemBlueColor;
    _sendReportingSetup.titleLabel.font = [UIFont systemFontOfSize:17];
    _sendReportingSetup.titleLabel.textColor = [UIColor whiteColor];
    _sendReportingSetup.layer.cornerRadius = 5;
    _sendReportingSetup.clipsToBounds = YES;
    [stackView addArrangedSubview:_sendReportingSetup];
    
    _sendReportingSetup.translatesAutoresizingMaskIntoConstraints = false;
    [_sendReportingSetup.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Refresh button
    UIBarButtonItem *button = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                            target:self
                                                                            action:@selector(refreshTemperatureMeasurement:)];
    self.navigationItem.rightBarButtonItem = button;
}

- (IBAction)sendReportingSetup:(id)sender
{
    // TODO: Call send reporting API
    NSLog(@"Status: User request to send reporting setup.");
}

- (IBAction)refreshTemperatureMeasurement:(id)sender
{
    // TODO: Call read API with callback
    NSLog(@"Status: User request to refresh temperature reading.");
}


// MARK: CHIPDeviceControllerDelegate
- (void)deviceControllerOnConnected
{
    NSLog(@"Status: Device connected");
}

- (void)deviceControllerOnError:(nonnull NSError *)error
{
    NSLog(@"Status: Device Controller error %@", [error description]);
}

- (void)deviceControllerOnMessage:(nonnull NSData *)message
{
    NSLog(@"Status: Received a message.");
    // TODO: Use callback APIs to show read response
}

@end
