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

#include <platform/CHIPDeviceLayer.h>

#include <app/DeviceControllerInteractionModelDelegate.h>
#include <app/OperationalDeviceProxy.h>
#include <app/server/Server.h>
#include <lib/core/CHIPCallback.h>
#include <lib/core/Optional.h>
#include <lib/shell/Commands.h>
#include <lib/shell/Engine.h>
#include <lib/shell/commands/Help.h>
#include <lib/support/BytesToHex.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>

// TODO: Use common software update layer to store the image
#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#include "DFUManager_nrfconnect.h"
#endif

#include <controller-clusters/zap-generated/CHIPClientCallbacks.h>
#include <controller-clusters/zap-generated/CHIPClusters.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters::OtaSoftwareUpdateProvider::Commands;
using namespace chip::bdx;
using namespace chip::Callback;

namespace chip {
namespace Shell {
namespace {

constexpr const char kRequestorLocation[]                  = { 'U', 'S' };
constexpr EmberAfOTADownloadProtocol kRequestorProtocols[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
constexpr bool kRequestorCanConsent                        = false;
constexpr uint16_t kBlockSize                              = 1024;
constexpr uint8_t kMaxUpdateTokenLen                       = 32;

struct OTAContext
{
    OperationalDeviceProxy * deviceProxy    = nullptr;
    Transport::PeerAddress providerAddress  = {};
    EndpointId providerEndpointId           = 0;
    uint8_t updateToken[kMaxUpdateTokenLen] = {};
    uint8_t updateTokenLen                  = 0;
    uint32_t updateVersion                  = 0;

    void Clear();
};

Shell::Engine sSubShell;
Controller::DeviceControllerInteractionModelDelegate sIMDelegate;
DFUManager sDfuManager;
OTAContext sOtaContext;

inline void OTAContext::Clear()
{
    if (deviceProxy != nullptr)
    {
        deviceProxy->Disconnect();
        delete deviceProxy;
    }

    *this = {};
}

CharSpan ExtractResourceName(CharSpan imageURI)
{
    // TODO: Implement a correct URI parser
    constexpr char protocol[]    = "bdx://";
    constexpr size_t protocolLen = sizeof(protocol) - 1;

    if (imageURI.size() >= protocolLen && strncmp(protocol, imageURI.data(), imageURI.size()) == 0)
    {
        imageURI = imageURI.SubSpan(protocolLen, imageURI.size() - protocolLen);
    }

    for (size_t i = 0; i < imageURI.size(); ++i)
    {
        if (imageURI.data()[i] == '/')
        {
            return imageURI.SubSpan(i + 1, imageURI.size() - i - 1);
        }
    }

    return imageURI;
}

void OnQueryImageResponse(void * context, const QueryImageResponse::DecodableType & response)
{
    CharSpan imageURI     = response.imageURI.ValueOr({});
    ByteSpan updateToken  = response.updateToken.ValueOr({});
    CharSpan resourceName = ExtractResourceName(imageURI);
    uint32_t version      = response.softwareVersion.ValueOr(0u);

    ChipLogProgress(SoftwareUpdate, "Received QueryImage response: %" PRIu16, static_cast<uint16_t>(response.status));
    ChipLogProgress(SoftwareUpdate, "  Image URI: %.*s", static_cast<int>(imageURI.size()), imageURI.data());
    ChipLogProgress(SoftwareUpdate, "  Resource: %.*s", static_cast<int>(resourceName.size()), resourceName.data());
    ChipLogProgress(SoftwareUpdate, "  Version: %" PRIu32, version);

    if (updateToken.size() > kMaxUpdateTokenLen)
    {
        ChipLogError(SoftwareUpdate, "Update token too long");
        return;
    }

    memcpy(sOtaContext.updateToken, updateToken.data(), updateToken.size());
    sOtaContext.updateTokenLen = static_cast<uint8_t>(updateToken.size());
    sOtaContext.updateVersion  = version;

    OperationalDeviceProxy * deviceProxy = sOtaContext.deviceProxy;
    VerifyOrReturn(deviceProxy != nullptr);

    Messaging::ExchangeManager * exchangeMgr = deviceProxy->GetExchangeManager();
    Optional<SessionHandle> session          = deviceProxy->GetSecureSession();
    Messaging::ExchangeContext * exchangeCtx = nullptr;

    if (exchangeMgr != nullptr && session.HasValue())
    {
        exchangeCtx = exchangeMgr->NewContext(session.Value(), &sDfuManager);
    }

    if (exchangeCtx == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed to allocate exchange");
        return;
    }

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = kBlockSize;
    initOptions.FileDesLength    = resourceName.size();
    initOptions.FileDesignator   = reinterpret_cast<const uint8_t *>(resourceName.data());

    sDfuManager.SetInitialExchange(exchangeCtx);
    CHIP_ERROR error = sDfuManager.InitiateTransfer(&DeviceLayer::SystemLayer(), TransferRole::kReceiver, initOptions,
                                                    System::Clock::Seconds16(20));

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Failed to initiate BDX transfer: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void OnQueryImageFailure(void * /* context */, EmberAfStatus status)
{
    ChipLogError(SoftwareUpdate, "QueryImage failed: %" PRIu16, static_cast<uint16_t>(status));
}

void OnQueryImageConnection(void * /* context */, DeviceProxy * deviceProxy)
{
    // Initialize cluster object
    Controller::OtaSoftwareUpdateProviderCluster cluster;
    CHIP_ERROR error = cluster.Associate(deviceProxy, sOtaContext.providerEndpointId);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Send QueryImage command
    uint16_t vendorId        = 0;
    uint16_t productId       = 0;
    uint16_t hardwareVersion = 0;
    uint16_t softwareVersion = 0;

    DeviceLayer::ConfigurationMgr().GetVendorId(vendorId);
    DeviceLayer::ConfigurationMgr().GetProductId(productId);
    DeviceLayer::ConfigurationMgr().GetProductRevision(hardwareVersion);
    DeviceLayer::ConfigurationMgr().GetFirmwareRevision(softwareVersion);

    QueryImage::Type request;
    request.vendorId           = static_cast<VendorId>(vendorId);
    request.productId          = productId;
    request.softwareVersion    = softwareVersion;
    request.protocolsSupported = kRequestorProtocols;
    request.hardwareVersion.SetValue(hardwareVersion);
    request.location.Emplace(kRequestorLocation);
    request.requestorCanConsent.SetValue(kRequestorCanConsent);
    error = cluster.InvokeCommand(request, /* context */ nullptr, OnQueryImageResponse, OnQueryImageFailure);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "QueryImage failed: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void OnApplyUpdateResponse(void * context, const ApplyUpdateResponse::DecodableType & response)
{
    ChipLogProgress(SoftwareUpdate, "Received ApplyUpdate response: %" PRIu16, static_cast<uint16_t>(response.action));

    switch (response.action)
    {
    case EMBER_ZCL_OTA_APPLY_UPDATE_ACTION_PROCEED: {
        CHIP_ERROR error = sDfuManager.ApplyUpdate();
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to apply update: %" CHIP_ERROR_FORMAT, error.Format());
        }
        break;
    }
    case EMBER_ZCL_OTA_APPLY_UPDATE_ACTION_DISCONTINUE: {
        CHIP_ERROR error = sDfuManager.DiscardUpdate();
        if (error != CHIP_NO_ERROR)
        {
            ChipLogError(SoftwareUpdate, "Failed to discard update: %" CHIP_ERROR_FORMAT, error.Format());
        }
        break;
    }
    default:
        break;
    }
}

void OnApplyUpdateFailure(void * /* context */, EmberAfStatus status)
{
    ChipLogError(SoftwareUpdate, "ApplyUpdate failed: %" PRIu16, static_cast<uint16_t>(status));
}

void OnApplyUpdateConnection(void * /* context */, DeviceProxy * deviceProxy)
{
    // Initialize cluster object
    Controller::OtaSoftwareUpdateProviderCluster cluster;
    CHIP_ERROR error = cluster.Associate(deviceProxy, sOtaContext.providerEndpointId);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate failed: %" CHIP_ERROR_FORMAT, error.Format());
        return;
    }

    // Send QueryImage command
    uint16_t vendorId        = 0;
    uint16_t productId       = 0;
    uint16_t hardwareVersion = 0;
    uint16_t softwareVersion = 0;

    DeviceLayer::ConfigurationMgr().GetVendorId(vendorId);
    DeviceLayer::ConfigurationMgr().GetProductId(productId);
    DeviceLayer::ConfigurationMgr().GetProductRevision(hardwareVersion);
    DeviceLayer::ConfigurationMgr().GetFirmwareRevision(softwareVersion);

    ApplyUpdateRequest::Type request;
    request.updateToken = ByteSpan(sOtaContext.updateToken, sOtaContext.updateTokenLen);
    request.newVersion  = sOtaContext.updateVersion;

    error = cluster.InvokeCommand(request, /* context */ nullptr, OnApplyUpdateResponse, OnApplyUpdateFailure);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "ApplyUpdate failed: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void OnConnectionFailure(void * /* context */, NodeId nodeId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "Connection failed: %" CHIP_ERROR_FORMAT, error.Format());
}

template <OnDeviceConnected OnConnected>
void ConnectDeviceAsync(intptr_t)
{
    static Callback::Callback<OnDeviceConnected> successCallback(OnConnected, nullptr);
    static Callback::Callback<OnDeviceConnectionFailure> failureCallback(OnConnectionFailure, nullptr);

    OperationalDeviceProxy * deviceProxy = sOtaContext.deviceProxy;
    VerifyOrReturn(deviceProxy != nullptr);

    deviceProxy->UpdateDeviceData(sOtaContext.providerAddress, deviceProxy->GetMRPConfig());
    deviceProxy->Connect(&successCallback, &failureCallback);
}

template <OnDeviceConnected OnConnected>
CHIP_ERROR ConnectProvider(FabricIndex fabricIndex, NodeId nodeId, const Transport::PeerAddress & address)
{
    // Allocate new device proxy
    FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(fabricIndex);
    VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    DeviceProxyInitParams initParams = { .sessionManager = &Server::GetInstance().GetSecureSessionManager(),
                                         .exchangeMgr    = &Server::GetInstance().GetExchangeManager(),
                                         .idAllocator    = &Server::GetInstance().GetSessionIDAllocator(),
                                         .fabricInfo     = fabric,
                                         .imDelegate     = &sIMDelegate };

    auto deviceProxy = Platform::New<OperationalDeviceProxy>(initParams, fabric->GetPeerIdForNode(nodeId));
    VerifyOrReturnError(deviceProxy != nullptr, CHIP_ERROR_NO_MEMORY);

    // Initiate the connection and send QueryImage command using CHIP thread
    streamer_printf(streamer_get(), "Connecting...\r\n");

    sOtaContext.deviceProxy     = deviceProxy;
    sOtaContext.providerAddress = address;
    DeviceLayer::PlatformMgr().ScheduleWork(ConnectDeviceAsync<OnConnected>);
    return CHIP_NO_ERROR;
}

CHIP_ERROR QueryImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 5, CHIP_ERROR_INVALID_ARGUMENT);

