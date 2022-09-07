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

/* This file contains the implementation of the OTARequestorInterface class. All the core
 * OTA Requestor logic is contained in this class.
 */

#include <app/clusters/basic/basic.h>
#include <app/clusters/ota-requestor/ota-requestor-server.h>
#include <lib/core/CHIPEncoding.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/OTAImageProcessor.h>
#include <protocols/bdx/BdxUri.h>
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "DefaultOTARequestor.h"

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
using Protocols::InteractionModel::Status;

// Global instance of the OTARequestorInterface.
OTARequestorInterface * globalOTARequestorInstance = nullptr;

// Abort the QueryImage download request if there's been no progress for 5 minutes
static constexpr System::Clock::Timeout kDownloadTimeoutSec = chip::System::Clock::Seconds32(5 * 60);

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
        ChipLogDetail(SoftwareUpdate, "  updateToken: %u", static_cast<unsigned int>(response.updateToken.Value().size()));
    }
    if (response.userConsentNeeded.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  userConsentNeeded: %d", response.userConsentNeeded.Value());
    }
    if (response.metadataForRequestor.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  metadataForRequestor: %u",
                      static_cast<unsigned int>(response.metadataForRequestor.Value().size()));
    }
}

static void LogApplyUpdateResponse(const ApplyUpdateResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "ApplyUpdateResponse:");
    ChipLogDetail(SoftwareUpdate, "  action: %u", to_underlying(response.action));
    ChipLogDetail(SoftwareUpdate, "  delayedActionTime: %" PRIu32 " seconds", response.delayedActionTime);
}

void SetRequestorInstance(OTARequestorInterface * instance)
{
    globalOTARequestorInstance = instance;
}

OTARequestorInterface * GetRequestorInstance()
{
    return globalOTARequestorInstance;
}

void DefaultOTARequestor::InitState(intptr_t context)
{
    DefaultOTARequestor * requestorCore = reinterpret_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    // This initialization may occur due to the following:
    //   1) Regular boot up - the states should already be correct
    //   2) Reboot from applying an image - once the image has been confirmed, the provider will be notified of the new version and
    //   all relevant states will reset for a new OTA update. If the image cannot be confirmed, the driver will be responsible for
    //   resetting the states appropriately, including the current update state.
    OtaRequestorServerSetUpdateState(requestorCore->mCurrentUpdateState);
    OtaRequestorServerSetUpdateStateProgress(app::DataModel::NullNullable);
}

CHIP_ERROR DefaultOTARequestor::Init(Server & server, OTARequestorStorage & storage, OTARequestorDriver & driver,
                                     BDXDownloader & downloader)
{
    mServer             = &server;
    mCASESessionManager = server.GetCASESessionManager();
    mStorage            = &storage;
    mOtaRequestorDriver = &driver;
    mBdxDownloader      = &downloader;

    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(mCurrentVersion));

    // Load data from KVS
    LoadCurrentUpdateInfo();

    // Schedule the initializations that needs to be performed in the CHIP context
    DeviceLayer::PlatformMgr().ScheduleWork(InitState, reinterpret_cast<intptr_t>(this));

    return chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(OnCommissioningCompleteRequestor, reinterpret_cast<intptr_t>(this));
}

