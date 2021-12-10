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
#import "CustomFlowViewController.h"

// local imports
#import "CHIPUIViewUtils.h"
#import "DefaultsUtils.h"
#import "DeviceSelector.h"
#import <CHIP/CHIP.h>
#import <CHIP/CHIPSetupPayload.h>

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

@interface CustomFlowViewController ()

@property (nonatomic, strong) AVCaptureSession * captureSession;
@property (nonatomic, strong) AVCaptureVideoPreviewLayer * videoPreviewLayer;

@property (strong, nonatomic) UIView * qrCodeViewPreview;

@property (strong, nonatomic) UIButton * nfcScanButton;
@property (readwrite) BOOL sessionIsActive;

@property (strong, nonatomic) UILabel * titleLabel;
@property (strong, nonatomic) UIView * setupPayloadView;
@property (strong, nonatomic) UIButton * readFromLedgerButton;
@property (strong, nonatomic) UIButton * redirectButton;
@property (strong, nonatomic) UILabel * versionLabel;
@property (strong, nonatomic) UILabel * discriminatorLabel;
@property (strong, nonatomic) UILabel * setupPinCodeLabel;
@property (strong, nonatomic) UILabel * rendezVousInformation;
@property (strong, nonatomic) UILabel * vendorID;
@property (strong, nonatomic) UILabel * productID;
@property (strong, nonatomic) UILabel * vendorIDDeviceInfo;
@property (strong, nonatomic) UILabel * productIDDeviceInfo;
@property (strong, nonatomic) UILabel * serialNumber;
@property (strong, nonatomic) UILabel * commissioningFlow;
@property (strong, nonatomic) UILabel * commissioningUrl;
@property (strong, nonatomic) UIView * deviceModelInfoView;
@property (strong, nonatomic) NSArray * redirectPayload;

@property (strong, nonatomic) UIActivityIndicatorView * activityIndicator;
@property (strong, nonatomic) UILabel * errorLabel;

@property (strong, nonatomic) CHIPSetupPayload * setupPayload;
@end

NSString * const ledgerUrl = @"https://dcl.dev.dsr-corporation.com/api/modelinfo/models";

