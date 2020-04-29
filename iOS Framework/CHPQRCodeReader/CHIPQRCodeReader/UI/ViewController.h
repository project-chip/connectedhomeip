//
//  ViewController.h
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 20/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

@interface ViewController : UIViewController <AVCaptureMetadataOutputObjectsDelegate>

@property (weak, nonatomic) IBOutlet UIView                         *qrCodeViewPreview;
@property (weak, nonatomic) IBOutlet UITextField                    *manualCodeTextField;
@property (weak, nonatomic) IBOutlet UIButton                       *qrCodeButton;
@property (weak, nonatomic) IBOutlet UIButton                       *doneQrCodeButton;
@property (weak, nonatomic) IBOutlet UIButton                       *doneManualCodeButton;

@property (weak, nonatomic) IBOutlet UIActivityIndicatorView        *activityIndicator;
@property (weak, nonatomic) IBOutlet UILabel                        *errorLabel;

@property (weak, nonatomic) IBOutlet UIView                         *setupPayloadView;
@property (weak, nonatomic) IBOutlet UILabel                        *manualCodeLabel;
@property (weak, nonatomic) IBOutlet UILabel                        *versionLabel;
@property (weak, nonatomic) IBOutlet UILabel                        *discriminatorLabel;
@property (weak, nonatomic) IBOutlet UILabel                        *setupPinCodeLabel;
@property (weak, nonatomic) IBOutlet UILabel                        *rendezVousInformation;
@property (weak, nonatomic) IBOutlet UILabel                        *vendorID;
@property (weak, nonatomic) IBOutlet UILabel                        *productID;


- (IBAction)startScanningQRCode:(id)sender;
- (IBAction)stopScanningQRCode:(id)sender;
- (IBAction)enteredManualCode:(id)sender;
@end

