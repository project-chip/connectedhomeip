/**
 *    Copyright (c) 2024 Project CHIP Authors
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

#import "MTRCommissioneeInfo_Internal.h"

#import "MTRDefines_Internal.h"
#import "MTREndpointInfo_Internal.h"
#import "MTRProductIdentity.h"
#import "MTRUtilities.h"

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@implementation MTRCommissioneeInfo

- (instancetype)initWithCommissioningInfo:(const chip::Controller::ReadCommissioningInfo &)info
{
    self = [super init];
    _productIdentity = [[MTRProductIdentity alloc] initWithVendorID:@(info.basic.vendorId) productID:@(info.basic.productId)];

    // TODO: We should probably hold onto our MTRCommissioningParameters so we can look at `readEndpointInformation`
    // instead of just reading whatever Descriptor cluster information happens to be in the cache.
    auto * endpoints = [MTREndpointInfo endpointsFromAttributeCache:info.attributes];
    if (endpoints.count > 0) {
        _endpointsById = endpoints;
    }

    return self;
}

static NSString * const sProductIdentityCodingKey = @"pi";
static NSString * const sEndpointsCodingKey = @"ep";

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    _productIdentity = [coder decodeObjectOfClass:MTRProductIdentity.class forKey:sProductIdentityCodingKey];
    VerifyOrReturnValue(_productIdentity != nil, nil);
    _endpointsById = [coder decodeDictionaryWithKeysOfClass:NSNumber.class
                                             objectsOfClass:MTREndpointInfo.class
                                                     forKey:sEndpointsCodingKey];
    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_productIdentity forKey:sProductIdentityCodingKey];
    [coder encodeObject:_endpointsById forKey:sEndpointsCodingKey];
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (NSUInteger)hash
{
    return _productIdentity.hash;
}

- (BOOL)isEqual:(id)object
{
    VerifyOrReturnValue([object class] == [self class], NO);
    MTRCommissioneeInfo * other = object;
    VerifyOrReturnValue(MTREqualObjects(_productIdentity, other->_productIdentity), NO);
    VerifyOrReturnValue(MTREqualObjects(_endpointsById, other->_endpointsById), NO);
    return YES;
}

- (id)copyWithZone:(nullable NSZone *)zone
{
    return self; // immutable
}

- (nullable MTREndpointInfo *)rootEndpoint
{
    return self.endpointsById[@0];
}

@end

NS_ASSUME_NONNULL_END
