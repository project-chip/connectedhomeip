
  /*
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


#include "callback.h"
#include "cluster-id.h"

// Cluster Init Functions
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId)
{
    switch (clusterId)
    {
     case ZCL_ON_OFF_CLUSTER_ID :
        emberAfOnOffClusterInitCallback(endpoint);
        break;

    default:
        // Unrecognized cluster ID
        break;
    }
}

void __attribute__((weak)) emberAfOnOffClusterInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}

// Cluster Command callback

/**
* @brief On/off Cluster Off Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterOffCallback(void)
{
    return false;
}

/**
* @brief On/off Cluster On Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterOnCallback(void)
{
    return false;
}

/**
* @brief On/off Cluster Toggle Command callback
*/

bool __attribute__((weak))  emberAfOnOffClusterToggleCallback(void)
{
    return false;
}

