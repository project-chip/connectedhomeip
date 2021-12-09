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
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "OTARequestor.h"

namespace chip {

using namespace app::Clusters;
using namespace app::Clusters::OtaSoftwareUpdateProvider::Commands;
using namespace app::Clusters::OtaSoftwareUpdateRequestor::Commands;
using bdx::TransferSession;

// Global instance of the OTARequestorInterface.
OTARequestorInterface * globalOTARequestorInstance = nullptr;

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

static void LogQueryImageResponse(const QueryImageResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "QueryImageResponse:");
    ChipLogDetail(SoftwareUpdate, "  status: %" PRIu8 "", response.status);
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

    VerifyOrReturn(context != nullptr, ChipLogError(SoftwareUpdate, "Received QueryImageResponse with invalid context"));

    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);

    switch (response.status)
    {
    case EMBER_ZCL_OTA_QUERY_STATUS_UPDATE_AVAILABLE: {
        // TODO: Add a method to OTARequestorDriver used to report error condictions
        VerifyOrReturn(response.imageURI.HasValue(), ChipLogError(SoftwareUpdate, "Update is available but no image URI present"));

        // Parse out the provider node ID and file designator from the image URI
        NodeId nodeId                         = kUndefinedNodeId;
        char fileDesignatorBuffer[kUriMaxLen] = { 0 };
        MutableCharSpan fileDesignator(fileDesignatorBuffer, kUriMaxLen);
        CHIP_ERROR err = requestorCore->mBdxDownloader->ParseBdxUri(response.imageURI.Value(), nodeId, fileDesignator);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Parse BDX image URI (%.*s) returned err=%" CHIP_ERROR_FORMAT,
                                    static_cast<int>(response.imageURI.Value().size()), response.imageURI.Value().data(),
                                    err.Format()));
        requestorCore->mProviderNodeId = nodeId;

        // CSM should already be created for sending QueryImage command so use the same CSM since the
        // provider node ID that will supply the OTA image must be on the same fabric as the sender of the QueryImageResponse
        requestorCore->ConnectToProvider(kStartBDX);
        break;
    }
    case EMBER_ZCL_OTA_QUERY_STATUS_BUSY:
        break;
    case EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE:
        break;
    // TODO: Add download protocol not supported
    // Issue #9524 should handle all response status appropriately
    default:
        break;
    }
}

void OTARequestor::OnQueryImageFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

EmberAfStatus OTARequestor::HandleAnnounceOTAProvider(app::CommandHandler * commandObj,
                                                      const app::ConcreteCommandPath & commandPath,
                                                      const AnnounceOtaProvider::DecodableType & commandData)
{
    auto & providerLocation   = commandData.providerLocation;
    auto & announcementReason = commandData.announcementReason;

    if (commandObj == nullptr || commandObj->GetExchangeContext() == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access ExchangeContext for FabricIndex");
        return EMBER_ZCL_STATUS_FAILURE;
    }

    mProviderNodeId      = providerLocation;
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

CHIP_ERROR OTARequestor::SetupCASESessionManager(FabricIndex fabricIndex)
{
    // A previous CASE session had been established
    if (mCASESessionManager != nullptr)
    {
        if (mCASESessionManager->GetFabricInfo()->GetFabricIndex() != fabricIndex)
        {
            // CSM is per fabric so if fabric index does not match the previous session, CSM needs to be set up again
            Platform::Delete(mCASESessionManager);
            mCASESessionManager = nullptr;
        }
        else
        {
            // Fabric index matches so use previous instance
            return CHIP_NO_ERROR;
        }
    }

    // CSM has not been setup so create a new instance of it
    if (mCASESessionManager == nullptr)
    {
        FabricInfo * fabricInfo = mServer->GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabricInfo == nullptr)
        {
            ChipLogError(SoftwareUpdate, "Did not find fabric for index %d", fabricIndex);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        DeviceProxyInitParams initParams = {
            .sessionManager = &(mServer->GetSecureSessionManager()),
            .exchangeMgr    = &(mServer->GetExchangeManager()),
            .idAllocator    = &(mServer->GetSessionIDAllocator()),
            .fabricInfo     = fabricInfo,
            .clientPool     = mServer->GetCASEClientPool(),
            // TODO: Determine where this should be instantiated
            .imDelegate = Platform::New<Controller::DeviceControllerInteractionModelDelegate>(),
        };

        CASESessionManagerConfig sessionManagerConfig = {
            .sessionInitParams = initParams,
            .dnsCache          = nullptr,
            .devicePool        = mServer->GetDevicePool(),
            .dnsResolver       = nullptr,
        };

        mCASESessionManager = Platform::New<CASESessionManager>(sessionManagerConfig);
    }

    if (mCASESessionManager == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed in creating an instance of CASESessionManager");
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void OTARequestor::ConnectToProvider(OnConnectedAction onConnectedAction)
{
    CHIP_ERROR err = SetupCASESessionManager(mProviderFabricIndex);
    VerifyOrReturn(err == CHIP_NO_ERROR,
                   ChipLogError(SoftwareUpdate, "Cannot setup CASESessionManager: %" CHIP_ERROR_FORMAT, err.Format()));

    // Set the action to take once connection is successfully established
    mOnConnectedAction = onConnectedAction;

    ChipLogDetail(SoftwareUpdate, "Establishing session to provider node ID 0x" ChipLogFormatX64 " on fabric index %d",
                  ChipLogValueX64(mProviderNodeId), mProviderFabricIndex);
    err = mCASESessionManager->FindOrEstablishSession(mProviderNodeId, &mOnConnectedCallback, &mOnConnectionFailureCallback);
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
        constexpr EndpointId kOtaProviderEndpoint = 0;

        QueryImageRequest request;
        CHIP_ERROR err = requestorCore->BuildQueryImageRequest(request);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "Failed to build QueryImage command: %" CHIP_ERROR_FORMAT, err.Format()));

        Controller::OtaSoftwareUpdateProviderCluster cluster;
        cluster.Associate(deviceProxy, kOtaProviderEndpoint);

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
    default:
        break;
    }
}

OTARequestor::OTATriggerResult OTARequestor::TriggerImmediateQuery()
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
void OTARequestor::OnConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "Failed to connect to node 0x%" PRIX64 ": %" CHIP_ERROR_FORMAT, deviceId, error.Format());
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

} // namespace chip
