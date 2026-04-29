/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppConfig.h"
#include "RefrigeratorManager.h"
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <app/clusters/temperature-control-server/CodegenIntegration.h>
#include <app/data-model-provider/AttributeChangeListener.h>
#include <lib/support/logging/CHIPLogging.h>

#ifdef SL_MATTER_ENABLE_AWS
#include "MatterAwsControl.h"
#endif // SL_MATTER_ENABLE_AWS

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogDetail(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    switch (clusterId)
    {
    case app::Clusters::RefrigeratorAlarm::Id:
        RefrigeratorMgr().RefAlarmAttributeChangeHandler(attributePath.mEndpointId, attributeId, value, size);
#ifdef SL_MATTER_ENABLE_AWS
        matterAws::control::AttributeHandler(attributePath.mEndpointId, attributeId);
#endif // SL_MATTER_ENABLE_AWS
        break;
    default:
        break;
    }
}

void MatterCodegenPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & path,
                                              chip::app::DataModel::AttributeChangeType type)
{
    if (path.mClusterId == app::Clusters::Identify::Id)
    {
        ChipLogProgress(Zcl, "Identify cluster ID: " ChipLogFormatMEI " changed", ChipLogValueMEI(path.mAttributeId));
    }
    else if (path.mClusterId == app::Clusters::TemperatureControl::Id)
    {
        auto * cluster = TemperatureControl::FindClusterOnEndpoint(path.mEndpointId);
        if (cluster != nullptr)
        {
            if (path.mAttributeId == app::Clusters::TemperatureControl::Attributes::TemperatureSetpoint::Id)
            {
                int16_t setpoint = cluster->GetTemperatureSetpoint();
                RefrigeratorMgr().TempCtrlAttributeChangeHandler(path.mEndpointId, path.mAttributeId,
                                                                 reinterpret_cast<uint8_t *>(&setpoint), sizeof(setpoint));
            }
#ifdef SL_MATTER_ENABLE_AWS
            matterAws::control::AttributeHandler(path.mEndpointId, path.mAttributeId);
#endif // SL_MATTER_ENABLE_AWS
        }
    }
}

/** @brief Refrigerator Alarm Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfRefrigeratorAlarmClusterInitCallback(EndpointId endpoint) {}

/** @brief Temperature Control Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfTemperatureControlClusterInitCallback(EndpointId endpoint) {}
