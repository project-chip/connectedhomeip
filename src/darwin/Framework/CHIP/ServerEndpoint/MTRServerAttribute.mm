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
#import "MTRUnfairLock.h"
#import "NSDataSpanConversion.h"

#import <Matter/MTRClusterConstants.h>
#import <Matter/MTRServerAttribute.h>

#include <app/reporting/reporting.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/SafeInt.h>
#import <os/lock.h>

using namespace chip;

MTR_DIRECT_MEMBERS
@implementation MTRServerAttribute {
    // _lock always protects access to _deviceController, _value,
    // _serializedValue, and _parentCluster.
    os_unfair_lock _lock;
    MTRDeviceController * __weak _deviceController;
    NSDictionary<NSString *, id> * _value;
    app::ConcreteClusterPath _parentCluster;
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

    if (attrId == MTRAttributeIDTypeGlobalAttributeFeatureMapID) {
        // Some sanity checks: value should be an unsigned-integer NSNumber, and
        // requiredReadPrivilege should be View.
        if (requiredReadPrivilege != MTRAccessControlEntryPrivilegeView) {
            MTR_LOG_ERROR("MTRServerAttribute for FeatureMap provided with invalid read privilege %d", requiredReadPrivilege);
            return nil;
        }

        if (![MTRUnsignedIntegerValueType isEqual:value[MTRTypeKey]]) {
            MTR_LOG_ERROR("MTRServerAttribute for FeatureMap provided with value that is not an unsigned integer: %@", value);
            return nil;
        }
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

    _lock = OS_UNFAIR_LOCK_INIT;
    _attributeID = attributeID;
    _requiredReadPrivilege = requiredReadPrivilege;
    _writable = writable;
    _parentCluster = app::ConcreteClusterPath(kInvalidEndpointId, kInvalidClusterId);

    // Now store the value and its serialization. This will also check that the
    // value is serializable to TLV.
    if ([self setValueInternal:value logIfNotAssociated:NO] == NO) {
        return nil;
    }

    return self;
}

- (BOOL)setValue:(NSDictionary<NSString *, id> *)value
{
    return [self setValueInternal:value logIfNotAssociated:YES];
}

+ (MTRServerAttribute *)newFeatureMapAttributeWithInitialValue:(NSNumber *)value
{
    return [[MTRServerAttribute alloc] initReadonlyAttributeWithID:@(MTRAttributeIDTypeGlobalAttributeFeatureMapID)
                                                      initialValue:@{
                                                          MTRTypeKey : MTRUnsignedIntegerValueType,
                                                          MTRValueKey : value,
                                                      }
                                                 requiredPrivilege:MTRAccessControlEntryPrivilegeView];
}

- (BOOL)setValueInternal:(NSDictionary<NSString *, id> *)value logIfNotAssociated:(BOOL)logIfNotAssociated
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

    // We serialized properly, so should be good to go on the value.  Lock
    // around our ivar accesses.
    std::lock_guard lock(_lock);

    _value = [value copy];

    MTR_LOG("Attribute value updated: %@", [self _descriptionWhileLocked]); // Logs new value as part of our description.

    MTRDeviceController * deviceController = _deviceController;
    if (deviceController == nil) {
        // We're not bound to a controller, so safe to directly update
        // _serializedValue.
        if (logIfNotAssociated) {
            MTR_LOG("Not publishing value for attribute " ChipLogFormatMEI "; not bound to a controller",
                ChipLogValueMEI(static_cast<AttributeId>(_attributeID.unsignedLongLongValue)));
        }
        _serializedValue = serializedValue;
    } else {
        [deviceController asyncDispatchToMatterQueue:^{
            std::lock_guard lock(self->_lock);
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

- (NSDictionary<NSString *, id> *)value
{
    std::lock_guard lock(_lock);
    return [_value copy];
}

- (BOOL)associateWithController:(nullable MTRDeviceController *)controller
{
    std::lock_guard lock(_lock);

    MTRDeviceController * existingController = _deviceController;
    if (existingController != nil) {
        MTR_LOG_ERROR("Cannot associate MTRServerAttribute with controller %@; already associated with controller %@",
            controller.uniqueIdentifier, existingController.uniqueIdentifier);
        return NO;
    }

    _deviceController = controller;

    MTR_LOG("Associated %@ with controller", [self _descriptionWhileLocked]);

    return YES;
}

- (void)invalidate
{
    std::lock_guard lock(_lock);

    _deviceController = nil;
}

- (BOOL)addToCluster:(const app::ConcreteClusterPath &)cluster
{
    std::lock_guard lock(_lock);

    if (_parentCluster.mClusterId != kInvalidClusterId) {
        MTR_LOG_ERROR("Cannot add attribute to cluster " ChipLogFormatMEI "; already added to cluster " ChipLogFormatMEI, ChipLogValueMEI(cluster.mClusterId), ChipLogValueMEI(_parentCluster.mClusterId));
        return NO;
    }

    _parentCluster = cluster;
    return YES;
}

- (void)updateParentCluster:(const app::ConcreteClusterPath &)cluster
{
    std::lock_guard lock(_lock);
    _parentCluster = cluster;
}

- (const app::ConcreteClusterPath &)parentCluster
{
    std::lock_guard lock(_lock);
    return _parentCluster;
}

- (NSString *)description
{
    std::lock_guard lock(_lock);
    return [self _descriptionWhileLocked];
}

- (NSString *)_descriptionWhileLocked
{
    os_unfair_lock_assert_owner(&_lock);
    return [NSString stringWithFormat:@"<MTRServerAttribute endpoint %u, cluster " ChipLogFormatMEI ", id " ChipLogFormatMEI ", value '%@'>", static_cast<EndpointId>(_parentCluster.mEndpointId), ChipLogValueMEI(_parentCluster.mClusterId), ChipLogValueMEI(_attributeID.unsignedLongLongValue), _value];
}

@end
