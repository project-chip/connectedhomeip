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
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app;

static id _Nullable DecodeGlobalAttributeValue(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Globals;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}

static id _Nullable DecodeAttributeValueForIdentifyCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Identify;
    switch (aAttributeId) {
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForGroupsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Groups;
    switch (aAttributeId) {
    case Attributes::NameSupport::Id: {
        using TypeInfo = Attributes::NameSupport::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOnOffCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OnOff;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForLevelControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LevelControl;
    switch (aAttributeId) {
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPulseWidthModulationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PulseWidthModulation;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDescriptorCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Descriptor;
    switch (aAttributeId) {
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
                newElement_0.deviceType = [NSNumber numberWithUnsignedInt:entry_0.deviceType];
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
    case Attributes::TagList::Id: {
        using TypeInfo = Attributes::TagList::TypeInfo;
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
                MTRDescriptorClusterSemanticTagStruct * newElement_0;
                newElement_0 = [MTRDescriptorClusterSemanticTagStruct new];
                if (entry_0.mfgCode.IsNull()) {
                    newElement_0.mfgCode = nil;
                } else {
                    newElement_0.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.mfgCode.Value())];
                }
                newElement_0.namespaceID = [NSNumber numberWithUnsignedChar:entry_0.namespaceID];
                newElement_0.tag = [NSNumber numberWithUnsignedChar:entry_0.tag];
                if (entry_0.label.HasValue()) {
                    if (entry_0.label.Value().IsNull()) {
                        newElement_0.label = nil;
                    } else {
                        newElement_0.label = AsString(entry_0.label.Value().Value());
                        if (newElement_0.label == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                    }
                } else {
                    newElement_0.label = nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForBindingCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Binding;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForAccessControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AccessControl;
    switch (aAttributeId) {
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
                MTRAccessControlClusterAccessControlEntryStruct * newElement_0;
                newElement_0 = [MTRAccessControlClusterAccessControlEntryStruct new];
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
                            MTRAccessControlClusterAccessControlTargetStruct * newElement_3;
                            newElement_3 = [MTRAccessControlClusterAccessControlTargetStruct new];
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
                MTRAccessControlClusterAccessControlExtensionStruct * newElement_0;
                newElement_0 = [MTRAccessControlClusterAccessControlExtensionStruct new];
                newElement_0.data = AsData(entry_0.data);
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
    case Attributes::CommissioningARL::Id: {
        using TypeInfo = Attributes::CommissioningARL::TypeInfo;
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
                MTRAccessControlClusterCommissioningAccessRestrictionEntryStruct * newElement_0;
                newElement_0 = [MTRAccessControlClusterCommissioningAccessRestrictionEntryStruct new];
                newElement_0.endpoint = [NSNumber numberWithUnsignedShort:entry_0.endpoint];
                newElement_0.cluster = [NSNumber numberWithUnsignedInt:entry_0.cluster];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.restrictions.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRAccessControlClusterAccessRestrictionStruct * newElement_2;
                        newElement_2 = [MTRAccessControlClusterAccessRestrictionStruct new];
                        newElement_2.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.type)];
                        if (entry_2.id.IsNull()) {
                            newElement_2.id = nil;
                        } else {
                            newElement_2.id = [NSNumber numberWithUnsignedInt:entry_2.id.Value()];
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.restrictions = array_2;
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
    case Attributes::Arl::Id: {
        using TypeInfo = Attributes::Arl::TypeInfo;
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
                MTRAccessControlClusterAccessRestrictionEntryStruct * newElement_0;
                newElement_0 = [MTRAccessControlClusterAccessRestrictionEntryStruct new];
                newElement_0.endpoint = [NSNumber numberWithUnsignedShort:entry_0.endpoint];
                newElement_0.cluster = [NSNumber numberWithUnsignedInt:entry_0.cluster];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.restrictions.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRAccessControlClusterAccessRestrictionStruct * newElement_2;
                        newElement_2 = [MTRAccessControlClusterAccessRestrictionStruct new];
                        newElement_2.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.type)];
                        if (entry_2.id.IsNull()) {
                            newElement_2.id = nil;
                        } else {
                            newElement_2.id = [NSNumber numberWithUnsignedInt:entry_2.id.Value()];
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.restrictions = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForActionsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Actions;
    switch (aAttributeId) {
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
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
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
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForBasicInformationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BasicInformation;
    switch (aAttributeId) {
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    case Attributes::CapabilityMinima::Id: {
        using TypeInfo = Attributes::CapabilityMinima::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRBasicInformationClusterCapabilityMinimaStruct * _Nonnull value;
        value = [MTRBasicInformationClusterCapabilityMinimaStruct new];
        value.caseSessionsPerFabric = [NSNumber numberWithUnsignedShort:cppValue.caseSessionsPerFabric];
        value.subscriptionsPerFabric = [NSNumber numberWithUnsignedShort:cppValue.subscriptionsPerFabric];
        return value;
    }
    case Attributes::ProductAppearance::Id: {
        using TypeInfo = Attributes::ProductAppearance::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRBasicInformationClusterProductAppearanceStruct * _Nonnull value;
        value = [MTRBasicInformationClusterProductAppearanceStruct new];
        value.finish = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.finish)];
        if (cppValue.primaryColor.IsNull()) {
            value.primaryColor = nil;
        } else {
            value.primaryColor = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.primaryColor.Value())];
        }
        return value;
    }
    case Attributes::SpecificationVersion::Id: {
        using TypeInfo = Attributes::SpecificationVersion::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::MaxPathsPerInvoke::Id: {
        using TypeInfo = Attributes::MaxPathsPerInvoke::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOTASoftwareUpdateProviderCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OtaSoftwareUpdateProvider;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOTASoftwareUpdateRequestorCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OtaSoftwareUpdateRequestor;
    switch (aAttributeId) {
    case Attributes::DefaultOTAProviders::Id: {
        using TypeInfo = Attributes::DefaultOTAProviders::TypeInfo;
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
                MTROTASoftwareUpdateRequestorClusterProviderLocation * newElement_0;
                newElement_0 = [MTROTASoftwareUpdateRequestorClusterProviderLocation new];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForLocalizationConfigurationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LocalizationConfiguration;
    switch (aAttributeId) {
    case Attributes::ActiveLocale::Id: {
        using TypeInfo = Attributes::ActiveLocale::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
                newElement_0 = AsString(entry_0);
                if (newElement_0 == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTimeFormatLocalizationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TimeFormatLocalization;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForUnitLocalizationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UnitLocalization;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPowerSourceConfigurationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PowerSourceConfiguration;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPowerSourceCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PowerSource;
    switch (aAttributeId) {
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue)];
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue)];
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
    case Attributes::EndpointList::Id: {
        using TypeInfo = Attributes::EndpointList::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForGeneralCommissioningCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::GeneralCommissioning;
    switch (aAttributeId) {
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
    case Attributes::TCAcceptedVersion::Id: {
        using TypeInfo = Attributes::TCAcceptedVersion::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::TCMinRequiredVersion::Id: {
        using TypeInfo = Attributes::TCMinRequiredVersion::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::TCAcknowledgements::Id: {
        using TypeInfo = Attributes::TCAcknowledgements::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::TCAcknowledgementsRequired::Id: {
        using TypeInfo = Attributes::TCAcknowledgementsRequired::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::TCUpdateDeadline::Id: {
        using TypeInfo = Attributes::TCUpdateDeadline::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForNetworkCommissioningCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::NetworkCommissioning;
    switch (aAttributeId) {
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
                MTRNetworkCommissioningClusterNetworkInfoStruct * newElement_0;
                newElement_0 = [MTRNetworkCommissioningClusterNetworkInfoStruct new];
                newElement_0.networkID = AsData(entry_0.networkID);
                newElement_0.connected = [NSNumber numberWithBool:entry_0.connected];
                if (entry_0.networkIdentifier.HasValue()) {
                    if (entry_0.networkIdentifier.Value().IsNull()) {
                        newElement_0.networkIdentifier = nil;
                    } else {
                        newElement_0.networkIdentifier = AsData(entry_0.networkIdentifier.Value().Value());
                    }
                } else {
                    newElement_0.networkIdentifier = nil;
                }
                if (entry_0.clientIdentifier.HasValue()) {
                    if (entry_0.clientIdentifier.Value().IsNull()) {
                        newElement_0.clientIdentifier = nil;
                    } else {
                        newElement_0.clientIdentifier = AsData(entry_0.clientIdentifier.Value().Value());
                    }
                } else {
                    newElement_0.clientIdentifier = nil;
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
            value = AsData(cppValue.Value());
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
    case Attributes::SupportedWiFiBands::Id: {
        using TypeInfo = Attributes::SupportedWiFiBands::TypeInfo;
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
    case Attributes::SupportedThreadFeatures::Id: {
        using TypeInfo = Attributes::SupportedThreadFeatures::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
        return value;
    }
    case Attributes::ThreadVersion::Id: {
        using TypeInfo = Attributes::ThreadVersion::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDiagnosticLogsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DiagnosticLogs;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForGeneralDiagnosticsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::GeneralDiagnostics;
    switch (aAttributeId) {
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
                MTRGeneralDiagnosticsClusterNetworkInterface * newElement_0;
                newElement_0 = [MTRGeneralDiagnosticsClusterNetworkInterface new];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.isOperational = [NSNumber numberWithBool:entry_0.isOperational];
                if (entry_0.offPremiseServicesReachableIPv4.IsNull()) {
                    newElement_0.offPremiseServicesReachableIPv4 = nil;
                } else {
                    newElement_0.offPremiseServicesReachableIPv4 = [NSNumber numberWithBool:entry_0.offPremiseServicesReachableIPv4.Value()];
                }
                if (entry_0.offPremiseServicesReachableIPv6.IsNull()) {
                    newElement_0.offPremiseServicesReachableIPv6 = nil;
                } else {
                    newElement_0.offPremiseServicesReachableIPv6 = [NSNumber numberWithBool:entry_0.offPremiseServicesReachableIPv6.Value()];
                }
                newElement_0.hardwareAddress = AsData(entry_0.hardwareAddress);
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.IPv4Addresses.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        NSData * newElement_2;
                        newElement_2 = AsData(entry_2);
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
                        newElement_2 = AsData(entry_2);
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
    case Attributes::BootReason::Id: {
        using TypeInfo = Attributes::BootReason::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForSoftwareDiagnosticsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SoftwareDiagnostics;
    switch (aAttributeId) {
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
                MTRSoftwareDiagnosticsClusterThreadMetricsStruct * newElement_0;
                newElement_0 = [MTRSoftwareDiagnosticsClusterThreadMetricsStruct new];
                newElement_0.id = [NSNumber numberWithUnsignedLongLong:entry_0.id];
                if (entry_0.name.HasValue()) {
                    newElement_0.name = AsString(entry_0.name.Value());
                    if (newElement_0.name == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForThreadNetworkDiagnosticsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ThreadNetworkDiagnostics;
    switch (aAttributeId) {
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
            value = AsString(cppValue.Value());
            if (value == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
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
            value = AsData(cppValue.Value());
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
    case Attributes::NeighborTable::Id: {
        using TypeInfo = Attributes::NeighborTable::TypeInfo;
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
                MTRThreadNetworkDiagnosticsClusterNeighborTableStruct * newElement_0;
                newElement_0 = [MTRThreadNetworkDiagnosticsClusterNeighborTableStruct new];
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
    case Attributes::RouteTable::Id: {
        using TypeInfo = Attributes::RouteTable::TypeInfo;
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
                MTRThreadNetworkDiagnosticsClusterRouteTableStruct * newElement_0;
                newElement_0 = [MTRThreadNetworkDiagnosticsClusterRouteTableStruct new];
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
            value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
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
            value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
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
            value = [NSNumber numberWithUnsignedShort:cppValue.Value()];
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
            value = AsData(cppValue.Value());
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWiFiNetworkDiagnosticsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WiFiNetworkDiagnostics;
    switch (aAttributeId) {
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
            value = AsData(cppValue.Value());
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForEthernetNetworkDiagnosticsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EthernetNetworkDiagnostics;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTimeSynchronizationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TimeSynchronization;
    switch (aAttributeId) {
    case Attributes::UTCTime::Id: {
        using TypeInfo = Attributes::UTCTime::TypeInfo;
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
    case Attributes::Granularity::Id: {
        using TypeInfo = Attributes::Granularity::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::TimeSource::Id: {
        using TypeInfo = Attributes::TimeSource::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::TrustedTimeSource::Id: {
        using TypeInfo = Attributes::TrustedTimeSource::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRTimeSynchronizationClusterTrustedTimeSourceStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRTimeSynchronizationClusterTrustedTimeSourceStruct new];
            value.fabricIndex = [NSNumber numberWithUnsignedChar:cppValue.Value().fabricIndex];
            value.nodeID = [NSNumber numberWithUnsignedLongLong:cppValue.Value().nodeID];
            value.endpoint = [NSNumber numberWithUnsignedShort:cppValue.Value().endpoint];
        }
        return value;
    }
    case Attributes::DefaultNTP::Id: {
        using TypeInfo = Attributes::DefaultNTP::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsString(cppValue.Value());
            if (value == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        }
        return value;
    }
    case Attributes::TimeZone::Id: {
        using TypeInfo = Attributes::TimeZone::TypeInfo;
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
                MTRTimeSynchronizationClusterTimeZoneStruct * newElement_0;
                newElement_0 = [MTRTimeSynchronizationClusterTimeZoneStruct new];
                newElement_0.offset = [NSNumber numberWithInt:entry_0.offset];
                newElement_0.validAt = [NSNumber numberWithUnsignedLongLong:entry_0.validAt];
                if (entry_0.name.HasValue()) {
                    newElement_0.name = AsString(entry_0.name.Value());
                    if (newElement_0.name == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.name = nil;
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
    case Attributes::DSTOffset::Id: {
        using TypeInfo = Attributes::DSTOffset::TypeInfo;
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
                MTRTimeSynchronizationClusterDSTOffsetStruct * newElement_0;
                newElement_0 = [MTRTimeSynchronizationClusterDSTOffsetStruct new];
                newElement_0.offset = [NSNumber numberWithInt:entry_0.offset];
                newElement_0.validStarting = [NSNumber numberWithUnsignedLongLong:entry_0.validStarting];
                if (entry_0.validUntil.IsNull()) {
                    newElement_0.validUntil = nil;
                } else {
                    newElement_0.validUntil = [NSNumber numberWithUnsignedLongLong:entry_0.validUntil.Value()];
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
    case Attributes::LocalTime::Id: {
        using TypeInfo = Attributes::LocalTime::TypeInfo;
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
    case Attributes::TimeZoneDatabase::Id: {
        using TypeInfo = Attributes::TimeZoneDatabase::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::NTPServerAvailable::Id: {
        using TypeInfo = Attributes::NTPServerAvailable::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::TimeZoneListMaxSize::Id: {
        using TypeInfo = Attributes::TimeZoneListMaxSize::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::DSTOffsetListMaxSize::Id: {
        using TypeInfo = Attributes::DSTOffsetListMaxSize::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::SupportsDNSResolve::Id: {
        using TypeInfo = Attributes::SupportsDNSResolve::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForBridgedDeviceBasicInformationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BridgedDeviceBasicInformation;
    switch (aAttributeId) {
    case Attributes::VendorName::Id: {
        using TypeInfo = Attributes::VendorName::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    case Attributes::ProductAppearance::Id: {
        using TypeInfo = Attributes::ProductAppearance::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct * _Nonnull value;
        value = [MTRBridgedDeviceBasicInformationClusterProductAppearanceStruct new];
        value.finish = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.finish)];
        if (cppValue.primaryColor.IsNull()) {
            value.primaryColor = nil;
        } else {
            value.primaryColor = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.primaryColor.Value())];
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForSwitchCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Switch;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForAdministratorCommissioningCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AdministratorCommissioning;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue.Value())];
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOperationalCredentialsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OperationalCredentials;
    switch (aAttributeId) {
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
                newElement_0.noc = AsData(entry_0.noc);
                if (entry_0.icac.IsNull()) {
                    newElement_0.icac = nil;
                } else {
                    newElement_0.icac = AsData(entry_0.icac.Value());
                }
                if (entry_0.vvsc.HasValue()) {
                    newElement_0.vvsc = AsData(entry_0.vvsc.Value());
                } else {
                    newElement_0.vvsc = nil;
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
                MTROperationalCredentialsClusterFabricDescriptorStruct * newElement_0;
                newElement_0 = [MTROperationalCredentialsClusterFabricDescriptorStruct new];
                newElement_0.rootPublicKey = AsData(entry_0.rootPublicKey);
                newElement_0.vendorID = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.vendorID)];
                newElement_0.fabricID = [NSNumber numberWithUnsignedLongLong:entry_0.fabricID];
                newElement_0.nodeID = [NSNumber numberWithUnsignedLongLong:entry_0.nodeID];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (entry_0.vidVerificationStatement.HasValue()) {
                    newElement_0.vidVerificationStatement = AsData(entry_0.vidVerificationStatement.Value());
                } else {
                    newElement_0.vidVerificationStatement = nil;
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
                newElement_0 = AsData(entry_0);
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForGroupKeyManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::GroupKeyManagement;
    switch (aAttributeId) {
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
                    newElement_0.groupName = AsString(entry_0.groupName.Value());
                    if (newElement_0.groupName == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForFixedLabelCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FixedLabel;
    switch (aAttributeId) {
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
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.value = AsString(entry_0.value);
                if (newElement_0.value == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForUserLabelCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UserLabel;
    switch (aAttributeId) {
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
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.value = AsString(entry_0.value);
                if (newElement_0.value == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForBooleanStateCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BooleanState;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForICDManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::IcdManagement;
    switch (aAttributeId) {
    case Attributes::IdleModeDuration::Id: {
        using TypeInfo = Attributes::IdleModeDuration::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::ActiveModeDuration::Id: {
        using TypeInfo = Attributes::ActiveModeDuration::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::ActiveModeThreshold::Id: {
        using TypeInfo = Attributes::ActiveModeThreshold::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::RegisteredClients::Id: {
        using TypeInfo = Attributes::RegisteredClients::TypeInfo;
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
                MTRICDManagementClusterMonitoringRegistrationStruct * newElement_0;
                newElement_0 = [MTRICDManagementClusterMonitoringRegistrationStruct new];
                newElement_0.checkInNodeID = [NSNumber numberWithUnsignedLongLong:entry_0.checkInNodeID];
                newElement_0.monitoredSubject = [NSNumber numberWithUnsignedLongLong:entry_0.monitoredSubject];
                newElement_0.clientType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.clientType)];
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
    case Attributes::ICDCounter::Id: {
        using TypeInfo = Attributes::ICDCounter::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::ClientsSupportedPerFabric::Id: {
        using TypeInfo = Attributes::ClientsSupportedPerFabric::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::UserActiveModeTriggerHint::Id: {
        using TypeInfo = Attributes::UserActiveModeTriggerHint::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    case Attributes::UserActiveModeTriggerInstruction::Id: {
        using TypeInfo = Attributes::UserActiveModeTriggerInstruction::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
    case Attributes::MaximumCheckInBackOff::Id: {
        using TypeInfo = Attributes::MaximumCheckInBackOff::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTimerCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Timer;
    switch (aAttributeId) {
    case Attributes::SetTime::Id: {
        using TypeInfo = Attributes::SetTime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::TimeRemaining::Id: {
        using TypeInfo = Attributes::TimeRemaining::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::TimerState::Id: {
        using TypeInfo = Attributes::TimerState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOvenCavityOperationalStateCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OvenCavityOperationalState;
    switch (aAttributeId) {
    case Attributes::PhaseList::Id: {
        using TypeInfo = Attributes::PhaseList::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSString * newElement_1;
                    newElement_1 = AsString(entry_1);
                    if (newElement_1 == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    case Attributes::CurrentPhase::Id: {
        using TypeInfo = Attributes::CurrentPhase::TypeInfo;
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
    case Attributes::CountdownTime::Id: {
        using TypeInfo = Attributes::CountdownTime::TypeInfo;
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
    case Attributes::OperationalStateList::Id: {
        using TypeInfo = Attributes::OperationalStateList::TypeInfo;
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
                MTROvenCavityOperationalStateClusterOperationalStateStruct * newElement_0;
                newElement_0 = [MTROvenCavityOperationalStateClusterOperationalStateStruct new];
                newElement_0.operationalStateID = [NSNumber numberWithUnsignedChar:entry_0.operationalStateID];
                if (entry_0.operationalStateLabel.HasValue()) {
                    newElement_0.operationalStateLabel = AsString(entry_0.operationalStateLabel.Value());
                    if (newElement_0.operationalStateLabel == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.operationalStateLabel = nil;
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
    case Attributes::OperationalState::Id: {
        using TypeInfo = Attributes::OperationalState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::OperationalError::Id: {
        using TypeInfo = Attributes::OperationalError::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTROvenCavityOperationalStateClusterErrorStateStruct * _Nonnull value;
        value = [MTROvenCavityOperationalStateClusterErrorStateStruct new];
        value.errorStateID = [NSNumber numberWithUnsignedChar:cppValue.errorStateID];
        if (cppValue.errorStateLabel.HasValue()) {
            value.errorStateLabel = AsString(cppValue.errorStateLabel.Value());
            if (value.errorStateLabel == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        } else {
            value.errorStateLabel = nil;
        }
        if (cppValue.errorStateDetails.HasValue()) {
            value.errorStateDetails = AsString(cppValue.errorStateDetails.Value());
            if (value.errorStateDetails == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        } else {
            value.errorStateDetails = nil;
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOvenModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OvenMode;
    switch (aAttributeId) {
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
                MTROvenModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTROvenModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTROvenModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTROvenModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForLaundryDryerControlsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LaundryDryerControls;
    switch (aAttributeId) {
    case Attributes::SupportedDrynessLevels::Id: {
        using TypeInfo = Attributes::SupportedDrynessLevels::TypeInfo;
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
    case Attributes::SelectedDrynessLevel::Id: {
        using TypeInfo = Attributes::SelectedDrynessLevel::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForModeSelectCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ModeSelect;
    switch (aAttributeId) {
    case Attributes::Description::Id: {
        using TypeInfo = Attributes::Description::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.semanticTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRModeSelectClusterSemanticTagStruct * newElement_2;
                        newElement_2 = [MTRModeSelectClusterSemanticTagStruct new];
                        newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode)];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForLaundryWasherModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LaundryWasherMode;
    switch (aAttributeId) {
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
                MTRLaundryWasherModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRLaundryWasherModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRLaundryWasherModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRLaundryWasherModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRefrigeratorAndTemperatureControlledCabinetModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RefrigeratorAndTemperatureControlledCabinetMode;
    switch (aAttributeId) {
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
                MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRRefrigeratorAndTemperatureControlledCabinetModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForLaundryWasherControlsCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LaundryWasherControls;
    switch (aAttributeId) {
    case Attributes::SpinSpeeds::Id: {
        using TypeInfo = Attributes::SpinSpeeds::TypeInfo;
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
                newElement_0 = AsString(entry_0);
                if (newElement_0 == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    case Attributes::SpinSpeedCurrent::Id: {
        using TypeInfo = Attributes::SpinSpeedCurrent::TypeInfo;
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
    case Attributes::NumberOfRinses::Id: {
        using TypeInfo = Attributes::NumberOfRinses::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::SupportedRinses::Id: {
        using TypeInfo = Attributes::SupportedRinses::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRVCRunModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RvcRunMode;
    switch (aAttributeId) {
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
                MTRRVCRunModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRRVCRunModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRRVCRunModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRRVCRunModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRVCCleanModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RvcCleanMode;
    switch (aAttributeId) {
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
                MTRRVCCleanModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRRVCCleanModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRRVCCleanModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRRVCCleanModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTemperatureControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TemperatureControl;
    switch (aAttributeId) {
    case Attributes::TemperatureSetpoint::Id: {
        using TypeInfo = Attributes::TemperatureSetpoint::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::MinTemperature::Id: {
        using TypeInfo = Attributes::MinTemperature::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::MaxTemperature::Id: {
        using TypeInfo = Attributes::MaxTemperature::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::Step::Id: {
        using TypeInfo = Attributes::Step::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::SelectedTemperatureLevel::Id: {
        using TypeInfo = Attributes::SelectedTemperatureLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::SupportedTemperatureLevels::Id: {
        using TypeInfo = Attributes::SupportedTemperatureLevels::TypeInfo;
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
                newElement_0 = AsString(entry_0);
                if (newElement_0 == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRefrigeratorAlarmCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RefrigeratorAlarm;
    switch (aAttributeId) {
    case Attributes::Mask::Id: {
        using TypeInfo = Attributes::Mask::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    case Attributes::State::Id: {
        using TypeInfo = Attributes::State::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    case Attributes::Supported::Id: {
        using TypeInfo = Attributes::Supported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDishwasherModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DishwasherMode;
    switch (aAttributeId) {
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
                MTRDishwasherModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRDishwasherModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRDishwasherModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRDishwasherModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForAirQualityCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AirQuality;
    switch (aAttributeId) {
    case Attributes::AirQuality::Id: {
        using TypeInfo = Attributes::AirQuality::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForSmokeCOAlarmCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SmokeCoAlarm;
    switch (aAttributeId) {
    case Attributes::ExpressedState::Id: {
        using TypeInfo = Attributes::ExpressedState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::SmokeState::Id: {
        using TypeInfo = Attributes::SmokeState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::COState::Id: {
        using TypeInfo = Attributes::COState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::BatteryAlert::Id: {
        using TypeInfo = Attributes::BatteryAlert::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::DeviceMuted::Id: {
        using TypeInfo = Attributes::DeviceMuted::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::TestInProgress::Id: {
        using TypeInfo = Attributes::TestInProgress::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::HardwareFaultAlert::Id: {
        using TypeInfo = Attributes::HardwareFaultAlert::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::EndOfServiceAlert::Id: {
        using TypeInfo = Attributes::EndOfServiceAlert::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::InterconnectSmokeAlarm::Id: {
        using TypeInfo = Attributes::InterconnectSmokeAlarm::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::InterconnectCOAlarm::Id: {
        using TypeInfo = Attributes::InterconnectCOAlarm::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::ContaminationState::Id: {
        using TypeInfo = Attributes::ContaminationState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::SmokeSensitivityLevel::Id: {
        using TypeInfo = Attributes::SmokeSensitivityLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::ExpiryDate::Id: {
        using TypeInfo = Attributes::ExpiryDate::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDishwasherAlarmCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DishwasherAlarm;
    switch (aAttributeId) {
    case Attributes::Mask::Id: {
        using TypeInfo = Attributes::Mask::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    case Attributes::Latch::Id: {
        using TypeInfo = Attributes::Latch::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    case Attributes::State::Id: {
        using TypeInfo = Attributes::State::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    case Attributes::Supported::Id: {
        using TypeInfo = Attributes::Supported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForMicrowaveOvenModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::MicrowaveOvenMode;
    switch (aAttributeId) {
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
                MTRMicrowaveOvenModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRMicrowaveOvenModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRMicrowaveOvenModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRMicrowaveOvenModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForMicrowaveOvenControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::MicrowaveOvenControl;
    switch (aAttributeId) {
    case Attributes::CookTime::Id: {
        using TypeInfo = Attributes::CookTime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::MaxCookTime::Id: {
        using TypeInfo = Attributes::MaxCookTime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::PowerSetting::Id: {
        using TypeInfo = Attributes::PowerSetting::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MinPower::Id: {
        using TypeInfo = Attributes::MinPower::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MaxPower::Id: {
        using TypeInfo = Attributes::MaxPower::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::PowerStep::Id: {
        using TypeInfo = Attributes::PowerStep::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::SupportedWatts::Id: {
        using TypeInfo = Attributes::SupportedWatts::TypeInfo;
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
    case Attributes::SelectedWattIndex::Id: {
        using TypeInfo = Attributes::SelectedWattIndex::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::WattRating::Id: {
        using TypeInfo = Attributes::WattRating::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOperationalStateCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OperationalState;
    switch (aAttributeId) {
    case Attributes::PhaseList::Id: {
        using TypeInfo = Attributes::PhaseList::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSString * newElement_1;
                    newElement_1 = AsString(entry_1);
                    if (newElement_1 == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    case Attributes::CurrentPhase::Id: {
        using TypeInfo = Attributes::CurrentPhase::TypeInfo;
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
    case Attributes::CountdownTime::Id: {
        using TypeInfo = Attributes::CountdownTime::TypeInfo;
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
    case Attributes::OperationalStateList::Id: {
        using TypeInfo = Attributes::OperationalStateList::TypeInfo;
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
                MTROperationalStateClusterOperationalStateStruct * newElement_0;
                newElement_0 = [MTROperationalStateClusterOperationalStateStruct new];
                newElement_0.operationalStateID = [NSNumber numberWithUnsignedChar:entry_0.operationalStateID];
                if (entry_0.operationalStateLabel.HasValue()) {
                    newElement_0.operationalStateLabel = AsString(entry_0.operationalStateLabel.Value());
                    if (newElement_0.operationalStateLabel == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.operationalStateLabel = nil;
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
    case Attributes::OperationalState::Id: {
        using TypeInfo = Attributes::OperationalState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::OperationalError::Id: {
        using TypeInfo = Attributes::OperationalError::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTROperationalStateClusterErrorStateStruct * _Nonnull value;
        value = [MTROperationalStateClusterErrorStateStruct new];
        value.errorStateID = [NSNumber numberWithUnsignedChar:cppValue.errorStateID];
        if (cppValue.errorStateLabel.HasValue()) {
            value.errorStateLabel = AsString(cppValue.errorStateLabel.Value());
            if (value.errorStateLabel == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        } else {
            value.errorStateLabel = nil;
        }
        if (cppValue.errorStateDetails.HasValue()) {
            value.errorStateDetails = AsString(cppValue.errorStateDetails.Value());
            if (value.errorStateDetails == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        } else {
            value.errorStateDetails = nil;
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRVCOperationalStateCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RvcOperationalState;
    switch (aAttributeId) {
    case Attributes::PhaseList::Id: {
        using TypeInfo = Attributes::PhaseList::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    NSString * newElement_1;
                    newElement_1 = AsString(entry_1);
                    if (newElement_1 == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    case Attributes::CurrentPhase::Id: {
        using TypeInfo = Attributes::CurrentPhase::TypeInfo;
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
    case Attributes::CountdownTime::Id: {
        using TypeInfo = Attributes::CountdownTime::TypeInfo;
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
    case Attributes::OperationalStateList::Id: {
        using TypeInfo = Attributes::OperationalStateList::TypeInfo;
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
                MTRRVCOperationalStateClusterOperationalStateStruct * newElement_0;
                newElement_0 = [MTRRVCOperationalStateClusterOperationalStateStruct new];
                newElement_0.operationalStateID = [NSNumber numberWithUnsignedChar:entry_0.operationalStateID];
                if (entry_0.operationalStateLabel.HasValue()) {
                    newElement_0.operationalStateLabel = AsString(entry_0.operationalStateLabel.Value());
                    if (newElement_0.operationalStateLabel == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.operationalStateLabel = nil;
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
    case Attributes::OperationalState::Id: {
        using TypeInfo = Attributes::OperationalState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::OperationalError::Id: {
        using TypeInfo = Attributes::OperationalError::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRRVCOperationalStateClusterErrorStateStruct * _Nonnull value;
        value = [MTRRVCOperationalStateClusterErrorStateStruct new];
        value.errorStateID = [NSNumber numberWithUnsignedChar:cppValue.errorStateID];
        if (cppValue.errorStateLabel.HasValue()) {
            value.errorStateLabel = AsString(cppValue.errorStateLabel.Value());
            if (value.errorStateLabel == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        } else {
            value.errorStateLabel = nil;
        }
        if (cppValue.errorStateDetails.HasValue()) {
            value.errorStateDetails = AsString(cppValue.errorStateDetails.Value());
            if (value.errorStateDetails == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        } else {
            value.errorStateDetails = nil;
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForScenesManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ScenesManagement;
    switch (aAttributeId) {
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
    case Attributes::SceneTableSize::Id: {
        using TypeInfo = Attributes::SceneTableSize::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::FabricSceneInfo::Id: {
        using TypeInfo = Attributes::FabricSceneInfo::TypeInfo;
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
                MTRScenesManagementClusterSceneInfoStruct * newElement_0;
                newElement_0 = [MTRScenesManagementClusterSceneInfoStruct new];
                newElement_0.sceneCount = [NSNumber numberWithUnsignedChar:entry_0.sceneCount];
                newElement_0.currentScene = [NSNumber numberWithUnsignedChar:entry_0.currentScene];
                newElement_0.currentGroup = [NSNumber numberWithUnsignedShort:entry_0.currentGroup];
                newElement_0.sceneValid = [NSNumber numberWithBool:entry_0.sceneValid];
                newElement_0.remainingCapacity = [NSNumber numberWithUnsignedChar:entry_0.remainingCapacity];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForHEPAFilterMonitoringCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::HepaFilterMonitoring;
    switch (aAttributeId) {
    case Attributes::Condition::Id: {
        using TypeInfo = Attributes::Condition::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::DegradationDirection::Id: {
        using TypeInfo = Attributes::DegradationDirection::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::ChangeIndication::Id: {
        using TypeInfo = Attributes::ChangeIndication::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::InPlaceIndicator::Id: {
        using TypeInfo = Attributes::InPlaceIndicator::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::LastChangedTime::Id: {
        using TypeInfo = Attributes::LastChangedTime::TypeInfo;
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
    case Attributes::ReplacementProductList::Id: {
        using TypeInfo = Attributes::ReplacementProductList::TypeInfo;
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
                MTRHEPAFilterMonitoringClusterReplacementProductStruct * newElement_0;
                newElement_0 = [MTRHEPAFilterMonitoringClusterReplacementProductStruct new];
                newElement_0.productIdentifierType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.productIdentifierType)];
                newElement_0.productIdentifierValue = AsString(entry_0.productIdentifierValue);
                if (newElement_0.productIdentifierValue == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForActivatedCarbonFilterMonitoringCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ActivatedCarbonFilterMonitoring;
    switch (aAttributeId) {
    case Attributes::Condition::Id: {
        using TypeInfo = Attributes::Condition::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::DegradationDirection::Id: {
        using TypeInfo = Attributes::DegradationDirection::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::ChangeIndication::Id: {
        using TypeInfo = Attributes::ChangeIndication::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::InPlaceIndicator::Id: {
        using TypeInfo = Attributes::InPlaceIndicator::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::LastChangedTime::Id: {
        using TypeInfo = Attributes::LastChangedTime::TypeInfo;
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
    case Attributes::ReplacementProductList::Id: {
        using TypeInfo = Attributes::ReplacementProductList::TypeInfo;
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
                MTRActivatedCarbonFilterMonitoringClusterReplacementProductStruct * newElement_0;
                newElement_0 = [MTRActivatedCarbonFilterMonitoringClusterReplacementProductStruct new];
                newElement_0.productIdentifierType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.productIdentifierType)];
                newElement_0.productIdentifierValue = AsString(entry_0.productIdentifierValue);
                if (newElement_0.productIdentifierValue == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForBooleanStateConfigurationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BooleanStateConfiguration;
    switch (aAttributeId) {
    case Attributes::CurrentSensitivityLevel::Id: {
        using TypeInfo = Attributes::CurrentSensitivityLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::SupportedSensitivityLevels::Id: {
        using TypeInfo = Attributes::SupportedSensitivityLevels::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::DefaultSensitivityLevel::Id: {
        using TypeInfo = Attributes::DefaultSensitivityLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::AlarmsActive::Id: {
        using TypeInfo = Attributes::AlarmsActive::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::AlarmsSuppressed::Id: {
        using TypeInfo = Attributes::AlarmsSuppressed::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::AlarmsEnabled::Id: {
        using TypeInfo = Attributes::AlarmsEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::AlarmsSupported::Id: {
        using TypeInfo = Attributes::AlarmsSupported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::SensorFault::Id: {
        using TypeInfo = Attributes::SensorFault::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForValveConfigurationAndControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ValveConfigurationAndControl;
    switch (aAttributeId) {
    case Attributes::OpenDuration::Id: {
        using TypeInfo = Attributes::OpenDuration::TypeInfo;
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
    case Attributes::DefaultOpenDuration::Id: {
        using TypeInfo = Attributes::DefaultOpenDuration::TypeInfo;
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
    case Attributes::AutoCloseTime::Id: {
        using TypeInfo = Attributes::AutoCloseTime::TypeInfo;
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
    case Attributes::RemainingDuration::Id: {
        using TypeInfo = Attributes::RemainingDuration::TypeInfo;
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
    case Attributes::CurrentState::Id: {
        using TypeInfo = Attributes::CurrentState::TypeInfo;
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
    case Attributes::TargetState::Id: {
        using TypeInfo = Attributes::TargetState::TypeInfo;
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
    case Attributes::TargetLevel::Id: {
        using TypeInfo = Attributes::TargetLevel::TypeInfo;
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
    case Attributes::DefaultOpenLevel::Id: {
        using TypeInfo = Attributes::DefaultOpenLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::ValveFault::Id: {
        using TypeInfo = Attributes::ValveFault::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
        return value;
    }
    case Attributes::LevelStep::Id: {
        using TypeInfo = Attributes::LevelStep::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForElectricalPowerMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ElectricalPowerMeasurement;
    switch (aAttributeId) {
    case Attributes::PowerMode::Id: {
        using TypeInfo = Attributes::PowerMode::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::NumberOfMeasurementTypes::Id: {
        using TypeInfo = Attributes::NumberOfMeasurementTypes::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::Accuracy::Id: {
        using TypeInfo = Attributes::Accuracy::TypeInfo;
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
                MTRElectricalPowerMeasurementClusterMeasurementAccuracyStruct * newElement_0;
                newElement_0 = [MTRElectricalPowerMeasurementClusterMeasurementAccuracyStruct new];
                newElement_0.measurementType = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.measurementType)];
                newElement_0.measured = [NSNumber numberWithBool:entry_0.measured];
                newElement_0.minMeasuredValue = [NSNumber numberWithLongLong:entry_0.minMeasuredValue];
                newElement_0.maxMeasuredValue = [NSNumber numberWithLongLong:entry_0.maxMeasuredValue];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.accuracyRanges.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct * newElement_2;
                        newElement_2 = [MTRElectricalPowerMeasurementClusterMeasurementAccuracyRangeStruct new];
                        newElement_2.rangeMin = [NSNumber numberWithLongLong:entry_2.rangeMin];
                        newElement_2.rangeMax = [NSNumber numberWithLongLong:entry_2.rangeMax];
                        if (entry_2.percentMax.HasValue()) {
                            newElement_2.percentMax = [NSNumber numberWithUnsignedShort:entry_2.percentMax.Value()];
                        } else {
                            newElement_2.percentMax = nil;
                        }
                        if (entry_2.percentMin.HasValue()) {
                            newElement_2.percentMin = [NSNumber numberWithUnsignedShort:entry_2.percentMin.Value()];
                        } else {
                            newElement_2.percentMin = nil;
                        }
                        if (entry_2.percentTypical.HasValue()) {
                            newElement_2.percentTypical = [NSNumber numberWithUnsignedShort:entry_2.percentTypical.Value()];
                        } else {
                            newElement_2.percentTypical = nil;
                        }
                        if (entry_2.fixedMax.HasValue()) {
                            newElement_2.fixedMax = [NSNumber numberWithUnsignedLongLong:entry_2.fixedMax.Value()];
                        } else {
                            newElement_2.fixedMax = nil;
                        }
                        if (entry_2.fixedMin.HasValue()) {
                            newElement_2.fixedMin = [NSNumber numberWithUnsignedLongLong:entry_2.fixedMin.Value()];
                        } else {
                            newElement_2.fixedMin = nil;
                        }
                        if (entry_2.fixedTypical.HasValue()) {
                            newElement_2.fixedTypical = [NSNumber numberWithUnsignedLongLong:entry_2.fixedTypical.Value()];
                        } else {
                            newElement_2.fixedTypical = nil;
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.accuracyRanges = array_2;
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
    case Attributes::Ranges::Id: {
        using TypeInfo = Attributes::Ranges::TypeInfo;
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
                MTRElectricalPowerMeasurementClusterMeasurementRangeStruct * newElement_0;
                newElement_0 = [MTRElectricalPowerMeasurementClusterMeasurementRangeStruct new];
                newElement_0.measurementType = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_0.measurementType)];
                newElement_0.min = [NSNumber numberWithLongLong:entry_0.min];
                newElement_0.max = [NSNumber numberWithLongLong:entry_0.max];
                if (entry_0.startTimestamp.HasValue()) {
                    newElement_0.startTimestamp = [NSNumber numberWithUnsignedInt:entry_0.startTimestamp.Value()];
                } else {
                    newElement_0.startTimestamp = nil;
                }
                if (entry_0.endTimestamp.HasValue()) {
                    newElement_0.endTimestamp = [NSNumber numberWithUnsignedInt:entry_0.endTimestamp.Value()];
                } else {
                    newElement_0.endTimestamp = nil;
                }
                if (entry_0.minTimestamp.HasValue()) {
                    newElement_0.minTimestamp = [NSNumber numberWithUnsignedInt:entry_0.minTimestamp.Value()];
                } else {
                    newElement_0.minTimestamp = nil;
                }
                if (entry_0.maxTimestamp.HasValue()) {
                    newElement_0.maxTimestamp = [NSNumber numberWithUnsignedInt:entry_0.maxTimestamp.Value()];
                } else {
                    newElement_0.maxTimestamp = nil;
                }
                if (entry_0.startSystime.HasValue()) {
                    newElement_0.startSystime = [NSNumber numberWithUnsignedLongLong:entry_0.startSystime.Value()];
                } else {
                    newElement_0.startSystime = nil;
                }
                if (entry_0.endSystime.HasValue()) {
                    newElement_0.endSystime = [NSNumber numberWithUnsignedLongLong:entry_0.endSystime.Value()];
                } else {
                    newElement_0.endSystime = nil;
                }
                if (entry_0.minSystime.HasValue()) {
                    newElement_0.minSystime = [NSNumber numberWithUnsignedLongLong:entry_0.minSystime.Value()];
                } else {
                    newElement_0.minSystime = nil;
                }
                if (entry_0.maxSystime.HasValue()) {
                    newElement_0.maxSystime = [NSNumber numberWithUnsignedLongLong:entry_0.maxSystime.Value()];
                } else {
                    newElement_0.maxSystime = nil;
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
    case Attributes::Voltage::Id: {
        using TypeInfo = Attributes::Voltage::TypeInfo;
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
    case Attributes::ActiveCurrent::Id: {
        using TypeInfo = Attributes::ActiveCurrent::TypeInfo;
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
    case Attributes::ReactiveCurrent::Id: {
        using TypeInfo = Attributes::ReactiveCurrent::TypeInfo;
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
    case Attributes::ApparentCurrent::Id: {
        using TypeInfo = Attributes::ApparentCurrent::TypeInfo;
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
    case Attributes::ActivePower::Id: {
        using TypeInfo = Attributes::ActivePower::TypeInfo;
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
    case Attributes::ReactivePower::Id: {
        using TypeInfo = Attributes::ReactivePower::TypeInfo;
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
    case Attributes::ApparentPower::Id: {
        using TypeInfo = Attributes::ApparentPower::TypeInfo;
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
    case Attributes::RMSVoltage::Id: {
        using TypeInfo = Attributes::RMSVoltage::TypeInfo;
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
    case Attributes::RMSCurrent::Id: {
        using TypeInfo = Attributes::RMSCurrent::TypeInfo;
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
    case Attributes::RMSPower::Id: {
        using TypeInfo = Attributes::RMSPower::TypeInfo;
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
    case Attributes::Frequency::Id: {
        using TypeInfo = Attributes::Frequency::TypeInfo;
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
    case Attributes::HarmonicCurrents::Id: {
        using TypeInfo = Attributes::HarmonicCurrents::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct * newElement_1;
                    newElement_1 = [MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct new];
                    newElement_1.order = [NSNumber numberWithUnsignedChar:entry_1.order];
                    if (entry_1.measurement.IsNull()) {
                        newElement_1.measurement = nil;
                    } else {
                        newElement_1.measurement = [NSNumber numberWithLongLong:entry_1.measurement.Value()];
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    case Attributes::HarmonicPhases::Id: {
        using TypeInfo = Attributes::HarmonicPhases::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct * newElement_1;
                    newElement_1 = [MTRElectricalPowerMeasurementClusterHarmonicMeasurementStruct new];
                    newElement_1.order = [NSNumber numberWithUnsignedChar:entry_1.order];
                    if (entry_1.measurement.IsNull()) {
                        newElement_1.measurement = nil;
                    } else {
                        newElement_1.measurement = [NSNumber numberWithLongLong:entry_1.measurement.Value()];
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    case Attributes::PowerFactor::Id: {
        using TypeInfo = Attributes::PowerFactor::TypeInfo;
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
    case Attributes::NeutralCurrent::Id: {
        using TypeInfo = Attributes::NeutralCurrent::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForElectricalEnergyMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ElectricalEnergyMeasurement;
    switch (aAttributeId) {
    case Attributes::Accuracy::Id: {
        using TypeInfo = Attributes::Accuracy::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRElectricalEnergyMeasurementClusterMeasurementAccuracyStruct * _Nonnull value;
        value = [MTRElectricalEnergyMeasurementClusterMeasurementAccuracyStruct new];
        value.measurementType = [NSNumber numberWithUnsignedShort:chip::to_underlying(cppValue.measurementType)];
        value.measured = [NSNumber numberWithBool:cppValue.measured];
        value.minMeasuredValue = [NSNumber numberWithLongLong:cppValue.minMeasuredValue];
        value.maxMeasuredValue = [NSNumber numberWithLongLong:cppValue.maxMeasuredValue];
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.accuracyRanges.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                MTRElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct * newElement_1;
                newElement_1 = [MTRElectricalEnergyMeasurementClusterMeasurementAccuracyRangeStruct new];
                newElement_1.rangeMin = [NSNumber numberWithLongLong:entry_1.rangeMin];
                newElement_1.rangeMax = [NSNumber numberWithLongLong:entry_1.rangeMax];
                if (entry_1.percentMax.HasValue()) {
                    newElement_1.percentMax = [NSNumber numberWithUnsignedShort:entry_1.percentMax.Value()];
                } else {
                    newElement_1.percentMax = nil;
                }
                if (entry_1.percentMin.HasValue()) {
                    newElement_1.percentMin = [NSNumber numberWithUnsignedShort:entry_1.percentMin.Value()];
                } else {
                    newElement_1.percentMin = nil;
                }
                if (entry_1.percentTypical.HasValue()) {
                    newElement_1.percentTypical = [NSNumber numberWithUnsignedShort:entry_1.percentTypical.Value()];
                } else {
                    newElement_1.percentTypical = nil;
                }
                if (entry_1.fixedMax.HasValue()) {
                    newElement_1.fixedMax = [NSNumber numberWithUnsignedLongLong:entry_1.fixedMax.Value()];
                } else {
                    newElement_1.fixedMax = nil;
                }
                if (entry_1.fixedMin.HasValue()) {
                    newElement_1.fixedMin = [NSNumber numberWithUnsignedLongLong:entry_1.fixedMin.Value()];
                } else {
                    newElement_1.fixedMin = nil;
                }
                if (entry_1.fixedTypical.HasValue()) {
                    newElement_1.fixedTypical = [NSNumber numberWithUnsignedLongLong:entry_1.fixedTypical.Value()];
                } else {
                    newElement_1.fixedTypical = nil;
                }
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.accuracyRanges = array_1;
        }
        return value;
    }
    case Attributes::CumulativeEnergyImported::Id: {
        using TypeInfo = Attributes::CumulativeEnergyImported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct new];
            value.energy = [NSNumber numberWithLongLong:cppValue.Value().energy];
            if (cppValue.Value().startTimestamp.HasValue()) {
                value.startTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().startTimestamp.Value()];
            } else {
                value.startTimestamp = nil;
            }
            if (cppValue.Value().endTimestamp.HasValue()) {
                value.endTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().endTimestamp.Value()];
            } else {
                value.endTimestamp = nil;
            }
            if (cppValue.Value().startSystime.HasValue()) {
                value.startSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().startSystime.Value()];
            } else {
                value.startSystime = nil;
            }
            if (cppValue.Value().endSystime.HasValue()) {
                value.endSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().endSystime.Value()];
            } else {
                value.endSystime = nil;
            }
        }
        return value;
    }
    case Attributes::CumulativeEnergyExported::Id: {
        using TypeInfo = Attributes::CumulativeEnergyExported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct new];
            value.energy = [NSNumber numberWithLongLong:cppValue.Value().energy];
            if (cppValue.Value().startTimestamp.HasValue()) {
                value.startTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().startTimestamp.Value()];
            } else {
                value.startTimestamp = nil;
            }
            if (cppValue.Value().endTimestamp.HasValue()) {
                value.endTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().endTimestamp.Value()];
            } else {
                value.endTimestamp = nil;
            }
            if (cppValue.Value().startSystime.HasValue()) {
                value.startSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().startSystime.Value()];
            } else {
                value.startSystime = nil;
            }
            if (cppValue.Value().endSystime.HasValue()) {
                value.endSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().endSystime.Value()];
            } else {
                value.endSystime = nil;
            }
        }
        return value;
    }
    case Attributes::PeriodicEnergyImported::Id: {
        using TypeInfo = Attributes::PeriodicEnergyImported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct new];
            value.energy = [NSNumber numberWithLongLong:cppValue.Value().energy];
            if (cppValue.Value().startTimestamp.HasValue()) {
                value.startTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().startTimestamp.Value()];
            } else {
                value.startTimestamp = nil;
            }
            if (cppValue.Value().endTimestamp.HasValue()) {
                value.endTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().endTimestamp.Value()];
            } else {
                value.endTimestamp = nil;
            }
            if (cppValue.Value().startSystime.HasValue()) {
                value.startSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().startSystime.Value()];
            } else {
                value.startSystime = nil;
            }
            if (cppValue.Value().endSystime.HasValue()) {
                value.endSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().endSystime.Value()];
            } else {
                value.endSystime = nil;
            }
        }
        return value;
    }
    case Attributes::PeriodicEnergyExported::Id: {
        using TypeInfo = Attributes::PeriodicEnergyExported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRElectricalEnergyMeasurementClusterEnergyMeasurementStruct new];
            value.energy = [NSNumber numberWithLongLong:cppValue.Value().energy];
            if (cppValue.Value().startTimestamp.HasValue()) {
                value.startTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().startTimestamp.Value()];
            } else {
                value.startTimestamp = nil;
            }
            if (cppValue.Value().endTimestamp.HasValue()) {
                value.endTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().endTimestamp.Value()];
            } else {
                value.endTimestamp = nil;
            }
            if (cppValue.Value().startSystime.HasValue()) {
                value.startSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().startSystime.Value()];
            } else {
                value.startSystime = nil;
            }
            if (cppValue.Value().endSystime.HasValue()) {
                value.endSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().endSystime.Value()];
            } else {
                value.endSystime = nil;
            }
        }
        return value;
    }
    case Attributes::CumulativeEnergyReset::Id: {
        using TypeInfo = Attributes::CumulativeEnergyReset::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRElectricalEnergyMeasurementClusterCumulativeEnergyResetStruct new];
            if (cppValue.Value().importedResetTimestamp.HasValue()) {
                if (cppValue.Value().importedResetTimestamp.Value().IsNull()) {
                    value.importedResetTimestamp = nil;
                } else {
                    value.importedResetTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().importedResetTimestamp.Value().Value()];
                }
            } else {
                value.importedResetTimestamp = nil;
            }
            if (cppValue.Value().exportedResetTimestamp.HasValue()) {
                if (cppValue.Value().exportedResetTimestamp.Value().IsNull()) {
                    value.exportedResetTimestamp = nil;
                } else {
                    value.exportedResetTimestamp = [NSNumber numberWithUnsignedInt:cppValue.Value().exportedResetTimestamp.Value().Value()];
                }
            } else {
                value.exportedResetTimestamp = nil;
            }
            if (cppValue.Value().importedResetSystime.HasValue()) {
                if (cppValue.Value().importedResetSystime.Value().IsNull()) {
                    value.importedResetSystime = nil;
                } else {
                    value.importedResetSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().importedResetSystime.Value().Value()];
                }
            } else {
                value.importedResetSystime = nil;
            }
            if (cppValue.Value().exportedResetSystime.HasValue()) {
                if (cppValue.Value().exportedResetSystime.Value().IsNull()) {
                    value.exportedResetSystime = nil;
                } else {
                    value.exportedResetSystime = [NSNumber numberWithUnsignedLongLong:cppValue.Value().exportedResetSystime.Value().Value()];
                }
            } else {
                value.exportedResetSystime = nil;
            }
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWaterHeaterManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WaterHeaterManagement;
    switch (aAttributeId) {
    case Attributes::HeaterTypes::Id: {
        using TypeInfo = Attributes::HeaterTypes::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::HeatDemand::Id: {
        using TypeInfo = Attributes::HeatDemand::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::TankVolume::Id: {
        using TypeInfo = Attributes::TankVolume::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::EstimatedHeatRequired::Id: {
        using TypeInfo = Attributes::EstimatedHeatRequired::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::TankPercentage::Id: {
        using TypeInfo = Attributes::TankPercentage::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::BoostState::Id: {
        using TypeInfo = Attributes::BoostState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDemandResponseLoadControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DemandResponseLoadControl;
    switch (aAttributeId) {
    case Attributes::LoadControlPrograms::Id: {
        using TypeInfo = Attributes::LoadControlPrograms::TypeInfo;
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
                MTRDemandResponseLoadControlClusterLoadControlProgramStruct * newElement_0;
                newElement_0 = [MTRDemandResponseLoadControlClusterLoadControlProgramStruct new];
                newElement_0.programID = AsData(entry_0.programID);
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (entry_0.enrollmentGroup.IsNull()) {
                    newElement_0.enrollmentGroup = nil;
                } else {
                    newElement_0.enrollmentGroup = [NSNumber numberWithUnsignedChar:entry_0.enrollmentGroup.Value()];
                }
                if (entry_0.randomStartMinutes.IsNull()) {
                    newElement_0.randomStartMinutes = nil;
                } else {
                    newElement_0.randomStartMinutes = [NSNumber numberWithUnsignedChar:entry_0.randomStartMinutes.Value()];
                }
                if (entry_0.randomDurationMinutes.IsNull()) {
                    newElement_0.randomDurationMinutes = nil;
                } else {
                    newElement_0.randomDurationMinutes = [NSNumber numberWithUnsignedChar:entry_0.randomDurationMinutes.Value()];
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
    case Attributes::NumberOfLoadControlPrograms::Id: {
        using TypeInfo = Attributes::NumberOfLoadControlPrograms::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::Events::Id: {
        using TypeInfo = Attributes::Events::TypeInfo;
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
                MTRDemandResponseLoadControlClusterLoadControlEventStruct * newElement_0;
                newElement_0 = [MTRDemandResponseLoadControlClusterLoadControlEventStruct new];
                newElement_0.eventID = AsData(entry_0.eventID);
                if (entry_0.programID.IsNull()) {
                    newElement_0.programID = nil;
                } else {
                    newElement_0.programID = AsData(entry_0.programID.Value());
                }
                newElement_0.control = [NSNumber numberWithUnsignedShort:entry_0.control.Raw()];
                newElement_0.deviceClass = [NSNumber numberWithUnsignedInt:entry_0.deviceClass.Raw()];
                if (entry_0.enrollmentGroup.HasValue()) {
                    newElement_0.enrollmentGroup = [NSNumber numberWithUnsignedChar:entry_0.enrollmentGroup.Value()];
                } else {
                    newElement_0.enrollmentGroup = nil;
                }
                newElement_0.criticality = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.criticality)];
                if (entry_0.startTime.IsNull()) {
                    newElement_0.startTime = nil;
                } else {
                    newElement_0.startTime = [NSNumber numberWithUnsignedInt:entry_0.startTime.Value()];
                }
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.transitions.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct * newElement_2;
                        newElement_2 = [MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct new];
                        newElement_2.duration = [NSNumber numberWithUnsignedShort:entry_2.duration];
                        newElement_2.control = [NSNumber numberWithUnsignedShort:entry_2.control.Raw()];
                        if (entry_2.temperatureControl.HasValue()) {
                            newElement_2.temperatureControl = [MTRDemandResponseLoadControlClusterTemperatureControlStruct new];
                            if (entry_2.temperatureControl.Value().coolingTempOffset.HasValue()) {
                                if (entry_2.temperatureControl.Value().coolingTempOffset.Value().IsNull()) {
                                    newElement_2.temperatureControl.coolingTempOffset = nil;
                                } else {
                                    newElement_2.temperatureControl.coolingTempOffset = [NSNumber numberWithUnsignedShort:entry_2.temperatureControl.Value().coolingTempOffset.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.coolingTempOffset = nil;
                            }
                            if (entry_2.temperatureControl.Value().heatingtTempOffset.HasValue()) {
                                if (entry_2.temperatureControl.Value().heatingtTempOffset.Value().IsNull()) {
                                    newElement_2.temperatureControl.heatingtTempOffset = nil;
                                } else {
                                    newElement_2.temperatureControl.heatingtTempOffset = [NSNumber numberWithUnsignedShort:entry_2.temperatureControl.Value().heatingtTempOffset.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.heatingtTempOffset = nil;
                            }
                            if (entry_2.temperatureControl.Value().coolingTempSetpoint.HasValue()) {
                                if (entry_2.temperatureControl.Value().coolingTempSetpoint.Value().IsNull()) {
                                    newElement_2.temperatureControl.coolingTempSetpoint = nil;
                                } else {
                                    newElement_2.temperatureControl.coolingTempSetpoint = [NSNumber numberWithShort:entry_2.temperatureControl.Value().coolingTempSetpoint.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.coolingTempSetpoint = nil;
                            }
                            if (entry_2.temperatureControl.Value().heatingTempSetpoint.HasValue()) {
                                if (entry_2.temperatureControl.Value().heatingTempSetpoint.Value().IsNull()) {
                                    newElement_2.temperatureControl.heatingTempSetpoint = nil;
                                } else {
                                    newElement_2.temperatureControl.heatingTempSetpoint = [NSNumber numberWithShort:entry_2.temperatureControl.Value().heatingTempSetpoint.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.heatingTempSetpoint = nil;
                            }
                        } else {
                            newElement_2.temperatureControl = nil;
                        }
                        if (entry_2.averageLoadControl.HasValue()) {
                            newElement_2.averageLoadControl = [MTRDemandResponseLoadControlClusterAverageLoadControlStruct new];
                            newElement_2.averageLoadControl.loadAdjustment = [NSNumber numberWithChar:entry_2.averageLoadControl.Value().loadAdjustment];
                        } else {
                            newElement_2.averageLoadControl = nil;
                        }
                        if (entry_2.dutyCycleControl.HasValue()) {
                            newElement_2.dutyCycleControl = [MTRDemandResponseLoadControlClusterDutyCycleControlStruct new];
                            newElement_2.dutyCycleControl.dutyCycle = [NSNumber numberWithUnsignedChar:entry_2.dutyCycleControl.Value().dutyCycle];
                        } else {
                            newElement_2.dutyCycleControl = nil;
                        }
                        if (entry_2.powerSavingsControl.HasValue()) {
                            newElement_2.powerSavingsControl = [MTRDemandResponseLoadControlClusterPowerSavingsControlStruct new];
                            newElement_2.powerSavingsControl.powerSavings = [NSNumber numberWithUnsignedChar:entry_2.powerSavingsControl.Value().powerSavings];
                        } else {
                            newElement_2.powerSavingsControl = nil;
                        }
                        if (entry_2.heatingSourceControl.HasValue()) {
                            newElement_2.heatingSourceControl = [MTRDemandResponseLoadControlClusterHeatingSourceControlStruct new];
                            newElement_2.heatingSourceControl.heatingSource = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.heatingSourceControl.Value().heatingSource)];
                        } else {
                            newElement_2.heatingSourceControl = nil;
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.transitions = array_2;
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
    case Attributes::ActiveEvents::Id: {
        using TypeInfo = Attributes::ActiveEvents::TypeInfo;
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
                MTRDemandResponseLoadControlClusterLoadControlEventStruct * newElement_0;
                newElement_0 = [MTRDemandResponseLoadControlClusterLoadControlEventStruct new];
                newElement_0.eventID = AsData(entry_0.eventID);
                if (entry_0.programID.IsNull()) {
                    newElement_0.programID = nil;
                } else {
                    newElement_0.programID = AsData(entry_0.programID.Value());
                }
                newElement_0.control = [NSNumber numberWithUnsignedShort:entry_0.control.Raw()];
                newElement_0.deviceClass = [NSNumber numberWithUnsignedInt:entry_0.deviceClass.Raw()];
                if (entry_0.enrollmentGroup.HasValue()) {
                    newElement_0.enrollmentGroup = [NSNumber numberWithUnsignedChar:entry_0.enrollmentGroup.Value()];
                } else {
                    newElement_0.enrollmentGroup = nil;
                }
                newElement_0.criticality = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.criticality)];
                if (entry_0.startTime.IsNull()) {
                    newElement_0.startTime = nil;
                } else {
                    newElement_0.startTime = [NSNumber numberWithUnsignedInt:entry_0.startTime.Value()];
                }
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.transitions.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct * newElement_2;
                        newElement_2 = [MTRDemandResponseLoadControlClusterLoadControlEventTransitionStruct new];
                        newElement_2.duration = [NSNumber numberWithUnsignedShort:entry_2.duration];
                        newElement_2.control = [NSNumber numberWithUnsignedShort:entry_2.control.Raw()];
                        if (entry_2.temperatureControl.HasValue()) {
                            newElement_2.temperatureControl = [MTRDemandResponseLoadControlClusterTemperatureControlStruct new];
                            if (entry_2.temperatureControl.Value().coolingTempOffset.HasValue()) {
                                if (entry_2.temperatureControl.Value().coolingTempOffset.Value().IsNull()) {
                                    newElement_2.temperatureControl.coolingTempOffset = nil;
                                } else {
                                    newElement_2.temperatureControl.coolingTempOffset = [NSNumber numberWithUnsignedShort:entry_2.temperatureControl.Value().coolingTempOffset.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.coolingTempOffset = nil;
                            }
                            if (entry_2.temperatureControl.Value().heatingtTempOffset.HasValue()) {
                                if (entry_2.temperatureControl.Value().heatingtTempOffset.Value().IsNull()) {
                                    newElement_2.temperatureControl.heatingtTempOffset = nil;
                                } else {
                                    newElement_2.temperatureControl.heatingtTempOffset = [NSNumber numberWithUnsignedShort:entry_2.temperatureControl.Value().heatingtTempOffset.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.heatingtTempOffset = nil;
                            }
                            if (entry_2.temperatureControl.Value().coolingTempSetpoint.HasValue()) {
                                if (entry_2.temperatureControl.Value().coolingTempSetpoint.Value().IsNull()) {
                                    newElement_2.temperatureControl.coolingTempSetpoint = nil;
                                } else {
                                    newElement_2.temperatureControl.coolingTempSetpoint = [NSNumber numberWithShort:entry_2.temperatureControl.Value().coolingTempSetpoint.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.coolingTempSetpoint = nil;
                            }
                            if (entry_2.temperatureControl.Value().heatingTempSetpoint.HasValue()) {
                                if (entry_2.temperatureControl.Value().heatingTempSetpoint.Value().IsNull()) {
                                    newElement_2.temperatureControl.heatingTempSetpoint = nil;
                                } else {
                                    newElement_2.temperatureControl.heatingTempSetpoint = [NSNumber numberWithShort:entry_2.temperatureControl.Value().heatingTempSetpoint.Value().Value()];
                                }
                            } else {
                                newElement_2.temperatureControl.heatingTempSetpoint = nil;
                            }
                        } else {
                            newElement_2.temperatureControl = nil;
                        }
                        if (entry_2.averageLoadControl.HasValue()) {
                            newElement_2.averageLoadControl = [MTRDemandResponseLoadControlClusterAverageLoadControlStruct new];
                            newElement_2.averageLoadControl.loadAdjustment = [NSNumber numberWithChar:entry_2.averageLoadControl.Value().loadAdjustment];
                        } else {
                            newElement_2.averageLoadControl = nil;
                        }
                        if (entry_2.dutyCycleControl.HasValue()) {
                            newElement_2.dutyCycleControl = [MTRDemandResponseLoadControlClusterDutyCycleControlStruct new];
                            newElement_2.dutyCycleControl.dutyCycle = [NSNumber numberWithUnsignedChar:entry_2.dutyCycleControl.Value().dutyCycle];
                        } else {
                            newElement_2.dutyCycleControl = nil;
                        }
                        if (entry_2.powerSavingsControl.HasValue()) {
                            newElement_2.powerSavingsControl = [MTRDemandResponseLoadControlClusterPowerSavingsControlStruct new];
                            newElement_2.powerSavingsControl.powerSavings = [NSNumber numberWithUnsignedChar:entry_2.powerSavingsControl.Value().powerSavings];
                        } else {
                            newElement_2.powerSavingsControl = nil;
                        }
                        if (entry_2.heatingSourceControl.HasValue()) {
                            newElement_2.heatingSourceControl = [MTRDemandResponseLoadControlClusterHeatingSourceControlStruct new];
                            newElement_2.heatingSourceControl.heatingSource = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.heatingSourceControl.Value().heatingSource)];
                        } else {
                            newElement_2.heatingSourceControl = nil;
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.transitions = array_2;
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
    case Attributes::NumberOfEventsPerProgram::Id: {
        using TypeInfo = Attributes::NumberOfEventsPerProgram::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::NumberOfTransitions::Id: {
        using TypeInfo = Attributes::NumberOfTransitions::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::DefaultRandomStart::Id: {
        using TypeInfo = Attributes::DefaultRandomStart::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::DefaultRandomDuration::Id: {
        using TypeInfo = Attributes::DefaultRandomDuration::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForMessagesCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Messages;
    switch (aAttributeId) {
    case Attributes::Messages::Id: {
        using TypeInfo = Attributes::Messages::TypeInfo;
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
                MTRMessagesClusterMessageStruct * newElement_0;
                newElement_0 = [MTRMessagesClusterMessageStruct new];
                newElement_0.messageID = AsData(entry_0.messageID);
                newElement_0.priority = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.priority)];
                newElement_0.messageControl = [NSNumber numberWithUnsignedChar:entry_0.messageControl.Raw()];
                if (entry_0.startTime.IsNull()) {
                    newElement_0.startTime = nil;
                } else {
                    newElement_0.startTime = [NSNumber numberWithUnsignedInt:entry_0.startTime.Value()];
                }
                if (entry_0.duration.IsNull()) {
                    newElement_0.duration = nil;
                } else {
                    newElement_0.duration = [NSNumber numberWithUnsignedLongLong:entry_0.duration.Value()];
                }
                newElement_0.messageText = AsString(entry_0.messageText);
                if (newElement_0.messageText == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (entry_0.responses.HasValue()) {
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = entry_0.responses.Value().begin();
                        while (iter_3.Next()) {
                            auto & entry_3 = iter_3.GetValue();
                            MTRMessagesClusterMessageResponseOptionStruct * newElement_3;
                            newElement_3 = [MTRMessagesClusterMessageResponseOptionStruct new];
                            if (entry_3.messageResponseID.HasValue()) {
                                newElement_3.messageResponseID = [NSNumber numberWithUnsignedInt:entry_3.messageResponseID.Value()];
                            } else {
                                newElement_3.messageResponseID = nil;
                            }
                            if (entry_3.label.HasValue()) {
                                newElement_3.label = AsString(entry_3.label.Value());
                                if (newElement_3.label == nil) {
                                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                                    *aError = err;
                                    return nil;
                                }
                            } else {
                                newElement_3.label = nil;
                            }
                            [array_3 addObject:newElement_3];
                        }
                        CHIP_ERROR err = iter_3.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.responses = array_3;
                    }
                } else {
                    newElement_0.responses = nil;
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
    case Attributes::ActiveMessageIDs::Id: {
        using TypeInfo = Attributes::ActiveMessageIDs::TypeInfo;
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
                newElement_0 = AsData(entry_0);
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDeviceEnergyManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DeviceEnergyManagement;
    switch (aAttributeId) {
    case Attributes::ESAType::Id: {
        using TypeInfo = Attributes::ESAType::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::ESACanGenerate::Id: {
        using TypeInfo = Attributes::ESACanGenerate::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::ESAState::Id: {
        using TypeInfo = Attributes::ESAState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::AbsMinPower::Id: {
        using TypeInfo = Attributes::AbsMinPower::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::AbsMaxPower::Id: {
        using TypeInfo = Attributes::AbsMaxPower::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::PowerAdjustmentCapability::Id: {
        using TypeInfo = Attributes::PowerAdjustmentCapability::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRDeviceEnergyManagementClusterPowerAdjustCapabilityStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRDeviceEnergyManagementClusterPowerAdjustCapabilityStruct new];
            if (cppValue.Value().powerAdjustCapability.IsNull()) {
                value.powerAdjustCapability = nil;
            } else {
                { // Scope for our temporary variables
                    auto * array_3 = [NSMutableArray new];
                    auto iter_3 = cppValue.Value().powerAdjustCapability.Value().begin();
                    while (iter_3.Next()) {
                        auto & entry_3 = iter_3.GetValue();
                        MTRDeviceEnergyManagementClusterPowerAdjustStruct * newElement_3;
                        newElement_3 = [MTRDeviceEnergyManagementClusterPowerAdjustStruct new];
                        newElement_3.minPower = [NSNumber numberWithLongLong:entry_3.minPower];
                        newElement_3.maxPower = [NSNumber numberWithLongLong:entry_3.maxPower];
                        newElement_3.minDuration = [NSNumber numberWithUnsignedInt:entry_3.minDuration];
                        newElement_3.maxDuration = [NSNumber numberWithUnsignedInt:entry_3.maxDuration];
                        [array_3 addObject:newElement_3];
                    }
                    CHIP_ERROR err = iter_3.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    value.powerAdjustCapability = array_3;
                }
            }
            value.cause = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().cause)];
        }
        return value;
    }
    case Attributes::Forecast::Id: {
        using TypeInfo = Attributes::Forecast::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRDeviceEnergyManagementClusterForecastStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRDeviceEnergyManagementClusterForecastStruct new];
            value.forecastID = [NSNumber numberWithUnsignedInt:cppValue.Value().forecastID];
            if (cppValue.Value().activeSlotNumber.IsNull()) {
                value.activeSlotNumber = nil;
            } else {
                value.activeSlotNumber = [NSNumber numberWithUnsignedShort:cppValue.Value().activeSlotNumber.Value()];
            }
            value.startTime = [NSNumber numberWithUnsignedInt:cppValue.Value().startTime];
            value.endTime = [NSNumber numberWithUnsignedInt:cppValue.Value().endTime];
            if (cppValue.Value().earliestStartTime.HasValue()) {
                if (cppValue.Value().earliestStartTime.Value().IsNull()) {
                    value.earliestStartTime = nil;
                } else {
                    value.earliestStartTime = [NSNumber numberWithUnsignedInt:cppValue.Value().earliestStartTime.Value().Value()];
                }
            } else {
                value.earliestStartTime = nil;
            }
            if (cppValue.Value().latestEndTime.HasValue()) {
                value.latestEndTime = [NSNumber numberWithUnsignedInt:cppValue.Value().latestEndTime.Value()];
            } else {
                value.latestEndTime = nil;
            }
            value.isPausable = [NSNumber numberWithBool:cppValue.Value().isPausable];
            { // Scope for our temporary variables
                auto * array_2 = [NSMutableArray new];
                auto iter_2 = cppValue.Value().slots.begin();
                while (iter_2.Next()) {
                    auto & entry_2 = iter_2.GetValue();
                    MTRDeviceEnergyManagementClusterSlotStruct * newElement_2;
                    newElement_2 = [MTRDeviceEnergyManagementClusterSlotStruct new];
                    newElement_2.minDuration = [NSNumber numberWithUnsignedInt:entry_2.minDuration];
                    newElement_2.maxDuration = [NSNumber numberWithUnsignedInt:entry_2.maxDuration];
                    newElement_2.defaultDuration = [NSNumber numberWithUnsignedInt:entry_2.defaultDuration];
                    newElement_2.elapsedSlotTime = [NSNumber numberWithUnsignedInt:entry_2.elapsedSlotTime];
                    newElement_2.remainingSlotTime = [NSNumber numberWithUnsignedInt:entry_2.remainingSlotTime];
                    if (entry_2.slotIsPausable.HasValue()) {
                        newElement_2.slotIsPausable = [NSNumber numberWithBool:entry_2.slotIsPausable.Value()];
                    } else {
                        newElement_2.slotIsPausable = nil;
                    }
                    if (entry_2.minPauseDuration.HasValue()) {
                        newElement_2.minPauseDuration = [NSNumber numberWithUnsignedInt:entry_2.minPauseDuration.Value()];
                    } else {
                        newElement_2.minPauseDuration = nil;
                    }
                    if (entry_2.maxPauseDuration.HasValue()) {
                        newElement_2.maxPauseDuration = [NSNumber numberWithUnsignedInt:entry_2.maxPauseDuration.Value()];
                    } else {
                        newElement_2.maxPauseDuration = nil;
                    }
                    if (entry_2.manufacturerESAState.HasValue()) {
                        newElement_2.manufacturerESAState = [NSNumber numberWithUnsignedShort:entry_2.manufacturerESAState.Value()];
                    } else {
                        newElement_2.manufacturerESAState = nil;
                    }
                    if (entry_2.nominalPower.HasValue()) {
                        newElement_2.nominalPower = [NSNumber numberWithLongLong:entry_2.nominalPower.Value()];
                    } else {
                        newElement_2.nominalPower = nil;
                    }
                    if (entry_2.minPower.HasValue()) {
                        newElement_2.minPower = [NSNumber numberWithLongLong:entry_2.minPower.Value()];
                    } else {
                        newElement_2.minPower = nil;
                    }
                    if (entry_2.maxPower.HasValue()) {
                        newElement_2.maxPower = [NSNumber numberWithLongLong:entry_2.maxPower.Value()];
                    } else {
                        newElement_2.maxPower = nil;
                    }
                    if (entry_2.nominalEnergy.HasValue()) {
                        newElement_2.nominalEnergy = [NSNumber numberWithLongLong:entry_2.nominalEnergy.Value()];
                    } else {
                        newElement_2.nominalEnergy = nil;
                    }
                    if (entry_2.costs.HasValue()) {
                        { // Scope for our temporary variables
                            auto * array_5 = [NSMutableArray new];
                            auto iter_5 = entry_2.costs.Value().begin();
                            while (iter_5.Next()) {
                                auto & entry_5 = iter_5.GetValue();
                                MTRDeviceEnergyManagementClusterCostStruct * newElement_5;
                                newElement_5 = [MTRDeviceEnergyManagementClusterCostStruct new];
                                newElement_5.costType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_5.costType)];
                                newElement_5.value = [NSNumber numberWithInt:entry_5.value];
                                newElement_5.decimalPoints = [NSNumber numberWithUnsignedChar:entry_5.decimalPoints];
                                if (entry_5.currency.HasValue()) {
                                    newElement_5.currency = [NSNumber numberWithUnsignedShort:entry_5.currency.Value()];
                                } else {
                                    newElement_5.currency = nil;
                                }
                                [array_5 addObject:newElement_5];
                            }
                            CHIP_ERROR err = iter_5.GetStatus();
                            if (err != CHIP_NO_ERROR) {
                                *aError = err;
                                return nil;
                            }
                            newElement_2.costs = array_5;
                        }
                    } else {
                        newElement_2.costs = nil;
                    }
                    if (entry_2.minPowerAdjustment.HasValue()) {
                        newElement_2.minPowerAdjustment = [NSNumber numberWithLongLong:entry_2.minPowerAdjustment.Value()];
                    } else {
                        newElement_2.minPowerAdjustment = nil;
                    }
                    if (entry_2.maxPowerAdjustment.HasValue()) {
                        newElement_2.maxPowerAdjustment = [NSNumber numberWithLongLong:entry_2.maxPowerAdjustment.Value()];
                    } else {
                        newElement_2.maxPowerAdjustment = nil;
                    }
                    if (entry_2.minDurationAdjustment.HasValue()) {
                        newElement_2.minDurationAdjustment = [NSNumber numberWithUnsignedInt:entry_2.minDurationAdjustment.Value()];
                    } else {
                        newElement_2.minDurationAdjustment = nil;
                    }
                    if (entry_2.maxDurationAdjustment.HasValue()) {
                        newElement_2.maxDurationAdjustment = [NSNumber numberWithUnsignedInt:entry_2.maxDurationAdjustment.Value()];
                    } else {
                        newElement_2.maxDurationAdjustment = nil;
                    }
                    [array_2 addObject:newElement_2];
                }
                CHIP_ERROR err = iter_2.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value.slots = array_2;
            }
            value.forecastUpdateReason = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().forecastUpdateReason)];
        }
        return value;
    }
    case Attributes::OptOutState::Id: {
        using TypeInfo = Attributes::OptOutState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForEnergyEVSECluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EnergyEvse;
    switch (aAttributeId) {
    case Attributes::State::Id: {
        using TypeInfo = Attributes::State::TypeInfo;
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
    case Attributes::SupplyState::Id: {
        using TypeInfo = Attributes::SupplyState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::FaultState::Id: {
        using TypeInfo = Attributes::FaultState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::ChargingEnabledUntil::Id: {
        using TypeInfo = Attributes::ChargingEnabledUntil::TypeInfo;
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
    case Attributes::DischargingEnabledUntil::Id: {
        using TypeInfo = Attributes::DischargingEnabledUntil::TypeInfo;
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
    case Attributes::CircuitCapacity::Id: {
        using TypeInfo = Attributes::CircuitCapacity::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::MinimumChargeCurrent::Id: {
        using TypeInfo = Attributes::MinimumChargeCurrent::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::MaximumChargeCurrent::Id: {
        using TypeInfo = Attributes::MaximumChargeCurrent::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::MaximumDischargeCurrent::Id: {
        using TypeInfo = Attributes::MaximumDischargeCurrent::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::UserMaximumChargeCurrent::Id: {
        using TypeInfo = Attributes::UserMaximumChargeCurrent::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithLongLong:cppValue];
        return value;
    }
    case Attributes::RandomizationDelayWindow::Id: {
        using TypeInfo = Attributes::RandomizationDelayWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::NextChargeStartTime::Id: {
        using TypeInfo = Attributes::NextChargeStartTime::TypeInfo;
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
    case Attributes::NextChargeTargetTime::Id: {
        using TypeInfo = Attributes::NextChargeTargetTime::TypeInfo;
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
    case Attributes::NextChargeRequiredEnergy::Id: {
        using TypeInfo = Attributes::NextChargeRequiredEnergy::TypeInfo;
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
    case Attributes::NextChargeTargetSoC::Id: {
        using TypeInfo = Attributes::NextChargeTargetSoC::TypeInfo;
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
    case Attributes::ApproximateEVEfficiency::Id: {
        using TypeInfo = Attributes::ApproximateEVEfficiency::TypeInfo;
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
    case Attributes::StateOfCharge::Id: {
        using TypeInfo = Attributes::StateOfCharge::TypeInfo;
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
    case Attributes::BatteryCapacity::Id: {
        using TypeInfo = Attributes::BatteryCapacity::TypeInfo;
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
    case Attributes::VehicleID::Id: {
        using TypeInfo = Attributes::VehicleID::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsString(cppValue.Value());
            if (value == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
        }
        return value;
    }
    case Attributes::SessionID::Id: {
        using TypeInfo = Attributes::SessionID::TypeInfo;
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
    case Attributes::SessionDuration::Id: {
        using TypeInfo = Attributes::SessionDuration::TypeInfo;
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
    case Attributes::SessionEnergyCharged::Id: {
        using TypeInfo = Attributes::SessionEnergyCharged::TypeInfo;
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
    case Attributes::SessionEnergyDischarged::Id: {
        using TypeInfo = Attributes::SessionEnergyDischarged::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForEnergyPreferenceCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EnergyPreference;
    switch (aAttributeId) {
    case Attributes::EnergyBalances::Id: {
        using TypeInfo = Attributes::EnergyBalances::TypeInfo;
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
                MTREnergyPreferenceClusterBalanceStruct * newElement_0;
                newElement_0 = [MTREnergyPreferenceClusterBalanceStruct new];
                newElement_0.step = [NSNumber numberWithUnsignedChar:entry_0.step];
                if (entry_0.label.HasValue()) {
                    newElement_0.label = AsString(entry_0.label.Value());
                    if (newElement_0.label == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.label = nil;
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
    case Attributes::CurrentEnergyBalance::Id: {
        using TypeInfo = Attributes::CurrentEnergyBalance::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::EnergyPriorities::Id: {
        using TypeInfo = Attributes::EnergyPriorities::TypeInfo;
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
    case Attributes::LowPowerModeSensitivities::Id: {
        using TypeInfo = Attributes::LowPowerModeSensitivities::TypeInfo;
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
                MTREnergyPreferenceClusterBalanceStruct * newElement_0;
                newElement_0 = [MTREnergyPreferenceClusterBalanceStruct new];
                newElement_0.step = [NSNumber numberWithUnsignedChar:entry_0.step];
                if (entry_0.label.HasValue()) {
                    newElement_0.label = AsString(entry_0.label.Value());
                    if (newElement_0.label == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.label = nil;
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
    case Attributes::CurrentLowPowerModeSensitivity::Id: {
        using TypeInfo = Attributes::CurrentLowPowerModeSensitivity::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPowerTopologyCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PowerTopology;
    switch (aAttributeId) {
    case Attributes::AvailableEndpoints::Id: {
        using TypeInfo = Attributes::AvailableEndpoints::TypeInfo;
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
    case Attributes::ActiveEndpoints::Id: {
        using TypeInfo = Attributes::ActiveEndpoints::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForEnergyEVSEModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EnergyEvseMode;
    switch (aAttributeId) {
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
                MTREnergyEVSEModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTREnergyEVSEModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTREnergyEVSEModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTREnergyEVSEModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWaterHeaterModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WaterHeaterMode;
    switch (aAttributeId) {
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
                MTRWaterHeaterModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRWaterHeaterModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRWaterHeaterModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRWaterHeaterModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDeviceEnergyManagementModeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DeviceEnergyManagementMode;
    switch (aAttributeId) {
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
                MTRDeviceEnergyManagementModeClusterModeOptionStruct * newElement_0;
                newElement_0 = [MTRDeviceEnergyManagementModeClusterModeOptionStruct new];
                newElement_0.label = AsString(entry_0.label);
                if (newElement_0.label == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.mode = [NSNumber numberWithUnsignedChar:entry_0.mode];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.modeTags.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRDeviceEnergyManagementModeClusterModeTagStruct * newElement_2;
                        newElement_2 = [MTRDeviceEnergyManagementModeClusterModeTagStruct new];
                        if (entry_2.mfgCode.HasValue()) {
                            newElement_2.mfgCode = [NSNumber numberWithUnsignedShort:chip::to_underlying(entry_2.mfgCode.Value())];
                        } else {
                            newElement_2.mfgCode = nil;
                        }
                        newElement_2.value = [NSNumber numberWithUnsignedShort:entry_2.value];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.modeTags = array_2;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForDoorLockCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DoorLock;
    switch (aAttributeId) {
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
    case Attributes::AliroReaderVerificationKey::Id: {
        using TypeInfo = Attributes::AliroReaderVerificationKey::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::AliroReaderGroupIdentifier::Id: {
        using TypeInfo = Attributes::AliroReaderGroupIdentifier::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::AliroReaderGroupSubIdentifier::Id: {
        using TypeInfo = Attributes::AliroReaderGroupSubIdentifier::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nonnull value;
        value = AsData(cppValue);
        return value;
    }
    case Attributes::AliroExpeditedTransactionSupportedProtocolVersions::Id: {
        using TypeInfo = Attributes::AliroExpeditedTransactionSupportedProtocolVersions::TypeInfo;
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
                newElement_0 = AsData(entry_0);
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
    case Attributes::AliroGroupResolvingKey::Id: {
        using TypeInfo = Attributes::AliroGroupResolvingKey::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::AliroSupportedBLEUWBProtocolVersions::Id: {
        using TypeInfo = Attributes::AliroSupportedBLEUWBProtocolVersions::TypeInfo;
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
                newElement_0 = AsData(entry_0);
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
    case Attributes::AliroBLEAdvertisingVersion::Id: {
        using TypeInfo = Attributes::AliroBLEAdvertisingVersion::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::NumberOfAliroCredentialIssuerKeysSupported::Id: {
        using TypeInfo = Attributes::NumberOfAliroCredentialIssuerKeysSupported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::NumberOfAliroEndpointKeysSupported::Id: {
        using TypeInfo = Attributes::NumberOfAliroEndpointKeysSupported::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWindowCoveringCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WindowCovering;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForClosureControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ClosureControl;
    switch (aAttributeId) {
    case Attributes::CountdownTime::Id: {
        using TypeInfo = Attributes::CountdownTime::TypeInfo;
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
    case Attributes::MainState::Id: {
        using TypeInfo = Attributes::MainState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::CurrentErrorList::Id: {
        using TypeInfo = Attributes::CurrentErrorList::TypeInfo;
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
    case Attributes::OverallState::Id: {
        using TypeInfo = Attributes::OverallState::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRClosureControlClusterOverallStateStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRClosureControlClusterOverallStateStruct new];
            if (cppValue.Value().positioning.HasValue()) {
                value.positioning = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().positioning.Value())];
            } else {
                value.positioning = nil;
            }
            if (cppValue.Value().latching.HasValue()) {
                value.latching = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().latching.Value())];
            } else {
                value.latching = nil;
            }
            if (cppValue.Value().speed.HasValue()) {
                value.speed = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().speed.Value())];
            } else {
                value.speed = nil;
            }
            if (cppValue.Value().extraInfo.HasValue()) {
                value.extraInfo = [NSNumber numberWithUnsignedInt:cppValue.Value().extraInfo.Value()];
            } else {
                value.extraInfo = nil;
            }
        }
        return value;
    }
    case Attributes::OverallTarget::Id: {
        using TypeInfo = Attributes::OverallTarget::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRClosureControlClusterOverallTargetStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRClosureControlClusterOverallTargetStruct new];
            if (cppValue.Value().tagPosition.HasValue()) {
                value.tagPosition = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().tagPosition.Value())];
            } else {
                value.tagPosition = nil;
            }
            if (cppValue.Value().tagLatch.HasValue()) {
                value.tagLatch = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().tagLatch.Value())];
            } else {
                value.tagLatch = nil;
            }
            if (cppValue.Value().speed.HasValue()) {
                value.speed = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().speed.Value())];
            } else {
                value.speed = nil;
            }
        }
        return value;
    }
    case Attributes::RestingProcedure::Id: {
        using TypeInfo = Attributes::RestingProcedure::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::TriggerCondition::Id: {
        using TypeInfo = Attributes::TriggerCondition::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::TriggerPosition::Id: {
        using TypeInfo = Attributes::TriggerPosition::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::WaitingDelay::Id: {
        using TypeInfo = Attributes::WaitingDelay::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::KickoffTimer::Id: {
        using TypeInfo = Attributes::KickoffTimer::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForServiceAreaCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ServiceArea;
    switch (aAttributeId) {
    case Attributes::SupportedAreas::Id: {
        using TypeInfo = Attributes::SupportedAreas::TypeInfo;
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
                MTRServiceAreaClusterAreaStruct * newElement_0;
                newElement_0 = [MTRServiceAreaClusterAreaStruct new];
                newElement_0.areaID = [NSNumber numberWithUnsignedInt:entry_0.areaID];
                if (entry_0.mapID.IsNull()) {
                    newElement_0.mapID = nil;
                } else {
                    newElement_0.mapID = [NSNumber numberWithUnsignedInt:entry_0.mapID.Value()];
                }
                newElement_0.areaInfo = [MTRServiceAreaClusterAreaInfoStruct new];
                if (entry_0.areaInfo.locationInfo.IsNull()) {
                    newElement_0.areaInfo.locationInfo = nil;
                } else {
                    newElement_0.areaInfo.locationInfo = [MTRDataTypeLocationDescriptorStruct new];
                    newElement_0.areaInfo.locationInfo.locationName = AsString(entry_0.areaInfo.locationInfo.Value().locationName);
                    if (newElement_0.areaInfo.locationInfo.locationName == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    if (entry_0.areaInfo.locationInfo.Value().floorNumber.IsNull()) {
                        newElement_0.areaInfo.locationInfo.floorNumber = nil;
                    } else {
                        newElement_0.areaInfo.locationInfo.floorNumber = [NSNumber numberWithShort:entry_0.areaInfo.locationInfo.Value().floorNumber.Value()];
                    }
                    if (entry_0.areaInfo.locationInfo.Value().areaType.IsNull()) {
                        newElement_0.areaInfo.locationInfo.areaType = nil;
                    } else {
                        newElement_0.areaInfo.locationInfo.areaType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.areaInfo.locationInfo.Value().areaType.Value())];
                    }
                }
                if (entry_0.areaInfo.landmarkInfo.IsNull()) {
                    newElement_0.areaInfo.landmarkInfo = nil;
                } else {
                    newElement_0.areaInfo.landmarkInfo = [MTRServiceAreaClusterLandmarkInfoStruct new];
                    newElement_0.areaInfo.landmarkInfo.landmarkTag = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.areaInfo.landmarkInfo.Value().landmarkTag)];
                    if (entry_0.areaInfo.landmarkInfo.Value().relativePositionTag.IsNull()) {
                        newElement_0.areaInfo.landmarkInfo.relativePositionTag = nil;
                    } else {
                        newElement_0.areaInfo.landmarkInfo.relativePositionTag = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.areaInfo.landmarkInfo.Value().relativePositionTag.Value())];
                    }
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
    case Attributes::SupportedMaps::Id: {
        using TypeInfo = Attributes::SupportedMaps::TypeInfo;
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
                MTRServiceAreaClusterMapStruct * newElement_0;
                newElement_0 = [MTRServiceAreaClusterMapStruct new];
                newElement_0.mapID = [NSNumber numberWithUnsignedInt:entry_0.mapID];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    case Attributes::SelectedAreas::Id: {
        using TypeInfo = Attributes::SelectedAreas::TypeInfo;
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
    case Attributes::CurrentArea::Id: {
        using TypeInfo = Attributes::CurrentArea::TypeInfo;
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
    case Attributes::EstimatedEndTime::Id: {
        using TypeInfo = Attributes::EstimatedEndTime::TypeInfo;
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
    case Attributes::Progress::Id: {
        using TypeInfo = Attributes::Progress::TypeInfo;
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
                MTRServiceAreaClusterProgressStruct * newElement_0;
                newElement_0 = [MTRServiceAreaClusterProgressStruct new];
                newElement_0.areaID = [NSNumber numberWithUnsignedInt:entry_0.areaID];
                newElement_0.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.status)];
                if (entry_0.totalOperationalTime.HasValue()) {
                    if (entry_0.totalOperationalTime.Value().IsNull()) {
                        newElement_0.totalOperationalTime = nil;
                    } else {
                        newElement_0.totalOperationalTime = [NSNumber numberWithUnsignedInt:entry_0.totalOperationalTime.Value().Value()];
                    }
                } else {
                    newElement_0.totalOperationalTime = nil;
                }
                if (entry_0.estimatedTime.HasValue()) {
                    if (entry_0.estimatedTime.Value().IsNull()) {
                        newElement_0.estimatedTime = nil;
                    } else {
                        newElement_0.estimatedTime = [NSNumber numberWithUnsignedInt:entry_0.estimatedTime.Value().Value()];
                    }
                } else {
                    newElement_0.estimatedTime = nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPumpConfigurationAndControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PumpConfigurationAndControl;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForThermostatCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Thermostat;
    switch (aAttributeId) {
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::PresetTypes::Id: {
        using TypeInfo = Attributes::PresetTypes::TypeInfo;
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
                MTRThermostatClusterPresetTypeStruct * newElement_0;
                newElement_0 = [MTRThermostatClusterPresetTypeStruct new];
                newElement_0.presetScenario = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.presetScenario)];
                newElement_0.numberOfPresets = [NSNumber numberWithUnsignedChar:entry_0.numberOfPresets];
                newElement_0.presetTypeFeatures = [NSNumber numberWithUnsignedShort:entry_0.presetTypeFeatures.Raw()];
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
    case Attributes::ScheduleTypes::Id: {
        using TypeInfo = Attributes::ScheduleTypes::TypeInfo;
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
                MTRThermostatClusterScheduleTypeStruct * newElement_0;
                newElement_0 = [MTRThermostatClusterScheduleTypeStruct new];
                newElement_0.systemMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.systemMode)];
                newElement_0.numberOfSchedules = [NSNumber numberWithUnsignedChar:entry_0.numberOfSchedules];
                newElement_0.scheduleTypeFeatures = [NSNumber numberWithUnsignedShort:entry_0.scheduleTypeFeatures.Raw()];
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
    case Attributes::NumberOfPresets::Id: {
        using TypeInfo = Attributes::NumberOfPresets::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::NumberOfSchedules::Id: {
        using TypeInfo = Attributes::NumberOfSchedules::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::NumberOfScheduleTransitions::Id: {
        using TypeInfo = Attributes::NumberOfScheduleTransitions::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::NumberOfScheduleTransitionPerDay::Id: {
        using TypeInfo = Attributes::NumberOfScheduleTransitionPerDay::TypeInfo;
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
    case Attributes::ActivePresetHandle::Id: {
        using TypeInfo = Attributes::ActivePresetHandle::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::ActiveScheduleHandle::Id: {
        using TypeInfo = Attributes::ActiveScheduleHandle::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::Presets::Id: {
        using TypeInfo = Attributes::Presets::TypeInfo;
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
                MTRThermostatClusterPresetStruct * newElement_0;
                newElement_0 = [MTRThermostatClusterPresetStruct new];
                if (entry_0.presetHandle.IsNull()) {
                    newElement_0.presetHandle = nil;
                } else {
                    newElement_0.presetHandle = AsData(entry_0.presetHandle.Value());
                }
                newElement_0.presetScenario = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.presetScenario)];
                if (entry_0.name.HasValue()) {
                    if (entry_0.name.Value().IsNull()) {
                        newElement_0.name = nil;
                    } else {
                        newElement_0.name = AsString(entry_0.name.Value().Value());
                        if (newElement_0.name == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                    }
                } else {
                    newElement_0.name = nil;
                }
                if (entry_0.coolingSetpoint.HasValue()) {
                    newElement_0.coolingSetpoint = [NSNumber numberWithShort:entry_0.coolingSetpoint.Value()];
                } else {
                    newElement_0.coolingSetpoint = nil;
                }
                if (entry_0.heatingSetpoint.HasValue()) {
                    newElement_0.heatingSetpoint = [NSNumber numberWithShort:entry_0.heatingSetpoint.Value()];
                } else {
                    newElement_0.heatingSetpoint = nil;
                }
                if (entry_0.builtIn.IsNull()) {
                    newElement_0.builtIn = nil;
                } else {
                    newElement_0.builtIn = [NSNumber numberWithBool:entry_0.builtIn.Value()];
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
    case Attributes::Schedules::Id: {
        using TypeInfo = Attributes::Schedules::TypeInfo;
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
                MTRThermostatClusterScheduleStruct * newElement_0;
                newElement_0 = [MTRThermostatClusterScheduleStruct new];
                if (entry_0.scheduleHandle.IsNull()) {
                    newElement_0.scheduleHandle = nil;
                } else {
                    newElement_0.scheduleHandle = AsData(entry_0.scheduleHandle.Value());
                }
                newElement_0.systemMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.systemMode)];
                if (entry_0.name.HasValue()) {
                    newElement_0.name = AsString(entry_0.name.Value());
                    if (newElement_0.name == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.name = nil;
                }
                if (entry_0.presetHandle.HasValue()) {
                    newElement_0.presetHandle = AsData(entry_0.presetHandle.Value());
                } else {
                    newElement_0.presetHandle = nil;
                }
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.transitions.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTRThermostatClusterScheduleTransitionStruct * newElement_2;
                        newElement_2 = [MTRThermostatClusterScheduleTransitionStruct new];
                        newElement_2.dayOfWeek = [NSNumber numberWithUnsignedChar:entry_2.dayOfWeek.Raw()];
                        newElement_2.transitionTime = [NSNumber numberWithUnsignedShort:entry_2.transitionTime];
                        if (entry_2.presetHandle.HasValue()) {
                            newElement_2.presetHandle = AsData(entry_2.presetHandle.Value());
                        } else {
                            newElement_2.presetHandle = nil;
                        }
                        if (entry_2.systemMode.HasValue()) {
                            newElement_2.systemMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.systemMode.Value())];
                        } else {
                            newElement_2.systemMode = nil;
                        }
                        if (entry_2.coolingSetpoint.HasValue()) {
                            newElement_2.coolingSetpoint = [NSNumber numberWithShort:entry_2.coolingSetpoint.Value()];
                        } else {
                            newElement_2.coolingSetpoint = nil;
                        }
                        if (entry_2.heatingSetpoint.HasValue()) {
                            newElement_2.heatingSetpoint = [NSNumber numberWithShort:entry_2.heatingSetpoint.Value()];
                        } else {
                            newElement_2.heatingSetpoint = nil;
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.transitions = array_2;
                }
                if (entry_0.builtIn.IsNull()) {
                    newElement_0.builtIn = nil;
                } else {
                    newElement_0.builtIn = [NSNumber numberWithBool:entry_0.builtIn.Value()];
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
    case Attributes::SetpointHoldExpiryTimestamp::Id: {
        using TypeInfo = Attributes::SetpointHoldExpiryTimestamp::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForFanControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FanControl;
    switch (aAttributeId) {
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::AirflowDirection::Id: {
        using TypeInfo = Attributes::AirflowDirection::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForThermostatUserInterfaceConfigurationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ThermostatUserInterfaceConfiguration;
    switch (aAttributeId) {
    case Attributes::TemperatureDisplayMode::Id: {
        using TypeInfo = Attributes::TemperatureDisplayMode::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForColorControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ColorControl;
    switch (aAttributeId) {
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedShort:cppValue.Raw()];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForBallastConfigurationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BallastConfiguration;
    switch (aAttributeId) {
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
    case Attributes::IntrinsicBallastFactor::Id: {
        using TypeInfo = Attributes::IntrinsicBallastFactor::TypeInfo;
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForIlluminanceMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::IlluminanceMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value())];
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTemperatureMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TemperatureMeasurement;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPressureMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PressureMeasurement;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForFlowMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FlowMeasurement;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRelativeHumidityMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RelativeHumidityMeasurement;
    switch (aAttributeId) {
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOccupancySensingCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OccupancySensing;
    switch (aAttributeId) {
    case Attributes::Occupancy::Id: {
        using TypeInfo = Attributes::Occupancy::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
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
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
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
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::HoldTime::Id: {
        using TypeInfo = Attributes::HoldTime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::HoldTimeLimits::Id: {
        using TypeInfo = Attributes::HoldTimeLimits::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTROccupancySensingClusterHoldTimeLimitsStruct * _Nonnull value;
        value = [MTROccupancySensingClusterHoldTimeLimitsStruct new];
        value.holdTimeMin = [NSNumber numberWithUnsignedShort:cppValue.holdTimeMin];
        value.holdTimeMax = [NSNumber numberWithUnsignedShort:cppValue.holdTimeMax];
        value.holdTimeDefault = [NSNumber numberWithUnsignedShort:cppValue.holdTimeDefault];
        return value;
    }
    case Attributes::PIROccupiedToUnoccupiedDelay::Id: {
        using TypeInfo = Attributes::PIROccupiedToUnoccupiedDelay::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::PIRUnoccupiedToOccupiedDelay::Id: {
        using TypeInfo = Attributes::PIRUnoccupiedToOccupiedDelay::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::PIRUnoccupiedToOccupiedThreshold::Id: {
        using TypeInfo = Attributes::PIRUnoccupiedToOccupiedThreshold::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForCarbonMonoxideConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CarbonMonoxideConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForCarbonDioxideConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CarbonDioxideConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForNitrogenDioxideConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::NitrogenDioxideConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForOzoneConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OzoneConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPM25ConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Pm25ConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForFormaldehydeConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FormaldehydeConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPM1ConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Pm1ConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPM10ConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Pm10ConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForRadonConcentrationMeasurementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RadonConcentrationMeasurement;
    switch (aAttributeId) {
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
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
            value = [NSNumber numberWithFloat:cppValue.Value()];
        }
        return value;
    }
    case Attributes::PeakMeasuredValue::Id: {
        using TypeInfo = Attributes::PeakMeasuredValue::TypeInfo;
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
    case Attributes::PeakMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::PeakMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::AverageMeasuredValue::Id: {
        using TypeInfo = Attributes::AverageMeasuredValue::TypeInfo;
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
    case Attributes::AverageMeasuredValueWindow::Id: {
        using TypeInfo = Attributes::AverageMeasuredValueWindow::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::Uncertainty::Id: {
        using TypeInfo = Attributes::Uncertainty::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithFloat:cppValue];
        return value;
    }
    case Attributes::MeasurementUnit::Id: {
        using TypeInfo = Attributes::MeasurementUnit::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::MeasurementMedium::Id: {
        using TypeInfo = Attributes::MeasurementMedium::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::LevelValue::Id: {
        using TypeInfo = Attributes::LevelValue::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWiFiNetworkManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WiFiNetworkManagement;
    switch (aAttributeId) {
    case Attributes::Ssid::Id: {
        using TypeInfo = Attributes::Ssid::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::PassphraseSurrogate::Id: {
        using TypeInfo = Attributes::PassphraseSurrogate::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForThreadBorderRouterManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ThreadBorderRouterManagement;
    switch (aAttributeId) {
    case Attributes::BorderRouterName::Id: {
        using TypeInfo = Attributes::BorderRouterName::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    case Attributes::BorderAgentID::Id: {
        using TypeInfo = Attributes::BorderAgentID::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nonnull value;
        value = AsData(cppValue);
        return value;
    }
    case Attributes::ThreadVersion::Id: {
        using TypeInfo = Attributes::ThreadVersion::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
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
    case Attributes::ActiveDatasetTimestamp::Id: {
        using TypeInfo = Attributes::ActiveDatasetTimestamp::TypeInfo;
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
    case Attributes::PendingDatasetTimestamp::Id: {
        using TypeInfo = Attributes::PendingDatasetTimestamp::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForThreadNetworkDirectoryCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ThreadNetworkDirectory;
    switch (aAttributeId) {
    case Attributes::PreferredExtendedPanID::Id: {
        using TypeInfo = Attributes::PreferredExtendedPanID::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = AsData(cppValue.Value());
        }
        return value;
    }
    case Attributes::ThreadNetworks::Id: {
        using TypeInfo = Attributes::ThreadNetworks::TypeInfo;
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
                MTRThreadNetworkDirectoryClusterThreadNetworkStruct * newElement_0;
                newElement_0 = [MTRThreadNetworkDirectoryClusterThreadNetworkStruct new];
                newElement_0.extendedPanID = AsData(entry_0.extendedPanID);
                newElement_0.networkName = AsString(entry_0.networkName);
                if (newElement_0.networkName == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.channel = [NSNumber numberWithUnsignedShort:entry_0.channel];
                newElement_0.activeTimestamp = [NSNumber numberWithUnsignedLongLong:entry_0.activeTimestamp];
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
    case Attributes::ThreadNetworkTableSize::Id: {
        using TypeInfo = Attributes::ThreadNetworkTableSize::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWakeOnLANCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WakeOnLan;
    switch (aAttributeId) {
    case Attributes::MACAddress::Id: {
        using TypeInfo = Attributes::MACAddress::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    case Attributes::LinkLocalAddress::Id: {
        using TypeInfo = Attributes::LinkLocalAddress::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSData * _Nonnull value;
        value = AsData(cppValue);
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForChannelCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Channel;
    switch (aAttributeId) {
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
                MTRChannelClusterChannelInfoStruct * newElement_0;
                newElement_0 = [MTRChannelClusterChannelInfoStruct new];
                newElement_0.majorNumber = [NSNumber numberWithUnsignedShort:entry_0.majorNumber];
                newElement_0.minorNumber = [NSNumber numberWithUnsignedShort:entry_0.minorNumber];
                if (entry_0.name.HasValue()) {
                    newElement_0.name = AsString(entry_0.name.Value());
                    if (newElement_0.name == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.name = nil;
                }
                if (entry_0.callSign.HasValue()) {
                    newElement_0.callSign = AsString(entry_0.callSign.Value());
                    if (newElement_0.callSign == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.callSign = nil;
                }
                if (entry_0.affiliateCallSign.HasValue()) {
                    newElement_0.affiliateCallSign = AsString(entry_0.affiliateCallSign.Value());
                    if (newElement_0.affiliateCallSign == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.affiliateCallSign = nil;
                }
                if (entry_0.identifier.HasValue()) {
                    newElement_0.identifier = AsString(entry_0.identifier.Value());
                    if (newElement_0.identifier == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.identifier = nil;
                }
                if (entry_0.type.HasValue()) {
                    newElement_0.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.type.Value())];
                } else {
                    newElement_0.type = nil;
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
        MTRChannelClusterLineupInfoStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRChannelClusterLineupInfoStruct new];
            value.operatorName = AsString(cppValue.Value().operatorName);
            if (value.operatorName == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            if (cppValue.Value().lineupName.HasValue()) {
                value.lineupName = AsString(cppValue.Value().lineupName.Value());
                if (value.lineupName == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                value.lineupName = nil;
            }
            if (cppValue.Value().postalCode.HasValue()) {
                value.postalCode = AsString(cppValue.Value().postalCode.Value());
                if (value.postalCode == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
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
        MTRChannelClusterChannelInfoStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRChannelClusterChannelInfoStruct new];
            value.majorNumber = [NSNumber numberWithUnsignedShort:cppValue.Value().majorNumber];
            value.minorNumber = [NSNumber numberWithUnsignedShort:cppValue.Value().minorNumber];
            if (cppValue.Value().name.HasValue()) {
                value.name = AsString(cppValue.Value().name.Value());
                if (value.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                value.name = nil;
            }
            if (cppValue.Value().callSign.HasValue()) {
                value.callSign = AsString(cppValue.Value().callSign.Value());
                if (value.callSign == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                value.callSign = nil;
            }
            if (cppValue.Value().affiliateCallSign.HasValue()) {
                value.affiliateCallSign = AsString(cppValue.Value().affiliateCallSign.Value());
                if (value.affiliateCallSign == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                value.affiliateCallSign = nil;
            }
            if (cppValue.Value().identifier.HasValue()) {
                value.identifier = AsString(cppValue.Value().identifier.Value());
                if (value.identifier == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                value.identifier = nil;
            }
            if (cppValue.Value().type.HasValue()) {
                value.type = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().type.Value())];
            } else {
                value.type = nil;
            }
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTargetNavigatorCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TargetNavigator;
    switch (aAttributeId) {
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
                MTRTargetNavigatorClusterTargetInfoStruct * newElement_0;
                newElement_0 = [MTRTargetNavigatorClusterTargetInfoStruct new];
                newElement_0.identifier = [NSNumber numberWithUnsignedChar:entry_0.identifier];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForMediaPlaybackCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::MediaPlayback;
    switch (aAttributeId) {
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
        MTRMediaPlaybackClusterPlaybackPositionStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRMediaPlaybackClusterPlaybackPositionStruct new];
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
    case Attributes::ActiveAudioTrack::Id: {
        using TypeInfo = Attributes::ActiveAudioTrack::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRMediaPlaybackClusterTrackStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRMediaPlaybackClusterTrackStruct new];
            value.id = AsString(cppValue.Value().id);
            if (value.id == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            if (cppValue.Value().trackAttributes.IsNull()) {
                value.trackAttributes = nil;
            } else {
                value.trackAttributes = [MTRMediaPlaybackClusterTrackAttributesStruct new];
                value.trackAttributes.languageCode = AsString(cppValue.Value().trackAttributes.Value().languageCode);
                if (value.trackAttributes.languageCode == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (cppValue.Value().trackAttributes.Value().displayName.HasValue()) {
                    if (cppValue.Value().trackAttributes.Value().displayName.Value().IsNull()) {
                        value.trackAttributes.displayName = nil;
                    } else {
                        value.trackAttributes.displayName = AsString(cppValue.Value().trackAttributes.Value().displayName.Value().Value());
                        if (value.trackAttributes.displayName == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                    }
                } else {
                    value.trackAttributes.displayName = nil;
                }
            }
        }
        return value;
    }
    case Attributes::AvailableAudioTracks::Id: {
        using TypeInfo = Attributes::AvailableAudioTracks::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRMediaPlaybackClusterTrackStruct * newElement_1;
                    newElement_1 = [MTRMediaPlaybackClusterTrackStruct new];
                    newElement_1.id = AsString(entry_1.id);
                    if (newElement_1.id == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    if (entry_1.trackAttributes.IsNull()) {
                        newElement_1.trackAttributes = nil;
                    } else {
                        newElement_1.trackAttributes = [MTRMediaPlaybackClusterTrackAttributesStruct new];
                        newElement_1.trackAttributes.languageCode = AsString(entry_1.trackAttributes.Value().languageCode);
                        if (newElement_1.trackAttributes.languageCode == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                        if (entry_1.trackAttributes.Value().displayName.HasValue()) {
                            if (entry_1.trackAttributes.Value().displayName.Value().IsNull()) {
                                newElement_1.trackAttributes.displayName = nil;
                            } else {
                                newElement_1.trackAttributes.displayName = AsString(entry_1.trackAttributes.Value().displayName.Value().Value());
                                if (newElement_1.trackAttributes.displayName == nil) {
                                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                                    *aError = err;
                                    return nil;
                                }
                            }
                        } else {
                            newElement_1.trackAttributes.displayName = nil;
                        }
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    case Attributes::ActiveTextTrack::Id: {
        using TypeInfo = Attributes::ActiveTextTrack::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRMediaPlaybackClusterTrackStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRMediaPlaybackClusterTrackStruct new];
            value.id = AsString(cppValue.Value().id);
            if (value.id == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            if (cppValue.Value().trackAttributes.IsNull()) {
                value.trackAttributes = nil;
            } else {
                value.trackAttributes = [MTRMediaPlaybackClusterTrackAttributesStruct new];
                value.trackAttributes.languageCode = AsString(cppValue.Value().trackAttributes.Value().languageCode);
                if (value.trackAttributes.languageCode == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (cppValue.Value().trackAttributes.Value().displayName.HasValue()) {
                    if (cppValue.Value().trackAttributes.Value().displayName.Value().IsNull()) {
                        value.trackAttributes.displayName = nil;
                    } else {
                        value.trackAttributes.displayName = AsString(cppValue.Value().trackAttributes.Value().displayName.Value().Value());
                        if (value.trackAttributes.displayName == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                    }
                } else {
                    value.trackAttributes.displayName = nil;
                }
            }
        }
        return value;
    }
    case Attributes::AvailableTextTracks::Id: {
        using TypeInfo = Attributes::AvailableTextTracks::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSArray * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            { // Scope for our temporary variables
                auto * array_1 = [NSMutableArray new];
                auto iter_1 = cppValue.Value().begin();
                while (iter_1.Next()) {
                    auto & entry_1 = iter_1.GetValue();
                    MTRMediaPlaybackClusterTrackStruct * newElement_1;
                    newElement_1 = [MTRMediaPlaybackClusterTrackStruct new];
                    newElement_1.id = AsString(entry_1.id);
                    if (newElement_1.id == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    if (entry_1.trackAttributes.IsNull()) {
                        newElement_1.trackAttributes = nil;
                    } else {
                        newElement_1.trackAttributes = [MTRMediaPlaybackClusterTrackAttributesStruct new];
                        newElement_1.trackAttributes.languageCode = AsString(entry_1.trackAttributes.Value().languageCode);
                        if (newElement_1.trackAttributes.languageCode == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                        if (entry_1.trackAttributes.Value().displayName.HasValue()) {
                            if (entry_1.trackAttributes.Value().displayName.Value().IsNull()) {
                                newElement_1.trackAttributes.displayName = nil;
                            } else {
                                newElement_1.trackAttributes.displayName = AsString(entry_1.trackAttributes.Value().displayName.Value().Value());
                                if (newElement_1.trackAttributes.displayName == nil) {
                                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                                    *aError = err;
                                    return nil;
                                }
                            }
                        } else {
                            newElement_1.trackAttributes.displayName = nil;
                        }
                    }
                    [array_1 addObject:newElement_1];
                }
                CHIP_ERROR err = iter_1.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                value = array_1;
            }
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForMediaInputCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::MediaInput;
    switch (aAttributeId) {
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
                MTRMediaInputClusterInputInfoStruct * newElement_0;
                newElement_0 = [MTRMediaInputClusterInputInfoStruct new];
                newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
                newElement_0.inputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.inputType)];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.descriptionString = AsString(entry_0.description);
                if (newElement_0.descriptionString == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForLowPowerCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LowPower;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForKeypadInputCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::KeypadInput;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForContentLauncherCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ContentLauncher;
    switch (aAttributeId) {
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
                newElement_0 = AsString(entry_0);
                if (newElement_0 == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    case Attributes::SupportedStreamingProtocols::Id: {
        using TypeInfo = Attributes::SupportedStreamingProtocols::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForAudioOutputCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AudioOutput;
    switch (aAttributeId) {
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
                MTRAudioOutputClusterOutputInfoStruct * newElement_0;
                newElement_0 = [MTRAudioOutputClusterOutputInfoStruct new];
                newElement_0.index = [NSNumber numberWithUnsignedChar:entry_0.index];
                newElement_0.outputType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.outputType)];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForApplicationLauncherCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ApplicationLauncher;
    switch (aAttributeId) {
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
        MTRApplicationLauncherClusterApplicationEPStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRApplicationLauncherClusterApplicationEPStruct new];
            value.application = [MTRApplicationLauncherClusterApplicationStruct new];
            value.application.catalogVendorID = [NSNumber numberWithUnsignedShort:cppValue.Value().application.catalogVendorID];
            value.application.applicationID = AsString(cppValue.Value().application.applicationID);
            if (value.application.applicationID == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            if (cppValue.Value().endpoint.HasValue()) {
                value.endpoint = [NSNumber numberWithUnsignedShort:cppValue.Value().endpoint.Value()];
            } else {
                value.endpoint = nil;
            }
        }
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForApplicationBasicCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ApplicationBasic;
    switch (aAttributeId) {
    case Attributes::VendorName::Id: {
        using TypeInfo = Attributes::VendorName::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        MTRApplicationBasicClusterApplicationStruct * _Nonnull value;
        value = [MTRApplicationBasicClusterApplicationStruct new];
        value.catalogVendorID = [NSNumber numberWithUnsignedShort:cppValue.catalogVendorID];
        value.applicationID = AsString(cppValue.applicationID);
        if (value.applicationID == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForAccountLoginCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AccountLogin;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForContentControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ContentControl;
    switch (aAttributeId) {
    case Attributes::Enabled::Id: {
        using TypeInfo = Attributes::Enabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::OnDemandRatings::Id: {
        using TypeInfo = Attributes::OnDemandRatings::TypeInfo;
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
                MTRContentControlClusterRatingNameStruct * newElement_0;
                newElement_0 = [MTRContentControlClusterRatingNameStruct new];
                newElement_0.ratingName = AsString(entry_0.ratingName);
                if (newElement_0.ratingName == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (entry_0.ratingNameDesc.HasValue()) {
                    newElement_0.ratingNameDesc = AsString(entry_0.ratingNameDesc.Value());
                    if (newElement_0.ratingNameDesc == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.ratingNameDesc = nil;
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
    case Attributes::OnDemandRatingThreshold::Id: {
        using TypeInfo = Attributes::OnDemandRatingThreshold::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    case Attributes::ScheduledContentRatings::Id: {
        using TypeInfo = Attributes::ScheduledContentRatings::TypeInfo;
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
                MTRContentControlClusterRatingNameStruct * newElement_0;
                newElement_0 = [MTRContentControlClusterRatingNameStruct new];
                newElement_0.ratingName = AsString(entry_0.ratingName);
                if (newElement_0.ratingName == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (entry_0.ratingNameDesc.HasValue()) {
                    newElement_0.ratingNameDesc = AsString(entry_0.ratingNameDesc.Value());
                    if (newElement_0.ratingNameDesc == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.ratingNameDesc = nil;
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
    case Attributes::ScheduledContentRatingThreshold::Id: {
        using TypeInfo = Attributes::ScheduledContentRatingThreshold::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSString * _Nonnull value;
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        return value;
    }
    case Attributes::ScreenDailyTime::Id: {
        using TypeInfo = Attributes::ScreenDailyTime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::RemainingScreenTime::Id: {
        using TypeInfo = Attributes::RemainingScreenTime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::BlockUnrated::Id: {
        using TypeInfo = Attributes::BlockUnrated::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForContentAppObserverCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ContentAppObserver;
    switch (aAttributeId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForZoneManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ZoneManagement;
    switch (aAttributeId) {
    case Attributes::SupportedZoneSources::Id: {
        using TypeInfo = Attributes::SupportedZoneSources::TypeInfo;
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
    case Attributes::Zones::Id: {
        using TypeInfo = Attributes::Zones::TypeInfo;
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
                MTRZoneManagementClusterZoneInformationStruct * newElement_0;
                newElement_0 = [MTRZoneManagementClusterZoneInformationStruct new];
                newElement_0.zoneID = [NSNumber numberWithUnsignedShort:entry_0.zoneID];
                newElement_0.zoneType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.zoneType)];
                newElement_0.zoneSource = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.zoneSource)];
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
    case Attributes::Triggers::Id: {
        using TypeInfo = Attributes::Triggers::TypeInfo;
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
                MTRZoneManagementClusterZoneTriggerControlStruct * newElement_0;
                newElement_0 = [MTRZoneManagementClusterZoneTriggerControlStruct new];
                newElement_0.initialDuration = [NSNumber numberWithUnsignedShort:entry_0.initialDuration];
                newElement_0.augmentationDuration = [NSNumber numberWithUnsignedShort:entry_0.augmentationDuration];
                newElement_0.maxDuration = [NSNumber numberWithUnsignedInt:entry_0.maxDuration];
                newElement_0.blindDuration = [NSNumber numberWithUnsignedShort:entry_0.blindDuration];
                if (entry_0.sensitivity.HasValue()) {
                    newElement_0.sensitivity = [NSNumber numberWithUnsignedChar:entry_0.sensitivity.Value()];
                } else {
                    newElement_0.sensitivity = nil;
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
    case Attributes::Sensitivity::Id: {
        using TypeInfo = Attributes::Sensitivity::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForCameraAVStreamManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CameraAvStreamManagement;
    switch (aAttributeId) {
    case Attributes::MaxConcurrentVideoEncoders::Id: {
        using TypeInfo = Attributes::MaxConcurrentVideoEncoders::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MaxEncodedPixelRate::Id: {
        using TypeInfo = Attributes::MaxEncodedPixelRate::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::VideoSensorParams::Id: {
        using TypeInfo = Attributes::VideoSensorParams::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRCameraAVStreamManagementClusterVideoSensorParamsStruct * _Nonnull value;
        value = [MTRCameraAVStreamManagementClusterVideoSensorParamsStruct new];
        value.sensorWidth = [NSNumber numberWithUnsignedShort:cppValue.sensorWidth];
        value.sensorHeight = [NSNumber numberWithUnsignedShort:cppValue.sensorHeight];
        value.maxFPS = [NSNumber numberWithUnsignedShort:cppValue.maxFPS];
        if (cppValue.maxHDRFPS.HasValue()) {
            value.maxHDRFPS = [NSNumber numberWithUnsignedShort:cppValue.maxHDRFPS.Value()];
        } else {
            value.maxHDRFPS = nil;
        }
        return value;
    }
    case Attributes::NightVisionCapable::Id: {
        using TypeInfo = Attributes::NightVisionCapable::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::MinViewport::Id: {
        using TypeInfo = Attributes::MinViewport::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRCameraAVStreamManagementClusterVideoResolutionStruct * _Nonnull value;
        value = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
        value.width = [NSNumber numberWithUnsignedShort:cppValue.width];
        value.height = [NSNumber numberWithUnsignedShort:cppValue.height];
        return value;
    }
    case Attributes::RateDistortionTradeOffPoints::Id: {
        using TypeInfo = Attributes::RateDistortionTradeOffPoints::TypeInfo;
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
                MTRCameraAVStreamManagementClusterRateDistortionTradeOffPointsStruct * newElement_0;
                newElement_0 = [MTRCameraAVStreamManagementClusterRateDistortionTradeOffPointsStruct new];
                newElement_0.codec = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.codec)];
                newElement_0.resolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
                newElement_0.resolution.width = [NSNumber numberWithUnsignedShort:entry_0.resolution.width];
                newElement_0.resolution.height = [NSNumber numberWithUnsignedShort:entry_0.resolution.height];
                newElement_0.minBitRate = [NSNumber numberWithUnsignedInt:entry_0.minBitRate];
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
    case Attributes::MaxContentBufferSize::Id: {
        using TypeInfo = Attributes::MaxContentBufferSize::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::MicrophoneCapabilities::Id: {
        using TypeInfo = Attributes::MicrophoneCapabilities::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct * _Nonnull value;
        value = [MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct new];
        value.maxNumberOfChannels = [NSNumber numberWithUnsignedChar:cppValue.maxNumberOfChannels];
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.supportedCodecs.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSNumber * newElement_1;
                newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.supportedCodecs = array_1;
        }
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.supportedSampleRates.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSNumber * newElement_1;
                newElement_1 = [NSNumber numberWithUnsignedInt:entry_1];
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.supportedSampleRates = array_1;
        }
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.supportedBitDepths.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSNumber * newElement_1;
                newElement_1 = [NSNumber numberWithUnsignedChar:entry_1];
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.supportedBitDepths = array_1;
        }
        return value;
    }
    case Attributes::SpeakerCapabilities::Id: {
        using TypeInfo = Attributes::SpeakerCapabilities::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct * _Nonnull value;
        value = [MTRCameraAVStreamManagementClusterAudioCapabilitiesStruct new];
        value.maxNumberOfChannels = [NSNumber numberWithUnsignedChar:cppValue.maxNumberOfChannels];
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.supportedCodecs.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSNumber * newElement_1;
                newElement_1 = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_1)];
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.supportedCodecs = array_1;
        }
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.supportedSampleRates.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSNumber * newElement_1;
                newElement_1 = [NSNumber numberWithUnsignedInt:entry_1];
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.supportedSampleRates = array_1;
        }
        { // Scope for our temporary variables
            auto * array_1 = [NSMutableArray new];
            auto iter_1 = cppValue.supportedBitDepths.begin();
            while (iter_1.Next()) {
                auto & entry_1 = iter_1.GetValue();
                NSNumber * newElement_1;
                newElement_1 = [NSNumber numberWithUnsignedChar:entry_1];
                [array_1 addObject:newElement_1];
            }
            CHIP_ERROR err = iter_1.GetStatus();
            if (err != CHIP_NO_ERROR) {
                *aError = err;
                return nil;
            }
            value.supportedBitDepths = array_1;
        }
        return value;
    }
    case Attributes::TwoWayTalkSupport::Id: {
        using TypeInfo = Attributes::TwoWayTalkSupport::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::SupportedSnapshotParams::Id: {
        using TypeInfo = Attributes::SupportedSnapshotParams::TypeInfo;
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
                MTRCameraAVStreamManagementClusterSnapshotParamsStruct * newElement_0;
                newElement_0 = [MTRCameraAVStreamManagementClusterSnapshotParamsStruct new];
                newElement_0.resolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
                newElement_0.resolution.width = [NSNumber numberWithUnsignedShort:entry_0.resolution.width];
                newElement_0.resolution.height = [NSNumber numberWithUnsignedShort:entry_0.resolution.height];
                newElement_0.maxFrameRate = [NSNumber numberWithUnsignedShort:entry_0.maxFrameRate];
                newElement_0.imageCodec = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.imageCodec)];
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
    case Attributes::MaxNetworkBandwidth::Id: {
        using TypeInfo = Attributes::MaxNetworkBandwidth::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
        return value;
    }
    case Attributes::CurrentFrameRate::Id: {
        using TypeInfo = Attributes::CurrentFrameRate::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::HDRModeEnabled::Id: {
        using TypeInfo = Attributes::HDRModeEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::SupportedStreamUsages::Id: {
        using TypeInfo = Attributes::SupportedStreamUsages::TypeInfo;
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
    case Attributes::AllocatedVideoStreams::Id: {
        using TypeInfo = Attributes::AllocatedVideoStreams::TypeInfo;
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
                MTRCameraAVStreamManagementClusterVideoStreamStruct * newElement_0;
                newElement_0 = [MTRCameraAVStreamManagementClusterVideoStreamStruct new];
                newElement_0.videoStreamID = [NSNumber numberWithUnsignedShort:entry_0.videoStreamID];
                newElement_0.streamUsage = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.streamUsage)];
                newElement_0.videoCodec = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.videoCodec)];
                newElement_0.minFrameRate = [NSNumber numberWithUnsignedShort:entry_0.minFrameRate];
                newElement_0.maxFrameRate = [NSNumber numberWithUnsignedShort:entry_0.maxFrameRate];
                newElement_0.minResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
                newElement_0.minResolution.width = [NSNumber numberWithUnsignedShort:entry_0.minResolution.width];
                newElement_0.minResolution.height = [NSNumber numberWithUnsignedShort:entry_0.minResolution.height];
                newElement_0.maxResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
                newElement_0.maxResolution.width = [NSNumber numberWithUnsignedShort:entry_0.maxResolution.width];
                newElement_0.maxResolution.height = [NSNumber numberWithUnsignedShort:entry_0.maxResolution.height];
                newElement_0.minBitRate = [NSNumber numberWithUnsignedInt:entry_0.minBitRate];
                newElement_0.maxBitRate = [NSNumber numberWithUnsignedInt:entry_0.maxBitRate];
                newElement_0.minFragmentLen = [NSNumber numberWithUnsignedShort:entry_0.minFragmentLen];
                newElement_0.maxFragmentLen = [NSNumber numberWithUnsignedShort:entry_0.maxFragmentLen];
                if (entry_0.watermarkEnabled.HasValue()) {
                    newElement_0.watermarkEnabled = [NSNumber numberWithBool:entry_0.watermarkEnabled.Value()];
                } else {
                    newElement_0.watermarkEnabled = nil;
                }
                if (entry_0.OSDEnabled.HasValue()) {
                    newElement_0.osdEnabled = [NSNumber numberWithBool:entry_0.OSDEnabled.Value()];
                } else {
                    newElement_0.osdEnabled = nil;
                }
                newElement_0.referenceCount = [NSNumber numberWithUnsignedChar:entry_0.referenceCount];
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
    case Attributes::AllocatedAudioStreams::Id: {
        using TypeInfo = Attributes::AllocatedAudioStreams::TypeInfo;
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
                MTRCameraAVStreamManagementClusterAudioStreamStruct * newElement_0;
                newElement_0 = [MTRCameraAVStreamManagementClusterAudioStreamStruct new];
                newElement_0.audioStreamID = [NSNumber numberWithUnsignedShort:entry_0.audioStreamID];
                newElement_0.streamUsage = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.streamUsage)];
                newElement_0.audioCodec = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.audioCodec)];
                newElement_0.channelCount = [NSNumber numberWithUnsignedChar:entry_0.channelCount];
                newElement_0.sampleRate = [NSNumber numberWithUnsignedInt:entry_0.sampleRate];
                newElement_0.bitRate = [NSNumber numberWithUnsignedInt:entry_0.bitRate];
                newElement_0.bitDepth = [NSNumber numberWithUnsignedChar:entry_0.bitDepth];
                newElement_0.referenceCount = [NSNumber numberWithUnsignedChar:entry_0.referenceCount];
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
    case Attributes::AllocatedSnapshotStreams::Id: {
        using TypeInfo = Attributes::AllocatedSnapshotStreams::TypeInfo;
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
                MTRCameraAVStreamManagementClusterSnapshotStreamStruct * newElement_0;
                newElement_0 = [MTRCameraAVStreamManagementClusterSnapshotStreamStruct new];
                newElement_0.snapshotStreamID = [NSNumber numberWithUnsignedShort:entry_0.snapshotStreamID];
                newElement_0.imageCodec = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.imageCodec)];
                newElement_0.frameRate = [NSNumber numberWithUnsignedShort:entry_0.frameRate];
                newElement_0.bitRate = [NSNumber numberWithUnsignedInt:entry_0.bitRate];
                newElement_0.minResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
                newElement_0.minResolution.width = [NSNumber numberWithUnsignedShort:entry_0.minResolution.width];
                newElement_0.minResolution.height = [NSNumber numberWithUnsignedShort:entry_0.minResolution.height];
                newElement_0.maxResolution = [MTRCameraAVStreamManagementClusterVideoResolutionStruct new];
                newElement_0.maxResolution.width = [NSNumber numberWithUnsignedShort:entry_0.maxResolution.width];
                newElement_0.maxResolution.height = [NSNumber numberWithUnsignedShort:entry_0.maxResolution.height];
                newElement_0.quality = [NSNumber numberWithUnsignedChar:entry_0.quality];
                newElement_0.referenceCount = [NSNumber numberWithUnsignedChar:entry_0.referenceCount];
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
    case Attributes::RankedVideoStreamPrioritiesList::Id: {
        using TypeInfo = Attributes::RankedVideoStreamPrioritiesList::TypeInfo;
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
    case Attributes::SoftRecordingPrivacyModeEnabled::Id: {
        using TypeInfo = Attributes::SoftRecordingPrivacyModeEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::SoftLivestreamPrivacyModeEnabled::Id: {
        using TypeInfo = Attributes::SoftLivestreamPrivacyModeEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::HardPrivacyModeOn::Id: {
        using TypeInfo = Attributes::HardPrivacyModeOn::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::NightVision::Id: {
        using TypeInfo = Attributes::NightVision::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::NightVisionIllum::Id: {
        using TypeInfo = Attributes::NightVisionIllum::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::Viewport::Id: {
        using TypeInfo = Attributes::Viewport::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRCameraAVStreamManagementClusterViewportStruct * _Nonnull value;
        value = [MTRCameraAVStreamManagementClusterViewportStruct new];
        value.x1 = [NSNumber numberWithUnsignedShort:cppValue.x1];
        value.y1 = [NSNumber numberWithUnsignedShort:cppValue.y1];
        value.x2 = [NSNumber numberWithUnsignedShort:cppValue.x2];
        value.y2 = [NSNumber numberWithUnsignedShort:cppValue.y2];
        return value;
    }
    case Attributes::SpeakerMuted::Id: {
        using TypeInfo = Attributes::SpeakerMuted::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::SpeakerVolumeLevel::Id: {
        using TypeInfo = Attributes::SpeakerVolumeLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::SpeakerMaxLevel::Id: {
        using TypeInfo = Attributes::SpeakerMaxLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::SpeakerMinLevel::Id: {
        using TypeInfo = Attributes::SpeakerMinLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MicrophoneMuted::Id: {
        using TypeInfo = Attributes::MicrophoneMuted::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::MicrophoneVolumeLevel::Id: {
        using TypeInfo = Attributes::MicrophoneVolumeLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MicrophoneMaxLevel::Id: {
        using TypeInfo = Attributes::MicrophoneMaxLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MicrophoneMinLevel::Id: {
        using TypeInfo = Attributes::MicrophoneMinLevel::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MicrophoneAGCEnabled::Id: {
        using TypeInfo = Attributes::MicrophoneAGCEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::ImageRotation::Id: {
        using TypeInfo = Attributes::ImageRotation::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedShort:cppValue];
        return value;
    }
    case Attributes::ImageFlipHorizontal::Id: {
        using TypeInfo = Attributes::ImageFlipHorizontal::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::ImageFlipVertical::Id: {
        using TypeInfo = Attributes::ImageFlipVertical::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::LocalVideoRecordingEnabled::Id: {
        using TypeInfo = Attributes::LocalVideoRecordingEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::LocalSnapshotRecordingEnabled::Id: {
        using TypeInfo = Attributes::LocalSnapshotRecordingEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::StatusLightEnabled::Id: {
        using TypeInfo = Attributes::StatusLightEnabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    case Attributes::StatusLightBrightness::Id: {
        using TypeInfo = Attributes::StatusLightBrightness::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForCameraAVSettingsUserLevelManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CameraAvSettingsUserLevelManagement;
    switch (aAttributeId) {
    case Attributes::MPTZPosition::Id: {
        using TypeInfo = Attributes::MPTZPosition::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct * _Nonnull value;
        value = [MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct new];
        if (cppValue.pan.HasValue()) {
            value.pan = [NSNumber numberWithShort:cppValue.pan.Value()];
        } else {
            value.pan = nil;
        }
        if (cppValue.tilt.HasValue()) {
            value.tilt = [NSNumber numberWithShort:cppValue.tilt.Value()];
        } else {
            value.tilt = nil;
        }
        if (cppValue.zoom.HasValue()) {
            value.zoom = [NSNumber numberWithUnsignedChar:cppValue.zoom.Value()];
        } else {
            value.zoom = nil;
        }
        return value;
    }
    case Attributes::MaxPresets::Id: {
        using TypeInfo = Attributes::MaxPresets::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::MPTZPresets::Id: {
        using TypeInfo = Attributes::MPTZPresets::TypeInfo;
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
                MTRCameraAVSettingsUserLevelManagementClusterMPTZPresetStruct * newElement_0;
                newElement_0 = [MTRCameraAVSettingsUserLevelManagementClusterMPTZPresetStruct new];
                newElement_0.presetID = [NSNumber numberWithUnsignedChar:entry_0.presetID];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.settings = [MTRCameraAVSettingsUserLevelManagementClusterMPTZStruct new];
                if (entry_0.settings.pan.HasValue()) {
                    newElement_0.settings.pan = [NSNumber numberWithShort:entry_0.settings.pan.Value()];
                } else {
                    newElement_0.settings.pan = nil;
                }
                if (entry_0.settings.tilt.HasValue()) {
                    newElement_0.settings.tilt = [NSNumber numberWithShort:entry_0.settings.tilt.Value()];
                } else {
                    newElement_0.settings.tilt = nil;
                }
                if (entry_0.settings.zoom.HasValue()) {
                    newElement_0.settings.zoom = [NSNumber numberWithUnsignedChar:entry_0.settings.zoom.Value()];
                } else {
                    newElement_0.settings.zoom = nil;
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
    case Attributes::DPTZRelativeMove::Id: {
        using TypeInfo = Attributes::DPTZRelativeMove::TypeInfo;
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
    case Attributes::ZoomMax::Id: {
        using TypeInfo = Attributes::ZoomMax::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::TiltMin::Id: {
        using TypeInfo = Attributes::TiltMin::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::TiltMax::Id: {
        using TypeInfo = Attributes::TiltMax::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::PanMin::Id: {
        using TypeInfo = Attributes::PanMin::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    case Attributes::PanMax::Id: {
        using TypeInfo = Attributes::PanMax::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithShort:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWebRTCTransportProviderCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WebRTCTransportProvider;
    switch (aAttributeId) {
    case Attributes::CurrentSessions::Id: {
        using TypeInfo = Attributes::CurrentSessions::TypeInfo;
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
                MTRWebRTCTransportProviderClusterWebRTCSessionStruct * newElement_0;
                newElement_0 = [MTRWebRTCTransportProviderClusterWebRTCSessionStruct new];
                newElement_0.id = [NSNumber numberWithUnsignedShort:entry_0.id];
                newElement_0.peerNodeID = [NSNumber numberWithUnsignedLongLong:entry_0.peerNodeID];
                newElement_0.streamUsage = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.streamUsage)];
                if (entry_0.videoStreamID.IsNull()) {
                    newElement_0.videoStreamID = nil;
                } else {
                    newElement_0.videoStreamID = [NSNumber numberWithUnsignedShort:entry_0.videoStreamID.Value()];
                }
                if (entry_0.audioStreamID.IsNull()) {
                    newElement_0.audioStreamID = nil;
                } else {
                    newElement_0.audioStreamID = [NSNumber numberWithUnsignedShort:entry_0.audioStreamID.Value()];
                }
                newElement_0.metadataOptions = [NSNumber numberWithUnsignedChar:entry_0.metadataOptions.Raw()];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForWebRTCTransportRequestorCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WebRTCTransportRequestor;
    switch (aAttributeId) {
    case Attributes::CurrentSessions::Id: {
        using TypeInfo = Attributes::CurrentSessions::TypeInfo;
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
                MTRWebRTCTransportRequestorClusterWebRTCSessionStruct * newElement_0;
                newElement_0 = [MTRWebRTCTransportRequestorClusterWebRTCSessionStruct new];
                newElement_0.id = [NSNumber numberWithUnsignedShort:entry_0.id];
                newElement_0.peerNodeID = [NSNumber numberWithUnsignedLongLong:entry_0.peerNodeID];
                newElement_0.streamUsage = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.streamUsage)];
                if (entry_0.videoStreamID.IsNull()) {
                    newElement_0.videoStreamID = nil;
                } else {
                    newElement_0.videoStreamID = [NSNumber numberWithUnsignedShort:entry_0.videoStreamID.Value()];
                }
                if (entry_0.audioStreamID.IsNull()) {
                    newElement_0.audioStreamID = nil;
                } else {
                    newElement_0.audioStreamID = [NSNumber numberWithUnsignedShort:entry_0.audioStreamID.Value()];
                }
                newElement_0.metadataOptions = [NSNumber numberWithUnsignedChar:entry_0.metadataOptions.Raw()];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForPushAVStreamTransportCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PushAvStreamTransport;
    switch (aAttributeId) {
    case Attributes::SupportedContainerFormats::Id: {
        using TypeInfo = Attributes::SupportedContainerFormats::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::SupportedIngestMethods::Id: {
        using TypeInfo = Attributes::SupportedIngestMethods::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue.Raw()];
        return value;
    }
    case Attributes::CurrentConnections::Id: {
        using TypeInfo = Attributes::CurrentConnections::TypeInfo;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForChimeCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Chime;
    switch (aAttributeId) {
    case Attributes::InstalledChimeSounds::Id: {
        using TypeInfo = Attributes::InstalledChimeSounds::TypeInfo;
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
                MTRChimeClusterChimeSoundStruct * newElement_0;
                newElement_0 = [MTRChimeClusterChimeSoundStruct new];
                newElement_0.chimeID = [NSNumber numberWithUnsignedChar:entry_0.chimeID];
                newElement_0.name = AsString(entry_0.name);
                if (newElement_0.name == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
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
    case Attributes::SelectedChime::Id: {
        using TypeInfo = Attributes::SelectedChime::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::Enabled::Id: {
        using TypeInfo = Attributes::Enabled::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForEcosystemInformationCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EcosystemInformation;
    switch (aAttributeId) {
    case Attributes::DeviceDirectory::Id: {
        using TypeInfo = Attributes::DeviceDirectory::TypeInfo;
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
                MTREcosystemInformationClusterEcosystemDeviceStruct * newElement_0;
                newElement_0 = [MTREcosystemInformationClusterEcosystemDeviceStruct new];
                if (entry_0.deviceName.HasValue()) {
                    newElement_0.deviceName = AsString(entry_0.deviceName.Value());
                    if (newElement_0.deviceName == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.deviceName = nil;
                }
                if (entry_0.deviceNameLastEdit.HasValue()) {
                    newElement_0.deviceNameLastEdit = [NSNumber numberWithUnsignedLongLong:entry_0.deviceNameLastEdit.Value()];
                } else {
                    newElement_0.deviceNameLastEdit = nil;
                }
                newElement_0.bridgedEndpoint = [NSNumber numberWithUnsignedShort:entry_0.bridgedEndpoint];
                newElement_0.originalEndpoint = [NSNumber numberWithUnsignedShort:entry_0.originalEndpoint];
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.deviceTypes.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        MTREcosystemInformationClusterDeviceTypeStruct * newElement_2;
                        newElement_2 = [MTREcosystemInformationClusterDeviceTypeStruct new];
                        newElement_2.deviceType = [NSNumber numberWithUnsignedInt:entry_2.deviceType];
                        newElement_2.revision = [NSNumber numberWithUnsignedShort:entry_2.revision];
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.deviceTypes = array_2;
                }
                { // Scope for our temporary variables
                    auto * array_2 = [NSMutableArray new];
                    auto iter_2 = entry_0.uniqueLocationIDs.begin();
                    while (iter_2.Next()) {
                        auto & entry_2 = iter_2.GetValue();
                        NSString * newElement_2;
                        newElement_2 = AsString(entry_2);
                        if (newElement_2 == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                        [array_2 addObject:newElement_2];
                    }
                    CHIP_ERROR err = iter_2.GetStatus();
                    if (err != CHIP_NO_ERROR) {
                        *aError = err;
                        return nil;
                    }
                    newElement_0.uniqueLocationIDs = array_2;
                }
                newElement_0.uniqueLocationIDsLastEdit = [NSNumber numberWithUnsignedLongLong:entry_0.uniqueLocationIDsLastEdit];
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
    case Attributes::LocationDirectory::Id: {
        using TypeInfo = Attributes::LocationDirectory::TypeInfo;
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
                MTREcosystemInformationClusterEcosystemLocationStruct * newElement_0;
                newElement_0 = [MTREcosystemInformationClusterEcosystemLocationStruct new];
                newElement_0.uniqueLocationID = AsString(entry_0.uniqueLocationID);
                if (newElement_0.uniqueLocationID == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.locationDescriptor = [MTRDataTypeLocationDescriptorStruct new];
                newElement_0.locationDescriptor.locationName = AsString(entry_0.locationDescriptor.locationName);
                if (newElement_0.locationDescriptor.locationName == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                if (entry_0.locationDescriptor.floorNumber.IsNull()) {
                    newElement_0.locationDescriptor.floorNumber = nil;
                } else {
                    newElement_0.locationDescriptor.floorNumber = [NSNumber numberWithShort:entry_0.locationDescriptor.floorNumber.Value()];
                }
                if (entry_0.locationDescriptor.areaType.IsNull()) {
                    newElement_0.locationDescriptor.areaType = nil;
                } else {
                    newElement_0.locationDescriptor.areaType = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.locationDescriptor.areaType.Value())];
                }
                newElement_0.locationDescriptorLastEdit = [NSNumber numberWithUnsignedLongLong:entry_0.locationDescriptorLastEdit];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForCommissionerControlCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CommissionerControl;
    switch (aAttributeId) {
    case Attributes::SupportedDeviceCategories::Id: {
        using TypeInfo = Attributes::SupportedDeviceCategories::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue.Raw()];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTLSCertificateManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TlsCertificateManagement;
    switch (aAttributeId) {
    case Attributes::MaxRootCertificates::Id: {
        using TypeInfo = Attributes::MaxRootCertificates::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::ProvisionedRootCertificates::Id: {
        using TypeInfo = Attributes::ProvisionedRootCertificates::TypeInfo;
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
                MTRTLSCertificateManagementClusterTLSCertStruct * newElement_0;
                newElement_0 = [MTRTLSCertificateManagementClusterTLSCertStruct new];
                newElement_0.caid = [NSNumber numberWithUnsignedShort:entry_0.caid];
                if (entry_0.certificate.HasValue()) {
                    newElement_0.certificate = AsData(entry_0.certificate.Value());
                } else {
                    newElement_0.certificate = nil;
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
    case Attributes::MaxClientCertificates::Id: {
        using TypeInfo = Attributes::MaxClientCertificates::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::ProvisionedClientCertificates::Id: {
        using TypeInfo = Attributes::ProvisionedClientCertificates::TypeInfo;
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
                MTRTLSCertificateManagementClusterTLSClientCertificateDetailStruct * newElement_0;
                newElement_0 = [MTRTLSCertificateManagementClusterTLSClientCertificateDetailStruct new];
                newElement_0.ccdid = [NSNumber numberWithUnsignedShort:entry_0.ccdid];
                if (entry_0.clientCertificate.HasValue()) {
                    newElement_0.clientCertificate = AsData(entry_0.clientCertificate.Value());
                } else {
                    newElement_0.clientCertificate = nil;
                }
                if (entry_0.intermediateCertificates.HasValue()) {
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = entry_0.intermediateCertificates.Value().begin();
                        while (iter_3.Next()) {
                            auto & entry_3 = iter_3.GetValue();
                            NSData * newElement_3;
                            newElement_3 = AsData(entry_3);
                            [array_3 addObject:newElement_3];
                        }
                        CHIP_ERROR err = iter_3.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        newElement_0.intermediateCertificates = array_3;
                    }
                } else {
                    newElement_0.intermediateCertificates = nil;
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForTLSClientManagementCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TlsClientManagement;
    switch (aAttributeId) {
    case Attributes::MaxProvisioned::Id: {
        using TypeInfo = Attributes::MaxProvisioned::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::ProvisionedEndpoints::Id: {
        using TypeInfo = Attributes::ProvisionedEndpoints::TypeInfo;
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
                MTRTLSClientManagementClusterTLSEndpointStruct * newElement_0;
                newElement_0 = [MTRTLSClientManagementClusterTLSEndpointStruct new];
                newElement_0.endpointID = [NSNumber numberWithUnsignedShort:entry_0.endpointID];
                newElement_0.hostname = AsData(entry_0.hostname);
                newElement_0.port = [NSNumber numberWithUnsignedShort:entry_0.port];
                newElement_0.caid = [NSNumber numberWithUnsignedShort:entry_0.caid];
                if (entry_0.ccdid.IsNull()) {
                    newElement_0.ccdid = nil;
                } else {
                    newElement_0.ccdid = [NSNumber numberWithUnsignedShort:entry_0.ccdid.Value()];
                }
                newElement_0.status = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.status)];
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
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForUnitTestingCluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UnitTesting;
    switch (aAttributeId) {
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
        value = AsData(cppValue);
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
                newElement_0 = AsData(entry_0);
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
                MTRUnitTestingClusterTestListStructOctet * newElement_0;
                newElement_0 = [MTRUnitTestingClusterTestListStructOctet new];
                newElement_0.member1 = [NSNumber numberWithUnsignedLongLong:entry_0.member1];
                newElement_0.member2 = AsData(entry_0.member2);
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
        value = AsData(cppValue);
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
        value = AsString(cppValue);
        if (value == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
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
                MTRUnitTestingClusterNullablesAndOptionalsStruct * newElement_0;
                newElement_0 = [MTRUnitTestingClusterNullablesAndOptionalsStruct new];
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
                        newElement_0.nullableOptionalInt = [NSNumber numberWithUnsignedShort:entry_0.nullableOptionalInt.Value().Value()];
                    }
                } else {
                    newElement_0.nullableOptionalInt = nil;
                }
                if (entry_0.nullableString.IsNull()) {
                    newElement_0.nullableString = nil;
                } else {
                    newElement_0.nullableString = AsString(entry_0.nullableString.Value());
                    if (newElement_0.nullableString == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                }
                if (entry_0.optionalString.HasValue()) {
                    newElement_0.optionalString = AsString(entry_0.optionalString.Value());
                    if (newElement_0.optionalString == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                } else {
                    newElement_0.optionalString = nil;
                }
                if (entry_0.nullableOptionalString.HasValue()) {
                    if (entry_0.nullableOptionalString.Value().IsNull()) {
                        newElement_0.nullableOptionalString = nil;
                    } else {
                        newElement_0.nullableOptionalString = AsString(entry_0.nullableOptionalString.Value().Value());
                        if (newElement_0.nullableOptionalString == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                    }
                } else {
                    newElement_0.nullableOptionalString = nil;
                }
                if (entry_0.nullableStruct.IsNull()) {
                    newElement_0.nullableStruct = nil;
                } else {
                    newElement_0.nullableStruct = [MTRUnitTestingClusterSimpleStruct new];
                    newElement_0.nullableStruct.a = [NSNumber numberWithUnsignedChar:entry_0.nullableStruct.Value().a];
                    newElement_0.nullableStruct.b = [NSNumber numberWithBool:entry_0.nullableStruct.Value().b];
                    newElement_0.nullableStruct.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableStruct.Value().c)];
                    newElement_0.nullableStruct.d = AsData(entry_0.nullableStruct.Value().d);
                    newElement_0.nullableStruct.e = AsString(entry_0.nullableStruct.Value().e);
                    if (newElement_0.nullableStruct.e == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    newElement_0.nullableStruct.f = [NSNumber numberWithUnsignedChar:entry_0.nullableStruct.Value().f.Raw()];
                    newElement_0.nullableStruct.g = [NSNumber numberWithFloat:entry_0.nullableStruct.Value().g];
                    newElement_0.nullableStruct.h = [NSNumber numberWithDouble:entry_0.nullableStruct.Value().h];
                    if (entry_0.nullableStruct.Value().i.HasValue()) {
                        newElement_0.nullableStruct.i = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableStruct.Value().i.Value())];
                    } else {
                        newElement_0.nullableStruct.i = nil;
                    }
                }
                if (entry_0.optionalStruct.HasValue()) {
                    newElement_0.optionalStruct = [MTRUnitTestingClusterSimpleStruct new];
                    newElement_0.optionalStruct.a = [NSNumber numberWithUnsignedChar:entry_0.optionalStruct.Value().a];
                    newElement_0.optionalStruct.b = [NSNumber numberWithBool:entry_0.optionalStruct.Value().b];
                    newElement_0.optionalStruct.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.optionalStruct.Value().c)];
                    newElement_0.optionalStruct.d = AsData(entry_0.optionalStruct.Value().d);
                    newElement_0.optionalStruct.e = AsString(entry_0.optionalStruct.Value().e);
                    if (newElement_0.optionalStruct.e == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    newElement_0.optionalStruct.f = [NSNumber numberWithUnsignedChar:entry_0.optionalStruct.Value().f.Raw()];
                    newElement_0.optionalStruct.g = [NSNumber numberWithFloat:entry_0.optionalStruct.Value().g];
                    newElement_0.optionalStruct.h = [NSNumber numberWithDouble:entry_0.optionalStruct.Value().h];
                    if (entry_0.optionalStruct.Value().i.HasValue()) {
                        newElement_0.optionalStruct.i = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.optionalStruct.Value().i.Value())];
                    } else {
                        newElement_0.optionalStruct.i = nil;
                    }
                } else {
                    newElement_0.optionalStruct = nil;
                }
                if (entry_0.nullableOptionalStruct.HasValue()) {
                    if (entry_0.nullableOptionalStruct.Value().IsNull()) {
                        newElement_0.nullableOptionalStruct = nil;
                    } else {
                        newElement_0.nullableOptionalStruct = [MTRUnitTestingClusterSimpleStruct new];
                        newElement_0.nullableOptionalStruct.a = [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalStruct.Value().Value().a];
                        newElement_0.nullableOptionalStruct.b = [NSNumber numberWithBool:entry_0.nullableOptionalStruct.Value().Value().b];
                        newElement_0.nullableOptionalStruct.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableOptionalStruct.Value().Value().c)];
                        newElement_0.nullableOptionalStruct.d = AsData(entry_0.nullableOptionalStruct.Value().Value().d);
                        newElement_0.nullableOptionalStruct.e = AsString(entry_0.nullableOptionalStruct.Value().Value().e);
                        if (newElement_0.nullableOptionalStruct.e == nil) {
                            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                            *aError = err;
                            return nil;
                        }
                        newElement_0.nullableOptionalStruct.f = [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalStruct.Value().Value().f.Raw()];
                        newElement_0.nullableOptionalStruct.g = [NSNumber numberWithFloat:entry_0.nullableOptionalStruct.Value().Value().g];
                        newElement_0.nullableOptionalStruct.h = [NSNumber numberWithDouble:entry_0.nullableOptionalStruct.Value().Value().h];
                        if (entry_0.nullableOptionalStruct.Value().Value().i.HasValue()) {
                            newElement_0.nullableOptionalStruct.i = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.nullableOptionalStruct.Value().Value().i.Value())];
                        } else {
                            newElement_0.nullableOptionalStruct.i = nil;
                        }
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
        MTRUnitTestingClusterSimpleStruct * _Nonnull value;
        value = [MTRUnitTestingClusterSimpleStruct new];
        value.a = [NSNumber numberWithUnsignedChar:cppValue.a];
        value.b = [NSNumber numberWithBool:cppValue.b];
        value.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.c)];
        value.d = AsData(cppValue.d);
        value.e = AsString(cppValue.e);
        if (value.e == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        value.f = [NSNumber numberWithUnsignedChar:cppValue.f.Raw()];
        value.g = [NSNumber numberWithFloat:cppValue.g];
        value.h = [NSNumber numberWithDouble:cppValue.h];
        if (cppValue.i.HasValue()) {
            value.i = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.i.Value())];
        } else {
            value.i = nil;
        }
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
                newElement_0 = AsData(entry_0);
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
                MTRUnitTestingClusterTestFabricScoped * newElement_0;
                newElement_0 = [MTRUnitTestingClusterTestFabricScoped new];
                newElement_0.fabricSensitiveInt8u = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveInt8u];
                if (entry_0.optionalFabricSensitiveInt8u.HasValue()) {
                    newElement_0.optionalFabricSensitiveInt8u = [NSNumber numberWithUnsignedChar:entry_0.optionalFabricSensitiveInt8u.Value()];
                } else {
                    newElement_0.optionalFabricSensitiveInt8u = nil;
                }
                if (entry_0.nullableFabricSensitiveInt8u.IsNull()) {
                    newElement_0.nullableFabricSensitiveInt8u = nil;
                } else {
                    newElement_0.nullableFabricSensitiveInt8u = [NSNumber numberWithUnsignedChar:entry_0.nullableFabricSensitiveInt8u.Value()];
                }
                if (entry_0.nullableOptionalFabricSensitiveInt8u.HasValue()) {
                    if (entry_0.nullableOptionalFabricSensitiveInt8u.Value().IsNull()) {
                        newElement_0.nullableOptionalFabricSensitiveInt8u = nil;
                    } else {
                        newElement_0.nullableOptionalFabricSensitiveInt8u = [NSNumber numberWithUnsignedChar:entry_0.nullableOptionalFabricSensitiveInt8u.Value().Value()];
                    }
                } else {
                    newElement_0.nullableOptionalFabricSensitiveInt8u = nil;
                }
                newElement_0.fabricSensitiveCharString = AsString(entry_0.fabricSensitiveCharString);
                if (newElement_0.fabricSensitiveCharString == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.fabricSensitiveStruct = [MTRUnitTestingClusterSimpleStruct new];
                newElement_0.fabricSensitiveStruct.a = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.a];
                newElement_0.fabricSensitiveStruct.b = [NSNumber numberWithBool:entry_0.fabricSensitiveStruct.b];
                newElement_0.fabricSensitiveStruct.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.fabricSensitiveStruct.c)];
                newElement_0.fabricSensitiveStruct.d = AsData(entry_0.fabricSensitiveStruct.d);
                newElement_0.fabricSensitiveStruct.e = AsString(entry_0.fabricSensitiveStruct.e);
                if (newElement_0.fabricSensitiveStruct.e == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
                newElement_0.fabricSensitiveStruct.f = [NSNumber numberWithUnsignedChar:entry_0.fabricSensitiveStruct.f.Raw()];
                newElement_0.fabricSensitiveStruct.g = [NSNumber numberWithFloat:entry_0.fabricSensitiveStruct.g];
                newElement_0.fabricSensitiveStruct.h = [NSNumber numberWithDouble:entry_0.fabricSensitiveStruct.h];
                if (entry_0.fabricSensitiveStruct.i.HasValue()) {
                    newElement_0.fabricSensitiveStruct.i = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.fabricSensitiveStruct.i.Value())];
                } else {
                    newElement_0.fabricSensitiveStruct.i = nil;
                }
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
    case Attributes::GlobalEnum::Id: {
        using TypeInfo = Attributes::GlobalEnum::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue)];
        return value;
    }
    case Attributes::GlobalStruct::Id: {
        using TypeInfo = Attributes::GlobalStruct::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRDataTypeTestGlobalStruct * _Nonnull value;
        value = [MTRDataTypeTestGlobalStruct new];
        value.name = AsString(cppValue.name);
        if (value.name == nil) {
            CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
            *aError = err;
            return nil;
        }
        if (cppValue.myBitmap.IsNull()) {
            value.myBitmap = nil;
        } else {
            value.myBitmap = [NSNumber numberWithUnsignedInt:cppValue.myBitmap.Value().Raw()];
        }
        if (cppValue.myEnum.HasValue()) {
            if (cppValue.myEnum.Value().IsNull()) {
                value.myEnum = nil;
            } else {
                value.myEnum = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.myEnum.Value().Value())];
            }
        } else {
            value.myEnum = nil;
        }
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
    case Attributes::ReadFailureCode::Id: {
        using TypeInfo = Attributes::ReadFailureCode::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    case Attributes::FailureInt32U::Id: {
        using TypeInfo = Attributes::FailureInt32U::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedInt:cppValue];
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
            value = AsData(cppValue.Value());
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
            value = AsString(cppValue.Value());
            if (value == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
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
        MTRUnitTestingClusterSimpleStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRUnitTestingClusterSimpleStruct new];
            value.a = [NSNumber numberWithUnsignedChar:cppValue.Value().a];
            value.b = [NSNumber numberWithBool:cppValue.Value().b];
            value.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().c)];
            value.d = AsData(cppValue.Value().d);
            value.e = AsString(cppValue.Value().e);
            if (value.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            value.f = [NSNumber numberWithUnsignedChar:cppValue.Value().f.Raw()];
            value.g = [NSNumber numberWithFloat:cppValue.Value().g];
            value.h = [NSNumber numberWithDouble:cppValue.Value().h];
            if (cppValue.Value().i.HasValue()) {
                value.i = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().i.Value())];
            } else {
                value.i = nil;
            }
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
    case Attributes::NullableGlobalEnum::Id: {
        using TypeInfo = Attributes::NullableGlobalEnum::TypeInfo;
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
    case Attributes::NullableGlobalStruct::Id: {
        using TypeInfo = Attributes::NullableGlobalStruct::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        MTRDataTypeTestGlobalStruct * _Nullable value;
        if (cppValue.IsNull()) {
            value = nil;
        } else {
            value = [MTRDataTypeTestGlobalStruct new];
            value.name = AsString(cppValue.Value().name);
            if (value.name == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            if (cppValue.Value().myBitmap.IsNull()) {
                value.myBitmap = nil;
            } else {
                value.myBitmap = [NSNumber numberWithUnsignedInt:cppValue.Value().myBitmap.Value().Raw()];
            }
            if (cppValue.Value().myEnum.HasValue()) {
                if (cppValue.Value().myEnum.Value().IsNull()) {
                    value.myEnum = nil;
                } else {
                    value.myEnum = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.Value().myEnum.Value().Value())];
                }
            } else {
                value.myEnum = nil;
            }
        }
        return value;
    }
    case Attributes::MeiInt8u::Id: {
        using TypeInfo = Attributes::MeiInt8u::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithUnsignedChar:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
static id _Nullable DecodeAttributeValueForSampleMEICluster(AttributeId aAttributeId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SampleMei;
    switch (aAttributeId) {
    case Attributes::FlipFlop::Id: {
        using TypeInfo = Attributes::FlipFlop::TypeInfo;
        TypeInfo::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }
        NSNumber * _Nonnull value;
        value = [NSNumber numberWithBool:cppValue];
        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}

id _Nullable MTRDecodeAttributeValue(const ConcreteAttributePath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    if (IsGlobalAttribute(aPath.mAttributeId)) {
        return DecodeGlobalAttributeValue(aPath.mAttributeId, aReader, aError);
    }

    switch (aPath.mClusterId) {
    case Clusters::Identify::Id: {
        return DecodeAttributeValueForIdentifyCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Groups::Id: {
        return DecodeAttributeValueForGroupsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OnOff::Id: {
        return DecodeAttributeValueForOnOffCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::LevelControl::Id: {
        return DecodeAttributeValueForLevelControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PulseWidthModulation::Id: {
        return DecodeAttributeValueForPulseWidthModulationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Descriptor::Id: {
        return DecodeAttributeValueForDescriptorCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Binding::Id: {
        return DecodeAttributeValueForBindingCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::AccessControl::Id: {
        return DecodeAttributeValueForAccessControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Actions::Id: {
        return DecodeAttributeValueForActionsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::BasicInformation::Id: {
        return DecodeAttributeValueForBasicInformationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        return DecodeAttributeValueForOTASoftwareUpdateProviderCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        return DecodeAttributeValueForOTASoftwareUpdateRequestorCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::LocalizationConfiguration::Id: {
        return DecodeAttributeValueForLocalizationConfigurationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TimeFormatLocalization::Id: {
        return DecodeAttributeValueForTimeFormatLocalizationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::UnitLocalization::Id: {
        return DecodeAttributeValueForUnitLocalizationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PowerSourceConfiguration::Id: {
        return DecodeAttributeValueForPowerSourceConfigurationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PowerSource::Id: {
        return DecodeAttributeValueForPowerSourceCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::GeneralCommissioning::Id: {
        return DecodeAttributeValueForGeneralCommissioningCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::NetworkCommissioning::Id: {
        return DecodeAttributeValueForNetworkCommissioningCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DiagnosticLogs::Id: {
        return DecodeAttributeValueForDiagnosticLogsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::GeneralDiagnostics::Id: {
        return DecodeAttributeValueForGeneralDiagnosticsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::SoftwareDiagnostics::Id: {
        return DecodeAttributeValueForSoftwareDiagnosticsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        return DecodeAttributeValueForThreadNetworkDiagnosticsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        return DecodeAttributeValueForWiFiNetworkDiagnosticsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        return DecodeAttributeValueForEthernetNetworkDiagnosticsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TimeSynchronization::Id: {
        return DecodeAttributeValueForTimeSynchronizationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::BridgedDeviceBasicInformation::Id: {
        return DecodeAttributeValueForBridgedDeviceBasicInformationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Switch::Id: {
        return DecodeAttributeValueForSwitchCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::AdministratorCommissioning::Id: {
        return DecodeAttributeValueForAdministratorCommissioningCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OperationalCredentials::Id: {
        return DecodeAttributeValueForOperationalCredentialsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::GroupKeyManagement::Id: {
        return DecodeAttributeValueForGroupKeyManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::FixedLabel::Id: {
        return DecodeAttributeValueForFixedLabelCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::UserLabel::Id: {
        return DecodeAttributeValueForUserLabelCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::BooleanState::Id: {
        return DecodeAttributeValueForBooleanStateCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::IcdManagement::Id: {
        return DecodeAttributeValueForICDManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Timer::Id: {
        return DecodeAttributeValueForTimerCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OvenCavityOperationalState::Id: {
        return DecodeAttributeValueForOvenCavityOperationalStateCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OvenMode::Id: {
        return DecodeAttributeValueForOvenModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::LaundryDryerControls::Id: {
        return DecodeAttributeValueForLaundryDryerControlsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ModeSelect::Id: {
        return DecodeAttributeValueForModeSelectCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::LaundryWasherMode::Id: {
        return DecodeAttributeValueForLaundryWasherModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RefrigeratorAndTemperatureControlledCabinetMode::Id: {
        return DecodeAttributeValueForRefrigeratorAndTemperatureControlledCabinetModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::LaundryWasherControls::Id: {
        return DecodeAttributeValueForLaundryWasherControlsCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RvcRunMode::Id: {
        return DecodeAttributeValueForRVCRunModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RvcCleanMode::Id: {
        return DecodeAttributeValueForRVCCleanModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TemperatureControl::Id: {
        return DecodeAttributeValueForTemperatureControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RefrigeratorAlarm::Id: {
        return DecodeAttributeValueForRefrigeratorAlarmCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DishwasherMode::Id: {
        return DecodeAttributeValueForDishwasherModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::AirQuality::Id: {
        return DecodeAttributeValueForAirQualityCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::SmokeCoAlarm::Id: {
        return DecodeAttributeValueForSmokeCOAlarmCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DishwasherAlarm::Id: {
        return DecodeAttributeValueForDishwasherAlarmCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::MicrowaveOvenMode::Id: {
        return DecodeAttributeValueForMicrowaveOvenModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::MicrowaveOvenControl::Id: {
        return DecodeAttributeValueForMicrowaveOvenControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OperationalState::Id: {
        return DecodeAttributeValueForOperationalStateCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RvcOperationalState::Id: {
        return DecodeAttributeValueForRVCOperationalStateCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ScenesManagement::Id: {
        return DecodeAttributeValueForScenesManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::HepaFilterMonitoring::Id: {
        return DecodeAttributeValueForHEPAFilterMonitoringCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ActivatedCarbonFilterMonitoring::Id: {
        return DecodeAttributeValueForActivatedCarbonFilterMonitoringCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::BooleanStateConfiguration::Id: {
        return DecodeAttributeValueForBooleanStateConfigurationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ValveConfigurationAndControl::Id: {
        return DecodeAttributeValueForValveConfigurationAndControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ElectricalPowerMeasurement::Id: {
        return DecodeAttributeValueForElectricalPowerMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ElectricalEnergyMeasurement::Id: {
        return DecodeAttributeValueForElectricalEnergyMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WaterHeaterManagement::Id: {
        return DecodeAttributeValueForWaterHeaterManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DemandResponseLoadControl::Id: {
        return DecodeAttributeValueForDemandResponseLoadControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Messages::Id: {
        return DecodeAttributeValueForMessagesCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DeviceEnergyManagement::Id: {
        return DecodeAttributeValueForDeviceEnergyManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::EnergyEvse::Id: {
        return DecodeAttributeValueForEnergyEVSECluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::EnergyPreference::Id: {
        return DecodeAttributeValueForEnergyPreferenceCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PowerTopology::Id: {
        return DecodeAttributeValueForPowerTopologyCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::EnergyEvseMode::Id: {
        return DecodeAttributeValueForEnergyEVSEModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WaterHeaterMode::Id: {
        return DecodeAttributeValueForWaterHeaterModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DeviceEnergyManagementMode::Id: {
        return DecodeAttributeValueForDeviceEnergyManagementModeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::DoorLock::Id: {
        return DecodeAttributeValueForDoorLockCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WindowCovering::Id: {
        return DecodeAttributeValueForWindowCoveringCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ClosureControl::Id: {
        return DecodeAttributeValueForClosureControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ServiceArea::Id: {
        return DecodeAttributeValueForServiceAreaCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        return DecodeAttributeValueForPumpConfigurationAndControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Thermostat::Id: {
        return DecodeAttributeValueForThermostatCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::FanControl::Id: {
        return DecodeAttributeValueForFanControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        return DecodeAttributeValueForThermostatUserInterfaceConfigurationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ColorControl::Id: {
        return DecodeAttributeValueForColorControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::BallastConfiguration::Id: {
        return DecodeAttributeValueForBallastConfigurationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::IlluminanceMeasurement::Id: {
        return DecodeAttributeValueForIlluminanceMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TemperatureMeasurement::Id: {
        return DecodeAttributeValueForTemperatureMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PressureMeasurement::Id: {
        return DecodeAttributeValueForPressureMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::FlowMeasurement::Id: {
        return DecodeAttributeValueForFlowMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        return DecodeAttributeValueForRelativeHumidityMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OccupancySensing::Id: {
        return DecodeAttributeValueForOccupancySensingCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id: {
        return DecodeAttributeValueForCarbonMonoxideConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::CarbonDioxideConcentrationMeasurement::Id: {
        return DecodeAttributeValueForCarbonDioxideConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id: {
        return DecodeAttributeValueForNitrogenDioxideConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::OzoneConcentrationMeasurement::Id: {
        return DecodeAttributeValueForOzoneConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Pm25ConcentrationMeasurement::Id: {
        return DecodeAttributeValueForPM25ConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::FormaldehydeConcentrationMeasurement::Id: {
        return DecodeAttributeValueForFormaldehydeConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Pm1ConcentrationMeasurement::Id: {
        return DecodeAttributeValueForPM1ConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Pm10ConcentrationMeasurement::Id: {
        return DecodeAttributeValueForPM10ConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id: {
        return DecodeAttributeValueForTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::RadonConcentrationMeasurement::Id: {
        return DecodeAttributeValueForRadonConcentrationMeasurementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WiFiNetworkManagement::Id: {
        return DecodeAttributeValueForWiFiNetworkManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ThreadBorderRouterManagement::Id: {
        return DecodeAttributeValueForThreadBorderRouterManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ThreadNetworkDirectory::Id: {
        return DecodeAttributeValueForThreadNetworkDirectoryCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WakeOnLan::Id: {
        return DecodeAttributeValueForWakeOnLANCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Channel::Id: {
        return DecodeAttributeValueForChannelCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TargetNavigator::Id: {
        return DecodeAttributeValueForTargetNavigatorCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::MediaPlayback::Id: {
        return DecodeAttributeValueForMediaPlaybackCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::MediaInput::Id: {
        return DecodeAttributeValueForMediaInputCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::LowPower::Id: {
        return DecodeAttributeValueForLowPowerCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::KeypadInput::Id: {
        return DecodeAttributeValueForKeypadInputCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ContentLauncher::Id: {
        return DecodeAttributeValueForContentLauncherCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::AudioOutput::Id: {
        return DecodeAttributeValueForAudioOutputCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ApplicationLauncher::Id: {
        return DecodeAttributeValueForApplicationLauncherCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ApplicationBasic::Id: {
        return DecodeAttributeValueForApplicationBasicCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::AccountLogin::Id: {
        return DecodeAttributeValueForAccountLoginCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ContentControl::Id: {
        return DecodeAttributeValueForContentControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ContentAppObserver::Id: {
        return DecodeAttributeValueForContentAppObserverCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::ZoneManagement::Id: {
        return DecodeAttributeValueForZoneManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::CameraAvStreamManagement::Id: {
        return DecodeAttributeValueForCameraAVStreamManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::CameraAvSettingsUserLevelManagement::Id: {
        return DecodeAttributeValueForCameraAVSettingsUserLevelManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WebRTCTransportProvider::Id: {
        return DecodeAttributeValueForWebRTCTransportProviderCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::WebRTCTransportRequestor::Id: {
        return DecodeAttributeValueForWebRTCTransportRequestorCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::PushAvStreamTransport::Id: {
        return DecodeAttributeValueForPushAVStreamTransportCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::Chime::Id: {
        return DecodeAttributeValueForChimeCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::EcosystemInformation::Id: {
        return DecodeAttributeValueForEcosystemInformationCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::CommissionerControl::Id: {
        return DecodeAttributeValueForCommissionerControlCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TlsCertificateManagement::Id: {
        return DecodeAttributeValueForTLSCertificateManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::TlsClientManagement::Id: {
        return DecodeAttributeValueForTLSClientManagementCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::UnitTesting::Id: {
        return DecodeAttributeValueForUnitTestingCluster(aPath.mAttributeId, aReader, aError);
    }
    case Clusters::SampleMei::Id: {
        return DecodeAttributeValueForSampleMEICluster(aPath.mAttributeId, aReader, aError);
    }
    default: {
        break;
    }
    }
    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
