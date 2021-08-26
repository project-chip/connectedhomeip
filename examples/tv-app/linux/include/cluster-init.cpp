/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "application-basic/ApplicationBasicManager.h"
#include "application-launcher/ApplicationLauncherManager.h"
#include "audio-output/AudioOutputManager.h"
#include "cluster-util/ClusterManager.h"
#include "content-launcher/ContentLauncherManager.h"
#include "media-input/MediaInputManager.h"
#include "target-navigator/TargetNavigatorManager.h"
#include "tv-channel/TvChannelManager.h"
#include "wake-on-lan/WakeOnLanManager.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>

using namespace chip;

/** @brief Application Basic Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfApplicationBasicClusterInitCallback(chip::EndpointId endpoint)
{
    CHIP_ERROR err                     = CHIP_NO_ERROR;
    ApplicationBasicManager & aManager = ApplicationBasicManager::GetInstance();
    err                                = aManager.Init();
    if (CHIP_NO_ERROR == err)
    {
        Application application = aManager.getApplicationForEndpoint(endpoint);
        aManager.store(endpoint, &application);
    }
    else
    {
        ChipLogError(Zcl, "Failed to store application for endpoint: %d. Error:%s", endpoint, chip::ErrorStr(err));
    }
}

/** @brief Wake On LAN Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfWakeOnLanClusterInitCallback(chip::EndpointId endpoint)
{
    CHIP_ERROR err                = CHIP_NO_ERROR;
    WakeOnLanManager & wolManager = WakeOnLanManager::GetInstance();
    err                           = wolManager.Init();
    if (CHIP_NO_ERROR == err)
    {
        char macAddress[32] = "";
        wolManager.setMacAddress(endpoint, macAddress);
        wolManager.store(endpoint, macAddress);
    }
    else
    {
        ChipLogError(Zcl, "Failed to store mac address for endpoint: %d. Error:%s", endpoint, chip::ErrorStr(err));
    }
}

/** @brief Tv Channel  Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfTvChannelClusterInitCallback(EndpointId endpoint)
{
    ClusterManager().writeListAttribute(endpoint, ZCL_TV_CHANNEL_CLUSTER_ID, ZCL_TV_CHANNEL_LIST_ATTRIBUTE_ID,
                                        TvChannelManager().proxyGetTvChannelList());
}

/** @brief Application Launcher  Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfApplicationLauncherClusterInitCallback(EndpointId endpoint)
{
    ClusterManager().writeListAttribute(endpoint, ZCL_APPLICATION_LAUNCHER_CLUSTER_ID, ZCL_APPLICATION_LAUNCHER_LIST_ATTRIBUTE_ID,
                                        ApplicationLauncherManager().proxyGetApplicationList());
}

/** @brief Audio Output Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfAudioOutputClusterInitCallback(EndpointId endpoint)
{
    ClusterManager().writeListAttribute(endpoint, ZCL_AUDIO_OUTPUT_CLUSTER_ID, ZCL_AUDIO_OUTPUT_LIST_ATTRIBUTE_ID,
                                        AudioOutputManager().proxyGetListOfAudioOutputInfo());
}

/** @brief Content Launch Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfContentLauncherClusterInitCallback(EndpointId endpoint)
{
    ClusterManager().writeListAttribute(endpoint, ZCL_CONTENT_LAUNCH_CLUSTER_ID, ZCL_CONTENT_LAUNCHER_ACCEPTS_HEADER_ATTRIBUTE_ID,
                                        ContentLauncherManager().proxyGetAcceptsHeader());

    ClusterManager().writeListAttribute(endpoint, ZCL_CONTENT_LAUNCH_CLUSTER_ID,
                                        ZCL_CONTENT_LAUNCHER_SUPPORTED_STREAMING_TYPES_ATTRIBUTE_ID,
                                        ContentLauncherManager().proxyGetSupportedStreamingTypes());
}

/** @brief Media Input Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfMediaInputClusterInitCallback(EndpointId endpoint)
{
    ClusterManager().writeListAttribute(endpoint, ZCL_MEDIA_INPUT_CLUSTER_ID, ZCL_MEDIA_INPUT_LIST_ATTRIBUTE_ID,
                                        MediaInputManager().proxyGetInputList());
}

/** @brief Target Navigator Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfTargetNavigatorClusterInitCallback(EndpointId endpoint)
{
    ClusterManager().writeListAttribute(endpoint, ZCL_TARGET_NAVIGATOR_CLUSTER_ID, ZCL_TARGET_NAVIGATOR_LIST_ATTRIBUTE_ID,
                                        TargetNavigatorManager().proxyGetTargetInfoList());
}
