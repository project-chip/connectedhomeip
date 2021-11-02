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

#include <app/device/OperationalDeviceProxy.h>
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
using chip::Server;
using chip::VendorId;
using chip::bdx::TransferSession;
using chip::Callback::Callback;
using chip::Inet::IPAddress;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::Messaging;
using namespace chip::app::device;

void OnQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, CharSpan imageURI,
                          uint32_t softwareVersion, CharSpan softwareVersionString, ByteSpan updateToken,
                          bool userConsentNeeded, ByteSpan metadataForRequester);
void OnQueryImageFailure(void * context, uint8_t status);

void OnApplyUpdateResponse(void * context, uint8_t action, uint32_t delayedActionTime);
void OnApplyUpdateRequestFailure(void * context, uint8_t status);

void OnConnected(void * context, OperationalDeviceProxy * operationalDeviceProxy);
void OnConnectionFailure(void * context, OperationalDeviceProxy * operationalDeviceProxy, CHIP_ERROR error);

void OnBlockReceived(void *context, chip::bdx::TransferSession::BlockData & blockdata);
void OnTransferComplete(void *context);
void OnTransferFailed(void *context);

// TODO: Encapsulate these globals and the callbacks in some class
OperationalDeviceProxy gOperationalDeviceProxy;
ExchangeContext * exchangeCtx = nullptr;
BdxDownloader bdxDownloader;
uint8_t operationalDeviceContext; // 0 - none, 1 - QueryImage, 2 - ApplyUpdateRequest, 3 - NotifyUpdateApplied
ByteSpan otaUpdateToken;

/* Callbacks for QueryImage response */
Callback<OtaSoftwareUpdateProviderClusterQueryImageResponseCallback> mQueryImageResponseCallback(OnQueryImageResponse, nullptr);
Callback<DefaultFailureCallback> mOnQueryImageFailureCallback(OnQueryImageFailure, nullptr);

/* Callbacks for ApplyUpdateRequest response */
Callback<OtaSoftwareUpdateProviderClusterApplyUpdateRequestResponseCallback> mApplyUpdateResponseCallback(OnApplyUpdateResponse, nullptr);
Callback<DefaultFailureCallback> mOnApplyUpdateRequestFailureCallback(OnApplyUpdateRequestFailure, nullptr);

/* Callbacks for operational device proxy connect response */
Callback<OnOperationalDeviceConnected> mOnConnectedCallback(OnConnected, &operationalDeviceContext);
Callback<OnOperationalDeviceConnectionFailure> mOnConnectionFailureCallback(OnConnectionFailure, nullptr);

/* Callbacks for BDX data transfer */
Callback<OnBdxBlockReceived> mOnBlockReceived(OnBlockReceived, nullptr);
Callback<OnBdxTransferComplete> mOnTransferComplete(OnTransferComplete, nullptr);
Callback<OnBdxTransferFailed> mOnTransferFailed(OnTransferFailed, nullptr);

FabricIndex providerFabricIndex = 1;
uint16_t setupDiscriminator = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR;

void OnQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, CharSpan imageURI,
                          uint32_t softwareVersion, CharSpan softwareVersionString, ByteSpan updateToken,
                          bool userConsentNeeded, ByteSpan metadataForRequester)
{
    char fileDesignator[11] = { "blinky.bin" };

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = chip::bdx::TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize = 1024;
    initOptions.FileDesLength = static_cast<uint16_t>(strlen(fileDesignator));
    initOptions.FileDesignator = reinterpret_cast<uint8_t *>(fileDesignator);

    chip::Messaging::ExchangeManager * exchangeMgr = gOperationalDeviceProxy.GetDevice().GetExchangeManager();
    chip::Optional<chip::SessionHandle> session = gOperationalDeviceProxy.GetDevice().GetSecureSession();
    if (exchangeMgr != nullptr && session.HasValue())
    {
        exchangeCtx = exchangeMgr->NewContext(session.Value(), &bdxDownloader);
    }
    if (exchangeCtx == nullptr)
    {
        ChipLogError(BDX, "unable to allocate ec: exchangeMgr=%p sessionExists? %u", exchangeMgr, session.HasValue());
        return;
    }

    bdxDownloader.SetInitialExchange(exchangeCtx);

    BdxDownloaderCallbacks bdxCallbacks;
    bdxCallbacks.onBlockReceived    = &mOnBlockReceived;
    bdxCallbacks.onTransferComplete = &mOnTransferComplete;
    bdxCallbacks.onTransferFailed   = &mOnTransferFailed;
    bdxDownloader.SetCallbacks(bdxCallbacks);

    // This will kick of a timer which will regularly check for updates to the bdx::TransferSession state machine.
    bdxDownloader.InitiateTransfer(&chip::DeviceLayer::SystemLayer(), chip::bdx::TransferRole::kReceiver, initOptions,
                                   chip::System::Clock::Seconds16(20));
    otaUpdateToken = updateToken;

}

void OnApplyUpdateResponse(void * context, uint8_t action, uint32_t delayedActionTime)
{
    OTAUpdater::GetInstance().Apply(0);
}

void OnQueryImageFailure(void * context, uint8_t status)
{
    ChipLogDetail(SoftwareUpdate, "QueryImage failure response %" PRIu8, status);
}

void OnApplyUpdateRequestFailure(void * context, uint8_t status)
{
    ChipLogDetail(SoftwareUpdate, "ApplyUpdateRequest failure response %" PRIu8, status);
}

