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
    temperatureLabel.text = @"40°C";
    temperatureLabel.textColor = UIColor.blackColor;
    temperatureLabel.textAlignment = NSTextAlignmentCenter;
    temperatureLabel.font = [UIFont systemFontOfSize:50 weight:UIFontWeightThin];
    [stackView addArrangedSubview:temperatureLabel];
    temperatureLabel.translatesAutoresizingMaskIntoConstraints = false;
    [temperatureLabel.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Refresh button
    UIBarButtonItem *button = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemRefresh
                                                                            target:self
                                                                            action:@selector(refreshTemperatureMeasurement:)];
    self.navigationItem.rightBarButtonItem = button;
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
