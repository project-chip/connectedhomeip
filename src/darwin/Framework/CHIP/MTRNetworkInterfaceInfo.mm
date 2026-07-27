/**
 *    Copyright (c) 2025 Project CHIP Authors
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

#import "MTRNetworkInterfaceInfo_Internal.h"

#import <Matter/MTRBaseClusters.h>

#import "MTRDefines_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRUtilities.h"

#include <lib/support/CodeUtils.h>

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@implementation MTRNetworkInterfaceInfo

- (nullable instancetype)initWithEndpointID:(NSNumber *)endpointID featureMap:(NSNumber *)featureMap
{
    constexpr MTRNetworkCommissioningFeature typeBits = (MTRNetworkCommissioningFeatureWiFiNetworkInterface | MTRNetworkCommissioningFeatureThreadNetworkInterface | MTRNetworkCommissioningFeatureEthernetNetworkInterface);
    auto ourTypeBits = featureMap.unsignedLongLongValue & typeBits;

    if (ourTypeBits == 0) {
        MTR_LOG_ERROR("MTRNetworkInterfaceInfo for endpoint %@ has no network technology bits set", endpointID);
        return nil;
    }

    if ((ourTypeBits & (ourTypeBits - 1)) != 0) {
        MTR_LOG_ERROR("MTRNetworkInterfaceInfo for endpoint %@ has multiple network technology bits set: 0x%llx", endpointID, ourTypeBits);
        return nil;
    }

    if (!(self = [super init])) {
        return nil;
    }

    _endpointID = [endpointID copy];
    _featureMap = [featureMap copy];
    _type = static_cast<MTRNetworkCommissioningFeature>(ourTypeBits);

    return self;
}

#pragma mark - NSCopying implementation

- (id)copyWithZone:(nullable NSZone *)zone
{
    return self; // immutable
}

#pragma mark - NSSecureCoding implementation

+ (BOOL)supportsSecureCoding
{
    return YES;
}

static NSString * const sEndpointIDCodingKey = @"endpointID";
static NSString * const sFeatureMapCodingKey = @"featureMap";

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.endpointID forKey:sEndpointIDCodingKey];
    [coder encodeObject:self.featureMap forKey:sFeatureMapCodingKey];
}

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    NSNumber * endpointID = [coder decodeObjectOfClass:NSNumber.class forKey:sEndpointIDCodingKey];
    NSNumber * featureMap = [coder decodeObjectOfClass:NSNumber.class forKey:sFeatureMapCodingKey];

    if (endpointID == nil || featureMap == nil) {
        return nil;
    }

    return [self initWithEndpointID:endpointID featureMap:featureMap];
}

#pragma mark - NSObject implementation

- (NSUInteger)hash
{
    return self.endpointID.hash ^ self.featureMap.hash;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);

    MTRNetworkInterfaceInfo * other = object;

    VerifyOrReturnValue(MTREqualObjects(_endpointID, other->_endpointID), NO);
    VerifyOrReturnValue(MTREqualObjects(_featureMap, other->_featureMap), NO);

    return YES;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRNetworkInterfaceInfo: endpointID=%@, featureMap=0x%llx>",
                     self.endpointID, self.featureMap.unsignedLongLongValue];
}

@end

NS_ASSUME_NONNULL_END
