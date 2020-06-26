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
 * @brief Implemented routines for storing future prepayment modes.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_PREPAYMENT_MODES_TABLE_H
#define SILABS_PREPAYMENT_MODES_TABLE_H

/**
 * @brief Initializes the prepayment mode table.
 *
 **/
void emInitPrepaymentModesTable(void);

/**
 * @brief Schedules a prepayment mode at some current or future time.
 * @param endpoint The endpoint of the device that supports the prepayment server.
 * @param providerId A unique identifier for the commodity supplier to whom this command relates.
 * @param issuerEventId A unique identifier that identifies this prepayment mode event.
 * Newer events should have a larger issuerEventId than older events.
 * @param dateTimeUtc The UTC time that indicates when the payment mode change should be applied.
 * A value of 0x00000000 indicates the command should be executed immediately. A value of 0xFFFFFFFF
 * indicates an existing change payment mode command with the same provider ID and issuer event ID should be canceled.
 * @param paymentControlConfig A bitmap that indicates which actions should be taken when switching the payment mode.
 *
 **/
void emberAfPrepaymentSchedulePrepaymentMode(uint8_t endpoint, uint32_t providerId, uint32_t issuerEventId, uint32_t dateTimeUtc, uint16_t paymentControlConfig);

/**
 * @brief Determines the number of remaining seconds until the next prepayment mode event is scheduled to occur.
 * @param timeNowUtc The current UTC time.
 * @return Returns the number of remaining seconds until the next prepayment mode event.
 *
 **/
uint32_t emberAfPrepaymentServerSecondsUntilPaymentModeEvent(uint32_t timeNowUtc);

/**
 * @brief Sets the current payment mode on the prepayment server.
 * @param endpoint The endpoint of the prepayment server.
 *
 **/
void emberAfPrepaymentServerSetPaymentMode(uint8_t endpoint);

//void updatePaymentControlConfiguration( uint8_t endpoint, uint16_t paymentControlConfig );

#endif  // #ifndef _PREPAYMENT_MODES_TABLE_H_
