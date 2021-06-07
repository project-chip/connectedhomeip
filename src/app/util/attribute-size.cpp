/**
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

/**
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************/
/**
 * @file
 * @brief Contains storage and function for retrieving
 *attribute size.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af.h>

#include <app/common/gen/attribute-type.h>

using namespace chip;

static const uint8_t attributeSizes[] = {
#include <app/common/gen/attribute-size.h>
};

uint8_t emberAfGetDataSize(uint8_t dataType)
{
    for (unsigned i = 0; (i + 1) < sizeof(attributeSizes); i += 2)
    {
        if (attributeSizes[i] == dataType)
        {
            return attributeSizes[i + 1];
        }
    }

    return 0;
}

uint16_t emberAfAttributeValueSize(ClusterId clusterId, AttributeId attrId, EmberAfAttributeType dataType, const uint8_t * buffer)
{
    // If the dataType is a string or long string, refer to the buffer for the
    // string's length prefix; size is string length plus number of prefix bytes.
    // If non-string, determine size from dataType. If dataType is unrecognized,
    // return zero.
    //
    // Note: A non-empty long string has max length 0xFFFE, and adding 2 for its
    // length prefix would roll a uint16_t back to zero. Choosing not to
    // expand return type to uint32_t just to accommodate that one case.
    uint16_t dataSize = 0;
    if (emberAfIsThisDataTypeAStringType(dataType))
    {
        if (buffer != 0)
        {
            if (emberAfIsStringAttributeType(dataType))
            {
                // size is string length plus 1-byte length prefix
                dataSize = static_cast<uint16_t>(static_cast<uint16_t>(emberAfStringLength(buffer)) + 1u);
            }
            else
            {
                // size is long string length plus 2-byte length prefix
                dataSize = static_cast<uint16_t>(emberAfLongStringLength(buffer) + 2u);
            }
        }
    }
    else if (emberAfIsThisDataTypeAListType(dataType))
    {
        if (buffer != 0)
        {
            dataSize = emberAfAttributeValueListSize(clusterId, attrId, buffer);
        }
    }
    else
    {
        dataSize = (uint16_t) emberAfGetDataSize(dataType);
    }

    return dataSize;
}
