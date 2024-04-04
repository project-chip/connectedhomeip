/**
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include "PowerSourceManager.h"
#include "DeviceApp-JNI.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/door-lock-server/door-lock-server.h>
#include <app/reporting/reporting.h>
#include <app/util/config.h>
#include <jni.h>
#include <lib/support/CHIPJNIError.h>
#include <lib/support/JniReferences.h>
#include <lib/support/JniTypeWrappers.h>
#include <lib/support/ZclString.h>
#include <platform/PlatformManager.h>

using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::PowerSource;

static constexpr size_t kPowerSourceManagerTableSize =
    MATTER_DM_POWER_SOURCE_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

namespace {

PowerSourceManager * gPowerSourceManagerTable[kPowerSourceManagerTableSize] = { nullptr };

}

void emberAfPowerSourceClusterInitCallback(EndpointId endpoint)
{
    ChipLogProgress(Zcl, "Device App::PowerSource::PostClusterInit");
    DeviceAppJNIMgr().PostClusterInit(chip::app::Clusters::PowerSource::Id, endpoint);
}

void PowerSourceManager::NewManager(jint endpoint, jobject manager)
{
    ChipLogProgress(Zcl, "Device App: PowerSourceManager::NewManager");
    uint16_t ep = emberAfGetClusterServerEndpointIndex(static_cast<chip::EndpointId>(endpoint), app::Clusters::PowerSource::Id,
                                                       MATTER_DM_POWER_SOURCE_CLUSTER_SERVER_ENDPOINT_COUNT);
    VerifyOrReturn(ep < kPowerSourceManagerTableSize,
                   ChipLogError(Zcl, "Device App::PowerSource::NewManager: endpoint %d not found", endpoint));

    VerifyOrReturn(gPowerSourceManagerTable[ep] == nullptr,
                   ChipLogError(Zcl, "Device App::PowerSource::NewManager: endpoint %d already has a manager", endpoint));
    PowerSourceManager * mgr = new PowerSourceManager();
    CHIP_ERROR err           = mgr->InitializeWithObjects(manager);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Device App::PowerSource::NewManager: failed to initialize manager for endpoint %d", endpoint);
        delete mgr;
    }
    else
    {
        gPowerSourceManagerTable[ep] = mgr;
    }
}

PowerSourceManager * GetPowerSourceManager(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, app::Clusters::PowerSource::Id,
                                                       MATTER_DM_POWER_SOURCE_CLUSTER_SERVER_ENDPOINT_COUNT);
    return ((ep >= kPowerSourceManagerTableSize) ? nullptr : gPowerSourceManagerTable[ep]);
}

jboolean PowerSourceManager::SetBatPercentRemaining(jint endpoint, jint value)
{
    using namespace chip::app::Clusters;
    using namespace chip::DeviceLayer;
    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Success;

    status =
        PowerSource::Attributes::BatPercentRemaining::Set(static_cast<chip::EndpointId>(endpoint), static_cast<uint8_t>(value * 2));

    ChipLogDetail(Zcl, "Device App::PowerSource::SetBatPercentRemaining: endpoint:%d, percent:%d", endpoint, value);
    return status == Protocols::InteractionModel::Status::Success;
}

CHIP_ERROR PowerSourceManager::InitializeWithObjects(jobject managerObject)
{
    JNIEnv * env = JniReferences::GetInstance().GetEnvForCurrentThread();
    VerifyOrReturnLogError(env != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnLogErrorOnFailure(mPowerSourceManagerObject.Init(managerObject));

    jclass PowerSourceManagerClass = env->GetObjectClass(managerObject);
    VerifyOrReturnLogError(PowerSourceManagerClass != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    return CHIP_NO_ERROR;
}
