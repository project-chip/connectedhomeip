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

#pragma once

#include <app-common/zap-generated/enums.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>

#include <lib/core/CHIPError.h>
#include <vector>

class ClusterManager
{
public:
    EmberAfStatus writeAttribute(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                 uint8_t * buffer, int32_t index = -1)
    {
        EmberAfAttributeSearchRecord record;
        record.endpoint         = endpointId;
        record.clusterId        = clusterId;
        record.clusterMask      = CLUSTER_MASK_SERVER;
        record.manufacturerCode = EMBER_AF_NULL_MANUFACTURER_CODE;
        record.attributeId      = attributeId;

        // When reading or writing a List attribute the 'index' value could have 3 types of values:
        //  -1: Read/Write the whole list content, including the number of elements in the list
        //   0: Read/Write the number of elements in the list, represented as a uint16_t
        //   n: Read/Write the nth element of the list
        //
        // Since the first 2 bytes of the attribute are used to store the number of elements, elements indexing starts
        // at 1. In order to hide this to the rest of the code of this file, the element index is incremented by 1 here.
        // This also allows calling writeAttribute() with no index arg to mean "write the length".
        return emAfReadOrWriteAttribute(&record, NULL, buffer, 0, true, index + 1);
    }

    template <typename T>
    EmberAfStatus writeListAttribute(chip::EndpointId endpointId, chip::ClusterId clusterId, chip::AttributeId attributeId,
                                     const std::vector<T> vector)
    {
        EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
        size_t vectorSize    = vector.size();

        for (size_t i = 0; i < vectorSize; ++i)
        {
            status = writeAttribute(endpointId, clusterId, attributeId, (uint8_t *) &vector[i], static_cast<int>(i));
            VerifyOrReturnError(status == EMBER_ZCL_STATUS_SUCCESS, status);
        }

        return writeAttribute(endpointId, clusterId, attributeId, (uint8_t *) &vectorSize);
    }
};
