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
 * @brief CLI for the Partner Link Key Exchange plugin.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/plugin/partner-link-key-exchange/partner-link-key-exchange.h"

#if defined(EZSP_HOST)
extern EzspStatus emberAfSetEzspPolicy(EzspPolicyId policyId,
                                       EzspDecisionId decisionId,
                                       const char * policyName,
                                       const char * decisionName);
#endif

extern void emAfPrintStatus(const char * task,
                            EmberStatus status);

#if !defined(EMBER_AF_GENERATE_CLI)
void cbkeAllowPartner(void);
void cbkePartnerCommand(void);

EmberCommandEntry emberAfPluginPartnerLinkKeyExchangeCommands[] = {
  emberCommandEntryAction("partner", cbkePartnerCommand, "vu", ""),
  emberCommandEntryAction("allow-partner", cbkeAllowPartner, "u", ""),
  emberCommandEntryTerminator(),
};
#endif // EMBER_AF_GENERATE_CLI

void cbkeAllowPartner(void)
{
  emAfAllowPartnerLinkKey = (bool)emberUnsignedCommandArgument(0);

#if defined(EZSP_HOST)
  emberAfSetEzspPolicy(EZSP_APP_KEY_REQUEST_POLICY,
                       (emAfAllowPartnerLinkKey
                        ? EZSP_ALLOW_APP_KEY_REQUESTS
                        : EZSP_DENY_APP_KEY_REQUESTS),
                       "App Link Key Request Policy",
                       (emAfAllowPartnerLinkKey
                        ? "Allow"
                        : "Deny"));

#endif
}

void cbkePartnerCommand(void)
{
  EmberNodeId target = (EmberNodeId)emberUnsignedCommandArgument(0);
  uint8_t endpoint     = (uint8_t)emberUnsignedCommandArgument(1);
  EmberStatus status = emberAfInitiatePartnerLinkKeyExchange(target,
                                                             endpoint,
                                                             NULL); // callback
  emberAfCoreDebugExec(emAfPrintStatus("partner link key request", status));
}
