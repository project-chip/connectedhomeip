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
 * @brief This file records the state of crypto operations so that the application
 * can defer processing until after crypto operations have completed.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_CRYPTO_STATE_H
#define SILABS_CRYPTO_STATE_H

// For the host applications, if ECC operations are underway then
// the NCP will be completely consumed doing the processing for
// SECONDS.  Therefore the application should not expect it to be
// very responsive.  Normal operations (cluster and app. ticks) will
// not be fired during that period.

#ifndef CRYPTO_OPERATION_TIMEOUT_MS
#define CRYPTO_OPERATION_TIMEOUT_MS MILLISECOND_TICKS_PER_SECOND * 5
#endif //CRYPTO_OPERATION_TIMEOUT_MS

enum {
  EM_AF_NO_CRYPTO_OPERATION,
  EM_AF_CRYPTO_OPERATION_IN_PROGRESS,
};
typedef uint8_t EmAfCryptoStatus;

#define EM_AF_CRYPTO_STATUS_TEXT \
  {                              \
    "No operation",              \
    "Operation in progress",     \
    NULL                         \
  }

EmAfCryptoStatus emAfGetCryptoStatus(void);
void emAfSetCryptoStatus(EmAfCryptoStatus newStatus);

#define emAfSetCryptoOperationInProgress() \
  (emAfSetCryptoStatus(EM_AF_CRYPTO_OPERATION_IN_PROGRESS))

#define emAfIsCryptoOperationInProgress() \
  (EM_AF_CRYPTO_OPERATION_IN_PROGRESS == emAfGetCryptoStatus())

#define emAfCryptoOperationComplete() \
  (emAfSetCryptoStatus(EM_AF_NO_CRYPTO_OPERATION))

#endif // SILABS_CRYPTO_STATE_H
