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
#include "attribute-db.h"

// Callback implementations

ChipZclStatus_t chipZclExternalAttributeReadCallback(uint8_t endpoint,
                                                    ChipZclClusterId clusterId,
                                                    ChipZclAttributeMetadata *attributeMetadata,
                                                    uint16_t manufacturerCode,
                                                    uint8_t *buffer,
                                                    uint16_t maxReadLength) {
                                                        return CHIP_ZCL_STATUS_SUCCESS;
                                                    }

ChipZclStatus_t chipZclExternalAttributeWriteCallback(uint8_t endpoint,
                                                    ChipZclClusterId clusterId,
                                                    ChipZclAttributeMetadata *attributeMetadata,
                                                    uint16_t manufacturerCode,
                                                    uint8_t *buffer){
                                                        return CHIP_ZCL_STATUS_SUCCESS;
                                                    }
ChipZclStatus_t chipZclPreAttributeChangeCallback(uint8_t endpoint,
                                                ChipZclClusterId clusterId,
                                                ChipZclAttributeId attributeId,
                                                uint8_t mask,
                                                uint16_t manufacturerCode,
                                                uint8_t type,
                                                uint8_t size,
                                                uint8_t* value){
                                                        return CHIP_ZCL_STATUS_SUCCESS;
                                                    }

void chipZclPostAttributeChangeCallback(uint8_t endpoint,
                                        ChipZclClusterId clusterId,
                                        ChipZclAttributeId attributeId,
                                        uint8_t mask,
                                        uint16_t manufacturerCode,
                                        uint8_t type,
                                        uint8_t size,
                                        uint8_t* value){
                                                    }

bool chipZclAttributeReadAccessCallback(uint8_t endpoint,
                                        ChipZclClusterId clusterId,
                                        uint16_t manufacturerCode,
                                        uint16_t attributeId) 
                                        {
                                            return true;
                                        }

bool chipZclAttributeWriteAccessCallback(uint8_t endpoint,
                                         ChipZclClusterId clusterId,
                                         uint16_t manufacturerCode,
                                         uint16_t attributeId) {
                                                return true;
                                         }

int main()
{
    printf("Success \n");
    return 0;
}
