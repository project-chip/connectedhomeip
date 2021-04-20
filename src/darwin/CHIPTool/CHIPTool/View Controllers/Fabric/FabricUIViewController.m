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
@property (nonatomic, strong) UIStackView * stackView;

@property (nonatomic, strong) NSArray * fabricsList;
@property (nonatomic, strong) NSNumber * fabricID;

@property (readwrite) CHIPOperationalCredentials * cluster;

@end


@implementation FabricUIViewController
// MARK: UIViewController methods

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.cluster = [[CHIPOperationalCredentials alloc] initWithDevice:CHIPGetPairedDevice() endpoint:0 queue:dispatch_get_main_queue()];
    
    [self setupUIElements];
    [self fetchFabricId];

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

    // Get Fabric ID
    UIButton *getFabricIDButton = [UIButton new];
    [getFabricIDButton setTitle:@"Go" forState:UIControlStateNormal];
    [getFabricIDButton addTarget:self action:@selector(getFabricIDButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _getFabricIDLabel = [UILabel new];
    _getFabricIDLabel.text = @"FabricID:";
    UIStackView * stackViewButtons = [CHIPUIViewUtils stackViewWithLabel:_getFabricIDLabel
                                                                 buttons:@[getFabricIDButton]];
    stackViewButtons.axis = UILayoutConstraintAxisHorizontal;
    stackViewButtons.distribution = UIStackViewDistributionEqualSpacing;
    stackViewButtons.alignment = UIStackViewAlignmentLeading;
    stackViewButtons.spacing = 10;
    [_stackView addArrangedSubview:stackViewButtons];
    
    // Update Fabric Label
    UIButton *updateFabricLabelButton = [UIButton new];
    [updateFabricLabelButton setTitle:@"Go" forState:UIControlStateNormal];
    [updateFabricLabelButton addTarget:self action:@selector(updateFabricLabelButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _updateFabricLabelTextField = [UITextField new];
    _updateFabricLabelTextField.placeholder = @"Update Fabric Label";
    UIView *updateFabricLabelView = [CHIPUIViewUtils viewWithUITextField:_updateFabricLabelTextField button:updateFabricLabelButton];
    [_stackView addArrangedSubview:updateFabricLabelView];

    updateFabricLabelView.translatesAutoresizingMaskIntoConstraints = false;
    [updateFabricLabelView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    
    // Remove Fabric
    UIButton *removeFabricButton = [UIButton new];
    [removeFabricButton setTitle:@"Go" forState:UIControlStateNormal];
    [removeFabricButton addTarget:self action:@selector(removeFabricButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    _removeFabricTextField = [UITextField new];
    _removeFabricTextField.keyboardType = UIKeyboardTypeNumberPad;
    _removeFabricTextField.placeholder = @"Fabric index to remove";
    UIView *removeFabricView = [CHIPUIViewUtils viewWithUITextField:_removeFabricTextField button:removeFabricButton];
    [_stackView addArrangedSubview:removeFabricView];

    removeFabricView.translatesAutoresizingMaskIntoConstraints = false;
    [removeFabricView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    
    // Get Fabrics List
    
    UIButton *getFabricsListButton = [UIButton new];
    getFabricsListButton.titleLabel.font = [UIFont systemFontOfSize:17];
    getFabricsListButton.titleLabel.textColor = [UIColor blackColor];
    getFabricsListButton.layer.cornerRadius = 5;
    getFabricsListButton.clipsToBounds = YES;
    getFabricsListButton.backgroundColor = UIColor.systemBlueColor;
    [getFabricsListButton setTitle:@"Get Fabrics List" forState:UIControlStateNormal];
    [getFabricsListButton addTarget:self action:@selector(getFabricsListButtonPressed:) forControlEvents:UIControlEventTouchUpInside];
    [_stackView addArrangedSubview:getFabricsListButton];
    
    getFabricsListButton.translatesAutoresizingMaskIntoConstraints = false;
    [getFabricsListButton.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    [getFabricsListButton.leadingAnchor constraintEqualToAnchor:_stackView.leadingAnchor].active = YES;
    
    // Result message
    _resultLabel = [UILabel new];
    _resultLabel.font = [UIFont systemFontOfSize:14];
    _resultLabel.textColor = UIColor.systemBlueColor;
    _resultLabel.lineBreakMode = NSLineBreakByWordWrapping;
    _resultLabel.numberOfLines = 0;
    [_stackView addArrangedSubview:_resultLabel];

    _resultLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_resultLabel.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    _resultLabel.adjustsFontSizeToFitWidth = YES;
    
    // Fabrics text view
    _fabricsListTextView = [UITextView new];
    _fabricsListTextView.font = [UIFont systemFontOfSize:14];
    _fabricsListTextView.textColor = [UIColor systemBlueColor];
    _fabricsListTextView.scrollEnabled = YES;
    _fabricsListTextView.userInteractionEnabled = NO;
    _fabricsListTextView.text = @"";
    [self.view addSubview:_fabricsListTextView];
    
    _fabricsListTextView.translatesAutoresizingMaskIntoConstraints = false;
    [_fabricsListTextView.trailingAnchor constraintEqualToAnchor:_stackView.trailingAnchor].active = YES;
    [_fabricsListTextView.leadingAnchor constraintEqualToAnchor:_stackView.leadingAnchor].active = YES;
    [_fabricsListTextView.topAnchor constraintEqualToAnchor:_stackView.bottomAnchor constant:20].active = YES;
    [_fabricsListTextView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;
    
    
}

- (void)updateResult:(NSString *)result isError:(BOOL)isError
{
    if (isError)
    {
        _resultLabel.textColor = [UIColor systemRedColor];
    } else {
        _resultLabel.textColor = [UIColor systemBlueColor];
    }
    _resultLabel.text = result;
}

- (void)updateFabricsListUIWithFabrics:(NSArray *)fabricsList error:(NSError *)error
{
    NSMutableString *fabricsText = [NSMutableString new];
    if (fabricsList) {
        int fabricIndex = 0;
        for (NSDictionary *fabricDict in fabricsList)
        {
            NSNumber *fabricId = [fabricDict objectForKey:@"FabricId"];
            NSNumber *nodeID = [fabricDict objectForKey:@"NodeId"];
            NSNumber *vendorID = [fabricDict objectForKey:@"VendorId"];
            NSData *labelData = [fabricDict objectForKey:@"Label"];
            
            NSString* label = [[NSString alloc] initWithData:labelData encoding:NSUTF8StringEncoding];
            [fabricsText appendString:[NSString stringWithFormat:@"Fabric #%@\n", @(fabricIndex)]];
            [fabricsText appendString:[NSString stringWithFormat:@"FabricId: %@\n", fabricId]];
            [fabricsText appendString:[NSString stringWithFormat:@"NodeId: %@\n", nodeID]];
            [fabricsText appendString:[NSString stringWithFormat:@"VendorId: %@\n", vendorID]];
            [fabricsText appendString:[NSString stringWithFormat:@"FabricLabel: %@\n", [label length] > 0 ? label : @"not set"]];
            [fabricsText appendString:@"------"];
            fabricIndex ++;
        }
    } else {
        NSLog(@"Got back error trying to read fabrics list %@", error);
        [fabricsText appendString:[NSString stringWithFormat:@"Error: %@ /n", [error description]]];
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        if (fabricsList)
        {
            self->_fabricsList = fabricsList;
        }
        self->_fabricsListTextView.text = fabricsText;
    });
}

- (void)fetchFabricsList
{
    NSLog(@"Request to fetchFabricsList");
    [self updateResult:[NSString stringWithFormat:@"readAttributeFabricsList command sent."] isError:NO];
    [self.cluster readAttributeFabricsList:^(NSError * _Nullable error, NSDictionary * _Nullable values) {
        NSArray *fabricsList = [values objectForKey:@"value"];
        if (error)
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self updateResult:[NSString stringWithFormat:@"readAttributeFabricsList command failed: %@.", error] isError:YES];
            });
        } else
        {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self updateResult:[NSString stringWithFormat:@"Command readAttributeFabricsList command succeeded."] isError:NO];
            });
        }
        NSLog(@"Got back fabrics list: %@ error %@", values, error);
        [self updateFabricsListUIWithFabrics:fabricsList error:error];
    }];
}

- (void)fetchFabricId
{
    NSLog(@"Request to fetchFabricId");
    [self updateResult:[NSString stringWithFormat:@"getFabricId command sent."] isError:NO];
    [self.cluster getFabricId:^(NSError * _Nullable error, NSDictionary * _Nullable values) {
        if (error) {
            NSLog(@"Got back error trying to getFabricId %@", error);
            [self updateResult:[NSString stringWithFormat:@"getFabricId command failed with error %@.", error] isError:YES];
        } else {
            NSLog(@"Got back fabricID values %@", values);
            NSNumber *fabricID = [values objectForKey:@"FabricId"];
            if (fabricID)
            {
                dispatch_async(dispatch_get_main_queue(), ^{
                    self->_fabricID = fabricID;
                    self->_getFabricIDLabel.text = [NSString stringWithFormat:@"FabricID: %@", fabricID];
                    [self updateResult:[NSString stringWithFormat:@"getFabricId command returned %@. Updating fabrics list.", fabricID] isError:NO];
                    [self fetchFabricsList];
                });
            }
        }
    }];
}

// MARK: UIButton methods

- (IBAction)getFabricIDButtonPressed:(id)sender
{
    [self fetchFabricId];
}

- (IBAction)updateFabricLabelButtonPressed:(id)sender
{
    NSString *label = _updateFabricLabelTextField.text;
    NSLog(@"Request to updateFabricLabel %@", label);
    [self updateResult:[NSString stringWithFormat:@"updateFabricLabel command sent."] isError:NO];
    [self.cluster updateFabricLabel:label completionHandler:^(NSError * _Nullable error, NSDictionary * _Nullable values) {
        dispatch_async(dispatch_get_main_queue(), ^{
            if (error) {
                NSLog(@"Got back error trying to updateFabricLabel %@", error);
                self->_updateFabricLabelTextField.text = @"";
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self updateResult:[NSString stringWithFormat:@"Command updateFabricLabel failed with error %@", error] isError:YES];
                });
            } else {
                NSLog(@"Successfully updated the label: %@", values);
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self updateResult:[NSString stringWithFormat:@"Command updateFabricLabel succeeded to update label to %@", label] isError:NO];
                });
            }
        });
    }];
}

