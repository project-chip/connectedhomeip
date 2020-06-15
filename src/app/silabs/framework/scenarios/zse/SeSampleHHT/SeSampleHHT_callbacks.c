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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"
#include <stdlib.h>
#include "app/framework/plugin/gas-proxy-function/gas-proxy-function.h"
#include "app/framework/plugin/gbz-message-controller/gbz-message-controller.h"

/** @brief Fragment Transmission Failed
 *
 * This function is called by the Interpan plugin when a fragmented transmission
 * has failed.
 *
 * @param interpanFragmentationStatus The status describing why transmission
 * failed  Ver.: always
 * @param fragmentNum The fragment number that encountered the failure  Ver.:
 * always
 */
void emberAfPluginInterpanFragmentTransmissionFailedCallback(int8u interpanFragmentationStatus,
                                                             int8u fragmentNum)
{
}

/** @brief Message Received Over Fragments
 *
 * This function is called by the Interpan plugin when a fully reconstructed
 * message has been received over inter-PAN fragments, or IPMFs.
 *
 * @param header The inter-PAN header  Ver.: always
 * @param msgLen The message payload length  Ver.: always
 * @param message The message payload  Ver.: always
 */
void emberAfPluginInterpanMessageReceivedOverFragmentsCallback(const EmberAfInterpanHeader *header,
                                                               int8u msgLen,
                                                               int8u *message)
{
}
