/**
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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

#import "DiscoveredNodeData.h"
#import <Foundation/Foundation.h>

#ifndef CastingServerBridge_h
#define CastingServerBridge_h

@interface CastingServerBridge : NSObject

+ (CastingServerBridge * _Nullable)getSharedInstance;

- (void)discoverCommissioners:(dispatch_queue_t _Nonnull)clientQueue
    discoveryRequestSentHandler:(nullable void (^)(bool))discoveryRequestSentHandler;

- (void)getDiscoveredCommissioner:(int)index
                      clientQueue:(dispatch_queue_t _Nonnull)clientQueue
    discoveredCommissionerHandler:(nullable void (^)(DiscoveredNodeData * _Nullable))discoveredCommissionerHandler;

- (void)sendUserDirectedCommissioningRequest:(NSString * _Nonnull)commissionerIpAddress
                            commissionerPort:(uint16_t)commissionerPort
                           platformInterface:(unsigned int)platformInterface
                                 clientQueue:(dispatch_queue_t _Nonnull)clientQueue
                       udcRequestSentHandler:(nullable void (^)(bool))udcRequestSentHandler;

@end

#endif /* CastingServerBridge_h */
