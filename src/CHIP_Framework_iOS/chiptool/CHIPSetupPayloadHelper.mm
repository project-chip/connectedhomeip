//
//  CHIPSetupPayloadHelper.m
//  chiptool
//
//  Created by Shana Azria on 15/05/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "CHIPSetupPayloadHelper.h"
#import "SetupPayloadHelper.hpp"
#import "CHIPError.h"

@implementation CHIPSetupPayloadHelper


+ (NSString *)generateQRCodeFromFilePath:(NSString *)filePath error:(NSError * __autoreleasing *)error
{
    std::string outCode;
    CHIP_ERROR err = chip::generateQRCodeFromFilePath(std::string([filePath UTF8String]), outCode);
    if (err != CHIP_NO_ERROR && error) {
        *error = [NSError errorWithDomain:@"testdomain" code:1 userInfo:nil];
    }
    return [NSString stringWithUTF8String:outCode.c_str()];
}

+ (NSString *)generateManualCodeFromFilePath:(NSString *)filePath error:(NSError * __autoreleasing *)error
{
    std::string outCode;
    CHIP_ERROR err = chip::generateManualCodeFromFilePath(std::string([filePath UTF8String]), outCode);
    if (err != CHIP_NO_ERROR && error) {
        *error = [NSError errorWithDomain:@"testdomain" code:1 userInfo:nil];
    }
    return [NSString stringWithUTF8String:outCode.c_str()];
}
@end
