/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

/* This file contains the implementation of the OTARequestor class. All the core
 * OTA Requestor logic is contained in this class.
 */

#include <lib/core/CHIPEncoding.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/OTAImageProcessor.h>
#include <protocols/bdx/BdxUri.h>
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "OTARequestor.h"

namespace chip {

using namespace app;
using namespace app::Clusters;
using namespace app::Clusters::OtaSoftwareUpdateProvider;
using namespace app::Clusters::OtaSoftwareUpdateProvider::Commands;
using namespace app::Clusters::OtaSoftwareUpdateRequestor;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Commands;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Structs;
using app::DataModel::Nullable;
using bdx::TransferSession;

// Global instance of the OTARequestorInterface.
OTARequestorInterface * globalOTARequestorInstance = nullptr;

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

static void LogQueryImageResponse(const QueryImageResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "QueryImageResponse:");
    ChipLogDetail(SoftwareUpdate, "  status: %u", to_underlying(response.status));
    if (response.delayedActionTime.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  delayedActionTime: %" PRIu32 " seconds", response.delayedActionTime.Value());
    }
    if (response.imageURI.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  imageURI: %.*s", static_cast<int>(response.imageURI.Value().size()),
                      response.imageURI.Value().data());
    }
    if (response.softwareVersion.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  softwareVersion: %" PRIu32 "", response.softwareVersion.Value());
    }
    if (response.softwareVersionString.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  softwareVersionString: %.*s",
                      static_cast<int>(response.softwareVersionString.Value().size()),
                      response.softwareVersionString.Value().data());
    }
    if (response.updateToken.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  updateToken: %zu", response.updateToken.Value().size());
    }
    if (response.userConsentNeeded.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  userConsentNeeded: %d", response.userConsentNeeded.Value());
    }
    if (response.metadataForRequestor.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  metadataForRequestor: %zu", response.metadataForRequestor.Value().size());
    }
}

static void LogApplyUpdateResponse(const ApplyUpdateResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "ApplyUpdateResponse:");
    ChipLogDetail(SoftwareUpdate, "  action: %u", to_underlying(response.action));
    ChipLogDetail(SoftwareUpdate, "  delayedActionTime: %" PRIu32 " seconds", response.delayedActionTime);
}

void StartDelayTimerHandler(System::Layer * systemLayer, void * appState)
{
    VerifyOrReturn(appState != nullptr);
    static_cast<OTARequestor *>(appState)->ConnectToProvider(OTARequestor::kQueryImage);
}

void SetRequestorInstance(OTARequestorInterface * instance)
{
    globalOTARequestorInstance = instance;
}

OTARequestorInterface * GetRequestorInstance()
{
    return globalOTARequestorInstance;
}

void OTARequestor::OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    LogQueryImageResponse(response);

    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    switch (response.status)
    {
    case OTAQueryStatus::kUpdateAvailable: {
        UpdateDescription update;
        CHIP_ERROR err = requestorCore->ExtractUpdateDescription(response, update);

        if (err != CHIP_NO_ERROR)
        {
            requestorCore->RecordErrorUpdateState(UpdateFailureState::kQuerying, err);
            return;
        }

        if (update.softwareVersion > requestorCore->mCurrentVersion)
        {
            ChipLogDetail(SoftwareUpdate, "Update available from %" PRIu32 " to %" PRIu32 " version",
                          requestorCore->mCurrentVersion, update.softwareVersion);
            MutableByteSpan updateToken(requestorCore->mUpdateTokenBuffer);
            // This function copies the bytespan to mutablebytespan only if size of mutablebytespan buffer is greater or equal to
            // bytespan otherwise we are copying data upto available size.
            err = CopySpanToMutableSpan(update.updateToken, updateToken);
            if (err == CHIP_ERROR_BUFFER_TOO_SMALL)
            {
                memset(updateToken.data(), 0, updateToken.size());
                requestorCore->GenerateUpdateToken();
            }
            requestorCore->mTargetVersion = update.softwareVersion;
            requestorCore->mUpdateToken   = updateToken;

            requestorCore->mOtaRequestorDriver->UpdateAvailable(update,
                                                                System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        }
        else
        {
            ChipLogDetail(SoftwareUpdate, "Version %" PRIu32 " is older or same than current version %" PRIu32 ", not updating",
                          update.softwareVersion, requestorCore->mCurrentVersion);

            requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::UpToDate,
                                                               System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        }

        break;
    }
    case OTAQueryStatus::kBusy:
        requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::Busy,
                                                           System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kDelayedOnQuery, OTAChangeReasonEnum::kDelayByProvider);
        break;
    case OTAQueryStatus::kNotAvailable:
        requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::NotAvailable,
                                                           System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
        break;
    default:
        requestorCore->RecordErrorUpdateState(UpdateFailureState::kQuerying, CHIP_ERROR_BAD_REQUEST);
        break;
    }
}

