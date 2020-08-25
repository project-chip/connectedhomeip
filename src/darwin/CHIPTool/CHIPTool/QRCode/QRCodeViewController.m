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
#import "DefaultsUtils.h"
#import <CHIP/CHIP.h>

// system imports
#import <AVFoundation/AVFoundation.h>

#define INDICATOR_DELAY 0.5 * NSEC_PER_SEC
#define ERROR_DISPLAY_TIME 2.0 * NSEC_PER_SEC
#define QR_CODE_FREEZE 1.0 * NSEC_PER_SEC

// The expected Vendor ID for CHIP demos
// Spells CHIP on a dialer
#define EXAMPLE_VENDOR_ID 2447

#define EXAMPLE_VENDOR_TAG_IP 1
#define MAX_IP_LEN 46

#define NETWORK_CHIP_PREFIX @"CHIP-"

#define NOT_APPLICABLE_STRING @"N/A"

static NSString * const ipKey = @"ipk";

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
    _resetButton.layer.cornerRadius = 5;
    _resetButton.clipsToBounds = YES;
    _manualCodeTextField.keyboardType = UIKeyboardTypeNumberPad;

    __weak typeof(self) weakSelf = self;
    self.onConnectedBlock = ^() {
        typeof(self) strongSelf = weakSelf;
        [strongSelf onConnected];
    };

    self.onMessageBlock = ^(NSString * message) {
        typeof(self) strongSelf = weakSelf;
        [strongSelf onMessage:message];
    };

    self.onErrorBlock = ^(NSString * error) {
        typeof(self) strongSelf = weakSelf;
        [strongSelf onError:error];
    };

    UITapGestureRecognizer * tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(dismissKeyboard)];
    [self.view addGestureRecognizer:tap];

    [self manualCodeInitialState];
    [self qrCodeInitialState];
}

- (void)onConnected
{
    [self retrieveAndSendWifiCredentials];
}

- (void)onMessage:(NSString *)message
{
    [[NSUserDefaults standardUserDefaults] setObject:message forKey:ipKey];
    NSError * error;
    [self.chipController disconnect:&error];
}

- (void)onError:(NSString *)error
{
    NSLog(@"Receive an error: %@", error);
}

- (void)viewDidDisappear:(BOOL)animated
{
    [super viewDidDisappear:animated];
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
    // show the reset button if there's scanned data saved
    _resetButton.hidden = ![self hasScannedConnectionInfo];
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
    // reset the view and remove any preferences that were stored from a previous scan
    if ([self hasScannedConnectionInfo]) {
        [[NSUserDefaults standardUserDefaults] removeObjectForKey:ipKey];
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
                [[NSUserDefaults standardUserDefaults] setObject:infoValue forKey:ipKey];
            }
            break;
        }
    }
}

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
    NSString * ipAddress = [[NSUserDefaults standardUserDefaults] stringForKey:ipKey];
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
    [[NSUserDefaults standardUserDefaults] removeObjectForKey:ipKey];
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

@end
