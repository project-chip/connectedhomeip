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
#include <app/Command.h>
#include <app/util/CHIPDeviceCallbacksMgr.h>
#include <app/util/af-enums.h>
#include <app/util/af.h>
#include <app/util/attribute-list-byte-span.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/SafeInt.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app::List;

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
    case EMBER_ZCL_STATUS_NOT_AUTHORIZED:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_NOT_AUTHORIZED (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_MALFORMED_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_MALFORMED_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUP_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUP_CLUSTER_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUP_GENERAL_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUP_MANUF_CLUSTER_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUP_MANUF_GENERAL_COMMAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_FIELD:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_FIELD (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_VALUE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_VALUE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_READ_ONLY:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_READ_ONLY (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INSUFFICIENT_SPACE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INSUFFICIENT_SPACE (0x%02x)", status);
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
    case EMBER_ZCL_STATUS_INVALID_SELECTOR:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_SELECTOR (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_WRITE_ONLY:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_WRITE_ONLY (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INCONSISTENT_STARTUP_STATE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INCONSISTENT_STARTUP_STATE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_DEFINED_OUT_OF_BAND:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_DEFINED_OUT_Of_BAND (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_ACTION_DENIED:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_ACTION_DENIED (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_TIMEOUT:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_TIMEOUT (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_ABORT:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_ABORT (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_INVALID_IMAGE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_INVALID_IMAGE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_WAIT_FOR_DATA:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_WAIT_FOR_DATA (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_NO_IMAGE_AVAILABLE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_REQUIRE_MORE_IMAGE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_HARDWARE_FAILURE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_HARDWARE_FAILURE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_SOFTWARE_FAILURE:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_SOFTWARE_FAILURE (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_UNSUPPORTED_CLUSTER (0x%02x)", status);
        break;
    case EMBER_ZCL_STATUS_LIMIT_REACHED:
        ChipLogProgress(Zcl, "  status: EMBER_ZCL_STATUS_LIMIT_REACHED (0x%02x)", status);
        break;
    default:
        ChipLogError(Zcl, "Unknow status: 0x%02x", status);
        break;
    }
}

static void LogIMStatus(Protocols::InteractionModel::ProtocolCode status)
{
    switch (status)
    {
    case Protocols::InteractionModel::ProtocolCode::Success:
        ChipLogProgress(Zcl, "  status: Success                (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Failure:
        ChipLogProgress(Zcl, "  status: Failure                (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::InvalidSubscription:
        ChipLogProgress(Zcl, "  status: InvalidSubscription    (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedAccess:
        ChipLogProgress(Zcl, "  status: UnsupportedAccess      (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedEndpoint:
        ChipLogProgress(Zcl, "  status: UnsupportedEndpoint    (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::InvalidAction:
        ChipLogProgress(Zcl, "  status: InvalidAction          (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedCommand:
        ChipLogProgress(Zcl, "  status: UnsupportedCommand     (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated82:
        ChipLogProgress(Zcl, "  status: Deprecated82           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated83:
        ChipLogProgress(Zcl, "  status: Deprecated83           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated84:
        ChipLogProgress(Zcl, "  status: Deprecated84           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::InvalidCommand:
        ChipLogProgress(Zcl, "  status: InvalidCommand         (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedAttribute:
        ChipLogProgress(Zcl, "  status: UnsupportedAttribute   (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::InvalidValue:
        ChipLogProgress(Zcl, "  status: InvalidValue           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedWrite:
        ChipLogProgress(Zcl, "  status: UnsupportedWrite       (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::ResourceExhausted:
        ChipLogProgress(Zcl, "  status: ResourceExhausted      (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated8a:
        ChipLogProgress(Zcl, "  status: Deprecated8a           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::NotFound:
        ChipLogProgress(Zcl, "  status: NotFound               (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnreportableAttribute:
        ChipLogProgress(Zcl, "  status: UnreportableAttribute  (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::InvalidDataType:
        ChipLogProgress(Zcl, "  status: InvalidDataType        (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated8e:
        ChipLogProgress(Zcl, "  status: Deprecated8e           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedRead:
        ChipLogProgress(Zcl, "  status: UnsupportedRead        (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated90:
        ChipLogProgress(Zcl, "  status: Deprecated90           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated91:
        ChipLogProgress(Zcl, "  status: Deprecated91           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved92:
        ChipLogProgress(Zcl, "  status: Reserved92             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecated93:
        ChipLogProgress(Zcl, "  status: Deprecated93           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Timeout:
        ChipLogProgress(Zcl, "  status: Timeout                (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved95:
        ChipLogProgress(Zcl, "  status: Reserved95             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved96:
        ChipLogProgress(Zcl, "  status: Reserved96             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved97:
        ChipLogProgress(Zcl, "  status: Reserved97             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved98:
        ChipLogProgress(Zcl, "  status: Reserved98             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved99:
        ChipLogProgress(Zcl, "  status: Reserved99             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Reserved9a:
        ChipLogProgress(Zcl, "  status: Reserved9a             (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::ConstraintError:
        ChipLogProgress(Zcl, "  status: ConstraintError        (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Busy:
        ChipLogProgress(Zcl, "  status: Busy                   (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecatedc0:
        ChipLogProgress(Zcl, "  status: Deprecatedc0           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecatedc1:
        ChipLogProgress(Zcl, "  status: Deprecatedc1           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecatedc2:
        ChipLogProgress(Zcl, "  status: Deprecatedc2           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::UnsupportedCluster:
        ChipLogProgress(Zcl, "  status: UnsupportedCluster     (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::Deprecatedc4:
        ChipLogProgress(Zcl, "  status: Deprecatedc4           (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::NoUpstreamSubscription:
        ChipLogProgress(Zcl, "  status: NoUpstreamSubscription (0x%04" PRIx16 ")", to_underlying(status));
        break;
    case Protocols::InteractionModel::ProtocolCode::InvalidArgument:
        ChipLogProgress(Zcl, "  status: InvalidArgument        (0x%04" PRIx16 ")", to_underlying(status));
        break;
    default:
        ChipLogError(Zcl, "Unknown status: 0x%04" PRIx16, to_underlying(status));
        break;
    }
}

// Singleton instance of the callbacks manager
static app::CHIPDeviceCallbacksMgr & gCallbacks = app::CHIPDeviceCallbacksMgr::GetInstance();

bool IMDefaultResponseCallback(const app::Command * commandObj, EmberAfStatus status)
{
    ChipLogProgress(Zcl, "DefaultResponse:");
    ChipLogProgress(Zcl, "  Transaction: %p", commandObj);
    LogStatus(status);

    Callback::Cancelable * onSuccessCallback = nullptr;
    Callback::Cancelable * onFailureCallback = nullptr;
    NodeId sourceIdentifier =
        reinterpret_cast<NodeId>(commandObj); /* #6559: Currently, we only have one commands for the IMInvokeCommands and to a
                                                 device, so the seqNum is always set to 0. */

    CHIP_ERROR err = gCallbacks.GetResponseCallback(sourceIdentifier, 0, &onSuccessCallback, &onFailureCallback);

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

bool IMWriteResponseCallback(const chip::app::WriteClient * writeClient, EmberAfStatus status)
{
    ChipLogProgress(Zcl, "WriteResponse:");
    LogStatus(status);

    Callback::Cancelable * onSuccessCallback = nullptr;
    Callback::Cancelable * onFailureCallback = nullptr;
    NodeId sourceNodeId                      = writeClient->GetSourceNodeId();
    uint8_t seq                              = static_cast<uint8_t>(writeClient->GetAppIdentifier());
    CHIP_ERROR err = gCallbacks.GetResponseCallback(sourceNodeId, seq, &onSuccessCallback, &onFailureCallback);

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

bool IMReadReportAttributesResponseCallback(const app::ReadClient * apReadClient, const app::ClusterInfo & aPath,
                                            TLV::TLVReader * apData, Protocols::InteractionModel::ProtocolCode status)
{
    ChipLogProgress(Zcl, "ReadAttributesResponse:");
    ChipLogProgress(Zcl, "  ClusterId: " ChipLogFormatMEI, ChipLogValueMEI(aPath.mClusterId));

    Callback::Cancelable * onSuccessCallback = nullptr;
    Callback::Cancelable * onFailureCallback = nullptr;
    app::TLVDataFilter tlvFilter             = nullptr;
    NodeId sourceId                          = aPath.mNodeId;
    // In CHIPClusters.cpp, we are using sequenceNumber as application identifier.
    uint8_t sequenceNumber = static_cast<uint8_t>(apReadClient->GetAppIdentifier());
    CHIP_ERROR err = gCallbacks.GetResponseCallback(sourceId, sequenceNumber, &onSuccessCallback, &onFailureCallback, &tlvFilter);

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

        if (tlvFilter == nullptr)
        {
            ChipLogDetail(Zcl, "%s: Missing TLV Data Filter", __FUNCTION__);
        }
        return true;
    }

    ChipLogProgress(Zcl, "  attributeId: " ChipLogFormatMEI, ChipLogValueMEI(aPath.mFieldId));
    LogIMStatus(status);

    if (status == Protocols::InteractionModel::ProtocolCode::Success && apData != nullptr)
    {
        ChipLogProgress(Zcl, "  attribute TLV Type: 0x%02x", apData->GetType());
        tlvFilter(apData, onSuccessCallback, onFailureCallback);
    }
    else
    {
        Callback::Callback<DefaultFailureCallback> * cb =
            Callback::Callback<DefaultFailureCallback>::FromCancelable(onFailureCallback);
        // TODO: Should change failure callbacks to accept uint16 status code.
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