@implementation CustomFlowViewController {
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
    _titleLabel = [CHIPUIViewUtils addTitle:@"CHIP Custom Flow" toView:self.view];

    // stack view
    UIStackView * stackView = [UIStackView new];
    stackView.axis = UILayoutConstraintAxisVertical;
    stackView.distribution = UIStackViewDistributionFill;
    stackView.alignment = UIStackViewAlignmentLeading;
    stackView.spacing = 15;
    [self.view addSubview:stackView];

    stackView.translatesAutoresizingMaskIntoConstraints = false;
    [stackView.topAnchor constraintEqualToAnchor:_titleLabel.bottomAnchor constant:30].active = YES;
    [stackView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [stackView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;

    // Results view
    _setupPayloadView = [UIView new];
    [self.view addSubview:_setupPayloadView];

    _setupPayloadView.translatesAutoresizingMaskIntoConstraints = false;
    [_setupPayloadView.topAnchor constraintEqualToAnchor:stackView.bottomAnchor constant:10].active = YES;
    [_setupPayloadView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_setupPayloadView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_setupPayloadView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;
    [self addDetailSubview:_setupPayloadView];
    
    _deviceModelInfoView = [UIView new];
    [self.view addSubview:_deviceModelInfoView];

    _deviceModelInfoView.translatesAutoresizingMaskIntoConstraints = false;
    [_deviceModelInfoView.topAnchor constraintEqualToAnchor:stackView.bottomAnchor constant:10].active = YES;
    [_deviceModelInfoView.leadingAnchor constraintEqualToAnchor:self.view.leadingAnchor constant:30].active = YES;
    [_deviceModelInfoView.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    [_deviceModelInfoView.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;
    [self addDetailSubview:_deviceModelInfoView];
    
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
    [_qrCodeViewPreview.topAnchor constraintEqualToAnchor:_titleLabel.bottomAnchor constant:30].active = YES;
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
    [_readFromLedgerButton.bottomAnchor constraintEqualToAnchor:self.view.safeAreaLayoutGuide.bottomAnchor constant:-30].active = YES;
    [_readFromLedgerButton.trailingAnchor constraintEqualToAnchor:self.view.trailingAnchor constant:-30].active = YES;
    
    // Redirect Custom Flow button
    _redirectButton = [UIButton new];
    [_redirectButton setTitle:@"Redirect" forState:UIControlStateNormal];
    [_redirectButton addTarget:self action:@selector(redirect:) forControlEvents:UIControlEventTouchUpInside];
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
    _commissioningFlow = [UILabel new];
    _commissioningUrl = [UILabel new];
    _vendorIDDeviceInfo = [UILabel new];
    _productIDDeviceInfo = [UILabel new];
}


- (void)addDetailSubview:(UIView *)superView
{
    // Results scroll view
    UIScrollView * resultsScrollView = [UIScrollView new];
    [superView addSubview:resultsScrollView];

    resultsScrollView.translatesAutoresizingMaskIntoConstraints = false;
    [resultsScrollView.topAnchor constraintEqualToAnchor:_titleLabel.bottomAnchor constant:10].active = YES;
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
    
    if (superView == _setupPayloadView) {
        [self addResultsUIToStackView:parserResultsView];
    } else if (superView == _deviceModelInfoView) {
        [self addDeviceInfoUIToStackView:parserResultsView];
    }
}

- (void)addResultsUIToStackView:(UIStackView *)stackView
{
    NSArray<NSString *> * resultLabelTexts =
        @[ @"Version", @"Vendor ID", @"Product ID", @"Discriminator", @"Setup PIN Code", @"Rendez Vous Information", @"Serial #", @"Commissioning Flow" ];
    NSArray<UILabel *> * resultLabels =
        @[ _versionLabel, _vendorID, _productID, _discriminatorLabel, _setupPinCodeLabel, _rendezVousInformation, _serialNumber, _commissioningFlow ];
    [self addItemToStackView:stackView resultLabels:resultLabels resultLabelTexts:resultLabelTexts];
}

- (void)addDeviceInfoUIToStackView:(UIStackView *)stackView
{
    NSArray<NSString *> * resultLabelTexts =
        @[ @"Vendor ID", @"Product ID", @"Commissioning URL" ];
    NSArray<UILabel *> * resultLabels =
        @[ _vendorIDDeviceInfo, _productIDDeviceInfo, _commissioningUrl ];
    [self addItemToStackView:stackView resultLabels:resultLabels resultLabelTexts:resultLabelTexts];
}

- (void)addItemToStackView:(UIStackView *) stackView
              resultLabels:(NSArray<UILabel *> *) resultLabels
          resultLabelTexts:(NSArray<NSString *> *) resultLabelTexts
{
    for (int i = 0; i < resultLabels.count && i < resultLabelTexts.count; i++) {
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

- (void)viewDidLoad
{
    [super viewDidLoad];
    [self setupUI];
    [self initialState];
    [self qrCodeInitialState];
}

// MARK: UI Helper methods

- (void)initialState
{
    _deviceModelInfoView.hidden = YES;
    _setupPayloadView.hidden = YES;
    _readFromLedgerButton.hidden = YES;
    _redirectButton.hidden = YES;
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
    [self changeNavBarButtonToCamera];
    _activityIndicator.hidden = YES;
    _captureSession = nil;
    [_videoPreviewLayer removeFromSuperlayer];
}

- (void)scanningStartState
{
    [self changeNavBarButtonToCancel];
    _setupPayloadView.hidden = YES;
    _deviceModelInfoView.hidden = YES;
    _readFromLedgerButton.hidden = YES;
    _redirectButton.hidden = YES;
    _errorLabel.hidden = YES;
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
    _readFromLedgerButton.hidden = YES;
    _redirectButton.hidden = YES;

    self->_errorLabel.text = error.localizedDescription;
    self->_errorLabel.hidden = NO;
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, ERROR_DISPLAY_TIME), dispatch_get_main_queue(), ^{
        self->_errorLabel.hidden = YES;
    });
}

- (void)showPayload:(CHIPSetupPayload *)payload rawPayload:(NSString *)rawPayload
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    self->_errorLabel.hidden = YES;
    // reset the view and remove any preferences that were stored from a previous scan
    self->_setupPayloadView.hidden = NO;

    [self updateUIFields:payload rawPayload:rawPayload];
}

- (void)updateUIFields:(CHIPSetupPayload *)payload rawPayload:(nullable NSString *)rawPayload
{
    _versionLabel.text = [NSString stringWithFormat:@"%@", payload.version];
    _rendezVousInformation.text = [NSString stringWithFormat:@"%lu", payload.rendezvousInformation];
    if ([payload.serialNumber length] > 0) {
        self->_serialNumber.text = payload.serialNumber;
    } else {
        self->_serialNumber.text = NOT_APPLICABLE_STRING;
    }

    _discriminatorLabel.text = [NSString stringWithFormat:@"%@", payload.discriminator];
    _setupPinCodeLabel.text = [NSString stringWithFormat:@"%@", payload.setUpPINCode];
    // TODO: Only display vid and pid if present
    _vendorID.text = [NSString stringWithFormat:@"%@", payload.vendorID];
    _productID.text = [NSString stringWithFormat:@"%@", payload.productID];
    _vendorIDDeviceInfo.text = [NSString stringWithFormat:@"%@", payload.vendorID];
    _productIDDeviceInfo.text = [NSString stringWithFormat:@"%@", payload.productID];
    _commissioningFlow.text = [NSString stringWithFormat:@"%lu", payload.commissioningFlow];
    
    if (payload.commissioningFlow == 2) {
        _readFromLedgerButton.hidden = NO;
    }
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

- (void)displayQRCodeInSetupPayloadView:(CHIPSetupPayload *)payload rawPayload:(NSString *)rawPayload error:(NSError *)error
{
    if (error) {
        [self showError:error];
    } else {
        [self showPayload:payload rawPayload:rawPayload];
    }
}

- (void)scannedQRCode:(NSString *)qrCode
{
    dispatch_async(dispatch_get_main_queue(), ^{
        [self->_captureSession stopRunning];
    });
    CHIPQRCodeSetupPayloadParser * parser = [[CHIPQRCodeSetupPayloadParser alloc] initWithBase38Representation:qrCode];
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

// Ledger

- (IBAction)readFromLedgerApi:(id)sender
{
    NSLog(@"Clicked readFromLedger...");
    _readFromLedgerButton.hidden = YES;
    _setupPayloadView.hidden = YES;
    _activityIndicator.hidden = NO;
    [_activityIndicator startAnimating];
    
    [self updateLedgerFields];
}

- (void)updateLedgerFields
{
    // check vendor Id and product Id
    NSLog(@"Validating Vender Id and Product Id...");
    if ([_vendorIDDeviceInfo.text isEqual:@"N/A"] || [_productIDDeviceInfo.text isEqual:@"N/A"] ) {
        NSError * error = [[NSError alloc] initWithDomain:@"com.chiptool.customflow"
                                                     code:1
                                                 userInfo:@{ NSLocalizedDescriptionKey : @"Vendor ID or Product Id is invalid." }];
        [self showError:error];
        return;
    }
    // make API call
    NSLog(@"Making API call...");
    // [self getRequest:ledgerUrl vendorId:_vendorIDDeviceInfo.text productId:_productIDDeviceInfo.text];
    // mock the respond for now, since the ledge Url is not determined yet.
    _commissioningUrl.text = @"https://lijusankar.github.io/commissioning-react-app/";
    
    // getting redirecting prepared
    NSLog(@"Updating Url and redirectButton...");
    
    
    
    [_activityIndicator stopAnimating];
    _activityIndicator.hidden = YES;
    _deviceModelInfoView.hidden = NO;
    _redirectButton.hidden = NO;
}

- (void)getRequest:(NSString *)url
                  vendorId:(NSString *)vendorId
                 productId:(NSString *)productId
{
    NSString *targetUrl = [NSString stringWithFormat:@"%@/%@/%@", url, vendorId, productId];
    NSMutableURLRequest *request = [[NSMutableURLRequest alloc] init];
    [request setHTTPMethod:@"GET"];
    [request setURL:[NSURL URLWithString:targetUrl]];

    [[[NSURLSession sharedSession] dataTaskWithRequest:request completionHandler:
      ^(NSData * _Nullable data,
        NSURLResponse * _Nullable response,
        NSError * _Nullable error) {

          NSString *myString = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
          NSLog(@"Data received: %@", myString);
    }] resume];
}

// redirect
- (IBAction)redirect:(id)sender
{
    NSLog(@"Clicked redirect...");
    [self redirectToUrl];
}

- (void)redirectToUrl
{
    _redirectPayload = @[ @{
        @"version": _versionLabel.text,
        @"vendorID": _vendorID.text,
        @"productID": _productID.text,
        @"commissioingFlow": _commissioningFlow.text,
        @"discriminator": _discriminatorLabel.text,
        @"setupPinCode": _setupPinCodeLabel.text,
        @"serialNumber": _serialNumber.text,
        @"rendezvousInformation": _rendezVousInformation.text
    } ];
    NSString * base64EncodedString = [self encodeStringTo64:_redirectPayload];
    NSString * urlString = [NSString stringWithFormat:@"%@?payload=%@&returnUrl=%@", _commissioningUrl.text, base64EncodedString, @"https://www.baidu.com"];
    NSURL * url = [NSURL URLWithString:urlString];
    NSLog(@"%@", url);
    [[UIApplication sharedApplication] openURL:url options:@{} completionHandler:nil];
}

- (NSString*)encodeStringTo64:(NSArray*)fromArray
{
    NSData * jsonData = [NSJSONSerialization dataWithJSONObject:fromArray options:NSJSONWritingWithoutEscapingSlashes error:nil];
    NSString * base64String = [jsonData base64EncodedStringWithOptions:kNilOptions];
    return base64String;
}


@synthesize description;

@end
