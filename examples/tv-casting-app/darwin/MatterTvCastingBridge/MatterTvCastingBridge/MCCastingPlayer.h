/**
 *
 *    Copyright (c) 2020-2023 Project CHIP Authors
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

#import "MCEndpointFilter.h"

#import <Foundation/Foundation.h>

#ifndef MCCastingPlayer_h
#define MCCastingPlayer_h

@class MCEndpoint;

/**
 * @brief MCCastingPlayer represents a Matter commissioner that is able to play media to a physical
 * output or to a display screen which is part of the device.
 */
@interface MCCastingPlayer : NSObject

+ (NSInteger)kMinCommissioningWindowTimeoutSec;

/**
 * @brief (async) Verifies that a connection exists with this CastingPlayer, or triggers a new session request. If the
 * CastingApp does not have the nodeId and fabricIndex of this CastingPlayer cached on disk, this will execute the user
 * directed commissioning process.
 *
 * @param completion - called back when the connection process completes. Parameter is nil if it completed successfully
 * @param timeout -  time (in sec) to keep the commissioning window open, if commissioning is required.
 * Needs to be >= CastingPlayer.kMinCommissioningWindowTimeoutSec.
 * @param desiredEndpointFilter - Attributes (such as VendorId) describing an Endpoint that the client wants to interact
 * with after commissioning. If this value is passed in, the VerifyOrEstablishConnection will force User Directed
 * Commissioning, in case the desired Endpoint is not found in the on-device cached information about the CastingPlayer
 * (if any)
 */
- (void)verifyOrEstablishConnectionWithCompletionBlock:(void (^_Nonnull)(NSError * _Nullable))completion timeout:(long long)timeout desiredEndpointFilter:(MCEndpointFilter * _Nullable)desiredEndpointFilter;

/**
 * @brief (async) Verifies that a connection exists with this MCCastingPlayer, or triggers a new session request. If the
 * MCCastingApp does not have the nodeId and fabricIndex of this MCCastingPlayer cached on disk, this will execute the user
 * directed commissioning process.
 *
 * @param completion - called back when the connection process completes. Parameter is nil if it completed successfully
 * @param desiredEndpointFilter - Attributes (such as VendorId) describing an MCEndpoint that the client wants to interact
 * with after commissioning. If this value is passed in, the VerifyOrEstablishConnection will force User Directed
 * Commissioning, in case the desired Endpoint is not found in the on-device cached information about the MCCastingPlayer
 * (if any)
 */
- (void)verifyOrEstablishConnectionWithCompletionBlock:(void (^_Nonnull)(NSError * _Nullable))completion desiredEndpointFilter:(MCEndpointFilter * _Nullable)desiredEndpointFilter;

/**
 * @brief Sets the internal connection state of this MCCastingPlayer to "disconnected"
 */
- (void)disconnect;

- (NSString * _Nonnull)identifier;
- (NSString * _Nonnull)deviceName;
- (uint16_t)vendorId;
- (uint16_t)productId;
- (uint32_t)deviceType;
- (NSArray * _Nonnull)ipAddresses;

/**
 * @brief Returns the NSArray of MCEndpoints associated with this MCCastingPlayer
 */
- (NSArray<MCEndpoint *> * _Nonnull)endpoints;

- (nonnull instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (nonnull instancetype)new UNAVAILABLE_ATTRIBUTE;

@end

#endif /* MCCastingPlayer_h */
