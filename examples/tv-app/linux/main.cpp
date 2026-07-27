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

#include "media-file-management/MediaFileManagementManager.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/clusters/media-file-management-server/CodegenIntegration.h>
#include <app/util/endpoint-config-api.h>

#include <optional>

#if defined(ENABLE_CHIP_SHELL)
#include "AppTvShellCommands.h"
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;

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
