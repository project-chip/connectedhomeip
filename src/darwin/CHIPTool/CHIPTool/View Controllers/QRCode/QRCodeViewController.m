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
#import "DeviceSelector.h"
#import <Matter/MTRDeviceAttestationDelegate.h>
#import <Matter/MTRSetupPayload.h>
#import <Matter/Matter.h>

// system imports
#import <AVFoundation/AVFoundation.h>

#define INDICATOR_DELAY 0.5 * NSEC_PER_SEC
#define ERROR_DISPLAY_TIME 2.0 * NSEC_PER_SEC
#define QR_CODE_FREEZE 1.0 * NSEC_PER_SEC

// The expected Vendor ID for CHIP demos
// 0xFFF1: Chip's Vendor Id
#define EXAMPLE_VENDOR_ID 0xFFF1

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

@property (strong, nonatomic) UIButton * nfcScanButton;
@property (readwrite) BOOL sessionIsActive;

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

@property (strong, nonatomic) UIButton * readFromLedgerButton;
@property (strong, nonatomic) UIButton * redirectButton;
@property (strong, nonatomic) UILabel * commissioningFlowLabel;
@property (strong, nonatomic) UILabel * commissioningCustomFlowUrl;
@property (strong, nonatomic) UIView * deviceModelInfoView;
@property (strong, nonatomic) NSDictionary * ledgerRespond;

@property (strong, nonatomic) UIActivityIndicatorView * activityIndicator;
@property (strong, nonatomic) UILabel * errorLabel;

@property (readwrite) MTRDeviceController * chipController;
@property (nonatomic, strong) MTRBaseClusterNetworkCommissioning * cluster;

@property (strong, nonatomic) NFCNDEFReaderSession * session;
@property (strong, nonatomic) MTRSetupPayload * setupPayload;
@property (strong, nonatomic) DeviceSelector * deviceList;
@end

@interface CHIPToolDeviceAttestationDelegate : NSObject <MTRDeviceAttestationDelegate>

@property (weak, nonatomic) QRCodeViewController * viewController;

