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

/****************************************************************************
 * @file
 * @brief Routines for the Identify Client plugin, the
 *        client implementation of the Identify cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;

bool emberAfIdentifyClusterIdentifyQueryResponseCallback(chip::app::CommandHandler * commandObj, uint16_t timeout)
{
    emberAfIdentifyClusterPrintln("RX: IdentifyQueryResponse 0x%4x", timeout);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
