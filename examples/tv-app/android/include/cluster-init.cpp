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
#include "audio-output/AudioOutputManager.h"
#include "target-navigator/TargetNavigatorManager.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/attribute-storage.h>

using namespace chip;

namespace {
static ApplicationBasicManager applicationBasicManager;
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