- (instancetype)initWithViewController:(QRCodeViewController *)viewController;

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

    // Initialize all Labels
    [self initializeAllLabels];

    // Title
    UILabel * titleLabel = [CHIPUIViewUtils addTitle:@"QR Code Parser" toView:self.view];

    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionFill;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 10;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:30].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // Manual entry view
    _manualCodeTextField = [UITextField new];
    _doneManualCodeButton = [UIButton new];
    [_doneManualCodeButton addTarget:self action:@selector(enteredManualCode:) forControlEvents:UIControlEventTouchUpInside];
    _manualCodeTextField.placeholder = @"Manual Code";
    _manualCodeTextField.keyboardType = UIKeyboardTypeNumberPad;
    [_doneManualCodeButton setTitle:@"Go" forState:UIControlStateNormal];
    UIView * manualEntryView = [CHIPUIViewUtils viewWithUITextField:_manualCodeTextField button:_doneManualCodeButton];
    [stackView addArrangedSubview:manualEntryView];

    manualEntryView.translatesAutoresizingMaskIntoConstraints = false;
    [manualEntryView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [manualEntryView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [manualEntryView.topAnchor constraintEqualToAnchor:titleLabel.bottomAnchor constant:30].active = YES;

    _nfcScanButton = [UIButton new];
    [_nfcScanButton setTitle:@"Scan NFC Tag" forState:UIControlStateNormal];
    [_nfcScanButton addTarget:self action:@selector(startScanningNFCTags:) forControlEvents:UIControlEventTouchDown];
    _nfcScanButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _nfcScanButton.titleLabel.textColor = [UIColor blackColor];
    _nfcScanButton.layer.cornerRadius = 5;
    _nfcScanButton.clipsToBounds = YES;
    _nfcScanButton.backgroundColor = UIColor.systemBlueColor;
    [stackView addArrangedSubview:_nfcScanButton];

    _nfcScanButton.translatesAutoresizingMaskIntoConstraints = false;
    [_nfcScanButton.leadingAnchor constraintEqualToAnchor:stackView.leadingAnchor].active = YES;
    [_nfcScanButton.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = YES;
    [_nfcScanButton.heightAnchor constraintEqualToConstant:40].active = YES;

    _deviceList = [DeviceSelector new];
    [_deviceList setEnabled:NO];

    UILabel * deviceIDLabel = [UILabel new];
    deviceIDLabel.text = @"Paired Devices:";
    UIView * deviceIDView = [CHIPUIViewUtils viewWithLabel:deviceIDLabel textField:_deviceList];
    [stackView addArrangedSubview:deviceIDView];

    deviceIDView.translatesAutoresizingMaskIntoConstraints = false;
    [deviceIDView.trailingAnchor constraintEqualToAnchor:stackView.trailingAnchor].active = true;

    // Results view
    _setupPayloadView = [UIView new];
    [self.view addSubview:_setupPayloadView];

    _setupPayloadView.translatesAutoresizingMaskIntoConstraints = false;
    [_setupPayloadView.topAnchor constraintEqualToAnchor:stackView.bottomAnchor constant:10].active = YES;
    [_setupPayloadView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_setupPayloadView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_setupPayloadView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-60].active = YES;

    _deviceModelInfoView = [UIView new];
    [self.view addSubview:_deviceModelInfoView];

    _deviceModelInfoView.translatesAutoresizingMaskIntoConstraints = false;
    [_deviceModelInfoView.topAnchor constraintEqualToAnchor:stackView.bottomAnchor constant:10].active = YES;
    [_deviceModelInfoView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_deviceModelInfoView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_deviceModelInfoView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-60].active
        = YES;

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
    [_qrCodeViewPreview.topAnchor constraintEqualToAnchor:_nfcScanButton.bottomAnchor constant:30].active = YES;
    [_qrCodeViewPreview.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_qrCodeViewPreview.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_qrCodeViewPreview.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-50].active = YES;

    // Error label
    _errorLabel = [UILabel new];
    _errorLabel.text = @"Error Text";
    _errorLabel.textColor = UIColor.blackColor;
    _errorLabel.font = [UIFont systemFontOfSize:17];
    [stackView addArrangedSubview:_errorLabel];

    _errorLabel.translatesAutoresizingMaskIntoConstraints = false;
    [_errorLabel.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_errorLabel.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

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

    // Read from Ledger button
    _readFromLedgerButton = [UIButton new];
    [_readFromLedgerButton setTitle:@"Read from Ledger" forState:UIControlStateNormal];
    [_readFromLedgerButton addTarget:self action:@selector(readFromLedgerApi:) forControlEvents:UIControlEventTouchUpInside];
    _readFromLedgerButton.backgroundColor = UIColor.systemBlueColor;
    _readFromLedgerButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _readFromLedgerButton.titleLabel.textColor = [UIColor whiteColor];
    _readFromLedgerButton.layer.cornerRadius = 5;
    _readFromLedgerButton.clipsToBounds = YES;
    _readFromLedgerButton.hidden = YES;
    [self.view addSubview:_readFromLedgerButton];

    _readFromLedgerButton.translatesAutoresizingMaskIntoConstraints = false;
    [_readFromLedgerButton.widthAnchor constraintEqualToConstant:200].active = YES;
    [_readFromLedgerButton.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active
        = YES;
    [_readFromLedgerButton.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // Redirect Custom Flow button
    _redirectButton = [UIButton new];
    [_redirectButton setTitle:@"Redirect" forState:UIControlStateNormal];
    [_redirectButton addTarget:self action:@selector(redirectToUrl:) forControlEvents:UIControlEventTouchUpInside];
    _redirectButton.backgroundColor = UIColor.systemBlueColor;
    _redirectButton.titleLabel.font = [UIFont systemFontOfSize:17];
    _redirectButton.titleLabel.textColor = [UIColor whiteColor];
    _redirectButton.layer.cornerRadius = 5;
    _redirectButton.clipsToBounds = YES;
    _redirectButton.hidden = YES;
    [self.view addSubview:_redirectButton];

    _redirectButton.translatesAutoresizingMaskIntoConstraints = false;
    [_redirectButton.widthAnchor constraintEqualToConstant:200].active = YES;
    [_redirectButton.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;
    [_redirectButton.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
}

- (void)initializeAllLabels
{
    _versionLabel = [UILabel new];
    _discriminatorLabel = [UILabel new];
    _setupPinCodeLabel = [UILabel new];
    _rendezVousInformation = [UILabel new];
    _vendorID = [UILabel new];
    _productID = [UILabel new];
    _serialNumber = [UILabel new];
    _commissioningFlowLabel = [UILabel new];
    _commissioningCustomFlowUrl = [UILabel new];
}

- (void)addDetailSubview:(UIView *)superView
{
    // Results scroll view
    UIScrollView * resultsScrollView = [UIScrollView new];
    [superView addSubview:resultsScrollView];

    resultsScrollView.translatesAutoresizingMaskIntoConstraints = false;
    [resultsScrollView.topAnchor constraintEqualToAnchor:superView.topAnchor constant:10].active = YES;
    [resultsScrollView.leadingAnchor constraintEqualToAnchor:superView.leadingAnchor].active = YES;
    [resultsScrollView.trailingAnchor constraintEqualToAnchor:superView.trailingAnchor].active = YES;
    [resultsScrollView.bottomAnchor constraintEqualToAnchor:superView.bottomAnchor constant:-20].active = YES;

    UIStackView * parserResultsView = [UIStackView new];
    parserResultsView.axis = UILayoutConstraintAxisVertical;
    parserResultsView.distribution = UIStackViewDistributionEqualSpacing;
    parserResultsView.alignment = UIStackViewAlignmentLeading;
    parserResultsView.spacing = 5;
    [resultsScrollView addSubview:parserResultsView];

    parserResultsView.translatesAutoresizingMaskIntoConstraints = false;
    [parserResultsView.topAnchor constraintEqualToAnchor:resultsScrollView.topAnchor].active = YES;
    [parserResultsView.leadingAnchor constraintEqualToAnchor:resultsScrollView.leadingAnchor].active = YES;
    [parserResultsView.trailingAnchor constraintEqualToAnchor:resultsScrollView.trailingAnchor].active = YES;
    [parserResultsView.bottomAnchor constraintEqualToAnchor:resultsScrollView.bottomAnchor].active = YES;

    if (superView == _setupPayloadView) {
        [superView addSubview:_manualCodeLabel];
        [self addResultsUIToStackView:parserResultsView];
    } else if (superView == _deviceModelInfoView) {
        [self addDeviceInfoUIToStackView:parserResultsView];
    }
}

- (void)addResultsUIToStackView:(UIStackView *)stackView
{
    NSArray<NSString *> * resultLabelTexts = @[
        @"Version", @"Vendor ID", @"Product ID", @"Discriminator", @"Setup PIN Code", @"Rendez Vous Information", @"Serial #",
        @"Commissioning Flow"
    ];
    NSArray<UILabel *> * resultLabels = @[
        _versionLabel, _vendorID, _productID, _discriminatorLabel, _setupPinCodeLabel, _rendezVousInformation, _serialNumber,
        _commissioningFlowLabel
    ];
    [self addItemToStackView:stackView resultLabels:resultLabels resultLabelTexts:resultLabelTexts];
}

- (void)addDeviceInfoUIToStackView:(UIStackView *)stackView
{
    NSArray<NSString *> * resultLabelTexts = @[ @"Vendor ID", @"Product ID", @"Commissioning URL" ];
    NSArray<UILabel *> * resultLabels = @[ _vendorID, _productID, _commissioningCustomFlowUrl ];
    [self addItemToStackView:stackView resultLabels:resultLabels resultLabelTexts:resultLabelTexts];
}

- (void)addItemToStackView:(UIStackView *)stackView
              resultLabels:(NSArray<UILabel *> *)resultLabels
          resultLabelTexts:(NSArray<NSString *> *)resultLabelTexts
{
    for (int i = 0; i < resultLabels.count && i < resultLabelTexts.count; i++) {
        UILabel * label = [UILabel new];
        label.text = [resultLabelTexts objectAtIndex:i];
        UILabel * result = [resultLabels objectAtIndex:i];
        if (!result.text)
            result.text = @"N/A";
        UIStackView * labelStackView = [CHIPUIViewUtils stackViewWithLabel:label result:result];
        labelStackView.translatesAutoresizingMaskIntoConstraints = false;
        [stackView addArrangedSubview:labelStackView];
    }
}

- (void)updateResultViewUI:(UIView *)superView
{
    NSArray * viewsToRemove = [superView subviews];
    for (UIView * v in viewsToRemove) {
        [v removeFromSuperview];
    }
    [self addDetailSubview:superView];
}

// MARK: UIViewController methods

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
    [_session invalidateSession];
    _session = nil;
}

- (void)dismissKeyboard
{
    [_manualCodeTextField resignFirstResponder];
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];

    dispatch_queue_t callbackQueue = dispatch_queue_create("com.csa.matter.qrcodevc.callback", DISPATCH_QUEUE_SERIAL);
    self.chipController = InitializeMTR();
    [self.chipController setDeviceControllerDelegate:self queue:callbackQueue];

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self manualCodeInitialState];
    [self qrCodeInitialState];
}

// MARK: NFCNDEFReaderSessionDelegate

- (void)readerSession:(nonnull NFCNDEFReaderSession *)session didDetectNDEFs:(nonnull NSArray<NFCNDEFMessage *> *)messages
{
    [_session invalidateSession];
    NSString * errorMessage;
    if (messages.count == 1) {
        for (NFCNDEFMessage * message in messages) {
            if (message.records.count == 1) {
                for (NFCNDEFPayload * payload in message.records) {
                    NSString * payloadType = [[NSString alloc] initWithData:payload.type encoding:NSUTF8StringEncoding];
                    if ([payloadType isEqualToString:@"U"]) {
                        NSURL * payloadURI = [payload wellKnownTypeURIPayload];
                        NSLog(@"Payload text:%@", payloadURI);
                        if (payloadURI) {
                            NSString * qrCode = [payloadURI absoluteString];
                            NSLog(@"Scanned code string:%@", qrCode);
                            [self scannedQRCode:qrCode];
                        }
                    } else {
                        errorMessage = @"Record must be of type text.";
                    }
                }
            } else {
                errorMessage = @"Only one record in NFC tag is accepted.";
            }
        }
    } else {
        errorMessage = @"Only one message in NFC tag is accepted.";
    }
    if ([errorMessage length] > 0) {
        NSError * error = [[NSError alloc] initWithDomain:@"com.chiptool.nfctagscanning"
                                                     code:1
                                                 userInfo:@{ NSLocalizedDescriptionKey : errorMessage }];
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, DISPATCH_TIME_NOW), dispatch_get_main_queue(), ^{
            [self showError:error];
        });
    }
}

- (void)readerSession:(nonnull NFCNDEFReaderSession *)session didInvalidateWithError:(nonnull NSError *)error
{
    NSLog(@"If no NFC reading UI is appearing, target may me missing the appropriate capability. Turn on Near Field Communication "
          @"Tag Reading under the Capabilities tab for the project’s target. A paid developer account is needed for this.");
    _session = nil;
}

- (void)setVendorIDOnAccessory
{
    NSLog(@"Call to setVendorIDOnAccessory");
    if (MTRGetConnectedDevice(^(MTRBaseDevice * _Nullable device, NSError * _Nullable error) {
            if (!device) {
                NSLog(@"Status: Failed to establish a connection with the device");
            }
        })) {
        NSLog(@"Status: Waiting for connection with the device");
    } else {
        NSLog(@"Status: Failed to trigger the connection with the device");
    }
}

// MARK: MTRDeviceControllerDelegate
- (void)controller:(MTRDeviceController *)controller commissioningSessionEstablishmentDone:(NSError * _Nullable)error
{
    if (error != nil) {
        NSLog(@"Got pairing error back %@", error);
    } else {
        MTRDeviceController * controller = InitializeMTR();
        uint64_t deviceId = MTRGetLastPairedDeviceId();
        MTRBaseDevice * device = [controller deviceBeingCommissionedWithNodeID:@(deviceId) error:NULL];
        if (device.sessionTransportType == MTRTransportTypeBLE) {
            dispatch_async(dispatch_get_main_queue(), ^{
                [self->_deviceList refreshDeviceList];
                [self retrieveAndSendWiFiCredentials];
            });
        } else {
            MTRCommissioningParameters * params = [[MTRCommissioningParameters alloc] init];
            params.deviceAttestationDelegate = [[CHIPToolDeviceAttestationDelegate alloc] initWithViewController:self];
            params.failSafeExpiryTimeoutSecs = @600;
            NSError * error;
            if (![controller commissionDevice:deviceId commissioningParams:params error:&error]) {
                NSLog(@"Failed to commission Device %llu, with error %@", deviceId, error);
            }
        }
    }
}

// MARK: UI Helper methods

- (void)manualCodeInitialState
{
    _deviceModelInfoView.hidden = YES;
    _readFromLedgerButton.hidden = YES;
    _redirectButton.hidden = YES;
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
    _deviceModelInfoView.hidden = YES;
    _redirectButton.hidden = YES;
    _readFromLedgerButton.hidden = YES;
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

- (void)showPayload:(MTRSetupPayload *)payload rawPayload:(NSString *)rawPayload isManualCode:(BOOL)isManualCode
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    self->_errorLabel.hidden = YES;
    // reset the view and remove any preferences that were stored from a previous scan
    self->_setupPayloadView.hidden = NO;
    self->_resetButton.hidden = NO;

    [self updateUIFields:payload rawPayload:rawPayload isManualCode:isManualCode];
    [self parseOptionalData:payload];
    [self handleRendezVous:payload rawPayload:rawPayload];
}

- (void)retrieveAndSendWiFiCredentials
{
    UIAlertController * alertController =
        [UIAlertController alertControllerWithTitle:@"WiFi Configuration"
                                            message:@"Input network SSID and password that your phone is connected to."
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

                                                 [strongSelf commissionWithSSID:networkSSID.text password:networkPassword.text];
                                             }
                                         }]];
    [self presentViewController:alertController animated:YES completion:nil];
}

