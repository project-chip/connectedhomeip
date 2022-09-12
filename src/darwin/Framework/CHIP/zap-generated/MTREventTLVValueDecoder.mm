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

id MTRDecodeEventPayload(const ConcreteEventPath & aPath, TLV::TLVReader & aReader, CHIP_ERROR * aError)
{
    switch (aPath.mClusterId) {
    case Clusters::Identify::Id: {
        using namespace Clusters::Identify;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Groups::Id: {
        using namespace Clusters::Groups;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Scenes::Id: {
        using namespace Clusters::Scenes;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OnOff::Id: {
        using namespace Clusters::OnOff;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OnOffSwitchConfiguration::Id: {
        using namespace Clusters::OnOffSwitchConfiguration;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::LevelControl::Id: {
        using namespace Clusters::LevelControl;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BinaryInputBasic::Id: {
        using namespace Clusters::BinaryInputBasic;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Descriptor::Id: {
        using namespace Clusters::Descriptor;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Binding::Id: {
        using namespace Clusters::Binding;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AccessControl::Id: {
        using namespace Clusters::AccessControl;
        switch (aPath.mEventId) {

        case Events::AccessControlEntryChanged::Id: {
            Events::AccessControlEntryChanged::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRAccessControlClusterAccessControlEntryChangedEvent * value =
                [MTRAccessControlClusterAccessControlEntryChangedEvent new];

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
                MTRAccessControlClusterAccessControlEntry * _Nullable memberValue;
                if (cppValue.latestValue.IsNull()) {
                    memberValue = nil;
                } else {
                    memberValue = [MTRAccessControlClusterAccessControlEntry new];
                    memberValue.privilege =
                        [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.latestValue.Value().privilege)];
                    memberValue.authMode =
                        [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.latestValue.Value().authMode)];
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

            MTRAccessControlClusterAccessControlExtensionChangedEvent * value =
                [MTRAccessControlClusterAccessControlExtensionChangedEvent new];

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
                MTRAccessControlClusterExtensionEntry * _Nullable memberValue;
                if (cppValue.latestValue.IsNull()) {
                    memberValue = nil;
                } else {
                    memberValue = [MTRAccessControlClusterExtensionEntry new];
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

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Actions::Id: {
        using namespace Clusters::Actions;
        switch (aPath.mEventId) {

        case Events::StateChanged::Id: {
            Events::StateChanged::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRActionsClusterStateChangedEvent * value = [MTRActionsClusterStateChangedEvent new];

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

            MTRActionsClusterActionFailedEvent * value = [MTRActionsClusterActionFailedEvent new];

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

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Basic::Id: {
        using namespace Clusters::Basic;
        switch (aPath.mEventId) {

        case Events::StartUp::Id: {
            Events::StartUp::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRBasicClusterStartUpEvent * value = [MTRBasicClusterStartUpEvent new];

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

            MTRBasicClusterShutDownEvent * value = [MTRBasicClusterShutDownEvent new];

            return value;
        }

        case Events::Leave::Id: {
            Events::Leave::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRBasicClusterLeaveEvent * value = [MTRBasicClusterLeaveEvent new];

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

            MTRBasicClusterReachableChangedEvent * value = [MTRBasicClusterReachableChangedEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithBool:cppValue.reachableNewValue];
                value.reachableNewValue = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OtaSoftwareUpdateProvider::Id: {
        using namespace Clusters::OtaSoftwareUpdateProvider;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OtaSoftwareUpdateRequestor::Id: {
        using namespace Clusters::OtaSoftwareUpdateRequestor;
        switch (aPath.mEventId) {

        case Events::StateTransition::Id: {
            Events::StateTransition::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent * value =
                [MTROtaSoftwareUpdateRequestorClusterStateTransitionEvent new];

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

            MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent * value =
                [MTROtaSoftwareUpdateRequestorClusterVersionAppliedEvent new];

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

            MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent * value =
                [MTROtaSoftwareUpdateRequestorClusterDownloadErrorEvent new];

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

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::LocalizationConfiguration::Id: {
        using namespace Clusters::LocalizationConfiguration;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TimeFormatLocalization::Id: {
        using namespace Clusters::TimeFormatLocalization;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::UnitLocalization::Id: {
        using namespace Clusters::UnitLocalization;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PowerSourceConfiguration::Id: {
        using namespace Clusters::PowerSourceConfiguration;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PowerSource::Id: {
        using namespace Clusters::PowerSource;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::GeneralCommissioning::Id: {
        using namespace Clusters::GeneralCommissioning;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::NetworkCommissioning::Id: {
        using namespace Clusters::NetworkCommissioning;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::DiagnosticLogs::Id: {
        using namespace Clusters::DiagnosticLogs;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::GeneralDiagnostics::Id: {
        using namespace Clusters::GeneralDiagnostics;
        switch (aPath.mEventId) {

        case Events::HardwareFaultChange::Id: {
            Events::HardwareFaultChange::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent * value =
                [MTRGeneralDiagnosticsClusterHardwareFaultChangeEvent new];

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

            MTRGeneralDiagnosticsClusterRadioFaultChangeEvent * value = [MTRGeneralDiagnosticsClusterRadioFaultChangeEvent new];

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

            MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent * value = [MTRGeneralDiagnosticsClusterNetworkFaultChangeEvent new];

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

            MTRGeneralDiagnosticsClusterBootReasonEvent * value = [MTRGeneralDiagnosticsClusterBootReasonEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.bootReason)];
                value.bootReason = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::SoftwareDiagnostics::Id: {
        using namespace Clusters::SoftwareDiagnostics;
        switch (aPath.mEventId) {

        case Events::SoftwareFault::Id: {
            Events::SoftwareFault::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRSoftwareDiagnosticsClusterSoftwareFaultEvent * value = [MTRSoftwareDiagnosticsClusterSoftwareFaultEvent new];

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

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ThreadNetworkDiagnostics::Id: {
        using namespace Clusters::ThreadNetworkDiagnostics;
        switch (aPath.mEventId) {

        case Events::ConnectionStatus::Id: {
            Events::ConnectionStatus::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent * value =
                [MTRThreadNetworkDiagnosticsClusterConnectionStatusEvent new];

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

            MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent * value =
                [MTRThreadNetworkDiagnosticsClusterNetworkFaultChangeEvent new];

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

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::WiFiNetworkDiagnostics::Id: {
        using namespace Clusters::WiFiNetworkDiagnostics;
        switch (aPath.mEventId) {

        case Events::Disconnection::Id: {
            Events::Disconnection::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent * value = [MTRWiFiNetworkDiagnosticsClusterDisconnectionEvent new];

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

            MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent * value =
                [MTRWiFiNetworkDiagnosticsClusterAssociationFailureEvent new];

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

            MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent * value =
                [MTRWiFiNetworkDiagnosticsClusterConnectionStatusEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithUnsignedChar:chip::to_underlying(cppValue.connectionStatus)];
                value.connectionStatus = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::EthernetNetworkDiagnostics::Id: {
        using namespace Clusters::EthernetNetworkDiagnostics;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BridgedDeviceBasic::Id: {
        using namespace Clusters::BridgedDeviceBasic;
        switch (aPath.mEventId) {

        case Events::StartUp::Id: {
            Events::StartUp::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRBridgedDeviceBasicClusterStartUpEvent * value = [MTRBridgedDeviceBasicClusterStartUpEvent new];

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

            MTRBridgedDeviceBasicClusterShutDownEvent * value = [MTRBridgedDeviceBasicClusterShutDownEvent new];

            return value;
        }

        case Events::Leave::Id: {
            Events::Leave::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRBridgedDeviceBasicClusterLeaveEvent * value = [MTRBridgedDeviceBasicClusterLeaveEvent new];

            return value;
        }

        case Events::ReachableChanged::Id: {
            Events::ReachableChanged::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRBridgedDeviceBasicClusterReachableChangedEvent * value = [MTRBridgedDeviceBasicClusterReachableChangedEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithBool:cppValue.reachableNewValue];
                value.reachableNewValue = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Switch::Id: {
        using namespace Clusters::Switch;
        switch (aPath.mEventId) {

        case Events::SwitchLatched::Id: {
            Events::SwitchLatched::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRSwitchClusterSwitchLatchedEvent * value = [MTRSwitchClusterSwitchLatchedEvent new];

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

            MTRSwitchClusterInitialPressEvent * value = [MTRSwitchClusterInitialPressEvent new];

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

            MTRSwitchClusterLongPressEvent * value = [MTRSwitchClusterLongPressEvent new];

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

            MTRSwitchClusterShortReleaseEvent * value = [MTRSwitchClusterShortReleaseEvent new];

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

            MTRSwitchClusterLongReleaseEvent * value = [MTRSwitchClusterLongReleaseEvent new];

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

            MTRSwitchClusterMultiPressOngoingEvent * value = [MTRSwitchClusterMultiPressOngoingEvent new];

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

            MTRSwitchClusterMultiPressCompleteEvent * value = [MTRSwitchClusterMultiPressCompleteEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.newPosition];
                value.newPosition = memberValue;
            } while (0);
            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.totalNumberOfPressesCounted];
                value.totalNumberOfPressesCounted = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AdministratorCommissioning::Id: {
        using namespace Clusters::AdministratorCommissioning;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OperationalCredentials::Id: {
        using namespace Clusters::OperationalCredentials;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::GroupKeyManagement::Id: {
        using namespace Clusters::GroupKeyManagement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::FixedLabel::Id: {
        using namespace Clusters::FixedLabel;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::UserLabel::Id: {
        using namespace Clusters::UserLabel;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BooleanState::Id: {
        using namespace Clusters::BooleanState;
        switch (aPath.mEventId) {

        case Events::StateChange::Id: {
            Events::StateChange::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRBooleanStateClusterStateChangeEvent * value = [MTRBooleanStateClusterStateChangeEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithBool:cppValue.stateValue];
                value.stateValue = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ModeSelect::Id: {
        using namespace Clusters::ModeSelect;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::DoorLock::Id: {
        using namespace Clusters::DoorLock;
        switch (aPath.mEventId) {

        case Events::DoorLockAlarm::Id: {
            Events::DoorLockAlarm::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRDoorLockClusterDoorLockAlarmEvent * value = [MTRDoorLockClusterDoorLockAlarmEvent new];

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

            MTRDoorLockClusterDoorStateChangeEvent * value = [MTRDoorLockClusterDoorStateChangeEvent new];

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

            MTRDoorLockClusterLockOperationEvent * value = [MTRDoorLockClusterLockOperationEvent new];

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
                                MTRDoorLockClusterDlCredential * newElement_2;
                                newElement_2 = [MTRDoorLockClusterDlCredential new];
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

            MTRDoorLockClusterLockOperationErrorEvent * value = [MTRDoorLockClusterLockOperationErrorEvent new];

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
                                MTRDoorLockClusterDlCredential * newElement_2;
                                newElement_2 = [MTRDoorLockClusterDlCredential new];
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

            MTRDoorLockClusterLockUserChangeEvent * value = [MTRDoorLockClusterLockUserChangeEvent new];

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

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::WindowCovering::Id: {
        using namespace Clusters::WindowCovering;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BarrierControl::Id: {
        using namespace Clusters::BarrierControl;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PumpConfigurationAndControl::Id: {
        using namespace Clusters::PumpConfigurationAndControl;
        switch (aPath.mEventId) {

        case Events::SupplyVoltageLow::Id: {
            Events::SupplyVoltageLow::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent * value =
                [MTRPumpConfigurationAndControlClusterSupplyVoltageLowEvent new];

            return value;
        }

        case Events::SupplyVoltageHigh::Id: {
            Events::SupplyVoltageHigh::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent * value =
                [MTRPumpConfigurationAndControlClusterSupplyVoltageHighEvent new];

            return value;
        }

        case Events::PowerMissingPhase::Id: {
            Events::PowerMissingPhase::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent * value =
                [MTRPumpConfigurationAndControlClusterPowerMissingPhaseEvent new];

            return value;
        }

        case Events::SystemPressureLow::Id: {
            Events::SystemPressureLow::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterSystemPressureLowEvent * value =
                [MTRPumpConfigurationAndControlClusterSystemPressureLowEvent new];

            return value;
        }

        case Events::SystemPressureHigh::Id: {
            Events::SystemPressureHigh::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterSystemPressureHighEvent * value =
                [MTRPumpConfigurationAndControlClusterSystemPressureHighEvent new];

            return value;
        }

        case Events::DryRunning::Id: {
            Events::DryRunning::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterDryRunningEvent * value =
                [MTRPumpConfigurationAndControlClusterDryRunningEvent new];

            return value;
        }

        case Events::MotorTemperatureHigh::Id: {
            Events::MotorTemperatureHigh::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent * value =
                [MTRPumpConfigurationAndControlClusterMotorTemperatureHighEvent new];

            return value;
        }

        case Events::PumpMotorFatalFailure::Id: {
            Events::PumpMotorFatalFailure::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent * value =
                [MTRPumpConfigurationAndControlClusterPumpMotorFatalFailureEvent new];

            return value;
        }

        case Events::ElectronicTemperatureHigh::Id: {
            Events::ElectronicTemperatureHigh::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent * value =
                [MTRPumpConfigurationAndControlClusterElectronicTemperatureHighEvent new];

            return value;
        }

        case Events::PumpBlocked::Id: {
            Events::PumpBlocked::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterPumpBlockedEvent * value =
                [MTRPumpConfigurationAndControlClusterPumpBlockedEvent new];

            return value;
        }

        case Events::SensorFailure::Id: {
            Events::SensorFailure::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterSensorFailureEvent * value =
                [MTRPumpConfigurationAndControlClusterSensorFailureEvent new];

            return value;
        }

        case Events::ElectronicNonFatalFailure::Id: {
            Events::ElectronicNonFatalFailure::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent * value =
                [MTRPumpConfigurationAndControlClusterElectronicNonFatalFailureEvent new];

            return value;
        }

        case Events::ElectronicFatalFailure::Id: {
            Events::ElectronicFatalFailure::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent * value =
                [MTRPumpConfigurationAndControlClusterElectronicFatalFailureEvent new];

            return value;
        }

        case Events::GeneralFault::Id: {
            Events::GeneralFault::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterGeneralFaultEvent * value =
                [MTRPumpConfigurationAndControlClusterGeneralFaultEvent new];

            return value;
        }

        case Events::Leakage::Id: {
            Events::Leakage::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterLeakageEvent * value = [MTRPumpConfigurationAndControlClusterLeakageEvent new];

            return value;
        }

        case Events::AirDetection::Id: {
            Events::AirDetection::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterAirDetectionEvent * value =
                [MTRPumpConfigurationAndControlClusterAirDetectionEvent new];

            return value;
        }

        case Events::TurbineOperation::Id: {
            Events::TurbineOperation::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRPumpConfigurationAndControlClusterTurbineOperationEvent * value =
                [MTRPumpConfigurationAndControlClusterTurbineOperationEvent new];

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Thermostat::Id: {
        using namespace Clusters::Thermostat;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::FanControl::Id: {
        using namespace Clusters::FanControl;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ThermostatUserInterfaceConfiguration::Id: {
        using namespace Clusters::ThermostatUserInterfaceConfiguration;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ColorControl::Id: {
        using namespace Clusters::ColorControl;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::BallastConfiguration::Id: {
        using namespace Clusters::BallastConfiguration;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::IlluminanceMeasurement::Id: {
        using namespace Clusters::IlluminanceMeasurement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TemperatureMeasurement::Id: {
        using namespace Clusters::TemperatureMeasurement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::PressureMeasurement::Id: {
        using namespace Clusters::PressureMeasurement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::FlowMeasurement::Id: {
        using namespace Clusters::FlowMeasurement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::RelativeHumidityMeasurement::Id: {
        using namespace Clusters::RelativeHumidityMeasurement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::OccupancySensing::Id: {
        using namespace Clusters::OccupancySensing;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::WakeOnLan::Id: {
        using namespace Clusters::WakeOnLan;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::Channel::Id: {
        using namespace Clusters::Channel;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TargetNavigator::Id: {
        using namespace Clusters::TargetNavigator;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::MediaPlayback::Id: {
        using namespace Clusters::MediaPlayback;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::MediaInput::Id: {
        using namespace Clusters::MediaInput;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::LowPower::Id: {
        using namespace Clusters::LowPower;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::KeypadInput::Id: {
        using namespace Clusters::KeypadInput;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ContentLauncher::Id: {
        using namespace Clusters::ContentLauncher;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AudioOutput::Id: {
        using namespace Clusters::AudioOutput;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ApplicationLauncher::Id: {
        using namespace Clusters::ApplicationLauncher;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ApplicationBasic::Id: {
        using namespace Clusters::ApplicationBasic;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::AccountLogin::Id: {
        using namespace Clusters::AccountLogin;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::ElectricalMeasurement::Id: {
        using namespace Clusters::ElectricalMeasurement;
        switch (aPath.mEventId) {

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    case Clusters::TestCluster::Id: {
        using namespace Clusters::TestCluster;
        switch (aPath.mEventId) {

        case Events::TestEvent::Id: {
            Events::TestEvent::DecodableType cppValue;
            *aError = DataModel::Decode(aReader, cppValue);
            if (*aError != CHIP_NO_ERROR) {
                return nil;
            }

            MTRTestClusterClusterTestEventEvent * value = [MTRTestClusterClusterTestEventEvent new];

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
                MTRTestClusterClusterSimpleStruct * _Nonnull memberValue;
                memberValue = [MTRTestClusterClusterSimpleStruct new];
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
                        MTRTestClusterClusterSimpleStruct * newElement_0;
                        newElement_0 = [MTRTestClusterClusterSimpleStruct new];
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

            MTRTestClusterClusterTestFabricScopedEventEvent * value = [MTRTestClusterClusterTestFabricScopedEventEvent new];

            do {
                NSNumber * _Nonnull memberValue;
                memberValue = [NSNumber numberWithUnsignedChar:cppValue.fabricIndex];
                value.fabricIndex = memberValue;
            } while (0);

            return value;
        }

        default:
            *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
            break;
        }
        break;
    }
    default: {
        *aError = CHIP_ERROR_IM_MALFORMED_EVENT_PATH_IB;
        break;
    }
    }
    return nil;
}
