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

#import "DiscoveredNodeData.h"
#import "VideoPlayer.h"

#include <lib/dnssd/Resolver.h>

@interface DiscoveredNodeData ()

@property (nonatomic) VideoPlayer * connectableVideoPlayer;

@property (nonatomic) MCCastingPlayer * castingPlayer;

@end

@implementation DiscoveredNodeData

- (DiscoveredNodeData *)initWithDeviceName:(NSString *)deviceName vendorId:(uint16_t)vendorId productId:(uint16_t)productId
{
    self = [super init];
    if (self) {
        _deviceName = deviceName;
        _vendorId = vendorId;
        _productId = productId;
    }
    return self;
}

- (instancetype)initWithCastingPlayer:(MCCastingPlayer *)castingPlayer
{
    self = [super init];
    if (self) {
        _castingPlayer = castingPlayer;
        _deviceType = castingPlayer.deviceType;
        _vendorId = castingPlayer.vendorId;
        _productId = castingPlayer.productId;
        _deviceName = castingPlayer.deviceName;
        _instanceName = castingPlayer.instanceName;
        _hostName = castingPlayer.hostName;
        _numIPs = castingPlayer.ipAddresses.count;
        _ipAddresses = [castingPlayer.ipAddresses mutableCopy];
        _connectableVideoPlayer = [[VideoPlayer alloc] initWithCastingPlayer:castingPlayer];
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ with Product ID: %d and Vendor ID: %d", _deviceName, _productId, _vendorId];
}

- (BOOL)isEqualToDiscoveredNodeData:(DiscoveredNodeData *)other
{
    return [self.instanceName isEqualToString:other.instanceName];
}

- (BOOL)isEqual:(id)other
{
    if (other == nil) {
        return NO;
    }

    if (self == other) {
        return YES;
    }

    if (![other isKindOfClass:[DiscoveredNodeData class]]) {
        return NO;
    }

    return [self isEqualToDiscoveredNodeData:(DiscoveredNodeData *) other];
}

- (NSUInteger)hash
{
    const NSUInteger prime = 31;
    NSUInteger result = 1;

    result = prime * result + [self.instanceName hash];

    return result;
}

- (void)setConnectableVideoPlayer:(VideoPlayer * _Nonnull)videoPlayer
{
    _connectableVideoPlayer = videoPlayer;
}

- (bool)isPreCommissioned
{
    // Returning false will make the compat shim call the openBasicCommissioningWindow API every time.
    // The shim will internally handle selecting the commissioning path or simply re-establish
    // CASE (if the player nodeId/fabricIndex were found in the cache)
    return false;
}

- (VideoPlayer *)getConnectableVideoPlayer
{
    return _connectableVideoPlayer;
}

- (MCCastingPlayer *)getCastingPlayer
{
    return _castingPlayer;
}

@end
