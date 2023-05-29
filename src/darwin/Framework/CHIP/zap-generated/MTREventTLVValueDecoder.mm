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

#import "MTREventTLVValueDecoder_Internal.h"

#import "MTRStructsObjc.h"
#import "NSDataSpanConversion.h"
#import "NSStringSpanConversion.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Events.h>
#include <app/EventHeader.h>
#include <app/EventLoggingTypes.h>
#include <app/data-model/DecodableList.h>
#include <app/data-model/Decode.h>
#include <lib/support/TypeTraits.h>

using namespace chip;
using namespace chip::app;

static id _Nullable DecodeEventPayloadForIdentifyCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Identify;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForGroupsCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Groups;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForScenesCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Scenes;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOnOffCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OnOff;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOnOffSwitchConfigurationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OnOffSwitchConfiguration;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForLevelControlCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LevelControl;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBinaryInputBasicCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BinaryInputBasic;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForDescriptorCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Descriptor;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBindingCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Binding;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForAccessControlCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AccessControl;
    switch (aEventId) {
    case Events::AccessControlEntryChanged::Id: {
        Events::AccessControlEntryChanged::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRAccessControlClusterAccessControlEntryChangedEvent new];

        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.adminNodeID.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.adminNodeID.Value()];
            }
            value.adminNodeID = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.adminPasscodeID.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedShort:cppValue.adminPasscodeID.Value()];
            }
            value.adminPasscodeID = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.changeType)];
            value.changeType = memberValue;
        } while (0);
        do {
            MTRAccessControlClusterAccessControlEntryStruct * _Nullable memberValue;
            if (cppValue.latestValue.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [MTRAccessControlClusterAccessControlEntryStruct new];
                memberValue.privilege =
                    [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.latestValue.Value().privilege)];
                memberValue.authMode = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.latestValue.Value().authMode)];
                if (cppValue.latestValue.Value().subjects.IsNull()) {
                    memberValue.subjects = nil;
                } else {
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = cppValue.latestValue.Value().subjects.Value().begin();
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
                        memberValue.subjects = array_3;
                    }
                }
                if (cppValue.latestValue.Value().targets.IsNull()) {
                    memberValue.targets = nil;
                } else {
                    { // Scope for our temporary variables
                        auto * array_3 = [NSMutableArray new];
                        auto iter_3 = cppValue.latestValue.Value().targets.Value().begin();
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
                        memberValue.targets = array_3;
                    }
                }
                memberValue.fabricIndex = [NSNumber numberWithUnsignedChar:cppValue.latestValue.Value().fabricIndex];
            }
            value.latestValue = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex];
            value.fabricIndex = memberValue;
        } while (0);

        return value;
    }
    case Events::AccessControlExtensionChanged::Id: {
        Events::AccessControlExtensionChanged::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRAccessControlClusterAccessControlExtensionChangedEvent new];

        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.adminNodeID.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.adminNodeID.Value()];
            }
            value.adminNodeID = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.adminPasscodeID.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedShort:cppValue.adminPasscodeID.Value()];
            }
            value.adminPasscodeID = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.changeType)];
            value.changeType = memberValue;
        } while (0);
        do {
            MTRAccessControlClusterAccessControlExtensionStruct * _Nullable memberValue;
            if (cppValue.latestValue.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [MTRAccessControlClusterAccessControlExtensionStruct new];
                memberValue.data = AsData(cppValue.latestValue.Value().data);
                memberValue.fabricIndex = [NSNumber numberWithUnsignedChar:cppValue.latestValue.Value().fabricIndex];
            }
            value.latestValue = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex];
            value.fabricIndex = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForActionsCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Actions;
    switch (aEventId) {
    case Events::StateChanged::Id: {
        Events::StateChanged::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRActionsClusterStateChangedEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedShort:cppValue.actionID];
            value.actionID = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.invokeID];
            value.invokeID = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.newState)];
            value.newState = memberValue;
        } while (0);

        return value;
    }
    case Events::ActionFailed::Id: {
        Events::ActionFailed::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRActionsClusterActionFailedEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedShort:cppValue.actionID];
            value.actionID = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.invokeID];
            value.invokeID = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.newState)];
            value.newState = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.error)];
            value.error = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBasicInformationCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BasicInformation;
    switch (aEventId) {
    case Events::StartUp::Id: {
        Events::StartUp::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBasicInformationClusterStartUpEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.softwareVersion];
            value.softwareVersion = memberValue;
        } while (0);

        return value;
    }
    case Events::ShutDown::Id: {
        Events::ShutDown::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBasicInformationClusterShutDownEvent new];

        return value;
    }
    case Events::Leave::Id: {
        Events::Leave::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBasicInformationClusterLeaveEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex];
            value.fabricIndex = memberValue;
        } while (0);

        return value;
    }
    case Events::ReachableChanged::Id: {
        Events::ReachableChanged::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBasicInformationClusterReachableChangedEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithBool:cppValue.reachableNewValue];
            value.reachableNewValue = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOTASoftwareUpdateProviderCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OtaSoftwareUpdateProvider;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOTASoftwareUpdateRequestorCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OtaSoftwareUpdateRequestor;
    switch (aEventId) {
    case Events::StateTransition::Id: {
        Events::StateTransition::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTROTASoftwareUpdateRequestorClusterStateTransitionEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.previousState)];
            value.previousState = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.newState)];
            value.newState = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.reason)];
            value.reason = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.targetSoftwareVersion.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedInt:cppValue.targetSoftwareVersion.Value()];
            }
            value.targetSoftwareVersion = memberValue;
        } while (0);

        return value;
    }
    case Events::VersionApplied::Id: {
        Events::VersionApplied::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTROTASoftwareUpdateRequestorClusterVersionAppliedEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.softwareVersion];
            value.softwareVersion = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedShort:cppValue.productID];
            value.productID = memberValue;
        } while (0);

        return value;
    }
    case Events::DownloadError::Id: {
        Events::DownloadError::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTROTASoftwareUpdateRequestorClusterDownloadErrorEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.softwareVersion];
            value.softwareVersion = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.bytesDownloaded];
            value.bytesDownloaded = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.progressPercent.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.progressPercent.Value()];
            }
            value.progressPercent = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.platformCode.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithLongLong:cppValue.platformCode.Value()];
            }
            value.platformCode = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForLocalizationConfigurationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LocalizationConfiguration;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTimeFormatLocalizationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TimeFormatLocalization;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForUnitLocalizationCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UnitLocalization;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPowerSourceConfigurationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PowerSourceConfiguration;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPowerSourceCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PowerSource;
    switch (aEventId) {
    case Events::WiredFaultChange::Id: {
        Events::WiredFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPowerSourceClusterWiredFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    case Events::BatFaultChange::Id: {
        Events::BatFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPowerSourceClusterBatFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    case Events::BatChargeFaultChange::Id: {
        Events::BatChargeFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPowerSourceClusterBatChargeFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForGeneralCommissioningCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::GeneralCommissioning;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForNetworkCommissioningCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::NetworkCommissioning;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForDiagnosticLogsCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DiagnosticLogs;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForGeneralDiagnosticsCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::GeneralDiagnostics;
    switch (aEventId) {
    case Events::HardwareFaultChange::Id: {
        Events::HardwareFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    case Events::RadioFaultChange::Id: {
        Events::RadioFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRGeneralDiagnosticsClusterRadioFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    case Events::NetworkFaultChange::Id: {
        Events::NetworkFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    case Events::BootReason::Id: {
        Events::BootReason::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRGeneralDiagnosticsClusterBootReasonEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.bootReason)];
            value.bootReason = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForSoftwareDiagnosticsCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SoftwareDiagnostics;
    switch (aEventId) {
    case Events::SoftwareFault::Id: {
        Events::SoftwareFault::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSoftwareDiagnosticsClusterSoftwareFaultEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.id];
            value.id = memberValue;
        } while (0);
        do {
            NSString * _Nullable memberValue;
            if (cppValue.name.HasValue()) {
                memberValue = AsString(cppValue.name.Value());
                if (memberValue == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                memberValue = nil;
            }
            value.name = memberValue;
        } while (0);
        do {
            NSData * _Nullable memberValue;
            if (cppValue.faultRecording.HasValue()) {
                memberValue = AsData(cppValue.faultRecording.Value());
            } else {
                memberValue = nil;
            }
            value.faultRecording = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForThreadNetworkDiagnosticsCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ThreadNetworkDiagnostics;
    switch (aEventId) {
    case Events::ConnectionStatus::Id: {
        Events::ConnectionStatus::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.connectionStatus)];
            value.connectionStatus = memberValue;
        } while (0);

        return value;
    }
    case Events::NetworkFaultChange::Id: {
        Events::NetworkFaultChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent new];

        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.current.begin();
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
                memberValue = array_0;
            }
            value.current = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.previous.begin();
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
                memberValue = array_0;
            }
            value.previous = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForWiFiNetworkDiagnosticsCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WiFiNetworkDiagnostics;
    switch (aEventId) {
    case Events::Disconnection::Id: {
        Events::Disconnection::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedShort:cppValue.reasonCode];
            value.reasonCode = memberValue;
        } while (0);

        return value;
    }
    case Events::AssociationFailure::Id: {
        Events::AssociationFailure::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.associationFailure)];
            value.associationFailure = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedShort:cppValue.status];
            value.status = memberValue;
        } while (0);

        return value;
    }
    case Events::ConnectionStatus::Id: {
        Events::ConnectionStatus::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.connectionStatus)];
            value.connectionStatus = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForEthernetNetworkDiagnosticsCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EthernetNetworkDiagnostics;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBridgedDeviceBasicInformationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BridgedDeviceBasicInformation;
    switch (aEventId) {
    case Events::StartUp::Id: {
        Events::StartUp::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBridgedDeviceBasicInformationClusterStartUpEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.softwareVersion];
            value.softwareVersion = memberValue;
        } while (0);

        return value;
    }
    case Events::ShutDown::Id: {
        Events::ShutDown::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBridgedDeviceBasicInformationClusterShutDownEvent new];

        return value;
    }
    case Events::Leave::Id: {
        Events::Leave::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBridgedDeviceBasicInformationClusterLeaveEvent new];

        return value;
    }
    case Events::ReachableChanged::Id: {
        Events::ReachableChanged::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBridgedDeviceBasicInformationClusterReachableChangedEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithBool:cppValue.reachableNewValue];
            value.reachableNewValue = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForSwitchCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Switch;
    switch (aEventId) {
    case Events::SwitchLatched::Id: {
        Events::SwitchLatched::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterSwitchLatchedEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.newPosition];
            value.newPosition = memberValue;
        } while (0);

        return value;
    }
    case Events::InitialPress::Id: {
        Events::InitialPress::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterInitialPressEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.newPosition];
            value.newPosition = memberValue;
        } while (0);

        return value;
    }
    case Events::LongPress::Id: {
        Events::LongPress::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterLongPressEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.newPosition];
            value.newPosition = memberValue;
        } while (0);

        return value;
    }
    case Events::ShortRelease::Id: {
        Events::ShortRelease::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterShortReleaseEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.previousPosition];
            value.previousPosition = memberValue;
        } while (0);

        return value;
    }
    case Events::LongRelease::Id: {
        Events::LongRelease::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterLongReleaseEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.previousPosition];
            value.previousPosition = memberValue;
        } while (0);

        return value;
    }
    case Events::MultiPressOngoing::Id: {
        Events::MultiPressOngoing::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterMultiPressOngoingEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.newPosition];
            value.newPosition = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.currentNumberOfPressesCounted];
            value.currentNumberOfPressesCounted = memberValue;
        } while (0);

        return value;
    }
    case Events::MultiPressComplete::Id: {
        Events::MultiPressComplete::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSwitchClusterMultiPressCompleteEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.previousPosition];
            value.previousPosition = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.totalNumberOfPressesCounted];
            value.totalNumberOfPressesCounted = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForAdministratorCommissioningCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AdministratorCommissioning;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOperationalCredentialsCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OperationalCredentials;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForGroupKeyManagementCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::GroupKeyManagement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFixedLabelCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FixedLabel;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForUserLabelCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UserLabel;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBooleanStateCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BooleanState;
    switch (aEventId) {
    case Events::StateChange::Id: {
        Events::StateChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRBooleanStateClusterStateChangeEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithBool:cppValue.stateValue];
            value.stateValue = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForModeSelectCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ModeSelect;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTemperatureControlCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TemperatureControl;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForRefrigeratorAlarmCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RefrigeratorAlarm;
    switch (aEventId) {
    case Events::Notify::Id: {
        Events::Notify::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRRefrigeratorAlarmClusterNotifyEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.active.Raw()];
            value.active = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.inactive.Raw()];
            value.inactive = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.state.Raw()];
            value.state = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedInt:cppValue.mask.Raw()];
            value.mask = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForAirQualityCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AirQuality;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForSmokeCOAlarmCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SmokeCoAlarm;
    switch (aEventId) {
    case Events::SmokeAlarm::Id: {
        Events::SmokeAlarm::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterSmokeAlarmEvent new];

        return value;
    }
    case Events::COAlarm::Id: {
        Events::COAlarm::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterCOAlarmEvent new];

        return value;
    }
    case Events::LowBattery::Id: {
        Events::LowBattery::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterLowBatteryEvent new];

        return value;
    }
    case Events::HardwareFault::Id: {
        Events::HardwareFault::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterHardwareFaultEvent new];

        return value;
    }
    case Events::EndOfService::Id: {
        Events::EndOfService::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterEndOfServiceEvent new];

        return value;
    }
    case Events::SelfTestComplete::Id: {
        Events::SelfTestComplete::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterSelfTestCompleteEvent new];

        return value;
    }
    case Events::AlarmMuted::Id: {
        Events::AlarmMuted::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterAlarmMutedEvent new];

        return value;
    }
    case Events::MuteEnded::Id: {
        Events::MuteEnded::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterMuteEndedEvent new];

        return value;
    }
    case Events::InterconnectSmokeAlarm::Id: {
        Events::InterconnectSmokeAlarm::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterInterconnectSmokeAlarmEvent new];

        return value;
    }
    case Events::InterconnectCOAlarm::Id: {
        Events::InterconnectCOAlarm::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterInterconnectCOAlarmEvent new];

        return value;
    }
    case Events::AllClear::Id: {
        Events::AllClear::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRSmokeCOAlarmClusterAllClearEvent new];

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOperationalStateCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OperationalState;
    switch (aEventId) {
    case Events::OperationalError::Id: {
        Events::OperationalError::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTROperationalStateClusterOperationalErrorEvent new];

        do {
            MTROperationalStateClusterErrorStateStruct * _Nonnull memberValue;
            memberValue = [MTROperationalStateClusterErrorStateStruct new];
            memberValue.errorStateID = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.errorState.errorStateID)];
            if (cppValue.errorState.errorStateLabel.IsNull()) {
                memberValue.errorStateLabel = nil;
            } else {
                memberValue.errorStateLabel = AsString(cppValue.errorState.errorStateLabel.Value());
                if (memberValue.errorStateLabel == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            }
            if (cppValue.errorState.errorStateDetails.HasValue()) {
                memberValue.errorStateDetails = AsString(cppValue.errorState.errorStateDetails.Value());
                if (memberValue.errorStateDetails == nil) {
                    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                    *aError = err;
                    return nil;
                }
            } else {
                memberValue.errorStateDetails = nil;
            }
            value.errorState = memberValue;
        } while (0);

        return value;
    }
    case Events::OperationCompletion::Id: {
        Events::OperationCompletion::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTROperationalStateClusterOperationCompletionEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.completionErrorCode)];
            value.completionErrorCode = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.totalOperationalTime.HasValue()) {
                if (cppValue.totalOperationalTime.Value().IsNull()) {
                    memberValue = nil;
                } else {
                    memberValue = [NSNumber numberWithUnsignedInt:cppValue.totalOperationalTime.Value().Value()];
                }
            } else {
                memberValue = nil;
            }
            value.totalOperationalTime = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.pausedTime.HasValue()) {
                if (cppValue.pausedTime.Value().IsNull()) {
                    memberValue = nil;
                } else {
                    memberValue = [NSNumber numberWithUnsignedInt:cppValue.pausedTime.Value().Value()];
                }
            } else {
                memberValue = nil;
            }
            value.pausedTime = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForHEPAFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::HepaFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForActivatedCarbonFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ActivatedCarbonFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForCeramicFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CeramicFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForElectrostaticFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ElectrostaticFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForUVFilterMonitoringCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UvFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForIonizingFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::IonizingFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForZeoliteFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ZeoliteFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOzoneFilterMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OzoneFilterMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForWaterTankMonitoringCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WaterTankMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFuelTankMonitoringCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FuelTankMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForInkCartridgeMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::InkCartridgeMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTonerCartridgeMonitoringCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TonerCartridgeMonitoring;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForDoorLockCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DoorLock;
    switch (aEventId) {
    case Events::DoorLockAlarm::Id: {
        Events::DoorLockAlarm::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRDoorLockClusterDoorLockAlarmEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.alarmCode)];
            value.alarmCode = memberValue;
        } while (0);

        return value;
    }
    case Events::DoorStateChange::Id: {
        Events::DoorStateChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRDoorLockClusterDoorStateChangeEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.doorState)];
            value.doorState = memberValue;
        } while (0);

        return value;
    }
    case Events::LockOperation::Id: {
        Events::LockOperation::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRDoorLockClusterLockOperationEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.lockOperationType)];
            value.lockOperationType = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.operationSource)];
            value.operationSource = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.userIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedShort:cppValue.userIndex.Value()];
            }
            value.userIndex = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.fabricIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex.Value()];
            }
            value.fabricIndex = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.sourceNode.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.sourceNode.Value()];
            }
            value.sourceNode = memberValue;
        } while (0);
        do {
            NSArray * _Nullable memberValue;
            if (cppValue.credentials.HasValue()) {
                if (cppValue.credentials.Value().IsNull()) {
                    memberValue = nil;
                } else {
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = cppValue.credentials.Value().Value().begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            MTRDoorLockClusterCredentialStruct * newElement_2;
                            newElement_2 = [MTRDoorLockClusterCredentialStruct new];
                            newElement_2.credentialType =
                                [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.credentialType)];
                            newElement_2.credentialIndex = [NSNumber numberWithUnsignedShort:entry_2.credentialIndex];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        memberValue = array_2;
                    }
                }
            } else {
                memberValue = nil;
            }
            value.credentials = memberValue;
        } while (0);

        return value;
    }
    case Events::LockOperationError::Id: {
        Events::LockOperationError::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRDoorLockClusterLockOperationErrorEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.lockOperationType)];
            value.lockOperationType = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.operationSource)];
            value.operationSource = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.operationError)];
            value.operationError = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.userIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedShort:cppValue.userIndex.Value()];
            }
            value.userIndex = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.fabricIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex.Value()];
            }
            value.fabricIndex = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.sourceNode.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.sourceNode.Value()];
            }
            value.sourceNode = memberValue;
        } while (0);
        do {
            NSArray * _Nullable memberValue;
            if (cppValue.credentials.HasValue()) {
                if (cppValue.credentials.Value().IsNull()) {
                    memberValue = nil;
                } else {
                    { // Scope for our temporary variables
                        auto * array_2 = [NSMutableArray new];
                        auto iter_2 = cppValue.credentials.Value().Value().begin();
                        while (iter_2.Next()) {
                            auto & entry_2 = iter_2.GetValue();
                            MTRDoorLockClusterCredentialStruct * newElement_2;
                            newElement_2 = [MTRDoorLockClusterCredentialStruct new];
                            newElement_2.credentialType =
                                [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_2.credentialType)];
                            newElement_2.credentialIndex = [NSNumber numberWithUnsignedShort:entry_2.credentialIndex];
                            [array_2 addObject:newElement_2];
                        }
                        CHIP_ERROR err = iter_2.GetStatus();
                        if (err != CHIP_NO_ERROR) {
                            *aError = err;
                            return nil;
                        }
                        memberValue = array_2;
                    }
                }
            } else {
                memberValue = nil;
            }
            value.credentials = memberValue;
        } while (0);

        return value;
    }
    case Events::LockUserChange::Id: {
        Events::LockUserChange::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRDoorLockClusterLockUserChangeEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.lockDataType)];
            value.lockDataType = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.dataOperationType)];
            value.dataOperationType = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.operationSource)];
            value.operationSource = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.userIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedShort:cppValue.userIndex.Value()];
            }
            value.userIndex = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.fabricIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex.Value()];
            }
            value.fabricIndex = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.sourceNode.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedLongLong:cppValue.sourceNode.Value()];
            }
            value.sourceNode = memberValue;
        } while (0);
        do {
            NSNumber * _Nullable memberValue;
            if (cppValue.dataIndex.IsNull()) {
                memberValue = nil;
            } else {
                memberValue = [NSNumber numberWithUnsignedShort:cppValue.dataIndex.Value()];
            }
            value.dataIndex = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForWindowCoveringCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WindowCovering;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBarrierControlCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BarrierControl;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPumpConfigurationAndControlCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PumpConfigurationAndControl;
    switch (aEventId) {
    case Events::SupplyVoltageLow::Id: {
        Events::SupplyVoltageLow::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent new];

        return value;
    }
    case Events::SupplyVoltageHigh::Id: {
        Events::SupplyVoltageHigh::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent new];

        return value;
    }
    case Events::PowerMissingPhase::Id: {
        Events::PowerMissingPhase::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent new];

        return value;
    }
    case Events::SystemPressureLow::Id: {
        Events::SystemPressureLow::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterSystemPressureLowEvent new];

        return value;
    }
    case Events::SystemPressureHigh::Id: {
        Events::SystemPressureHigh::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterSystemPressureHighEvent new];

        return value;
    }
    case Events::DryRunning::Id: {
        Events::DryRunning::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterDryRunningEvent new];

        return value;
    }
    case Events::MotorTemperatureHigh::Id: {
        Events::MotorTemperatureHigh::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent new];

        return value;
    }
    case Events::PumpMotorFatalFailure::Id: {
        Events::PumpMotorFatalFailure::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent new];

        return value;
    }
    case Events::ElectronicTemperatureHigh::Id: {
        Events::ElectronicTemperatureHigh::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent new];

        return value;
    }
    case Events::PumpBlocked::Id: {
        Events::PumpBlocked::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterPumpBlockedEvent new];

        return value;
    }
    case Events::SensorFailure::Id: {
        Events::SensorFailure::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterSensorFailureEvent new];

        return value;
    }
    case Events::ElectronicNonFatalFailure::Id: {
        Events::ElectronicNonFatalFailure::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent new];

        return value;
    }
    case Events::ElectronicFatalFailure::Id: {
        Events::ElectronicFatalFailure::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent new];

        return value;
    }
    case Events::GeneralFault::Id: {
        Events::GeneralFault::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterGeneralFaultEvent new];

        return value;
    }
    case Events::Leakage::Id: {
        Events::Leakage::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterLeakageEvent new];

        return value;
    }
    case Events::AirDetection::Id: {
        Events::AirDetection::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterAirDetectionEvent new];

        return value;
    }
    case Events::TurbineOperation::Id: {
        Events::TurbineOperation::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRPumpConfigurationAndControlClusterTurbineOperationEvent new];

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForThermostatCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Thermostat;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFanControlCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FanControl;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForThermostatUserInterfaceConfigurationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ThermostatUserInterfaceConfiguration;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForColorControlCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ColorControl;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBallastConfigurationCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BallastConfiguration;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForIlluminanceMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::IlluminanceMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTemperatureMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TemperatureMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPressureMeasurementCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::PressureMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFlowMeasurementCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FlowMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForRelativeHumidityMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RelativeHumidityMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOccupancySensingCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OccupancySensing;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForCarbonMonoxideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CarbonMonoxideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForCarbonDioxideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CarbonDioxideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForEthyleneConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EthyleneConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForEthyleneOxideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::EthyleneOxideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForHydrogenConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::HydrogenConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForHydrogenSulfideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::HydrogenSulfideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForNitricOxideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::NitricOxideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForNitrogenDioxideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::NitrogenDioxideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOxygenConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OxygenConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForOzoneConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::OzoneConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForSulfurDioxideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SulfurDioxideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForDissolvedOxygenConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::DissolvedOxygenConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBromateConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BromateConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForChloraminesConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ChloraminesConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForChlorineConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ChlorineConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFecalColiformEColiConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FecalColiformEColiConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFluorideConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FluorideConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForHaloaceticAcidsConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::HaloaceticAcidsConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTotalTrihalomethanesConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TotalTrihalomethanesConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTotalColiformBacteriaConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TotalColiformBacteriaConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTurbidityConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TurbidityConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForCopperConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::CopperConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForLeadConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LeadConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForManganeseConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ManganeseConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForSulfateConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SulfateConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBromodichloromethaneConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BromodichloromethaneConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForBromoformConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::BromoformConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForChlorodibromomethaneConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ChlorodibromomethaneConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForChloroformConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ChloroformConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForSodiumConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::SodiumConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPM25ConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Pm25ConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForFormaldehydeConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::FormaldehydeConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPM1ConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Pm1ConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForPM10ConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Pm10ConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForRadonConcentrationMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::RadonConcentrationMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForWakeOnLANCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::WakeOnLan;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForChannelCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::Channel;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForTargetNavigatorCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::TargetNavigator;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForMediaPlaybackCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::MediaPlayback;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForMediaInputCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::MediaInput;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForLowPowerCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::LowPower;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForKeypadInputCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::KeypadInput;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForContentLauncherCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ContentLauncher;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForAudioOutputCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AudioOutput;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForApplicationLauncherCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ApplicationLauncher;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForApplicationBasicCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ApplicationBasic;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForAccountLoginCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::AccountLogin;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForElectricalMeasurementCluster(
    EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::ElectricalMeasurement;
    switch (aEventId) {
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
static id _Nullable DecodeEventPayloadForUnitTestingCluster(EventId aEventId, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    using namespace Clusters::UnitTesting;
    switch (aEventId) {
    case Events::TestEvent::Id: {
        Events::TestEvent::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRUnitTestingClusterTestEventEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.arg1];
            value.arg1 = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.arg2)];
            value.arg2 = memberValue;
        } while (0);
        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithBool:cppValue.arg3];
            value.arg3 = memberValue;
        } while (0);
        do {
            MTRUnitTestingClusterSimpleStruct * _Nonnull memberValue;
            memberValue = [MTRUnitTestingClusterSimpleStruct new];
            memberValue.a = [NSNumber numberWithUnsignedChar:cppValue.arg4.a];
            memberValue.b = [NSNumber numberWithBool:cppValue.arg4.b];
            memberValue.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.arg4.c)];
            memberValue.d = AsData(cppValue.arg4.d);
            memberValue.e = AsString(cppValue.arg4.e);
            if (memberValue.e == nil) {
                CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                *aError = err;
                return nil;
            }
            memberValue.f = [NSNumber numberWithUnsignedChar:cppValue.arg4.f.Raw()];
            memberValue.g = [NSNumber numberWithFloat:cppValue.arg4.g];
            memberValue.h = [NSNumber numberWithDouble:cppValue.arg4.h];
            value.arg4 = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.arg5.begin();
                while (iter_0.Next()) {
                    auto & entry_0 = iter_0.GetValue();
                    MTRUnitTestingClusterSimpleStruct * newElement_0;
                    newElement_0 = [MTRUnitTestingClusterSimpleStruct new];
                    newElement_0.a = [NSNumber numberWithUnsignedChar:entry_0.a];
                    newElement_0.b = [NSNumber numberWithBool:entry_0.b];
                    newElement_0.c = [NSNumber numberWithUnsignedChar:chip::to_underlying(entry_0.c)];
                    newElement_0.d = AsData(entry_0.d);
                    newElement_0.e = AsString(entry_0.e);
                    if (newElement_0.e == nil) {
                        CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;
                        *aError = err;
                        return nil;
                    }
                    newElement_0.f = [NSNumber numberWithUnsignedChar:entry_0.f.Raw()];
                    newElement_0.g = [NSNumber numberWithFloat:entry_0.g];
                    newElement_0.h = [NSNumber numberWithDouble:entry_0.h];
                    [array_0 addObject:newElement_0];
                }
                CHIP_ERROR err = iter_0.GetStatus();
                if (err != CHIP_NO_ERROR) {
                    *aError = err;
                    return nil;
                }
                memberValue = array_0;
            }
            value.arg5 = memberValue;
        } while (0);
        do {
            NSArray * _Nonnull memberValue;
            { // Scope for our temporary variables
                auto * array_0 = [NSMutableArray new];
                auto iter_0 = cppValue.arg6.begin();
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
                memberValue = array_0;
            }
            value.arg6 = memberValue;
        } while (0);

        return value;
    }
    case Events::TestFabricScopedEvent::Id: {
        Events::TestFabricScopedEvent::DecodableType cppValue;
        *aError = DataModel::Decode(aReader, cppValue);
        if (*aError != CHIP_NO_ERROR) {
            return nil;
        }

        __auto_type * value = [MTRUnitTestingClusterTestFabricScopedEventEvent new];

        do {
            NSNumber * _Nonnull memberValue;
            memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex];
            value.fabricIndex = memberValue;
        } while (0);

        return value;
    }
    default: {
        break;
    }
    }

    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}

