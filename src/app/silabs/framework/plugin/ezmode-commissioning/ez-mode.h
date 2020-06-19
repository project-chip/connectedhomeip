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
 * @brief APIs and defines for the EZ-Mode plugin, which implements EZ-mode
 *        commissioning.
 *******************************************************************************
   ******************************************************************************/

typedef enum {
  /* Allows the creation of bindings from
   * an attribute server to an attribute client. */
  EMBER_AF_EZMODE_COMMISSIONING_SERVER_TO_CLIENT = 0,

  /* Allows the creation of bindings from
   * an attribute client to an attribute server. */
  EMBER_AF_EZMODE_COMMISSIONING_CLIENT_TO_SERVER = 1,
} EmberAfEzModeCommissioningDirection;

/**
 * @brief Starts EZ-Mode Client commissioning.
 *
 * Kicks off the EZ-Mode commissioning process by sending out
 * an identify query command to the given endpoint. Endpoints that
 * return an identify query response are interrogated for the given
 * cluster ids in the given direction (client or server).
 *
 * @param endpoint The endpoint to send the identify query command from
 * @param direction The side of the cluster IDs given either client or server
 * @param clusterIds An array of clusters against which to match.
 *        *NOTE* The API only keeps the pointer to
 *        to the data structure. The data is expected to exist throughout the
 *        EZ-Mode-commissioning calls.
 * @param clusterIdsLength The number of cluster IDs passed for the match.
 */
EmberStatus emberAfEzmodeClientCommission(uint8_t endpoint,
                                          EmberAfEzModeCommissioningDirection direction,
                                          const uint16_t *clusterIds,
                                          uint8_t clusterIdsLength);

/**
 * @brief Begins EZ-Mode server commissioning.
 *
 * Kicks of the server side of EZ-Mode commissioning by putting the
 * device into identify mode.
 *
 * @param endpoint The endpoint on which to begin identifying.
 */
EmberStatus emberAfEzmodeServerCommission(uint8_t endpoint);

/**
 * @brief Begins EZ-Mode server commissioning with a given timeout.
 *
 * Kicks of the server side of EZ-Mode commissioning by putting the
 * device into identify mode for a given time.
 *
 * @param endpoint The endpoint on which to begin identifying
 * @param identifyTimeoutSeconds The number of seconds to identify for before
 *                               stopping identify mode.
 */
EmberStatus emberAfEzmodeServerCommissionWithTimeout(uint8_t endpoint,
                                                     uint16_t identifyTimeoutSeconds);
