//
//  CHIPSetupPayloadHelper.h
//  chiptool
//
//  Created by Shana Azria on 15/05/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface CHIPSetupPayloadHelper : NSObject
+ (NSString *)generateQRCodeFromFilePath:(NSString *)filePath error:(NSError * __autoreleasing *)error;
+ (NSString *)generateManualCodeFromFilePath:(NSString *)filePath error:(NSError * __autoreleasing *)error;
@end

NS_ASSUME_NONNULL_END
