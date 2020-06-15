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

#include "app/framework/util/common.h"
#include "crypto-state.h"

//------------------------------------------------------------------------------

static EmAfCryptoStatus cryptoStatus = EM_AF_NO_CRYPTO_OPERATION;
#if defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_SECURITY)
static const char * cryptoStatusText[] = EM_AF_CRYPTO_STATUS_TEXT;
#endif //defined(EMBER_AF_PRINT_ENABLE) && defined(EMBER_AF_PRINT_SECURITY)

// If we are on a host micro, there is the possibility that a crypto
// operation ends on the NCP and we fail to hear about it, in this case
// we need a timeout to safeguard against this flag getting locked
#if defined(EZSP_HOST)
EmberEventControl emAfCryptoOperationTimeoutEventControl;
void emAfCryptoOperationTimeoutEvent(void)
{
  if (cryptoStatus == EM_AF_CRYPTO_OPERATION_IN_PROGRESS) {
    emAfSetCryptoStatus(EM_AF_NO_CRYPTO_OPERATION);
  }
}
EmberEventData emAfCryptoEvents[] = {
  { &emAfCryptoOperationTimeoutEventControl, emAfCryptoOperationTimeoutEvent },
  { NULL, NULL }
};
#endif //EZSP_HOST

//------------------------------------------------------------------------------

EmAfCryptoStatus emAfGetCryptoStatus(void)
{
#if defined(EZSP_HOST)
  // Everytime someone asks for the crypto state, we check our
  // crypto timeout
  emberRunEvents(emAfCryptoEvents);
#endif //EZSP_HOST

  return cryptoStatus;
}

void emAfSetCryptoStatus(EmAfCryptoStatus newStatus)
{
  cryptoStatus = newStatus;
  emberAfSecurityPrintln("Crypto state: %p", cryptoStatusText[cryptoStatus]);

#if defined(EZSP_HOST)
  // If crypto is starting, we set the timeout, otherwise we cancel it.
  if (cryptoStatus == EM_AF_CRYPTO_OPERATION_IN_PROGRESS) {
    emberEventControlSetDelayMS(emAfCryptoOperationTimeoutEventControl,
                                CRYPTO_OPERATION_TIMEOUT_MS);
  } else {
    emberEventControlSetInactive(emAfCryptoOperationTimeoutEventControl);
  }
#endif //EZSP_HOST
}
