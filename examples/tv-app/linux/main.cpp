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

#include <cassert>
#include <iostream>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/Command.h>
#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/RandUtils.h>

#include "AppMain.h"

#include <iostream>
#include <lib/support/ErrorStr.h>

#include "include/application-launcher/ApplicationLauncherManager.h"
#include "include/audio-output/AudioOutputManager.h"
#include "include/content-launcher/ContentLauncherManager.h"
#include "include/keypad-input/KeypadInputManager.h"
#include "include/media-input/MediaInputManager.h"
#include "include/media-playback/MediaPlaybackManager.h"
#include "include/target-navigator/TargetNavigatorManager.h"
#include "include/tv-channel/TvChannelManager.h"

using namespace chip;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{}

bool emberAfBasicClusterMfgSpecificPingCallback(chip::app::Command * commandObj)
{
    emberAfSendDefaultResponse(emberAfCurrentCommand(), EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

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
