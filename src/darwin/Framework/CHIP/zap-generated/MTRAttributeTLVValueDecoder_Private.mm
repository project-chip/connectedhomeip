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
#import "MTRStructsObjc_Private.h"
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

id _Nullable MTRPrivateDecodeAttributeValue(const ConcreteAttributePath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    if (IsGlobalAttribute(aPath.mAttributeId)) {
        return DecodeGlobalAttributeValue(aPath.mAttributeId, aReader, aError);
    }

    switch (aPath.mClusterId) {
    default: {
        break;
    }
    }
    *aError = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH_IB;
    return nil;
}
