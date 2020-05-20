/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Silicon Laboratories Inc. www.silabs.com
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
 *      This file provides a header for the CHIP ZCL Application Layer's
 *      On Off Cluster Server
 *
 */

#ifndef ZCL_ON_OFF_SERVER_H
#define ZCL_ON_OFF_SERVER_H

// Define OnOff plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct
{
    bool hasOnOffValue;
    bool onOffValue;
} ChipZclOnOffSceneSubTableEntry_t;

#endif // ZCL_ON_OFF_SERVER_H
