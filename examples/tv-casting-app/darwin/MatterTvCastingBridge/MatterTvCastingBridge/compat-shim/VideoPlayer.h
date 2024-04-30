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

#import "../MCCastingPlayer.h"

#ifndef VideoPlayer_h
#define VideoPlayer_h

__attribute__((deprecated("Use the APIs described in /examples/tv-casting-app/APIs.md instead.")))
@interface VideoPlayer : NSObject

@property uint64_t nodeId;

@property uint8_t fabricIndex;

/**
 * @brief true if this VideoPlayer is connected, false otherwise
 */
@property bool isConnected;

/**
 * @brief contentApps will be nil the VideoPlayer is not connected
 */
@property NSMutableArray * contentApps;

@property NSString * deviceName;

@property uint16_t vendorId;

@property uint16_t productId;

@property uint16_t deviceType;

@property NSString * hostName;

@property NSString * instanceName;

@property uint16_t port;

@property NSString * MACAddress;

@property uint64_t lastDiscoveredMs;

@property bool isAsleep;

/**
 @brief true, if all the required fields are initialized, false otherwise
 */
@property BOOL isInitialized;

- (instancetype)initWithNodeId:(uint64_t)nodeId
                   fabricIndex:(uint8_t)fabricIndex
                   isConnected:(bool)isConnected
                   contentApps:(NSMutableArray *)contentApps
                    deviceName:(NSString *)deviceName
                      vendorId:(uint16_t)vendorId
                     productId:(uint16_t)productId
                    deviceType:(uint16_t)deviceType
                      hostName:(NSString *)hostName
                  instanceName:(NSString *)instanceName
                          port:(uint16_t)port
                    MACAddress:(NSString *)MACAddress
              lastDiscoveredMs:(uint64_t)lastDiscoveredMs;

- (instancetype)initWithCastingPlayer:(MCCastingPlayer *)castingPlayer;

- (MCCastingPlayer *)getCastingPlayer;

@end

#endif /* VideoPlayer_h */
