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
 *    @file
 *      This file provides unit test code for the CHIP ZCL Application Layer's
 *      On Off Cluster Server
 *
 */

#include "utest.h" /* TODO pass all that stuff in -D */
#include "zcl-data-model.h"

int main()
{
    ChipZclAttributeMetadata * metadata;
    int count;

    chipZclEndpointInit();

    count = chipZclEndpointCount();
    if (count != 3)
    {
        printf("Error: expecting one endpoint, got %d\n", count);
        return 1;
    }

    // This attribute doesn't exist
    metadata = chipZclLocateAttributeMetadata(1, 2, 3, 4, 5);
    if (metadata != NULL)
    {
        printf("Error: expecting NULL\n");
        return 1;
    }

    // This one does, on endpoint 1
    metadata = chipZclLocateAttributeMetadata(1, 0, 0, CLUSTER_MASK_SERVER, CHIP_ZCL_NULL_MANUFACTURER_CODE);
    if (metadata == NULL)
    {
        printf("Error: expecting non-NULL\n");
        return 1;
    }

    // Another existing attribute, on endpoint 242
    metadata = chipZclLocateAttributeMetadata(242, 0x0021, 0x0011, CLUSTER_MASK_CLIENT, CHIP_ZCL_NULL_MANUFACTURER_CODE);
    if (metadata == NULL)
    {
        printf("Error: expecting non-NULL\n");
        return 1;
    }

    printf("Success \n");
    return 0;
}
