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
// module header
#import "QRCodeViewController.h"

// local imports
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import <CHIP/CHIP.h>

// system imports
#import <AVFoundation/AVFoundation.h>

#define INDICATOR_DELAY 0.5 * NSEC_PER_SEC
#define ERROR_DISPLAY_TIME 2.0 * NSEC_PER_SEC
#define QR_CODE_FREEZE 1.0 * NSEC_PER_SEC

// The expected Vendor ID for CHIP demos
// 0x235A: Chip's Vendor Id
#define EXAMPLE_VENDOR_ID 0x235A

#define EXAMPLE_VENDOR_TAG_IP 1
#define MAX_IP_LEN 46

#define NETWORK_CHIP_PREFIX @"CHIP-"

#define NOT_APPLICABLE_STRING @"N/A"

@interface QRCodeViewController ()

@property (nonatomic, strong) AVCaptureSession * captureSession;
@property (nonatomic, strong) AVCaptureVideoPreviewLayer * videoPreviewLayer;

@property (strong, nonatomic) UIView * qrCodeViewPreview;

@property (strong, nonatomic) UITextField * manualCodeTextField;
@property (strong, nonatomic) UIButton * doneManualCodeButton;

@property (strong, nonatomic) UIView * setupPayloadView;
@property (strong, nonatomic) UILabel * manualCodeLabel;
@property (strong, nonatomic) UIButton * resetButton;
@property (strong, nonatomic) UILabel * versionLabel;
@property (strong, nonatomic) UILabel * discriminatorLabel;
@property (strong, nonatomic) UILabel * setupPinCodeLabel;
@property (strong, nonatomic) UILabel * rendezVousInformation;
@property (strong, nonatomic) UILabel * vendorID;
@property (strong, nonatomic) UILabel * productID;
@property (strong, nonatomic) UILabel * serialNumber;

@property (strong, nonatomic) UIActivityIndicatorView * activityIndicator;
@property (strong, nonatomic) UILabel * errorLabel;

@property (readwrite) CHIPDeviceController * chipController;
@end

@implementation QRCodeViewController {
    dispatch_queue_t _captureSessionQueue;
}

// MARK: UI Setup

- (void)changeNavBarButtonToCamera
{
    UIBarButtonItem * camera = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCamera
                                                                             target:self
                                                                             action:@selector(startScanningQRCode:)];
    self.navigationItem.rightBarButtonItem = camera;
}

- (void)changeNavBarButtonToCancel
{
    UIBarButtonItem * cancel = [[UIBarButtonItem alloc] initWithBarButtonSystemItem:UIBarButtonSystemItemCancel
                                                                             target:self
                                                                             action:@selector(stopScanningQRCode:)];
    self.navigationItem.rightBarButtonItem = cancel;
}

