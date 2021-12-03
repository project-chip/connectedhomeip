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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/OperationalDeviceProxy.h>
#include <app/server/Server.h>
#include <platform/CHIPDeviceLayer.h>
#include <zap-generated/CHIPClientCallbacks.h>
#include <zap-generated/CHIPClusters.h>

#include <esp_log.h>

#include "BDXDownloader.h"
#include "OTARequesterImpl.h"
#include "OTAUpdater.h"

#define TAG "OTARequesterImpl"

using chip::ByteSpan;
using chip::CharSpan;
using chip::EndpointId;
using chip::FabricIndex;
using chip::NodeId;
using chip::OnDeviceConnected;
using chip::OnDeviceConnectionFailure;
using chip::OperationalDeviceProxy;
using chip::Optional;
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::bdx::TransferSession;
using chip::Callback::Callback;
using chip::Inet::IPAddress;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::Messaging;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;

void OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response);
void OnQueryImageFailure(void * context, EmberAfStatus status);
void OnApplyUpdateResponse(void * context, const ApplyUpdateResponse::DecodableType & response);
void OnApplyUpdateRequestFailure(void * context, EmberAfStatus status);

void OnConnected(void * context, chip::OperationalDeviceProxy * deviceProxy);
void OnConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error);

void OnBlockReceived(void * context, const chip::bdx::TransferSession::BlockData & blockdata);
void OnTransferComplete(void * context);
void OnTransferFailed(void * context, BdxDownloaderErrorTypes status);

enum OTARequestorCommands
{
    kCommandQueryImage = 0,
    kCommandApplyUpdateRequest,
    kCommandNotifyUpdateApplied,
};

namespace {
// TODO: Encapsulate these globals and the callbacks in some class
ExchangeContext * exchangeCtx = nullptr;
BdxDownloader bdxDownloader;
enum OTARequestorCommands operationalDeviceContext;

constexpr uint8_t kMaxUpdateTokenLen       = 32; // must be between 8 and 32
uint8_t otaUpdateToken[kMaxUpdateTokenLen] = { 0 };
uint8_t otaUpdateTokenLen                  = 0;

/* Callbacks for operational device proxy connect response */
Callback<OnDeviceConnected> onConnectedCallback(OnConnected, &operationalDeviceContext);
Callback<OnDeviceConnectionFailure> onConnectionFailureCallback(OnConnectionFailure, nullptr);

/* Callbacks for BDX data transfer */
Callback<OnBdxBlockReceived> onBlockReceivedCallback(OnBlockReceived, nullptr);
Callback<OnBdxTransferComplete> onTransferCompleteCallback(OnTransferComplete, nullptr);
Callback<OnBdxTransferFailed> onTransferFailedCallback(OnTransferFailed, nullptr);

FabricIndex providerFabricIndex = 1;
} // namespace

void OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "QueryImageResponse responded with action %" PRIu8, response.status);

    if (response.updateToken.HasValue())
    {
        otaUpdateTokenLen = response.updateToken.Value().size();
        memcpy(otaUpdateToken, response.updateToken.Value().data(), otaUpdateTokenLen);
    }
    if (response.imageURI.HasValue() == false)
    {
        ChipLogError(BDX, "OTA image URI missing");
        return;
    }

    // TODO: Handle image URI for protocol other than bdx
    // Ignore the first 23 "bdx://<NodeId>/"
    char fileDesignator[128]; // 128 is arbitrary value
    memset(fileDesignator, 0, sizeof(fileDesignator));
    size_t fileDesignatorLength = response.imageURI.Value().size() - 23 + 1; // + 1 for \0
    if ((response.imageURI.Value().size() - 23) > sizeof(fileDesignator))
    {
        fileDesignatorLength = sizeof(fileDesignator);
    }
    strlcpy(fileDesignator, response.imageURI.Value().data() + 23, fileDesignatorLength);

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = 1024;
    initOptions.FileDesLength    = static_cast<uint16_t>(fileDesignatorLength);
    initOptions.FileDesignator   = reinterpret_cast<const uint8_t *>(fileDesignator);

    chip::OperationalDeviceProxy * operationalDeviceProxy = Server::GetInstance().GetOperationalDeviceProxy();
    if (operationalDeviceProxy != nullptr)
    {
        chip::Messaging::ExchangeManager * exchangeMgr = operationalDeviceProxy->GetExchangeManager();
        chip::Optional<chip::SessionHandle> session    = operationalDeviceProxy->GetSecureSession();
        if (exchangeMgr != nullptr && session.HasValue())
        {
            exchangeCtx = exchangeMgr->NewContext(session.Value(), &bdxDownloader);
        }
        if (exchangeCtx == nullptr)
        {
            ChipLogError(BDX, "unable to allocate ec: exchangeMgr=%p sessionExists? %u", exchangeMgr, session.HasValue());
            return;
        }
    }
    else
    {
        ChipLogError(BDX, "Failed to get OperationalDeviceProxy");
        return;
    }

    bdxDownloader.SetInitialExchange(exchangeCtx);

    BdxDownloaderCallbacks bdxCallbacks;
    bdxCallbacks.onBlockReceived    = &onBlockReceivedCallback;
    bdxCallbacks.onTransferComplete = &onTransferCompleteCallback;
    bdxCallbacks.onTransferFailed   = &onTransferFailedCallback;
    bdxDownloader.SetCallbacks(bdxCallbacks);

    // This will kick of a timer which will regularly check for updates to the bdx::TransferSession state machine.
    bdxDownloader.InitiateTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kReceiver, initOptions,
                                   chip::System::Clock::Seconds16(20));
}

void OnApplyUpdateResponse(void * context, const ApplyUpdateResponse::DecodableType & response)
{
    ChipLogDetail(SoftwareUpdate, "ApplyUpdateResponse responded with action %" PRIu8, response.action);
    // Providing arbitrary value
    OTAUpdater::GetInstance().Apply(3);
}

void OnQueryImageFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

void OnApplyUpdateRequestFailure(void * context, EmberAfStatus status)
{
    ChipLogDetail(SoftwareUpdate, "ApplyUpdateRequest failure response %" PRIu8, status);
}

void OnConnected(void * context, chip::OperationalDeviceProxy * deviceProxy)
{
    ChipLogDetail(SoftwareUpdate, "Callback OnConnected");
    uint8_t * command = reinterpret_cast<uint8_t *>(context);

    chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
    constexpr EndpointId kOtaProviderEndpoint = 0;

    CHIP_ERROR err = cluster.Associate(deviceProxy, kOtaProviderEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate() failed: %s", chip::ErrorStr(err));
        return;
    }

    switch (*command)
    {
    case kCommandQueryImage: {
        // These parameters are chosen arbitrarily
        constexpr VendorId kExampleVendorId                               = VendorId::Common;
        constexpr uint16_t kExampleProductId                              = CONFIG_DEVICE_PRODUCT_ID;
        constexpr uint16_t kExampleHWVersion                              = 0;
        constexpr uint16_t kExampleSoftwareVersion                        = 0;
        constexpr EmberAfOTADownloadProtocol kExampleProtocolsSupported[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
        const char locationBuf[]                                          = { 'U', 'S' };
        CharSpan exampleLocation(locationBuf);
        constexpr bool kExampleClientCanConsent = false;
        ByteSpan metadata;

        QueryImage::Type args;
        args.vendorId           = kExampleVendorId;
        args.productId          = kExampleProductId;
        args.softwareVersion    = kExampleSoftwareVersion;
        args.protocolsSupported = kExampleProtocolsSupported;
        args.hardwareVersion.Emplace(kExampleHWVersion);
        args.location.Emplace(exampleLocation);
        args.requestorCanConsent.Emplace(kExampleClientCanConsent);
        args.metadataForProvider.Emplace(metadata);

        err = cluster.InvokeCommand(args, nullptr, OnQueryImageResponse, OnQueryImageFailure);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "QueryImage() failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
        break;
    }
    case kCommandApplyUpdateRequest: {
        constexpr uint32_t kNewVersion = 1;

        ApplyUpdateRequest::Type args;
        args.updateToken = ByteSpan(otaUpdateToken, otaUpdateTokenLen);
        args.newVersion  = kNewVersion;

        err = cluster.InvokeCommand(args, nullptr, OnApplyUpdateResponse, OnApplyUpdateRequestFailure);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "ApplyUpdateRequest() failed: %s", chip::ErrorStr(err));
        }
        break;
    }
    default:
        break;
    }
}

void OnConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "failed to connect to 0x%" PRIX64 ": %" CHIP_ERROR_FORMAT, deviceId, error.Format());
}

void OnBlockReceived(void * context, const chip::bdx::TransferSession::BlockData & blockdata)
{
    if (OTAUpdater::GetInstance().IsInProgress() == false)
    {
        OTAUpdater::GetInstance().Begin();
    }
    // TODO: Process/skip the Matter OTA header
    OTAUpdater::GetInstance().Write(reinterpret_cast<const void *>(blockdata.Data), blockdata.Length);
}

void OnTransferComplete(void * context)
{
    ESP_LOGI(TAG, "Transfer complete!");
    OTAUpdater::GetInstance().End();
}

void OnTransferFailed(void * context, BdxDownloaderErrorTypes status)
{
    ESP_LOGI(TAG, "Transfer Failed, status:%x", status);
    OTAUpdater::GetInstance().Abort();
}

void ConnectToProvider(const char * ipAddress, uint32_t nodeId)
{
    NodeId providerNodeId = nodeId;

    chip::OperationalDeviceProxy * operationalDeviceProxy = Server::GetInstance().GetOperationalDeviceProxy();
    if (operationalDeviceProxy != nullptr && operationalDeviceProxy->GetDeviceId() != providerNodeId)
    {
        operationalDeviceProxy->Disconnect();
        delete operationalDeviceProxy;
        operationalDeviceProxy = nullptr;
    }

    if (operationalDeviceProxy == nullptr)
    {
        Server * server           = &(Server::GetInstance());
        chip::FabricInfo * fabric = server->GetFabricTable().FindFabricWithIndex(providerFabricIndex);

        chip::DeviceProxyInitParams initParams = {
            .sessionManager = &(server->GetSecureSessionManager()),
            .exchangeMgr    = &(server->GetExchangeManager()),
            .idAllocator    = &(server->GetSessionIDAllocator()),
            .fabricInfo     = fabric,
            // TODO: Determine where this should be instantiated
            .imDelegate = chip::Platform::New<chip::Controller::DeviceControllerInteractionModelDelegate>(),
        };

        PeerId peerID = fabric->GetPeerId();
        peerID.SetNodeId(providerNodeId);
        operationalDeviceProxy = new chip::OperationalDeviceProxy(initParams, peerID);
        server->SetOperationalDeviceProxy(operationalDeviceProxy);

        // Explicitly calling UpdateDeviceData() should not be needed once OperationalDeviceProxy can resolve IP address from node
        // ID and fabric index
        IPAddress ipAddr;
        IPAddress::FromString(ipAddress, ipAddr);
        PeerAddress addr = PeerAddress::UDP(ipAddr, CHIP_PORT);
        operationalDeviceProxy->UpdateDeviceData(addr, operationalDeviceProxy->GetMRPConfig());
    }

    CHIP_ERROR err = operationalDeviceProxy->Connect(&onConnectedCallback, &onConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot establish connection to peer device: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void OTARequesterImpl::SendQueryImageCommand(const char * ipAddress, uint32_t nodeId)
{
    operationalDeviceContext = kCommandQueryImage;
    ConnectToProvider(ipAddress, nodeId);
}

void OTARequesterImpl::SendApplyUpdateRequestCommand(const char * ipAddress, uint32_t nodeId)
{
    operationalDeviceContext = kCommandApplyUpdateRequest;
    ConnectToProvider(ipAddress, nodeId);
}
