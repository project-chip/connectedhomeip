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
#include <lib/dnssd/Resolver.h>

@implementation DiscoveredNodeData

- (DiscoveredNodeData *)initWithChipDiscoveredNodeData:(void *)chipDiscoveredNodedata
{
    self = [super init];
    if (self) {
        chip::Dnssd::DiscoveredNodeData * data = (chip::Dnssd::DiscoveredNodeData *) chipDiscoveredNodedata;

        // from CommissionNodeData
        _deviceType = data->commissionData.deviceType;
        _vendorId = data->commissionData.vendorId;
        _productId = data->commissionData.productId;
        _longDiscriminator = data->commissionData.longDiscriminator;
        _commissioningMode = data->commissionData.commissioningMode;
        _pairingHint = data->commissionData.pairingHint;
        _deviceName = [NSString stringWithCString:data->commissionData.deviceName encoding:NSASCIIStringEncoding];
        _rotatingIdLen = data->commissionData.rotatingIdLen;
        _rotatingId = data->commissionData.rotatingId;
        _instanceName = [NSString stringWithCString:data->commissionData.instanceName encoding:NSASCIIStringEncoding];

        // from CommonResolutionData
        _port = data->resolutionData.port;
        _hostName = [NSString stringWithCString:data->resolutionData.hostName encoding:NSASCIIStringEncoding];
        _platformInterface = data->resolutionData.interfaceId.GetPlatformInterface();
        _numIPs = data->resolutionData.numIPs;
        if (data->resolutionData.numIPs > 0) {
            _ipAddresses = [NSMutableArray new];
        }
        for (int i = 0; i < data->resolutionData.numIPs; i++) {
            char addrCString[chip::Inet::IPAddress::kMaxStringLength];
            data->resolutionData.ipAddress->ToString(addrCString, chip::Inet::IPAddress::kMaxStringLength);
            _ipAddresses[i] = [NSString stringWithCString:addrCString encoding:NSASCIIStringEncoding];
        }
    }
    return self;
}

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

- (NSString *)description
{
    return [NSString stringWithFormat:@"%@ with Product ID: %d and Vendor ID: %d", _deviceName, _productId, _vendorId];
}

@end