    const FabricIndex fabricIndex       = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
    const NodeId providerNodeId         = static_cast<NodeId>(strtoull(argv[1], nullptr, 10));
    const EndpointId providerEndpointId = static_cast<EndpointId>(strtoul(argv[2], nullptr, 10));
    const char * ipAddressStr           = argv[3];
    const uint16_t udpPort              = static_cast<uint16_t>(strtoul(argv[4], nullptr, 10));

    Inet::IPAddress ipAddress;
    VerifyOrReturnError(Inet::IPAddress::FromString(ipAddressStr, ipAddress), CHIP_ERROR_INVALID_ARGUMENT);

    sOtaContext.Clear();
    sOtaContext.providerEndpointId = providerEndpointId;

    return ConnectProvider<OnQueryImageConnection>(fabricIndex, providerNodeId, Transport::PeerAddress::UDP(ipAddress, udpPort));
}

CHIP_ERROR ShowUpdateHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 0, CHIP_ERROR_INVALID_ARGUMENT);

    char token[kMaxUpdateTokenLen * 2 + 1];
    ReturnErrorOnFailure(
        Encoding::BytesToUppercaseHexString(sOtaContext.updateToken, sOtaContext.updateTokenLen, token, sizeof(token)));

    streamer_printf(streamer_get(), "Token: %s\r\n", token);
    streamer_printf(streamer_get(), "Version: %" PRIu32 "\r\n", sOtaContext.updateVersion);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ApplyImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 7, CHIP_ERROR_INVALID_ARGUMENT);

    const FabricIndex fabricIndex       = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
    const NodeId providerNodeId         = static_cast<NodeId>(strtoull(argv[1], nullptr, 10));
    const EndpointId providerEndpointId = static_cast<EndpointId>(strtoul(argv[2], nullptr, 10));
    const char * ipAddressStr           = argv[3];
    const uint16_t udpPort              = static_cast<uint16_t>(strtoul(argv[4], nullptr, 10));
    const char * updateTokenStr         = argv[5];
    const uint32_t updateVersion        = static_cast<uint32_t>(strtoul(argv[6], nullptr, 10));

    Inet::IPAddress ipAddress;
    VerifyOrReturnError(Inet::IPAddress::FromString(ipAddressStr, ipAddress), CHIP_ERROR_INVALID_ARGUMENT);

    uint8_t updateToken[kMaxUpdateTokenLen];
    size_t updateTokenLen = Encoding::HexToBytes(updateTokenStr, strlen(updateTokenStr), updateToken, sizeof(updateToken));
    VerifyOrReturnError(updateTokenLen > 0, CHIP_ERROR_INVALID_ARGUMENT);

    sOtaContext.Clear();
    sOtaContext.providerEndpointId = providerEndpointId;
    sOtaContext.updateTokenLen     = updateTokenLen;
    sOtaContext.updateVersion      = updateVersion;
    memcpy(sOtaContext.updateToken, updateToken, updateTokenLen);

    return ConnectProvider<OnApplyUpdateConnection>(fabricIndex, providerNodeId, Transport::PeerAddress::UDP(ipAddress, udpPort));
}

