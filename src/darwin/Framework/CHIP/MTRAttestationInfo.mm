/**
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