void DefaultOTARequestor::OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    LogQueryImageResponse(response);

    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    switch (response.status)
    {
    case OTAQueryStatus::kUpdateAvailable: {
        UpdateDescription update;
        CHIP_ERROR err = requestorCore->ExtractUpdateDescription(response, update);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "QueryImageResponse contains invalid fields: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(err);
            return;
        }

        // This should never happen since receiving a response implies that a CASE session had previously been established with a
        // valid provider
        if (!requestorCore->mProviderLocation.HasValue())
        {
            ChipLogError(SoftwareUpdate, "No provider location set");
            requestorCore->RecordErrorUpdateState(CHIP_ERROR_INCORRECT_STATE);
            return;
        }

        // The Operational Node ID in the host field SHALL match the NodeID of the OTA Provider responding with the
        // QueryImageResponse
        if (update.nodeId != requestorCore->mProviderLocation.Value().providerNodeID)
        {
            ChipLogError(SoftwareUpdate,
                         "The ImageURI provider node 0x" ChipLogFormatX64
                         " does not match the QueryImageResponse provider node 0x" ChipLogFormatX64,
                         ChipLogValueX64(update.nodeId), ChipLogValueX64(requestorCore->mProviderLocation.Value().providerNodeID));
            requestorCore->RecordErrorUpdateState(CHIP_ERROR_WRONG_NODE_ID);
            return;
        }

        if (update.softwareVersion > requestorCore->mCurrentVersion)
        {
            ChipLogDetail(SoftwareUpdate, "Update available from version %" PRIu32 " to %" PRIu32, requestorCore->mCurrentVersion,
                          update.softwareVersion);
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

            // Store file designator needed for BDX transfers
            MutableCharSpan fileDesignator(requestorCore->mFileDesignatorBuffer);
            if (update.fileDesignator.size() > fileDesignator.size())
            {
                ChipLogError(SoftwareUpdate, "File designator size %u is too large to store",
                             static_cast<unsigned int>(update.fileDesignator.size()));
                requestorCore->RecordErrorUpdateState(CHIP_ERROR_BUFFER_TOO_SMALL);
                return;
            }
            memcpy(fileDesignator.data(), update.fileDesignator.data(), update.fileDesignator.size());
            fileDesignator.reduce_size(update.fileDesignator.size());
            requestorCore->mFileDesignator = fileDesignator;

            requestorCore->mOtaRequestorDriver->UpdateAvailable(update,
                                                                System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        }
        else
        {
            ChipLogDetail(SoftwareUpdate, "Available update version %" PRIu32 " is <= current version %" PRIu32 ", update ignored",
                          update.softwareVersion, requestorCore->mCurrentVersion);

            requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::kUpToDate,
                                                               System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
            requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
        }

        break;
    }
    case OTAQueryStatus::kBusy: {
        CHIP_ERROR status = requestorCore->mOtaRequestorDriver->UpdateNotFound(
            UpdateNotFoundReason::kBusy, System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        if ((status == CHIP_ERROR_MAX_RETRY_EXCEEDED) || (status == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED))
        {
            requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
        }
        else
        {
            requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kDelayedOnQuery, OTAChangeReasonEnum::kDelayByProvider);
        }

        break;
    }
    case OTAQueryStatus::kNotAvailable: {
        requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::kNotAvailable,
                                                           System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        requestorCore->RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
        break;
    }
    default:
        requestorCore->RecordErrorUpdateState(CHIP_ERROR_BAD_REQUEST);
        break;
    }
}

void DefaultOTARequestor::OnQueryImageFailure(void * context, CHIP_ERROR error)
{
    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogError(SoftwareUpdate, "Received QueryImage failure response: %" CHIP_ERROR_FORMAT, error.Format());

    // A previously valid CASE session may have become invalid
    if (error == CHIP_ERROR_TIMEOUT)
    {
        ChipLogError(SoftwareUpdate, "CASE session may be invalid, tear down session");
        requestorCore->DisconnectFromProvider();
        error = CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY;
    }

    requestorCore->RecordErrorUpdateState(error);
}

void DefaultOTARequestor::OnApplyUpdateResponse(void * context, const ApplyUpdateResponse::DecodableType & response)
{
    LogApplyUpdateResponse(response);

    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
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
    case OTAApplyUpdateAction::kUnknownEnumValue:
        OnApplyUpdateFailure(context, CHIP_ERROR_INVALID_ARGUMENT);
        break;
    }
}

void DefaultOTARequestor::OnApplyUpdateFailure(void * context, CHIP_ERROR error)
{
    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "ApplyUpdate failure response %" CHIP_ERROR_FORMAT, error.Format());
    requestorCore->RecordErrorUpdateState(error);
}

void DefaultOTARequestor::OnNotifyUpdateAppliedResponse(void * context, const app::DataModel::NullObjectType & response) {}

void DefaultOTARequestor::OnNotifyUpdateAppliedFailure(void * context, CHIP_ERROR error)
{
    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "NotifyUpdateApplied failure response %" CHIP_ERROR_FORMAT, error.Format());
    requestorCore->RecordErrorUpdateState(error);
}

void DefaultOTARequestor::Reset()
{
    mProviderLocation.ClearValue();
    mUpdateToken.reduce_size(0);
    RecordNewUpdateState(OTAUpdateStateEnum::kIdle, OTAChangeReasonEnum::kSuccess);
    mTargetVersion = 0;

    // Persist in case of a reboot or crash
    StoreCurrentUpdateInfo();
}