id _Nullable MTRDecodeEventPayload(const ConcreteEventPath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    switch (aPath.mClusterId) {
    case Clusters::Identify::Id: {
        return DecodeEventPayloadForIdentifyCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Groups::Id: {
        return DecodeEventPayloadForGroupsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Scenes::Id: {
        return DecodeEventPayloadForScenesCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OnOff::Id: {
        return DecodeEventPayloadForOnOffCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OnOffSwitchConfiguration::Id: {
        return DecodeEventPayloadForOnOffSwitchConfigurationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::LevelControl::Id: {
        return DecodeEventPayloadForLevelControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BinaryInputBasic::Id: {
        return DecodeEventPayloadForBinaryInputBasicCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Descriptor::Id: {
        return DecodeEventPayloadForDescriptorCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Binding::Id: {
        return DecodeEventPayloadForBindingCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::AccessControl::Id: {
        return DecodeEventPayloadForAccessControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Actions::Id: {
        return DecodeEventPayloadForActionsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BasicInformation::Id: {
        return DecodeEventPayloadForBasicInformationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        return DecodeEventPayloadForOTASoftwareUpdateProviderCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        return DecodeEventPayloadForOTASoftwareUpdateRequestorCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::LocalizationConfiguration::Id: {
        return DecodeEventPayloadForLocalizationConfigurationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TimeFormatLocalization::Id: {
        return DecodeEventPayloadForTimeFormatLocalizationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::UnitLocalization::Id: {
        return DecodeEventPayloadForUnitLocalizationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::PowerSourceConfiguration::Id: {
        return DecodeEventPayloadForPowerSourceConfigurationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::PowerSource::Id: {
        return DecodeEventPayloadForPowerSourceCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::GeneralCommissioning::Id: {
        return DecodeEventPayloadForGeneralCommissioningCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::NetworkCommissioning::Id: {
        return DecodeEventPayloadForNetworkCommissioningCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::DiagnosticLogs::Id: {
        return DecodeEventPayloadForDiagnosticLogsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::GeneralDiagnostics::Id: {
        return DecodeEventPayloadForGeneralDiagnosticsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::SoftwareDiagnostics::Id: {
        return DecodeEventPayloadForSoftwareDiagnosticsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        return DecodeEventPayloadForThreadNetworkDiagnosticsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        return DecodeEventPayloadForWiFiNetworkDiagnosticsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        return DecodeEventPayloadForEthernetNetworkDiagnosticsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BridgedDeviceBasicInformation::Id: {
        return DecodeEventPayloadForBridgedDeviceBasicInformationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Switch::Id: {
        return DecodeEventPayloadForSwitchCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::AdministratorCommissioning::Id: {
        return DecodeEventPayloadForAdministratorCommissioningCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OperationalCredentials::Id: {
        return DecodeEventPayloadForOperationalCredentialsCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::GroupKeyManagement::Id: {
        return DecodeEventPayloadForGroupKeyManagementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FixedLabel::Id: {
        return DecodeEventPayloadForFixedLabelCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::UserLabel::Id: {
        return DecodeEventPayloadForUserLabelCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BooleanState::Id: {
        return DecodeEventPayloadForBooleanStateCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ModeSelect::Id: {
        return DecodeEventPayloadForModeSelectCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TemperatureControl::Id: {
        return DecodeEventPayloadForTemperatureControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::RefrigeratorAlarm::Id: {
        return DecodeEventPayloadForRefrigeratorAlarmCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::AirQuality::Id: {
        return DecodeEventPayloadForAirQualityCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::SmokeCoAlarm::Id: {
        return DecodeEventPayloadForSmokeCOAlarmCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OperationalState::Id: {
        return DecodeEventPayloadForOperationalStateCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::HepaFilterMonitoring::Id: {
        return DecodeEventPayloadForHEPAFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ActivatedCarbonFilterMonitoring::Id: {
        return DecodeEventPayloadForActivatedCarbonFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::CeramicFilterMonitoring::Id: {
        return DecodeEventPayloadForCeramicFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ElectrostaticFilterMonitoring::Id: {
        return DecodeEventPayloadForElectrostaticFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::UvFilterMonitoring::Id: {
        return DecodeEventPayloadForUVFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::IonizingFilterMonitoring::Id: {
        return DecodeEventPayloadForIonizingFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ZeoliteFilterMonitoring::Id: {
        return DecodeEventPayloadForZeoliteFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OzoneFilterMonitoring::Id: {
        return DecodeEventPayloadForOzoneFilterMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::WaterTankMonitoring::Id: {
        return DecodeEventPayloadForWaterTankMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FuelTankMonitoring::Id: {
        return DecodeEventPayloadForFuelTankMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::InkCartridgeMonitoring::Id: {
        return DecodeEventPayloadForInkCartridgeMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TonerCartridgeMonitoring::Id: {
        return DecodeEventPayloadForTonerCartridgeMonitoringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::DoorLock::Id: {
        return DecodeEventPayloadForDoorLockCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::WindowCovering::Id: {
        return DecodeEventPayloadForWindowCoveringCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BarrierControl::Id: {
        return DecodeEventPayloadForBarrierControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        return DecodeEventPayloadForPumpConfigurationAndControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Thermostat::Id: {
        return DecodeEventPayloadForThermostatCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FanControl::Id: {
        return DecodeEventPayloadForFanControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        return DecodeEventPayloadForThermostatUserInterfaceConfigurationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ColorControl::Id: {
        return DecodeEventPayloadForColorControlCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BallastConfiguration::Id: {
        return DecodeEventPayloadForBallastConfigurationCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::IlluminanceMeasurement::Id: {
        return DecodeEventPayloadForIlluminanceMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TemperatureMeasurement::Id: {
        return DecodeEventPayloadForTemperatureMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::PressureMeasurement::Id: {
        return DecodeEventPayloadForPressureMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FlowMeasurement::Id: {
        return DecodeEventPayloadForFlowMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        return DecodeEventPayloadForRelativeHumidityMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OccupancySensing::Id: {
        return DecodeEventPayloadForOccupancySensingCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::CarbonMonoxideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForCarbonMonoxideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::CarbonDioxideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForCarbonDioxideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::EthyleneConcentrationMeasurement::Id: {
        return DecodeEventPayloadForEthyleneConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::EthyleneOxideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForEthyleneOxideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::HydrogenConcentrationMeasurement::Id: {
        return DecodeEventPayloadForHydrogenConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::HydrogenSulfideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForHydrogenSulfideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::NitricOxideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForNitricOxideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::NitrogenDioxideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForNitrogenDioxideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OxygenConcentrationMeasurement::Id: {
        return DecodeEventPayloadForOxygenConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::OzoneConcentrationMeasurement::Id: {
        return DecodeEventPayloadForOzoneConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::SulfurDioxideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForSulfurDioxideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::DissolvedOxygenConcentrationMeasurement::Id: {
        return DecodeEventPayloadForDissolvedOxygenConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BromateConcentrationMeasurement::Id: {
        return DecodeEventPayloadForBromateConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ChloraminesConcentrationMeasurement::Id: {
        return DecodeEventPayloadForChloraminesConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ChlorineConcentrationMeasurement::Id: {
        return DecodeEventPayloadForChlorineConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FecalColiformEColiConcentrationMeasurement::Id: {
        return DecodeEventPayloadForFecalColiformEColiConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FluorideConcentrationMeasurement::Id: {
        return DecodeEventPayloadForFluorideConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::HaloaceticAcidsConcentrationMeasurement::Id: {
        return DecodeEventPayloadForHaloaceticAcidsConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TotalTrihalomethanesConcentrationMeasurement::Id: {
        return DecodeEventPayloadForTotalTrihalomethanesConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TotalColiformBacteriaConcentrationMeasurement::Id: {
        return DecodeEventPayloadForTotalColiformBacteriaConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TurbidityConcentrationMeasurement::Id: {
        return DecodeEventPayloadForTurbidityConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::CopperConcentrationMeasurement::Id: {
        return DecodeEventPayloadForCopperConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::LeadConcentrationMeasurement::Id: {
        return DecodeEventPayloadForLeadConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ManganeseConcentrationMeasurement::Id: {
        return DecodeEventPayloadForManganeseConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::SulfateConcentrationMeasurement::Id: {
        return DecodeEventPayloadForSulfateConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BromodichloromethaneConcentrationMeasurement::Id: {
        return DecodeEventPayloadForBromodichloromethaneConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::BromoformConcentrationMeasurement::Id: {
        return DecodeEventPayloadForBromoformConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ChlorodibromomethaneConcentrationMeasurement::Id: {
        return DecodeEventPayloadForChlorodibromomethaneConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ChloroformConcentrationMeasurement::Id: {
        return DecodeEventPayloadForChloroformConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::SodiumConcentrationMeasurement::Id: {
        return DecodeEventPayloadForSodiumConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Pm25ConcentrationMeasurement::Id: {
        return DecodeEventPayloadForPM25ConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::FormaldehydeConcentrationMeasurement::Id: {
        return DecodeEventPayloadForFormaldehydeConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Pm1ConcentrationMeasurement::Id: {
        return DecodeEventPayloadForPM1ConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Pm10ConcentrationMeasurement::Id: {
        return DecodeEventPayloadForPM10ConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TotalVolatileOrganicCompoundsConcentrationMeasurement::Id: {
        return DecodeEventPayloadForTotalVolatileOrganicCompoundsConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::RadonConcentrationMeasurement::Id: {
        return DecodeEventPayloadForRadonConcentrationMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::WakeOnLan::Id: {
        return DecodeEventPayloadForWakeOnLANCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::Channel::Id: {
        return DecodeEventPayloadForChannelCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::TargetNavigator::Id: {
        return DecodeEventPayloadForTargetNavigatorCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::MediaPlayback::Id: {
        return DecodeEventPayloadForMediaPlaybackCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::MediaInput::Id: {
        return DecodeEventPayloadForMediaInputCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::LowPower::Id: {
        return DecodeEventPayloadForLowPowerCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::KeypadInput::Id: {
        return DecodeEventPayloadForKeypadInputCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ContentLauncher::Id: {
        return DecodeEventPayloadForContentLauncherCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::AudioOutput::Id: {
        return DecodeEventPayloadForAudioOutputCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ApplicationLauncher::Id: {
        return DecodeEventPayloadForApplicationLauncherCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ApplicationBasic::Id: {
        return DecodeEventPayloadForApplicationBasicCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::AccountLogin::Id: {
        return DecodeEventPayloadForAccountLoginCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::ElectricalMeasurement::Id: {
        return DecodeEventPayloadForElectricalMeasurementCluster(aPath.mEventId, aReader, aError);
    }
    case Clusters::UnitTesting::Id: {
        return DecodeEventPayloadForUnitTestingCluster(aPath.mEventId, aReader, aError);
    }
    default: {
        break;
    }
    }
    *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
    return nil;
}
