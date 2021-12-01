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

#include <OTARequestorImpl.h>

#include <app/util/util.h>
#include <platform/CHIPDeviceLayer.h>

#include <app/OperationalDeviceProxy.h>
#include <app/server/Server.h>
#include <app/util/util.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/CommissioneeDeviceProxy.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPArgParser.hpp>
#include <platform/CHIPDeviceLayer.h>
#include <zap-generated/CHIPClusters.h>

using chip::DeviceProxy;
using chip::EndpointId;
using chip::FabricIndex;
using chip::NodeId;
using chip::OnDeviceConnected;
using chip::OnDeviceConnectionFailure;
using chip::PeerId;
using chip::Server;
using chip::VendorId;
using chip::Callback::Callback;
using chip::Inet::IPAddress;
using chip::System::Layer;
using chip::Transport::PeerAddress;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;
using namespace chip::DeviceLayer;

OTARequestorImpl OTARequestorImpl::sInstance;

constexpr const char gRequestorLocation[]                  = { 'U', 'S' };
constexpr EmberAfOTADownloadProtocol gRequestorProtocols[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
constexpr chip::EndpointId gOtaProviderEndpoint            = 0;
constexpr bool gRequestorCanConsent                        = false;

static void OnProviderConnected(void * context, chip::DeviceProxy * deviceProxy);
static void OnProviderConnectionFailure(void * context, chip::NodeId deviceId, CHIP_ERROR error);

static void
OnQueryImageResponse(void * context,
                     const chip::app::Clusters::OtaSoftwareUpdateProvider::Commands::QueryImageResponse::DecodableType & response);
static void OnQueryImageFailure(void * context, EmberAfStatus status);

static Callback<OnDeviceConnected> mOnConnectedCallback(OnProviderConnected, nullptr);
static Callback<OnDeviceConnectionFailure> mOnConnectionFailureCallback(OnProviderConnectionFailure, nullptr);

OTARequestorImpl::OTARequestorImpl()
{
    mProviderNodeId          = chip::kUndefinedNodeId;
    mProviderFabricIndex     = chip::kUndefinedFabricIndex;
    mConnectProviderCallback = nullptr;
}

bool OTARequestorImpl::HandleAnnounceOTAProvider(
    chip::app::CommandHandler * commandObj, const chip::app::ConcreteCommandPath & commandPath,
    const chip::app::Clusters::OtaSoftwareUpdateRequestor::Commands::AnnounceOtaProvider::DecodableType & commandData)
{
    if (commandObj == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Cannot access get FabricIndex");
        return true;
    }

    mProviderNodeId      = commandData.providerLocation;
    mProviderFabricIndex = commandObj->GetAccessingFabricIndex();
    mAnnouncementReason  = commandData.announcementReason;
    mProviderIpAddress   = commandData.metadataForNode;

    ChipLogProgress(SoftwareUpdate, "OTA Requestor received AnnounceOTAProvider");
    ChipLogProgress(SoftwareUpdate, "  FabricIndex: %" PRIu8, mProviderFabricIndex);
    ChipLogProgress(SoftwareUpdate, "  ProviderNodeID: %" PRIu64 " (0x%" PRIX64 ")", mProviderNodeId, mProviderNodeId);
    ChipLogProgress(SoftwareUpdate, "  VendorID: %" PRIu16 " (0x%" PRIX16 ")", commandData.vendorId, commandData.vendorId);
    ChipLogProgress(SoftwareUpdate, "  ProviderIP: %s", reinterpret_cast<const char *>(mProviderIpAddress.Value().data()));
    ChipLogProgress(SoftwareUpdate, "  AnnouncementReason: %" PRIu8, mAnnouncementReason);

    if (mConnectProviderCallback)
    {
        mConnectProviderCallback(mProviderNodeId, mProviderFabricIndex, mProviderIpAddress);
    }

    return false;
}

void OTARequestorImpl::ConnectProvider(NodeId peerNodeId, FabricIndex peerFabricIndex, const char * ipAddress)
{
    Server * server           = &(Server::GetInstance());
    chip::FabricInfo * fabric = server->GetFabricTable().FindFabricWithIndex(peerFabricIndex);
    if (fabric == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Did not find fabric for index %d", peerFabricIndex);
        return;
    }

    chip::DeviceProxyInitParams initParams = {
        .sessionManager = &(server->GetSecureSessionManager()),
        .exchangeMgr    = &(server->GetExchangeManager()),
        .idAllocator    = &(server->GetSessionIDAllocator()),
        .fabricInfo     = fabric,
        .imDelegate     = chip::Platform::New<chip::Controller::DeviceControllerInteractionModelDelegate>(),
    };

    chip::OperationalDeviceProxy * operationalDeviceProxy =
        chip::Platform::New<chip::OperationalDeviceProxy>(initParams, fabric->GetPeerIdForNode(peerNodeId));
    if (operationalDeviceProxy == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed in creating an instance of OperationalDeviceProxy");
        return;
    }

    server->SetOperationalDeviceProxy(operationalDeviceProxy);

    // Explicitly calling UpdateDeviceData() should not be needed once OperationalDeviceProxy can resolve IP address from node
    // ID and fabric index
    IPAddress ipAddr;
    IPAddress::FromString(ipAddress, ipAddr);
    PeerAddress addr = PeerAddress::UDP(ipAddr, CHIP_PORT);
    operationalDeviceProxy->UpdateDeviceData(addr, operationalDeviceProxy->GetMRPConfig());

    CHIP_ERROR err = operationalDeviceProxy->Connect(&mOnConnectedCallback, &mOnConnectionFailureCallback);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Cannot establish connection to peer device: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void OnProviderConnected(void * context, chip::DeviceProxy * deviceProxy)
{
    ChipLogProgress(SoftwareUpdate, "OnProviderConnected");
    chip::Controller::OtaSoftwareUpdateProviderCluster cluster;

    CHIP_ERROR err = cluster.Associate(deviceProxy, gOtaProviderEndpoint);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate() failed: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    // Send QueryImage command
    uint16_t vendorId        = 0;
    uint16_t productId       = 0;
    uint16_t hardwareVersion = 0;
    uint16_t softwareVersion = 0;

    ConfigurationMgr().GetVendorId(vendorId);
    ConfigurationMgr().GetProductId(productId);
    ConfigurationMgr().GetHardwareVersion(hardwareVersion);
    ConfigurationMgr().GetSoftwareVersion(softwareVersion);

    QueryImage::Type request;
    request.vendorId           = static_cast<VendorId>(vendorId);
    request.productId          = productId;
    request.softwareVersion    = softwareVersion;
    request.protocolsSupported = gRequestorProtocols;
    request.hardwareVersion.Emplace(hardwareVersion);
    request.location.Emplace(gRequestorLocation);
    request.requestorCanConsent.SetValue(gRequestorCanConsent);
    err = cluster.InvokeCommand(request, /* context = */ nullptr, OnQueryImageResponse, OnQueryImageFailure);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "QueryImage() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
}

void OnProviderConnectionFailure(void * context, NodeId deviceId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "Provider connection failure 0x%" PRIX64 ": %" CHIP_ERROR_FORMAT, deviceId, error.Format());
}

void OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    ChipLogProgress(SoftwareUpdate, "OnQueryImageResponse");

    OTARequestorImpl::GetInstance().HandleQueryImageResponse(
        context, response.status, response.delayedActionTime.ValueOr(0), response.imageURI.ValueOr({}),
        response.softwareVersion.ValueOr(0), response.softwareVersionString.ValueOr({}), response.updateToken.ValueOr({}),
        response.userConsentNeeded.ValueOr(false), response.metadataForRequestor.ValueOr({}));
}

