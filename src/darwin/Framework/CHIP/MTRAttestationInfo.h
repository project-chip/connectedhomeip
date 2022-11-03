/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

/**
 * Represents information relating to product attestation.
 *
 */
@interface AttestationInfo : NSObject

@property (nonatomic, copy) NSData * challenge;

@property (nonatomic, copy) NSData * nonce;

@property (nonatomic, copy) NSData * elements;

@property (nonatomic, copy) NSData * elementsSignature;

@property (nonatomic, copy) NSData * dac;

@property (nonatomic, copy) NSData * pai;

@property (nonatomic, copy) NSData * certificationDeclaration;

@property (nonatomic, copy) NSData * firmwareInfo;

- (instancetype)initWithChallenge:(NSData *)challenge
                            nonce:(NSData *)nonce
                         elements:(NSData *)elements
                elementsSignature:(NSData *)elementsSignature
                              dac:(NSData *)dac
                              pai:(NSData *)pai
         certificationDeclaration:(NSData *)certificationDeclaration
                     firmwareInfo:(NSData *)firmwareInfo;

@end

NS_ASSUME_NONNULL_END
