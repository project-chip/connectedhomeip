/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "AppMain.h"
#include "AppTv.h"

#include "media-file-management/MediaFileManagementBdxCoordinator.h"
#include "media-file-management/MediaFileManagementBdxProvider.h"
#include "media-file-management/MediaFileManagementBdxRequestor.h"
#include "media-file-management/MediaFileManagementManager.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/clusters/media-file-management-server/CodegenIntegration.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <lib/support/CHIPArgParser.hpp>
#include <platform/ConfigurationManager.h>
#include <protocols/Protocols.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <controller/CHIPDeviceController.h> // nogncheck
#endif                                       // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#include <cstring>
#include <optional>

#if defined(ENABLE_CHIP_SHELL)
#include "AppTvShellCommands.h"
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
// Defined in CommissionerMain.cpp; the combined server/commissioner tv-app runs
// a commissioner stack with its own ExchangeManager (see ApplicationInit).
extern Controller::DeviceCommissioner * GetDeviceCommissioner();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

namespace {

// Endpoint that hosts the Media File Management cluster in tv-app.zap.
constexpr EndpointId kMediaFileManagementEndpointId = 1;

// The Media File Management cluster is code-driven, so the application owns the
// delegate and the cluster registration lifecycle (unlike the legacy Ember
// clusters wired up via SetDefaultDelegate in ZCLCallbacks.cpp).
//
// Both are constructed in ApplicationInit() (not as eagerly-constructed globals)
// because the manager's constructor performs filesystem I/O and logging, which
// must not run during static initialization before the SDK is ready.
std::optional<MediaFileManagement::MediaFileManagementManager> gMediaFileManagementManager;
std::optional<MediaFileManagement::MediaFileManagementServer> gMediaFileManagementServer;

// BDX endpoints that move the actual file/thumbnail bytes for the sharing
// commands (the cluster itself only carries metadata). The provider serves
// GetSharedFile pulls; the requestor downloads for AddFile/OfferFile. The
// coordinator ties them to the manager and owns the RequestID/ResponseID map.
std::optional<MediaFileManagement::MediaFileManagementBdxProvider> gMediaFileManagementBdxProvider;
std::optional<MediaFileManagement::MediaFileManagementBdxRequestor> gMediaFileManagementBdxRequestor;
std::optional<MediaFileManagement::MediaFileManagementBdxCoordinator> gMediaFileManagementBdxCoordinator;

// ---------------------------------------------------------------------------
// --device-type flag: choose which media device type endpoint 1 presents as.
//
// tv-app is built as a Casting Video Player (0x0023) in tv-app.zap, but
// endpoint 1 hosts a superset of clusters that also satisfies the mandatory set
// of the other three media player device types. This flag makes the app present
// as any of the four without a rebuild by, at boot:
//   - overriding the device type id used for DNS-SD "_T<id>" commissioning
//     advertising (ConfigurationMgr().SetDeviceTypeId, applied during argument
//     parsing so it is in place before the server starts advertising), and
//   - rewriting endpoint 1's Descriptor DeviceTypeList (emberAfSetDeviceTypeList,
//     applied in ApplicationInit once the endpoint table is populated).
// See examples/tv-app/README.md.
//
// NOTE: this changes the advertised and declared device type only. The
// commissioner role (Casting players are Commissioners) and the cluster set are
// as compiled; the build-time variant documented in the README is the path to a
// fully faithful data model.
constexpr EndpointId kVideoPlayerEndpointId = 1;

struct MediaDeviceTypeOption
{
    const char * name;
    EmberAfDeviceType deviceType; // { deviceTypeId, deviceTypeRevision }
};

// Revisions match the current Device Library (video players, revision 2) and the
// streaming-casting-speakers spec additions (audio players, revision 1).
constexpr MediaDeviceTypeOption kMediaDeviceTypes[] = {
    { "casting-video", { 0x0023, 2 } },   // Casting Video Player (tv-app.zap default)
    { "basic-video", { 0x0028, 2 } },     // Basic Video Player
    { "casting-audio", { 0x0021, 1 } },   // Casting Audio Player
    { "streaming-audio", { 0x0020, 1 } }, // Streaming Audio Player
};

// Backing storage for the runtime override; emberAfSetDeviceTypeList stores the
// span (not a copy), so this must outlive the endpoint - hence file scope.
EmberAfDeviceType gMediaDeviceTypeList[1];
bool gMediaDeviceTypeOverridden = false;

constexpr uint16_t kOptionDeviceType = 0xffe0;

bool TvAppOptionHandler(const char * program, chip::ArgParser::OptionSet * options, int identifier, const char * name,
                        const char * value)
{
    if (identifier != kOptionDeviceType)
    {
        ChipLogError(DeviceLayer, "%s: INTERNAL ERROR: Unhandled option: %s", program, name);
        return false;
    }

    for (const MediaDeviceTypeOption & option : kMediaDeviceTypes)
    {
        if (strcmp(value, option.name) == 0)
        {
            gMediaDeviceTypeList[0]    = option.deviceType;
            gMediaDeviceTypeOverridden = true;
            // Override the DNS-SD advertised device type now, before the server
            // starts advertising. The Descriptor DeviceTypeList is updated later
            // in ApplicationInit (the endpoint table does not exist yet here).
            CHIP_ERROR err = ConfigurationMgr().SetDeviceTypeId(option.deviceType.deviceTypeId);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "%s: failed to override advertised device type: %" CHIP_ERROR_FORMAT, program,
                             err.Format());
            }
            ChipLogProgress(DeviceLayer, "TV Linux App: endpoint 1 device type selected: %s (0x%04X revision %u)", option.name,
                            static_cast<unsigned>(option.deviceType.deviceTypeId), option.deviceType.deviceTypeRevision);
            return true;
        }
    }

    ChipLogError(DeviceLayer, "%s: unknown --device-type '%s' (expected casting-video|basic-video|casting-audio|streaming-audio)",
                 program, value);
    return false;
}

