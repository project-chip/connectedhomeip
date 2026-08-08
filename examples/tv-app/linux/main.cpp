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
#include <app/util/endpoint-config-api.h>
#include <protocols/Protocols.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <controller/CHIPDeviceController.h> // nogncheck
#endif                                       // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

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
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
}

void ApplicationShutdown()
{
    // Unregister the cluster from the data model provider. Shutdown() mirrors
    // Init(); the optionals are left intact and torn down at process exit.
    if (gMediaFileManagementBdxProvider.has_value())
    {
        TEMPORARY_RETURN_IGNORED Server::GetInstance().GetExchangeManager().UnregisterUnsolicitedMessageHandlerForProtocol(
            Protocols::BDX::Id);
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

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    TEMPORARY_RETURN_IGNORED AppTvInit();

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    Shell::RegisterAppTvCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}
