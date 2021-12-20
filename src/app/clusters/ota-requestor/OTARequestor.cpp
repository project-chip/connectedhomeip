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

#include <app-common/zap-generated/attributes/Accessors.h>
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

struct OTARequestor::QueryImageRequest
{
    char location[2];
    QueryImage::Type args;
};

void OTARequestor::OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    LogQueryImageResponse(response);

    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);

    VerifyOrReturn(requestorCore != nullptr, ChipLogError(SoftwareUpdate, "Received QueryImageResponse with invalid context"));
    // TODO: Add a method to OTARequestorDriver used to report error condictions
    VerifyOrReturn(requestorCore->ValidateQueryImageResponse(response),
                   ChipLogError(SoftwareUpdate, "Received invalid QueryImageResponse"));

    switch (response.status)
    {
    case OTAQueryStatus::kUpdateAvailable: {
        // Parse out the provider node ID and file designator from the image URI
        NodeId nodeId = kUndefinedNodeId;
        CharSpan fileDesignator;
        CHIP_ERROR err = bdx::ParseURI(response.imageURI.Value(), nodeId, fileDesignator);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Parse BDX image URI (%.*s) returned err=%" CHIP_ERROR_FORMAT,
                                    static_cast<int>(response.imageURI.Value().size()), response.imageURI.Value().data(),
                                    err.Format()));
        requestorCore->mProviderNodeId = nodeId;

        MutableByteSpan updateToken(requestorCore->mUpdateTokenBuffer);
        CopySpanToMutableSpan(response.updateToken.Value(), updateToken);
        requestorCore->mUpdateVersion = response.softwareVersion.Value();
        requestorCore->mUpdateToken   = updateToken;

        // CSM should already be created for sending QueryImage command so use the same CSM since the
        // provider node ID that will supply the OTA image must be on the same fabric as the sender of the QueryImageResponse
        requestorCore->ConnectToProvider(kStartBDX);
        break;
    }
    case OTAQueryStatus::kBusy:
        break;
    case OTAQueryStatus::kNotAvailable:
        break;
    case OTAQueryStatus::kDownloadProtocolNotSupported:
        break;
    // Issue #9524 should handle all response status appropriately
    default:
        break;
    }
}

void OTARequestor::OnQueryImageFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

void OTARequestor::OnApplyUpdateResponse(void * context, const ApplyUpdateResponse::DecodableType & response)
{
    VerifyOrReturn(context != nullptr, ChipLogError(SoftwareUpdate, "Received ApplyUpdateResponse with invalid context"));

    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);

    switch (response.action)
    {
    case EMBER_ZCL_OTA_APPLY_UPDATE_ACTION_PROCEED: {
        // TODO: Call OTARequestorDriver to schedule the image application.
        VerifyOrReturn(requestorCore->mBdxDownloader != nullptr, ChipLogError(SoftwareUpdate, "Downloader is not set"));
        OTAImageProcessorInterface * imageProcessor = requestorCore->mBdxDownloader->GetImageProcessorDelegate();
        VerifyOrReturn(imageProcessor != nullptr, ChipLogError(SoftwareUpdate, "Image processor is not set"));
        imageProcessor->Apply();
        break;
    }
    default:
        break;
    }
}

void OTARequestor::OnApplyUpdateFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "ApplyUpdate failure response %" PRIu8, status);
}