- (void)commissionWithSSID:(NSString *)ssid password:(NSString *)password
{

    NSError * error;
    MTRDeviceController * controller = InitializeMTR();
    // create commissioning params in ObjC. Pass those in here with network credentials.
    // maybe this just becomes the new norm
    MTRCommissioningParameters * params = [[MTRCommissioningParameters alloc] init];
    params.wifiSSID = [ssid dataUsingEncoding:NSUTF8StringEncoding];
    params.wifiCredentials = [password dataUsingEncoding:NSUTF8StringEncoding];
    params.deviceAttestationDelegate = [[CHIPToolDeviceAttestationDelegate alloc] initWithViewController:self];
    params.failSafeExpiryTimeoutSecs = @600;

    uint64_t deviceId = MTRGetNextAvailableDeviceID() - 1;

    if (![controller commissionDevice:deviceId commissioningParams:params error:&error]) {
        NSLog(@"Failed to commission Device %llu, with error %@", deviceId, error);
    }
}

// MARK: MTRDeviceControllerDelegate
- (void)controller:(MTRDeviceController *)controller
    commissioningComplete:(NSError * _Nullable)error
                   nodeID:(NSNumber * _Nullable)nodeID
{
    if (error != nil) {
        NSLog(@"Error retrieving device informations over Mdns: %@", error);
        return;
    }
    // track this device
    MTRSetDevicePaired([nodeID unsignedLongLongValue], YES);
    [self setVendorIDOnAccessory];
}

