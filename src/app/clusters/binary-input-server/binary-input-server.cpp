/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/cluster-id.h>
#include <app/util/af-event.h>
#include <app/util/attribute-storage.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;

#ifndef emberAfBinaryInputBasicClusterPrintln
#define emberAfBinaryInputBasicClusterPrintln(...) ChipLogProgress(Zcl, __VA_ARGS__);
#endif

EmberAfStatus emberAfBinaryInputBasicClusterGetPresentValue(EndpointId endpoint, bool * presentValue)
{
    return emberAfReadServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_PRESENT_VALUE_ATTRIBUTE_ID,
                                      (uint8_t *) presentValue, sizeof(uint8_t));
}

EmberAfStatus emberAfBinaryInputBasicClusterGetOutOfService(EndpointId endpoint, bool * isOutOfService)
{
    return emberAfReadServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_OUT_OF_SERVICE_ATTRIBUTE_ID,
                                      (uint8_t *) isOutOfService, sizeof(uint8_t));
}

EmberAfStatus emberAfBinaryInputBasicClusterSetPresentValueCallback(EndpointId endpoint, bool presentValue)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_PRESENT_VALUE_ATTRIBUTE_ID,
                                       (uint8_t *) &presentValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

EmberAfStatus emberAfBinaryInputBasicClusterSetOutOfServiceCallback(EndpointId endpoint, bool isOutOfService)
{
    return emberAfWriteServerAttribute(endpoint, ZCL_BINARY_INPUT_BASIC_CLUSTER_ID, ZCL_OUT_OF_SERVICE_ATTRIBUTE_ID,
                                       (uint8_t *) &isOutOfService, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}
