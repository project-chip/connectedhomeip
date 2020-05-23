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
#include "ChipZclUnitTests.h"
#include "chip-zcl.h"
#include <stdio.h>
#include <stdlib.h>

void chipZclPostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    printf("Attribute change callback: ep=%d clusterId=%d attributeId=%d\n", endpoint, clusterId, attributeId);
}

int main(int argc, char ** argv)
{
    printf("%s: === start ===\n", argv[0]);
    testClusterServerBasic();
    testClusterServerIdentify();
    testClusterServerLevelControl();
    testClusterServerOnOff();
    testCodecSimple();
    testCoreDataModel();
    testCoreMessageDispatch();
    printf("%s: === end ===\n", argv[0]);
    return 0;
}
