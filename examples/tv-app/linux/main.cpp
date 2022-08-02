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

#include "AppImpl.h"
#include "AppMain.h"
#include "AppPlatformShellCommands.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/app-platform/ContentAppPlatform.h>
#include <app/util/af.h>

#include "include/account-login/AccountLoginManager.h"
#include "include/application-basic/ApplicationBasicManager.h"
#include "include/application-launcher/ApplicationLauncherManager.h"
#include "include/audio-output/AudioOutputManager.h"
#include "include/channel/ChannelManager.h"
#include "include/content-launcher/ContentLauncherManager.h"
#include "include/keypad-input/KeypadInputManager.h"
#include "include/low-power/LowPowerManager.h"
#include "include/media-input/MediaInputManager.h"
#include "include/media-playback/MediaPlaybackManager.h"
#include "include/target-navigator/TargetNavigatorManager.h"
#include "include/wake-on-lan/WakeOnLanManager.h"

#if defined(ENABLE_CHIP_SHELL)
#include <lib/shell/Engine.h>
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;
using namespace chip::app::Clusters;

bool emberAfBasicClusterMfgSpecificPingCallback(app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

namespace {
static AccountLoginManager accountLoginManager;
static ApplicationBasicManager applicationBasicManager;
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
static ApplicationLauncherManager applicationLauncherManager(true);
#else  // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
static ApplicationLauncherManager applicationLauncherManager(false);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
static AudioOutputManager audioOutputManager;
static ChannelManager channelManager;
static ContentLauncherManager contentLauncherManager;
static KeypadInputManager keypadInputManager;
static LowPowerManager lowPowerManager;
static MediaInputManager mediaInputManager;
static MediaPlaybackManager mediaPlaybackManager;
static TargetNavigatorManager targetNavigatorManager;
static WakeOnLanManager wakeOnLanManager;
} // namespace

void ApplicationInit()
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationInit()");

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    ChipLogDetail(DeviceLayer, "TV Linux App: Warning - Fixed Content App Endpoint Not Disabled");
    // Can't disable this without breaking CI unit tests that act upon account login cluster (only available on ep3)
    // emberAfEndpointEnableDisable(3, false);
}

int main(int argc, char * argv[])
{

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

    InitVideoPlayerPlatform();

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    Shell::RegisterAppPlatformCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}

void emberAfContentLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ContentLauncher::SetDefaultDelegate");
    ContentLauncher::SetDefaultDelegate(endpoint, &contentLauncherManager);
}

void emberAfAccountLoginClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AccountLogin::SetDefaultDelegate");
    AccountLogin::SetDefaultDelegate(endpoint, &accountLoginManager);
}

void emberAfApplicationBasicClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationBasic::SetDefaultDelegate");
    ApplicationBasic::SetDefaultDelegate(endpoint, &applicationBasicManager);
}

void emberAfApplicationLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationLauncher::SetDefaultDelegate");
    ApplicationLauncher::SetDefaultDelegate(endpoint, &applicationLauncherManager);
}

void emberAfAudioOutputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AudioOutput::SetDefaultDelegate");
    AudioOutput::SetDefaultDelegate(endpoint, &audioOutputManager);
}

void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: Channel::SetDefaultDelegate");
    Channel::SetDefaultDelegate(endpoint, &channelManager);
}

void emberAfKeypadInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: KeypadInput::SetDefaultDelegate");
    KeypadInput::SetDefaultDelegate(endpoint, &keypadInputManager);
}

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}

void emberAfMediaInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaInput::SetDefaultDelegate");
    MediaInput::SetDefaultDelegate(endpoint, &mediaInputManager);
}

void emberAfMediaPlaybackClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaPlayback::SetDefaultDelegate");
    MediaPlayback::SetDefaultDelegate(endpoint, &mediaPlaybackManager);
}

void emberAfTargetNavigatorClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: TargetNavigator::SetDefaultDelegate");
    TargetNavigator::SetDefaultDelegate(endpoint, &targetNavigatorManager);
}

void emberAfWakeOnLanClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: WakeOnLanManager::SetDefaultDelegate");
    WakeOnLan::SetDefaultDelegate(endpoint, &wakeOnLanManager);
}
