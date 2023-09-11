/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRCommissionableData.h"
#import <Foundation/Foundation.h>

@implementation MTRCommissionableData

- (instancetype)initWithPasscode:(uint32_t)passcode
                   discriminator:(uint16_t)discriminator
           spake2pIterationCount:(uint32_t)spake2pIterationCount
                 spake2pVerifier:(NSData * _Nullable)spake2pVerifier
                     spake2pSalt:(NSData * _Nullable)spake2pSalt
{
    if (self = [super init]) {
        _passcode = passcode;
        _discriminator = discriminator;
        _spake2pIterationCount = spake2pIterationCount;
        _spake2pVerifier = spake2pVerifier;
        _spake2pVerifier = spake2pVerifier;
    }
    return self;
}

@end
