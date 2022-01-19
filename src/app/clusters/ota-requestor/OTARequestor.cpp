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

using namespace app::Clusters;
using namespace app::Clusters::OtaSoftwareUpdateProvider;
using namespace app::Clusters::OtaSoftwareUpdateProvider::Commands;
using namespace app::Clusters::OtaSoftwareUpdateRequestor;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Commands;
using bdx::TransferSession;

// Global instance of the OTARequestorInterface.
OTARequestorInterface * globalOTARequestorInstance = nullptr;

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

static void LogQueryImageResponse(const QueryImageResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "QueryImageResponse:");
    ChipLogDetail(SoftwareUpdate, "  status: %" PRIu8 "", to_underlying(response.status));
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
    ChipLogDetail(SoftwareUpdate, "  action: %" PRIu8 "", to_underlying(response.action));
    ChipLogDetail(SoftwareUpdate, "  delayedActionTime: %" PRIu32 " seconds", response.delayedActionTime);
}

static void SetUpdateStateAttribute(OTAUpdateStateEnum state)
{
    OtaRequestorServerSetUpdateState(state);

    // The UpdateStateProgress attribute only applies to the querying state
    if (state != OTAUpdateStateEnum::kQuerying)
    {
        OtaRequestorServerSetUpdateStateProgress(0);
    }
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
            requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kQuerying, err);
            return;
        }

        MutableByteSpan updateToken(requestorCore->mUpdateTokenBuffer);
        CopySpanToMutableSpan(update.updateToken, updateToken);
        requestorCore->mUpdateVersion = update.softwareVersion;
        requestorCore->mUpdateToken   = updateToken;

        requestorCore->mOtaRequestorDriver->UpdateAvailable(update,
                                                            System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        break;
    }
    case OTAQueryStatus::kBusy:
        requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::Busy,
                                                           System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        SetUpdateStateAttribute(OTAUpdateStateEnum::kDelayedOnQuery);
        break;
    case OTAQueryStatus::kNotAvailable:
        requestorCore->mOtaRequestorDriver->UpdateNotFound(UpdateNotFoundReason::NotAvailable,
                                                           System::Clock::Seconds32(response.delayedActionTime.ValueOr(0)));
        SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
        break;
    default:
        requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kQuerying, CHIP_ERROR_BAD_REQUEST);
        SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
        break;
    }
}

void OTARequestor::OnQueryImageFailure(void * context, EmberAfStatus status)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
    requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kQuerying, CHIP_ERROR_BAD_REQUEST);
    SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
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
        SetUpdateStateAttribute(OTAUpdateStateEnum::kDelayedOnApply);
        break;
    case OTAApplyUpdateAction::kDiscontinue:
        requestorCore->mOtaRequestorDriver->UpdateDiscontinued();
        SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
        break;
    }
}

void OTARequestor::OnApplyUpdateFailure(void * context, EmberAfStatus status)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "ApplyUpdate failure response %" PRIu8, status);
    requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kApplying, CHIP_ERROR_BAD_REQUEST);
    SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
}

void OTARequestor::OnNotifyUpdateAppliedResponse(void * context, const app::DataModel::NullObjectType & response) {}

void OTARequestor::OnNotifyUpdateAppliedFailure(void * context, EmberAfStatus status)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogDetail(SoftwareUpdate, "NotifyUpdateApplied failure response %" PRIu8, status);
    requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kNotifying, CHIP_ERROR_BAD_REQUEST);
    SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
}

