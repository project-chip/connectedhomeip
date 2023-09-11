/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/**
 * @file
 *   This file implements the handler for data model messages.
 */

#include "AppConfig.h"

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t type, uint16_t size,
                                       uint8_t * value)
{
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    ChipLogProgress(Zcl, "Cluster callback: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));

    if (clusterId == HepaFilterMonitoring::Id && attributeId == HepaFilterMonitoring::Attributes::DegradationDirection::Id)
    {
        static_assert(sizeof(HepaFilterMonitoring::DegradationDirectionEnum) == 1, "Wrong size");
        HepaFilterMonitoring::DegradationDirectionEnum HepaFilterState =
            *(reinterpret_cast<HepaFilterMonitoring::DegradationDirectionEnum *>(value));
        ChipLogProgress(Zcl, "Hepa Filter Monitoring cluster: " ChipLogFormatMEI " state %d", ChipLogValueMEI(clusterId),
                        to_underlying(HepaFilterState));
    }
    else if (clusterId == ActivatedCarbonFilterMonitoring::Id &&
             attributeId == ActivatedCarbonFilterMonitoring::Attributes::DegradationDirection::Id)
    {
        static_assert(sizeof(ActivatedCarbonFilterMonitoring::DegradationDirectionEnum) == 1, "Wrong size");
        ActivatedCarbonFilterMonitoring::DegradationDirectionEnum CarbonFilterState =
            *(reinterpret_cast<ActivatedCarbonFilterMonitoring::DegradationDirectionEnum *>(value));
        ChipLogProgress(Zcl, "Activated Carbon Filter cluster: " ChipLogFormatMEI " state %d", ChipLogValueMEI(clusterId),
                        to_underlying(CarbonFilterState));
    }
}

/** @brief Hepa Filter Monitoring Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfHepaFilterMonitoringClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

/** @brief Activated Carbon Filter Monitoring Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 */
void emberAfActivatedCarbonFilterMonitoringClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}