// MARK: MTRDeviceControllerDelegate
- (void)controller:(MTRDeviceController *)controller readCommissioningInfo:(MTRProductIdentity *)info
{
    NSLog(@"readCommissioningInfo, vendorID:%@, productID:%@", info.vendorID, info.productID);
}

- (void)updateUIFields:(MTRSetupPayload *)payload rawPayload:(nullable NSString *)rawPayload isManualCode:(BOOL)isManualCode
{
    if (isManualCode) {
        _manualCodeLabel.hidden = NO;
        _manualCodeLabel.text = rawPayload;
        _versionLabel.text = NOT_APPLICABLE_STRING;
        _rendezVousInformation.text = NOT_APPLICABLE_STRING;
        _serialNumber.text = NOT_APPLICABLE_STRING;
    } else {
        _manualCodeLabel.hidden = YES;
        _versionLabel.text = [NSString stringWithFormat:@"%@", payload.version];
        if (payload.rendezvousInformation == nil) {
            _rendezVousInformation.text = NOT_APPLICABLE_STRING;
        } else {
            _rendezVousInformation.text = [NSString stringWithFormat:@"%lu", [payload.rendezvousInformation unsignedLongValue]];
        }
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
    _commissioningFlowLabel.text = [NSString stringWithFormat:@"%lu", payload.commissioningFlow];

    [self updateResultViewUI:_setupPayloadView];

    if (payload.commissioningFlow == MTRCommissioningFlowCustom) {
        _readFromLedgerButton.hidden = NO;
    }
}

- (void)parseOptionalData:(MTRSetupPayload *)payload
{
    NSLog(@"Payload vendorID %@", payload.vendorID);
    BOOL isSameVendorID = [payload.vendorID isEqualToNumber:[NSNumber numberWithInt:EXAMPLE_VENDOR_ID]];
    if (!isSameVendorID) {
        return;
    }

    NSArray * optionalInfo = [payload getAllOptionalVendorData:nil];
    for (MTROptionalQRCodeInfo * info in optionalInfo) {
        NSNumber * tag = info.tag;
        if (!tag) {
            continue;
        }

        BOOL isTypeString = [info.infoType isEqualToNumber:[NSNumber numberWithInt:MTROptionalQRCodeInfoTypeString]];
        if (!isTypeString) {
            return;
        }

        NSString * infoValue = info.stringValue;
        switch (tag.unsignedCharValue) {
        case EXAMPLE_VENDOR_TAG_IP:
            if ([infoValue length] > MAX_IP_LEN) {
                NSLog(@"Unexpected IP String... %@", infoValue);
            }
            break;
        }
    }
}

// MARK: Rendez Vous

- (void)handleRendezVous:(MTRSetupPayload *)payload rawPayload:(NSString *)rawPayload
{
    if (payload.rendezvousInformation == nil) {
        NSLog(@"Rendezvous Default");
        [self handleRendezVousDefault:rawPayload];
        return;
    }

    // TODO: This is a pretty broken way to handle a bitmask.
    switch ([payload.rendezvousInformation unsignedLongValue]) {
    case MTRDiscoveryCapabilitiesNone:
    case MTRDiscoveryCapabilitiesOnNetwork:
    case MTRDiscoveryCapabilitiesBLE:
    case MTRDiscoveryCapabilitiesAllMask:
        NSLog(@"Rendezvous Default");
        [self handleRendezVousDefault:rawPayload];
        break;
    case MTRDiscoveryCapabilitiesSoftAP:
        NSLog(@"Rendezvous Wi-Fi");
        [self handleRendezVousWiFi:[self getNetworkName:payload.discriminator]];
        break;
    }
}

- (NSString *)getNetworkName:(NSNumber *)discriminator
{
    NSString * peripheralDiscriminator = [NSString stringWithFormat:@"%04u", discriminator.unsignedShortValue];
    NSString * peripheralFullName = [NSString stringWithFormat:@"%@%@", NETWORK_CHIP_PREFIX, peripheralDiscriminator];
    return peripheralFullName;
}

- (void)_restartMatterStack
{
    self.chipController = MTRRestartController(self.chipController);
    dispatch_queue_t callbackQueue = dispatch_queue_create("com.csa.matter.qrcodevc.callback", DISPATCH_QUEUE_SERIAL);
    [self.chipController setDeviceControllerDelegate:self queue:callbackQueue];
}

- (void)handleRendezVousDefault:(NSString *)payload
{
    NSError * error;
    uint64_t deviceID = MTRGetNextAvailableDeviceID();

    // restart the Matter Stack before pairing (for reliability + testing restarts)
    [self _restartMatterStack];

    if ([self.chipController pairDevice:deviceID onboardingPayload:payload error:&error]) {
        deviceID++;
        MTRSetNextAvailableDeviceID(deviceID);
    }
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

- (void)displayQRCodeInSetupPayloadView:(MTRSetupPayload *)payload rawPayload:(NSString *)rawPayload error:(NSError *)error
{
    if (error) {
        [self showError:error];
    } else {
        [self showPayload:payload rawPayload:rawPayload isManualCode:NO];
    }
}

- (void)scannedQRCode:(NSString *)qrCode
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_captureSession stopRunning];
        [self->_session invalidateSession];
    });
    MTRQRCodeSetupPayloadParser * parser = [[MTRQRCodeSetupPayloadParser alloc] initWithBase38Representation:qrCode];
    NSError * error;
    _setupPayload = [parser populatePayload:&error];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 1.0 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        [self postScanningQRCodeState];

        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, INDICATOR_DELAY), dispatch_get_main_queue(), ^{
            [self displayQRCodeInSetupPayloadView:self->_setupPayload rawPayload:qrCode error:error];
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
- (void)displayManualCodeInSetupPayloadView:(MTRSetupPayload *)payload
                              decimalString:(NSString *)decimalString
                                  withError:(NSError *)error
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    if (error) {
        [self showError:error];
    } else {
        [self showPayload:payload rawPayload:decimalString isManualCode:YES];
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
    [self manualCodeInitialState];
    [self qrCodeInitialState];
}

- (IBAction)startScanningNFCTags:(id)sender
{
    if (!_session) {
        _session = [[NFCNDEFReaderSession alloc] initWithDelegate:self
                                                            queue:dispatch_queue_create(NULL, DISPATCH_QUEUE_CONCURRENT)
                                         invalidateAfterFirstRead:NO];
    }
    [_session beginSession];
}

- (IBAction)enteredManualCode:(id)sender
{
    NSString * decimalString = _manualCodeTextField.text;
    [self manualCodeEnteredStartState];

    MTRManualSetupPayloadParser * parser = [[MTRManualSetupPayloadParser alloc] initWithDecimalStringRepresentation:decimalString];
    NSError * error;
    _setupPayload = [parser populatePayload:&error];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, INDICATOR_DELAY), dispatch_get_main_queue(), ^{
        [self displayManualCodeInSetupPayloadView:self->_setupPayload decimalString:decimalString withError:error];
    });
    [_manualCodeTextField resignFirstResponder];
}

