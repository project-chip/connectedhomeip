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
/***************************************************************************//**
 * @file
 * @brief Host specific routines for performing service discovery.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-host.h"

//------------------------------------------------------------------------------

EmberStatus emAfSendMatchDescriptor(EmberNodeId target,
                                    EmberAfProfileId profileId,
                                    EmberAfClusterId clusterId,
                                    bool serverCluster)
{
  uint8_t inCount = 0;
  uint8_t outCount = 0;
  EmberAfClusterId inCluster, outCluster;

  if (serverCluster) {
    inCount = 1;
    inCluster = clusterId;
    outCluster = 0x0000;
  } else {
    outCount = 1;
    outCluster = clusterId;
    inCluster = 0x0000;
  }

  return ezspMatchDescriptorsRequest(target,
                                     profileId,
                                     inCount,
                                     outCount,
                                     &inCluster,
                                     &outCluster,
                                     EMBER_AF_DEFAULT_APS_OPTIONS);
}
