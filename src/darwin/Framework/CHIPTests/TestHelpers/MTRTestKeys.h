/**
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>
#import <Matter/Matter.h>

NS_ASSUME_NONNULL_BEGIN

@interface MTRTestKeys : NSObject <MTRKeypair>

@property (readonly, nonatomic, strong) NSData * ipk;

@property (readonly, nonatomic) NSData * publicKeyData;

// Count of how many times this keypair has been used to signMessageECDSA_DER.
@property (readonly, nonatomic, assign) unsigned signatureCount;

- (instancetype)init;

@end

NS_ASSUME_NONNULL_END
