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

#import "NetworkRecoveryViewController.h"
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import "DeviceSelector.h"
#import <Matter/Matter.h>

@interface NetworkRecoveryViewController ()

@property (nonatomic, strong) UILabel * resultLabel;
@property (nonatomic, strong) UILabel * titleLabel;
@property (nonatomic, strong) UIStackView * stackView;
@property (nonatomic, strong) UITextField * recoveryIDTextField;

@property (nonatomic, strong) DeviceSelector * deviceSelector;
@property (strong, nonatomic) UIButton * discoverButton;
@property (strong, nonatomic) UIButton * recoverButton;

@property (nonatomic, strong) UIAlertController *loadingAlert;
@property (nonatomic) dispatch_queue_t matterQueue;
@property (readwrite) MTRDeviceController * chipController;

@end

@implementation NetworkRecoveryViewController

- (void)viewDidLoad
{
    [super viewDidLoad];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self setupUIElements];
    
    [self setupRecoveryID];

    _matterQueue = dispatch_queue_create("matterQueue", DISPATCH_QUEUE_SERIAL);
}

- (void)dismissKeyboard
{
    [_recoveryIDTextField resignFirstResponder];
    [_deviceSelector resignFirstResponder];
}

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;
    
    // Title
    _titleLabel = [CHIPUIViewUtils addTitle:@"Network Recovery" toView:self.view];
    [self setupStackView];
}