void OTARequestor::OnQueryImageFailure(void * context, CHIP_ERROR error)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" CHIP_ERROR_FORMAT, error.Format());
    requestorCore->RecordErrorUpdateState(UpdateFailureState::kQuerying, error);
}

void OTARequestor::OnApplyUpdateResponse(void * context, const ApplyUpdateResponse::DecodableType & response)
{
    LogApplyUpdateResponse(response);

    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    switch (response.action)
    {
    case OTAApplyUpdateAction::kProceed:
        requestorCore->mOtaRequestorDriver->UpdateConfirmed(System::Clock::Seconds32(response.delayedActionTime));
        break;
    case OTAApplyUpdateAction::kAwaitNextAction:
        requestorCore->mOtaRequestorDriver->UpdateSuspended(System::Clock::Seconds32(response.delayedActionTime));
        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kDelayedOnApply, OTAChangeReasonEnum::kDelayByProvider);
        break;
    case OTAApplyUpdateAction::kDiscontinue:
        requestorCore->mOtaRequestorDriver->UpdateDiscontinued();
        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
        break;
    }
}

void OTARequestor::OnApplyUpdateFailure(void * context, CHIP_ERROR error)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "ApplyUpdate failure response %" CHIP_ERROR_FORMAT, error.Format());
    requestorCore->RecordErrorUpdateState(UpdateFailureState::kApplying, error);
}

void OTARequestor::OnNotifyUpdateAppliedResponse(void * context, const app::DataModel::NullObjectType & response) {}

void OTARequestor::OnNotifyUpdateAppliedFailure(void * context, CHIP_ERROR error)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "NotifyUpdateApplied failure response %" CHIP_ERROR_FORMAT, error.Format());
    requestorCore->RecordErrorUpdateState(UpdateFailureState::kNotifying, error);
}

EmberAfStatus OTARequestor::HandleAnnounceOTAProvider(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const AnnounceOtaProvider::DecodableType & commandData)
{
    auto & announcementReason = commandData.announcementReason;

    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    ProviderLocation::Type providerLocation = { .fabricIndex    = commandObj->GetAccessingFabricIndex(),
                                                .providerNodeID = commandData.providerNodeId,
                                                .endpoint       = commandData.endpoint };

    mProviderLocation.SetValue(providerLocation);

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");
    ChipLogDetail(SoftwareUpdate, "  FabricIndex: %u", providerLocation.fabricIndex);
    ChipLogDetail(SoftwareUpdate, "  ProviderNodeID: 0x" ChipLogFormatX64, ChipLogValueX64(providerLocation.providerNodeID));
    ChipLogDetail(SoftwareUpdate, "  VendorID: 0x%" PRIx16, commandData.vendorId);
    ChipLogDetail(SoftwareUpdate, "  AnnouncementReason: %u", to_underlying(announcementReason));
    if (commandData.metadataForNode.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  MetadataForNode: %zu", commandData.metadataForNode.Value().size());
    }
    ChipLogDetail(SoftwareUpdate, "  Endpoint: %" PRIu16, providerLocation.endpoint);

    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelayMs.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t msToStart = 0;
    switch (announcementReason)
    {
    case OTAAnnouncementReason::kSimpleAnnouncement:
    case OTAAnnouncementReason::kUpdateAvailable:
        msToStart = mOtaStartDelayMs;
        break;
    case OTAAnnouncementReason::kUrgentUpdateAvailable:
        msToStart = kImmediateStartDelayMs;
        break;
    default:
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %u", static_cast<uint8_t>(announcementReason));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(msToStart), StartDelayTimerHandler, this);

    return EMBER_ZCL_STATUS_SUCCESS;
}

void OTARequestor::ConnectToProvider(OnConnectedAction onConnectedAction)
{
    VerifyOrReturn(mOtaRequestorDriver != nullptr, ChipLogError(SoftwareUpdate, "OTA requestor driver not set"));
    VerifyOrReturn(mServer != nullptr, ChipLogError(SoftwareUpdate, "Server not set"));
    VerifyOrReturn(mProviderLocation.HasValue(), ChipLogError(SoftwareUpdate, "Provider location not set"));

    FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderLocation.Value().fabricIndex);
    VerifyOrReturn(fabricInfo != nullptr, ChipLogError(SoftwareUpdate, "Cannot find fabric"));

    // Set the action to take once connection is successfully established
    mOnConnectedAction = onConnectedAction;

    ChipLogDetail(SoftwareUpdate, "Establishing session to provider node ID 0x" ChipLogFormatX64 " on fabric index %d",
                  ChipLogValueX64(mProviderLocation.Value().providerNodeID), mProviderLocation.Value().fabricIndex);
    CHIP_ERROR err =
        mCASESessionManager->FindOrEstablishSession(fabricInfo->GetPeerIdForNode(mProviderLocation.Value().providerNodeID),
                                                    &mOnConnectedCallback, &mOnConnectionFailureCallback);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(SoftwareUpdate, "Cannot establish connection to provider: %" CHIP_ERROR_FORMAT, err.Format()));
}

