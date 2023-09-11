/**
 *
 * SPDX-FileCopyrightText: 2020-2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import <Foundation/Foundation.h>

#import "VideoPlayer.h"

@implementation VideoPlayer

- (instancetype)init
{
    if (self = [super init]) {
        _isInitialized = false;
    }
    return self;
}

- (instancetype)initWithNodeId:(uint64_t)nodeId
                   fabricIndex:(uint8_t)fabricIndex
                   isConnected:(bool)isConnected
                   contentApps:(NSMutableArray *)contentApps
                    deviceName:(NSString *)deviceName
                      vendorId:(uint16_t)vendorId
                     productId:(uint16_t)productId
                    deviceType:(uint16_t)deviceType;
{
    if (self = [super init]) {
        _nodeId = nodeId;
        _fabricIndex = fabricIndex;
        _isConnected = isConnected;
        _contentApps = contentApps;
        _deviceName = deviceName;
        _vendorId = vendorId;
        _productId = productId;
        _deviceType = deviceType;
        _isInitialized = true;
    }
    return self;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ with Product ID: %d, Vendor ID: %d", _deviceName, _productId, _vendorId];
}
@end
