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

#import "MTRCSRInfo.h"

NS_ASSUME_NONNULL_BEGIN

@implementation MTROperationalCSRInfo : NSObject

- (instancetype)initWithCSR:(MTRCSRDERBytes)csr
                   csrNonce:(NSData *)csrNonce
             csrElementsTLV:(MTRTLVBytes)csrElementsTLV
       attestationSignature:(NSData *)attestationSignature;
{
    if (self = [super init]) {
        _csr = csr;
        _csrNonce = csrNonce;
        _csrElementsTLV = csrElementsTLV;
        _attestationSignature = attestationSignature;
    }
    return self;
}
@end

@implementation CSRInfo : NSObject

- (instancetype)initWithNonce:(NSData *)nonce
                     elements:(NSData *)elements
            elementsSignature:(NSData *)elementsSignature
                          csr:(NSData *)csr
{
    if (self = [super init]) {
        _nonce = nonce;
        _elements = elements;
        _elementsSignature = elementsSignature;
        _csr = csr;
    }
    return self;
}
@end

NS_ASSUME_NONNULL_END