chip::ArgParser::OptionDef sTvAppOptionDefs[] = {
    { "device-type", chip::ArgParser::kArgumentRequired, kOptionDeviceType },
    { nullptr },
};

chip::ArgParser::OptionSet sTvAppOptions = {
    TvAppOptionHandler,
    sTvAppOptionDefs,
    "TV APP OPTIONS",
    "  --device-type <casting-video|basic-video|casting-audio|streaming-audio>\n"
    "       Present endpoint 1 as the given media device type: sets both the DNS-SD\n"
    "       _T advertising subtype and the Descriptor cluster DeviceTypeList.\n"
    "       Defaults to casting-video, as built into tv-app.zap. The commissioner\n"
    "       role and cluster set are unchanged. See examples/tv-app/README.md.\n",
};

} // namespace

void ApplicationInit()
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationInit()");

    // Register the code-driven Media File Management cluster on endpoint 1.
    gMediaFileManagementManager.emplace();
    gMediaFileManagementServer.emplace(kMediaFileManagementEndpointId, *gMediaFileManagementManager);
    CHIP_ERROR err = gMediaFileManagementServer->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "TV Linux App: MediaFileManagement init failed: %" CHIP_ERROR_FORMAT, err.Format());
        gMediaFileManagementServer.reset();
        gMediaFileManagementManager.reset();
    }
    else
    {
        // Bring up the BDX byte-transfer layer for the sharing commands.
        gMediaFileManagementBdxProvider.emplace();
        gMediaFileManagementBdxRequestor.emplace();
        gMediaFileManagementBdxCoordinator.emplace(*gMediaFileManagementManager, *gMediaFileManagementBdxProvider,
                                                   *gMediaFileManagementBdxRequestor, gMediaFileManagementServer->Cluster());
        gMediaFileManagementManager->SetBdxCoordinator(&*gMediaFileManagementBdxCoordinator);

        // The provider serves incoming (client-initiated) BDX pulls, so it must
        // be the unsolicited handler for the BDX protocol. This tv-app is a
        // combined server + commissioner: the commissioner owns a *separate*
        // ExchangeManager, and clients commission via UDC onto the commissioner's
        // fabric, so their GetSharedFile BDX ReceiveInit arrives on the
        // commissioner's EM - not the server's. Register on both so the provider
        // is found regardless of which stack terminates the client's session.
        err = Server::GetInstance().GetExchangeManager().RegisterUnsolicitedMessageHandlerForProtocol(
            Protocols::BDX::Id, &*gMediaFileManagementBdxProvider);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "TV Linux App: BDX handler registration (server) failed: %" CHIP_ERROR_FORMAT, err.Format());
        }

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
        if (Controller::DeviceCommissioner * commissioner = GetDeviceCommissioner();
            commissioner != nullptr && commissioner->ExchangeMgr() != nullptr)
        {
            err = commissioner->ExchangeMgr()->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::BDX::Id,
                                                                                            &*gMediaFileManagementBdxProvider);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Zcl, "TV Linux App: BDX handler registration (commissioner) failed: %" CHIP_ERROR_FORMAT,
                             err.Format());
            }
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    }

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    ChipLogDetail(DeviceLayer, "TV Linux App: Warning - Fixed Content App Endpoint Not Disabled");
    // Can't disable this without breaking CI unit tests that act upon account login cluster (only available on ep3)
    // emberAfEndpointEnableDisable(3, false);

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    // Install Content Apps
    ContentAppFactoryImpl * factory = GetContentAppFactoryImpl();

    // Content App 1
    constexpr uint16_t kApp1VendorId  = 65521;
    constexpr uint16_t kApp1ProductId = 32769;
    factory->InstallContentApp(kApp1VendorId, kApp1ProductId);

    // Content App 2
    constexpr uint16_t kApp2VendorId  = 1;
    constexpr uint16_t kApp2ProductId = 11;
    factory->InstallContentApp(kApp2VendorId, kApp2ProductId);

    // Content App 3
    constexpr uint16_t kApp3VendorId  = 9050;
    constexpr uint16_t kApp3ProductId = 22;
    factory->InstallContentApp(kApp3VendorId, kApp3ProductId);

    // Content App 4
    constexpr uint16_t kApp4VendorId  = 1111;
    constexpr uint16_t kApp4ProductId = 22;
    factory->InstallContentApp(kApp4VendorId, kApp4ProductId);

    // Content App 5 - OAuth-only login (see OAuthAccountLoginManager)
    constexpr uint16_t kApp5VendorId  = 4242;
    constexpr uint16_t kApp5ProductId = 1;
    factory->InstallContentApp(kApp5VendorId, kApp5ProductId);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    // Apply the --device-type override (if any) to endpoint 1's declared device
    // type. Done here, after the server has populated the endpoint table.
    if (gMediaDeviceTypeOverridden)
    {
        CHIP_ERROR err2 = emberAfSetDeviceTypeList(kVideoPlayerEndpointId, Span<const EmberAfDeviceType>(gMediaDeviceTypeList, 1));
        if (err2 != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "TV Linux App: failed to set endpoint 1 device type: %" CHIP_ERROR_FORMAT, err2.Format());
        }
        else
        {
            ChipLogProgress(Zcl, "TV Linux App: endpoint 1 now declares device type 0x%04X (DNS-SD _T subtype set to match).",
                            static_cast<unsigned>(gMediaDeviceTypeList[0].deviceTypeId));
        }
    }
}

