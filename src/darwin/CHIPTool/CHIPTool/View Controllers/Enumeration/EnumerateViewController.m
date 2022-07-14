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

#import "EnumerateViewController.h"

#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"

@interface EnumerateViewController ()

@property (strong, nonatomic) UIButton * sendButton;

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UIStackView * stackView;

@end

@implementation EnumerateViewController

// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUIElements];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Enumeration" toView:self.view];

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
    _sendButton = [UIButton new];
    [_sendButton setTitle:@"Start" forState:UIControlStateNormal];
    [_sendButton addTarget:self action:@selector(sendMessage:) forControlEvents:UIControlEventTouchUpInside];
    UIView * startView = [CHIPUIViewUtils stackViewWithButtons:@[ _sendButton ]];
    [_stackView addArrangedSubview:startView];
    startView.translatesAutoresizingMaskIntoConstraints = false;
    [startView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;

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
    NSLog(@"UpdatingUIResult: %@", result);
    _resultLabel.hidden = NO;
    _resultLabel.text = result;
}

// MARK: UIButton actions

- (IBAction)sendMessage:(id)sender
{
    [self updateResult:@"Enumerating..."];
    [self enumerate];
}

// MARK: Enumeration Implementation

// This API just walks over the accessory and tries to display its information.
// 1. Get the endpoint list
// 2. Get the device list on each endpoint
// 3. Get the clusters in use on each endpoint
// 4. Success?
- (void)enumerate
{
    MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
        if (error) {
            NSString * resultLog = [[NSString alloc] initWithFormat:@"Unable to get connected device: Error: %@", error];
            [self updateResult:resultLog];
            return;
        }

        MTRBaseClusterDescriptor * descriptorCluster = [[MTRBaseClusterDescriptor alloc] initWithDevice:device
                                                                                               endpoint:0
                                                                                                  queue:dispatch_get_main_queue()];
        NSLog(@"Reading parts list to get list of endpoints in use...");
        [descriptorCluster readAttributePartsListWithCompletionHandler:^(
            NSArray<NSNumber *> * _Nullable endpointsInUse, NSError * _Nullable error) {
            if (error) {
                NSString * resultLog = [[NSString alloc] initWithFormat:@"Unable to read parts list: Error: %@", error];
                [self updateResult:resultLog];
                return;
            }

            NSString * resultLog = [[NSString alloc] initWithFormat:@"Got list of endpoints in Use: %@", endpointsInUse];
            [self updateResult:resultLog];

            for (NSNumber * endpoint in endpointsInUse) {
                MTRBaseClusterDescriptor * descriptorCluster =
                    [[MTRBaseClusterDescriptor alloc] initWithDevice:device
                                                            endpoint:[endpoint unsignedShortValue]
                                                               queue:dispatch_get_main_queue()];
                [descriptorCluster readAttributeDeviceListWithCompletionHandler:^(
                    NSArray * _Nullable value, NSError * _Nullable error) {
                    if (error) {
                        NSString * resultLog = [[NSString alloc]
                            initWithFormat:@"Unable to read device list for Endpoint:%@ Error: %@", endpoint, error];
                        [self updateResult:resultLog];
                        return;
                    }

                    NSString * resultLog = [[NSString alloc] initWithFormat:@"Got device list for endpoint:%@ %@", endpoint, value];
                    [self updateResult:resultLog];

                    [descriptorCluster
                        readAttributeServerListWithCompletionHandler:^(NSArray * _Nullable value, NSError * _Nullable error) {
                            if (error) {
                                NSString * resultLog = [[NSString alloc]
                                    initWithFormat:@"Unable to read server list for Endpoint:%@ Error: %@", endpoint, error];
                                [self updateResult:resultLog];
                                return;
                            }

                            NSString * resultLog =
                                [[NSString alloc] initWithFormat:@"Got server list for endpoint:%@ %@", endpoint, value];
                            [self updateResult:resultLog];
                        }];
                }];
            }
        }];
    });
}

@end