// Ledger

- (IBAction)readFromLedgerApi:(id)sender
{
    NSLog(@"Clicked readFromLedger...");
    _readFromLedgerButton.hidden = YES;
    _setupPayloadView.hidden = YES;
    _activityIndicator.hidden = NO;
    [_activityIndicator startAnimating];

    [self updateResultViewUI:_deviceModelInfoView];
    [self updateLedgerFields];
}

- (void)updateLedgerFields
{
    // check vendor Id and product Id
    NSLog(@"Validating Vender Id and Product Id...");
    if ([_vendorID.text isEqual:@"N/A"] || [_productID.text isEqual:@"N/A"]) {
        NSError * error = [[NSError alloc] initWithDomain:@"com.chiptool.customflow"
                                                     code:1
                                                 userInfo:@{ NSLocalizedDescriptionKey : @"Vendor ID or Product Id is invalid." }];
        [self showError:error];
        return;
    }
    // make API call
    NSLog(@"Making API call...");
    [self getRequest:[[[NSBundle mainBundle] objectForInfoDictionaryKey:@"LSEnvironment"]
                         objectForKey:@"CommissioningCustomFlowLedgerUrl"]
            vendorId:self->_vendorID.text
           productId:self->_productID.text];
}

- (void)getRequest:(NSString *)url vendorId:(NSString *)vendorId productId:(NSString *)productId
{
    [_activityIndicator startAnimating];
    _activityIndicator.hidden = NO;
    NSString * targetUrl = [NSString stringWithFormat:@"%@/%@/%@", url, vendorId, productId];
    NSMutableURLRequest * request = [[NSMutableURLRequest alloc] init];
    [request setHTTPMethod:@"GET"];
    [request setURL:[NSURL URLWithString:targetUrl]];

    [[[NSURLSession sharedSession]
        dataTaskWithRequest:request
          completionHandler:^(NSData * _Nullable data, NSURLResponse * _Nullable response, NSError * _Nullable error) {
              NSString * myString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
              NSLog(@"Data received: %@", myString);
              self->_ledgerRespond = [NSJSONSerialization JSONObjectWithData:data options:0 error:&error];
              [self getRequestCallback];
          }] resume];
}

