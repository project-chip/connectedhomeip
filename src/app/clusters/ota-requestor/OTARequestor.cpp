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

#include <app/server/Server.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <zap-generated/CHIPClusters.h>

#include "BDXDownloader.h"
#include "OTARequestor.h"

namespace chip {

using bdx::TransferSession;

// Global instance of the OTARequestorInterface.
OTARequestorInterface * globalOTARequestorInstance = nullptr;

constexpr uint32_t kImmediateStartDelayMs = 1; // Start the timer with this value when starting OTA "immediately"

// Constants for BDX URI parsing
constexpr char bdxPrefix[]               = "bdx://";
constexpr char bdxSeparator[]            = "/";
constexpr uint8_t kValidBdxUriMinLen     = 24;
constexpr uint8_t kNodeIdHexStringMaxLen = 16;

void SetRequestorInstance(OTARequestorInterface * instance)
{
    globalOTARequestorInstance = instance;
}

OTARequestorInterface * GetRequestorInstance()
{
    return globalOTARequestorInstance;
}

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

// TODO: Add unit tests for parsing BDX URI
CHIP_ERROR OTARequestor::ParseBdxUri(CharSpan uri, NodeId & nodeId, CharSpan & fileDesignator)
{
    // Check against minimum length of a valid BDX URI
    if (uri.size() < kValidBdxUriMinLen)
    {
        ChipLogError(SoftwareUpdate, "Expect BDX URI to be at least %d in length, actual length is %zu", kValidBdxUriMinLen,
                     uri.size());
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    }

    // Check the scheme field matches the BDX prefix
    CharSpan expectedScheme(bdxPrefix, strlen(bdxPrefix));
    if (!uri.SubSpan(0, strlen(bdxPrefix)).data_equal(expectedScheme))
    {
        ChipLogError(SoftwareUpdate, "Expect scheme to contain prefix %s, actual URI: %.*s", bdxPrefix,
                     static_cast<int>(uri.size()), uri.data());
        return CHIP_ERROR_INVALID_SCHEME_PREFIX;
    }

    // Extract the node ID from the authority field
    CharSpan nodeIdString = uri.SubSpan(strlen(bdxPrefix), kNodeIdHexStringMaxLen);
    uint8_t buffer[kNodeIdHexStringMaxLen];
    if (chip::Encoding::HexToBytes(nodeIdString.data(), nodeIdString.size(), buffer, kNodeIdHexStringMaxLen) == 0)
    {
        ChipLogError(SoftwareUpdate, "Cannot convert image URI provider node ID: %.*s", static_cast<int>(nodeIdString.size()),
                     nodeIdString.data());
        return CHIP_ERROR_INVALID_DESTINATION_NODE_ID;
    }

    nodeId = chip::Encoding::BigEndian::Get64(buffer);
    if (!IsOperationalNodeId(nodeId))
    {
        ChipLogError(SoftwareUpdate, "Image URI contains invalid node ID: 0x" ChipLogFormatX64, ChipLogValueX64(nodeId));
        return CHIP_ERROR_INVALID_DESTINATION_NODE_ID;
    }

    // Verify the separator between authority and path fields
    CharSpan separator(bdxSeparator, strlen(bdxSeparator));
    if (!uri.SubSpan(strlen(bdxPrefix) + kNodeIdHexStringMaxLen, strlen(bdxSeparator)).data_equal(separator))
    {
        return CHIP_ERROR_MISSING_URI_SEPARATOR;
    }

    // Extract file designator from the path field
    size_t fileDesignatorLength = uri.size() - (strlen(bdxPrefix) + kNodeIdHexStringMaxLen + strlen(bdxSeparator));
    fileDesignator              = uri.SubSpan(uri.size() - fileDesignatorLength, fileDesignatorLength);

    return CHIP_NO_ERROR;
}

void OTARequestor::OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    LogQueryImageResponse(response);

    VerifyOrReturn(context != nullptr, ChipLogError(SoftwareUpdate, "Received QueryImageResponse with invalid context"));

    OTARequestor * requestorCore = static_cast<OTARequestor *>(context);

    switch (response.status)
    {
    case EMBER_ZCL_OTA_QUERY_STATUS_UPDATE_AVAILABLE: {
        VerifyOrReturn(response.imageURI.HasValue(), ChipLogError(SoftwareUpdate, "Update is available but no image URI present"));

        // Parse out the provider node ID and file designator from the image URI
        NodeId nodeId = chip::kUndefinedNodeId;
        CharSpan fileDesignator;
        CHIP_ERROR err = requestorCore->ParseBdxUri(response.imageURI.Value(), nodeId, fileDesignator);
        VerifyOrReturn(err == CHIP_NO_ERROR,
                       ChipLogError(SoftwareUpdate, "BDX image URI is invalid: %" CHIP_ERROR_FORMAT, err.Format()));
        requestorCore->mProviderNodeId = nodeId;

        // CSM should already be created for sending QueryImage command so use the same CSM based on the assumption that the
        // provider node ID that will supply the OTA image is on the same fabric as the sender of the QueryImageResponse
        requestorCore->ConnectToProvider(kStartBDX);
        break;
    }
    case EMBER_ZCL_OTA_QUERY_STATUS_BUSY:
        break;
    case EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE:
        break;
    // TODO: Add download protocol not supported
    default:
        break;
    }
}

