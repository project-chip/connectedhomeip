/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include "binary-input-server.h"

#include <app/util/af.h>

#include "gen/att-storage.h"
#include "gen/attribute-id.h"
#include "gen/attribute-type.h"
#include "gen/cluster-id.h"
#include "gen/command-id.h"

static inline EmberAfStatus emberAfBinaryInputBasicClusterGetPresentValueCallback(chip::EndpointId endpoint, bool* presentValue)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_BINARY_INPUT_BASIC_CLUSTER_ID,
                                ZCL_PRESENT_VALUE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)presentValue,
                                sizeof(uint8_t),
                                NULL);
}

static inline EmberAfStatus emberAfBinaryInputBasicClusterGetOutOfServiceCallback(chip::EndpointId endpoint, bool* isOutOfService)
{
    return emberAfReadAttribute(endpoint,
                                ZCL_BINARY_INPUT_BASIC_CLUSTER_ID,
                                ZCL_OUT_OF_SERVICE_ATTRIBUTE_ID,
                                CLUSTER_MASK_SERVER,
                                (uint8_t*)isOutOfService,
                                sizeof(uint8_t),
                                NULL);
}

void emberAfBinaryInputBasicServerClusterInitCallback(chip::EndpointId endpoint)
{
    bool presentValue = false;
    EmberAfStatus status = emberAfBinaryInputBasicClusterGetPresentValueCallback(endpoint, &presentValue);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfBinaryInputBasicClusterSetPresentValueCallback(endpoint, false);
    }

    bool isOutOfService = false;
    status = emberAfBinaryInputBasicClusterGetOutOfServiceCallback(endpoint, &isOutOfService);

    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        (void)emberAfBinaryInputBasicClusterSetOutOfServiceCallback(endpoint, false);
    }
}


EmberAfStatus emberAfBinaryInputBasicClusterSetPresentValueCallback(chip::EndpointId endpoint, bool presentValue)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_BINARY_INPUT_BASIC_CLUSTER_ID,
                                                 ZCL_PRESENT_VALUE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&presentValue,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfBinaryInputBasicClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}

EmberAfStatus emberAfBinaryInputBasicClusterSetOutOfServiceCallback(chip::EndpointId endpoint, bool isOutOfService)
{
    EmberAfStatus status = emberAfWriteAttribute(endpoint,
                                                 ZCL_BINARY_INPUT_BASIC_CLUSTER_ID,
                                                 ZCL_OUT_OF_SERVICE_ATTRIBUTE_ID,
                                                 CLUSTER_MASK_SERVER,
                                                 (uint8_t *)&isOutOfService,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (EMBER_ZCL_STATUS_SUCCESS != status)
    {
        emberAfBinaryInputBasicClusterPrintln("ERR: writing present value %x", status);
    }

    return status;
}