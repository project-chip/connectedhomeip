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
#include <lib/support/logging/CHIPLogging.h>

#ifdef DIC_ENABLE
#include "dic_control.h"
#endif // DIC_ENABLE

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    switch (clusterId)
    {
    case app::Clusters::Identify::Id:
        ChipLogProgress(Zcl, "Identify cluster ID: " ChipLogFormatMEI " Type: %u Value: %u, length %u",
                        ChipLogValueMEI(attributePath.mAttributeId), type, *value, size);
        break;
    case app::Clusters::RefrigeratorAlarm::Id:
        RefrigeratorMgr().RefAlaramAttributeChangeHandler(attributePath.mEndpointId, attributeId, value, size);
#ifdef DIC_ENABLE
        dic::control::AttributeHandler(attributePath.mEndpointId, attributeId);
#endif // DIC_ENABLE
        break;
    case app::Clusters::TemperatureControl::Id:
        RefrigeratorMgr().TempCtrlAttributeChangeHandler(attributePath.mEndpointId, attributeId, value, size);
#ifdef DIC_ENABLE
        dic::control::AttributeHandler(attributePath.mEndpointId, attributeId);
#endif // DIC_ENABLE
        break;
    default:
        break;
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
