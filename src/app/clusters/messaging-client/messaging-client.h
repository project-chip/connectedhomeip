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
/***************************************************************************
 * @file
 * @brief APIs and defines for the Messaging Client
 *plugin.
 *******************************************************************************
 ******************************************************************************/

#pragma once

// ----------------------------------------------------------------------------
// Message Control byte
// ----------------------------------------------------------------------------

#define ZCL_MESSAGING_CLUSTER_TRANSMISSION_MASK (BIT(1) | BIT(0))
#define ZCL_MESSAGING_CLUSTER_IMPORTANCE_MASK (BIT(3) | BIT(2))
#define ZCL_MESSAGING_CLUSTER_CONFIRMATION_MASK BIT(7)

#define ZCL_MESSAGING_CLUSTER_START_TIME_NOW 0x00000000UL
#define ZCL_MESSAGING_CLUSTER_END_TIME_NEVER 0xFFFFFFFFUL
#define ZCL_MESSAGING_CLUSTER_DURATION_UNTIL_CHANGED 0xFFFF

/**
 * @brief Clears the message.
 *
 * This function is used to manually inactivate or clear the message.
 *
 * @param endpoint The relevant endpoint.
 *
 **/
void emAfPluginMessagingClientClearMessage(chip::EndpointId endpoint);

/**
 * @brief Prints information about the message.
 *
 * @param endpoint The relevant endpoint.
 *
 **/
void emAfPluginMessagingClientPrintInfo(chip::EndpointId endpoint);

/**
 * @brief Confirms a message.
 *
 * This function is used to confirm a message. The messaging client plugin
 * will send the confirmation command to the endpoint on the node that sent the
 * message.
 *
 * @return ::EMBER_ZCL_STATUS_SUCCESS if the confirmation was sent,
 * ::EMBER_ZCL_STATUS_FAILURE if an error occurred, or
 * ::EMBER_ZCL_STATUS_NOT_FOUND if the message does not exist.
 */
EmberAfStatus emberAfPluginMessagingClientConfirmMessage(chip::EndpointId endpoint);