// Application directs the Requestor to cancel image update in progress. All the Requestor state is
// cleared, UpdateState is reset to Idle
void OTARequestor::CancelImageUpdate()
{
    mBdxDownloader->EndDownload(CHIP_ERROR_CONNECTION_ABORTED);

    RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kUnknown);

    // Notify the platform, it might choose to take additional actions
    mOtaRequestorDriver->UpdateCancelled();
}

CHIP_ERROR OTARequestor::GetUpdateProgress(EndpointId endpointId, app::DataModel::Nullable<uint8_t> & progress)
{
    VerifyOrReturnError(OtaRequestorServerGetUpdateStateProgress(endpointId, progress) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_BAD_REQUEST);
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestor::GetState(EndpointId endpointId, OTAUpdateStateEnum & state)
{
    VerifyOrReturnError(OtaRequestorServerGetUpdateState(endpointId, state) == EMBER_ZCL_STATUS_SUCCESS, CHIP_ERROR_BAD_REQUEST);
    return CHIP_NO_ERROR;
}

// Called whenever FindOrEstablishSession is successful
void OTARequestor::OnConnected(void * context, OperationalDeviceProxy * deviceProxy)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);
    VerifyOrDie(deviceProxy != nullptr);

    switch (requestorCore->mOnConnectedAction)
    {
    case kQueryImage: {
        CHIP_ERROR err = requestorCore->SendQueryImageRequest(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send QueryImage command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(UpdateFailureState::kQuerying, err);
            return;
        }

        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kQuerying, OTAChangeReasonEnum::kSuccess);
        break;
    }
    case kStartBDX: {
        CHIP_ERROR err = requestorCore->StartDownload(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to start download: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(UpdateFailureState::kDownloading, err);
            return;
        }

        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kDownloading, OTAChangeReasonEnum::kSuccess);
        break;
    }
    case kApplyUpdate: {
        CHIP_ERROR err = requestorCore->SendApplyUpdateRequest(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send ApplyUpdate command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(UpdateFailureState::kApplying, err);
            return;
        }

        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kApplying, OTAChangeReasonEnum::kSuccess);
        break;
    }
    case kNotifyUpdateApplied: {
        CHIP_ERROR err = requestorCore->SendNotifyUpdateAppliedRequest(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send NotifyUpdateApplied command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(UpdateFailureState::kNotifying, err);
            return;
        }

        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
        break;
    }
    default:
        break;
    }
}