void DefaultOTARequestor::HandleAnnounceOTAProvider(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                    const AnnounceOtaProvider::DecodableType & commandData)
{
    VerifyOrReturn(commandObj != nullptr, ChipLogError(SoftwareUpdate, "Invalid commandObj, cannot handle AnnounceOTAProvider"));

    auto & announcementReason = commandData.announcementReason;

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");

    ProviderLocationType providerLocation = { .providerNodeID = commandData.providerNodeId,
                                              .endpoint       = commandData.endpoint,
                                              .fabricIndex    = commandObj->GetAccessingFabricIndex() };

    ChipLogDetail(SoftwareUpdate, "  FabricIndex: %u", providerLocation.fabricIndex);
    ChipLogDetail(SoftwareUpdate, "  ProviderNodeID: 0x" ChipLogFormatX64, ChipLogValueX64(providerLocation.providerNodeID));
    ChipLogDetail(SoftwareUpdate, "  VendorID: 0x%x", commandData.vendorId);
    ChipLogDetail(SoftwareUpdate, "  AnnouncementReason: %u", to_underlying(announcementReason));
    if (commandData.metadataForNode.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  MetadataForNode: %u",
                      static_cast<unsigned int>(commandData.metadataForNode.Value().size()));
    }
    ChipLogDetail(SoftwareUpdate, "  Endpoint: %u", providerLocation.endpoint);

    mOtaRequestorDriver->ProcessAnnounceOTAProviders(providerLocation, announcementReason);

    commandObj->AddStatus(commandPath, Status::Success);
}

void DefaultOTARequestor::ConnectToProvider(OnConnectedAction onConnectedAction)
{
    VerifyOrDie(mServer != nullptr);

    if (!mProviderLocation.HasValue())
    {
        ChipLogError(SoftwareUpdate, "Provider location not set");
        RecordErrorUpdateState(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    // Set the action to take once connection is successfully established
    mOnConnectedAction = onConnectedAction;

    ChipLogDetail(SoftwareUpdate, "Establishing session to provider node ID 0x" ChipLogFormatX64 " on fabric index %d",
                  ChipLogValueX64(mProviderLocation.Value().providerNodeID), mProviderLocation.Value().fabricIndex);

    mCASESessionManager->FindOrEstablishSession(GetProviderScopedId(), &mOnConnectedCallback, &mOnConnectionFailureCallback);
}

void DefaultOTARequestor::DisconnectFromProvider()
{
    VerifyOrDie(mServer != nullptr);

    if (!mProviderLocation.HasValue())
    {
        ChipLogError(SoftwareUpdate, "Provider location not set");
        RecordErrorUpdateState(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    auto optionalSessionHandle = mSessionHolder.Get();
    if (optionalSessionHandle.HasValue())
    {
        if (optionalSessionHandle.Value()->IsActiveSession())
        {
            optionalSessionHandle.Value()->AsSecureSession()->MarkAsDefunct();
        }
    }
    mSessionHolder.Release();
}

// Requestor is directed to cancel image update in progress. All the Requestor state is
// cleared, UpdateState is reset to Idle
void DefaultOTARequestor::CancelImageUpdate()
{
    mBdxDownloader->EndDownload(CHIP_ERROR_CONNECTION_ABORTED);

    mOtaRequestorDriver->UpdateCancelled();

    Reset();
}

CHIP_ERROR DefaultOTARequestor::GetUpdateStateProgressAttribute(EndpointId endpointId, app::DataModel::Nullable<uint8_t> & progress)
{
    VerifyOrReturnError(OtaRequestorServerGetUpdateStateProgress(endpointId, progress) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_BAD_REQUEST);
    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultOTARequestor::GetUpdateStateAttribute(EndpointId endpointId, OTAUpdateStateEnum & state)
{
    VerifyOrReturnError(OtaRequestorServerGetUpdateState(endpointId, state) == EMBER_ZCL_STATUS_SUCCESS, CHIP_ERROR_BAD_REQUEST);
    return CHIP_NO_ERROR;
}

// Called whenever FindOrEstablishSession is successful
void DefaultOTARequestor::OnConnected(void * context, Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);
    requestorCore->mSessionHolder.Grab(sessionHandle);

    switch (requestorCore->mOnConnectedAction)
    {
    case kQueryImage: {
        CHIP_ERROR err = requestorCore->SendQueryImageRequest(exchangeMgr, sessionHandle);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send QueryImage command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(err);
            return;
        }
        break;
    }
    case kDownload: {
        CHIP_ERROR err = requestorCore->StartDownload(exchangeMgr, sessionHandle);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to start download: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(err);
            return;
        }
        break;
    }
    case kApplyUpdate: {
        CHIP_ERROR err = requestorCore->SendApplyUpdateRequest(exchangeMgr, sessionHandle);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send ApplyUpdate command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(err);
            return;
        }
        break;
    }
    case kNotifyUpdateApplied: {
        CHIP_ERROR err = requestorCore->SendNotifyUpdateAppliedRequest(exchangeMgr, sessionHandle);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send NotifyUpdateApplied command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->RecordErrorUpdateState(err);
            return;
        }
        break;
    }
    default:
        break;
    }
}