- (void)setupStackView
{
    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionEqualSpacing;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 30;
    [_stackView removeFromSuperview];
    _stackView = stackView;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:_titleLabel.bottomAnchor constant:30].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    
    // Device List and picker
    _deviceSelector = [DeviceSelector new];

    UILabel * deviceIDLabel = [UILabel new];
    deviceIDLabel.text = @"Device ID:";
    UIView * deviceIDView = [CHIPUIViewUtils viewWithLabel:deviceIDLabel textField:_deviceSelector];
    [stackView addArrangedSubview:deviceIDView];

    deviceIDView.translatesAutoresizingMaskIntoConstraints = false;
    [deviceIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = true;
    
    // recoveryID entry
    UILabel * recoveryIDLabel = [UILabel new];
    recoveryIDLabel.text = @"recovery ID";
    _recoveryIDTextField = [UITextField new];
    UIView * recoveryIDView = [CHIPUIViewUtils viewWithLabel:recoveryIDLabel textField:_recoveryIDTextField];
    [stackView addArrangedSubview:recoveryIDView];
    recoveryIDView.translatesAutoresizingMaskIntoConstraints = false;
    [recoveryIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Discover recoverable device button
    _discoverButton = [UIButton new];
    [_discoverButton setTitle:@"Discover Recoverable Devices" forState:UIControlStateNormal];
    [_discoverButton addTarget:self action:@selector(onDiscoverBtnTapped:) forControlEvents:UIControlEventTouchUpInside];
    _discoverButton.backgroundColor = UIColor.systemBlueColor;
    _discoverButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _discoverButton.titleLabel.textColor = [UIColor whiteColor];
    _discoverButton.layer.cornerRadius = 5;
    _discoverButton.clipsToBounds = YES;
    [stackView addArrangedSubview:_discoverButton];

    _discoverButton.translatesAutoresizingMaskIntoConstraints = false;
    [_discoverButton.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
    // Recover button
    _recoverButton = [UIButton new];
    [_recoverButton setTitle:@"Start Network Recovery" forState:UIControlStateNormal];
    [_recoverButton addTarget:self action:@selector(onRecoverBtnTapped:) forControlEvents:UIControlEventTouchUpInside];
    _recoverButton.backgroundColor = UIColor.lightGrayColor;
    _recoverButton.enabled = NO;
    _recoverButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _recoverButton.titleLabel.textColor = [UIColor whiteColor];
    _recoverButton.layer.cornerRadius = 5;
    _recoverButton.clipsToBounds = YES;
    [stackView addArrangedSubview:_recoverButton];

    _recoverButton.translatesAutoresizingMaskIntoConstraints = false;
    [_recoverButton.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    
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

- (void)updateResult:(NSString *)result
{
    _resultLabel.hidden = NO;
    _resultLabel.text = result;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    self.chipController = InitializeMTR();
    [self.chipController setDeviceControllerDelegate:self queue:_matterQueue];
}

- (void)setupRecoveryID
{
    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        uint64_t recoveryID = MTRGetRecoveryIdentifier(deviceId);
        if (recoveryID != 0) {
            self->_recoveryIDTextField.text = [NSString stringWithFormat:@"%llu",recoveryID];
        }
        
        [self updateResult:[NSString stringWithFormat:@"Fetching the network recovery identifier attribute"]];
        
        // Read recovery ID
        if (MTRGetConnectedDeviceWithID(deviceId, ^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
                if (chipDevice) {
                    MTRBaseClusterGeneralCommissioning * commissioningCluster = [[MTRBaseClusterGeneralCommissioning alloc] initWithDevice:chipDevice endpointID:@0 queue:self->_matterQueue];
                    [commissioningCluster readAttributeRecoveryIdentifierWithCompletion:^(NSData * _Nullable value, NSError * _Nullable error) {
                        
                        dispatch_async(dispatch_get_main_queue(), ^{
                            NSString * resultString = nil;
                            if (error == nil) {
                                unsigned long long receivedLongLongValue = 0;
                                [value getBytes:&receivedLongLongValue length:8];
                                MTRSetRecoveryIdentifier(deviceId, receivedLongLongValue);
                                self->_recoveryIDTextField.text = [NSString stringWithFormat:@"%llu",receivedLongLongValue];
                                
                                resultString = [NSString stringWithFormat:@"readAttributeRecoveryIdentifier command failed: %@.", error];
                            } else {
                                resultString = [NSString
                                                stringWithFormat:@"Command readAttributeRecoveryIdentifier command succeeded."];
                            }
                            
                            [self updateResult:resultString];
                        });
                    }];
                } else {
                    [self updateResult:[NSString stringWithFormat:@"Failed to establish a connection with the device"]];
                }
            })) {
            [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"]];
        } else {
            [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"]];
        }
    }];
}

// MARK: MTRCommissionableBrowserDelegate
- (void)controller:(MTRDeviceController *)controller didFindNetworkRecoverableDevice:(nonnull MTRNetworkRecoverableBrowserResult *)device
{
    [self updateResult:[NSString stringWithFormat:@"Found recoverable device with recoveryID: %@, recovery reason:%@", device.recoveryID, device.recoveryReason]];
    
    [_deviceSelector forSelectedDevices:^(uint64_t deviceId) {
        if ([device.recoveryID longLongValue] == deviceId) {
            self->_recoverButton.backgroundColor = UIColor.systemBlueColor;
            self->_recoverButton.enabled = YES;
        }
    }];
}

- (void)controller:(nonnull MTRDeviceController *)controller didFindCommissionableDevice:(nonnull MTRCommissionableBrowserResult *)device { 
    
}


- (void)controller:(nonnull MTRDeviceController *)controller didRemoveCommissionableDevice:(nonnull MTRCommissionableBrowserResult *)device { 
    
}


// MARK: MTRDeviceControllerDelegate
- (void)controller:(MTRDeviceController *)controller
networkRecoverComplete:(nonnull NSError *)error nodeID:(nonnull NSNumber *)nodeID
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if (self.loadingAlert) {
            [self dismissViewControllerAnimated:YES completion:^{
                self.loadingAlert = nil;
                
                NSString *title = error ? @"Error" : @"Success";
                NSString *message = error ? error.localizedDescription : @"The network has been recovered successfully!";
                UIAlertController *resultAlert = [UIAlertController alertControllerWithTitle:title
                                                                                     message:message
                                                                              preferredStyle:UIAlertControllerStyleAlert];
                [resultAlert addAction:[UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault handler:nil]];
                [self presentViewController:resultAlert animated:YES completion:nil];
            }];
        }
    });
}

