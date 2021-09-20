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
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>

// TODO: Use common software update layer to store the image
#if CHIP_DEVICE_LAYER_TARGET_NRFCONNECT
#include "BDXDownloader_nrfconnect.h"
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

constexpr EndpointId kProviderEndpointId                   = 0;
constexpr const char kRequestorLocation[]                  = { 'U', 'S' };
constexpr EmberAfOTADownloadProtocol kRequestorProtocols[] = { EMBER_ZCL_OTA_DOWNLOAD_PROTOCOL_BDX_SYNCHRONOUS };
constexpr bool kRequestorCanConsent                        = false;

BdxDownloader sBdxDownloader;
Controller::DeviceControllerInteractionModelDelegate sIMDelegate;
Transport::PeerAddress sProviderAddress;
Shell::Engine sSubShell;

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
    ChipLogProgress(SoftwareUpdate, "Received QueryImage response: %" PRIu16, static_cast<uint16_t>(response.status));

    CharSpan imageURI     = response.imageURI.ValueOr({});
    CharSpan resourceName = ExtractResourceName(imageURI);
    ChipLogProgress(SoftwareUpdate, "  Image URI: %.*s", static_cast<int>(imageURI.size()), imageURI.data());
    ChipLogProgress(SoftwareUpdate, "  Resource: %.*s", static_cast<int>(resourceName.size()), resourceName.data());

    OperationalDeviceProxy * deviceProxy = Server::GetInstance().GetOperationalDeviceProxy();
    VerifyOrReturn(deviceProxy != nullptr);

    Messaging::ExchangeManager * exchangeMgr = deviceProxy->GetExchangeManager();
    Optional<SessionHandle> session          = deviceProxy->GetSecureSession();
    Messaging::ExchangeContext * exchangeCtx = nullptr;

    if (exchangeMgr != nullptr && session.HasValue())
    {
        exchangeCtx = exchangeMgr->NewContext(session.Value(), &sBdxDownloader);
    }

    if (exchangeCtx == nullptr)
    {
        ChipLogError(SoftwareUpdate, "Failed to allocate exchange");
        return;
    }

    TransferSession::TransferInitData initOptions;
    initOptions.TransferCtlFlags = TransferControlFlags::kReceiverDrive;
    initOptions.MaxBlockSize     = 1024;
    initOptions.FileDesLength    = resourceName.size();
    initOptions.FileDesignator   = reinterpret_cast<const uint8_t *>(resourceName.data());

    sBdxDownloader.SetInitialExchange(exchangeCtx);
    CHIP_ERROR error = sBdxDownloader.InitiateTransfer(&DeviceLayer::SystemLayer(), TransferRole::kReceiver, initOptions,
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
    CHIP_ERROR error = cluster.Associate(deviceProxy, kProviderEndpointId);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(SoftwareUpdate, "Associate() failed: %" CHIP_ERROR_FORMAT, error.Format());
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
        ChipLogError(SoftwareUpdate, "QueryImage() failed: %" CHIP_ERROR_FORMAT, error.Format());
    }
}

void OnConnectionFailure(void * /* context */, NodeId nodeId, CHIP_ERROR error)
{
    ChipLogError(SoftwareUpdate, "Connection failed: %" CHIP_ERROR_FORMAT, error.Format());
}

void QueryImageAsyncHandler(intptr_t)
{
    static Callback::Callback<OnDeviceConnected> successCallback(OnQueryImageConnection, nullptr);
    static Callback::Callback<OnDeviceConnectionFailure> failureCallback(OnConnectionFailure, nullptr);

    OperationalDeviceProxy * deviceProxy = Server::GetInstance().GetOperationalDeviceProxy();
    VerifyOrReturn(deviceProxy != nullptr);

    deviceProxy->UpdateDeviceData(sProviderAddress, deviceProxy->GetMRPIdleInterval(), deviceProxy->GetMRPActiveInterval());
    deviceProxy->Connect(&successCallback, &failureCallback);
}

CHIP_ERROR QueryImageHandler(int argc, char ** argv)
{
    VerifyOrReturnError(argc == 4, CHIP_ERROR_INVALID_ARGUMENT);

    const FabricIndex providerFabricIndex = static_cast<FabricIndex>(strtoul(argv[0], nullptr, 10));
    const NodeId providerNodeId           = static_cast<NodeId>(strtoull(argv[1], nullptr, 10));
    const char * ipAddressStr             = argv[2];
    const uint16_t udpPort                = static_cast<uint16_t>(strtoul(argv[3], nullptr, 10));

    Inet::IPAddress ipAddress;
    VerifyOrReturnError(Inet::IPAddress::FromString(ipAddressStr, ipAddress), CHIP_ERROR_INVALID_ARGUMENT);

    FabricInfo * fabric = Server::GetInstance().GetFabricTable().FindFabricWithIndex(providerFabricIndex);
    VerifyOrReturnError(fabric != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    // Disconnect old device proxy if exists
    OperationalDeviceProxy * deviceProxy = Server::GetInstance().GetOperationalDeviceProxy();
    if (deviceProxy != nullptr)
    {
        deviceProxy->Disconnect();
        delete deviceProxy;
        Server::GetInstance().SetOperationalDeviceProxy(nullptr);
    }

    // Allocate new device proxy
    DeviceProxyInitParams initParams = { .sessionManager = &Server::GetInstance().GetSecureSessionManager(),
                                         .exchangeMgr    = &Server::GetInstance().GetExchangeManager(),
                                         .idAllocator    = &Server::GetInstance().GetSessionIDAllocator(),
                                         .fabricInfo     = fabric,
                                         .imDelegate     = &sIMDelegate };

    deviceProxy = Platform::New<OperationalDeviceProxy>(initParams, fabric->GetPeerIdForNode(providerNodeId));
    VerifyOrReturnError(deviceProxy != nullptr, CHIP_ERROR_NO_MEMORY);
    Server::GetInstance().SetOperationalDeviceProxy(deviceProxy);

    // Initiate the connection and send QueryImage command using CHIP thread
    streamer_printf(streamer_get(), "Querying image from %s:%" PRIu16 "... \r\n", ipAddressStr, udpPort);

    sProviderAddress = Transport::PeerAddress::UDP(ipAddress, udpPort);
    DeviceLayer::PlatformMgr().ScheduleWork(QueryImageAsyncHandler);

    return CHIP_NO_ERROR;
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
          "Query for new image. Usage: ota query <ota-provider-fabric-index> <ota-provider-node-id> <ip-address> <udp-port>" },
    };

    sSubShell.RegisterCommands(subCommands, ArraySize(subCommands));

    // Register the root `ota` command in the top-level shell.
    static const shell_command_t otaCommand = { &OtaHandler, "ota", "OTA commands" };

    Engine::Root().RegisterCommands(&otaCommand, 1);
}

} // namespace Shell
} // namespace chip
