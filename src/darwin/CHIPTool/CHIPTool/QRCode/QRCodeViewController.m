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
#import <CHIP/CHIP.h>

// system imports
#import <AVFoundation/AVFoundation.h>

#define INDICATOR_DELAY 0.5 * NSEC_PER_SEC
#define ERROR_DISPLAY_TIME 2.0 * NSEC_PER_SEC
#define QR_CODE_FREEZE 1.0 * NSEC_PER_SEC

// The expected Vendor ID for CHIP demos
// Spells CHIP on a dialer
#define EXAMPLE_VENDOR_ID 3447
#define EXAMPLE_VENDOR_TAG_SSID 1
#define MAX_SSID_LEN 32

#define EXAMPLE_VENDOR_TAG_IP 2 
#define MAX_IP_LEN 15


#define NOT_APPLICABLE_STRING @"N/A"

static const NSString *ipKey = @"ipk";
static const NSString *portKey = @"pk";


@interface QRCodeViewController ()

@property (nonatomic, strong) AVCaptureSession * captureSession;
@property (nonatomic, strong) AVCaptureVideoPreviewLayer * videoPreviewLayer;
@end

@implementation QRCodeViewController {
    dispatch_queue_t _captureSessionQueue;
}

// MARK: UIViewController methods
- (void)viewDidLoad
{
    [super viewDidLoad];

    _doneManualCodeButton.layer.cornerRadius = 5;
    _doneManualCodeButton.clipsToBounds = YES;
    _manualCodeTextField.keyboardType = UIKeyboardTypeNumberPad;

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self manualCodeInitialState];
    [self qrCodeInitialState];
}

- (void)dismissKeyboard
{
    [_manualCodeTextField resignFirstResponder];
}

// MARK: UI Helper methods

- (void)manualCodeInitialState
{
    _setupPayloadView.hidden = YES;
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
    _qrCodeButton.hidden = NO;
    _doneQrCodeButton.hidden = YES;
    _activityIndicator.hidden = YES;
    _captureSession = nil;
    [_videoPreviewLayer removeFromSuperlayer];
}

- (void)scanningStartState
{
    _qrCodeButton.hidden = YES;
    _doneQrCodeButton.hidden = NO;
    _setupPayloadView.hidden = YES;
    _errorLabel.hidden = YES;
}

- (void)manualCodeEnteredStartState
{
    self->_activityIndicator.hidden = NO;
    [self->_activityIndicator startAnimating];
    _setupPayloadView.hidden = YES;
    _errorLabel.hidden = YES;
    _manualCodeTextField.text = @"";
}

- (void)postScanningQRCodeState
{
    _captureSession = nil;
    _qrCodeButton.hidden = NO;
    _doneQrCodeButton.hidden = YES;

    [_videoPreviewLayer removeFromSuperlayer];

    self->_activityIndicator.hidden = NO;
    [self->_activityIndicator startAnimating];
}

- (void)showError:(NSError *)error
{
    [self->_activityIndicator stopAnimating];
    self->_activityIndicator.hidden = YES;
    self->_manualCodeLabel.hidden = YES;

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

    if (decimalString) {
        self->_manualCodeLabel.hidden = NO;
        self->_manualCodeLabel.text = decimalString;
        self->_versionLabel.text = NOT_APPLICABLE_STRING;
        self->_discriminatorLabel.text = [NSString stringWithFormat:@"%@", payload.discriminator];
        self->_setupPinCodeLabel.text = [NSString stringWithFormat:@"%@", payload.setUpPINCode];
        self->_rendezVousInformation.text = NOT_APPLICABLE_STRING;
        self->_serialNumber.text = NOT_APPLICABLE_STRING;
        // TODO: Only display vid and pid if present
        self->_vendorID.text = [NSString stringWithFormat:@"%@", payload.vendorID];
        self->_productID.text = [NSString stringWithFormat:@"%@", payload.productID];
    } else {
        self->_manualCodeLabel.hidden = YES;
        self->_versionLabel.text = [NSString stringWithFormat:@"%@", payload.version];
        self->_discriminatorLabel.text = [NSString stringWithFormat:@"%@", payload.discriminator];
        self->_setupPinCodeLabel.text = [NSString stringWithFormat:@"%@", payload.setUpPINCode];
        self->_rendezVousInformation.text = [NSString stringWithFormat:@"%@", payload.rendezvousInformation];
        if ([payload.serialNumber length] > 0) {
            self->_serialNumber.text = payload.serialNumber;
        } else {
            self->_serialNumber.text = NOT_APPLICABLE_STRING;
        }
        // TODO: Only display vid and pid if present
        self->_vendorID.text = [NSString stringWithFormat:@"%@", payload.vendorID];
        self->_productID.text = [NSString stringWithFormat:@"%@", payload.productID];
    }
    self->_setupPayloadView.hidden = NO;
    
    NSLog(@"Payload vendorID %@", payload.vendorID);
    if ([payload.vendorID isEqualToNumber:[NSNumber numberWithInt:EXAMPLE_VENDOR_ID]]) {
        NSArray * optionalInfo = [payload getAllOptionalData:nil];
        NSLog(@"Count of payload info %@", @(optionalInfo.count));
        for (CHIPOptionalQRCodeInfo * info in optionalInfo) {
            NSNumber * tag = info.tag;
            if (tag) {
                switch (tag.unsignedCharValue) {
                    case EXAMPLE_VENDOR_TAG_SSID:
                        if ([info.infoType isEqualToNumber:[NSNumber numberWithInt:kOptionalQRCodeInfoTypeString]]) {
                            if ([info.stringValue length] > MAX_SSID_LEN) {
                                NSLog(@"Unexpected SSID String...");
                            } else {
                                // show SoftAP detection
                                [self RequestConnectSoftAPWithSSID:info.stringValue];
                            }
                        }
                    break;
                    case EXAMPLE_VENDOR_TAG_IP:
                        if ([info.infoType isEqualToNumber:[NSNumber numberWithInt:kOptionalQRCodeInfoTypeString]]) {
                            if ([info.stringValue length] > MAX_IP_LEN) {
                                NSLog(@"Unexpected IP String... %@", info.stringValue);
                            } else {
                                NSLog(@"Got IP String... %@", info.stringValue);
                                [[NSUserDefaults standardUserDefaults] setObject:info.stringValue forKey:ipKey];
                                [[NSUserDefaults standardUserDefaults] setInteger:8000 forKey:portKey];
                            }
                        }
                    break;
                }
                // If the vendor id and tag match the example values, there should be an ssid encoded
            }
        }
    }
}

- (void)RequestConnectSoftAPWithSSID:(NSString *)ssid
{
    NSString * message = [NSString
        stringWithFormat:@"The scanned CHIP accessory supports a SoftAP.\n\nSSID: %@\n\nUse WiFi Settings to connect to it.", ssid];
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
    CHIPQRCodeSetupPayloadParser * parser = [[CHIPQRCodeSetupPayloadParser alloc] initWithBase45Representation:qrCode];
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

@end