// MARK: Button methods

- (IBAction)onDiscoverBtnTapped:(id)sender {
    [self updateResult:[NSString stringWithFormat:@"Discover recoverable devices..."]];
    //    [self.chipController discoverRecoverableNodes:self queue:_matterQueue timeout:3];
}

- (IBAction)onRecoverBtnTapped:(id)sender {
    UIAlertController * alertController =
    [UIAlertController alertControllerWithTitle:@"Network Recovery"
                                        message:@"Input the new network SSID and password"
                                 preferredStyle:UIAlertControllerStyleAlert];
    [alertController addTextFieldWithConfigurationHandler:^(UITextField * textField) {
        textField.placeholder = @"Network SSID";
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.borderStyle = UITextBorderStyleRoundedRect;
        
        NSString * networkSSID = MTRGetDomainValueForKey(MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey);
        if ([networkSSID length] > 0) {
            textField.text = networkSSID;
        }
    }];
    [alertController addTextFieldWithConfigurationHandler:^(UITextField * textField) {
        [textField setSecureTextEntry:YES];
        textField.placeholder = @"Password";
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.borderStyle = UITextBorderStyleRoundedRect;
        textField.secureTextEntry = YES;
        
        NSString * networkPassword = MTRGetDomainValueForKey(MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey);
        if ([networkPassword length] > 0) {
            textField.text = networkPassword;
        }
    }];
    [alertController addAction:[UIAlertAction actionWithTitle:@"Cancel"
                                                        style:UIAlertActionStyleDefault
                                                      handler:^(UIAlertAction * action) {
    }]];
    
    __weak typeof(self) weakSelf = self;
    [alertController
     addAction:[UIAlertAction actionWithTitle:@"Send"
                                        style:UIAlertActionStyleDefault
                                      handler:^(UIAlertAction * action) {
        typeof(self) strongSelf = weakSelf;
        if (strongSelf) {
            NSArray * textfields = alertController.textFields;
            UITextField * networkSSID = textfields[0];
            UITextField * networkPassword = textfields[1];
            if ([networkSSID.text length] > 0) {
                MTRSetDomainValueForKey(
                                        MTRToolDefaultsDomain, kNetworkSSIDDefaultsKey, networkSSID.text);
            }
            
            if ([networkPassword.text length] > 0) {
                MTRSetDomainValueForKey(
                                        MTRToolDefaultsDomain, kNetworkPasswordDefaultsKey, networkPassword.text);
            }
            NSLog(@"New SSID: %@ Password: %@", networkSSID.text, networkPassword.text);
            
            UIAlertController *loading = [UIAlertController alertControllerWithTitle:nil
                                                                             message:@"Network recover in progress...\n\n"
                                                                      preferredStyle:UIAlertControllerStyleAlert];
            UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleMedium];
            indicator.translatesAutoresizingMaskIntoConstraints = NO;
            [loading.view addSubview:indicator];
            [indicator.centerXAnchor constraintEqualToAnchor:loading.view.centerXAnchor].active = YES;
            [indicator.bottomAnchor constraintEqualToAnchor:loading.view.bottomAnchor constant:-20].active = YES;
            [indicator startAnimating];
            
            strongSelf.loadingAlert = loading;
            [strongSelf presentViewController:loading animated:YES completion:nil];
            
            NSError * error;
            
            //                                                 if(![strongSelf.chipController recoverDevice:device.nodeId recoveryIdentifier:device.recoveryId wifiSSID:networkSSID.text wifiCredentials:networkPassword.text error:&error]){
            //                                                     NSLog(@"Failed to recover device %llu, with error %@", device.nodeId, error);
            //                                                 }
            
        }
    }]];
    [self presentViewController:alertController animated:YES completion:nil];
}

@end