// Called whenever FindOrEstablishSession fails
void DefaultOTARequestor::OnConnectionFailure(void * context, const ScopedNodeId & peerId, CHIP_ERROR error)
{
    DefaultOTARequestor * requestorCore = static_cast<DefaultOTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);
    requestorCore->mSessionHolder.Release();

    ChipLogError(SoftwareUpdate, "Failed to connect to node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                 ChipLogValueX64(peerId.GetNodeId()), error.Format());

    switch (requestorCore->mOnConnectedAction)
    {
    case kQueryImage:
    case kDownload:
    case kApplyUpdate:
    case kNotifyUpdateApplied:
        requestorCore->RecordErrorUpdateState(error);
        break;
    default:
        break;
    }
}

void DefaultOTARequestor::TriggerImmediateQueryInternal()
{
    // We are now connecting to a provider for the purpose of sending a QueryImage,
    // treat this as a move to the Querying state
    RecordNewUpdateState(OTAUpdateStateEnum::kQuerying, OTAChangeReasonEnum::kSuccess);

    ConnectToProvider(kQueryImage);
}

CHIP_ERROR DefaultOTARequestor::TriggerImmediateQuery(FabricIndex fabricIndex)
{
    ProviderLocationType providerLocation;
    bool providerFound = false;

    if (fabricIndex == kUndefinedFabricIndex)
    {
        bool listExhausted = false;
        providerFound      = mOtaRequestorDriver->GetNextProviderLocation(providerLocation, listExhausted);
    }
    else
    {
        for (auto providerIter = mDefaultOtaProviderList.Begin(); providerIter.Next();)
        {
            providerLocation = providerIter.GetValue();

            if (providerLocation.GetFabricIndex() == fabricIndex)
            {
                providerFound = true;
                break;
            }
        }
    }

    if (!providerFound)
    {
        ChipLogError(SoftwareUpdate, "No OTA Providers available for immediate query");
        return CHIP_ERROR_NOT_FOUND;
    }

    SetCurrentProviderLocation(providerLocation);

    // Go through the driver as it has additional logic to execute
    mOtaRequestorDriver->SendQueryImage();

    ChipLogProgress(SoftwareUpdate, "Triggered immediate OTA query for fabric: 0x%x",
                    static_cast<unsigned>(providerLocation.GetFabricIndex()));

    return CHIP_NO_ERROR;
}

void DefaultOTARequestor::DownloadUpdate()
{
    RecordNewUpdateState(OTAUpdateStateEnum::kDownloading, OTAChangeReasonEnum::kSuccess);
    ConnectToProvider(kDownload);
}

void DefaultOTARequestor::DownloadUpdateDelayedOnUserConsent()
{
    RecordNewUpdateState(OTAUpdateStateEnum::kDelayedOnUserConsent, OTAChangeReasonEnum::kSuccess);
}

void DefaultOTARequestor::ApplyUpdate()
{
    RecordNewUpdateState(OTAUpdateStateEnum::kApplying, OTAChangeReasonEnum::kSuccess);

    // If image is successfully applied, the device will reboot so persist all relevant data
    StoreCurrentUpdateInfo();

    ConnectToProvider(kApplyUpdate);
}