void ApplicationShutdown()
{
    // Unregister the cluster from the data model provider. Shutdown() mirrors
    // Init(); the optionals are left intact and torn down at process exit.
    if (gMediaFileManagementBdxProvider.has_value())
    {
        TEMPORARY_RETURN_IGNORED Server::GetInstance().GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(
            Protocols::BDX::Id);
#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
        // The provider was also registered on the commissioner's separate
        // ExchangeManager in ApplicationInit; unregister it there too.
        if (Controller::DeviceCommissioner * commissioner = GetDeviceCommissioner();
            commissioner != nullptr && commissioner->ExchangeMgr() != nullptr)
        {
            TEMPORARY_RETURN_IGNORED commissioner->ExchangeMgr()->UnregisterUnsolicitedMessageHandlerForProtocol(
                Protocols::BDX::Id);
        }
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
        gMediaFileManagementBdxProvider->AbortTransfer();
    }
    if (gMediaFileManagementBdxRequestor.has_value())
    {
        gMediaFileManagementBdxRequestor->AbortTransfer();
    }
    if (gMediaFileManagementServer.has_value())
    {
        gMediaFileManagementServer->Shutdown();
    }
}

int main(int argc, char * argv[])
{

    VerifyOrDie(ChipLinuxAppInit(argc, argv, &sTvAppOptions) == 0);

    TEMPORARY_RETURN_IGNORED AppTvInit();

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    Shell::RegisterAppTvCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}