- (IBAction)removeFabricButtonPressed:(id)sender
{
    int fabricIndex = [_removeFabricTextField.text intValue];
    NSLog(@"Request to fabric at index %@", @(fabricIndex));
    if (fabricIndex < [_fabricsList count] && [_fabricsList objectAtIndex:fabricIndex])
    {
        NSDictionary *fabricToRemove = [_fabricsList objectAtIndex:fabricIndex];
        NSLog(@"Request to remove %@", fabricToRemove);
        NSNumber *fabricId = [fabricToRemove objectForKey:@"FabricId"];
        NSNumber *nodeID = [fabricToRemove objectForKey:@"NodeId"];
        NSNumber *vendorID = [fabricToRemove objectForKey:@"VendorId"];
        [self updateResult:[NSString stringWithFormat:@"removeFabric command sent for fabricID %@.", fabricId] isError:NO];
        [self.cluster removeFabric:[fabricId unsignedLongLongValue] nodeId:[nodeID unsignedLongLongValue] vendorId:[vendorID unsignedShortValue] completionHandler:^(NSError * _Nullable error, NSDictionary * _Nullable values) {
            if (error) {
                NSLog(@"Failed to remove fabric with error %@", error);
                dispatch_async(dispatch_get_main_queue(), ^{
                    [self updateResult:[NSString stringWithFormat:@"Command removeFabric failed with error %@", error] isError:YES];
                    self->_removeFabricTextField.text = @"";
                });
            } else
            {
                NSLog(@"Succeeded removing fabric!");
                dispatch_async(dispatch_get_main_queue(), ^{
                    self->_removeFabricTextField.text = @"";
                    [self updateResult:[NSString stringWithFormat:@"Command removeFabric succeeded to remove %@", fabricId] isError:NO];
                });
            }
        }];
    } else
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            self->_removeFabricTextField.text = @"";
            [self updateResult:[NSString stringWithFormat:@"Cannot removeFabric, no fabric found at index %@", @(fabricIndex)] isError:YES];
        });
        
    }
   
}

- (IBAction)getFabricsListButtonPressed:(id)sender
{
    [self fetchFabricsList];
}



@end
