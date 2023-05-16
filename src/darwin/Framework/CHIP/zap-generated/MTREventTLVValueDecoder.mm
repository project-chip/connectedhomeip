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
                memberValue.data = [NSData dataWithBytes:cppValue.latestValue.Value().data.data()
                                                  length:cppValue.latestValue.Value().data.size()];
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
                memberValue = [[NSString alloc] initWithBytes:cppValue.name.Value().data()
                                                       length:cppValue.name.Value().size()
                                                     encoding:NSUTF8StringEncoding];
            } else {
                memberValue = nil;
            }
            value.name = memberValue;
        } while (0);
        do {
            NSData * _Nullable memberValue;
            if (cppValue.faultRecording.HasValue()) {
                memberValue = [NSData dataWithBytes:cppValue.faultRecording.Value().data()
                                             length:cppValue.faultRecording.Value().size()];
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
            memberValue.d = [NSData dataWithBytes:cppValue.arg4.d.data() length:cppValue.arg4.d.size()];
            memberValue.e = [[NSString alloc] initWithBytes:cppValue.arg4.e.data()
                                                     length:cppValue.arg4.e.size()
                                                   encoding:NSUTF8StringEncoding];
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
                    newElement_0.d = [NSData dataWithBytes:entry_0.d.data() length:entry_0.d.size()];
                    newElement_0.e = [[NSString alloc] initWithBytes:entry_0.e.data()
                                                              length:entry_0.e.size()
                                                            encoding:NSUTF8StringEncoding];
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
