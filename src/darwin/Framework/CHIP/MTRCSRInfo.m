/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRCSRInfo.h"

NS_ASSUME_NONNULL_BEGIN

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
