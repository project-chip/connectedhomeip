
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

#include "callback.h"
#include "cluster-id.h"

// Cluster Init Functions
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId)
{
    switch (clusterId)
    {
    case ZCL_BASIC_CLUSTER_ID :
        emberAfBasicClusterServerInitCallback(endpoint);
        break;
    case ZCL_TEMP_MEASUREMENT_CLUSTER_ID :
        emberAfTemperatureMeasurementClusterServerInitCallback(endpoint);
        break;

    default:
        // Unrecognized cluster ID
        break;
    }
}

void __attribute__((weak)) emberAfBasicClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}
void __attribute__((weak)) emberAfTemperatureMeasurementClusterServerInitCallback(uint8_t endpoint)
{
    // To prevent warning
    (void) endpoint;
}

// Cluster Command callback

