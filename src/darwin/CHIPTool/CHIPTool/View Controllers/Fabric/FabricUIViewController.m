//
//  FabricUIViewController.m
//  CHIPTool
//
//  Created by Shana Azria on 15/04/2021.
//  Copyright © 2021 CHIP. All rights reserved.
//

#import "FabricUIViewController.h"

#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"

@interface FabricUIViewController ()

@property (nonatomic, strong) UILabel * getFabricIDLabel;
@property (nonatomic, strong) UITextField * updateFabricLabelTextField;
@property (nonatomic, strong) UITextField * removeFabricTextField;
@property (nonatomic, strong) UILabel * resultLabel;

@property (nonatomic, strong) UITextView * fabricsListTextView;
@property (nonatomic, strong) UILabel * commissionedFabricsLabel;
@property (nonatomic, strong) UIStackView * stackView;

@property (nonatomic, strong) NSArray<MTROperationalCredentialsClusterFabricDescriptor *> * fabricsList;
@property (nonatomic, strong) NSNumber * currentFabricIndex;
@end

@implementation FabricUIViewController
// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];

    [self setupUIElements];
    _currentFabricIndex = @(-1);
    [self fetchFabricsList];

    // listen for taps to dismiss the keyboard
    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];
}

- (void)dismissKeyboard
{
    [self.updateFabricLabelTextField resignFirstResponder];
    [self.removeFabricTextField resignFirstResponder];
}

// MARK: UI Setup