- (void)getRequestCallback
{
    BOOL commissioningCustomFlowUseMockFlag = (BOOL)
        [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"LSEnvironment"] objectForKey:@"CommissioningCustomFlowUseMockFlag"];
    // use mock respond if useMockFlag is TRUE
    if (commissioningCustomFlowUseMockFlag) {
        NSLog(@"Using mock respond");
        _ledgerRespond = @{
            @"height" : @"mockHeight",
            @"result" : @ {
                @"vid" : @1,
                @"pid" : @1,
                @"cid" : @1,
                @"name" : @"mockName",
                @"owner" : @"mockOwner",
                @"description" : @"mockDescription",
                @"sku" : @"mockSku",
                @"firmware_version" : @"mockFirmware",
                @"hardware_version" : @"mockHardware",
                @"tis_or_trp_testing_completed" : @TRUE,
                @"CommissioningCustomFlowUrl" : @"https://lijusankar.github.io/commissioning-react-app/"
            }
        };
    }
    dispatch_async(dispatch_get_main_queue(), ^{
        self->_commissioningCustomFlowUrl.text =
            [[self->_ledgerRespond objectForKey:@"result"] objectForKey:@"CommissioningCustomFlowUrl"];
        [self->_activityIndicator stopAnimating];
        self->_activityIndicator.hidden = YES;
        self->_deviceModelInfoView.hidden = NO;
        self->_redirectButton.hidden = NO;
    });
}

