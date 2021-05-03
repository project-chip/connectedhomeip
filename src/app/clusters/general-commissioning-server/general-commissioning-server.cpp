/**
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
/****************************************************************************
 * @file
 * @brief Routines for the Content Launch plugin, the
 *server implementation of the Content Launch cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/Command.h>
#include <app/util/af.h>
#include <lib/support/Span.h>

using namespace chip;

bool emberAfGeneralCommissioningClusterArmFailSafeCallback(chip::app::Command * commandObj, uint16_t expiryLengthSeconds,
                                                           uint64_t breadcrumb, uint32_t timeoutMs)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfGeneralCommissioningClusterCommissioningCompleteCallback(chip::app::Command * commandObj)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfGeneralCommissioningClusterSetRegulatoryConfigCallback(chip::app::Command * commandObj, uint8_t location,
                                                                   uint8_t * countryCode, uint64_t breadcrumb, uint32_t timeoutMs)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_FAILURE;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
