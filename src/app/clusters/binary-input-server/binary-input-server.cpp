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

#include <support/logging/CHIPLogging.h>

#ifndef emberAfBinaryInputBasicClusterPrintln
#define emberAfBinaryInputBasicClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

EmberAfStatus emberAfBinaryInputBasicClusterGetPresentValue(chip::EndpointId endpoint, bool * presentValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_PRESENT_VALUE_ATTRIBUTE_ID,
                                      (uint8_t *) presentValue, sizeof(uint8_t));
}

EmberAfStatus emberAfBinaryInputBasicClusterGetOutOfService(chip::EndpointId endpoint, bool * isOutOfService)
{
    return emberAfReadServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_OUT_OF_SERVICE_ATTRIBUTE_ID,
                                      (uint8_t *) isOutOfService, sizeof(uint8_t));
}

void emberAfBinaryInputBasicClusterServerInitCallback(chip::EndpointId endpoint)
{
    /**
     * nothing to do here - default values set by attribute storage
     */
    (void) endpoint;
}

EmberAfStatus emberAfBinaryInputBasicClusterSetPresentValueCallback(chip::EndpointId endpoint, bool presentValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_PRESENT_VALUE_ATTRIBUTE_ID,
                                       (uint8_t *) &presentValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfBinaryInputBasicClusterSetOutOfServiceCallback(chip::EndpointId endpoint, bool isOutOfService)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_OUT_OF_SERVICE_ATTRIBUTE_ID,
                                       (uint8_t *) &isOutOfService, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}
