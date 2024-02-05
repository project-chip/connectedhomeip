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
#import "MTRDefines_Internal.h"
#import "MTRDeviceController_Internal.h"
#import "MTRLogging_Internal.h"
#import "MTRServerAttribute_Internal.h"
#import "MTRServerEndpoint_Internal.h"
#import "NSDataSpanConversion.h"
#import <Matter/MTRServerAttribute.h>

#include <app/reporting/reporting.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRServerAttribute {
    MTRDeviceController * __weak _deviceController;
}

- (nullable instancetype)initAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value requiredReadPrivilege:(MTRAccessControlEntryPrivilege)requiredReadPrivilege writable:(BOOL)writable
{
    auto attrIDValue = attributeID.unsignedLongLongValue;
    if (!CanCastTo<AttributeId>(attrIDValue)) {
        MTR_LOG_ERROR("MTRServerAttribute provided too-large attribute ID: 0x%llx", attrIDValue);
        return nil;
    }

    auto attrId = static_cast<AttributeId>(attrIDValue);
    if (!IsValidAttributeId(attrId)) {
        MTR_LOG_ERROR("MTRServerAttribute provided invalid attribute ID: 0x%" PRIx32, attrId);
        return nil;
    }

    return [self initWithAttributeID:[attributeID copy] value:[value copy] requiredReadPrivilege:requiredReadPrivilege writable:writable];
}

- (nullable instancetype)initReadonlyAttributeWithID:(NSNumber *)attributeID initialValue:(NSDictionary<NSString *, id> *)value requiredPrivilege:(MTRAccessControlEntryPrivilege)requiredPrivilege
{
    return [self initAttributeWithID:attributeID initialValue:value requiredReadPrivilege:requiredPrivilege writable:NO];
}

// initWithAttributeID:value:serializedValue:requiredReadPrivilege:writable:
// assumes that the attribute ID, value, serializedValue, have already been
// validated and, if needed, copied from the input.
- (nullable instancetype)initWithAttributeID:(NSNumber *)attributeID value:(NSDictionary<NSString *, id> *)value requiredReadPrivilege:(MTRAccessControlEntryPrivilege)requiredReadPrivilege writable:(BOOL)writable
{
    if (!(self = [super init])) {
        return nil;
    }

    _attributeID = attributeID;
    _requiredReadPrivilege = requiredReadPrivilege;
    _writable = writable;
    _parentCluster = app::ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId);

    // Now call setValue to store the value and its serialization.
    if ([self setValue:value] == NO) {
        return nil;
    }

    return self;
}

- (BOOL)setValue:(NSDictionary<NSString *, id> *)value
{
    id serializedValue;
    id dataType = value[MTRTypeKey];
    if ([MTRArrayValueType isEqual:dataType]) {
        id dataValue = value[MTRValueKey];
        if (![dataValue isKindOfClass:NSArray.class]) {
            MTR_LOG_ERROR("MTRServerAttribute value claims to be a list but isn't: %@", value);
            return NO;
        }
        NSArray * dataValueList = dataValue;
        auto * listValue = [NSMutableArray arrayWithCapacity:dataValueList.count];
        if (listValue == nil) {
            return NO;
        }
        for (id item in dataValueList) {
            if (![item isKindOfClass:NSDictionary.class]) {
                MTR_LOG_ERROR("MTRServerAttribute value array should contain dictionaries");
            }
            NSDictionary<NSString *, id> * itemDictionary = item;

            NSError * encodingError;
            NSData * encodedItem = MTREncodeTLVFromDataValueDictionary(itemDictionary[MTRDataKey], &encodingError);
            if (encodedItem == nil) {
                return NO;
            }
            [listValue addObject:encodedItem];
        }
        serializedValue = listValue;
    } else {
        NSError * encodingError;
        serializedValue = MTREncodeTLVFromDataValueDictionary(value, &encodingError);
        if (serializedValue == nil) {
            return NO;
        }
    }

    // We serialized properly, so should be good to go on the value.
    _value = [value copy];

    MTRDeviceController * deviceController = _deviceController;
    if (deviceController == nil) {
        // We're not bound to a controller, so safe to directly update _serializedValue.
        _serializedValue = serializedValue;
    } else {
        [deviceController asyncDispatchToMatterQueue:^{
            auto changed = ![self->_serializedValue isEqual:serializedValue];
            self->_serializedValue = serializedValue;
            if (changed) {
                MatterReportingAttributeChangeCallback(self->_parentCluster.mEndpointId, self->_parentCluster.mClusterId, static_cast<AttributeId>(self->_attributeID.unsignedLongLongValue));
            }
        }
                                        errorHandler:nil];
    }

    return YES;
}

- (BOOL)associateWithController:(nullable MTRDeviceController *)controller
{
    MTRDeviceController * existingController = _deviceController;
    if (existingController != nil) {
#if MTR_PER_CONTROLLER_STORAGE_ENABLED
        MTR_LOG_ERROR("Cannot associate MTRServerAttribute with controller %@; already associated with controller %@",
            controller.uniqueIdentifier, existingController.uniqueIdentifier);
#else
        MTR_LOG_ERROR("Cannot associate MTRServerAttribute with controller; already associated with a different controller");
#endif
        return NO;
    }

    _deviceController = controller;

    return YES;
}

- (void)invalidate
{
    _deviceController = nil;
}

@end
