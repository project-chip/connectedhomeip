/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRAttestationInfo.h"

NS_ASSUME_NONNULL_BEGIN

@implementation AttestationInfo : NSObject

- (instancetype)initWithChallenge:(NSData *)challenge
                            nonce:(NSData *)nonce
                         elements:(NSData *)elements
                elementsSignature:(NSData *)elementsSignature
                              dac:(NSData *)dac
                              pai:(NSData *)pai
         certificationDeclaration:(NSData *)certificationDeclaration
                     firmwareInfo:(NSData *)firmwareInfo
{
    if (self = [super init]) {
        _challenge = challenge;
        _nonce = nonce;
        _elements = elements;
        _elementsSignature = elementsSignature;
        _dac = dac;
        _pai = pai;
        _certificationDeclaration = certificationDeclaration;
        _firmwareInfo = firmwareInfo;
    }
    return self;
}

@end

NS_ASSUME_NONNULL_END