// Called whenever FindOrEstablishSession fails
void OTARequestor::OnConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogError(SoftwareUpdate, "Failed to connect to node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                 ChipLogValueX64(peerId.GetNodeId()), error.Format());

    switch (requestorCore->mOnConnectedAction)
    {
    case kQueryImage:
        requestorCore->RecordErrorUpdateState(UpdateFailureState::kQuerying, error);
        break;
    case kStartBDX:
        requestorCore->RecordErrorUpdateState(UpdateFailureState::kDownloading, error);
        break;
    case kApplyUpdate:
        requestorCore->RecordErrorUpdateState(UpdateFailureState::kApplying, error);
        break;
    default:
        break;
    }
}

OTARequestorInterface::OTATriggerResult OTARequestor::TriggerImmediateQuery()
{
    if (mProviderLocation.HasValue())
    {
        ConnectToProvider(kQueryImage);
        return kTriggerSuccessful;
    }
    else
    {
        ChipLogError(SoftwareUpdate, "No OTA Providers available");
        return kNoProviderKnown;
    }
}

void OTARequestor::DownloadUpdate()
{
    ConnectToProvider(kStartBDX);
}

void OTARequestor::ApplyUpdate()
{
    ConnectToProvider(kApplyUpdate);
}

void OTARequestor::NotifyUpdateApplied(uint32_t version)
{
    // New version is executing so update where applicable
    mCurrentVersion = version;

    // Log the VersionApplied event
    uint16_t productId;
    VerifyOrReturn(DeviceLayer::ConfigurationMgr().GetProductId(productId) == CHIP_NO_ERROR);
    OtaRequestorServerOnVersionApplied(version, productId);

    ConnectToProvider(kNotifyUpdateApplied);
}

CHIP_ERROR OTARequestor::GetDefaultOtaProviderList(AttributeValueEncoder & encoder)
{
    return encoder.EncodeList([&](const auto & enc) -> CHIP_ERROR {
        CHIP_ERROR err = CHIP_NO_ERROR;
        mDefaultOtaProviderList.ForEachActiveObject([&](ProviderLocation::Type * pl) {
            err = enc.Encode(*pl);
            VerifyOrReturnError(err == CHIP_NO_ERROR, Loop::Break);
            return Loop::Continue;
        });
        return err;
    });
}

