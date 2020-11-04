
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


#ifndef ZCL_CALLBACK_HEADER
#define ZCL_CALLBACK_HEADER

#include "af-structs.h"
#include "af-types.h"

/** @brief Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 * @param clusterId   Ver.: always
 */
void emberAfClusterInitCallback(uint8_t endpoint, EmberAfClusterId clusterId);

// Cluster Init Functions
void emberAfOnOffClusterInitCallback(uint8_t endpoint);


// CLuster Commands Callback

/**
* @brief On/off Cluster Off Command callback
*/

bool emberAfOnOffClusterOffCallback(void);


/**
* @brief On/off Cluster On Command callback
*/

bool emberAfOnOffClusterOnCallback(void);


/**
* @brief On/off Cluster Toggle Command callback
*/

bool emberAfOnOffClusterToggleCallback(void);



#endif //ZCL_CALLBACK_HEADER