- (void)setupUI
{
    self.view.backgroundColor = UIColor.whiteColor;

    // Setup nav bar button
    [self changeNavBarButtonToCamera];

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"QR Code Parser" toView:self.view];

    // Manual entry view
    _manualCodeTextField = [UITextField new];
    _doneManualCodeButton = [UIButton new];
    [_doneManualCodeButton addTarget:self action:@selector(enteredManualCode:) forControlEvents:UIControlEventTouchUpInside];
    _manualCodeTextField.placeholder = @"Manual Code";
    _manualCodeTextField.keyboardType = UIKeyboardTypeNumberPad;
    [_doneManualCodeButton setTitle:@"Go" forState:UIControlStateNormal];
    UIView * manualEntryView = [CHIPUIViewUtils viewWithUITextField:_manualCodeTextField button:_doneManualCodeButton];
    [self.view addSubview:manualEntryView];

    manualEntryView.translatesAutoresizingMaskIntoConstraints = false;
    [manualEntryView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [manualEntryView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [manualEntryView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:30].active = YES;

    // Results view
    _setupPayloadView = [UIView new];
    [self.view addSubview:_setupPayloadView];

    _setupPayloadView.translatesAutoresizingMaskIntoConstraints = false;
    [_setupPayloadView.topAnchor constraintEqualToAnchor:manualEntryView.bottomAnchor constant:10].active = YES;
    [_setupPayloadView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_setupPayloadView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_setupPayloadView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;

    [self addViewsToSetupPayloadView];

    // activity indicator
    _activityIndicator = [UIActivityIndicatorView new];
    [self.view addSubview:_activityIndicator];

    _activityIndicator.translatesAutoresizingMaskIntoConstraints = false;
    [_activityIndicator.centerXAnchor constraintEqualToAnchor:_setupPayloadView.centerXAnchor].active = YES;
    [_activityIndicator.centerYAnchor constraintEqualToAnchor:_setupPayloadView.centerYAnchor].active = YES;
    _activityIndicator.color = UIColor.blackColor;

    // QRCode preview
    _qrCodeViewPreview = [UIView new];
    [self.view addSubview:_qrCodeViewPreview];

    _qrCodeViewPreview.translatesAutoresizingMaskIntoConstraints = false;
    [_qrCodeViewPreview.topAnchor constraintEqualToAnchor:manualEntryView.bottomAnchor constant:30].active = YES;
    [_qrCodeViewPreview.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_qrCodeViewPreview.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_qrCodeViewPreview.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-50].active = YES;

    // Error label
    _errorLabel = [UILabel new];
    _errorLabel.text = @"Error Text";
    _errorLabel.textColor = UIColor.blackColor;
    _errorLabel.font = [UIFont systemFontOfSize:17];
    [self.view addSubview:_errorLabel];

    _errorLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_errorLabel.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_errorLabel.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_errorLabel.topAnchor constraintEqualToAnchor:manualEntryView.bottomAnchor constant:40].active = YES;

    // Reset button
    _resetButton = [UIButton new];
    [_resetButton setTitle:@"Reset" forState:UIControlStateNormal];
    [_resetButton addTarget:self action:@selector(resetView:) forControlEvents:UIControlEventTouchUpInside];
    _resetButton.backgroundColor = UIColor.systemBlueColor;
    _resetButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _resetButton.titleLabel.textColor = [UIColor whiteColor];
    _resetButton.layer.cornerRadius = 5;
    _resetButton.clipsToBounds = YES;
    [self.view addSubview:_resetButton];

    _resetButton.translatesAutoresizingMaskIntoConstraints = false;
    [_resetButton.widthAnchor constraintEqualToConstant:60].active = YES;
    [_resetButton.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;
    [_resetButton.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
}

- (void)addViewsToSetupPayloadView
{
    // manual entry field
    _manualCodeLabel = [UILabel new];
    _manualCodeLabel.text = @"00000000000000000000";
    _manualCodeLabel.textColor = UIColor.systemBlueColor;
    _manualCodeLabel.font = [UIFont systemFontOfSize:17];
    _manualCodeLabel.textAlignment = NSTextAlignmentRight;
    [_setupPayloadView addSubview:_manualCodeLabel];

    _manualCodeLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_manualCodeLabel.topAnchor constraintEqualToAnchor:_setupPayloadView.topAnchor].active = YES;
    [_manualCodeLabel.trailingAnchor constraintEqualToAnchor:_setupPayloadView.trailingAnchor].active = YES;

    // Results scroll view
    UIScrollView * resultsScrollView = [UIScrollView new];
    [_setupPayloadView addSubview:resultsScrollView];

    resultsScrollView.translatesAutoresizingMaskIntoConstraints = false;
    [resultsScrollView.topAnchor constraintEqualToAnchor:_manualCodeLabel.bottomAnchor constant:10].active = YES;
    [resultsScrollView.leadingAnchor constraintEqualToAnchor:_setupPayloadView.leadingAnchor].active = YES;
    [resultsScrollView.trailingAnchor constraintEqualToAnchor:_setupPayloadView.trailingAnchor].active = YES;
    [resultsScrollView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-20].active = YES;

    UIStackView * parserResultsView = [UIStackView new];
    parserResultsView.axis = UILayoutConstraintAxisVertical;
    parserResultsView.distribution = UIStackViewDistributionEqualSpacing;
    parserResultsView.alignment = UIStackViewAlignmentLeading;
    parserResultsView.spacing = 15;
    [resultsScrollView addSubview:parserResultsView];

    parserResultsView.translatesAutoresizingMaskIntoConstraints = false;
    [parserResultsView.topAnchor constraintEqualToAnchor:resultsScrollView.topAnchor].active = YES;
    [parserResultsView.leadingAnchor constraintEqualToAnchor:resultsScrollView.leadingAnchor].active = YES;
    [parserResultsView.trailingAnchor constraintEqualToAnchor:resultsScrollView.trailingAnchor].active = YES;
    [parserResultsView.bottomAnchor constraintEqualToAnchor:resultsScrollView.bottomAnchor].active = YES;
    [self addResultsUIToStackView:parserResultsView];
}

- (void)addResultsUIToStackView:(UIStackView *)stackView
{
    NSArray<NSString *> * resultLabelTexts =
        @[ @"version", @"discriminator", @"setup pin code", @"rendez vous information", @"vendor ID", @"product ID", @"serial #" ];
    _versionLabel = [UILabel new];
    _discriminatorLabel = [UILabel new];
    _setupPinCodeLabel = [UILabel new];
    _rendezVousInformation = [UILabel new];
    _vendorID = [UILabel new];
    _productID = [UILabel new];
    _serialNumber = [UILabel new];
    NSArray<UILabel *> * resultLabels =
        @[ _versionLabel, _discriminatorLabel, _setupPinCodeLabel, _rendezVousInformation, _vendorID, _productID, _serialNumber ];
    for (int i = 0; i < resultLabels.count && i < resultLabels.count; i++) {
        UILabel * label = [UILabel new];
        label.text = [resultLabelTexts objectAtIndex:i];
        UILabel * result = [resultLabels objectAtIndex:i];
        result.text = @"N/A";
        UIStackView * labelStackView = [CHIPUIViewUtils stackViewWithLabel:label result:result];
        labelStackView.translatesAutoresizingMaskIntoConstraints = false;
        [stackView addArrangedSubview:labelStackView];
    }
}

// MARK: UIViewController methods

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
}

- (void)dismissKeyboard
{
    [_manualCodeTextField resignFirstResponder];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];

    dispatch_queue_t callbackQueue = dispatch_queue_create("com.zigbee.chip.qrcodevc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = [CHIPDeviceController sharedController];
    [self.chipController setDelegate:self queue:callbackQueue];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self manualCodeInitialState];
    [self qrCodeInitialState];
}

