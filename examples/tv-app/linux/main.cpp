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
#include <app/Command.h>
#include <app/util/ContentAppPlatform.h>
#include <app/util/af.h>

#include <iostream>

#include "include/application-launcher/ApplicationLauncherManager.h"
#include "include/audio-output/AudioOutputManager.h"
#include "include/content-launcher/ContentLauncherManager.h"
#include "include/keypad-input/KeypadInputManager.h"
#include "include/media-input/MediaInputManager.h"
#include "include/media-playback/MediaPlaybackManager.h"
#include "include/target-navigator/TargetNavigatorManager.h"
#include "include/tv-channel/TvChannelManager.h"

#if defined(ENABLE_CHIP_SHELL)
#include <lib/shell/Engine.h>
#endif

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::AppPlatform;

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::Command * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    chip::AppPlatform::ContentAppFactoryImpl factory;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

    // Init Keypad Input manager
    err = KeypadInputManager().Init();
    SuccessOrExit(err);

    // Init Application Launcher Manager
    err = ApplicationLauncherManager().Init();
    SuccessOrExit(err);

    // Init Audio Output Manager
    err = AudioOutputManager().Init();
    SuccessOrExit(err);

    // Init Content Launcher Manager
    err = ContentLauncherManager().Init();
    SuccessOrExit(err);

    // Init Media Input Manager
    err = MediaInputManager().Init();
    SuccessOrExit(err);

    // Init Media Playback Manager
    err = MediaPlaybackManager().Init();
    SuccessOrExit(err);

    // Init Target Navigator Manager
    err = TargetNavigatorManager().Init();
    SuccessOrExit(err);

    // Init Tv Channel Manager
    err = TvChannelManager().Init();
    SuccessOrExit(err);

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
exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run TV App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}
