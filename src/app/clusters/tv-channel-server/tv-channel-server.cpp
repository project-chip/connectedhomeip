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

/**
 *
 *    Copyright (c) 2021 Silicon Labs
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
 * @brief Routines for the TV Channel plugin, the
 *server implementation of the TV Channel cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/Command.h>
#include <app/util/af.h>

bool emberAfTvChannelClusterChangeChannelCallback(chip::app::Command * commandObj, unsigned char *)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}

bool emberAfTvChannelClusterChangeChannelByNumberCallback(chip::app::Command * commandObj, unsigned short, unsigned short)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
bool emberAfTvChannelClusterSkipChannelCallback(chip::app::Command * commandObj, unsigned short)
{
    EmberAfStatus status = EMBER_ZCL_STATUS_SUCCESS;
    emberAfSendImmediateDefaultResponse(status);
    return true;
}
