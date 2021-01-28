//
//  TemperatureSensorViewController.m
//  CHIPTool
//
//  Created by Shana Azria on 07/10/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "TemperatureSensorViewController.h"
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import <CHIP/CHIP.h>

@interface TemperatureSensorViewController ()
@property (nonatomic, strong) UILabel *temperatureLabel;
@property (nonatomic, strong) UITextField *minIntervalInSecondsTextField;
@property (nonatomic, strong) UITextField *maxIntervalInSecondsTextField;
@property (nonatomic, strong) UITextField *deltaInFahrenheitTextField;
@property (nonatomic, strong) UIButton *sendReportingSetup;

@property (nonatomic, strong) CHIPTemperatureMeasurement *chipTempMeasurement;
@property (readwrite) CHIPDevice * chipDevice;
@property (readwrite) CHIPDeviceController * chipController;

@end

@implementation TemperatureSensorViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];
    
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    // initialize the device controller
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.tempsensorvc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];
    
    uint64_t deviceID = CHIPGetNextAvailableDeviceID();
    if (deviceID > 1) {
        // Let's use the last device that was paired
        deviceID--;
        NSError * error;
        self.chipDevice = [self.chipController getPairedDevice:deviceID error:&error];
        self.chipTempMeasurement = [[CHIPTemperatureMeasurement alloc] initWithDevice:self.chipDevice endpoint:1 queue:callbackQueue];
    }
    
    [self readCurrentTemperature];
}

- (IBAction)sendReportingSetup:(id)sender
{
    NSLog(@"Status: User request to send reporting setup.");
    [self reportFromUserEnteredSettings];
}

- (IBAction)refreshTemperatureMeasurement:(id)sender
{
    NSLog(@"Status: User request to refresh temperature reading.");
    [self readCurrentTemperature];
}

// MARK: UI helpers

- (void)dismissKeyboard
{
    [_minIntervalInSecondsTextField resignFirstResponder];
    [_maxIntervalInSecondsTextField resignFirstResponder];
    [_deltaInFahrenheitTextField resignFirstResponder];
}

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
    _temperatureLabel = [UILabel new];
    _temperatureLabel.text = @"°F";
    _temperatureLabel.textColor = UIColor.blackColor;
    _temperatureLabel.textAlignment = NSTextAlignmentCenter;
    _temperatureLabel.font = [UIFont systemFontOfSize:50 weight:UIFontWeightThin];
    [stackView addArrangedSubview:_temperatureLabel];
    _temperatureLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_temperatureLabel.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
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

- (void)updateTempInUI:(int)newTemp
{
    _temperatureLabel.text = [NSString stringWithFormat:@"%@ °F", @(newTemp)];
    NSLog(@"Status: Updated temp in UI to %@", _temperatureLabel.text);
}


// MARK: CHIPTemperatureMeasurement

- (void)readCurrentTemperature
{
    CHIPDeviceCallback completionHandler = ^(NSError * error) {
        NSLog(@"Status: Read temperature request completed with error %@", [error description]);
    };
    
    [self.chipTempMeasurement readAttributeMeasuredValue:completionHandler];
}

- (void)reportFromUserEnteredSettings
{
    CHIPDeviceCallback onCompletionCallback = ^(NSError * error) {
        NSLog(@"Status: update reportAttributeMeasuredValue completed with error %@", [error description]);
    };
    
    CHIPDeviceCallback onChangeCallback = ^(NSError * error) {
        NSLog(@"Status: Temp value changed with error %@", [error description]);
    };
    int minIntervalSeconds = [_minIntervalInSecondsTextField.text intValue]*1000;
    int maxIntervalSeconds = [_maxIntervalInSecondsTextField.text intValue]*1000;
    int deltaInFahrenheit = [_deltaInFahrenheitTextField.text intValue];
    
    NSLog(@"Sending temp reporting values: min %@ max %@ value %@", @(minIntervalSeconds), @(maxIntervalSeconds), @(deltaInFahrenheit));
    
    [self.chipTempMeasurement reportAttributeMeasuredValue:onCompletionCallback
                                                  onChange:onChangeCallback
                                               minInterval:minIntervalSeconds
                                               maxInterval:maxIntervalSeconds
                                                    change:deltaInFahrenheit];
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
