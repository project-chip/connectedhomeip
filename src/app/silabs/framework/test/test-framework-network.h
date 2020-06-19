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
 * @brief Network stubs for AFV2 tests.
 *******************************************************************************
   ******************************************************************************/

extern EmberEUI64 localEui64;
extern EmberNodeId testFrameworkNodeId;
extern EmberNetworkStatus testFrameworkNetworkState;

void setLocalEui64(EmberEUI64 eui64);

void setNetworkParameters(EmberNodeType *nodeType,
                          EmberNetworkParameters *parameters);

#define EM_AF_MAX_SUPPORTED_NETWORKS_FOR_UNIT_TESTS 2
#define EM_AF_DEFAULT_NETWORK_INDEX_FOR_UNIT_TESTS (EM_AF_MAX_SUPPORTED_NETWORKS_FOR_UNIT_TESTS - 1)