void DefaultOTARequestor::NotifyUpdateApplied()
{
    // Log the VersionApplied event
    uint16_t productId;
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(productId) != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot get Product ID");
        RecordErrorUpdateState(CHIP_ERROR_INCORRECT_STATE);
        return;
    }

    OtaRequestorServerOnVersionApplied(mCurrentVersion, productId);

    ConnectToProvider(kNotifyUpdateApplied);
}

CHIP_ERROR DefaultOTARequestor::ClearDefaultOtaProviderList(FabricIndex fabricIndex)
{
    CHIP_ERROR error = mDefaultOtaProviderList.Delete(fabricIndex);

    // Ignore the error if no entry for the associated fabric index has been found.
    ReturnErrorCodeIf(error == CHIP_ERROR_NOT_FOUND, CHIP_NO_ERROR);
    ReturnErrorOnFailure(error);

    return mStorage->StoreDefaultProviders(mDefaultOtaProviderList);
}

CHIP_ERROR DefaultOTARequestor::AddDefaultOtaProvider(const ProviderLocationType & providerLocation)
{
    // Look for an entry with the same fabric index indicated
    auto iterator = mDefaultOtaProviderList.Begin();
    while (iterator.Next())
    {
        ProviderLocationType pl = iterator.GetValue();
        if (pl.GetFabricIndex() == providerLocation.GetFabricIndex())
        {
            ChipLogError(SoftwareUpdate, "Default OTA provider entry with fabric %d already exists", pl.GetFabricIndex());
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
    }

    ReturnErrorOnFailure(mDefaultOtaProviderList.Add(providerLocation));

    return mStorage->StoreDefaultProviders(mDefaultOtaProviderList);
}

void DefaultOTARequestor::OnDownloadStateChanged(OTADownloader::State state, OTAChangeReasonEnum reason)
{
    VerifyOrDie(mOtaRequestorDriver != nullptr);

    switch (state)
    {
    case OTADownloader::State::kComplete:
        mOtaRequestorDriver->UpdateDownloaded();
        mBdxMessenger.Reset();
        break;
    case OTADownloader::State::kIdle:
        if (reason != OTAChangeReasonEnum::kSuccess)
        {
            RecordErrorUpdateState(CHIP_ERROR_CONNECTION_ABORTED, reason);
        }
        mBdxMessenger.Reset();
        break;
    default:
        break;
    }
}

void DefaultOTARequestor::OnUpdateProgressChanged(Nullable<uint8_t> percent)
{
    OtaRequestorServerSetUpdateStateProgress(percent);
}

IdleStateReason DefaultOTARequestor::MapErrorToIdleStateReason(CHIP_ERROR error)
{
    if (error == CHIP_NO_ERROR)
    {
        return IdleStateReason::kIdle;
    }
    if (error == CHIP_ERROR_CONNECTION_CLOSED_UNEXPECTEDLY)
    {
        return IdleStateReason::kInvalidSession;
    }

    return IdleStateReason::kUnknown;
}

void DefaultOTARequestor::RecordNewUpdateState(OTAUpdateStateEnum newState, OTAChangeReasonEnum reason, CHIP_ERROR error)
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
    Nullable<uint32_t> targetSoftwareVersion;
    if ((newState == OTAUpdateStateEnum::kDownloading) || (newState == OTAUpdateStateEnum::kApplying) ||
        (newState == OTAUpdateStateEnum::kRollingBack))
    {
        targetSoftwareVersion.SetNonNull(mTargetVersion);
    }
    OtaRequestorServerOnStateTransition(mCurrentUpdateState, newState, reason, targetSoftwareVersion);

    OTAUpdateStateEnum prevState = mCurrentUpdateState;
    // Update the new state before handling the state transition
    mCurrentUpdateState = newState;

    if ((newState == OTAUpdateStateEnum::kIdle) && (prevState != OTAUpdateStateEnum::kIdle))
    {
        IdleStateReason idleStateReason = MapErrorToIdleStateReason(error);
        mOtaRequestorDriver->HandleIdleStateEnter(idleStateReason);
    }
    else if ((prevState == OTAUpdateStateEnum::kIdle) && (newState != OTAUpdateStateEnum::kIdle))
    {
        mOtaRequestorDriver->HandleIdleStateExit();
    }
}

