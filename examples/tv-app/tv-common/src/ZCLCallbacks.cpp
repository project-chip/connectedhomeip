/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceConfig.h>

#include "account-login/AccountLoginManager.h"
#include "application-basic/ApplicationBasicManager.h"
#include "application-launcher/ApplicationLauncherManager.h"
#include "audio-output/AudioOutputManager.h"
#include "channel/ChannelManager.h"
#include "content-control/ContentController.h"
#include "content-launcher/ContentLauncherManager.h"
#include "keypad-input/KeypadInputManager.h"
#include "low-power/LowPowerManager.h"
#include "media-input/MediaInputManager.h"
#include "media-playback/MediaPlaybackManager.h"
#include "messages/MessagesManager.h"
#include "target-navigator/TargetNavigatorManager.h"
#include "wake-on-lan/WakeOnLanManager.h"

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
static ContentControlManager contentControlManager;
static ContentLauncherManager contentLauncherManager;
static KeypadInputManager keypadInputManager;
static LowPowerManager lowPowerManager;
static MediaInputManager mediaInputManager;
static MediaPlaybackManager mediaPlaybackManager;
static MessagesManager messagesManager;
static TargetNavigatorManager targetNavigatorManager;
static WakeOnLanManager wakeOnLanManager;
} // namespace

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == OnOff::Id && attributeId == OnOff::Attributes::OnOff::Id)
    {
        ChipLogProgress(Zcl, "OnOff attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u", ChipLogValueMEI(attributeId),
                        type, *value, size);
    }
    else if (clusterId == LevelControl::Id)
    {
        ChipLogProgress(Zcl, "Level Control attribute ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributeId), type, *value, size);

        // WIP Apply attribute change to Light
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
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

void emberAfContentControlClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ContentControlManager::SetDefaultDelegate");
    ContentControl::SetDefaultDelegate(endpoint, &contentControlManager);
}

void emberAfContentLauncherClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: ContentLauncher::SetDefaultDelegate");
    ContentLauncher::SetDefaultDelegate(endpoint, &contentLauncherManager);
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

void emberAfMessagesClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "TV Linux App: Messages::SetDefaultDelegate");
    Messages::SetDefaultDelegate(endpoint, &messagesManager);
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