void OnConnected(void * context, OperationalDeviceProxy * operationalDeviceProxy)
{
    ChipLogDetail(SoftwareUpdate, "Callback OnConnected");
    uint8_t *command = reinterpret_cast<uint8_t *>(context);

    chip::Controller::OtaSoftwareUpdateProviderCluster cluster;
    constexpr EndpointId kOtaProviderEndpoint = 0;

    CHIP_ERROR err = cluster.Associate(&(operationalDeviceProxy->GetDevice()),  kOtaProviderEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate() failed: %s", chip::ErrorStr(err));
        return;
    }

    switch (*command)
    {
    case 1: {   // QueryImage
        chip::Callback::Cancelable * successCallback = mQueryImageResponseCallback.Cancel();
        chip::Callback::Cancelable * failureCallback = mOnQueryImageFailureCallback.Cancel();

        // These parameters are choosen arbitrarily
        constexpr VendorId kExampleVendorId = VendorId::Common;
        constexpr uint16_t kExampleProductId = CONFIG_DEVICE_PRODUCT_ID;
        constexpr uint16_t kExampleHWVersion = 0;
        constexpr uint16_t kExampleSoftwareVersion = 0;
        constexpr uint8_t kExampleProtocolsSupported = EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS; // TODO: support this as a list once ember adds list support
        const char locationBuf[] = { 'U', 'S' };
        CharSpan exampleLocation(locationBuf);
        constexpr bool kExampleClientCanConsent = false;
        ByteSpan metadata;

        err = cluster.QueryImage(successCallback, failureCallback, kExampleVendorId, kExampleProductId, kExampleHWVersion,
                                 kExampleSoftwareVersion, kExampleProtocolsSupported, exampleLocation,
                                 kExampleClientCanConsent, metadata);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "QueryImage() failed: %s", chip::ErrorStr(err));
        }
        break;
    }
    case 2: { // ApplyUpdateRequest
        chip::Callback::Cancelable * successCallback = mApplyUpdateResponseCallback.Cancel();
        chip::Callback::Cancelable * failureCallback = mOnApplyUpdateRequestFailureCallback.Cancel();

        constexpr uint32_t newVersion = 1;

        err = cluster.ApplyUpdateRequest(successCallback, failureCallback, otaUpdateToken, newVersion);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "ApplyUpdateRequest() failed: %s", chip::ErrorStr(err));
        }
        break;
    }
    case 3: { // NotifyUpdateApplied
        constexpr uint32_t softwareVersion = 1;

        err = cluster.NotifyUpdateApplied(nullptr, nullptr, otaUpdateToken, softwareVersion);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "NotifyUpdateApplied() failed %s", chip::ErrorStr(err));
        }
        break;
    }
    default:
        break;
    }
}

void OnConnectionFailure(void * context, OperationalDeviceProxy * operationalDeviceProxy, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "failed to connect to: %s", chip::ErrorStr(error));
}

void OnBlockReceived(void *context, chip::bdx::TransferSession::BlockData & blockdata)
{
    if (OTAUpdater::GetInstance().IsInProgress() == false)
    {
        OTAUpdater::GetInstance().Begin();
    }
    OTAUpdater::GetInstance().Write(reinterpret_cast<const void *>(blockdata.Data), blockdata.Length);
}

void OnTransferComplete(void *context)
{
    ESP_LOGI(TAG, "Transfer complete!");
    OTAUpdater::GetInstance().End();
}

void OnTransferFailed(void *context)
{
    ESP_LOGI(TAG, "Transfer Failed");
    OTAUpdater::GetInstance().Abort();
}

void ConnectToProvider(const char *ipAddress, uint32_t nodeId)
{
    // Explicitly calling UpdateAddress() should not be needed once OperationalDeviceProxy can resolve IP address from node ID and
    NodeId providerNodeId = nodeId;
    IPAddress ipAddr;
    IPAddress::FromString(ipAddress, ipAddr);
    PeerAddress addr = PeerAddress::UDP(ipAddr, CHIP_PORT);
    gOperationalDeviceProxy.UpdateAddress(addr);

    Server * server = &(Server::GetInstance());
    OperationalDeviceProxyInitParams initParams = {
        .sessionManager = &(server->GetSecureSessionManager()),
        .exchangeMgr    = &(server->GetExchangeManager()),
        .idAllocator    = &(server->GetSessionIDAllocator()),
        .fabricsTable   = &(server->GetFabricTable()),
    };

    CHIP_ERROR err = CHIP_NO_ERROR;
    FabricIndex peerFabricIndex = providerFabricIndex;
    gOperationalDeviceProxy.Init(providerNodeId, peerFabricIndex, initParams);
    err = gOperationalDeviceProxy.Connect(&mOnConnectedCallback, &mOnConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot establish connection to peer device: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void OTARequesterImpl::SendQueryImageCommand(const char *ipAddress, uint32_t nodeId)
{
    operationalDeviceContext = 1;
    ConnectToProvider(ipAddress, nodeId);
}

void OTARequesterImpl::SendApplyUpdateRequestCommand(const char *ipAddress, uint32_t nodeId)
{
    operationalDeviceContext = 2;
    ConnectToProvider(ipAddress, nodeId);
}

void OTARequesterImpl::SendNotifyUpdateAppliedCommand(const char *ipAddress, uint32_t nodeId)
{
    operationalDeviceContext = 2;
    ConnectToProvider(ipAddress, nodeId);
}