CHIP_ERROR OtaHandler(int argc, char ** argv)
{
    if (argc == 0)
    {
        sSubShell.ForEachCommand(PrintCommandHelp, nullptr);
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR error = sSubShell.ExecCommand(argc, argv);

    if (error != CHIP_NO_ERROR)
    {
        streamer_printf(streamer_get(), "Error: %" CHIP_ERROR_FORMAT "\r\n", error.Format());
    }

    return error;
}
} // namespace

void RegisterOtaCommands()
{
    // Register subcommands of the `ota` commands.
    static const shell_command_t subCommands[] = {
        { &QueryImageHandler, "query",
          "Query for a new image. Usage: ota query <fabric-index> <provider-node-id> <endpoint-id> <ip-address> <port>" },
        { &ShowUpdateHandler, "show-update", "Print the current update information" },
        { &ApplyImageHandler, "apply",
          "Apply the current update. Usage ota apply <fabric-index> <provider-node-id> <endpoint-id> <ip-address> <port> "
          "<update-token> <version>" },
    };

    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    // Register the root `ota` command in the top-level shell.
    static const shell_command_t otaCommand = { &OtaHandler, "ota", "OTA commands" };

    Engine::Root().RegisterCommands(&otaCommand, 1);
}

} // namespace Shell
} // namespace chip