// redirect
- (IBAction)redirectToUrl:(id)sender
{
    [self redirectToUrl];
}

- (void)redirectToUrl
{
    NSArray * redirectPayload = @[ @{
        @"version" : _versionLabel.text,
        @"vendorID" : _vendorID.text,
        @"productID" : _productID.text,
        @"commissioingFlow" : _commissioningFlowLabel.text,
        @"discriminator" : _discriminatorLabel.text,
        @"setupPinCode" : _setupPinCodeLabel.text,
        @"serialNumber" : _serialNumber.text,
        @"rendezvousInformation" : _rendezVousInformation.text
    } ];
    NSString * returnUrl =
        [[[NSBundle mainBundle] objectForInfoDictionaryKey:@"LSEnvironment"] objectForKey:@"CommissioningCustomFlowReturnUrl"];
    NSString * base64EncodedString = [self encodeStringTo64:redirectPayload];
    NSString * urlString =
        [NSString stringWithFormat:@"%@?payload=%@&returnUrl=%@", _commissioningCustomFlowUrl.text, base64EncodedString, returnUrl];
    NSURL * url = [NSURL URLWithString:urlString];
    [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
}

- (NSString *)encodeStringTo64:(NSArray *)fromArray
{
    NSData * jsonData = [NSJSONSerialization dataWithJSONObject:fromArray options:NSJSONWritingWithoutEscapingSlashes error:nil];
    NSString * base64String = [jsonData base64EncodedStringWithOptions:kNilOptions];
    return base64String;
}

@synthesize description;

@end

@implementation CHIPToolDeviceAttestationDelegate

- (instancetype)initWithViewController:(QRCodeViewController *)viewController
{
    if (self = [super init]) {
        _viewController = viewController;
    }
    return self;
}

- (void)deviceAttestation:(MTRDeviceController *)controller failedForDevice:(void *)device error:(NSError * _Nonnull)error
{
    dispatch_async(dispatch_get_main_queue(), ^{
        UIAlertController * alertController = [UIAlertController
            alertControllerWithTitle:@"Device Attestation"
                             message:@"Device Attestation failed for device under commissioning. Do you wish to continue pairing?"
                      preferredStyle:UIAlertControllerStyleAlert];

        [alertController addAction:[UIAlertAction actionWithTitle:@"No"
                                                            style:UIAlertActionStyleDefault
                                                          handler:^(UIAlertAction * action) {
                                                              NSError * err;
                                                              [controller continueCommissioningDevice:device
                                                                             ignoreAttestationFailure:NO
                                                                                                error:&err];
                                                          }]];

        [alertController addAction:[UIAlertAction actionWithTitle:@"Continue"
                                                            style:UIAlertActionStyleDefault
                                                          handler:^(UIAlertAction * action) {
                                                              NSError * err;
                                                              [controller continueCommissioningDevice:device
                                                                             ignoreAttestationFailure:YES
                                                                                                error:&err];
                                                          }]];

        [self.viewController presentViewController:alertController animated:YES completion:nil];
    });
}

@end