// MARK: CHIPDeviceControllerDelegate
- (void)deviceControllerOnConnected
{
    NSLog(@"Status: Device connected");
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, DISPATCH_TIME_NOW), dispatch_get_main_queue(), ^{
        [self retrieveAndSendWifiCredentials];
    });
}

- (void)deviceControllerOnError:(nonnull NSError *)error
{
    NSLog(@"Status: Device Controller error %@", [error description]);
}

- (void)deviceControllerOnMessage:(nonnull NSData *)message
{
    // WRONG
    NSString * stringMessage;
    if ([CHIPDeviceController isDataModelCommand:message] == YES) {
        stringMessage = [CHIPDeviceController commandToString:message];
    } else {
        stringMessage = [[NSString alloc] initWithData:message encoding:NSUTF8StringEncoding];
    }
    CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey, stringMessage);
    NSError * error;
    [self.chipController disconnect:&error];
}

// MARK: UI Helper methods

- (void)manualCodeInitialState
{
    _setupPayloadView.hidden = YES;
    _resetButton.hidden = YES;
    _activityIndicator.hidden = YES;
    _errorLabel.hidden = YES;
}

- (void)qrCodeInitialState
{
    if ([_captureSession isRunning]) {
        [_captureSession stopRunning];
    }
    if ([_activityIndicator isAnimating]) {
        [_activityIndicator stopAnimating];
    }
    _resetButton.hidden = YES;
    [self changeNavBarButtonToCamera];
    _activityIndicator.hidden = YES;
    _captureSession = nil;
    [_videoPreviewLayer removeFromSuperlayer];
}

- (void)scanningStartState
{
    [self changeNavBarButtonToCancel];
    _setupPayloadView.hidden = YES;
    _resetButton.hidden = YES;
    _errorLabel.hidden = YES;
}

