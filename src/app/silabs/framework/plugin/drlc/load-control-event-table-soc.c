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
 * @brief SoC specific code related to the event table.
 *******************************************************************************
   ******************************************************************************/

#include "../../include/af.h"
#include "../../util/common.h"
#include "load-control-event-table.h"

// necessary for supporting ECDSA
#include "stack/include/cbke-crypto-engine.h"

#include "app/framework/security/crypto-state.h"

void emberDsaSignHandler(EmberStatus status, EmberMessageBuffer message)
{
  // Message has been queued by the stack for sending.  Nothing more to do.
  emAfCryptoOperationComplete();

  if (status != EMBER_SUCCESS) {
    emAfNoteSignatureFailure();
  }

  emberAfDemandResponseLoadControlClusterPrintln("emberDsaSignHandler() returned 0x%x",
                                                 status);
}
