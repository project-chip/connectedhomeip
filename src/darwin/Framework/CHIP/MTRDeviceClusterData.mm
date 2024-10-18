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

#import "MTRDeviceClusterData.h"
#import "MTRLogging_Internal.h"
#import "MTRUtilities.h"

static NSString * const sDataVersionKey = @"dataVersion";
static NSString * const sAttributesKey = @"attributes";

@implementation MTRDeviceClusterData {
    NSMutableDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _attributes;
}

- (void)storeValue:(MTRDeviceDataValueDictionary _Nullable)value forAttribute:(NSNumber *)attribute
{
    _attributes[attribute] = value;
}

- (void)removeValueForAttribute:(NSNumber *)attribute
{
    [_attributes removeObjectForKey:attribute];
}

- (NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> *)attributes
{
    return _attributes;
}

+ (BOOL)supportsSecureCoding
{
    return YES;
}

- (NSString *)description
{
    return [NSString stringWithFormat:@"<MTRDeviceClusterData: dataVersion %@ attributes count %lu>", _dataVersion, static_cast<unsigned long>(_attributes.count)];
}

- (nullable instancetype)init
{
    return [self initWithDataVersion:nil attributes:nil];
}

// Attributes dictionary is: attributeID => data-value dictionary
- (nullable instancetype)initWithDataVersion:(NSNumber * _Nullable)dataVersion attributes:(NSDictionary<NSNumber *, MTRDeviceDataValueDictionary> * _Nullable)attributes
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _dataVersion = [dataVersion copy];
    _attributes = [NSMutableDictionary dictionaryWithCapacity:attributes.count];
    [_attributes addEntriesFromDictionary:attributes];

    return self;
}

- (nullable instancetype)initWithCoder:(NSCoder *)decoder
{
    self = [super init];
    if (self == nil) {
        return nil;
    }

    _dataVersion = [decoder decodeObjectOfClass:[NSNumber class] forKey:sDataVersionKey];
    if (_dataVersion != nil && ![_dataVersion isKindOfClass:[NSNumber class]]) {
        MTR_LOG_ERROR("MTRDeviceClusterData got %@ for data version, not NSNumber.", _dataVersion);
        return nil;
    }

    static NSSet * const sAttributeValueClasses = [NSSet setWithObjects:[NSDictionary class], [NSArray class], [NSData class], [NSString class], [NSNumber class], nil];
    _attributes = [decoder decodeObjectOfClasses:sAttributeValueClasses forKey:sAttributesKey];
    if (_attributes != nil && ![_attributes isKindOfClass:[NSDictionary class]]) {
        MTR_LOG_ERROR("MTRDeviceClusterData got %@ for attributes, not NSDictionary.", _attributes);
        return nil;
    }

    return self;
}

- (void)encodeWithCoder:(NSCoder *)coder
{
    [coder encodeObject:self.dataVersion forKey:sDataVersionKey];
    [coder encodeObject:self.attributes forKey:sAttributesKey];
}

- (id)copyWithZone:(NSZone *)zone
{
    return [[MTRDeviceClusterData alloc] initWithDataVersion:_dataVersion attributes:_attributes];
}

- (BOOL)isEqualToClusterData:(MTRDeviceClusterData *)otherClusterData
{
    return MTREqualObjects(_dataVersion, otherClusterData.dataVersion)
        && MTREqualObjects(_attributes, otherClusterData.attributes);
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    return [self isEqualToClusterData:object];
}

@end
