/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#import "MTRAttributeTLVValueDecoder_Internal.h"

#import "MTRStructsObjc.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app;

id MTRDecodeAttributeValue(const ConcreteAttributePath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    switch (aPath.mClusterId) {
    case Clusters::Identify::Id: {
        using namespace Clusters::Identify;
        switch (aPath.mAttributeId) {
        case Attributes::IdentifyTime::Id: {
            using TypeInfo = Attributes::IdentifyTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::IdentifyType::Id: {
            using TypeInfo = Attributes::IdentifyType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Groups::Id: {
        using namespace Clusters::Groups;
        switch (aPath.mAttributeId) {
        case Attributes::NameSupport::Id: {
            using TypeInfo = Attributes::NameSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Scenes::Id: {
        using namespace Clusters::Scenes;
        switch (aPath.mAttributeId) {
        case Attributes::SceneCount::Id: {
            using TypeInfo = Attributes::SceneCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CurrentScene::Id: {
            using TypeInfo = Attributes::CurrentScene::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CurrentGroup::Id: {
            using TypeInfo = Attributes::CurrentGroup::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::SceneValid::Id: {
            using TypeInfo = Attributes::SceneValid::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::NameSupport::Id: {
            using TypeInfo = Attributes::NameSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::LastConfiguredBy::Id: {
            using TypeInfo = Attributes::LastConfiguredBy::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OnOff::Id: {
        using namespace Clusters::OnOff;
        switch (aPath.mAttributeId) {
        case Attributes::OnOff::Id: {
            using TypeInfo = Attributes::OnOff::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::GlobalSceneControl::Id: {
            using TypeInfo = Attributes::GlobalSceneControl::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::OnTime::Id: {
            using TypeInfo = Attributes::OnTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::OffWaitTime::Id: {
            using TypeInfo = Attributes::OffWaitTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::StartUpOnOff::Id: {
            using TypeInfo = Attributes::StartUpOnOff::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OnOffSwitchConfiguration::Id: {
        using namespace Clusters::OnOffSwitchConfiguration;
        switch (aPath.mAttributeId) {
        case Attributes::SwitchType::Id: {
            using TypeInfo = Attributes::SwitchType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::SwitchActions::Id: {
            using TypeInfo = Attributes::SwitchActions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::LevelControl::Id: {
        using namespace Clusters::LevelControl;
        switch (aPath.mAttributeId) {
        case Attributes::CurrentLevel::Id: {
            using TypeInfo = Attributes::CurrentLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::RemainingTime::Id: {
            using TypeInfo = Attributes::RemainingTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::MinLevel::Id: {
            using TypeInfo = Attributes::MinLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MaxLevel::Id: {
            using TypeInfo = Attributes::MaxLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CurrentFrequency::Id: {
            using TypeInfo = Attributes::CurrentFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::MinFrequency::Id: {
            using TypeInfo = Attributes::MinFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::MaxFrequency::Id: {
            using TypeInfo = Attributes::MaxFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Options::Id: {
            using TypeInfo = Attributes::Options::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::OnOffTransitionTime::Id: {
            using TypeInfo = Attributes::OnOffTransitionTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::OnLevel::Id: {
            using TypeInfo = Attributes::OnLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OnTransitionTime::Id: {
            using TypeInfo = Attributes::OnTransitionTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OffTransitionTime::Id: {
            using TypeInfo = Attributes::OffTransitionTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::DefaultMoveRate::Id: {
            using TypeInfo = Attributes::DefaultMoveRate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::StartUpCurrentLevel::Id: {
            using TypeInfo = Attributes::StartUpCurrentLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BinaryInputBasic::Id: {
        using namespace Clusters::BinaryInputBasic;
        switch (aPath.mAttributeId) {
        case Attributes::ActiveText::Id: {
            using TypeInfo = Attributes::ActiveText::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::Description::Id: {
            using TypeInfo = Attributes::Description::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::InactiveText::Id: {
            using TypeInfo = Attributes::InactiveText::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::OutOfService::Id: {
            using TypeInfo = Attributes::OutOfService::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::Polarity::Id: {
            using TypeInfo = Attributes::Polarity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::PresentValue::Id: {
            using TypeInfo = Attributes::PresentValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::Reliability::Id: {
            using TypeInfo = Attributes::Reliability::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::StatusFlags::Id: {
            using TypeInfo = Attributes::StatusFlags::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ApplicationType::Id: {
            using TypeInfo = Attributes::ApplicationType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Descriptor::Id: {
        using namespace Clusters::Descriptor;
        switch (aPath.mAttributeId) {
        case Attributes::DeviceTypeList::Id: {
            using TypeInfo = Attributes::DeviceTypeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRDescriptorClusterDeviceTypeStruct * newElement_0;
                    newElement_0 = [MTRDescriptorClusterDeviceTypeStruct new];
                    newElement_0.type = [NSNumber numberWithUnsignedInt:entry_0.type];
                    newElement_0.revision = [NSNumber numberWithUnsignedShort:entry_0.revision];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ServerList::Id: {
            using TypeInfo = Attributes::ServerList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ClientList::Id: {
            using TypeInfo = Attributes::ClientList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::PartsList::Id: {
            using TypeInfo = Attributes::PartsList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Binding::Id: {
        using namespace Clusters::Binding;
        switch (aPath.mAttributeId) {
        case Attributes::Binding::Id: {
            using TypeInfo = Attributes::Binding::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRBindingClusterTargetStruct * newElement_0;
                    newElement_0 = [MTRBindingClusterTargetStruct new];
                    if (entry_0.node.HasValue()) {
                        newElement_0.node = [NSNumber numberWithUnsignedLongLong:entry_0.node.Value()];
                    } else {
                        newElement_0.node = nil;
                    }
                    if (entry_0.group.HasValue()) {
                        newElement_0.group = [NSNumber numberWithUnsignedShort:entry_0.group.Value()];
                    } else {
                        newElement_0.group = nil;
                    }
                    if (entry_0.endpoint.HasValue()) {
                        newElement_0.endpoint = [NSNumber numberWithUnsignedShort:entry_0.endpoint.Value()];
                    } else {
                        newElement_0.endpoint = nil;
                    }
                    if (entry_0.cluster.HasValue()) {
                        newElement_0.cluster = [NSNumber numberWithUnsignedInt:entry_0.cluster.Value()];
                    } else {
                        newElement_0.cluster = nil;
                    }
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AccessControl::Id: {
        using namespace Clusters::AccessControl;
        switch (aPath.mAttributeId) {
        case Attributes::Acl::Id: {
            using TypeInfo = Attributes::Acl::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRAccessControlClusterAccessControlEntry * newElement_0;
                    newElement_0 = [MTRAccessControlClusterAccessControlEntry new];
                    newElement_0.privilege = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.privilege)];
                    newElement_0.authMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.authMode)];
                    if (entry_0.subjects.IsNull()) {
                        newElement_0.subjects = nil;
                    } else {
                        { // Scope for our temporary variables
                            auto * array_3 = [NSMutableArray new];
                            auto iter_3 = entry_0.subjects.Value().begin();
                            while (iter_3.Next()) {
                                auto & entry_3 = iter_3.GetValue();
                                NSNumber * newElement_3;
                                newElement_3 = [NSNumber numberWithUnsignedLongLong:entry_3];
                                [array_3 addObject:newElement_3];
                            }
                            CHIP_ERROR err = iter_3.GetStatus();
                            if (err != CHIP_NO_ERROR) {
                                *aError = err;
                                return nil;
                            }
                            newElement_0.subjects = array_3;
                        }
                    }
                    if (entry_0.targets.IsNull()) {
                        newElement_0.targets = nil;
                    } else {
                        { // Scope for our temporary variables
                            auto * array_3 = [NSMutableArray new];
                            auto iter_3 = entry_0.targets.Value().begin();
                            while (iter_3.Next()) {
                                auto & entry_3 = iter_3.GetValue();
                                MTRAccessControlClusterTarget * newElement_3;
                                newElement_3 = [MTRAccessControlClusterTarget new];
                                if (entry_3.cluster.IsNull()) {
                                    newElement_3.cluster = nil;
                                } else {
                                    newElement_3.cluster = [NSNumber numberWithUnsignedInt:entry_3.cluster.Value()];
                                }
                                if (entry_3.endpoint.IsNull()) {
                                    newElement_3.endpoint = nil;
                                } else {
                                    newElement_3.endpoint = [NSNumber numberWithUnsignedShort:entry_3.endpoint.Value()];
                                }
                                if (entry_3.deviceType.IsNull()) {
                                    newElement_3.deviceType = nil;
                                } else {
                                    newElement_3.deviceType = [NSNumber numberWithUnsignedInt:entry_3.deviceType.Value()];
                                }
                                [array_3 addObject:newElement_3];
                            }
                            CHIP_ERROR err = iter_3.GetStatus();
                            if (err != CHIP_NO_ERROR) {
                                *aError = err;
                                return nil;
                            }
                            newElement_0.targets = array_3;
                        }
                    }
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::Extension::Id: {
            using TypeInfo = Attributes::Extension::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRAccessControlClusterExtensionEntry * newElement_0;
                    newElement_0 = [MTRAccessControlClusterExtensionEntry new];
                    newElement_0.data = [NSData dataWithBytes:entry_0.data.data() length:entry_0.data.size()];
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::SubjectsPerAccessControlEntry::Id: {
            using TypeInfo = Attributes::SubjectsPerAccessControlEntry::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::TargetsPerAccessControlEntry::Id: {
            using TypeInfo = Attributes::TargetsPerAccessControlEntry::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AccessControlEntriesPerFabric::Id: {
            using TypeInfo = Attributes::AccessControlEntriesPerFabric::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Actions::Id: {
        using namespace Clusters::Actions;
        switch (aPath.mAttributeId) {
        case Attributes::ActionList::Id: {
            using TypeInfo = Attributes::ActionList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRActionsClusterActionStruct * newElement_0;
                    newElement_0 = [MTRActionsClusterActionStruct new];
                    newElement_0.actionID = [NSNumber numberWithUnsignedShort:entry_0.actionID];
                    newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                                 length:entry_0.name.size()
                                                               encoding:NSUTF8StringEncoding];
                    newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type)];
                    newElement_0.endpointListID = [NSNumber numberWithUnsignedShort:entry_0.endpointListID];
                    newElement_0.supportedCommands = [NSNumber numberWithUnsignedShort:entry_0.supportedCommands.Raw()];
                    newElement_0.state = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.state)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::EndpointLists::Id: {
            using TypeInfo = Attributes::EndpointLists::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRActionsClusterEndpointListStruct * newElement_0;
                    newElement_0 = [MTRActionsClusterEndpointListStruct new];
                    newElement_0.endpointListID = [NSNumber numberWithUnsignedShort:entry_0.endpointListID];
                    newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                                 length:entry_0.name.size()
                                                               encoding:NSUTF8StringEncoding];
                    newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type)];
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = entry_0.endpoints.begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            NSNumber * newElement_2;
                            newElement_2 = [NSNumber numberWithUnsignedShort:entry_2];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.endpoints = array_2;
                    }
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::SetupURL::Id: {
            using TypeInfo = Attributes::SetupURL::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Basic::Id: {
        using namespace Clusters::Basic;
        switch (aPath.mAttributeId) {
        case Attributes::DataModelRevision::Id: {
            using TypeInfo = Attributes::DataModelRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::VendorName::Id: {
            using TypeInfo = Attributes::VendorName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::VendorID::Id: {
            using TypeInfo = Attributes::VendorID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ProductName::Id: {
            using TypeInfo = Attributes::ProductName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ProductID::Id: {
            using TypeInfo = Attributes::ProductID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NodeLabel::Id: {
            using TypeInfo = Attributes::NodeLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::Location::Id: {
            using TypeInfo = Attributes::Location::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::HardwareVersion::Id: {
            using TypeInfo = Attributes::HardwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::HardwareVersionString::Id: {
            using TypeInfo = Attributes::HardwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::SoftwareVersion::Id: {
            using TypeInfo = Attributes::SoftwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::SoftwareVersionString::Id: {
            using TypeInfo = Attributes::SoftwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ManufacturingDate::Id: {
            using TypeInfo = Attributes::ManufacturingDate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::PartNumber::Id: {
            using TypeInfo = Attributes::PartNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ProductURL::Id: {
            using TypeInfo = Attributes::ProductURL::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ProductLabel::Id: {
            using TypeInfo = Attributes::ProductLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::SerialNumber::Id: {
            using TypeInfo = Attributes::SerialNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::LocalConfigDisabled::Id: {
            using TypeInfo = Attributes::LocalConfigDisabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::Reachable::Id: {
            using TypeInfo = Attributes::Reachable::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::UniqueID::Id: {
            using TypeInfo = Attributes::UniqueID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::CapabilityMinima::Id: {
            using TypeInfo = Attributes::CapabilityMinima::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRBasicClusterCapabilityMinimaStruct * _Nonnull value;
            value = [MTRBasicClusterCapabilityMinimaStruct new];
            value.caseSessionsPerFabric = [NSNumber numberWithUnsignedShort:cppValue.caseSessionsPerFabric];
            value.subscriptionsPerFabric = [NSNumber numberWithUnsignedShort:cppValue.subscriptionsPerFabric];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        using namespace Clusters::OtaSoftwareUpdateProvider;
        switch (aPath.mAttributeId) {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        using namespace Clusters::OtaSoftwareUpdateRequestor;
        switch (aPath.mAttributeId) {
        case Attributes::DefaultOtaProviders::Id: {
            using TypeInfo = Attributes::DefaultOtaProviders::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTROtaSoftwareUpdateRequestorClusterProviderLocation * newElement_0;
                    newElement_0 = [MTROtaSoftwareUpdateRequestorClusterProviderLocation new];
                    newElement_0.providerNodeID = [NSNumber numberWithUnsignedLongLong:entry_0.providerNodeID];
                    newElement_0.endpoint = [NSNumber numberWithUnsignedShort:entry_0.endpoint];
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::UpdatePossible::Id: {
            using TypeInfo = Attributes::UpdatePossible::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::UpdateState::Id: {
            using TypeInfo = Attributes::UpdateState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::UpdateStateProgress::Id: {
            using TypeInfo = Attributes::UpdateStateProgress::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::LocalizationConfiguration::Id: {
        using namespace Clusters::LocalizationConfiguration;
        switch (aPath.mAttributeId) {
        case Attributes::ActiveLocale::Id: {
            using TypeInfo = Attributes::ActiveLocale::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::SupportedLocales::Id: {
            using TypeInfo = Attributes::SupportedLocales::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSString * newElement_0;
                    newElement_0 = [[NSString alloc] initWithBytes:entry_0.data()
                                                            length:entry_0.size()
                                                          encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TimeFormatLocalization::Id: {
        using namespace Clusters::TimeFormatLocalization;
        switch (aPath.mAttributeId) {
        case Attributes::HourFormat::Id: {
            using TypeInfo = Attributes::HourFormat::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ActiveCalendarType::Id: {
            using TypeInfo = Attributes::ActiveCalendarType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::SupportedCalendarTypes::Id: {
            using TypeInfo = Attributes::SupportedCalendarTypes::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::UnitLocalization::Id: {
        using namespace Clusters::UnitLocalization;
        switch (aPath.mAttributeId) {
        case Attributes::TemperatureUnit::Id: {
            using TypeInfo = Attributes::TemperatureUnit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PowerSourceConfiguration::Id: {
        using namespace Clusters::PowerSourceConfiguration;
        switch (aPath.mAttributeId) {
        case Attributes::Sources::Id: {
            using TypeInfo = Attributes::Sources::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PowerSource::Id: {
        using namespace Clusters::PowerSource;
        switch (aPath.mAttributeId) {
        case Attributes::Status::Id: {
            using TypeInfo = Attributes::Status::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::Order::Id: {
            using TypeInfo = Attributes::Order::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::Description::Id: {
            using TypeInfo = Attributes::Description::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::WiredAssessedInputVoltage::Id: {
            using TypeInfo = Attributes::WiredAssessedInputVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::WiredAssessedInputFrequency::Id: {
            using TypeInfo = Attributes::WiredAssessedInputFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::WiredCurrentType::Id: {
            using TypeInfo = Attributes::WiredCurrentType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::WiredAssessedCurrent::Id: {
            using TypeInfo = Attributes::WiredAssessedCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::WiredNominalVoltage::Id: {
            using TypeInfo = Attributes::WiredNominalVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::WiredMaximumCurrent::Id: {
            using TypeInfo = Attributes::WiredMaximumCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::WiredPresent::Id: {
            using TypeInfo = Attributes::WiredPresent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::ActiveWiredFaults::Id: {
            using TypeInfo = Attributes::ActiveWiredFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::BatVoltage::Id: {
            using TypeInfo = Attributes::BatVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BatPercentRemaining::Id: {
            using TypeInfo = Attributes::BatPercentRemaining::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BatTimeRemaining::Id: {
            using TypeInfo = Attributes::BatTimeRemaining::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BatChargeLevel::Id: {
            using TypeInfo = Attributes::BatChargeLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::BatReplacementNeeded::Id: {
            using TypeInfo = Attributes::BatReplacementNeeded::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::BatReplaceability::Id: {
            using TypeInfo = Attributes::BatReplaceability::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::BatPresent::Id: {
            using TypeInfo = Attributes::BatPresent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::ActiveBatFaults::Id: {
            using TypeInfo = Attributes::ActiveBatFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::BatReplacementDescription::Id: {
            using TypeInfo = Attributes::BatReplacementDescription::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::BatCommonDesignation::Id: {
            using TypeInfo = Attributes::BatCommonDesignation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::BatANSIDesignation::Id: {
            using TypeInfo = Attributes::BatANSIDesignation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::BatIECDesignation::Id: {
            using TypeInfo = Attributes::BatIECDesignation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::BatApprovedChemistry::Id: {
            using TypeInfo = Attributes::BatApprovedChemistry::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::BatCapacity::Id: {
            using TypeInfo = Attributes::BatCapacity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::BatQuantity::Id: {
            using TypeInfo = Attributes::BatQuantity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::BatChargeState::Id: {
            using TypeInfo = Attributes::BatChargeState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::BatTimeToFullCharge::Id: {
            using TypeInfo = Attributes::BatTimeToFullCharge::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BatFunctionalWhileCharging::Id: {
            using TypeInfo = Attributes::BatFunctionalWhileCharging::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::BatChargingCurrent::Id: {
            using TypeInfo = Attributes::BatChargingCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ActiveBatChargeFaults::Id: {
            using TypeInfo = Attributes::ActiveBatChargeFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::GeneralCommissioning::Id: {
        using namespace Clusters::GeneralCommissioning;
        switch (aPath.mAttributeId) {
        case Attributes::Breadcrumb::Id: {
            using TypeInfo = Attributes::Breadcrumb::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::BasicCommissioningInfo::Id: {
            using TypeInfo = Attributes::BasicCommissioningInfo::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRGeneralCommissioningClusterBasicCommissioningInfo * _Nonnull value;
            value = [MTRGeneralCommissioningClusterBasicCommissioningInfo new];
            value.failSafeExpiryLengthSeconds = [NSNumber numberWithUnsignedShort:cppValue.failSafeExpiryLengthSeconds];
            value.maxCumulativeFailsafeSeconds = [NSNumber numberWithUnsignedShort:cppValue.maxCumulativeFailsafeSeconds];
            return value;
        }
        case Attributes::RegulatoryConfig::Id: {
            using TypeInfo = Attributes::RegulatoryConfig::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::LocationCapability::Id: {
            using TypeInfo = Attributes::LocationCapability::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::SupportsConcurrentConnection::Id: {
            using TypeInfo = Attributes::SupportsConcurrentConnection::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::NetworkCommissioning::Id: {
        using namespace Clusters::NetworkCommissioning;
        switch (aPath.mAttributeId) {
        case Attributes::MaxNetworks::Id: {
            using TypeInfo = Attributes::MaxNetworks::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::Networks::Id: {
            using TypeInfo = Attributes::Networks::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRNetworkCommissioningClusterNetworkInfo * newElement_0;
                    newElement_0 = [MTRNetworkCommissioningClusterNetworkInfo new];
                    newElement_0.networkID = [NSData dataWithBytes:entry_0.networkID.data() length:entry_0.networkID.size()];
                    newElement_0.connected = [NSNumber numberWithBool:entry_0.connected];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ScanMaxTimeSeconds::Id: {
            using TypeInfo = Attributes::ScanMaxTimeSeconds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ConnectMaxTimeSeconds::Id: {
            using TypeInfo = Attributes::ConnectMaxTimeSeconds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::InterfaceEnabled::Id: {
            using TypeInfo = Attributes::InterfaceEnabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::LastNetworkingStatus::Id: {
            using TypeInfo = Attributes::LastNetworkingStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::LastNetworkID::Id: {
            using TypeInfo = Attributes::LastNetworkID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSData dataWithBytes:cppValue.Value().data() length:cppValue.Value().size()];
            }
            return value;
        }
        case Attributes::LastConnectErrorValue::Id: {
            using TypeInfo = Attributes::LastConnectErrorValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::DiagnosticLogs::Id: {
        using namespace Clusters::DiagnosticLogs;
        switch (aPath.mAttributeId) {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::GeneralDiagnostics::Id: {
        using namespace Clusters::GeneralDiagnostics;
        switch (aPath.mAttributeId) {
        case Attributes::NetworkInterfaces::Id: {
            using TypeInfo = Attributes::NetworkInterfaces::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRGeneralDiagnosticsClusterNetworkInterfaceType * newElement_0;
                    newElement_0 = [MTRGeneralDiagnosticsClusterNetworkInterfaceType new];
                    newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                                 length:entry_0.name.size()
                                                               encoding:NSUTF8StringEncoding];
                    newElement_0.isOperational = [NSNumber numberWithBool:entry_0.isOperational];
                    if (entry_0.offPremiseServicesReachableIPv4.IsNull()) {
                        newElement_0.offPremiseServicesReachableIPv4 = nil;
                    } else {
                        newElement_0.offPremiseServicesReachableIPv4 =
                            [NSNumber numberWithBool:entry_0.offPremiseServicesReachableIPv4.Value()];
                    }
                    if (entry_0.offPremiseServicesReachableIPv6.IsNull()) {
                        newElement_0.offPremiseServicesReachableIPv6 = nil;
                    } else {
                        newElement_0.offPremiseServicesReachableIPv6 =
                            [NSNumber numberWithBool:entry_0.offPremiseServicesReachableIPv6.Value()];
                    }
                    newElement_0.hardwareAddress = [NSData dataWithBytes:entry_0.hardwareAddress.data()
                                                                  length:entry_0.hardwareAddress.size()];
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = entry_0.IPv4Addresses.begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            NSData * newElement_2;
                            newElement_2 = [NSData dataWithBytes:entry_2.data() length:entry_2.size()];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.iPv4Addresses = array_2;
                    }
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = entry_0.IPv6Addresses.begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            NSData * newElement_2;
                            newElement_2 = [NSData dataWithBytes:entry_2.data() length:entry_2.size()];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.iPv6Addresses = array_2;
                    }
                    newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::RebootCount::Id: {
            using TypeInfo = Attributes::RebootCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::UpTime::Id: {
            using TypeInfo = Attributes::UpTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::TotalOperationalHours::Id: {
            using TypeInfo = Attributes::TotalOperationalHours::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::BootReasons::Id: {
            using TypeInfo = Attributes::BootReasons::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ActiveHardwareFaults::Id: {
            using TypeInfo = Attributes::ActiveHardwareFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ActiveRadioFaults::Id: {
            using TypeInfo = Attributes::ActiveRadioFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ActiveNetworkFaults::Id: {
            using TypeInfo = Attributes::ActiveNetworkFaults::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::TestEventTriggersEnabled::Id: {
            using TypeInfo = Attributes::TestEventTriggersEnabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::SoftwareDiagnostics::Id: {
        using namespace Clusters::SoftwareDiagnostics;
        switch (aPath.mAttributeId) {
        case Attributes::ThreadMetrics::Id: {
            using TypeInfo = Attributes::ThreadMetrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRSoftwareDiagnosticsClusterThreadMetrics * newElement_0;
                    newElement_0 = [MTRSoftwareDiagnosticsClusterThreadMetrics new];
                    newElement_0.id = [NSNumber numberWithUnsignedLongLong:entry_0.id];
                    if (entry_0.name.HasValue()) {
                        newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.Value().data()
                                                                     length:entry_0.name.Value().size()
                                                                   encoding:NSUTF8StringEncoding];
                    } else {
                        newElement_0.name = nil;
                    }
                    if (entry_0.stackFreeCurrent.HasValue()) {
                        newElement_0.stackFreeCurrent = [NSNumber numberWithUnsignedInt:entry_0.stackFreeCurrent.Value()];
                    } else {
                        newElement_0.stackFreeCurrent = nil;
                    }
                    if (entry_0.stackFreeMinimum.HasValue()) {
                        newElement_0.stackFreeMinimum = [NSNumber numberWithUnsignedInt:entry_0.stackFreeMinimum.Value()];
                    } else {
                        newElement_0.stackFreeMinimum = nil;
                    }
                    if (entry_0.stackSize.HasValue()) {
                        newElement_0.stackSize = [NSNumber numberWithUnsignedInt:entry_0.stackSize.Value()];
                    } else {
                        newElement_0.stackSize = nil;
                    }
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentHeapFree::Id: {
            using TypeInfo = Attributes::CurrentHeapFree::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::CurrentHeapUsed::Id: {
            using TypeInfo = Attributes::CurrentHeapUsed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::CurrentHeapHighWatermark::Id: {
            using TypeInfo = Attributes::CurrentHeapHighWatermark::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        using namespace Clusters::ThreadNetworkDiagnostics;
        switch (aPath.mAttributeId) {
        case Attributes::Channel::Id: {
            using TypeInfo = Attributes::Channel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::RoutingRole::Id: {
            using TypeInfo = Attributes::RoutingRole::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::NetworkName::Id: {
            using TypeInfo = Attributes::NetworkName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [[NSString alloc] initWithBytes:cppValue.Value().data()
                                                 length:cppValue.Value().size()
                                               encoding:NSUTF8StringEncoding];
            }
            return value;
        }
        case Attributes::PanId::Id: {
            using TypeInfo = Attributes::PanId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ExtendedPanId::Id: {
            using TypeInfo = Attributes::ExtendedPanId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MeshLocalPrefix::Id: {
            using TypeInfo = Attributes::MeshLocalPrefix::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSData dataWithBytes:cppValue.Value().data() length:cppValue.Value().size()];
            }
            return value;
        }
        case Attributes::OverrunCount::Id: {
            using TypeInfo = Attributes::OverrunCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::NeighborTableList::Id: {
            using TypeInfo = Attributes::NeighborTableList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRThreadNetworkDiagnosticsClusterNeighborTable * newElement_0;
                    newElement_0 = [MTRThreadNetworkDiagnosticsClusterNeighborTable new];
                    newElement_0.extAddress = [NSNumber numberWithUnsignedLongLong:entry_0.extAddress];
                    newElement_0.age = [NSNumber numberWithUnsignedInt:entry_0.age];
                    newElement_0.rloc16 = [NSNumber numberWithUnsignedShort:entry_0.rloc16];
                    newElement_0.linkFrameCounter = [NSNumber numberWithUnsignedInt:entry_0.linkFrameCounter];
                    newElement_0.mleFrameCounter = [NSNumber numberWithUnsignedInt:entry_0.mleFrameCounter];
                    newElement_0.lqi = [NSNumber numberWithUnsignedChar:entry_0.lqi];
                    if (entry_0.averageRssi.IsNull()) {
                        newElement_0.averageRssi = nil;
                    } else {
                        newElement_0.averageRssi = [NSNumber numberWithChar:entry_0.averageRssi.Value()];
                    }
                    if (entry_0.lastRssi.IsNull()) {
                        newElement_0.lastRssi = nil;
                    } else {
                        newElement_0.lastRssi = [NSNumber numberWithChar:entry_0.lastRssi.Value()];
                    }
                    newElement_0.frameErrorRate = [NSNumber numberWithUnsignedChar:entry_0.frameErrorRate];
                    newElement_0.messageErrorRate = [NSNumber numberWithUnsignedChar:entry_0.messageErrorRate];
                    newElement_0.rxOnWhenIdle = [NSNumber numberWithBool:entry_0.rxOnWhenIdle];
                    newElement_0.fullThreadDevice = [NSNumber numberWithBool:entry_0.fullThreadDevice];
                    newElement_0.fullNetworkData = [NSNumber numberWithBool:entry_0.fullNetworkData];
                    newElement_0.isChild = [NSNumber numberWithBool:entry_0.isChild];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::RouteTableList::Id: {
            using TypeInfo = Attributes::RouteTableList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRThreadNetworkDiagnosticsClusterRouteTable * newElement_0;
                    newElement_0 = [MTRThreadNetworkDiagnosticsClusterRouteTable new];
                    newElement_0.extAddress = [NSNumber numberWithUnsignedLongLong:entry_0.extAddress];
                    newElement_0.rloc16 = [NSNumber numberWithUnsignedShort:entry_0.rloc16];
                    newElement_0.routerId = [NSNumber numberWithUnsignedChar:entry_0.routerId];
                    newElement_0.nextHop = [NSNumber numberWithUnsignedChar:entry_0.nextHop];
                    newElement_0.pathCost = [NSNumber numberWithUnsignedChar:entry_0.pathCost];
                    newElement_0.lqiIn = [NSNumber numberWithUnsignedChar:entry_0.LQIIn];
                    newElement_0.lqiOut = [NSNumber numberWithUnsignedChar:entry_0.LQIOut];
                    newElement_0.age = [NSNumber numberWithUnsignedChar:entry_0.age];
                    newElement_0.allocated = [NSNumber numberWithBool:entry_0.allocated];
                    newElement_0.linkEstablished = [NSNumber numberWithBool:entry_0.linkEstablished];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::PartitionId::Id: {
            using TypeInfo = Attributes::PartitionId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Weighting::Id: {
            using TypeInfo = Attributes::Weighting::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::DataVersion::Id: {
            using TypeInfo = Attributes::DataVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::StableDataVersion::Id: {
            using TypeInfo = Attributes::StableDataVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::LeaderRouterId::Id: {
            using TypeInfo = Attributes::LeaderRouterId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::DetachedRoleCount::Id: {
            using TypeInfo = Attributes::DetachedRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ChildRoleCount::Id: {
            using TypeInfo = Attributes::ChildRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RouterRoleCount::Id: {
            using TypeInfo = Attributes::RouterRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::LeaderRoleCount::Id: {
            using TypeInfo = Attributes::LeaderRoleCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AttachAttemptCount::Id: {
            using TypeInfo = Attributes::AttachAttemptCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PartitionIdChangeCount::Id: {
            using TypeInfo = Attributes::PartitionIdChangeCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BetterPartitionAttachAttemptCount::Id: {
            using TypeInfo = Attributes::BetterPartitionAttachAttemptCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ParentChangeCount::Id: {
            using TypeInfo = Attributes::ParentChangeCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::TxTotalCount::Id: {
            using TypeInfo = Attributes::TxTotalCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxUnicastCount::Id: {
            using TypeInfo = Attributes::TxUnicastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxBroadcastCount::Id: {
            using TypeInfo = Attributes::TxBroadcastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxAckRequestedCount::Id: {
            using TypeInfo = Attributes::TxAckRequestedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxAckedCount::Id: {
            using TypeInfo = Attributes::TxAckedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxNoAckRequestedCount::Id: {
            using TypeInfo = Attributes::TxNoAckRequestedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxDataCount::Id: {
            using TypeInfo = Attributes::TxDataCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxDataPollCount::Id: {
            using TypeInfo = Attributes::TxDataPollCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxBeaconCount::Id: {
            using TypeInfo = Attributes::TxBeaconCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxBeaconRequestCount::Id: {
            using TypeInfo = Attributes::TxBeaconRequestCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxOtherCount::Id: {
            using TypeInfo = Attributes::TxOtherCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxRetryCount::Id: {
            using TypeInfo = Attributes::TxRetryCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxDirectMaxRetryExpiryCount::Id: {
            using TypeInfo = Attributes::TxDirectMaxRetryExpiryCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxIndirectMaxRetryExpiryCount::Id: {
            using TypeInfo = Attributes::TxIndirectMaxRetryExpiryCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxErrCcaCount::Id: {
            using TypeInfo = Attributes::TxErrCcaCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxErrAbortCount::Id: {
            using TypeInfo = Attributes::TxErrAbortCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::TxErrBusyChannelCount::Id: {
            using TypeInfo = Attributes::TxErrBusyChannelCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxTotalCount::Id: {
            using TypeInfo = Attributes::RxTotalCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxUnicastCount::Id: {
            using TypeInfo = Attributes::RxUnicastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxBroadcastCount::Id: {
            using TypeInfo = Attributes::RxBroadcastCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxDataCount::Id: {
            using TypeInfo = Attributes::RxDataCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxDataPollCount::Id: {
            using TypeInfo = Attributes::RxDataPollCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxBeaconCount::Id: {
            using TypeInfo = Attributes::RxBeaconCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxBeaconRequestCount::Id: {
            using TypeInfo = Attributes::RxBeaconRequestCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxOtherCount::Id: {
            using TypeInfo = Attributes::RxOtherCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxAddressFilteredCount::Id: {
            using TypeInfo = Attributes::RxAddressFilteredCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxDestAddrFilteredCount::Id: {
            using TypeInfo = Attributes::RxDestAddrFilteredCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxDuplicatedCount::Id: {
            using TypeInfo = Attributes::RxDuplicatedCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxErrNoFrameCount::Id: {
            using TypeInfo = Attributes::RxErrNoFrameCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxErrUnknownNeighborCount::Id: {
            using TypeInfo = Attributes::RxErrUnknownNeighborCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxErrInvalidSrcAddrCount::Id: {
            using TypeInfo = Attributes::RxErrInvalidSrcAddrCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxErrSecCount::Id: {
            using TypeInfo = Attributes::RxErrSecCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxErrFcsCount::Id: {
            using TypeInfo = Attributes::RxErrFcsCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::RxErrOtherCount::Id: {
            using TypeInfo = Attributes::RxErrOtherCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ActiveTimestamp::Id: {
            using TypeInfo = Attributes::ActiveTimestamp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PendingTimestamp::Id: {
            using TypeInfo = Attributes::PendingTimestamp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Delay::Id: {
            using TypeInfo = Attributes::Delay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::SecurityPolicy::Id: {
            using TypeInfo = Attributes::SecurityPolicy::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRThreadNetworkDiagnosticsClusterSecurityPolicy * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRThreadNetworkDiagnosticsClusterSecurityPolicy new];
                value.rotationTime = [NSNumber numberWithUnsignedShort:cppValue.Value().rotationTime];
                value.flags = [NSNumber numberWithUnsignedShort:cppValue.Value().flags];
            }
            return value;
        }
        case Attributes::ChannelPage0Mask::Id: {
            using TypeInfo = Attributes::ChannelPage0Mask::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSData dataWithBytes:cppValue.Value().data() length:cppValue.Value().size()];
            }
            return value;
        }
        case Attributes::OperationalDatasetComponents::Id: {
            using TypeInfo = Attributes::OperationalDatasetComponents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRThreadNetworkDiagnosticsClusterOperationalDatasetComponents new];
                value.activeTimestampPresent = [NSNumber numberWithBool:cppValue.Value().activeTimestampPresent];
                value.pendingTimestampPresent = [NSNumber numberWithBool:cppValue.Value().pendingTimestampPresent];
                value.masterKeyPresent = [NSNumber numberWithBool:cppValue.Value().masterKeyPresent];
                value.networkNamePresent = [NSNumber numberWithBool:cppValue.Value().networkNamePresent];
                value.extendedPanIdPresent = [NSNumber numberWithBool:cppValue.Value().extendedPanIdPresent];
                value.meshLocalPrefixPresent = [NSNumber numberWithBool:cppValue.Value().meshLocalPrefixPresent];
                value.delayPresent = [NSNumber numberWithBool:cppValue.Value().delayPresent];
                value.panIdPresent = [NSNumber numberWithBool:cppValue.Value().panIdPresent];
                value.channelPresent = [NSNumber numberWithBool:cppValue.Value().channelPresent];
                value.pskcPresent = [NSNumber numberWithBool:cppValue.Value().pskcPresent];
                value.securityPolicyPresent = [NSNumber numberWithBool:cppValue.Value().securityPolicyPresent];
                value.channelMaskPresent = [NSNumber numberWithBool:cppValue.Value().channelMaskPresent];
            }
            return value;
        }
        case Attributes::ActiveNetworkFaultsList::Id: {
            using TypeInfo = Attributes::ActiveNetworkFaultsList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        using namespace Clusters::WiFiNetworkDiagnostics;
        switch (aPath.mAttributeId) {
        case Attributes::Bssid::Id: {
            using TypeInfo = Attributes::Bssid::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSData dataWithBytes:cppValue.Value().data() length:cppValue.Value().size()];
            }
            return value;
        }
        case Attributes::SecurityType::Id: {
            using TypeInfo = Attributes::SecurityType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::WiFiVersion::Id: {
            using TypeInfo = Attributes::WiFiVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::ChannelNumber::Id: {
            using TypeInfo = Attributes::ChannelNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Rssi::Id: {
            using TypeInfo = Attributes::Rssi::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BeaconLostCount::Id: {
            using TypeInfo = Attributes::BeaconLostCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BeaconRxCount::Id: {
            using TypeInfo = Attributes::BeaconRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PacketMulticastRxCount::Id: {
            using TypeInfo = Attributes::PacketMulticastRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PacketMulticastTxCount::Id: {
            using TypeInfo = Attributes::PacketMulticastTxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PacketUnicastRxCount::Id: {
            using TypeInfo = Attributes::PacketUnicastRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PacketUnicastTxCount::Id: {
            using TypeInfo = Attributes::PacketUnicastTxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::CurrentMaxRate::Id: {
            using TypeInfo = Attributes::CurrentMaxRate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OverrunCount::Id: {
            using TypeInfo = Attributes::OverrunCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        using namespace Clusters::EthernetNetworkDiagnostics;
        switch (aPath.mAttributeId) {
        case Attributes::PHYRate::Id: {
            using TypeInfo = Attributes::PHYRate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::FullDuplex::Id: {
            using TypeInfo = Attributes::FullDuplex::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithBool:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PacketRxCount::Id: {
            using TypeInfo = Attributes::PacketRxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::PacketTxCount::Id: {
            using TypeInfo = Attributes::PacketTxCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::TxErrCount::Id: {
            using TypeInfo = Attributes::TxErrCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::CollisionCount::Id: {
            using TypeInfo = Attributes::CollisionCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::OverrunCount::Id: {
            using TypeInfo = Attributes::OverrunCount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::CarrierDetect::Id: {
            using TypeInfo = Attributes::CarrierDetect::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithBool:cppValue.Value()];
            }
            return value;
        }
        case Attributes::TimeSinceReset::Id: {
            using TypeInfo = Attributes::TimeSinceReset::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BridgedDeviceBasic::Id: {
        using namespace Clusters::BridgedDeviceBasic;
        switch (aPath.mAttributeId) {
        case Attributes::VendorName::Id: {
            using TypeInfo = Attributes::VendorName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::VendorID::Id: {
            using TypeInfo = Attributes::VendorID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ProductName::Id: {
            using TypeInfo = Attributes::ProductName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::NodeLabel::Id: {
            using TypeInfo = Attributes::NodeLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::HardwareVersion::Id: {
            using TypeInfo = Attributes::HardwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::HardwareVersionString::Id: {
            using TypeInfo = Attributes::HardwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::SoftwareVersion::Id: {
            using TypeInfo = Attributes::SoftwareVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::SoftwareVersionString::Id: {
            using TypeInfo = Attributes::SoftwareVersionString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ManufacturingDate::Id: {
            using TypeInfo = Attributes::ManufacturingDate::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::PartNumber::Id: {
            using TypeInfo = Attributes::PartNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ProductURL::Id: {
            using TypeInfo = Attributes::ProductURL::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ProductLabel::Id: {
            using TypeInfo = Attributes::ProductLabel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::SerialNumber::Id: {
            using TypeInfo = Attributes::SerialNumber::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::Reachable::Id: {
            using TypeInfo = Attributes::Reachable::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::UniqueID::Id: {
            using TypeInfo = Attributes::UniqueID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Switch::Id: {
        using namespace Clusters::Switch;
        switch (aPath.mAttributeId) {
        case Attributes::NumberOfPositions::Id: {
            using TypeInfo = Attributes::NumberOfPositions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CurrentPosition::Id: {
            using TypeInfo = Attributes::CurrentPosition::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MultiPressMax::Id: {
            using TypeInfo = Attributes::MultiPressMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AdministratorCommissioning::Id: {
        using namespace Clusters::AdministratorCommissioning;
        switch (aPath.mAttributeId) {
        case Attributes::WindowStatus::Id: {
            using TypeInfo = Attributes::WindowStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::AdminFabricIndex::Id: {
            using TypeInfo = Attributes::AdminFabricIndex::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::AdminVendorId::Id: {
            using TypeInfo = Attributes::AdminVendorId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OperationalCredentials::Id: {
        using namespace Clusters::OperationalCredentials;
        switch (aPath.mAttributeId) {
        case Attributes::NOCs::Id: {
            using TypeInfo = Attributes::NOCs::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTROperationalCredentialsClusterNOCStruct * newElement_0;
                    newElement_0 = [MTROperationalCredentialsClusterNOCStruct new];
                    newElement_0.noc = [NSData dataWithBytes:entry_0.noc.data() length:entry_0.noc.size()];
                    if (entry_0.icac.IsNull()) {
                        newElement_0.icac = nil;
                    } else {
                        newElement_0.icac = [NSData dataWithBytes:entry_0.icac.Value().data() length:entry_0.icac.Value().size()];
                    }
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::Fabrics::Id: {
            using TypeInfo = Attributes::Fabrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTROperationalCredentialsClusterFabricDescriptor * newElement_0;
                    newElement_0 = [MTROperationalCredentialsClusterFabricDescriptor new];
                    newElement_0.rootPublicKey = [NSData dataWithBytes:entry_0.rootPublicKey.data()
                                                                length:entry_0.rootPublicKey.size()];
                    newElement_0.vendorId = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.vendorId)];
                    newElement_0.fabricId = [NSNumber numberWithUnsignedLongLong:entry_0.fabricId];
                    newElement_0.nodeId = [NSNumber numberWithUnsignedLongLong:entry_0.nodeId];
                    newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                                  length:entry_0.label.size()
                                                                encoding:NSUTF8StringEncoding];
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::SupportedFabrics::Id: {
            using TypeInfo = Attributes::SupportedFabrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CommissionedFabrics::Id: {
            using TypeInfo = Attributes::CommissionedFabrics::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::TrustedRootCertificates::Id: {
            using TypeInfo = Attributes::TrustedRootCertificates::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSData * newElement_0;
                    newElement_0 = [NSData dataWithBytes:entry_0.data() length:entry_0.size()];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentFabricIndex::Id: {
            using TypeInfo = Attributes::CurrentFabricIndex::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::GroupKeyManagement::Id: {
        using namespace Clusters::GroupKeyManagement;
        switch (aPath.mAttributeId) {
        case Attributes::GroupKeyMap::Id: {
            using TypeInfo = Attributes::GroupKeyMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRGroupKeyManagementClusterGroupKeyMapStruct * newElement_0;
                    newElement_0 = [MTRGroupKeyManagementClusterGroupKeyMapStruct new];
                    newElement_0.groupId = [NSNumber numberWithUnsignedShort:entry_0.groupId];
                    newElement_0.groupKeySetID = [NSNumber numberWithUnsignedShort:entry_0.groupKeySetID];
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GroupTable::Id: {
            using TypeInfo = Attributes::GroupTable::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRGroupKeyManagementClusterGroupInfoMapStruct * newElement_0;
                    newElement_0 = [MTRGroupKeyManagementClusterGroupInfoMapStruct new];
                    newElement_0.groupId = [NSNumber numberWithUnsignedShort:entry_0.groupId];
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = entry_0.endpoints.begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            NSNumber * newElement_2;
                            newElement_2 = [NSNumber numberWithUnsignedShort:entry_2];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.endpoints = array_2;
                    }
                    if (entry_0.groupName.HasValue()) {
                        newElement_0.groupName = [[NSString alloc] initWithBytes:entry_0.groupName.Value().data()
                                                                          length:entry_0.groupName.Value().size()
                                                                        encoding:NSUTF8StringEncoding];
                    } else {
                        newElement_0.groupName = nil;
                    }
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::MaxGroupsPerFabric::Id: {
            using TypeInfo = Attributes::MaxGroupsPerFabric::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::MaxGroupKeysPerFabric::Id: {
            using TypeInfo = Attributes::MaxGroupKeysPerFabric::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::FixedLabel::Id: {
        using namespace Clusters::FixedLabel;
        switch (aPath.mAttributeId) {
        case Attributes::LabelList::Id: {
            using TypeInfo = Attributes::LabelList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRFixedLabelClusterLabelStruct * newElement_0;
                    newElement_0 = [MTRFixedLabelClusterLabelStruct new];
                    newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                                  length:entry_0.label.size()
                                                                encoding:NSUTF8StringEncoding];
                    newElement_0.value = [[NSString alloc] initWithBytes:entry_0.value.data()
                                                                  length:entry_0.value.size()
                                                                encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::UserLabel::Id: {
        using namespace Clusters::UserLabel;
        switch (aPath.mAttributeId) {
        case Attributes::LabelList::Id: {
            using TypeInfo = Attributes::LabelList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRUserLabelClusterLabelStruct * newElement_0;
                    newElement_0 = [MTRUserLabelClusterLabelStruct new];
                    newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                                  length:entry_0.label.size()
                                                                encoding:NSUTF8StringEncoding];
                    newElement_0.value = [[NSString alloc] initWithBytes:entry_0.value.data()
                                                                  length:entry_0.value.size()
                                                                encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BooleanState::Id: {
        using namespace Clusters::BooleanState;
        switch (aPath.mAttributeId) {
        case Attributes::StateValue::Id: {
            using TypeInfo = Attributes::StateValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ModeSelect::Id: {
        using namespace Clusters::ModeSelect;
        switch (aPath.mAttributeId) {
        case Attributes::Description::Id: {
            using TypeInfo = Attributes::Description::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::StandardNamespace::Id: {
            using TypeInfo = Attributes::StandardNamespace::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::SupportedModes::Id: {
            using TypeInfo = Attributes::SupportedModes::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRModeSelectClusterModeOptionStruct * newElement_0;
                    newElement_0 = [MTRModeSelectClusterModeOptionStruct new];
                    newElement_0.label = [[NSString alloc] initWithBytes:entry_0.label.data()
                                                                  length:entry_0.label.size()
                                                                encoding:NSUTF8StringEncoding];
                    newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = entry_0.semanticTags.begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            MTRModeSelectClusterSemanticTag * newElement_2;
                            newElement_2 = [MTRModeSelectClusterSemanticTag new];
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:entry_2.mfgCode];
                            newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.semanticTags = array_2;
                    }
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentMode::Id: {
            using TypeInfo = Attributes::CurrentMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::StartUpMode::Id: {
            using TypeInfo = Attributes::StartUpMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OnMode::Id: {
            using TypeInfo = Attributes::OnMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::DoorLock::Id: {
        using namespace Clusters::DoorLock;
        switch (aPath.mAttributeId) {
        case Attributes::LockState::Id: {
            using TypeInfo = Attributes::LockState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::LockType::Id: {
            using TypeInfo = Attributes::LockType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ActuatorEnabled::Id: {
            using TypeInfo = Attributes::ActuatorEnabled::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::DoorState::Id: {
            using TypeInfo = Attributes::DoorState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::DoorOpenEvents::Id: {
            using TypeInfo = Attributes::DoorOpenEvents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::DoorClosedEvents::Id: {
            using TypeInfo = Attributes::DoorClosedEvents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::OpenPeriod::Id: {
            using TypeInfo = Attributes::OpenPeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NumberOfTotalUsersSupported::Id: {
            using TypeInfo = Attributes::NumberOfTotalUsersSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NumberOfPINUsersSupported::Id: {
            using TypeInfo = Attributes::NumberOfPINUsersSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NumberOfRFIDUsersSupported::Id: {
            using TypeInfo = Attributes::NumberOfRFIDUsersSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NumberOfWeekDaySchedulesSupportedPerUser::Id: {
            using TypeInfo = Attributes::NumberOfWeekDaySchedulesSupportedPerUser::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::NumberOfYearDaySchedulesSupportedPerUser::Id: {
            using TypeInfo = Attributes::NumberOfYearDaySchedulesSupportedPerUser::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::NumberOfHolidaySchedulesSupported::Id: {
            using TypeInfo = Attributes::NumberOfHolidaySchedulesSupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MaxPINCodeLength::Id: {
            using TypeInfo = Attributes::MaxPINCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MinPINCodeLength::Id: {
            using TypeInfo = Attributes::MinPINCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MaxRFIDCodeLength::Id: {
            using TypeInfo = Attributes::MaxRFIDCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MinRFIDCodeLength::Id: {
            using TypeInfo = Attributes::MinRFIDCodeLength::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CredentialRulesSupport::Id: {
            using TypeInfo = Attributes::CredentialRulesSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
            return value;
        }
        case Attributes::NumberOfCredentialsSupportedPerUser::Id: {
            using TypeInfo = Attributes::NumberOfCredentialsSupportedPerUser::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::Language::Id: {
            using TypeInfo = Attributes::Language::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::LEDSettings::Id: {
            using TypeInfo = Attributes::LEDSettings::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::AutoRelockTime::Id: {
            using TypeInfo = Attributes::AutoRelockTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::SoundVolume::Id: {
            using TypeInfo = Attributes::SoundVolume::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::OperatingMode::Id: {
            using TypeInfo = Attributes::OperatingMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::SupportedOperatingModes::Id: {
            using TypeInfo = Attributes::SupportedOperatingModes::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
            return value;
        }
        case Attributes::DefaultConfigurationRegister::Id: {
            using TypeInfo = Attributes::DefaultConfigurationRegister::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
            return value;
        }
        case Attributes::EnableLocalProgramming::Id: {
            using TypeInfo = Attributes::EnableLocalProgramming::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::EnableOneTouchLocking::Id: {
            using TypeInfo = Attributes::EnableOneTouchLocking::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::EnableInsideStatusLED::Id: {
            using TypeInfo = Attributes::EnableInsideStatusLED::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::EnablePrivacyModeButton::Id: {
            using TypeInfo = Attributes::EnablePrivacyModeButton::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::LocalProgrammingFeatures::Id: {
            using TypeInfo = Attributes::LocalProgrammingFeatures::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
            return value;
        }
        case Attributes::WrongCodeEntryLimit::Id: {
            using TypeInfo = Attributes::WrongCodeEntryLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::UserCodeTemporaryDisableTime::Id: {
            using TypeInfo = Attributes::UserCodeTemporaryDisableTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::SendPINOverTheAir::Id: {
            using TypeInfo = Attributes::SendPINOverTheAir::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::RequirePINforRemoteOperation::Id: {
            using TypeInfo = Attributes::RequirePINforRemoteOperation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::ExpiringUserTimeout::Id: {
            using TypeInfo = Attributes::ExpiringUserTimeout::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::WindowCovering::Id: {
        using namespace Clusters::WindowCovering;
        switch (aPath.mAttributeId) {
        case Attributes::Type::Id: {
            using TypeInfo = Attributes::Type::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::PhysicalClosedLimitLift::Id: {
            using TypeInfo = Attributes::PhysicalClosedLimitLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PhysicalClosedLimitTilt::Id: {
            using TypeInfo = Attributes::PhysicalClosedLimitTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::CurrentPositionLift::Id: {
            using TypeInfo = Attributes::CurrentPositionLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::CurrentPositionTilt::Id: {
            using TypeInfo = Attributes::CurrentPositionTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NumberOfActuationsLift::Id: {
            using TypeInfo = Attributes::NumberOfActuationsLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NumberOfActuationsTilt::Id: {
            using TypeInfo = Attributes::NumberOfActuationsTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ConfigStatus::Id: {
            using TypeInfo = Attributes::ConfigStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
            return value;
        }
        case Attributes::CurrentPositionLiftPercentage::Id: {
            using TypeInfo = Attributes::CurrentPositionLiftPercentage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::CurrentPositionTiltPercentage::Id: {
            using TypeInfo = Attributes::CurrentPositionTiltPercentage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OperationalStatus::Id: {
            using TypeInfo = Attributes::OperationalStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
            return value;
        }
        case Attributes::TargetPositionLiftPercent100ths::Id: {
            using TypeInfo = Attributes::TargetPositionLiftPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::TargetPositionTiltPercent100ths::Id: {
            using TypeInfo = Attributes::TargetPositionTiltPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::EndProductType::Id: {
            using TypeInfo = Attributes::EndProductType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::CurrentPositionLiftPercent100ths::Id: {
            using TypeInfo = Attributes::CurrentPositionLiftPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::CurrentPositionTiltPercent100ths::Id: {
            using TypeInfo = Attributes::CurrentPositionTiltPercent100ths::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::InstalledOpenLimitLift::Id: {
            using TypeInfo = Attributes::InstalledOpenLimitLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::InstalledClosedLimitLift::Id: {
            using TypeInfo = Attributes::InstalledClosedLimitLift::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::InstalledOpenLimitTilt::Id: {
            using TypeInfo = Attributes::InstalledOpenLimitTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::InstalledClosedLimitTilt::Id: {
            using TypeInfo = Attributes::InstalledClosedLimitTilt::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Mode::Id: {
            using TypeInfo = Attributes::Mode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
            return value;
        }
        case Attributes::SafetyStatus::Id: {
            using TypeInfo = Attributes::SafetyStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BarrierControl::Id: {
        using namespace Clusters::BarrierControl;
        switch (aPath.mAttributeId) {
        case Attributes::BarrierMovingState::Id: {
            using TypeInfo = Attributes::BarrierMovingState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::BarrierSafetyStatus::Id: {
            using TypeInfo = Attributes::BarrierSafetyStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierCapabilities::Id: {
            using TypeInfo = Attributes::BarrierCapabilities::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::BarrierOpenEvents::Id: {
            using TypeInfo = Attributes::BarrierOpenEvents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierCloseEvents::Id: {
            using TypeInfo = Attributes::BarrierCloseEvents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierCommandOpenEvents::Id: {
            using TypeInfo = Attributes::BarrierCommandOpenEvents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierCommandCloseEvents::Id: {
            using TypeInfo = Attributes::BarrierCommandCloseEvents::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierOpenPeriod::Id: {
            using TypeInfo = Attributes::BarrierOpenPeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierClosePeriod::Id: {
            using TypeInfo = Attributes::BarrierClosePeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::BarrierPosition::Id: {
            using TypeInfo = Attributes::BarrierPosition::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        using namespace Clusters::PumpConfigurationAndControl;
        switch (aPath.mAttributeId) {
        case Attributes::MaxPressure::Id: {
            using TypeInfo = Attributes::MaxPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxSpeed::Id: {
            using TypeInfo = Attributes::MaxSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxFlow::Id: {
            using TypeInfo = Attributes::MaxFlow::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinConstPressure::Id: {
            using TypeInfo = Attributes::MinConstPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxConstPressure::Id: {
            using TypeInfo = Attributes::MaxConstPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinCompPressure::Id: {
            using TypeInfo = Attributes::MinCompPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxCompPressure::Id: {
            using TypeInfo = Attributes::MaxCompPressure::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinConstSpeed::Id: {
            using TypeInfo = Attributes::MinConstSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxConstSpeed::Id: {
            using TypeInfo = Attributes::MaxConstSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinConstFlow::Id: {
            using TypeInfo = Attributes::MinConstFlow::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxConstFlow::Id: {
            using TypeInfo = Attributes::MaxConstFlow::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinConstTemp::Id: {
            using TypeInfo = Attributes::MinConstTemp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxConstTemp::Id: {
            using TypeInfo = Attributes::MaxConstTemp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PumpStatus::Id: {
            using TypeInfo = Attributes::PumpStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
            return value;
        }
        case Attributes::EffectiveOperationMode::Id: {
            using TypeInfo = Attributes::EffectiveOperationMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::EffectiveControlMode::Id: {
            using TypeInfo = Attributes::EffectiveControlMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::Capacity::Id: {
            using TypeInfo = Attributes::Capacity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Speed::Id: {
            using TypeInfo = Attributes::Speed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::LifetimeRunningHours::Id: {
            using TypeInfo = Attributes::LifetimeRunningHours::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Power::Id: {
            using TypeInfo = Attributes::Power::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::LifetimeEnergyConsumed::Id: {
            using TypeInfo = Attributes::LifetimeEnergyConsumed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OperationMode::Id: {
            using TypeInfo = Attributes::OperationMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ControlMode::Id: {
            using TypeInfo = Attributes::ControlMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Thermostat::Id: {
        using namespace Clusters::Thermostat;
        switch (aPath.mAttributeId) {
        case Attributes::LocalTemperature::Id: {
            using TypeInfo = Attributes::LocalTemperature::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OutdoorTemperature::Id: {
            using TypeInfo = Attributes::OutdoorTemperature::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Occupancy::Id: {
            using TypeInfo = Attributes::Occupancy::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::AbsMinHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMinHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AbsMaxHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMaxHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AbsMinCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMinCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AbsMaxCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::AbsMaxCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::PICoolingDemand::Id: {
            using TypeInfo = Attributes::PICoolingDemand::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::PIHeatingDemand::Id: {
            using TypeInfo = Attributes::PIHeatingDemand::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::HVACSystemTypeConfiguration::Id: {
            using TypeInfo = Attributes::HVACSystemTypeConfiguration::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::LocalTemperatureCalibration::Id: {
            using TypeInfo = Attributes::LocalTemperatureCalibration::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::OccupiedCoolingSetpoint::Id: {
            using TypeInfo = Attributes::OccupiedCoolingSetpoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::OccupiedHeatingSetpoint::Id: {
            using TypeInfo = Attributes::OccupiedHeatingSetpoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::UnoccupiedCoolingSetpoint::Id: {
            using TypeInfo = Attributes::UnoccupiedCoolingSetpoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::UnoccupiedHeatingSetpoint::Id: {
            using TypeInfo = Attributes::UnoccupiedHeatingSetpoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MinHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::MinHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MaxHeatSetpointLimit::Id: {
            using TypeInfo = Attributes::MaxHeatSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MinCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::MinCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MaxCoolSetpointLimit::Id: {
            using TypeInfo = Attributes::MaxCoolSetpointLimit::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MinSetpointDeadBand::Id: {
            using TypeInfo = Attributes::MinSetpointDeadBand::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::RemoteSensing::Id: {
            using TypeInfo = Attributes::RemoteSensing::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ControlSequenceOfOperation::Id: {
            using TypeInfo = Attributes::ControlSequenceOfOperation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::SystemMode::Id: {
            using TypeInfo = Attributes::SystemMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ThermostatRunningMode::Id: {
            using TypeInfo = Attributes::ThermostatRunningMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::StartOfWeek::Id: {
            using TypeInfo = Attributes::StartOfWeek::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::NumberOfWeeklyTransitions::Id: {
            using TypeInfo = Attributes::NumberOfWeeklyTransitions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::NumberOfDailyTransitions::Id: {
            using TypeInfo = Attributes::NumberOfDailyTransitions::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::TemperatureSetpointHold::Id: {
            using TypeInfo = Attributes::TemperatureSetpointHold::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::TemperatureSetpointHoldDuration::Id: {
            using TypeInfo = Attributes::TemperatureSetpointHoldDuration::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ThermostatProgrammingOperationMode::Id: {
            using TypeInfo = Attributes::ThermostatProgrammingOperationMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ThermostatRunningState::Id: {
            using TypeInfo = Attributes::ThermostatRunningState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::SetpointChangeSource::Id: {
            using TypeInfo = Attributes::SetpointChangeSource::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::SetpointChangeAmount::Id: {
            using TypeInfo = Attributes::SetpointChangeAmount::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::SetpointChangeSourceTimestamp::Id: {
            using TypeInfo = Attributes::SetpointChangeSourceTimestamp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::OccupiedSetback::Id: {
            using TypeInfo = Attributes::OccupiedSetback::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OccupiedSetbackMin::Id: {
            using TypeInfo = Attributes::OccupiedSetbackMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::OccupiedSetbackMax::Id: {
            using TypeInfo = Attributes::OccupiedSetbackMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::UnoccupiedSetback::Id: {
            using TypeInfo = Attributes::UnoccupiedSetback::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::UnoccupiedSetbackMin::Id: {
            using TypeInfo = Attributes::UnoccupiedSetbackMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::UnoccupiedSetbackMax::Id: {
            using TypeInfo = Attributes::UnoccupiedSetbackMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::EmergencyHeatDelta::Id: {
            using TypeInfo = Attributes::EmergencyHeatDelta::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ACType::Id: {
            using TypeInfo = Attributes::ACType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ACCapacity::Id: {
            using TypeInfo = Attributes::ACCapacity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ACRefrigerantType::Id: {
            using TypeInfo = Attributes::ACRefrigerantType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ACCompressorType::Id: {
            using TypeInfo = Attributes::ACCompressorType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ACErrorCode::Id: {
            using TypeInfo = Attributes::ACErrorCode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ACLouverPosition::Id: {
            using TypeInfo = Attributes::ACLouverPosition::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ACCoilTemperature::Id: {
            using TypeInfo = Attributes::ACCoilTemperature::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ACCapacityformat::Id: {
            using TypeInfo = Attributes::ACCapacityformat::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::FanControl::Id: {
        using namespace Clusters::FanControl;
        switch (aPath.mAttributeId) {
        case Attributes::FanMode::Id: {
            using TypeInfo = Attributes::FanMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::FanModeSequence::Id: {
            using TypeInfo = Attributes::FanModeSequence::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::PercentSetting::Id: {
            using TypeInfo = Attributes::PercentSetting::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::PercentCurrent::Id: {
            using TypeInfo = Attributes::PercentCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::SpeedMax::Id: {
            using TypeInfo = Attributes::SpeedMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::SpeedSetting::Id: {
            using TypeInfo = Attributes::SpeedSetting::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::SpeedCurrent::Id: {
            using TypeInfo = Attributes::SpeedCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::RockSupport::Id: {
            using TypeInfo = Attributes::RockSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::RockSetting::Id: {
            using TypeInfo = Attributes::RockSetting::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::WindSupport::Id: {
            using TypeInfo = Attributes::WindSupport::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::WindSetting::Id: {
            using TypeInfo = Attributes::WindSetting::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        using namespace Clusters::ThermostatUserInterfaceConfiguration;
        switch (aPath.mAttributeId) {
        case Attributes::TemperatureDisplayMode::Id: {
            using TypeInfo = Attributes::TemperatureDisplayMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::KeypadLockout::Id: {
            using TypeInfo = Attributes::KeypadLockout::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ScheduleProgrammingVisibility::Id: {
            using TypeInfo = Attributes::ScheduleProgrammingVisibility::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ColorControl::Id: {
        using namespace Clusters::ColorControl;
        switch (aPath.mAttributeId) {
        case Attributes::CurrentHue::Id: {
            using TypeInfo = Attributes::CurrentHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CurrentSaturation::Id: {
            using TypeInfo = Attributes::CurrentSaturation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::RemainingTime::Id: {
            using TypeInfo = Attributes::RemainingTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::CurrentX::Id: {
            using TypeInfo = Attributes::CurrentX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::CurrentY::Id: {
            using TypeInfo = Attributes::CurrentY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::DriftCompensation::Id: {
            using TypeInfo = Attributes::DriftCompensation::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::CompensationText::Id: {
            using TypeInfo = Attributes::CompensationText::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ColorTemperatureMireds::Id: {
            using TypeInfo = Attributes::ColorTemperatureMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorMode::Id: {
            using TypeInfo = Attributes::ColorMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::Options::Id: {
            using TypeInfo = Attributes::Options::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::NumberOfPrimaries::Id: {
            using TypeInfo = Attributes::NumberOfPrimaries::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Primary1X::Id: {
            using TypeInfo = Attributes::Primary1X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary1Y::Id: {
            using TypeInfo = Attributes::Primary1Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary1Intensity::Id: {
            using TypeInfo = Attributes::Primary1Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Primary2X::Id: {
            using TypeInfo = Attributes::Primary2X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary2Y::Id: {
            using TypeInfo = Attributes::Primary2Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary2Intensity::Id: {
            using TypeInfo = Attributes::Primary2Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Primary3X::Id: {
            using TypeInfo = Attributes::Primary3X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary3Y::Id: {
            using TypeInfo = Attributes::Primary3Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary3Intensity::Id: {
            using TypeInfo = Attributes::Primary3Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Primary4X::Id: {
            using TypeInfo = Attributes::Primary4X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary4Y::Id: {
            using TypeInfo = Attributes::Primary4Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary4Intensity::Id: {
            using TypeInfo = Attributes::Primary4Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Primary5X::Id: {
            using TypeInfo = Attributes::Primary5X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary5Y::Id: {
            using TypeInfo = Attributes::Primary5Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary5Intensity::Id: {
            using TypeInfo = Attributes::Primary5Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Primary6X::Id: {
            using TypeInfo = Attributes::Primary6X::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary6Y::Id: {
            using TypeInfo = Attributes::Primary6Y::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Primary6Intensity::Id: {
            using TypeInfo = Attributes::Primary6Intensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::WhitePointX::Id: {
            using TypeInfo = Attributes::WhitePointX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::WhitePointY::Id: {
            using TypeInfo = Attributes::WhitePointY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointRX::Id: {
            using TypeInfo = Attributes::ColorPointRX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointRY::Id: {
            using TypeInfo = Attributes::ColorPointRY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointRIntensity::Id: {
            using TypeInfo = Attributes::ColorPointRIntensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ColorPointGX::Id: {
            using TypeInfo = Attributes::ColorPointGX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointGY::Id: {
            using TypeInfo = Attributes::ColorPointGY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointGIntensity::Id: {
            using TypeInfo = Attributes::ColorPointGIntensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ColorPointBX::Id: {
            using TypeInfo = Attributes::ColorPointBX::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointBY::Id: {
            using TypeInfo = Attributes::ColorPointBY::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorPointBIntensity::Id: {
            using TypeInfo = Attributes::ColorPointBIntensity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::EnhancedCurrentHue::Id: {
            using TypeInfo = Attributes::EnhancedCurrentHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::EnhancedColorMode::Id: {
            using TypeInfo = Attributes::EnhancedColorMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ColorLoopActive::Id: {
            using TypeInfo = Attributes::ColorLoopActive::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ColorLoopDirection::Id: {
            using TypeInfo = Attributes::ColorLoopDirection::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::ColorLoopTime::Id: {
            using TypeInfo = Attributes::ColorLoopTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorLoopStartEnhancedHue::Id: {
            using TypeInfo = Attributes::ColorLoopStartEnhancedHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorLoopStoredEnhancedHue::Id: {
            using TypeInfo = Attributes::ColorLoopStoredEnhancedHue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorCapabilities::Id: {
            using TypeInfo = Attributes::ColorCapabilities::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorTempPhysicalMinMireds::Id: {
            using TypeInfo = Attributes::ColorTempPhysicalMinMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ColorTempPhysicalMaxMireds::Id: {
            using TypeInfo = Attributes::ColorTempPhysicalMaxMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::CoupleColorTempToLevelMinMireds::Id: {
            using TypeInfo = Attributes::CoupleColorTempToLevelMinMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::StartUpColorTemperatureMireds::Id: {
            using TypeInfo = Attributes::StartUpColorTemperatureMireds::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BallastConfiguration::Id: {
        using namespace Clusters::BallastConfiguration;
        switch (aPath.mAttributeId) {
        case Attributes::PhysicalMinLevel::Id: {
            using TypeInfo = Attributes::PhysicalMinLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::PhysicalMaxLevel::Id: {
            using TypeInfo = Attributes::PhysicalMaxLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::BallastStatus::Id: {
            using TypeInfo = Attributes::BallastStatus::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MinLevel::Id: {
            using TypeInfo = Attributes::MinLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::MaxLevel::Id: {
            using TypeInfo = Attributes::MaxLevel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::IntrinsicBalanceFactor::Id: {
            using TypeInfo = Attributes::IntrinsicBalanceFactor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::BallastFactorAdjustment::Id: {
            using TypeInfo = Attributes::BallastFactorAdjustment::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::LampQuantity::Id: {
            using TypeInfo = Attributes::LampQuantity::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::LampType::Id: {
            using TypeInfo = Attributes::LampType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::LampManufacturer::Id: {
            using TypeInfo = Attributes::LampManufacturer::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::LampRatedHours::Id: {
            using TypeInfo = Attributes::LampRatedHours::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::LampBurnHours::Id: {
            using TypeInfo = Attributes::LampBurnHours::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::LampAlarmMode::Id: {
            using TypeInfo = Attributes::LampAlarmMode::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::LampBurnHoursTripPoint::Id: {
            using TypeInfo = Attributes::LampBurnHoursTripPoint::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::IlluminanceMeasurement::Id: {
        using namespace Clusters::IlluminanceMeasurement;
        switch (aPath.mAttributeId) {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::LightSensorType::Id: {
            using TypeInfo = Attributes::LightSensorType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TemperatureMeasurement::Id: {
        using namespace Clusters::TemperatureMeasurement;
        switch (aPath.mAttributeId) {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PressureMeasurement::Id: {
        using namespace Clusters::PressureMeasurement;
        switch (aPath.mAttributeId) {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ScaledValue::Id: {
            using TypeInfo = Attributes::ScaledValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinScaledValue::Id: {
            using TypeInfo = Attributes::MinScaledValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxScaledValue::Id: {
            using TypeInfo = Attributes::MaxScaledValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::ScaledTolerance::Id: {
            using TypeInfo = Attributes::ScaledTolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Scale::Id: {
            using TypeInfo = Attributes::Scale::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::FlowMeasurement::Id: {
        using namespace Clusters::FlowMeasurement;
        switch (aPath.mAttributeId) {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        using namespace Clusters::RelativeHumidityMeasurement;
        switch (aPath.mAttributeId) {
        case Attributes::MeasuredValue::Id: {
            using TypeInfo = Attributes::MeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MinMeasuredValue::Id: {
            using TypeInfo = Attributes::MinMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::MaxMeasuredValue::Id: {
            using TypeInfo = Attributes::MaxMeasuredValue::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Tolerance::Id: {
            using TypeInfo = Attributes::Tolerance::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OccupancySensing::Id: {
        using namespace Clusters::OccupancySensing;
        switch (aPath.mAttributeId) {
        case Attributes::Occupancy::Id: {
            using TypeInfo = Attributes::Occupancy::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::OccupancySensorType::Id: {
            using TypeInfo = Attributes::OccupancySensorType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::OccupancySensorTypeBitmap::Id: {
            using TypeInfo = Attributes::OccupancySensorTypeBitmap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::PirOccupiedToUnoccupiedDelay::Id: {
            using TypeInfo = Attributes::PirOccupiedToUnoccupiedDelay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PirUnoccupiedToOccupiedDelay::Id: {
            using TypeInfo = Attributes::PirUnoccupiedToOccupiedDelay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PirUnoccupiedToOccupiedThreshold::Id: {
            using TypeInfo = Attributes::PirUnoccupiedToOccupiedThreshold::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::UltrasonicOccupiedToUnoccupiedDelay::Id: {
            using TypeInfo = Attributes::UltrasonicOccupiedToUnoccupiedDelay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::UltrasonicUnoccupiedToOccupiedDelay::Id: {
            using TypeInfo = Attributes::UltrasonicUnoccupiedToOccupiedDelay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::UltrasonicUnoccupiedToOccupiedThreshold::Id: {
            using TypeInfo = Attributes::UltrasonicUnoccupiedToOccupiedThreshold::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::PhysicalContactOccupiedToUnoccupiedDelay::Id: {
            using TypeInfo = Attributes::PhysicalContactOccupiedToUnoccupiedDelay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PhysicalContactUnoccupiedToOccupiedDelay::Id: {
            using TypeInfo = Attributes::PhysicalContactUnoccupiedToOccupiedDelay::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::Id: {
            using TypeInfo = Attributes::PhysicalContactUnoccupiedToOccupiedThreshold::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::WakeOnLan::Id: {
        using namespace Clusters::WakeOnLan;
        switch (aPath.mAttributeId) {
        case Attributes::MACAddress::Id: {
            using TypeInfo = Attributes::MACAddress::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Channel::Id: {
        using namespace Clusters::Channel;
        switch (aPath.mAttributeId) {
        case Attributes::ChannelList::Id: {
            using TypeInfo = Attributes::ChannelList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRChannelClusterChannelInfo * newElement_0;
                    newElement_0 = [MTRChannelClusterChannelInfo new];
                    newElement_0.majorNumber = [NSNumber numberWithUnsignedShort:entry_0.majorNumber];
                    newElement_0.minorNumber = [NSNumber numberWithUnsignedShort:entry_0.minorNumber];
                    if (entry_0.name.HasValue()) {
                        newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.Value().data()
                                                                     length:entry_0.name.Value().size()
                                                                   encoding:NSUTF8StringEncoding];
                    } else {
                        newElement_0.name = nil;
                    }
                    if (entry_0.callSign.HasValue()) {
                        newElement_0.callSign = [[NSString alloc] initWithBytes:entry_0.callSign.Value().data()
                                                                         length:entry_0.callSign.Value().size()
                                                                       encoding:NSUTF8StringEncoding];
                    } else {
                        newElement_0.callSign = nil;
                    }
                    if (entry_0.affiliateCallSign.HasValue()) {
                        newElement_0.affiliateCallSign = [[NSString alloc] initWithBytes:entry_0.affiliateCallSign.Value().data()
                                                                                  length:entry_0.affiliateCallSign.Value().size()
                                                                                encoding:NSUTF8StringEncoding];
                    } else {
                        newElement_0.affiliateCallSign = nil;
                    }
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::Lineup::Id: {
            using TypeInfo = Attributes::Lineup::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRChannelClusterLineupInfo * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRChannelClusterLineupInfo new];
                value.operatorName = [[NSString alloc] initWithBytes:cppValue.Value().operatorName.data()
                                                              length:cppValue.Value().operatorName.size()
                                                            encoding:NSUTF8StringEncoding];
                if (cppValue.Value().lineupName.HasValue()) {
                    value.lineupName = [[NSString alloc] initWithBytes:cppValue.Value().lineupName.Value().data()
                                                                length:cppValue.Value().lineupName.Value().size()
                                                              encoding:NSUTF8StringEncoding];
                } else {
                    value.lineupName = nil;
                }
                if (cppValue.Value().postalCode.HasValue()) {
                    value.postalCode = [[NSString alloc] initWithBytes:cppValue.Value().postalCode.Value().data()
                                                                length:cppValue.Value().postalCode.Value().size()
                                                              encoding:NSUTF8StringEncoding];
                } else {
                    value.postalCode = nil;
                }
                value.lineupInfoType = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().lineupInfoType)];
            }
            return value;
        }
        case Attributes::CurrentChannel::Id: {
            using TypeInfo = Attributes::CurrentChannel::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRChannelClusterChannelInfo * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRChannelClusterChannelInfo new];
                value.majorNumber = [NSNumber numberWithUnsignedShort:cppValue.Value().majorNumber];
                value.minorNumber = [NSNumber numberWithUnsignedShort:cppValue.Value().minorNumber];
                if (cppValue.Value().name.HasValue()) {
                    value.name = [[NSString alloc] initWithBytes:cppValue.Value().name.Value().data()
                                                          length:cppValue.Value().name.Value().size()
                                                        encoding:NSUTF8StringEncoding];
                } else {
                    value.name = nil;
                }
                if (cppValue.Value().callSign.HasValue()) {
                    value.callSign = [[NSString alloc] initWithBytes:cppValue.Value().callSign.Value().data()
                                                              length:cppValue.Value().callSign.Value().size()
                                                            encoding:NSUTF8StringEncoding];
                } else {
                    value.callSign = nil;
                }
                if (cppValue.Value().affiliateCallSign.HasValue()) {
                    value.affiliateCallSign = [[NSString alloc] initWithBytes:cppValue.Value().affiliateCallSign.Value().data()
                                                                       length:cppValue.Value().affiliateCallSign.Value().size()
                                                                     encoding:NSUTF8StringEncoding];
                } else {
                    value.affiliateCallSign = nil;
                }
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TargetNavigator::Id: {
        using namespace Clusters::TargetNavigator;
        switch (aPath.mAttributeId) {
        case Attributes::TargetList::Id: {
            using TypeInfo = Attributes::TargetList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRTargetNavigatorClusterTargetInfo * newElement_0;
                    newElement_0 = [MTRTargetNavigatorClusterTargetInfo new];
                    newElement_0.identifier = [NSNumber numberWithUnsignedChar:entry_0.identifier];
                    newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                                 length:entry_0.name.size()
                                                               encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentTarget::Id: {
            using TypeInfo = Attributes::CurrentTarget::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::MediaPlayback::Id: {
        using namespace Clusters::MediaPlayback;
        switch (aPath.mAttributeId) {
        case Attributes::CurrentState::Id: {
            using TypeInfo = Attributes::CurrentState::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::StartTime::Id: {
            using TypeInfo = Attributes::StartTime::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::Duration::Id: {
            using TypeInfo = Attributes::Duration::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::SampledPosition::Id: {
            using TypeInfo = Attributes::SampledPosition::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRMediaPlaybackClusterPlaybackPosition * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRMediaPlaybackClusterPlaybackPosition new];
                value.updatedAt = [NSNumber numberWithUnsignedLongLong:cppValue.Value().updatedAt];
                if (cppValue.Value().position.IsNull()) {
                    value.position = nil;
                } else {
                    value.position = [NSNumber numberWithUnsignedLongLong:cppValue.Value().position.Value()];
                }
            }
            return value;
        }
        case Attributes::PlaybackSpeed::Id: {
            using TypeInfo = Attributes::PlaybackSpeed::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithFloat:cppValue];
            return value;
        }
        case Attributes::SeekRangeEnd::Id: {
            using TypeInfo = Attributes::SeekRangeEnd::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::SeekRangeStart::Id: {
            using TypeInfo = Attributes::SeekRangeStart::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::MediaInput::Id: {
        using namespace Clusters::MediaInput;
        switch (aPath.mAttributeId) {
        case Attributes::InputList::Id: {
            using TypeInfo = Attributes::InputList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRMediaInputClusterInputInfo * newElement_0;
                    newElement_0 = [MTRMediaInputClusterInputInfo new];
                    newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
                    newElement_0.inputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.inputType)];
                    newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                                 length:entry_0.name.size()
                                                               encoding:NSUTF8StringEncoding];
                    newElement_0.descriptionString = [[NSString alloc] initWithBytes:entry_0.description.data()
                                                                              length:entry_0.description.size()
                                                                            encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentInput::Id: {
            using TypeInfo = Attributes::CurrentInput::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::LowPower::Id: {
        using namespace Clusters::LowPower;
        switch (aPath.mAttributeId) {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::KeypadInput::Id: {
        using namespace Clusters::KeypadInput;
        switch (aPath.mAttributeId) {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ContentLauncher::Id: {
        using namespace Clusters::ContentLauncher;
        switch (aPath.mAttributeId) {
        case Attributes::AcceptHeader::Id: {
            using TypeInfo = Attributes::AcceptHeader::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSString * newElement_0;
                    newElement_0 = [[NSString alloc] initWithBytes:entry_0.data()
                                                            length:entry_0.size()
                                                          encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::SupportedStreamingProtocols::Id: {
            using TypeInfo = Attributes::SupportedStreamingProtocols::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AudioOutput::Id: {
        using namespace Clusters::AudioOutput;
        switch (aPath.mAttributeId) {
        case Attributes::OutputList::Id: {
            using TypeInfo = Attributes::OutputList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRAudioOutputClusterOutputInfo * newElement_0;
                    newElement_0 = [MTRAudioOutputClusterOutputInfo new];
                    newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
                    newElement_0.outputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.outputType)];
                    newElement_0.name = [[NSString alloc] initWithBytes:entry_0.name.data()
                                                                 length:entry_0.name.size()
                                                               encoding:NSUTF8StringEncoding];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentOutput::Id: {
            using TypeInfo = Attributes::CurrentOutput::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ApplicationLauncher::Id: {
        using namespace Clusters::ApplicationLauncher;
        switch (aPath.mAttributeId) {
        case Attributes::CatalogList::Id: {
            using TypeInfo = Attributes::CatalogList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedShort:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::CurrentApp::Id: {
            using TypeInfo = Attributes::CurrentApp::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRApplicationLauncherClusterApplicationEP * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRApplicationLauncherClusterApplicationEP new];
                value.application = [MTRApplicationLauncherClusterApplication new];
                value.application.catalogVendorId = [NSNumber numberWithUnsignedShort:cppValue.Value().application.catalogVendorId];
                value.application.applicationId = [[NSString alloc] initWithBytes:cppValue.Value().application.applicationId.data()
                                                                           length:cppValue.Value().application.applicationId.size()
                                                                         encoding:NSUTF8StringEncoding];
                if (cppValue.Value().endpoint.HasValue()) {
                    value.endpoint = [NSNumber numberWithUnsignedShort:cppValue.Value().endpoint.Value()];
                } else {
                    value.endpoint = nil;
                }
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ApplicationBasic::Id: {
        using namespace Clusters::ApplicationBasic;
        switch (aPath.mAttributeId) {
        case Attributes::VendorName::Id: {
            using TypeInfo = Attributes::VendorName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::VendorID::Id: {
            using TypeInfo = Attributes::VendorID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ApplicationName::Id: {
            using TypeInfo = Attributes::ApplicationName::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::ProductID::Id: {
            using TypeInfo = Attributes::ProductID::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Application::Id: {
            using TypeInfo = Attributes::Application::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRApplicationBasicClusterApplicationBasicApplication * _Nonnull value;
            value = [MTRApplicationBasicClusterApplicationBasicApplication new];
            value.catalogVendorId = [NSNumber numberWithUnsignedShort:cppValue.catalogVendorId];
            value.applicationId = [[NSString alloc] initWithBytes:cppValue.applicationId.data()
                                                           length:cppValue.applicationId.size()
                                                         encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::Status::Id: {
            using TypeInfo = Attributes::Status::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ApplicationVersion::Id: {
            using TypeInfo = Attributes::ApplicationVersion::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::AllowedVendorList::Id: {
            using TypeInfo = Attributes::AllowedVendorList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0)];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AccountLogin::Id: {
        using namespace Clusters::AccountLogin;
        switch (aPath.mAttributeId) {
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ElectricalMeasurement::Id: {
        using namespace Clusters::ElectricalMeasurement;
        switch (aPath.mAttributeId) {
        case Attributes::MeasurementType::Id: {
            using TypeInfo = Attributes::MeasurementType::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::DcVoltage::Id: {
            using TypeInfo = Attributes::DcVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcVoltageMin::Id: {
            using TypeInfo = Attributes::DcVoltageMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcVoltageMax::Id: {
            using TypeInfo = Attributes::DcVoltageMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcCurrent::Id: {
            using TypeInfo = Attributes::DcCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcCurrentMin::Id: {
            using TypeInfo = Attributes::DcCurrentMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcCurrentMax::Id: {
            using TypeInfo = Attributes::DcCurrentMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcPower::Id: {
            using TypeInfo = Attributes::DcPower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcPowerMin::Id: {
            using TypeInfo = Attributes::DcPowerMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcPowerMax::Id: {
            using TypeInfo = Attributes::DcPowerMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::DcVoltageMultiplier::Id: {
            using TypeInfo = Attributes::DcVoltageMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::DcVoltageDivisor::Id: {
            using TypeInfo = Attributes::DcVoltageDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::DcCurrentMultiplier::Id: {
            using TypeInfo = Attributes::DcCurrentMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::DcCurrentDivisor::Id: {
            using TypeInfo = Attributes::DcCurrentDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::DcPowerMultiplier::Id: {
            using TypeInfo = Attributes::DcPowerMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::DcPowerDivisor::Id: {
            using TypeInfo = Attributes::DcPowerDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcFrequency::Id: {
            using TypeInfo = Attributes::AcFrequency::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcFrequencyMin::Id: {
            using TypeInfo = Attributes::AcFrequencyMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcFrequencyMax::Id: {
            using TypeInfo = Attributes::AcFrequencyMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::NeutralCurrent::Id: {
            using TypeInfo = Attributes::NeutralCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::TotalActivePower::Id: {
            using TypeInfo = Attributes::TotalActivePower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithInt:cppValue];
            return value;
        }
        case Attributes::TotalReactivePower::Id: {
            using TypeInfo = Attributes::TotalReactivePower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithInt:cppValue];
            return value;
        }
        case Attributes::TotalApparentPower::Id: {
            using TypeInfo = Attributes::TotalApparentPower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::Measured1stHarmonicCurrent::Id: {
            using TypeInfo = Attributes::Measured1stHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::Measured3rdHarmonicCurrent::Id: {
            using TypeInfo = Attributes::Measured3rdHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::Measured5thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::Measured5thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::Measured7thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::Measured7thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::Measured9thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::Measured9thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::Measured11thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::Measured11thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MeasuredPhase1stHarmonicCurrent::Id: {
            using TypeInfo = Attributes::MeasuredPhase1stHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MeasuredPhase3rdHarmonicCurrent::Id: {
            using TypeInfo = Attributes::MeasuredPhase3rdHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MeasuredPhase5thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::MeasuredPhase5thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MeasuredPhase7thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::MeasuredPhase7thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MeasuredPhase9thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::MeasuredPhase9thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::MeasuredPhase11thHarmonicCurrent::Id: {
            using TypeInfo = Attributes::MeasuredPhase11thHarmonicCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AcFrequencyMultiplier::Id: {
            using TypeInfo = Attributes::AcFrequencyMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcFrequencyDivisor::Id: {
            using TypeInfo = Attributes::AcFrequencyDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PowerMultiplier::Id: {
            using TypeInfo = Attributes::PowerMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::PowerDivisor::Id: {
            using TypeInfo = Attributes::PowerDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::HarmonicCurrentMultiplier::Id: {
            using TypeInfo = Attributes::HarmonicCurrentMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::PhaseHarmonicCurrentMultiplier::Id: {
            using TypeInfo = Attributes::PhaseHarmonicCurrentMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::InstantaneousVoltage::Id: {
            using TypeInfo = Attributes::InstantaneousVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::InstantaneousLineCurrent::Id: {
            using TypeInfo = Attributes::InstantaneousLineCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::InstantaneousActiveCurrent::Id: {
            using TypeInfo = Attributes::InstantaneousActiveCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::InstantaneousReactiveCurrent::Id: {
            using TypeInfo = Attributes::InstantaneousReactiveCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::InstantaneousPower::Id: {
            using TypeInfo = Attributes::InstantaneousPower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltage::Id: {
            using TypeInfo = Attributes::RmsVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageMin::Id: {
            using TypeInfo = Attributes::RmsVoltageMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageMax::Id: {
            using TypeInfo = Attributes::RmsVoltageMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrent::Id: {
            using TypeInfo = Attributes::RmsCurrent::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentMin::Id: {
            using TypeInfo = Attributes::RmsCurrentMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentMax::Id: {
            using TypeInfo = Attributes::RmsCurrentMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ActivePower::Id: {
            using TypeInfo = Attributes::ActivePower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerMin::Id: {
            using TypeInfo = Attributes::ActivePowerMin::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerMax::Id: {
            using TypeInfo = Attributes::ActivePowerMax::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ReactivePower::Id: {
            using TypeInfo = Attributes::ReactivePower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ApparentPower::Id: {
            using TypeInfo = Attributes::ApparentPower::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PowerFactor::Id: {
            using TypeInfo = Attributes::PowerFactor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::AverageRmsVoltageMeasurementPeriod::Id: {
            using TypeInfo = Attributes::AverageRmsVoltageMeasurementPeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsUnderVoltageCounter::Id: {
            using TypeInfo = Attributes::AverageRmsUnderVoltageCounter::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeOverVoltagePeriod::Id: {
            using TypeInfo = Attributes::RmsExtremeOverVoltagePeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeUnderVoltagePeriod::Id: {
            using TypeInfo = Attributes::RmsExtremeUnderVoltagePeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSagPeriod::Id: {
            using TypeInfo = Attributes::RmsVoltageSagPeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSwellPeriod::Id: {
            using TypeInfo = Attributes::RmsVoltageSwellPeriod::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcVoltageMultiplier::Id: {
            using TypeInfo = Attributes::AcVoltageMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcVoltageDivisor::Id: {
            using TypeInfo = Attributes::AcVoltageDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcCurrentMultiplier::Id: {
            using TypeInfo = Attributes::AcCurrentMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcCurrentDivisor::Id: {
            using TypeInfo = Attributes::AcCurrentDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcPowerMultiplier::Id: {
            using TypeInfo = Attributes::AcPowerMultiplier::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcPowerDivisor::Id: {
            using TypeInfo = Attributes::AcPowerDivisor::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::OverloadAlarmsMask::Id: {
            using TypeInfo = Attributes::OverloadAlarmsMask::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::VoltageOverload::Id: {
            using TypeInfo = Attributes::VoltageOverload::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::CurrentOverload::Id: {
            using TypeInfo = Attributes::CurrentOverload::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AcOverloadAlarmsMask::Id: {
            using TypeInfo = Attributes::AcOverloadAlarmsMask::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AcVoltageOverload::Id: {
            using TypeInfo = Attributes::AcVoltageOverload::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AcCurrentOverload::Id: {
            using TypeInfo = Attributes::AcCurrentOverload::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AcActivePowerOverload::Id: {
            using TypeInfo = Attributes::AcActivePowerOverload::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AcReactivePowerOverload::Id: {
            using TypeInfo = Attributes::AcReactivePowerOverload::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsOverVoltage::Id: {
            using TypeInfo = Attributes::AverageRmsOverVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsUnderVoltage::Id: {
            using TypeInfo = Attributes::AverageRmsUnderVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeOverVoltage::Id: {
            using TypeInfo = Attributes::RmsExtremeOverVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeUnderVoltage::Id: {
            using TypeInfo = Attributes::RmsExtremeUnderVoltage::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSag::Id: {
            using TypeInfo = Attributes::RmsVoltageSag::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSwell::Id: {
            using TypeInfo = Attributes::RmsVoltageSwell::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::LineCurrentPhaseB::Id: {
            using TypeInfo = Attributes::LineCurrentPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ActiveCurrentPhaseB::Id: {
            using TypeInfo = Attributes::ActiveCurrentPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ReactiveCurrentPhaseB::Id: {
            using TypeInfo = Attributes::ReactiveCurrentPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltagePhaseB::Id: {
            using TypeInfo = Attributes::RmsVoltagePhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageMinPhaseB::Id: {
            using TypeInfo = Attributes::RmsVoltageMinPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageMaxPhaseB::Id: {
            using TypeInfo = Attributes::RmsVoltageMaxPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentPhaseB::Id: {
            using TypeInfo = Attributes::RmsCurrentPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentMinPhaseB::Id: {
            using TypeInfo = Attributes::RmsCurrentMinPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentMaxPhaseB::Id: {
            using TypeInfo = Attributes::RmsCurrentMaxPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerPhaseB::Id: {
            using TypeInfo = Attributes::ActivePowerPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerMinPhaseB::Id: {
            using TypeInfo = Attributes::ActivePowerMinPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerMaxPhaseB::Id: {
            using TypeInfo = Attributes::ActivePowerMaxPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ReactivePowerPhaseB::Id: {
            using TypeInfo = Attributes::ReactivePowerPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ApparentPowerPhaseB::Id: {
            using TypeInfo = Attributes::ApparentPowerPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PowerFactorPhaseB::Id: {
            using TypeInfo = Attributes::PowerFactorPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::AverageRmsVoltageMeasurementPeriodPhaseB::Id: {
            using TypeInfo = Attributes::AverageRmsVoltageMeasurementPeriodPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsOverVoltageCounterPhaseB::Id: {
            using TypeInfo = Attributes::AverageRmsOverVoltageCounterPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsUnderVoltageCounterPhaseB::Id: {
            using TypeInfo = Attributes::AverageRmsUnderVoltageCounterPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeOverVoltagePeriodPhaseB::Id: {
            using TypeInfo = Attributes::RmsExtremeOverVoltagePeriodPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeUnderVoltagePeriodPhaseB::Id: {
            using TypeInfo = Attributes::RmsExtremeUnderVoltagePeriodPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSagPeriodPhaseB::Id: {
            using TypeInfo = Attributes::RmsVoltageSagPeriodPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSwellPeriodPhaseB::Id: {
            using TypeInfo = Attributes::RmsVoltageSwellPeriodPhaseB::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::LineCurrentPhaseC::Id: {
            using TypeInfo = Attributes::LineCurrentPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ActiveCurrentPhaseC::Id: {
            using TypeInfo = Attributes::ActiveCurrentPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ReactiveCurrentPhaseC::Id: {
            using TypeInfo = Attributes::ReactiveCurrentPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltagePhaseC::Id: {
            using TypeInfo = Attributes::RmsVoltagePhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageMinPhaseC::Id: {
            using TypeInfo = Attributes::RmsVoltageMinPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageMaxPhaseC::Id: {
            using TypeInfo = Attributes::RmsVoltageMaxPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentPhaseC::Id: {
            using TypeInfo = Attributes::RmsCurrentPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentMinPhaseC::Id: {
            using TypeInfo = Attributes::RmsCurrentMinPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsCurrentMaxPhaseC::Id: {
            using TypeInfo = Attributes::RmsCurrentMaxPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerPhaseC::Id: {
            using TypeInfo = Attributes::ActivePowerPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerMinPhaseC::Id: {
            using TypeInfo = Attributes::ActivePowerMinPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ActivePowerMaxPhaseC::Id: {
            using TypeInfo = Attributes::ActivePowerMaxPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ReactivePowerPhaseC::Id: {
            using TypeInfo = Attributes::ReactivePowerPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ApparentPowerPhaseC::Id: {
            using TypeInfo = Attributes::ApparentPowerPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::PowerFactorPhaseC::Id: {
            using TypeInfo = Attributes::PowerFactorPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::AverageRmsVoltageMeasurementPeriodPhaseC::Id: {
            using TypeInfo = Attributes::AverageRmsVoltageMeasurementPeriodPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsOverVoltageCounterPhaseC::Id: {
            using TypeInfo = Attributes::AverageRmsOverVoltageCounterPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::AverageRmsUnderVoltageCounterPhaseC::Id: {
            using TypeInfo = Attributes::AverageRmsUnderVoltageCounterPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeOverVoltagePeriodPhaseC::Id: {
            using TypeInfo = Attributes::RmsExtremeOverVoltagePeriodPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsExtremeUnderVoltagePeriodPhaseC::Id: {
            using TypeInfo = Attributes::RmsExtremeUnderVoltagePeriodPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSagPeriodPhaseC::Id: {
            using TypeInfo = Attributes::RmsVoltageSagPeriodPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RmsVoltageSwellPeriodPhaseC::Id: {
            using TypeInfo = Attributes::RmsVoltageSwellPeriodPhaseC::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TestCluster::Id: {
        using namespace Clusters::TestCluster;
        switch (aPath.mAttributeId) {
        case Attributes::Boolean::Id: {
            using TypeInfo = Attributes::Boolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::Bitmap8::Id: {
            using TypeInfo = Attributes::Bitmap8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
            return value;
        }
        case Attributes::Bitmap16::Id: {
            using TypeInfo = Attributes::Bitmap16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
            return value;
        }
        case Attributes::Bitmap32::Id: {
            using TypeInfo = Attributes::Bitmap32::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
            return value;
        }
        case Attributes::Bitmap64::Id: {
            using TypeInfo = Attributes::Bitmap64::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue.Raw()];
            return value;
        }
        case Attributes::Int8u::Id: {
            using TypeInfo = Attributes::Int8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::Int16u::Id: {
            using TypeInfo = Attributes::Int16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::Int24u::Id: {
            using TypeInfo = Attributes::Int24u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::Int32u::Id: {
            using TypeInfo = Attributes::Int32u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::Int40u::Id: {
            using TypeInfo = Attributes::Int40u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::Int48u::Id: {
            using TypeInfo = Attributes::Int48u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::Int56u::Id: {
            using TypeInfo = Attributes::Int56u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::Int64u::Id: {
            using TypeInfo = Attributes::Int64u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::Int8s::Id: {
            using TypeInfo = Attributes::Int8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::Int16s::Id: {
            using TypeInfo = Attributes::Int16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::Int24s::Id: {
            using TypeInfo = Attributes::Int24s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithInt:cppValue];
            return value;
        }
        case Attributes::Int32s::Id: {
            using TypeInfo = Attributes::Int32s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithInt:cppValue];
            return value;
        }
        case Attributes::Int40s::Id: {
            using TypeInfo = Attributes::Int40s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithLongLong:cppValue];
            return value;
        }
        case Attributes::Int48s::Id: {
            using TypeInfo = Attributes::Int48s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithLongLong:cppValue];
            return value;
        }
        case Attributes::Int56s::Id: {
            using TypeInfo = Attributes::Int56s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithLongLong:cppValue];
            return value;
        }
        case Attributes::Int64s::Id: {
            using TypeInfo = Attributes::Int64s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithLongLong:cppValue];
            return value;
        }
        case Attributes::Enum8::Id: {
            using TypeInfo = Attributes::Enum8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::Enum16::Id: {
            using TypeInfo = Attributes::Enum16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::FloatSingle::Id: {
            using TypeInfo = Attributes::FloatSingle::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithFloat:cppValue];
            return value;
        }
        case Attributes::FloatDouble::Id: {
            using TypeInfo = Attributes::FloatDouble::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithDouble:cppValue];
            return value;
        }
        case Attributes::OctetString::Id: {
            using TypeInfo = Attributes::OctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nonnull value;
            value = [NSData dataWithBytes:cppValue.data() length:cppValue.size()];
            return value;
        }
        case Attributes::ListInt8u::Id: {
            using TypeInfo = Attributes::ListInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedChar:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ListOctetString::Id: {
            using TypeInfo = Attributes::ListOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSData * newElement_0;
                    newElement_0 = [NSData dataWithBytes:entry_0.data() length:entry_0.size()];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ListStructOctetString::Id: {
            using TypeInfo = Attributes::ListStructOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRTestClusterClusterTestListStructOctet * newElement_0;
                    newElement_0 = [MTRTestClusterClusterTestListStructOctet new];
                    newElement_0.member1 = [NSNumber numberWithUnsignedLongLong:entry_0.member1];
                    newElement_0.member2 = [NSData dataWithBytes:entry_0.member2.data() length:entry_0.member2.size()];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::LongOctetString::Id: {
            using TypeInfo = Attributes::LongOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nonnull value;
            value = [NSData dataWithBytes:cppValue.data() length:cppValue.size()];
            return value;
        }
        case Attributes::CharString::Id: {
            using TypeInfo = Attributes::CharString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::LongCharString::Id: {
            using TypeInfo = Attributes::LongCharString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nonnull value;
            value = [[NSString alloc] initWithBytes:cppValue.data() length:cppValue.size() encoding:NSUTF8StringEncoding];
            return value;
        }
        case Attributes::EpochUs::Id: {
            using TypeInfo = Attributes::EpochUs::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedLongLong:cppValue];
            return value;
        }
        case Attributes::EpochS::Id: {
            using TypeInfo = Attributes::EpochS::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::VendorId::Id: {
            using TypeInfo = Attributes::VendorId::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::ListNullablesAndOptionalsStruct::Id: {
            using TypeInfo = Attributes::ListNullablesAndOptionalsStruct::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRTestClusterClusterNullablesAndOptionalsStruct * newElement_0;
                    newElement_0 = [MTRTestClusterClusterNullablesAndOptionalsStruct new];
                    if (entry_0.nullableInt.IsNull()) {
                        newElement_0.nullableInt = nil;
                    } else {
                        newElement_0.nullableInt = [NSNumber numberWithUnsignedShort:entry_0.nullableInt.Value()];
                    }
                    if (entry_0.optionalInt.HasValue()) {
                        newElement_0.optionalInt = [NSNumber numberWithUnsignedShort:entry_0.optionalInt.Value()];
                    } else {
                        newElement_0.optionalInt = nil;
                    }
                    if (entry_0.nullableOptionalInt.HasValue()) {
                        if (entry_0.nullableOptionalInt.Value().IsNull()) {
                            newElement_0.nullableOptionalInt = nil;
                        } else {
                            newElement_0.nullableOptionalInt =
                                [NSNumber numberWithUnsignedShort:entry_0.nullableOptionalInt.Value().Value()];
                        }
                    } else {
                        newElement_0.nullableOptionalInt = nil;
                    }
                    if (entry_0.nullableString.IsNull()) {
                        newElement_0.nullableString = nil;
                    } else {
                        newElement_0.nullableString = [[NSString alloc] initWithBytes:entry_0.nullableString.Value().data()
                                                                               length:entry_0.nullableString.Value().size()
                                                                             encoding:NSUTF8StringEncoding];
                    }
                    if (entry_0.optionalString.HasValue()) {
                        newElement_0.optionalString = [[NSString alloc] initWithBytes:entry_0.optionalString.Value().data()
                                                                               length:entry_0.optionalString.Value().size()
                                                                             encoding:NSUTF8StringEncoding];
                    } else {
                        newElement_0.optionalString = nil;
                    }
                    if (entry_0.nullableOptionalString.HasValue()) {
                        if (entry_0.nullableOptionalString.Value().IsNull()) {
                            newElement_0.nullableOptionalString = nil;
                        } else {
                            newElement_0.nullableOptionalString =
                                [[NSString alloc] initWithBytes:entry_0.nullableOptionalString.Value().Value().data()
                                                         length:entry_0.nullableOptionalString.Value().Value().size()
                                                       encoding:NSUTF8StringEncoding];
                        }
                    } else {
                        newElement_0.nullableOptionalString = nil;
                    }
                    if (entry_0.nullableStruct.IsNull()) {
                        newElement_0.nullableStruct = nil;
                    } else {
                        newElement_0.nullableStruct = [MTRTestClusterClusterSimpleStruct new];
                        newElement_0.nullableStruct.a = [NSNumber numberWithUnsignedChar:entry_0.nullableStruct.Value().a];
                        newElement_0.nullableStruct.b = [NSNumber numberWithBool:entry_0.nullableStruct.Value().b];
                        newElement_0.nullableStruct.c =
                            [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableStruct.Value().c)];
                        newElement_0.nullableStruct.d = [NSData dataWithBytes:entry_0.nullableStruct.Value().d.data()
                                                                       length:entry_0.nullableStruct.Value().d.size()];
                        newElement_0.nullableStruct.e = [[NSString alloc] initWithBytes:entry_0.nullableStruct.Value().e.data()
                                                                                 length:entry_0.nullableStruct.Value().e.size()
                                                                               encoding:NSUTF8StringEncoding];
                        newElement_0.nullableStruct.f = [NSNumber numberWithUnsignedChar:entry_0.nullableStruct.Value().f.Raw()];
                        newElement_0.nullableStruct.g = [NSNumber numberWithFloat:entry_0.nullableStruct.Value().g];
                        newElement_0.nullableStruct.h = [NSNumber numberWithDouble:entry_0.nullableStruct.Value().h];
                    }
                    if (entry_0.optionalStruct.HasValue()) {
                        newElement_0.optionalStruct = [MTRTestClusterClusterSimpleStruct new];
                        newElement_0.optionalStruct.a = [NSNumber numberWithUnsignedChar:entry_0.optionalStruct.Value().a];
                        newElement_0.optionalStruct.b = [NSNumber numberWithBool:entry_0.optionalStruct.Value().b];
                        newElement_0.optionalStruct.c =
                            [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.optionalStruct.Value().c)];
                        newElement_0.optionalStruct.d = [NSData dataWithBytes:entry_0.optionalStruct.Value().d.data()
                                                                       length:entry_0.optionalStruct.Value().d.size()];
                        newElement_0.optionalStruct.e = [[NSString alloc] initWithBytes:entry_0.optionalStruct.Value().e.data()
                                                                                 length:entry_0.optionalStruct.Value().e.size()
                                                                               encoding:NSUTF8StringEncoding];
                        newElement_0.optionalStruct.f = [NSNumber numberWithUnsignedChar:entry_0.optionalStruct.Value().f.Raw()];
                        newElement_0.optionalStruct.g = [NSNumber numberWithFloat:entry_0.optionalStruct.Value().g];
                        newElement_0.optionalStruct.h = [NSNumber numberWithDouble:entry_0.optionalStruct.Value().h];
                    } else {
                        newElement_0.optionalStruct = nil;
                    }
                    if (entry_0.nullableOptionalStruct.HasValue()) {
                        if (entry_0.nullableOptionalStruct.Value().IsNull()) {
                            newElement_0.nullableOptionalStruct = nil;
                        } else {
                            newElement_0.nullableOptionalStruct = [MTRTestClusterClusterSimpleStruct new];
                            newElement_0.nullableOptionalStruct.a =
                                [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalStruct.Value().Value().a];
                            newElement_0.nullableOptionalStruct.b =
                                [NSNumber numberWithBool:entry_0.nullableOptionalStruct.Value().Value().b];
                            newElement_0.nullableOptionalStruct.c = [NSNumber
                                numberWithUnsignedChar:chip::to_underlying(entry_0.nullableOptionalStruct.Value().Value().c)];
                            newElement_0.nullableOptionalStruct.d =
                                [NSData dataWithBytes:entry_0.nullableOptionalStruct.Value().Value().d.data()
                                               length:entry_0.nullableOptionalStruct.Value().Value().d.size()];
                            newElement_0.nullableOptionalStruct.e =
                                [[NSString alloc] initWithBytes:entry_0.nullableOptionalStruct.Value().Value().e.data()
                                                         length:entry_0.nullableOptionalStruct.Value().Value().e.size()
                                                       encoding:NSUTF8StringEncoding];
                            newElement_0.nullableOptionalStruct.f =
                                [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalStruct.Value().Value().f.Raw()];
                            newElement_0.nullableOptionalStruct.g =
                                [NSNumber numberWithFloat:entry_0.nullableOptionalStruct.Value().Value().g];
                            newElement_0.nullableOptionalStruct.h =
                                [NSNumber numberWithDouble:entry_0.nullableOptionalStruct.Value().Value().h];
                        }
                    } else {
                        newElement_0.nullableOptionalStruct = nil;
                    }
                    if (entry_0.nullableList.IsNull()) {
                        newElement_0.nullableList = nil;
                    } else {
                        { // Scope for our temporary variables
                            auto * array_3 = [NSMutableArray new];
                            auto iter_3 = entry_0.nullableList.Value().begin();
                            while (iter_3.Next()) {
                                auto & entry_3 = iter_3.GetValue();
                                NSNumber * newElement_3;
                                newElement_3 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_3)];
                                [array_3 addObject:newElement_3];
                            }
                            CHIP_ERROR err = iter_3.GetStatus();
                            if (err != CHIP_NO_ERROR) {
                                *aError = err;
                                return nil;
                            }
                            newElement_0.nullableList = array_3;
                        }
                    }
                    if (entry_0.optionalList.HasValue()) {
                        { // Scope for our temporary variables
                            auto * array_3 = [NSMutableArray new];
                            auto iter_3 = entry_0.optionalList.Value().begin();
                            while (iter_3.Next()) {
                                auto & entry_3 = iter_3.GetValue();
                                NSNumber * newElement_3;
                                newElement_3 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_3)];
                                [array_3 addObject:newElement_3];
                            }
                            CHIP_ERROR err = iter_3.GetStatus();
                            if (err != CHIP_NO_ERROR) {
                                *aError = err;
                                return nil;
                            }
                            newElement_0.optionalList = array_3;
                        }
                    } else {
                        newElement_0.optionalList = nil;
                    }
                    if (entry_0.nullableOptionalList.HasValue()) {
                        if (entry_0.nullableOptionalList.Value().IsNull()) {
                            newElement_0.nullableOptionalList = nil;
                        } else {
                            { // Scope for our temporary variables
                                auto * array_4 = [NSMutableArray new];
                                auto iter_4 = entry_0.nullableOptionalList.Value().Value().begin();
                                while (iter_4.Next()) {
                                    auto & entry_4 = iter_4.GetValue();
                                    NSNumber * newElement_4;
                                    newElement_4 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_4)];
                                    [array_4 addObject:newElement_4];
                                }
                                CHIP_ERROR err = iter_4.GetStatus();
                                if (err != CHIP_NO_ERROR) {
                                    *aError = err;
                                    return nil;
                                }
                                newElement_0.nullableOptionalList = array_4;
                            }
                        }
                    } else {
                        newElement_0.nullableOptionalList = nil;
                    }
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::EnumAttr::Id: {
            using TypeInfo = Attributes::EnumAttr::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
            return value;
        }
        case Attributes::StructAttr::Id: {
            using TypeInfo = Attributes::StructAttr::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRTestClusterClusterSimpleStruct * _Nonnull value;
            value = [MTRTestClusterClusterSimpleStruct new];
            value.a = [NSNumber numberWithUnsignedChar:cppValue.a];
            value.b = [NSNumber numberWithBool:cppValue.b];
            value.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.c)];
            value.d = [NSData dataWithBytes:cppValue.d.data() length:cppValue.d.size()];
            value.e = [[NSString alloc] initWithBytes:cppValue.e.data() length:cppValue.e.size() encoding:NSUTF8StringEncoding];
            value.f = [NSNumber numberWithUnsignedChar:cppValue.f.Raw()];
            value.g = [NSNumber numberWithFloat:cppValue.g];
            value.h = [NSNumber numberWithDouble:cppValue.h];
            return value;
        }
        case Attributes::RangeRestrictedInt8u::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::RangeRestrictedInt8s::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithChar:cppValue];
            return value;
        }
        case Attributes::RangeRestrictedInt16u::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        case Attributes::RangeRestrictedInt16s::Id: {
            using TypeInfo = Attributes::RangeRestrictedInt16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithShort:cppValue];
            return value;
        }
        case Attributes::ListLongOctetString::Id: {
            using TypeInfo = Attributes::ListLongOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSData * newElement_0;
                    newElement_0 = [NSData dataWithBytes:entry_0.data() length:entry_0.size()];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::ListFabricScoped::Id: {
            using TypeInfo = Attributes::ListFabricScoped::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRTestClusterClusterTestFabricScoped * newElement_0;
                    newElement_0 = [MTRTestClusterClusterTestFabricScoped new];
                    newElement_0.fabricSensitiveInt8u = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveInt8u];
                    if (entry_0.optionalFabricSensitiveInt8u.HasValue()) {
                        newElement_0.optionalFabricSensitiveInt8u =
                            [NSNumber numberWithUnsignedChar:entry_0.optionalFabricSensitiveInt8u.Value()];
                    } else {
                        newElement_0.optionalFabricSensitiveInt8u = nil;
                    }
                    if (entry_0.nullableFabricSensitiveInt8u.IsNull()) {
                        newElement_0.nullableFabricSensitiveInt8u = nil;
                    } else {
                        newElement_0.nullableFabricSensitiveInt8u =
                            [NSNumber numberWithUnsignedChar:entry_0.nullableFabricSensitiveInt8u.Value()];
                    }
                    if (entry_0.nullableOptionalFabricSensitiveInt8u.HasValue()) {
                        if (entry_0.nullableOptionalFabricSensitiveInt8u.Value().IsNull()) {
                            newElement_0.nullableOptionalFabricSensitiveInt8u = nil;
                        } else {
                            newElement_0.nullableOptionalFabricSensitiveInt8u =
                                [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalFabricSensitiveInt8u.Value().Value()];
                        }
                    } else {
                        newElement_0.nullableOptionalFabricSensitiveInt8u = nil;
                    }
                    newElement_0.fabricSensitiveCharString =
                        [[NSString alloc] initWithBytes:entry_0.fabricSensitiveCharString.data()
                                                 length:entry_0.fabricSensitiveCharString.size()
                                               encoding:NSUTF8StringEncoding];
                    newElement_0.fabricSensitiveStruct = [MTRTestClusterClusterSimpleStruct new];
                    newElement_0.fabricSensitiveStruct.a = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.a];
                    newElement_0.fabricSensitiveStruct.b = [NSNumber numberWithBool:entry_0.fabricSensitiveStruct.b];
                    newElement_0.fabricSensitiveStruct.c =
                        [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.fabricSensitiveStruct.c)];
                    newElement_0.fabricSensitiveStruct.d = [NSData dataWithBytes:entry_0.fabricSensitiveStruct.d.data()
                                                                          length:entry_0.fabricSensitiveStruct.d.size()];
                    newElement_0.fabricSensitiveStruct.e = [[NSString alloc] initWithBytes:entry_0.fabricSensitiveStruct.e.data()
                                                                                    length:entry_0.fabricSensitiveStruct.e.size()
                                                                                  encoding:NSUTF8StringEncoding];
                    newElement_0.fabricSensitiveStruct.f = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.f.Raw()];
                    newElement_0.fabricSensitiveStruct.g = [NSNumber numberWithFloat:entry_0.fabricSensitiveStruct.g];
                    newElement_0.fabricSensitiveStruct.h = [NSNumber numberWithDouble:entry_0.fabricSensitiveStruct.h];
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = entry_0.fabricSensitiveInt8uList.begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            NSNumber * newElement_2;
                            newElement_2 = [NSNumber numberWithUnsignedChar:entry_2];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.fabricSensitiveInt8uList = array_2;
                    }
                    newElement_0.fabricIndex = [NSNumber numberWithUnsignedChar:entry_0.fabricIndex];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::TimedWriteBoolean::Id: {
            using TypeInfo = Attributes::TimedWriteBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::GeneralErrorBoolean::Id: {
            using TypeInfo = Attributes::GeneralErrorBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::ClusterErrorBoolean::Id: {
            using TypeInfo = Attributes::ClusterErrorBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::Unsupported::Id: {
            using TypeInfo = Attributes::Unsupported::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithBool:cppValue];
            return value;
        }
        case Attributes::NullableBoolean::Id: {
            using TypeInfo = Attributes::NullableBoolean::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithBool:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableBitmap8::Id: {
            using TypeInfo = Attributes::NullableBitmap8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value().Raw()];
            }
            return value;
        }
        case Attributes::NullableBitmap16::Id: {
            using TypeInfo = Attributes::NullableBitmap16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value().Raw()];
            }
            return value;
        }
        case Attributes::NullableBitmap32::Id: {
            using TypeInfo = Attributes::NullableBitmap32::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value().Raw()];
            }
            return value;
        }
        case Attributes::NullableBitmap64::Id: {
            using TypeInfo = Attributes::NullableBitmap64::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value().Raw()];
            }
            return value;
        }
        case Attributes::NullableInt8u::Id: {
            using TypeInfo = Attributes::NullableInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt16u::Id: {
            using TypeInfo = Attributes::NullableInt16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt24u::Id: {
            using TypeInfo = Attributes::NullableInt24u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt32u::Id: {
            using TypeInfo = Attributes::NullableInt32u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt40u::Id: {
            using TypeInfo = Attributes::NullableInt40u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt48u::Id: {
            using TypeInfo = Attributes::NullableInt48u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt56u::Id: {
            using TypeInfo = Attributes::NullableInt56u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt64u::Id: {
            using TypeInfo = Attributes::NullableInt64u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt8s::Id: {
            using TypeInfo = Attributes::NullableInt8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt16s::Id: {
            using TypeInfo = Attributes::NullableInt16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt24s::Id: {
            using TypeInfo = Attributes::NullableInt24s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt32s::Id: {
            using TypeInfo = Attributes::NullableInt32s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithInt:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt40s::Id: {
            using TypeInfo = Attributes::NullableInt40s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt48s::Id: {
            using TypeInfo = Attributes::NullableInt48s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt56s::Id: {
            using TypeInfo = Attributes::NullableInt56s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableInt64s::Id: {
            using TypeInfo = Attributes::NullableInt64s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithLongLong:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableEnum8::Id: {
            using TypeInfo = Attributes::NullableEnum8::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableEnum16::Id: {
            using TypeInfo = Attributes::NullableEnum16::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableFloatSingle::Id: {
            using TypeInfo = Attributes::NullableFloatSingle::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithFloat:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableFloatDouble::Id: {
            using TypeInfo = Attributes::NullableFloatDouble::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithDouble:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableOctetString::Id: {
            using TypeInfo = Attributes::NullableOctetString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSData * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSData dataWithBytes:cppValue.Value().data() length:cppValue.Value().size()];
            }
            return value;
        }
        case Attributes::NullableCharString::Id: {
            using TypeInfo = Attributes::NullableCharString::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSString * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [[NSString alloc] initWithBytes:cppValue.Value().data()
                                                 length:cppValue.Value().size()
                                               encoding:NSUTF8StringEncoding];
            }
            return value;
        }
        case Attributes::NullableEnumAttr::Id: {
            using TypeInfo = Attributes::NullableEnumAttr::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
            }
            return value;
        }
        case Attributes::NullableStruct::Id: {
            using TypeInfo = Attributes::NullableStruct::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            MTRTestClusterClusterSimpleStruct * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [MTRTestClusterClusterSimpleStruct new];
                value.a = [NSNumber numberWithUnsignedChar:cppValue.Value().a];
                value.b = [NSNumber numberWithBool:cppValue.Value().b];
                value.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().c)];
                value.d = [NSData dataWithBytes:cppValue.Value().d.data() length:cppValue.Value().d.size()];
                value.e = [[NSString alloc] initWithBytes:cppValue.Value().e.data()
                                                   length:cppValue.Value().e.size()
                                                 encoding:NSUTF8StringEncoding];
                value.f = [NSNumber numberWithUnsignedChar:cppValue.Value().f.Raw()];
                value.g = [NSNumber numberWithFloat:cppValue.Value().g];
                value.h = [NSNumber numberWithDouble:cppValue.Value().h];
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt8u::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt8s::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt8s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithChar:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt16u::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt16u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::NullableRangeRestrictedInt16s::Id: {
            using TypeInfo = Attributes::NullableRangeRestrictedInt16s::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nullable value;
            if (cppValue.IsNull()) {
                value = nil;
            } else {
                value = [NSNumber numberWithShort:cppValue.Value()];
            }
            return value;
        }
        case Attributes::WriteOnlyInt8u::Id: {
            using TypeInfo = Attributes::WriteOnlyInt8u::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedChar:cppValue];
            return value;
        }
        case Attributes::GeneratedCommandList::Id: {
            using TypeInfo = Attributes::GeneratedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AcceptedCommandList::Id: {
            using TypeInfo = Attributes::AcceptedCommandList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::AttributeList::Id: {
            using TypeInfo = Attributes::AttributeList::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSArray * _Nonnull value;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    NSNumber * newElement_0;
                    newElement_0 = [NSNumber numberWithUnsignedInt:entry_0];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_0;
            }
            return value;
        }
        case Attributes::FeatureMap::Id: {
            using TypeInfo = Attributes::FeatureMap::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedInt:cppValue];
            return value;
        }
        case Attributes::ClusterRevision::Id: {
            using TypeInfo = Attributes::ClusterRevision::TypeInfo;
            TypeInfo::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }
            NSNumber * _Nonnull value;
            value = [NSNumber numberWithUnsignedShort:cppValue];
            return value;
        }
        default:
            *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
            break;
        }
        break;
    }
    default: {
        *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
        break;
    }
    }
    return nil;
}