- (void)manualCodeEnteredStartState
{
    self->_activityIndicator.hidden = NO;
    [self->_activityIndicator startAnimating];
    _setupPayloadView.hidden = YES;
    _resetButton.hidden = YES;
    _errorLabel.hidden = YES;
    _manualCodeTextField.text = @"";
}

- (void)postScanningQRCodeState
{
    _captureSession = nil;
    [self changeNavBarButtonToCamera];

    [_videoPreviewLayer removeFromSuperlayer];

    self->_activityIndicator.hidden = NO;
    [self->_activityIndicator startAnimating];
}

- (void)showError:(NSError *)error
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    self->_manualCodeLabel.hidden = YES;
    _resetButton.hidden = YES;

    self->_errorLabel.text = error.localizedDescription;
    self->_errorLabel.hidden = NO;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, ERROR_DISPLAY_TIME), dispatch_get_main_queue(), ^{
        self->_errorLabel.hidden = YES;
    });
}

- (void)showPayload:(CHIPSetupPayload *)payload decimalString:(nullable NSString *)decimalString
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    self->_errorLabel.hidden = YES;
    // reset the view and remove any preferences that were stored from a previous scan
    if ([self hasScannedConnectionInfo]) {
        CHIPRemoveDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey);
    }
    self->_setupPayloadView.hidden = NO;
    self->_resetButton.hidden = NO;

    [self updateUIFields:payload decimalString:decimalString];
    [self parseOptionalData:payload];
    [self handleRendezVous:payload];
}

- (void)retrieveAndSendWifiCredentials
{
    UIAlertController * alertController =
        [UIAlertController alertControllerWithTitle:@"Wifi Configuration"
                                            message:@"Input network SSID and password that your phone is connected to."
                                     preferredStyle:UIAlertControllerStyleAlert];
    [alertController addTextFieldWithConfigurationHandler:^(UITextField * textField) {
        textField.placeholder = @"Network SSID";
        textField.clearButtonMode = UITextFieldViewModeWhileEditing;
        textField.borderStyle = UITextBorderStyleRoundedRect;

        NSString * networkSSID = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkSSIDDefaultsKey);
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

        NSString * networkPassword = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkPasswordDefaultsKey);
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
                                                     CHIPSetDomainValueForKey(
                                                         kCHIPToolDefaultsDomain, kNetworkSSIDDefaultsKey, networkSSID.text);
                                                 }

                                                 if ([networkPassword.text length] > 0) {
                                                     CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kNetworkPasswordDefaultsKey,
                                                         networkPassword.text);
                                                 }
                                                 NSLog(@"New SSID: %@ Password: %@", networkSSID.text, networkPassword.text);

                                                 [strongSelf sendWifiCredentialsWithSSID:networkSSID.text
                                                                                password:networkPassword.text];
                                             }
                                         }]];
    [self presentViewController:alertController animated:YES completion:nil];
}

- (void)sendWifiCredentialsWithSSID:(NSString *)ssid password:(NSString *)password
{
    NSString * msg = [NSString stringWithFormat:@"::%@:%@:", ssid, password];
    NSError * error;
    BOOL didSend = [self.chipController sendMessage:[msg dataUsingEncoding:NSUTF8StringEncoding] error:&error];
    if (!didSend) {
        NSLog(@"Error: %@", error.localizedDescription);
    } else {
        NSLog(@"Message Sent");
    }
}

- (void)updateUIFields:(CHIPSetupPayload *)payload decimalString:(nullable NSString *)decimalString
{
    if (decimalString) {
        _manualCodeLabel.hidden = NO;
        _manualCodeLabel.text = decimalString;
        _versionLabel.text = NOT_APPLICABLE_STRING;
        _rendezVousInformation.text = NOT_APPLICABLE_STRING;
        _serialNumber.text = NOT_APPLICABLE_STRING;
    } else {
        _manualCodeLabel.hidden = YES;
        _versionLabel.text = [NSString stringWithFormat:@"%@", payload.version];
        _rendezVousInformation.text = [NSString stringWithFormat:@"%lu", payload.rendezvousInformation];
        if ([payload.serialNumber length] > 0) {
            self->_serialNumber.text = payload.serialNumber;
        } else {
            self->_serialNumber.text = NOT_APPLICABLE_STRING;
        }
    }

    _discriminatorLabel.text = [NSString stringWithFormat:@"%@", payload.discriminator];
    _setupPinCodeLabel.text = [NSString stringWithFormat:@"%@", payload.setUpPINCode];
    // TODO: Only display vid and pid if present
    _vendorID.text = [NSString stringWithFormat:@"%@", payload.vendorID];
    _productID.text = [NSString stringWithFormat:@"%@", payload.productID];
}

