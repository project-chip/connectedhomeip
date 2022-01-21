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

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/CommandHandler.h>
#include <app/util/ContentAppPlatform.h>
#include <app/util/af.h>

#include <iostream>

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

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::CommandHandler * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

namespace {
static AccountLoginManager accountLoginManager;
static ApplicationBasicManager applicationBasicManager;
static ApplicationLauncherManager applicationLauncherManager;
static AudioOutputManager audioOutputManager;
static ChannelManager channelManager;
static KeypadInputManager keypadInputManager;
static LowPowerManager lowPowerManager;
static MediaInputManager mediaInputManager;
static MediaPlaybackManager mediaPlaybackManager;
static TargetNavigatorManager targetNavigatorManager;
static WakeOnLanManager wakeOnLanManager;
} // namespace

void ApplicationInit() {}

int main(int argc, char * argv[])
{

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    chip::AppPlatform::ContentAppFactoryImpl factory;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    VerifyOrDie(ChipLinuxAppInit(argc, argv) == 0);

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    chip::AppPlatform::AppPlatform::GetInstance().SetupAppPlatform();
    chip::AppPlatform::AppPlatform::GetInstance().SetContentAppFactory(&factory);
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

#if defined(ENABLE_CHIP_SHELL)
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    chip::Shell::RegisterAppPlatformCommands();
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
#endif

    ChipLinuxAppMainLoop();

    return 0;
}

void emberAfContentLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ContentLauncher::SetDelegate");
    chip::app::Clusters::ContentLauncher::SetDelegate(endpoint, new ContentLauncherManager(endpoint));
}

void emberAfAccountLoginClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AccountLogin::SetDefaultDelegate");
    chip::app::Clusters::AccountLogin::SetDefaultDelegate(endpoint, &accountLoginManager);
}

void emberAfApplicationBasicClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationBasic::SetDefaultDelegate");
    chip::app::Clusters::ApplicationBasic::SetDefaultDelegate(endpoint, &applicationBasicManager);
}

void emberAfApplicationLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ApplicationLauncher::SetDefaultDelegate");
    chip::app::Clusters::ApplicationLauncher::SetDefaultDelegate(endpoint, &applicationLauncherManager);
}

void emberAfAudioOutputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: AudioOutput::SetDefaultDelegate");
    chip::app::Clusters::AudioOutput::SetDefaultDelegate(endpoint, &audioOutputManager);
}

void emberAfChannelClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: Channel::SetDefaultDelegate");
    chip::app::Clusters::Channel::SetDefaultDelegate(endpoint, &channelManager);
}

void emberAfKeypadInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: KeypadInput::SetDefaultDelegate");
    chip::app::Clusters::KeypadInput::SetDefaultDelegate(endpoint, &keypadInputManager);
}

void emberAfLowPowerClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: LowPower::SetDefaultDelegate");
    chip::app::Clusters::LowPower::SetDefaultDelegate(endpoint, &lowPowerManager);
}

void emberAfMediaInputClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaInput::SetDefaultDelegate");
    chip::app::Clusters::MediaInput::SetDefaultDelegate(endpoint, &mediaInputManager);
}

void emberAfMediaPlaybackClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: MediaPlayback::SetDefaultDelegate");
    chip::app::Clusters::MediaPlayback::SetDefaultDelegate(endpoint, &mediaPlaybackManager);
}

void emberAfTargetNavigatorClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: TargetNavigator::SetDefaultDelegate");
    chip::app::Clusters::TargetNavigator::SetDefaultDelegate(endpoint, &targetNavigatorManager);
}

void emberAfWakeOnLanClusterInitCallback(chip::EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: WakeOnLanManager::SetDefaultDelegate");
    chip::app::Clusters::WakeOnLan::SetDefaultDelegate(endpoint, &wakeOnLanManager);
}