EmberAfStatus OTARequestor::HandleAnnounceOTAProvider(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const AnnounceOtaProvider::DecodableType & commandData)
{
    auto & providerNodeId     = commandData.providerNodeId;
    auto & announcementReason = commandData.announcementReason;

    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    mProviderNodeId      = providerNodeId;
    mProviderFabricIndex = commandObj->GetExchangeContext()->GetSessionHandle().GetFabricIndex();

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");
    ChipLogDetail(SoftwareUpdate, "  FabricIndex: %" PRIu8, mProviderFabricIndex);
    ChipLogDetail(SoftwareUpdate, "  ProviderNodeID: 0x" ChipLogFormatX64, ChipLogValueX64(mProviderNodeId));
    ChipLogDetail(SoftwareUpdate, "  VendorID: 0x%" PRIx16, commandData.vendorId);
    ChipLogDetail(SoftwareUpdate, "  AnnouncementReason: %" PRIu8, announcementReason);
    if (commandData.metadataForNode.HasValue())
    {
        ChipLogDetail(SoftwareUpdate, "  MetadataForNode: %zu", commandData.metadataForNode.Value().size());
    }

    // If reason is URGENT_UPDATE_AVAILABLE, we start OTA immediately. Otherwise, respect the timer value set in mOtaStartDelayMs.
    // This is done to exemplify what a real-world OTA Requestor might do while also being configurable enough to use as a test app.
    uint32_t msToStart = 0;
    switch (announcementReason)
    {
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_SIMPLE_ANNOUNCEMENT):
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_UPDATE_AVAILABLE):
        msToStart = mOtaStartDelayMs;
        break;
    case static_cast<uint8_t>(EMBER_ZCL_OTA_ANNOUNCEMENT_REASON_URGENT_UPDATE_AVAILABLE):
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
    FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderFabricIndex);
    VerifyOrReturn(fabricInfo != nullptr, ChipLogError(SoftwareUpdate, "Cannot find fabric"));

    // Set the action to take once connection is successfully established
    mOnConnectedAction = onConnectedAction;

    ChipLogDetail(SoftwareUpdate, "Establishing session to provider node ID 0x" ChipLogFormatX64 " on fabric index %d",
                  ChipLogValueX64(mProviderNodeId), mProviderFabricIndex);
    CHIP_ERROR err = mCASESessionManager->FindOrEstablishSession(fabricInfo->GetPeerIdForNode(mProviderNodeId),
                                                                 &mOnConnectedCallback, &mOnConnectionFailureCallback);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(SoftwareUpdate, "Cannot establish connection to provider: %" CHIP_ERROR_FORMAT, err.Format()));
}

// Called whenever FindOrEstablishSession is successful
void OTARequestor::OnConnected(void * context, OperationalDeviceProxy * deviceProxy)
{
    VerifyOrReturn(context != nullptr, ChipLogError(SoftwareUpdate, "Successfully established session but context is invalid"));
    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);

    switch (requestorCore->mOnConnectedAction)
    {
    case kQueryImage: {
        QueryImageRequest request;
        CHIP_ERROR err = requestorCore->BuildQueryImageRequest(request);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Failed to build QueryImage command: %" CHIP_ERROR_FORMAT, err.Format()));

        Controller::OtaSoftwareUpdateProviderCluster cluster;
        cluster.Associate(deviceProxy, requestorCore->mProviderEndpointId);

        err = cluster.InvokeCommand(request.args, requestorCore, OnQueryImageResponse, OnQueryImageFailure);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Failed to send QueryImage command: %" CHIP_ERROR_FORMAT, err.Format()));

        break;
    }
    case kStartBDX: {
        VerifyOrReturn(requestorCore->mBdxDownloader != nullptr, ChipLogError(SoftwareUpdate, "Downloader is null"));

        // TODO: allow caller to provide their own OTADownloader instance and set BDX parameters

        TransferSession::TransferInitData initOptions;
        initOptions.TransferCtlFlags = bdx::TransferControlFlags::kReceiverDrive;
        initOptions.MaxBlockSize     = 1024;
        char testFileDes[9]          = { "test.txt" };
        initOptions.FileDesLength    = static_cast<uint16_t>(strlen(testFileDes));
        initOptions.FileDesignator   = reinterpret_cast<uint8_t *>(testFileDes);

        if (deviceProxy != nullptr)
        {
            chip::Messaging::ExchangeManager * exchangeMgr = deviceProxy->GetExchangeManager();
            if (exchangeMgr == nullptr)
            {
                ChipLogError(SoftwareUpdate, "Cannot retrieve exchange manager from device");
                return;
            }

            Optional<SessionHandle> session = deviceProxy->GetSecureSession();
            if (!session.HasValue())
            {
                ChipLogError(SoftwareUpdate, "Cannot retrieve session from device");
                return;
            }

            requestorCore->mExchangeCtx = exchangeMgr->NewContext(session.Value(), &requestorCore->mBdxMessenger);

            if (requestorCore->mExchangeCtx == nullptr)
            {
                ChipLogError(BDX, "Unable to allocate ec: exchangeMgr=%p sessionExists? %u, OTA progress cannot continue",
                             exchangeMgr, session.HasValue());
                return;
            }
        }

        requestorCore->mBdxMessenger.Init(requestorCore->mBdxDownloader, requestorCore->mExchangeCtx);
        requestorCore->mBdxDownloader->SetMessageDelegate(&requestorCore->mBdxMessenger);
        CHIP_ERROR err = requestorCore->mBdxDownloader->SetBDXParams(initOptions);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Error init BDXDownloader: %" CHIP_ERROR_FORMAT, err.Format()));
        err = requestorCore->mBdxDownloader->BeginPrepareDownload();
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Cannot begin prepare download: %" CHIP_ERROR_FORMAT, err.Format()));
        break;
    }
    case kApplyUpdate: {
        ApplyUpdateRequest::Type args;
        CHIP_ERROR err = requestorCore->BuildApplyUpdateRequest(args);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Failed to build ApplyUpdate command: %" CHIP_ERROR_FORMAT, err.Format()));

        Controller::OtaSoftwareUpdateProviderCluster cluster;
        cluster.Associate(deviceProxy, requestorCore->mProviderEndpointId);

        err = cluster.InvokeCommand(args, requestorCore, OnApplyUpdateResponse, OnApplyUpdateFailure);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Failed to send ApplyUpdate command: %" CHIP_ERROR_FORMAT, err.Format()));

        break;
    }
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

// Called whenever FindOrEstablishSession fails
void OTARequestor::OnConnectionFailure(void * context, PeerId peerId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "Failed to connect to node 0x%" PRIX64 ": %" CHIP_ERROR_FORMAT, peerId.GetNodeId(),
                 error.Format());
}

