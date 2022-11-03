/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Represents information relating to NOC CSR.
 *
 */
@interface CSRInfo : NSObject

@property (nonatomic, copy) NSData * nonce;

@property (nonatomic, copy) NSData * elements;

@property (nonatomic, copy) NSData * elementsSignature;

@property (nonatomic, copy) NSData * csr;

- (instancetype)initWithNonce:(NSData *)nonce
                     elements:(NSData *)elements
            elementsSignature:(NSData *)elementsSignature
                          csr:(NSData *)csr;

@end

NS_ASSUME_NONNULL_END
