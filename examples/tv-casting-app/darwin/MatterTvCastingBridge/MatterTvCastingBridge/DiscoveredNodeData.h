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

#import <Foundation/Foundation.h>

#import "VideoPlayer.h"

#ifndef DiscoveredNodeData_h
#define DiscoveredNodeData_h

@interface DiscoveredNodeData : NSObject

@property NSString * deviceName;

@property uint16_t vendorId;

@property uint16_t productId;

@property uint16_t deviceType;

@property uint16_t longDiscriminator;

@property uint8_t commissioningMode;

@property uint16_t pairingHint;

@property const uint8_t * rotatingId;

@property size_t rotatingIdLen;

@property NSString * instanceName;

@property uint16_t port;

@property NSString * hostName;

@property unsigned int platformInterface;

@property NSMutableArray * ipAddresses;

@property size_t numIPs;

- (DiscoveredNodeData *)initWithDeviceName:(NSString *)deviceName vendorId:(uint16_t)vendorId productId:(uint16_t)productId;

- (bool)isPreCommissioned;

- (VideoPlayer *)getConnectableVideoPlayer;

- (void)setConnectableVideoPlayer:(VideoPlayer *)videoPlayer;

@end

#endif /* DiscoveredNodeData_h */
