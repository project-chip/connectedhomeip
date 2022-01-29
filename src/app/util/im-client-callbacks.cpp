/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/util/im-client-callbacks.h>

#include <cinttypes>

#include <app-common/zap-generated/enums.h>
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;

void LogStatus(uint8_t status)
{
    switch (status)
    {
    case EMBER_ZCL_STATUS_SUCCESS:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_SUCCESS (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_FAILURE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_FAILURE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_SUBSCRIPTION:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_SUBSCRIPTION (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_ACCESS (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_ENDPOINT (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_MALFORMED_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_MALFORMED_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED82:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED82 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED83:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED83 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED84:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED84 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_VALUE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_VALUE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_WRITE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_WRITE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESOURCE_EXHAUSTED (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DUPLICATE_EXISTS:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DUPLICATE_EXISTS (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_NOT_FOUND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_NOT_FOUND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNREPORTABLE_ATTRIBUTE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_DATA_TYPE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_DATA_TYPE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED8E:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED8E (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_READ:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_READ (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED90:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED90 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED91:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED91 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DATA_VERSION_MISMATCH:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DATA_VERSION_MISMATCH (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATED93:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATED93 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_TIMEOUT:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_TIMEOUT (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESERVED95:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESERVED95 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESERVED96:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESERVED96 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESERVED97:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESERVED97 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESERVED98:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESERVED98 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESERVED99:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESERVED99 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_RESERVED9A:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_RESERVED9A (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_BUSY:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_BUSY (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATEDC0:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATEDC0 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATEDC1:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATEDC1 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEPRECATEDC4:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEPRECATEDC4 (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_NO_UPSTREAM_SUBSCRIPTION:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_NO_UPSTREAM_SUBSCRIPTION (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_NEEDS_TIMED_INTERACTION:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_NEEDS_TIMED_INTERACTION (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_EVENT:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_EVENT (0x%02x)", status);
        break;
    default:
        ChipLogError(Zcl, "Unknown status: 0x%02x", status);
        break;
    }
}

static void LogIMStatus(Protocols::InteractionModel::Status status)
{
    switch (status)
    {
    case Protocols::InteractionModel::Status::Success:
        ChipLogProgress(Zcl, "  status: Success                (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Failure:
        ChipLogProgress(Zcl, "  status: Failure                (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::InvalidSubscription:
        ChipLogProgress(Zcl, "  status: InvalidSubscription    (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedAccess:
        ChipLogProgress(Zcl, "  status: UnsupportedAccess      (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedEndpoint:
        ChipLogProgress(Zcl, "  status: UnsupportedEndpoint    (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::InvalidAction:
        ChipLogProgress(Zcl, "  status: InvalidAction          (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedCommand:
        ChipLogProgress(Zcl, "  status: UnsupportedCommand     (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated82:
        ChipLogProgress(Zcl, "  status: Deprecated82           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated83:
        ChipLogProgress(Zcl, "  status: Deprecated83           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated84:
        ChipLogProgress(Zcl, "  status: Deprecated84           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::InvalidCommand:
        ChipLogProgress(Zcl, "  status: InvalidCommand         (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedAttribute:
        ChipLogProgress(Zcl, "  status: UnsupportedAttribute   (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::ConstraintError:
        ChipLogProgress(Zcl, "  status: InvalidValue           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedWrite:
        ChipLogProgress(Zcl, "  status: UnsupportedWrite       (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::ResourceExhausted:
        ChipLogProgress(Zcl, "  status: ResourceExhausted      (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated8a:
        ChipLogProgress(Zcl, "  status: Deprecated8a           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::NotFound:
        ChipLogProgress(Zcl, "  status: NotFound               (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnreportableAttribute:
        ChipLogProgress(Zcl, "  status: UnreportableAttribute  (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::InvalidDataType:
        ChipLogProgress(Zcl, "  status: InvalidDataType        (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated8e:
        ChipLogProgress(Zcl, "  status: Deprecated8e           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedRead:
        ChipLogProgress(Zcl, "  status: UnsupportedRead        (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated90:
        ChipLogProgress(Zcl, "  status: Deprecated90           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated91:
        ChipLogProgress(Zcl, "  status: Deprecated91           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::DataVersionMismatch:
        ChipLogProgress(Zcl, "  status: DataVersionMismatch    (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecated93:
        ChipLogProgress(Zcl, "  status: Deprecated93           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Timeout:
        ChipLogProgress(Zcl, "  status: Timeout                (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Reserved95:
        ChipLogProgress(Zcl, "  status: Reserved95             (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Reserved96:
        ChipLogProgress(Zcl, "  status: Reserved96             (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Reserved97:
        ChipLogProgress(Zcl, "  status: Reserved97             (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Reserved98:
        ChipLogProgress(Zcl, "  status: Reserved98             (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Reserved99:
        ChipLogProgress(Zcl, "  status: Reserved99             (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Reserved9a:
        ChipLogProgress(Zcl, "  status: Reserved9a             (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Busy:
        ChipLogProgress(Zcl, "  status: Busy                   (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecatedc0:
        ChipLogProgress(Zcl, "  status: Deprecatedc0           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecatedc1:
        ChipLogProgress(Zcl, "  status: Deprecatedc1           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecatedc2:
        ChipLogProgress(Zcl, "  status: Deprecatedc2           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedCluster:
        ChipLogProgress(Zcl, "  status: UnsupportedCluster     (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::Deprecatedc4:
        ChipLogProgress(Zcl, "  status: Deprecatedc4           (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::NoUpstreamSubscription:
        ChipLogProgress(Zcl, "  status: NoUpstreamSubscription (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::NeedsTimedInteraction:
        ChipLogProgress(Zcl, "  status: NeedsTimedInteraction  (0x%02" PRIx8 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::Status::UnsupportedEvent:
        ChipLogProgress(Zcl, "  status: UnsupportedEvent       (0x%02" PRIx8 ")", to_underlying(status));
        break;
    default:
        ChipLogError(Zcl, "Unknown status: 0x%02" PRIx8, to_underlying(status));
        break;
    }
}

// Singleton instance of the callbacks manager
static app::CHIPDeviceCallbacksMgr & gCallbacks = app::CHIPDeviceCallbacksMgr::GetInstance();

bool IMWriteResponseCallback(const chip::app::WriteClient * writeClient, chip::Protocols::InteractionModel::Status status)
{
    ChipLogProgress(Zcl, "WriteResponse:");
    LogIMStatus(status);

    Callback::Cancelable * onSuccessCallback = nullptr;
    Callback::Cancelable * onFailureCallback = nullptr;
    CHIP_ERROR err =
        gCallbacks.GetResponseCallback(reinterpret_cast<NodeId>(writeClient), 0, &onSuccessCallback, &onFailureCallback);

    if (CHIP_NO_ERROR != err)
    {
        if (onSuccessCallback == nullptr)
        {
            ChipLogDetail(Zcl, "%s: Missing success callback", __FUNCTION__);
        }

        if (onFailureCallback == nullptr)
        {
            ChipLogDetail(Zcl, "%s: Missing failure callback", __FUNCTION__);
        }

        return true;
    }

    if (status == Protocols::InteractionModel::Status::Success)
    {
        Callback::Callback<DefaultSuccessCallback> * cb =
            Callback::Callback<DefaultSuccessCallback>::FromCancelable(onSuccessCallback);
        cb->mCall(cb->mContext);
    }
    else
    {
        Callback::Callback<DefaultFailureCallback> * cb =
            Callback::Callback<DefaultFailureCallback>::FromCancelable(onFailureCallback);
        cb->mCall(cb->mContext, static_cast<uint8_t>(to_underlying(status)));
    }

    return true;
}

bool emberAfDefaultResponseCallback(ClusterId clusterId, CommandId commandId, EmberAfStatus status)
{
    ChipLogProgress(Zcl, "DefaultResponse:");
    ChipLogProgress(Zcl, "  ClusterId: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));
    ChipLogProgress(Zcl, "  CommandId: " ChipLogFormatMEI, ChipLogValueMEI(commandId));
    LogStatus(status);

    Callback::Cancelable * onSuccessCallback = nullptr;
    Callback::Cancelable * onFailureCallback = nullptr;
    NodeId sourceId                          = emberAfCurrentCommand()->SourceNodeId();
    uint8_t sequenceNumber                   = emberAfCurrentCommand()->seqNum;
    CHIP_ERROR err = gCallbacks.GetResponseCallback(sourceId, sequenceNumber, &onSuccessCallback, &onFailureCallback);

    if (CHIP_NO_ERROR != err)
    {
        if (onSuccessCallback == nullptr)
        {
            ChipLogDetail(Zcl, "%s: Missing success callback", __FUNCTION__);
        }

        if (onFailureCallback == nullptr)
        {
            ChipLogDetail(Zcl, "%s: Missing failure callback", __FUNCTION__);
        }

        return true;
    }

    if (status == EMBER_ZCL_STATUS_SUCCESS)
    {
        Callback::Callback<DefaultSuccessCallback> * cb =
            Callback::Callback<DefaultSuccessCallback>::FromCancelable(onSuccessCallback);
        cb->mCall(cb->mContext);
    }
    else
    {
        Callback::Callback<DefaultFailureCallback> * cb =
            Callback::Callback<DefaultFailureCallback>::FromCancelable(onFailureCallback);
        cb->mCall(cb->mContext, static_cast<uint8_t>(status));
    }

    return true;
}
