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

#import "MTRBaseDevice_Internal.h"
#import "MTRError_Internal.h"
#import "MTRLogging_Internal.h"
#import "NSDataSpanConversion.h"
#import <Matter/MTRAttributeDescription.h>

#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

@interface MTRAttributeDescription ()
// When initialized, exactly one of singleValue and listValue will be non-nil.
@property (nonatomic, strong, readonly, nullable) NSData * singleValue;
@property (nonatomic, strong, readonly, nullable) NSArray<NSData *> * listValue;
@end

@implementation MTRAttributeDescription

@dynamic value;

- (nullable instancetype)initAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value writable:(BOOL)writable error:(NSError * __autoreleasing *)error
{
    auto attrIDValue = attributeID.unsignedLongLongValue;
    if (!CanCastTo<AttributeId>(attrIDValue)) {
        MTR_LOG_ERROR("MTRAttributeDescription provided too-large attribute ID: 0x%llx", attrIDValue);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    auto attrId = static_cast<AttributeId>(attrIDValue);
    if (!IsValidAttributeId(attrId)) {
        MTR_LOG_ERROR("MTRAttributeDescription provided invalid attribute ID: 0x%" PRIx32, attrId);
        if (error) {
            *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
        }
        return nil;
    }

    NSData * singleValue;
    NSMutableArray<NSData *> * listValue;

    id dataType = value[MTRTypeKey];
    if ([MTRArrayValueType isEqual:dataType]) {
        singleValue = nil;
        id dataValue = value[MTRValueKey];
        if (![dataValue isKindOfClass:NSArray.class]) {
            MTR_LOG_ERROR("MTRAttributeDescription value claims to be a list but isn't: %@", value);
            if (error) {
                *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_INVALID_ARGUMENT];
            }
            return nil;
        }
        NSArray * dataValueList = dataValue;
        listValue = [NSMutableArray arrayWithCapacity:dataValueList.count];
        if (listValue == nil) {
            if (error) {
                *error = [MTRError errorForCHIPErrorCode:CHIP_ERROR_NO_MEMORY];
            }
            return nil;
        }
        for (id item in dataValueList) {
            NSData * encodedItem = MTREncodeTLVFromDataValueDictionary(item, error);
            if (encodedItem == nil) {
                return nil;
            }
            [listValue addObject:encodedItem];
        }
    } else {
        listValue = nil;
        singleValue = MTREncodeTLVFromDataValueDictionary(value, error);
        if (singleValue == nil) {
            return nil;
        }
    }

    return [self initWithAttributeID:[attributeID copy] singleValue:singleValue listValue:listValue writable:writable];
}

- (nullable instancetype)initReadonlyAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value error:(NSError * __autoreleasing *)error
{
    return [self initAttributeWithID:attributeID initialValue:value writable:NO error:error];
}

- (nullable instancetype)initWritableAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value error:(NSError * __autoreleasing *)error
{
    return [self initAttributeWithID:attributeID initialValue:value writable:YES error:error];
}

+ (NSDictionary<NSString *, id> *)singleValueToDataValue:(NSData *)singleValue
{
    ByteSpan dataSpan = AsByteSpan(singleValue);
    TLV::TLVReader reader;
    reader.Init(dataSpan);

    // This is not going to return an error, since we encoded the data
    // ourselves, so we know it's valid.
    CHIP_ERROR err = reader.Next();
    if (err != CHIP_NO_ERROR) {
        MTR_LOG_ERROR("Unexpected error trying to read TLV we wrote");
    }

    // This is not going to return nil, since we encoded the data ourselves, so
    // we know it's valid.
    auto decodedValue = MTRDecodeDataValueDictionaryFromCHIPTLV(&reader);
    if (decodedValue == nil) {
        MTR_LOG_ERROR("Unexpected error trying to read TLV we wrote");
    }
    return decodedValue;
}

- (NSDictionary<NSString *, id> *)value
{
    if (_singleValue) {
        return [MTRAttributeDescription singleValueToDataValue:_singleValue];
    }

    auto listValueToReturn = [NSMutableArray arrayWithCapacity:_listValue.count];
    for (NSData * item in _listValue) {
        [listValueToReturn addObject:[MTRAttributeDescription singleValueToDataValue:item]];
    }

    return @{
        MTRTypeKey : MTRArrayValueType,
        MTRValueKey : [listValueToReturn copy],
    };
}

// initWithAttributeID:value:writable: assumes that the attribute ID and singleValue/listValue have already been
// validated and, if needed, copied from the input.
- (instancetype)initWithAttributeID:(NSNumber *)attributeID singleValue:(nullable NSData *)singleValue listValue:(nullable NSArray<NSData *> *)listValue writable:(BOOL)writable
{
    if (!(self = [super init])) {
        return nil;
    }

    _attributeID = attributeID;
    _singleValue = singleValue;
    _listValue = listValue;
    _writable = writable;
    return self;
}

- (id)copyWithZone:(NSZone *)zone
{
    NSArray<NSData *> * listValueCopy = nil;
    if (_listValue) {
        listValueCopy = [[NSArray alloc] initWithArray:_listValue copyItems:YES];
    }
    return [[MTRAttributeDescription alloc] initWithAttributeID:[_attributeID copy] singleValue:[_singleValue copy] listValue:listValueCopy writable:_writable];
}

- (BOOL)isEqual:(id)object
{
    if ([object class] != [self class]) {
        return NO;
    }

    MTRAttributeDescription * other = object;

    BOOL valueEqual = NO;
    if (_singleValue != nil) {
        valueEqual = [_singleValue isEqual:other.singleValue];
    } else {
        valueEqual = [_listValue isEqual:other.listValue];
    }

    return [_attributeID isEqual:other.attributeID] && valueEqual && _writable == other.writable;
}

- (NSUInteger)hash
{
    return _attributeID.unsignedLongValue ^ [_singleValue hash] ^ [_listValue hash] ^ _writable;
}

@end
