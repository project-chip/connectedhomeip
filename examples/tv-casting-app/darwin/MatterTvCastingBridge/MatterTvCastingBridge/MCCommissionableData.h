/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#import <Foundation/Foundation.h>

#ifndef MCCommissionableData_h
#define MCCommissionableData_h

@interface MCCommissionableData : NSObject

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

#endif /* MCCommissionableData_h */