EmberAfStatus OTARequestor::HandleAnnounceOTAProvider(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const AnnounceOtaProvider::DecodableType & commandData)
{
    auto & providerNodeId     = commandData.providerNodeId;
    auto & announcementReason = commandData.announcementReason;
    auto & providerEndpoint   = commandData.endpoint;

    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    mProviderNodeId      = providerNodeId;
    mProviderFabricIndex = commandObj->GetAccessingFabricIndex();
    mProviderEndpointId  = providerEndpoint;

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");
    ChipLogDetail(SoftwareUpdate, "  FabricIndex: %" PRIu8, mProviderFabricIndex);
    ChipLogDetail(SoftwareUpdate, "  ProviderNodeID: 0x" ChipLogFormatX64, ChipLogValueX64(mProviderNodeId));
    ChipLogDetail(SoftwareUpdate, "  VendorID: 0x%" PRIx16, commandData.vendorId);
    ChipLogDetail(SoftwareUpdate, "  AnnouncementReason: %" PRIu8, to_underlying(announcementReason));
    if (commandData.metadataForNode.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  MetadataForNode: %zu", commandData.metadataForNode.Value().size());
    }
    ChipLogDetail(SoftwareUpdate, "  Endpoint: %" PRIu16, commandData.endpoint);

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
        ChipLogError(SoftwareUpdate, "Unexpected announcementReason: %" PRIu8, static_cast<uint8_t>(announcementReason));
        return EMBER_ZCL_STATUS_FAILURE;
    }

    DeviceLayer::SystemLayer().StartTimer(System::Clock::Milliseconds32(msToStart), StartDelayTimerHandler, this);

    return EMBER_ZCL_STATUS_SUCCESS;
}

void OTARequestor::ConnectToProvider(OnConnectedAction onConnectedAction)
{
    VerifyOrReturn(mOtaRequestorDriver != nullptr, ChipLogError(SoftwareUpdate, "OTA requestor driver not set"));
    VerifyOrReturn(mServer != nullptr, ChipLogError(SoftwareUpdate, "Server not set"));

    FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderFabricIndex);
    VerifyOrReturn(fabricInfo != nullptr, ChipLogError(SoftwareUpdate, "Cannot find fabric"));

    // Set the action to take once connection is successfully established
    mOnConnectedAction = onConnectedAction;

    ChipLogDetail(SoftwareUpdate, "Establishing session to provider node ID 0x" ChipLogFormatX64 " on fabric index %d",
                  ChipLogValueX64(mProviderNodeId), mProviderFabricIndex);
    CHIP_ERROR err = mOperationalDeviceManager->AcquireDevice(fabricInfo->GetPeerIdForNode(mProviderNodeId), &mOnConnectedCallback,
                                                              &mOnConnectionFailureCallback);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(SoftwareUpdate, "Cannot establish connection to provider: %" CHIP_ERROR_FORMAT, err.Format()));
}

// Called whenever AcquireDevice is successful
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
            requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kQuerying, err);
            return;
        }

        SetUpdateStateAttribute(OTAUpdateStateEnum::kQuerying);
        break;
    }
    case kStartBDX: {
        CHIP_ERROR err = requestorCore->StartDownload(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to start download: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kDownloading, err);
            SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
            return;
        }

        SetUpdateStateAttribute(OTAUpdateStateEnum::kDownloading);
        break;
    }
    case kApplyUpdate: {
        CHIP_ERROR err = requestorCore->SendApplyUpdateRequest(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send ApplyUpdate command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kApplying, err);
            SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
            return;
        }

        SetUpdateStateAttribute(OTAUpdateStateEnum::kApplying);
        break;
    }
    case kNotifyUpdateApplied: {
        CHIP_ERROR err = requestorCore->SendNotifyUpdateAppliedRequest(*deviceProxy);

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to send NotifyUpdateApplied command: %" CHIP_ERROR_FORMAT, err.Format());
            requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kNotifying, err);
            SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
            return;
        }

        SetUpdateStateAttribute(OTAUpdateStateEnum::kIdle);
        break;
    }
    default:
        break;
    }
}

// Called whenever AcquireDevice fails
void OTARequestor::OnConnectionFailure(void * context, OperationalDeviceProxy * deviceProxy, PeerId peerId, CHIP_ERROR error)
{
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);
    VerifyOrDie(requestorCore != nullptr);

    ChipLogError(SoftwareUpdate, "Failed to connect to node 0x" ChipLogFormatX64 ": %" CHIP_ERROR_FORMAT,
                 ChipLogValueX64(peerId.GetNodeId()), error.Format());
    requestorCore->mOperationalDeviceManager->ReleaseDevice(deviceProxy);

    switch (requestorCore->mOnConnectedAction)
    {
    case kQueryImage:
        requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kQuerying, error);
        break;
    case kStartBDX:
        requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kDownloading, error);
        break;
    case kApplyUpdate:
        requestorCore->mOtaRequestorDriver->HandleError(UpdateFailureState::kApplying, error);
        break;
    default:
        break;
    }
}