void OTARequestor::ApplyUpdate()
{
    ConnectToProvider(kApplyUpdate);
}

CHIP_ERROR OTARequestor::BuildQueryImageRequest(QueryImageRequest & request)
{
    constexpr EmberAfOTADownloadProtocol kProtocolsSupported[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
    constexpr bool kRequestorCanConsent                        = false;
    QueryImage::Type & args                                    = request.args;

    uint16_t vendorId;
    VerifyOrReturnError(Basic::Attributes::VendorID::Get(kRootEndpointId, &vendorId) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_READ_FAILED);
    args.vendorId = static_cast<VendorId>(vendorId);

    VerifyOrReturnError(Basic::Attributes::ProductID::Get(kRootEndpointId, &args.productId) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_READ_FAILED);

    VerifyOrReturnError(Basic::Attributes::SoftwareVersion::Get(kRootEndpointId, &args.softwareVersion) == EMBER_ZCL_STATUS_SUCCESS,
                        CHIP_ERROR_READ_FAILED);

    args.protocolsSupported = kProtocolsSupported;
    args.requestorCanConsent.SetValue(kRequestorCanConsent);

    uint16_t hardwareVersion;
    if (Basic::Attributes::HardwareVersion::Get(kRootEndpointId, &hardwareVersion) == EMBER_ZCL_STATUS_SUCCESS)
    {
        args.hardwareVersion.SetValue(hardwareVersion);
    }

    if (Basic::Attributes::Location::Get(kRootEndpointId, MutableCharSpan(request.location)) == EMBER_ZCL_STATUS_SUCCESS)
    {
        args.location.SetValue(CharSpan(request.location));
    }

    return CHIP_NO_ERROR;
}

bool OTARequestor::ValidateQueryImageResponse(const QueryImageResponse::DecodableType & response) const
{
    if (response.status == OTAQueryStatus::kUpdateAvailable)
    {
        VerifyOrReturnError(response.imageURI.HasValue(), false);
        VerifyOrReturnError(response.softwareVersion.HasValue() && response.softwareVersionString.HasValue(), false);
        VerifyOrReturnError(response.updateToken.HasValue(), false);
    }

    return true;
}

CHIP_ERROR OTARequestor::BuildApplyUpdateRequest(ApplyUpdateRequest::Type & args)
{
    if (mUpdateToken.empty())
    {
        // OTA Requestor shall use its node ID as the update token in case the original update
        // token, received in QueryImageResponse, got lost.
        VerifyOrReturnError(mServer != nullptr, CHIP_ERROR_INCORRECT_STATE);

        FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(mProviderFabricIndex);
        VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_INCORRECT_STATE);

        static_assert(sizeof(NodeId) == sizeof(uint64_t), "Unexpected NodeId size");
        Encoding::BigEndian::Put64(mUpdateTokenBuffer, fabricInfo->GetPeerId().GetNodeId());
        mUpdateToken = ByteSpan(mUpdateTokenBuffer, sizeof(NodeId));
    }

    args.updateToken = mUpdateToken;
    args.newVersion  = mUpdateVersion;
    return CHIP_NO_ERROR;
}

} // namespace chip