void OTARequestor::OnQueryImageFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

EmberAfStatus OTARequestor::HandleAnnounceOTAProvider(
    app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
    const app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
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

    ConnectToProvider(kQueryImage);

    return EMBER_ZCL_STATUS_SUCCESS;
}

CHIP_ERROR OTARequestor::SetupCASESessionManager(chip::FabricIndex fabricIndex)
{
    // A previous CASE session had been established
    if (mCASESessionManager != nullptr)
    {
        if (mCASESessionManager->GetFabricInfo()->GetFabricIndex() != fabricIndex)
        {
            // CSM is per fabric so if fabric index does not match the previous session, CSM needs to be set up again
            chip::Platform::Delete(mCASESessionManager);
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
        Server * server         = &(chip::Server::GetInstance());
        FabricInfo * fabricInfo = server->GetFabricTable().FindFabricWithIndex(fabricIndex);
        if (fabricInfo == nullptr)
        {
            ChipLogError(SoftwareUpdate, "Did not find fabric for index %d", fabricIndex);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        DeviceProxyInitParams initParams = {
            .sessionManager = &(server->GetSecureSessionManager()),
            .exchangeMgr    = &(server->GetExchangeManager()),
            .idAllocator    = &(server->GetSessionIDAllocator()),
            .fabricInfo     = fabricInfo,
            // TODO: Determine where this should be instantiated
            .imDelegate = chip::Platform::New<chip::Controller::DeviceControllerInteractionModelDelegate>(),
        };

        CASESessionManagerConfig sessionManagerConfig = {
            .sessionInitParams = initParams,
            .dnsCache          = nullptr,
        };

        mCASESessionManager = chip::Platform::New<chip::CASESessionManager>(sessionManagerConfig);
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

    ChipLogDetail(SoftwareUpdate, "Establishing session to provider node ID: 0x" ChipLogFormatX64,
                  ChipLogValueX64(mProviderNodeId));
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
        CHIP_ERROR err = CHIP_NO_ERROR;
        chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
        constexpr EndpointId kOtaProviderEndpoint = 0;

        // These QueryImage params have been chosen arbitrarily
        constexpr VendorId kExampleVendorId                               = VendorId::Common;
        constexpr uint16_t kExampleProductId                              = 77;
        constexpr uint16_t kExampleHWVersion                              = 3;
        constexpr uint16_t kExampleSoftwareVersion                        = 0;
        constexpr EmberAfOTADownloadProtocol kExampleProtocolsSupported[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
        const char locationBuf[]                                          = { 'U', 'S' };
        CharSpan exampleLocation(locationBuf);
        constexpr bool kExampleClientCanConsent = false;
        ByteSpan metadata;

        err = cluster.Associate(deviceProxy, kOtaProviderEndpoint);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
            return;
        }
        QueryImage::Type args;
        args.vendorId           = kExampleVendorId;
        args.productId          = kExampleProductId;
        args.softwareVersion    = kExampleSoftwareVersion;
        args.protocolsSupported = kExampleProtocolsSupported;
        args.hardwareVersion.Emplace(kExampleHWVersion);
        args.location.Emplace(exampleLocation);
        args.requestorCanConsent.Emplace(kExampleClientCanConsent);
        args.metadataForProvider.Emplace(metadata);
        err = cluster.InvokeCommand(args, requestorCore, OnQueryImageResponse, OnQueryImageFailure);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "QueryImage() failed: %" CHIP_ERROR_FORMAT, err.Format());
        }

        break;
    }
    case kStartBDX: {
        VerifyOrReturn(requestorCore->mBdxDownloader != nullptr, ChipLogError(SoftwareUpdate, "Downloader is null"));

        // TODO: allow caller to provide their own OTADownloader instance and set BDX parameters

        TransferSession::TransferInitData initOptions;
        initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
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

            chip::Optional<chip::SessionHandle> session = deviceProxy->GetSecureSession();
            if (!session.HasValue())
            {
                ChipLogError(SoftwareUpdate, "Cannot retrieve session from device");
                return;
            }

            requestorCore->mExchangeCtx = exchangeMgr->NewContext(session.Value(), &requestorCore->mBdxMessenger);

            if (requestorCore->mExchangeCtx == nullptr)
            {
                ChipLogError(BDX, "Unable to allocate ec: exchangeMgr=%p sessionExists? %u", exchangeMgr, session.HasValue());
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

// Called whenever FindOrEstablishSession fails
void OTARequestor::OnConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "Failed to connect to node 0x%" PRIX64 ": %" CHIP_ERROR_FORMAT, deviceId, error.Format());
}

void OTARequestor::TriggerImmediateQuery()
{
    ConnectToProvider(kQueryImage);
}

} // namespace chip
