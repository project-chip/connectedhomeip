/**
 *
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

#import "MCAttributeObjects.h"

#import "MCAttribute_Internal.h"
#import "MCCastingApp.h"
#import "MCErrorUtils.h"

#include "core/Attribute.h"
#include <app-common/zap-generated/cluster-objects.h>

#import <Foundation/Foundation.h>

@implementation MCApplicationBasicClusterVendorIDAttribute
- (void)read:(void * _Nullable)context
    completion:(void (^_Nonnull __strong)(void * _Nullable, id _Nullable __strong before, id _Nullable __strong after, NSError * _Nullable __strong error))completion
{
    MCAttributeTemplate<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo> * mcAttribute = new MCAttributeTemplate<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo>(self.cppAttribute,
        [self](std::any cppValue) {
            return [self getObjTypeFromCpp:cppValue];
        });
    mcAttribute->read(context, [mcAttribute, completion](void * context, id before, id after, NSError * err) {
        completion(context, before, after, err);
        delete mcAttribute;
    });
}

- (void)subscribe:(void * _Nullable)context
       completion:(void (^_Nonnull __strong)(void * _Nullable, id _Nullable __strong before, id _Nullable __strong after, NSError * _Nullable __strong error))completion
      minInterval:(NSNumber * _Nonnull)minInterval
      maxInterval:(NSNumber * _Nonnull)maxInterval
{
    MCAttributeTemplate<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo> * mcAttribute = new MCAttributeTemplate<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo>(self.cppAttribute,
        [self](std::any cppValue) {
            return [self getObjTypeFromCpp:cppValue];
        });
    mcAttribute->subscribe(
        context, [mcAttribute, completion](void * context, id before, id after, NSError * err) {
            completion(context, before, after, err);
            delete mcAttribute;
        }, minInterval, maxInterval);
}

- (id _Nullable)getObjTypeFromCpp:(std::any)cppValue
{
    NSNumber * outValue = nil;
    if (cppValue.type() == typeid(std::shared_ptr<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>)) {
        std::shared_ptr<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType> valueSharedPtr = std::any_cast<std::shared_ptr<chip::app::Clusters::ApplicationBasic::Attributes::VendorID::TypeInfo::DecodableArgType>>(cppValue);
        outValue = valueSharedPtr != nil ? [NSNumber numberWithUnsignedInteger:*valueSharedPtr] : nil;
    }
    return outValue;
}
@end

@implementation MCMediaPlaybackClusterCurrentStateAttribute
- (void)read:(void * _Nullable)context
    completion:(void (^_Nonnull __strong)(void * _Nullable, id _Nullable __strong before, id _Nullable __strong after, NSError * _Nullable __strong error))completion
{
    MCAttributeTemplate<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo> * mcAttribute = new MCAttributeTemplate<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo>(self.cppAttribute,
        [self](std::any cppValue) {
            return [self getObjTypeFromCpp:cppValue];
        });
    mcAttribute->read(context, [mcAttribute, completion](void * context, id before, id after, NSError * err) {
        completion(context, before, after, err);
        delete mcAttribute;
    });
}

- (void)subscribe:(void * _Nullable)context
       completion:(void (^_Nonnull __strong)(void * _Nullable, id _Nullable __strong before, id _Nullable __strong after, NSError * _Nullable __strong error))completion
      minInterval:(NSNumber * _Nonnull)minInterval
      maxInterval:(NSNumber * _Nonnull)maxInterval
{
    MCAttributeTemplate<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo> * mcAttribute = new MCAttributeTemplate<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo>(self.cppAttribute,
        [self](std::any cppValue) {
            return [self getObjTypeFromCpp:cppValue];
        });
    mcAttribute->subscribe(
        context, [mcAttribute, completion](void * context, id before, id after, NSError * err) {
            completion(context, before, after, err);
            delete mcAttribute;
        }, minInterval, maxInterval);
}

- (id _Nullable)getObjTypeFromCpp:(std::any)cppValue
{
    NSNumber * outValue = nil;
    if (cppValue.type() == typeid(std::shared_ptr<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType>)) {
        std::shared_ptr<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType> valueSharedPtr = std::any_cast<std::shared_ptr<chip::app::Clusters::MediaPlayback::Attributes::CurrentState::TypeInfo::DecodableArgType>>(cppValue);
        outValue = valueSharedPtr != nil ? [NSNumber numberWithUnsignedInteger:static_cast<uint8_t>(*valueSharedPtr)] : nil;
    }
    return outValue;
}
@end