CHIP_ERROR OTARequestor::ClearDefaultOtaProviderList(FabricIndex fabricIndex)
{
    // Remove all entries for the fabric index indicated
    mDefaultOtaProviderList.ForEachActiveObject([&](ProviderLocation::Type * pl) {
        if (pl->GetFabricIndex() == fabricIndex)
        {
            mDefaultOtaProviderList.ReleaseObject(pl);
        }
        return Loop::Continue;
    });
    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestor::AddDefaultOtaProvider(ProviderLocation::Type const & providerLocation)
{
    // Look for an entry with the same fabric index indicated
    Loop result = mDefaultOtaProviderList.ForEachActiveObject([&](auto * pl) {
        if (pl->GetFabricIndex() == providerLocation.GetFabricIndex())
        {
            return Loop::Break;
        }
        return Loop::Continue;
    });

    // Check if there is already an entry with the same fabric index
    if (result == Loop::Break)
    {
        ChipLogError(SoftwareUpdate, "Default OTA provider entry with fabric %d already exists", providerLocation.GetFabricIndex());
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    ProviderLocation::Type * entry = mDefaultOtaProviderList.CreateObject(providerLocation);
    if (entry == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    // Update the cached provider location if necessary
    UpdateDefaultProviderLocation();

    return CHIP_NO_ERROR;
}

void OTARequestor::OnDownloadStateChanged(OTADownloader::State state, OTAChangeReasonEnum reason)
{
    VerifyOrReturn(mOtaRequestorDriver != nullptr);

    switch (state)
    {
    case OTADownloader::State::kComplete:
        mOtaRequestorDriver->UpdateDownloaded();
        break;
    case OTADownloader::State::kIdle:
        if (reason != OTAChangeReasonEnum::kSuccess)
        {
            RecordErrorUpdateState(UpdateFailureState::kDownloading, CHIP_ERROR_CONNECTION_ABORTED, reason);
        }

        break;
    default:
        break;
    }
}

void OTARequestor::OnUpdateProgressChanged(Nullable<uint8_t> percent)
{
    OtaRequestorServerSetUpdateStateProgress(percent);
}

void OTARequestor::UpdateDefaultProviderLocation()
{
    // If there is currently no cached provider location or there is no OTA update in progress, update for the next periodic query
    if (!mProviderLocation.HasValue() || (mCurrentUpdateState == OTAUpdateStateEnum::kIdle))
    {
        // Make sure to clear any cached provider location that did not come from the default OTA provider list
        mProviderLocation.ClearValue();

        mDefaultOtaProviderList.ForEachActiveObject([&](ProviderLocation::Type * pl) {
            // For now, just find the first available
            mProviderLocation.SetValue(*pl);
            return Loop::Break;
        });
    }
}

void OTARequestor::RecordNewUpdateState(OTAUpdateStateEnum newState, OTAChangeReasonEnum reason)
{
    // Set server UpdateState attribute
    OtaRequestorServerSetUpdateState(newState);

    // The UpdateStateProgress attribute only applies to the downloading state
    if (newState != OTAUpdateStateEnum::kDownloading)
    {
        app::DataModel::Nullable<uint8_t> percent;
        percent.SetNull();
        OtaRequestorServerSetUpdateStateProgress(percent);
    }

    // Log the StateTransition event
    Nullable<OTAUpdateStateEnum> previousState;
    previousState.SetNonNull(mCurrentUpdateState);
    Nullable<uint32_t> targetSoftwareVersion;
    if ((newState == OTAUpdateStateEnum::kDownloading) || (newState == OTAUpdateStateEnum::kApplying) ||
        (newState == OTAUpdateStateEnum::kRollingBack))
    {
        targetSoftwareVersion.SetNonNull(mTargetVersion);
    }
    OtaRequestorServerOnStateTransition(previousState, newState, reason, targetSoftwareVersion);

    mCurrentUpdateState = newState;

    // Update the cached provider location if necessary
    UpdateDefaultProviderLocation();
}

void OTARequestor::RecordErrorUpdateState(UpdateFailureState failureState, CHIP_ERROR error, OTAChangeReasonEnum reason)
{
    // Inform driver of the error
    mOtaRequestorDriver->HandleError(failureState, error);

    // Log the DownloadError event
    OTAImageProcessorInterface * imageProcessor = mBdxDownloader->GetImageProcessorDelegate();
    VerifyOrReturn(imageProcessor != nullptr);
    Nullable<uint8_t> progressPercent = imageProcessor->GetPercentComplete();
    Nullable<int64_t> platformCode;
    OtaRequestorServerOnDownloadError(mTargetVersion, imageProcessor->GetBytesDownloaded(), progressPercent, platformCode);

    // Whenever an error occurs, always reset to Idle state
    RecordNewUpdateState(OTAUpdateStateEnum::kIdle, reason);
}

CHIP_ERROR OTARequestor::GenerateUpdateToken()
{
    if (mUpdateToken.empty())
    {
        VerifyOrReturnError(mServer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);

        FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderLocation.Value().fabricIndex);
        VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

        static_assert(sizeof(NodeId) == sizeof(uint64_t), "Unexpected NodeId size");
        Encoding::BigEndian::Put64(mUpdateTokenBuffer, fabricInfo->GetPeerId().GetNodeId());
        mUpdateToken = ByteSpan(mUpdateTokenBuffer, sizeof(NodeId));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestor::SendQueryImageRequest(OperationalDeviceProxy & deviceProxy)
{
    VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    constexpr OTADownloadProtocol kProtocolsSupported[] = { OTADownloadProtocol::kBDXSynchronous };
    QueryImage::Type args;

    uint16_t vendorId;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetVendorId(vendorId));
    args.vendorId = static_cast<VendorId>(vendorId);

    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetProductId(args.productId));

    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(args.softwareVersion));

    args.protocolsSupported = kProtocolsSupported;
    args.requestorCanConsent.SetValue(mRequestorCanConsent.ValueOr(mOtaRequestorDriver->CanConsent()));

    uint16_t hardwareVersion;
    if (DeviceLayer::ConfigurationMgr().GetHardwareVersion(hardwareVersion) == CHIP_NO_ERROR)
    {
        args.hardwareVersion.SetValue(hardwareVersion);
    }

    char location[DeviceLayer::ConfigurationManager::kMaxLocationLength];
    size_t codeLen = 0;
    if ((DeviceLayer::ConfigurationMgr().GetCountryCode(location, sizeof(location), codeLen) == CHIP_NO_ERROR) && (codeLen == 2))
    {
        args.location.SetValue(CharSpan(location, codeLen));
    }
    else
    {
        // Country code unavailable or invalid, use default
        args.location.SetValue(CharSpan("XX", strlen("XX")));
    }

    Controller::OtaSoftwareUpdateProviderCluster cluster;
    cluster.Associate(&deviceProxy, mProviderLocation.Value().endpoint);

    return cluster.InvokeCommand(args, this, OnQueryImageResponse, OnQueryImageFailure);
}

CHIP_ERROR OTARequestor::ExtractUpdateDescription(const QueryImageResponseDecodableType & response,
                                                  UpdateDescription & update) const
{
    NodeId nodeId;
    CharSpan fileDesignator;

    VerifyOrReturnError(response.imageURI.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(bdx::ParseURI(response.imageURI.Value(), nodeId, fileDesignator));
    update.imageURI = response.imageURI.Value();

    VerifyOrReturnError(response.softwareVersion.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(response.softwareVersionString.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    update.softwareVersion    = response.softwareVersion.Value();
    update.softwareVersionStr = response.softwareVersionString.Value();

    VerifyOrReturnError(response.updateToken.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    update.updateToken = response.updateToken.Value();

    update.userConsentNeeded    = response.userConsentNeeded.ValueOr(false);
    update.metadataForRequestor = response.metadataForRequestor.ValueOr({});

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestor::StartDownload(OperationalDeviceProxy & deviceProxy)
{
    VerifyOrReturnError(mBdxDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // TODO: allow caller to provide their own OTADownloader instance and set BDX parameters

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = mOtaRequestorDriver->GetMaxDownloadBlockSize();
    char testFileDes[9]          = { "test.txt" };
    initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
    initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

    chip::Messaging::ExchangeManager * exchangeMgr = deviceProxy.GetExchangeManager();
    VerifyOrReturnError(exchangeMgr != nullptr, CHIP_ERROR_INCORRECT_STATE);

    Optional<SessionHandle> session = deviceProxy.GetSecureSession();
    VerifyOrReturnError(session.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    mExchangeCtx = exchangeMgr->NewContext(session.Value(), &mBdxMessenger);
    VerifyOrReturnError(mExchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);

    mBdxMessenger.Init(mBdxDownloader, mExchangeCtx);
    mBdxDownloader->SetMessageDelegate(&mBdxMessenger);
    mBdxDownloader->SetStateDelegate(this);

    ReturnErrorOnFailure(mBdxDownloader->SetBDXParams(initOptions));
    return mBdxDownloader->BeginPrepareDownload();
}

CHIP_ERROR OTARequestor::SendApplyUpdateRequest(OperationalDeviceProxy & deviceProxy)
{
    VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(GenerateUpdateToken());

    ApplyUpdateRequest::Type args;
    args.updateToken = mUpdateToken;
    args.newVersion  = mTargetVersion;

    Controller::OtaSoftwareUpdateProviderCluster cluster;
    cluster.Associate(&deviceProxy, mProviderLocation.Value().endpoint);

    return cluster.InvokeCommand(args, this, OnApplyUpdateResponse, OnApplyUpdateFailure);
}

CHIP_ERROR OTARequestor::SendNotifyUpdateAppliedRequest(OperationalDeviceProxy & deviceProxy)
{
    VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(GenerateUpdateToken());

    NotifyUpdateApplied::Type args;
    args.updateToken     = mUpdateToken;
    args.softwareVersion = mCurrentVersion;

    Controller::OtaSoftwareUpdateProviderCluster cluster;
    cluster.Associate(&deviceProxy, mProviderLocation.Value().endpoint);

    return cluster.InvokeCommand(args, this, OnNotifyUpdateAppliedResponse, OnNotifyUpdateAppliedFailure);
}

} // namespace chip