- (void)parseOptionalData:(CHIPSetupPayload *)payload
{
    NSLog(@"Payload vendorID %@", payload.vendorID);
    BOOL isSameVendorID = [payload.vendorID isEqualToNumber:[NSNumber numberWithInt:EXAMPLE_VENDOR_ID]];
    if (!isSameVendorID) {
        return;
    }

    NSArray * optionalInfo = [payload getAllOptionalVendorData:nil];
    for (CHIPOptionalQRCodeInfo * info in optionalInfo) {
        NSNumber * tag = info.tag;
        if (!tag) {
            continue;
        }

        BOOL isTypeString = [info.infoType isEqualToNumber:[NSNumber numberWithInt:kOptionalQRCodeInfoTypeString]];
        if (!isTypeString) {
            return;
        }

        NSString * infoValue = info.stringValue;
        switch (tag.unsignedCharValue) {
        case EXAMPLE_VENDOR_TAG_IP:
            if ([infoValue length] > MAX_IP_LEN) {
                NSLog(@"Unexpected IP String... %@", infoValue);
            } else {
                NSLog(@"Got IP String... %@", infoValue);
                CHIPSetDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey, infoValue);
            }
            break;
        }
    }
}

// MARK: Rendez Vous

- (void)handleRendezVous:(CHIPSetupPayload *)payload
{
    switch (payload.rendezvousInformation) {
    case kRendezvousInformationNone:
    case kRendezvousInformationThread:
    case kRendezvousInformationEthernet:
    case kRendezvousInformationAllMask:
        NSLog(@"Rendezvous Unknown");
        break;
    case kRendezvousInformationWiFi:
        NSLog(@"Rendezvous Wi-Fi");
        [self handleRendezVousWiFi:[self getNetworkName:payload.discriminator]];
        break;
    case kRendezvousInformationBLE:
        NSLog(@"Rendezvous BLE");
        [self handleRendezVousBLE:payload.discriminator.unsignedShortValue setupPINCode:payload.setUpPINCode.unsignedIntValue];
        break;
    }
}

- (NSString *)getNetworkName:(NSNumber *)discriminator
{
    NSString * peripheralDiscriminator = [NSString stringWithFormat:@"%04u", discriminator.unsignedShortValue];
    NSString * peripheralFullName = [NSString stringWithFormat:@"%@%@", NETWORK_CHIP_PREFIX, peripheralDiscriminator];
    return peripheralFullName;
}

- (void)handleRendezVousBLE:(uint16_t)discriminator setupPINCode:(uint32_t)setupPINCode
{
    NSError * error;
    [self.chipController connect:discriminator setupPINCode:setupPINCode error:&error];
}

- (void)handleRendezVousWiFi:(NSString *)name
{
    NSString * message = [NSString stringWithFormat:@"SSID: %@\n\nUse WiFi Settings to connect to it.", name];
    UIAlertController * alert = [UIAlertController alertControllerWithTitle:@"SoftAP Detected"
                                                                    message:message
                                                             preferredStyle:UIAlertControllerStyleActionSheet];
    UIAlertAction * cancelAction = [UIAlertAction actionWithTitle:@"Dismiss" style:UIAlertActionStyleCancel handler:nil];
    [alert addAction:cancelAction];
    [self presentViewController:alert animated:YES completion:nil];
}

- (BOOL)hasScannedConnectionInfo
{
    NSString * ipAddress = CHIPGetDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey);
    return (ipAddress.length > 0);
}

// MARK: QR Code

