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
 * @brief SOC specific routines for performing service discovery.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/zigbee-framework/zigbee-device-library.h"

//------------------------------------------------------------------------------

EmberStatus emAfSendMatchDescriptor(EmberNodeId target,
                                    EmberAfProfileId profileId,
                                    EmberAfClusterId clusterId,
                                    bool serverCluster)
{
  EmberMessageBuffer clusterList = emberAllocateLinkedBuffers(1);
  EmberMessageBuffer inClusters = EMBER_NULL_MESSAGE_BUFFER;
  EmberMessageBuffer outClusters = EMBER_NULL_MESSAGE_BUFFER;
  EmberStatus status = EMBER_NO_BUFFERS;

  if (clusterList != EMBER_NULL_MESSAGE_BUFFER) {
    emberSetMessageBufferLength(clusterList, 2);
    emberSetLinkedBuffersLowHighInt16u(clusterList, 0, clusterId);

    if (serverCluster) {
      inClusters = clusterList;
    } else {
      outClusters = clusterList;
    }

    status = emberMatchDescriptorsRequest(target,
                                          profileId,
                                          inClusters,
                                          outClusters,
                                          EMBER_AF_DEFAULT_APS_OPTIONS);
    emberReleaseMessageBuffer(clusterList);
  }
  return status;
}
