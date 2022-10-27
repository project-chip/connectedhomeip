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