- (BOOL)startScanning
{
    NSError * error;
    AVCaptureDevice * captureDevice = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];

    AVCaptureDeviceInput * input = [AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:&error];
    if (error) {
        NSLog(@"Could not setup device input: %@", [error localizedDescription]);
        return NO;
    }

    AVCaptureMetadataOutput * captureMetadataOutput = [[AVCaptureMetadataOutput alloc] init];

    _captureSession = [[AVCaptureSession alloc] init];
    [_captureSession addInput:input];
    [_captureSession addOutput:captureMetadataOutput];

    if (!_captureSessionQueue) {
        _captureSessionQueue = dispatch_queue_create("captureSessionQueue", NULL);
    }

    [captureMetadataOutput setMetadataObjectsDelegate:self queue:_captureSessionQueue];
    [captureMetadataOutput setMetadataObjectTypes:[NSArray arrayWithObject:AVMetadataObjectTypeQRCode]];

    _videoPreviewLayer = [[AVCaptureVideoPreviewLayer alloc] initWithSession:_captureSession];
    [_videoPreviewLayer setVideoGravity:AVLayerVideoGravityResizeAspectFill];
    [_videoPreviewLayer setFrame:_qrCodeViewPreview.layer.bounds];
    [_qrCodeViewPreview.layer addSublayer:_videoPreviewLayer];

    [_captureSession startRunning];

    return YES;
}

- (void)displayQRCodeInSetupPayloadView:(CHIPSetupPayload *)payload withError:(NSError *)error
{
    if (error) {
        [self showError:error];
    } else {
        [self showPayload:payload decimalString:nil];
    }
}

- (void)scannedQRCode:(NSString *)qrCode
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_captureSession stopRunning];
    });
    CHIPQRCodeSetupPayloadParser * parser = [[CHIPQRCodeSetupPayloadParser alloc] initWithBase41Representation:qrCode];
    NSError * error;
    CHIPSetupPayload * payload = [parser populatePayload:&error];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        [self postScanningQRCodeState];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, INDICATOR_DELAY), dispatch_get_main_queue(), ^{
            [self displayQRCodeInSetupPayloadView:payload withError:error];
        });
    });
}

- (void)captureOutput:(AVCaptureOutput *)captureOutput
    didOutputMetadataObjects:(NSArray *)metadataObjects
              fromConnection:(AVCaptureConnection *)connection
{
    if (metadataObjects != nil && [metadataObjects count] > 0) {
        AVMetadataMachineReadableCodeObject * metadataObj = [metadataObjects objectAtIndex:0];
        if ([[metadataObj type] isEqualToString:AVMetadataObjectTypeQRCode]) {
            [self scannedQRCode:[metadataObj stringValue]];
        }
    }
}

// MARK: Manual Code
- (void)displayManualCodeInSetupPayloadView:(CHIPSetupPayload *)payload
                              decimalString:(NSString *)decimalString
                                  withError:(NSError *)error
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    if (error) {
        [self showError:error];
    } else {
        [self showPayload:payload decimalString:decimalString];
    }
}

// MARK: IBActions

- (IBAction)startScanningQRCode:(id)sender
{
    [self scanningStartState];
    [self startScanning];
}

- (IBAction)stopScanningQRCode:(id)sender
{
    [self qrCodeInitialState];
}

- (IBAction)resetView:(id)sender
{
    // reset the view and remove any preferences that were stored from scanning the QRCode
    CHIPRemoveDomainValueForKey(kCHIPToolDefaultsDomain, kIPKey);
    [self manualCodeInitialState];
    [self qrCodeInitialState];
}

- (IBAction)enteredManualCode:(id)sender
{
    NSString * decimalString = _manualCodeTextField.text;
    [self manualCodeEnteredStartState];

    CHIPManualSetupPayloadParser * parser =
        [[CHIPManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:decimalString];
    NSError * error;
    CHIPSetupPayload * payload = [parser populatePayload:&error];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, INDICATOR_DELAY), dispatch_get_main_queue(), ^{
        [self displayManualCodeInSetupPayloadView:payload decimalString:decimalString withError:error];
    });
    [_manualCodeTextField resignFirstResponder];
}

@synthesize description;

@end
