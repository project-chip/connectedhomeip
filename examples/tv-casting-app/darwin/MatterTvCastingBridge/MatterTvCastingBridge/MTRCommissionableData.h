/**
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#ifndef MTRCommissionableData_h
#define MTRCommissionableData_h

@interface MTRCommissionableData : NSObject

@property (nonatomic, readonly) uint32_t passcode;

@property (nonatomic, readonly) uint16_t discriminator;

@property (nonatomic, readonly) uint32_t spake2pIterationCount;

@property (nonatomic, strong, readonly) NSData * _Nullable spake2pVerifier;

@property (nonatomic, strong, readonly) NSData * _Nullable spake2pSalt;

- (instancetype _Nonnull)initWithPasscode:(uint32_t)passcode
                            discriminator:(uint16_t)discriminator
                    spake2pIterationCount:(uint32_t)spake2pIterationCount
                          spake2pVerifier:(NSData * _Nullable)spake2pVerifier
                              spake2pSalt:(NSData * _Nullable)spake2pSalt;

@end

#endif /* MTRCommissionableData_h */
