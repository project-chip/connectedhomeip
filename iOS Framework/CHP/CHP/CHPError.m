//
//  CHPError.m
//  CHIPQRCodeReader
//
//  Created by Shana Azria on 22/04/2020.
//  Copyright © 2020 CHIP. All rights reserved.
//

#import "CHPError.h"

NSString *const CHPErrorDomain = @"CHPErrorDomain";


@implementation CHPError

+ (NSError *)errorForCHIPErrorCode:(int32_t)errorCode
{
    switch (errorCode) {
        case 4030:
            return [NSError errorWithDomain:CHPErrorDomain
                                       code:CHPErrorCodeInvalidStringLength
                                   userInfo:@{
                                   NSLocalizedDescriptionKey: NSLocalizedString(@"A list length is invalid.", nil)}];
        case 4145:
            return [NSError errorWithDomain:CHPErrorDomain
                                       code:CHPErrorCodeInvalidIntegerValue
                                   userInfo:@{
                                       NSLocalizedDescriptionKey: NSLocalizedString(@"Unexpected integer value.", nil)}];
        case 4046:
                   return [NSError errorWithDomain:CHPErrorDomain
                                              code:CHPErrorCodeInvalidArgument
                                          userInfo:@{
                                              NSLocalizedDescriptionKey: NSLocalizedString(@"An argument is invalid.", nil)}];
        case 4023:
                   return [NSError errorWithDomain:CHPErrorDomain
                                              code:CHPErrorCodeInvalidMessageLength
                                          userInfo:@{
                                              NSLocalizedDescriptionKey: NSLocalizedString(@"A message length is invalid.", nil)}];
        default:
            return [NSError errorWithDomain:CHPErrorDomain
                                       code:CHPErrorCodeUndefinedError
                                   userInfo:@{
                                       NSLocalizedDescriptionKey: NSLocalizedString(@"Undefined error.", nil)}];;
    }
}
@end
