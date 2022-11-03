/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <AVFoundation/AVFoundation.h>
#import <CoreNFC/CoreNFC.h>
#import <Matter/Matter.h>
#import <UIKit/UIKit.h>

@interface QRCodeViewController
    : UIViewController <AVCaptureMetadataOutputObjectsDelegate, MTRDevicePairingDelegate, NFCNDEFReaderSessionDelegate>

@end
