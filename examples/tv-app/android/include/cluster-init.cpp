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
#include "target-navigator/TargetNavigatorManager.h"

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>

using namespace chip;

namespace {
template <typename Manager, typename AttrTypeInfo, CHIP_ERROR (Manager::*Getter)(app::AttributeValueEncoder &)>
class TvAttrAccess : public app::AttributeAccessInterface
{
public:
    TvAttrAccess() : app::AttributeAccessInterface(Optional<EndpointId>::Missing(), AttrTypeInfo::GetClusterId()) {}

    CHIP_ERROR Read(const app::ConcreteReadAttributePath & aPath, app::AttributeValueEncoder & aEncoder) override
    {
        if (aPath.mAttributeId == AttrTypeInfo::GetAttributeId())
        {
            return (Manager().*Getter)(aEncoder);
        }

        return CHIP_NO_ERROR;
    }
};

} // anonymous namespace

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
        chip::app::Clusters::ApplicationBasic::Application application = aManager.getApplicationForEndpoint(endpoint);
        aManager.store(endpoint, &application);
    }
    else
    {
        ChipLogError(Zcl, "Failed to store application for endpoint: %d. Error:%s", endpoint, chip::ErrorStr(err));
    }
}

namespace {

TvAttrAccess<ApplicationLauncherManager, app::Clusters::ApplicationLauncher::Attributes::ApplicationLauncherList::TypeInfo,
             &ApplicationLauncherManager::proxyGetApplicationList>
    gApplicationLauncherAttrAccess;

} // anonymous namespace

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
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gApplicationLauncherAttrAccess);
        attrAccessRegistered = true;
    }
}

namespace {

TvAttrAccess<AudioOutputManager, app::Clusters::AudioOutput::Attributes::AudioOutputList::TypeInfo,
             &AudioOutputManager::proxyGetListOfAudioOutputInfo>
    gAudioOutputAttrAccess;

} // anonymous namespace

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
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gAudioOutputAttrAccess);
        attrAccessRegistered = true;
    }
}

namespace {

TvAttrAccess<TargetNavigatorManager, app::Clusters::TargetNavigator::Attributes::TargetNavigatorList::TypeInfo,
             &TargetNavigatorManager::proxyGetTargetInfoList>
    gTargetNavigatorAttrAccess;

} // anonymous namespace

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
    static bool attrAccessRegistered = false;
    if (!attrAccessRegistered)
    {
        registerAttributeAccessOverride(&gTargetNavigatorAttrAccess);
        attrAccessRegistered = true;
    }
}
