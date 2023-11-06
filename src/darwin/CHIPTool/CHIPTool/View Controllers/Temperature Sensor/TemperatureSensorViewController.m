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

@interface TemperatureSensorViewController ()
@property (nonatomic, strong) UILabel * temperatureLabel;
@property (nonatomic, strong) UITextField * minIntervalInSecondsTextField;
@property (nonatomic, strong) UITextField * maxIntervalInSecondsTextField;
@property (nonatomic, strong) UITextField * deltaInCelsiusTextField;
@property (nonatomic, strong) UIButton * sendReportingSetup;
@end

static TemperatureSensorViewController * _Nullable sCurrentController = nil;

@implementation TemperatureSensorViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    sCurrentController = self;
    [super viewDidLoad];
    [self setupUI];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self readCurrentTemperature];
}

- (void)viewWillDisappear:(BOOL)animated
{
    sCurrentController = nil;
    [super viewWillDisappear:animated];
}

- (void)viewDidAppear:(BOOL)animated
{
    sCurrentController = self;
    [super viewDidAppear:animated];
}

+ (nullable TemperatureSensorViewController *)currentController
{
    return sCurrentController;
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
    [_deltaInCelsiusTextField resignFirstResponder];
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
    _temperatureLabel.text = @"°C";
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
    UILabel * minIntervalInSecondsLabel = [UILabel new];
    [minIntervalInSecondsLabel setText:@"Min. interval (sec):"];
    UIView * minIntervalInSecondsView = [CHIPUIViewUtils viewWithLabel:minIntervalInSecondsLabel
                                                             textField:_minIntervalInSecondsTextField];
    [stackView addArrangedSubview:minIntervalInSecondsView];

    minIntervalInSecondsView.translatesAutoresizingMaskIntoConstraints = false;
    [minIntervalInSecondsView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Min interval in seconds
    _maxIntervalInSecondsTextField = [UITextField new];
    _maxIntervalInSecondsTextField.keyboardType = UIKeyboardTypeNumberPad;
    UILabel * maxIntervalInSecondsLabel = [UILabel new];
    [maxIntervalInSecondsLabel setText:@"Max. interval (sec):"];
    UIView * maxIntervalInSecondsView = [CHIPUIViewUtils viewWithLabel:maxIntervalInSecondsLabel
                                                             textField:_maxIntervalInSecondsTextField];
    [stackView addArrangedSubview:maxIntervalInSecondsView];

    maxIntervalInSecondsView.translatesAutoresizingMaskIntoConstraints = false;
    [maxIntervalInSecondsView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

    // Delta
    _deltaInCelsiusTextField = [UITextField new];
    _deltaInCelsiusTextField.keyboardType = UIKeyboardTypeNumberPad;
    UILabel * deltaInCelsiusLabel = [UILabel new];
    [deltaInCelsiusLabel setText:@"Delta (°C):"];
    UIView * deltaInCelsiusView = [CHIPUIViewUtils viewWithLabel:deltaInCelsiusLabel textField:_deltaInCelsiusTextField];
    [stackView addArrangedSubview:deltaInCelsiusView];

    deltaInCelsiusView.translatesAutoresizingMaskIntoConstraints = false;
    [deltaInCelsiusView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;

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
    UIBarButtonItem * button = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                             target:self
                                                                             action:@selector(refreshTemperatureMeasurement:)];
    self.navigationItem.rightBarButtonItem = button;
}

- (void)updateTempInUI:(int)newTemp
{
    double tempInCelsius = (double) newTemp / 100;
    NSNumberFormatter * formatter = [[NSNumberFormatter alloc] init];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    formatter.minimumFractionDigits = 0;
    formatter.maximumFractionDigits = 2;
    [formatter setRoundingMode:NSNumberFormatterRoundFloor];
    _temperatureLabel.text =
        [NSString stringWithFormat:@"%@ °C", [formatter stringFromNumber:[NSNumber numberWithFloat:tempInCelsius]]];
    NSLog(@"Status: Updated temp in UI to %@", _temperatureLabel.text);
}

// MARK: MTRBaseClusterTemperatureMeasurement

- (void)readCurrentTemperature
{
    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice) {
                MTRBaseClusterTemperatureMeasurement * cluster =
                    [[MTRBaseClusterTemperatureMeasurement alloc] initWithDevice:chipDevice
                                                                        endpoint:1
                                                                           queue:dispatch_get_main_queue()];

                [cluster readAttributeMeasuredValueWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                    if (error != nil)
                        return;
                    [self updateTempInUI:value.shortValue];
                }];
            } else {
                NSLog(@"Status: Failed to establish a connection with the device");
            }
        })) {
        NSLog(@"Status: Waiting for connection with the device");
    } else {
        NSLog(@"Status: Failed to trigger the connection with the device");
    }
}

- (void)reportFromUserEnteredSettings
{
    int minIntervalSeconds = [_minIntervalInSecondsTextField.text intValue];
    int maxIntervalSeconds = [_maxIntervalInSecondsTextField.text intValue];
    int deltaInCelsius = [_deltaInCelsiusTextField.text intValue];

    NSLog(
        @"Sending temp reporting values: min %@ max %@ value %@", @(minIntervalSeconds), @(maxIntervalSeconds), @(deltaInCelsius));

    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice) {
                // Use a wildcard subscription
                [chipDevice subscribeWithQueue:dispatch_get_main_queue()
                    minInterval:minIntervalSeconds
                    maxInterval:maxIntervalSeconds
                    params:nil
                    cacheContainer:nil
                    attributeReportHandler:^(NSArray * _Nullable reports) {
                        if (!reports)
                            return;
                        for (MTRAttributeReport * report in reports) {
                            // These should be exposed by the SDK
                            if ([report.path.cluster isEqualToNumber:@(MTRClusterTemperatureMeasurementID)] &&
                                [report.path.attribute
                                    isEqualToNumber:@(MTRClusterTemperatureMeasurementAttributeMeasuredValueID)]) {
                                if (report.error != nil) {
                                    NSLog(@"Error reading temperature: %@", report.error);
                                } else {
                                    __auto_type controller = [TemperatureSensorViewController currentController];
                                    if (controller != nil) {
                                        [controller updateTempInUI:((NSNumber *) report.value).shortValue];
                                    }
                                }
                            }
                        }
                    }
                    eventReportHandler:nil
                    errorHandler:^(NSError * error) {
                        NSLog(@"Status: update reportAttributeMeasuredValue completed with error %@", [error description]);
                    }
                    subscriptionEstablished:^{
                    }
                    resubscriptionScheduled:nil];
            } else {
                NSLog(@"Status: Failed to establish a connection with the device");
            }
        })) {
        NSLog(@"Status: Waiting for connection with the device");
    } else {
        NSLog(@"Status: Failed to trigger the connection with the device");
    }
}

@end
