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

#import <Matter/MTRClusterConstants.h>

#import "MTRBaseDevice.h"
#import "MTRBaseDevice_Internal.h"
#import "MTRDefines_Internal.h"
#import "MTRDeviceDataValidation.h"
#import "MTREndpointInfo_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRProductIdentity.h"
#import "MTRUtilities.h"

#include <app/AttributePathParams.h>
#include <lib/core/TLVReader.h>

NS_ASSUME_NONNULL_BEGIN

MTR_DIRECT_MEMBERS
@implementation MTRCommissioneeInfo

- (instancetype)initWithCommissioningInfo:(const chip::Controller::ReadCommissioningInfo &)info commissioningParameters:(MTRCommissioningParameters *)commissioningParameters
{
    self = [super init];
    _productIdentity = [[MTRProductIdentity alloc] initWithVendorID:@(info.basic.vendorId) productID:@(info.basic.productId)];

    if (commissioningParameters.readEndpointInformation) {
        auto * endpoints = [MTREndpointInfo endpointsFromAttributeCache:info.attributes];
        if (endpoints.count > 0) {
            _endpointsById = endpoints;
        }
    }

    if (info.attributes != nullptr) {
        NSMutableDictionary<MTRAttributePath *, NSDictionary<NSString *, id> *> * attributes = [[NSMutableDictionary alloc] init];

        // Only expose attributes that match pathFilters, so that API consumers
        // don't start relying on undocumented internal details of which paths
        // we read from the device in which circumstances.
        std::vector<chip::app::AttributePathParams> pathFilters;
        if (commissioningParameters.extraAttributesToRead != nil) {
            for (MTRAttributeRequestPath * requestPath in commissioningParameters.extraAttributesToRead) {
                [requestPath convertToAttributePathParams:pathFilters.emplace_back()];
            }
        }

        // Always include the Network Commissioning cluster FeatureMap
        // attributes, using a wildcard-endpoint path.
        pathFilters.emplace_back(MTRClusterIDTypeNetworkCommissioningID, MTRAttributeIDTypeGlobalAttributeFeatureMapID);

        info.attributes->ForEachAttribute([&](const chip::app::ConcreteAttributePath & path) -> CHIP_ERROR {
            // Only grab paths that are included in extraAttributesToRead so that
            // API consumers don't develop dependencies on implementation details
            // (like which other attributes we happen to read).

            // TODO: This means API consumers might duplicate attribute reads we
            // already do.  We should either offer guarantees about things like
            // "network commissioning feature maps" that will always be present, or
            // perhaps dedup in some way under the hood when issuing the
            // reads.

            // This is unfortunately not very efficient; if we have a lot of
            // paths we may need a better way to do this.
            bool isRequestedPath = false;
            for (auto & filter : pathFilters) {
                if (!filter.IsAttributePathSupersetOf(path)) {
                    continue;
                }

                isRequestedPath = true;
                break;
            }

            if (!isRequestedPath) {
                // Skip it.
                return CHIP_NO_ERROR;
            }

            chip::TLV::TLVReader reader;
            CHIP_ERROR err = info.attributes->Get(path, reader);
            if (err != CHIP_NO_ERROR) {
                // We actually got an error, not data.  Just skip this path.
                return CHIP_NO_ERROR;
            }

            auto value = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
            if (value == nil) {
                // Decode errors can happen (e.g. invalid TLV); just skip this path.
                return CHIP_NO_ERROR;
            }

            auto * mtrPath = [[MTRAttributePath alloc] initWithPath:path];
            attributes[mtrPath] = value;
            return CHIP_NO_ERROR;
        });

        _attributes = attributes;
    }

    return self;
}

static NSString * const sProductIdentityCodingKey = @"pi";
static NSString * const sEndpointsCodingKey = @"ep";
static NSString * const sAttributesCodingKey = @"at";

- (nullable instancetype)initWithCoder:(NSCoder *)coder
{
    self = [super init];
    _productIdentity = [coder decodeObjectOfClass:MTRProductIdentity.class forKey:sProductIdentityCodingKey];
    VerifyOrReturnValue(_productIdentity != nil, nil);
    _endpointsById = [coder decodeDictionaryWithKeysOfClass:NSNumber.class
                                             objectsOfClass:MTREndpointInfo.class
                                                     forKey:sEndpointsCodingKey];

    // TODO: Can we do better about duplicating the set of classes that appear
    // in data-values?  We have this set in a bunch of places....  But here we need
    // not just those, but also MTRAttributePath.
    static NSSet * const sAttributeClasses = [NSSet setWithObjects:NSDictionary.class, NSArray.class, NSData.class, NSString.class, NSNumber.class, MTRAttributePath.class, nil];
    _attributes = [coder decodeObjectOfClasses:sAttributeClasses forKey:sAttributesCodingKey];

    if (_attributes != nil) {
        // Check that the right types are in the right places.
        if (![_attributes isKindOfClass:NSDictionary.class]) {
            MTR_LOG_ERROR("MTRCommissioneeInfo decoding: attributes are not a dictionary: %@", _attributes);
            return nil;
        }

        for (id key in _attributes) {
            if (![key isKindOfClass:MTRAttributePath.class]) {
                MTR_LOG_ERROR("MTRCommissioneeInfo decoding: expected MTRAttributePath but found %@", key);
                return nil;
            }

            id value = _attributes[key];
            if (![value isKindOfClass:NSDictionary.class] || !MTRDataValueDictionaryIsWellFormed(value)) {
                MTR_LOG_ERROR("MTRCommissioneeInfo decoding: expected data-value dictionary but found %@", value);
                return nil;
            }
        }
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:_productIdentity forKey:sProductIdentityCodingKey];
    [coder encodeObject:_endpointsById forKey:sEndpointsCodingKey];
    [coder encodeObject:_attributes forKey:sAttributesCodingKey];
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
    VerifyOrReturnValue(MTREqualObjects(_attributes, other->_attributes), NO);

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
