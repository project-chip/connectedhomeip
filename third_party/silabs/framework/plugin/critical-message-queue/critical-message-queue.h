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
 * @brief APIs and defines for the Critical Message Queue plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_CRITICAL_MESSAGE_QUEUE_H
#include "af-types.h"

// To send a message using the critical message queue functionality,
// pass this callback to any of the emberAfSendUnicastWithCallback functions
extern const EmberAfMessageSentFunction emberAfPluginCriticalMessageQueueEnqueueCallback;

// The retry callback is exposed for af-main-common.c; it is not to be used by
// a typical application
extern const EmberAfMessageSentFunction emAfPluginCriticalMessageQueueRetryCallback;

void emberAfPluginCriticalMessageQueuePrintQueue(void);
void emberAfPluginCriticalMessageQueuePrintConfig(void);
void emberAfPluginCriticalMessageQueueInit(void);

#endif