void DefaultOTARequestor::RecordErrorUpdateState(CHIP_ERROR error, OTAChangeReasonEnum reason)
{
    // Log the DownloadError event
    OTAImageProcessorInterface * imageProcessor = mBdxDownloader->GetImageProcessorDelegate();
    VerifyOrDie(imageProcessor != nullptr);
    Nullable<uint8_t> progressPercent = imageProcessor->GetPercentComplete();
    Nullable<int64_t> platformCode;
    OtaRequestorServerOnDownloadError(mTargetVersion, imageProcessor->GetBytesDownloaded(), progressPercent, platformCode);

    // Whenever an error occurs, always reset to Idle state
    RecordNewUpdateState(OTAUpdateStateEnum::kIdle, reason, error);
}

CHIP_ERROR DefaultOTARequestor::GenerateUpdateToken()
{
    if (mUpdateToken.empty())
    {
        VerifyOrReturnError(mServer != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);

        const FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderLocation.Value().fabricIndex);
        VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

        static_assert(sizeof(NodeId) == sizeof(uint64_t), "Unexpected NodeId size");
        Encoding::BigEndian::Put64(mUpdateTokenBuffer, fabricInfo->GetPeerId().GetNodeId());
        mUpdateToken = ByteSpan(mUpdateTokenBuffer, sizeof(NodeId));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR DefaultOTARequestor::SendQueryImageRequest(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    constexpr OTADownloadProtocol kProtocolsSupported[] = { OTADownloadProtocol::kBDXSynchronous };
    QueryImage::Type args;

    uint16_t vendorId;
    ReturnErrorOnFailure(DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(vendorId));
    args.vendorId = static_cast<VendorId>(vendorId);

    ReturnErrorOnFailure(DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(args.productId));

    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSoftwareVersion(args.softwareVersion));

    args.protocolsSupported = kProtocolsSupported;
    args.requestorCanConsent.SetValue(!Basic::IsLocalConfigDisabled() && mOtaRequestorDriver->CanConsent());

    uint16_t hardwareVersion;
    if (DeviceLayer::GetDeviceInstanceInfoProvider()->GetHardwareVersion(hardwareVersion) == CHIP_NO_ERROR)
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

    args.metadataForProvider = mMetadataForProvider;
    Controller::OtaSoftwareUpdateProviderCluster cluster(exchangeMgr, sessionHandle, mProviderLocation.Value().endpoint);

    return cluster.InvokeCommand(args, this, OnQueryImageResponse, OnQueryImageFailure);
}

