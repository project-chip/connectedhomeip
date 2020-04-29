//
//  CHPError.h
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 22/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
FOUNDATION_EXPORT NSErrorDomain const CHPErrorDomain;

typedef NS_ERROR_ENUM(CHPErrorDomain, CHPErrorCode) {
    CHPErrorCodeUndefinedError = 0,
    CHPErrorCodeInvalidStringLength = 1,
    CHPErrorCodeInvalidIntegerValue = 2,
    CHPErrorCodeInvalidArgument = 3,
    CHPErrorCodeInvalidMessageLength = 4,
};

@interface CHPError : NSObject
+ (nullable NSError *)errorForCHIPErrorCode:(int32_t)errorCode;
@end

NS_ASSUME_NONNULL_END
