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

#import "CHIPViewControllerBase.h"
#import <AVFoundation/AVFoundation.h>

@interface QRCodeViewController : CHIPViewControllerBase <AVCaptureMetadataOutputObjectsDelegate>

@property (weak, nonatomic) IBOutlet UIView * qrCodeViewPreview;
@property (weak, nonatomic) IBOutlet UITextField * manualCodeTextField;
@property (weak, nonatomic) IBOutlet UIButton * qrCodeButton;
@property (weak, nonatomic) IBOutlet UIButton * doneQrCodeButton;
@property (weak, nonatomic) IBOutlet UIButton * doneManualCodeButton;
@property (weak, nonatomic) IBOutlet UIButton * resetButton;

@property (weak, nonatomic) IBOutlet UIActivityIndicatorView * activityIndicator;
@property (weak, nonatomic) IBOutlet UILabel * errorLabel;

@property (weak, nonatomic) IBOutlet UIView * setupPayloadView;
@property (weak, nonatomic) IBOutlet UILabel * manualCodeLabel;
@property (weak, nonatomic) IBOutlet UILabel * versionLabel;
@property (weak, nonatomic) IBOutlet UILabel * discriminatorLabel;
@property (weak, nonatomic) IBOutlet UILabel * setupPinCodeLabel;
@property (weak, nonatomic) IBOutlet UILabel * rendezVousInformation;
@property (weak, nonatomic) IBOutlet UILabel * vendorID;
@property (weak, nonatomic) IBOutlet UILabel * productID;
@property (weak, nonatomic) IBOutlet UILabel * serialNumber;

- (IBAction)startScanningQRCode:(id)sender;
- (IBAction)stopScanningQRCode:(id)sender;
- (IBAction)enteredManualCode:(id)sender;
- (IBAction)resetView:(id)sender;
@end
