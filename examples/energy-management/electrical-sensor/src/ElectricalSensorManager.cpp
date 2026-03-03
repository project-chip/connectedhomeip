/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <ElectricalSensorManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace Clusters {

using namespace ElectricalPowerMeasurement;
using namespace ElectricalEnergyMeasurement;

CHIP_ERROR ElectricalSensorManager::Init(EndpointId endpointId, const EpmConfig & epmConfig, const EemConfig & eemConfig,
                                         const PtConfig & ptConfig)
{
    // --- Initialize EPM ---
    ReturnErrorOnFailure(
        ElectricalPowerMeasurementInit(endpointId, mEPMDelegate, mEPMInstance, epmConfig.features, epmConfig.optionalAttributes));

    // --- Initialize EEM ---
    ElectricalEnergyMeasurementCluster::Config clusterConfig{
        .endpointId         = endpointId,
        .featureFlags       = eemConfig.features,
        .optionalAttributes = eemConfig.optionalAttributes,
        .accuracyStruct     = eemConfig.accuracyStruct,
        .delegate           = mEEMDelegate,
        .timerDelegate      = mTimerDelegate,
    };

    mEEMCluster = std::make_unique<RegisteredServerCluster<ElectricalEnergyMeasurementCluster>>(clusterConfig);
    VerifyOrReturnError(mEEMCluster != nullptr, CHIP_ERROR_NO_MEMORY);

    ReturnErrorOnFailure(CodegenDataModelProvider::Instance().Registry().Register(mEEMCluster->Registration()));

    // --- Initialize Power Topology ---
    ReturnErrorOnFailure(PowerTopology::PowerTopologyInit(endpointId, mPTDelegate, mPTInstance, ptConfig.features));

    return CHIP_NO_ERROR;
}

void ElectricalSensorManager::Shutdown()
{
    // Shutdown in reverse init order
    TEMPORARY_RETURN_IGNORED PowerTopology::PowerTopologyShutdown(mPTInstance, mPTDelegate);

    if (mEEMCluster)
    {
        CHIP_ERROR err = CodegenDataModelProvider::Instance().Registry().Unregister(&mEEMCluster->Cluster());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "EEM: Failed to unregister cluster: %" CHIP_ERROR_FORMAT, err.Format());
        }
        mEEMCluster.reset();
    }

    TEMPORARY_RETURN_IGNORED ElectricalPowerMeasurementShutdown(mEPMInstance, mEPMDelegate);
}

CHIP_ERROR ElectricalSensorManager::SendPowerReading(int64_t aActivePower_mW, int64_t aVoltage_mV, int64_t aActiveCurrent_mA)
{
    VerifyOrReturnError(mEPMDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    TEMPORARY_RETURN_IGNORED mEPMDelegate->SetActivePower(DataModel::MakeNullable(aActivePower_mW));
    TEMPORARY_RETURN_IGNORED mEPMDelegate->SetVoltage(DataModel::MakeNullable(aVoltage_mV));
    TEMPORARY_RETURN_IGNORED mEPMDelegate->SetActiveCurrent(DataModel::MakeNullable(aActiveCurrent_mA));
    return CHIP_NO_ERROR;
}

ElectricalEnergyMeasurementCluster * ElectricalSensorManager::GetEEMCluster()
{
    if (mEEMCluster)
    {
        return &mEEMCluster->Cluster();
    }
    return nullptr;
}

} // namespace Clusters
} // namespace app
} // namespace chip