OTARequestorInterface::OTATriggerResult OTARequestor::TriggerImmediateQuery()
{
    if (mProviderNodeId != kUndefinedNodeId)
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

void OTARequestor::NotifyUpdateApplied()
{
    ConnectToProvider(kNotifyUpdateApplied);
}

void OTARequestor::OnDownloadStateChanged(OTADownloader::State state)
{
    VerifyOrReturn(mOtaRequestorDriver != nullptr);

    switch (state)
    {
    case OTADownloader::State::kComplete:
        mOtaRequestorDriver->UpdateDownloaded();
        break;
    case OTADownloader::State::kIdle:
        mOtaRequestorDriver->HandleError(UpdateFailureState::kDownloading, CHIP_ERROR_CONNECTION_ABORTED);
        break;
    default:
        break;
    }
}

void OTARequestor::OnUpdateProgressChanged(uint8_t percent)
{
    OtaRequestorServerSetUpdateStateProgress(percent);
}

CHIP_ERROR OTARequestor::GenerateUpdateToken()
{
    if (mUpdateToken.empty())
    {
        VerifyOrReturnError(mServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

        FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderFabricIndex);
        VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

        static_assert(sizeof(NodeId) == sizeof(uint64_t), "Unexpected NodeId size");
        Encoding::BigEndian::Put64(mUpdateTokenBuffer, fabricInfo->GetPeerId().GetNodeId());
        mUpdateToken = ByteSpan(mUpdateTokenBuffer, sizeof(NodeId));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OTARequestor::SendQueryImageRequest(OperationalDeviceProxy & deviceProxy)
{
    constexpr OTADownloadProtocol kProtocolsSupported[] = { OTADownloadProtocol::kBDXSynchronous };
    QueryImage::Type args;

    VerifyOrReturnError(Basic::Attributes::VendorID::Get(kRootEndpointId, &args.vendorId) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_READ_FAILED);

    VerifyOrReturnError(Basic::Attributes::ProductID::Get(kRootEndpointId, &args.productId) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_READ_FAILED);

    VerifyOrReturnError(Basic::Attributes::SoftwareVersion::Get(kRootEndpointId, &args.softwareVersion) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_READ_FAILED);

    args.protocolsSupported = kProtocolsSupported;
    args.requestorCanConsent.SetValue(mOtaRequestorDriver->CanConsent());

    uint16_t hardwareVersion;
    if (Basic::Attributes::HardwareVersion::Get(kRootEndpointId, &hardwareVersion) == EMBER_ZCL_STATUS_SUCCESS)
    {
        args.hardwareVersion.SetValue(hardwareVersion);
    }

    char location[DeviceLayer::ConfigurationManager::kMaxLocationLength];
    if (Basic::Attributes::Location::Get(kRootEndpointId, MutableCharSpan(location)) == EMBER_ZCL_STATUS_SUCCESS)
    {
        args.location.SetValue(CharSpan(location));
    }

    Controller::OtaSoftwareUpdateProviderCluster cluster;
    cluster.Associate(&deviceProxy, mProviderEndpointId);

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
    update.softwareVersion = response.softwareVersion.Value();

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
    ReturnErrorOnFailure(GenerateUpdateToken());

    ApplyUpdateRequest::Type args;
    args.updateToken = mUpdateToken;
    args.newVersion  = mUpdateVersion;

    Controller::OtaSoftwareUpdateProviderCluster cluster;
    cluster.Associate(&deviceProxy, mProviderEndpointId);

    return cluster.InvokeCommand(args, this, OnApplyUpdateResponse, OnApplyUpdateFailure);
}

CHIP_ERROR OTARequestor::SendNotifyUpdateAppliedRequest(OperationalDeviceProxy & deviceProxy)
{
    ReturnErrorOnFailure(GenerateUpdateToken());

    NotifyUpdateApplied::Type args;
    args.updateToken     = mUpdateToken;
    args.softwareVersion = mUpdateVersion;

    Controller::OtaSoftwareUpdateProviderCluster cluster;
    cluster.Associate(&deviceProxy, mProviderEndpointId);

    return cluster.InvokeCommand(args, this, OnNotifyUpdateAppliedResponse, OnNotifyUpdateAppliedFailure);
}

} // namespace chip