void OnQueryImageFailure(void * context, EmberAfStatus status)
{
    ChipLogError(SoftwareUpdate, "QueryImage failure %" PRIu8, status);
}

bool OTARequestorImpl::HandleQueryImageResponse(void * context, uint8_t status, uint32_t delayedActionTime, chip::CharSpan imageURI,
                                                uint32_t softwareVersion, chip::CharSpan softwareVersionString,
                                                chip::ByteSpan updateToken, bool userConsentNeeded,
                                                chip::ByteSpan metadataForRequestor)
{
    switch (status)
    {
    case EMBER_ZCL_OTA_QUERY_STATUS_UPDATE_AVAILABLE:
        ChipLogProgress(SoftwareUpdate, "Update available");
        break;
    case EMBER_ZCL_OTA_QUERY_STATUS_BUSY:
        ChipLogProgress(SoftwareUpdate, "OTA provider busy");
        return false;
    case EMBER_ZCL_OTA_QUERY_STATUS_NOT_AVAILABLE:
        ChipLogProgress(SoftwareUpdate, "Update not available");
        return false;
    }

    ChipLogProgress(SoftwareUpdate, "  Image URI: %.*s", static_cast<int>(imageURI.size()), imageURI.data());
    ChipLogProgress(SoftwareUpdate, "  Version: %" PRIu32, softwareVersion);

    if (updateToken.size() > sizeof(mUpdateDetails.updateToken))
    {
        ChipLogError(SoftwareUpdate, "Update token too long");
        return false;
    }

    memcpy(mUpdateDetails.updateToken, updateToken.data(), updateToken.size());
    mUpdateDetails.updateTokenLen = static_cast<uint8_t>(updateToken.size());
    mUpdateDetails.updateVersion  = softwareVersion;

    if (mOtaRequestorDriver)
    {
        auto ret = mOtaRequestorDriver->CheckImageDownloadAllowed();
        if (!ret)
        {
            ChipLogProgress(SoftwareUpdate, "Update download not allowed");
            return false;
        }
    }

    if (mDownloadUpdateCallback)
    {
        mDownloadUpdateCallback();
    }
    return false;
}