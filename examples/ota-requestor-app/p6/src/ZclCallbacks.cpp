/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
#include <lib/support/logging/CHIPLogging.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>

using namespace ::chip;
using namespace ::chip::app::Clusters;

void MatterPostAttributeChangeCallback(const chip::app::ConcreteAttributePath & attributePath, uint8_t mask, uint8_t type,
                                       uint16_t size, uint8_t * value)
{
    EndpointId endpoint     = attributePath.mEndpointId;
    ClusterId clusterId     = attributePath.mClusterId;
    AttributeId attributeId = attributePath.mAttributeId;
    P6_LOG("MatterPostAttributeChangeCallback - Cluster ID: " ChipLogFormatMEI
           ", EndPoint ID: '0x%02x', Attribute ID: " ChipLogFormatMEI,
           ChipLogValueMEI(clusterId), endpoint, ChipLogValueMEI(attributeId));

    switch (clusterId)
    {
    case Identify::Id:
        ChipLogProgress(Zcl, "Identify attribute ID: " ChipLogFormatMEI " Type: %u Value: %" PRIu16 ", length %" PRIu16,
                        ChipLogValueMEI(attributeId), type, *value, size);
        break;
    default:
        P6_LOG("Unhandled cluster ID: " ChipLogFormatMEI, ChipLogValueMEI(clusterId));
        break;
    }
}