CHIP_ERROR DefaultOTARequestor::ExtractUpdateDescription(const QueryImageResponseDecodableType & response,
                                                         UpdateDescription & update) const
{
    NodeId nodeId;
    CharSpan fileDesignator;

    VerifyOrReturnError(response.imageURI.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
    ReturnErrorOnFailure(bdx::ParseURI(response.imageURI.Value(), nodeId, fileDesignator));
    VerifyOrReturnError(IsSpanUsable(fileDesignator), CHIP_ERROR_INVALID_ARGUMENT);
    update.nodeId         = nodeId;
    update.fileDesignator = fileDesignator;

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

CHIP_ERROR DefaultOTARequestor::StartDownload(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    VerifyOrReturnError(mBdxDownloader != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // TODO: allow caller to provide their own OTADownloader instance and set BDX parameters

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = mOtaRequestorDriver->GetMaxDownloadBlockSize();
    initOptions.FileDesLength    = static_cast<uint16_t>(mFileDesignator.size());
    initOptions.FileDesignator   = reinterpret_cast<const uint8_t *>(mFileDesignator.data());

    chip::Messaging::ExchangeContext * exchangeCtx = exchangeMgr.NewContext(sessionHandle, &mBdxMessenger);
    VerifyOrReturnError(exchangeCtx != nullptr, CHIP_ERROR_NO_MEMORY);

    mBdxMessenger.Init(mBdxDownloader, exchangeCtx);
    mBdxDownloader->SetMessageDelegate(&mBdxMessenger);
    mBdxDownloader->SetStateDelegate(this);

    CHIP_ERROR err = mBdxDownloader->SetBDXParams(initOptions, kDownloadTimeoutSec);
    if (err == CHIP_NO_ERROR)
    {
        err = mBdxDownloader->BeginPrepareDownload();
    }

    if (err != CHIP_NO_ERROR)
    {
        mBdxMessenger.Reset();
    }

    return err;
}

CHIP_ERROR DefaultOTARequestor::SendApplyUpdateRequest(Messaging::ExchangeManager & exchangeMgr, SessionHandle & sessionHandle)
{
    VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(GenerateUpdateToken());

    ApplyUpdateRequest::Type args;
    args.updateToken = mUpdateToken;
    args.newVersion  = mTargetVersion;

    Controller::OtaSoftwareUpdateProviderCluster cluster(exchangeMgr, sessionHandle, mProviderLocation.Value().endpoint);

    return cluster.InvokeCommand(args, this, OnApplyUpdateResponse, OnApplyUpdateFailure);
}

CHIP_ERROR DefaultOTARequestor::SendNotifyUpdateAppliedRequest(Messaging::ExchangeManager & exchangeMgr,
                                                               SessionHandle & sessionHandle)
{
    VerifyOrReturnError(mProviderLocation.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(GenerateUpdateToken());

    NotifyUpdateApplied::Type args;
    args.updateToken     = mUpdateToken;
    args.softwareVersion = mCurrentVersion;

    Controller::OtaSoftwareUpdateProviderCluster cluster(exchangeMgr, sessionHandle, mProviderLocation.Value().endpoint);

    // There is no response for a notify so consider this OTA complete. Clear the provider location and reset any states to indicate
    // so.
    Reset();

    return cluster.InvokeCommand(args, this, OnNotifyUpdateAppliedResponse, OnNotifyUpdateAppliedFailure);
}

void DefaultOTARequestor::StoreCurrentUpdateInfo()
{
    // TODO: change OTA requestor storage interface to store both values at once
    CHIP_ERROR error = CHIP_NO_ERROR;
    if (mProviderLocation.HasValue())
    {
        error = mStorage->StoreCurrentProviderLocation(mProviderLocation.Value());
    }
    else
    {
        error = mStorage->ClearCurrentProviderLocation();
    }

    if ((error == CHIP_NO_ERROR) || (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
    {
        if (mUpdateToken.size() > 0)
        {
            error = mStorage->StoreUpdateToken(mUpdateToken);
        }
        else
        {
            error = mStorage->ClearUpdateToken();
        }
    }

    if ((error == CHIP_NO_ERROR) || (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
    {
        error = mStorage->StoreCurrentUpdateState(mCurrentUpdateState);
    }

    if ((error == CHIP_NO_ERROR) || (error == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
    {
        if (mTargetVersion > 0)
        {
            error = mStorage->StoreTargetVersion(mTargetVersion);
        }
        else
        {
            error = mStorage->ClearTargetVersion();
        }
    }

    if ((error != CHIP_NO_ERROR) && (error != CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND))
    {
        ChipLogError(SoftwareUpdate, "Failed to store current update: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void DefaultOTARequestor::LoadCurrentUpdateInfo()
{
    mStorage->LoadDefaultProviders(mDefaultOtaProviderList);

    ProviderLocationType providerLocation;
    if (mStorage->LoadCurrentProviderLocation(providerLocation) == CHIP_NO_ERROR)
    {
        mProviderLocation.SetValue(providerLocation);
    }

    MutableByteSpan updateToken(mUpdateTokenBuffer);
    if (mStorage->LoadUpdateToken(updateToken) == CHIP_NO_ERROR)
    {
        mUpdateToken = updateToken;
    }

    if (mStorage->LoadCurrentUpdateState(mCurrentUpdateState) != CHIP_NO_ERROR)
    {
        mCurrentUpdateState = OTAUpdateStateEnum::kIdle;
    }

    if (mStorage->LoadTargetVersion(mTargetVersion) != CHIP_NO_ERROR)
    {
        mTargetVersion = 0;
    }
}

// Invoked when the device becomes commissioned
void DefaultOTARequestor::OnCommissioningCompleteRequestor(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    VerifyOrReturn(event->Type == DeviceLayer::DeviceEventType::kCommissioningComplete);

    ChipLogProgress(SoftwareUpdate, "Device commissioned, schedule a default provider query");

    // TODO: Should we also send UpdateApplied here?

    // Schedule a query. At the end of this query/update process the Default Provider timer is started
    OTARequestorDriver * driver = (reinterpret_cast<DefaultOTARequestor *>(arg))->mOtaRequestorDriver;
    driver->OTACommissioningCallback();
}

} // namespace chip
