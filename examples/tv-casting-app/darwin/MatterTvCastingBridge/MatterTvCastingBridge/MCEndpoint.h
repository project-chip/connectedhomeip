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

#import "MCCastingPlayer.h"
#import "MCCluster.h"
#import "zap-generated/MCEndpointClusterType.h"

#import <Foundation/Foundation.h>

#ifndef MCEndpoint_h
#define MCEndpoint_h

@class MCCastingPlayer;
@class MCCluster;

@interface MCEndpoint : NSObject

- (NSNumber * _Nonnull)identifier;
- (NSNumber * _Nonnull)vendorId;
- (NSNumber * _Nonnull)productId;
- (NSArray * _Nonnull)deviceTypeList;
- (MCCastingPlayer * _Nonnull)castingPlayer;

- (nonnull instancetype)init UNAVAILABLE_ATTRIBUTE;
+ (nonnull instancetype)new UNAVAILABLE_ATTRIBUTE;

- (BOOL)hasCluster:(MCEndpointClusterType)type;
- (MCCluster * _Nullable)clusterForType:(MCEndpointClusterType)type;

- (NSString * _Nonnull)description;

@end

#endif /* MCEndpoint_h */