- (void)setupUIElements
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"Fabric Management" toView:self.view];

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

    // Update Fabric Label
    UIButton * updateFabricLabelButton = [UIButton new];
    [updateFabricLabelButton setTitle:@"Go" forState:UIControlStateNormal];
    [updateFabricLabelButton addTarget:self
                                action:@selector(updateFabricLabelButtonPressed:)
                      forControlEvents:UIControlEventTouchUpInside];
    _updateFabricLabelTextField = [UITextField new];
    _updateFabricLabelTextField.placeholder = @"Update Fabric Label";
    UIView * updateFabricLabelView = [CHIPUIViewUtils viewWithUITextField:_updateFabricLabelTextField
                                                                   button:updateFabricLabelButton];
    [_stackView addArrangedSubview:updateFabricLabelView];

    updateFabricLabelView.translatesAutoresizingMaskIntoConstraints = false;
    [updateFabricLabelView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;

    // Remove Fabric
    UIButton * removeFabricButton = [UIButton new];
    [removeFabricButton setTitle:@"Go" forState:UIControlStateNormal];
    [removeFabricButton addTarget:self action:@selector(removeFabricButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _removeFabricTextField = [UITextField new];
    _removeFabricTextField.keyboardType = UIKeyboardTypeNumberPad;
    _removeFabricTextField.placeholder = @"Fabric index to remove";
    UIView * removeFabricView = [CHIPUIViewUtils viewWithUITextField:_removeFabricTextField button:removeFabricButton];
    [_stackView addArrangedSubview:removeFabricView];

    removeFabricView.translatesAutoresizingMaskIntoConstraints = false;
    [removeFabricView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;

    // Remove All Fabrics

    UIButton * removeAllFabricsButton = [UIButton new];
    removeAllFabricsButton.titleLabel.font = [UIFont systemFontOfSize:17];
    removeAllFabricsButton.titleLabel.textColor = [UIColor blackColor];
    removeAllFabricsButton.layer.cornerRadius = 5;
    removeAllFabricsButton.clipsToBounds = YES;
    removeAllFabricsButton.backgroundColor = UIColor.systemBlueColor;
    [removeAllFabricsButton setTitle:@"Remove All Fabrics" forState:UIControlStateNormal];
    [removeAllFabricsButton addTarget:self
                               action:@selector(removeAllFabricsButtonPressed:)
                     forControlEvents:UIControlEventTouchUpInside];
    [_stackView addArrangedSubview:removeAllFabricsButton];

    removeAllFabricsButton.translatesAutoresizingMaskIntoConstraints = false;
    [removeAllFabricsButton.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    [removeAllFabricsButton.leadingAnchor constraintEqualToAnchor:_stackView.leadingAnchor].active = YES;

    // Get Fabrics List

    UIButton * getFabricsListButton = [UIButton new];
    getFabricsListButton.titleLabel.font = [UIFont systemFontOfSize:17];
    getFabricsListButton.titleLabel.textColor = [UIColor blackColor];
    getFabricsListButton.layer.cornerRadius = 5;
    getFabricsListButton.clipsToBounds = YES;
    getFabricsListButton.backgroundColor = UIColor.systemBlueColor;
    [getFabricsListButton setTitle:@"Update Fabrics List" forState:UIControlStateNormal];
    [getFabricsListButton addTarget:self
                             action:@selector(getFabricsListButtonPressed:)
                   forControlEvents:UIControlEventTouchUpInside];
    [_stackView addArrangedSubview:getFabricsListButton];

    getFabricsListButton.translatesAutoresizingMaskIntoConstraints = false;
    [getFabricsListButton.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    [getFabricsListButton.leadingAnchor constraintEqualToAnchor:_stackView.leadingAnchor].active = YES;

    // Result message
    _resultLabel = [UILabel new];
    _resultLabel.font = [UIFont systemFontOfSize:12];
    _resultLabel.textColor = UIColor.systemBlueColor;
    _resultLabel.lineBreakMode = NSLineBreakByWordWrapping;
    _resultLabel.numberOfLines = 0;
    [_stackView addArrangedSubview:_resultLabel];

    _resultLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_resultLabel.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    _resultLabel.adjustsFontSizeToFitWidth = YES;

    // commissionedFabricsTextView
    _commissionedFabricsLabel = [UILabel new];
    _commissionedFabricsLabel.font = [UIFont systemFontOfSize:12];
    _commissionedFabricsLabel.textColor = UIColor.systemBlueColor;
    [_stackView addArrangedSubview:_commissionedFabricsLabel];

    _commissionedFabricsLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_commissionedFabricsLabel.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    _commissionedFabricsLabel.adjustsFontSizeToFitWidth = YES;

    // Fabrics text view
    _fabricsListTextView = [UITextView new];
    _fabricsListTextView.font = [UIFont systemFontOfSize:12];
    _fabricsListTextView.textColor = [UIColor systemBlueColor];
    _fabricsListTextView.scrollEnabled = YES;
    _fabricsListTextView.userInteractionEnabled = NO;
    _fabricsListTextView.text = @"";
    [self.view addSubview:_fabricsListTextView];

    _fabricsListTextView.translatesAutoresizingMaskIntoConstraints = false;
    [_fabricsListTextView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    [_fabricsListTextView.leadingAnchor constraintEqualToAnchor:_stackView.leadingAnchor].active = YES;
    [_fabricsListTextView.topAnchor constraintEqualToAnchor:_stackView.bottomAnchor constant:20].active = YES;
    [_fabricsListTextView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active
        = YES;
}

- (void)updateResult:(NSString *)result isError:(BOOL)isError
{
    if (isError) {
        _resultLabel.textColor = [UIColor systemRedColor];
    } else {
        _resultLabel.textColor = [UIColor systemBlueColor];
    }
    _resultLabel.text = result;
}

- (void)updateFabricsListUIWithFabrics:(NSArray<MTROperationalCredentialsClusterFabricDescriptor *> *)fabricsList
                                 error:(NSError *)error
{
    NSMutableString * fabricsText = [NSMutableString new];
    if (fabricsList) {
        for (MTROperationalCredentialsClusterFabricDescriptor * fabricDescriptor in fabricsList) {
            NSNumber * fabricIndex = fabricDescriptor.fabricIndex;
            NSNumber * fabricId = fabricDescriptor.fabricId;
            NSNumber * nodeID = fabricDescriptor.nodeId;
            NSNumber * vendorID = fabricDescriptor.vendorId;
            NSString * label = fabricDescriptor.label;

            [fabricsText appendString:[NSString stringWithFormat:@"FabricIndex: %@\n", fabricIndex]];
            [fabricsText appendString:[NSString stringWithFormat:@"FabricId: %@\n", fabricId]];
            [fabricsText appendString:[NSString stringWithFormat:@"NodeId: %@\n", nodeID]];
            [fabricsText appendString:[NSString stringWithFormat:@"VendorId: %@\n", vendorID]];
            [fabricsText appendString:[NSString stringWithFormat:@"FabricLabel: %@\n", [label length] > 0 ? label : @"not set"]];
            [fabricsText appendString:@"------\n"];
        }
    } else {
        NSLog(@"Got back error trying to read fabrics list %@", error);
        [fabricsText appendString:[NSString stringWithFormat:@"Error: %@ /n", [error description]]];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        if (fabricsList) {
            self->_fabricsList = fabricsList;
        }
        self->_fabricsListTextView.text = fabricsText;
    });
}

- (void)fetchCommissionedFabricsNumber
{
    NSLog(@"Fetching the commissioned fabrics attribute");
    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice) {
                MTRBaseClusterOperationalCredentials * cluster =
                    [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:chipDevice
                                                                        endpoint:0
                                                                           queue:dispatch_get_main_queue()];
                [cluster
                    readAttributeCurrentFabricIndexWithCompletionHandler:^(NSNumber * _Nullable value, NSError * _Nullable error) {
                        if (!error) {
                            self->_currentFabricIndex = value;
                        }
                    }];

                [self
                    updateResult:[NSString stringWithFormat:@"readAttributeCommissionedFabricsWithCompletionHandler command sent."]
                         isError:NO];
                [cluster readAttributeCommissionedFabricsWithCompletionHandler:^(
                    NSNumber * _Nullable commissionedFabrics, NSError * _Nullable error) {
                    if (error) {
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [self updateResult:[NSString
                                                   stringWithFormat:@"readAttributeCommissionedFabrics command failed: %@.", error]
                                       isError:YES];
                        });
                    } else {
                        dispatch_async(dispatch_get_main_queue(), ^{
                            [self updateResult:[NSString
                                                   stringWithFormat:@"Command readAttributeCommissionedFabrics command succeeded."]
                                       isError:NO];
                            NSString * stringResult =
                                [NSString stringWithFormat:@"# commissioned fabrics: %@", commissionedFabrics];
                            self->_commissionedFabricsLabel.text = stringResult;
                        });
                    }
                }];

            } else {
                [self updateResult:[NSString stringWithFormat:@"Failed to establish a connection with the device"] isError:YES];
            }
        })) {
        [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"] isError:NO];
    } else {
        [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"] isError:YES];
    }
}

- (void)fetchFabricsList
{
    NSLog(@"Request to fetchFabricsList");
    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice) {
                MTRBaseClusterOperationalCredentials * cluster =
                    [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:chipDevice
                                                                        endpoint:0
                                                                           queue:dispatch_get_main_queue()];
                [self updateResult:[NSString stringWithFormat:@"readAttributeFabrics command sent."] isError:NO];
                MTRReadParams * params = [[MTRReadParams alloc] init];
                params.fabricFiltered = @NO;
                [cluster
                    readAttributeFabricsWithParams:params
                                 completionHandler:^(NSArray * _Nullable fabricsList, NSError * _Nullable error) {
                                     if (error) {
                                         dispatch_async(dispatch_get_main_queue(), ^{
                                             [self updateResult:[NSString
                                                                    stringWithFormat:@"readAttributeFabrics command failed: %@.",
                                                                    error]
                                                        isError:YES];
                                         });
                                     } else {
                                         dispatch_async(dispatch_get_main_queue(), ^{
                                             [self updateResult:[NSString stringWithFormat:
                                                                              @"Command readAttributeFabrics command succeeded."]
                                                        isError:NO];
                                         });
                                     }
                                     NSLog(@"Got back fabrics list: %@ error %@", fabricsList, error);
                                     [self updateFabricsListUIWithFabrics:fabricsList error:error];
                                 }];
            } else {
                [self updateResult:[NSString stringWithFormat:@"Failed to establish a connection with the device"] isError:YES];
            }
        })) {
        [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"] isError:NO];
    } else {
        [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"] isError:YES];
    }
    [self fetchCommissionedFabricsNumber];
}

// MARK: UIButton methods

- (IBAction)removeAllFabricsButtonPressed:(id)sender
{
    NSLog(@"Request to Remove All Fabrics.");
    [self.removeFabricTextField resignFirstResponder];
    UIAlertController * alert =
        [UIAlertController alertControllerWithTitle:@"Remove All Fabrics?"
                                            message:@"Are you sure you want to remove all fabrics, this will remove all fabrics on "
                                                    @"accessory, including this one, and put the device back in commissioning."
                                     preferredStyle:UIAlertControllerStyleAlert];

    UIAlertAction * defaultAction = [UIAlertAction
        actionWithTitle:@"Remove"
                  style:UIAlertActionStyleDefault
                handler:^(UIAlertAction * action) {
                    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
                            if (!chipDevice) {
                                [self
                                    updateResult:[NSString
                                                     stringWithFormat:@"Failed to establish a connection with the device %@", error]
                                         isError:YES];
                            }

                            MTRBaseClusterOperationalCredentials * opCredsCluster =
                                [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:chipDevice
                                                                                    endpoint:0
                                                                                       queue:dispatch_get_main_queue()];

                            dispatch_group_t removeGroup = dispatch_group_create();
                            // Loop over the list of all fabrics and for each, call remove
                            for (MTROperationalCredentialsClusterFabricDescriptor * fabricDescriptor in self.fabricsList) {
                                if ([fabricDescriptor.fabricIndex isEqualToNumber:self.currentFabricIndex]) {
                                    // We'll remove our own fabric later
                                    continue;
                                }

                                MTROperationalCredentialsClusterRemoveFabricParams * params =
                                    [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
                                params.fabricIndex = fabricDescriptor.fabricIndex;
                                dispatch_group_enter(removeGroup);
                                [opCredsCluster
                                    removeFabricWithParams:params
                                         completionHandler:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                             NSError * _Nullable error) {
                                             [self updateResult:[NSString stringWithFormat:@"Removed Fabric Index %@ with Error %@",
                                                                          params.fabricIndex, error]
                                                        isError:error];
                                             dispatch_group_leave(removeGroup);
                                         }];
                            }
                            dispatch_group_notify(removeGroup, dispatch_get_main_queue(), ^{
                                // now we can remove ourselves
                                MTROperationalCredentialsClusterRemoveFabricParams * params =
                                    [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
                                params.fabricIndex = self.currentFabricIndex;
                                [opCredsCluster
                                    removeFabricWithParams:params
                                         completionHandler:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable data,
                                             NSError * _Nullable error) {
                                             if (!error) {
                                                 MTRSetDevicePaired(MTRGetLastPairedDeviceId(), NO);
                                             }
                                             [self updateResult:[NSString
                                                                    stringWithFormat:@"Removed own Fabric Index %@ with Error %@",
                                                                    params.fabricIndex, error]
                                                        isError:error];
                                         }];
                            });
                        })) {
                        [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"] isError:NO];
                    } else {
                        [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"]
                                   isError:YES];
                    }
                }];

    UIAlertAction * cancelAction = [UIAlertAction actionWithTitle:@"Cancel"
                                                            style:UIAlertActionStyleCancel
                                                          handler:^(UIAlertAction * action) {
                                                          }];

    [alert addAction:cancelAction];
    [alert addAction:defaultAction];

    [self presentViewController:alert animated:YES completion:nil];
}

