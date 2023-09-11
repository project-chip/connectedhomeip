/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "application-basic/ApplicationBasicManager.h"
#include "application-launcher/ApplicationLauncherManager.h"
#include "audio-output/AudioOutputManager.h"
#include "target-navigator/TargetNavigatorManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>

using namespace chip;

namespace {
static ApplicationBasicManager applicationBasicManager;
static ApplicationLauncherManager applicationLauncherManager;
static AudioOutputManager audioOutputManager;
static TargetNavigatorManager targetNavigatorManager;
} // namespace

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
    ChipLogProgress(Zcl, "TV Android App: ApplicationBasic::SetDefaultDelegate");
    chip::app::Clusters::ApplicationBasic::SetDefaultDelegate(endpoint, &applicationBasicManager);
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
    ChipLogProgress(Zcl, "TV Android App: ApplicationLauncher::SetDefaultDelegate");
    chip::app::Clusters::ApplicationLauncher::SetDefaultDelegate(endpoint, &applicationLauncherManager);
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
    ChipLogProgress(Zcl, "TV Android App: AudioOutput::SetDefaultDelegate");
    chip::app::Clusters::AudioOutput::SetDefaultDelegate(endpoint, &audioOutputManager);
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
    ChipLogProgress(Zcl, "TV Android App: TargetNavigator::SetDefaultDelegate");
    chip::app::Clusters::TargetNavigator::SetDefaultDelegate(endpoint, &targetNavigatorManager);
}