- (IBAction)updateFabricLabelButtonPressed:(id)sender
{
    NSString * label = _updateFabricLabelTextField.text;
    NSLog(@"Request to updateFabricLabel %@", label);
    [self.updateFabricLabelTextField resignFirstResponder];

    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice) {
                MTRBaseClusterOperationalCredentials * cluster =
                    [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:chipDevice
                                                                        endpoint:0
                                                                           queue:dispatch_get_main_queue()];
                [self updateResult:[NSString stringWithFormat:@"updateFabricLabel command sent."] isError:NO];
                __auto_type * params = [[MTROperationalCredentialsClusterUpdateFabricLabelParams alloc] init];
                params.label = label;

                [cluster
                    updateFabricLabelWithParams:params
                              completionHandler:^(MTROperationalCredentialsClusterNOCResponseParams * _Nullable response,
                                  NSError * _Nullable error) {
                                  // TODO: UpdateFabricLabel can return errors
                                  // via the NOCResponse response, but that
                                  // seems like a spec bug that should be fixed
                                  // in the spec.
                                  if (error) {
                                      NSLog(@"Error trying to updateFabricLabel %@", error);
                                      dispatch_async(dispatch_get_main_queue(), ^{
                                          self->_updateFabricLabelTextField.text = @"";
                                          [self updateResult:[NSString
                                                                 stringWithFormat:@"Command updateFabricLabel failed with error %@",
                                                                 error]
                                                     isError:YES];
                                      });
                                  } else {
                                      NSLog(@"Successfully updated the label: %@", response);
                                      dispatch_async(dispatch_get_main_queue(), ^{
                                          self->_updateFabricLabelTextField.text = @"";
                                          [self updateResult:[NSString
                                                                 stringWithFormat:
                                                                     @"Command updateFabricLabel succeeded to update label to %@",
                                                                 label]
                                                     isError:NO];
                                          [self fetchFabricsList];
                                      });
                                  }
                              }];
            } else {
                [self updateResult:[NSString stringWithFormat:@"Failed to establish a connection with the device"] isError:YES];
            }
        })) {
        [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"] isError:NO];
    } else {
        [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"] isError:YES];
    }
}

- (IBAction)removeFabricButtonPressed:(id)sender
{
    NSNumber * fabricIndex = @([_removeFabricTextField.text intValue]);
    NSLog(@"Request to fabric at index %@", fabricIndex);
    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable chipDevice, NSError * _Nullable error) {
            if (chipDevice) {
                [self updateResult:[NSString stringWithFormat:@"removeFabric command sent for fabricIndex %@.", fabricIndex]
                           isError:NO];
                MTRBaseClusterOperationalCredentials * opCredsCluster =
                    [[MTRBaseClusterOperationalCredentials alloc] initWithDevice:chipDevice
                                                                        endpoint:0
                                                                           queue:dispatch_get_main_queue()];
                MTROperationalCredentialsClusterRemoveFabricParams * params =
                    [[MTROperationalCredentialsClusterRemoveFabricParams alloc] init];
                params.fabricIndex = fabricIndex;
                [opCredsCluster
                    removeFabricWithParams:params
                         completionHandler:^(
                             MTROperationalCredentialsClusterNOCResponseParams * _Nullable data, NSError * _Nullable error) {
                             if (!error) {
                                 if (fabricIndex == self.currentFabricIndex) {
                                     MTRSetDevicePaired(MTRGetLastPairedDeviceId(), NO);
                                 }
                             }
                             [self updateResult:[NSString stringWithFormat:@"Finished removing fabric Index %@ with Error :%@",
                                                          fabricIndex, error]
                                        isError:error];
                         }];
            } else {
                [self updateResult:[NSString stringWithFormat:@"Failed to establish a connection with the device"] isError:YES];
            }
        })) {
        [self updateResult:[NSString stringWithFormat:@"Waiting for connection with the device"] isError:NO];
    } else {
        [self updateResult:[NSString stringWithFormat:@"Failed to trigger the connection with the device"] isError:YES];
    }
}

- (IBAction)getFabricsListButtonPressed:(id)sender
{
    [self fetchFabricsList];
}

@end
