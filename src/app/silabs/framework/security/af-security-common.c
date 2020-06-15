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
 * @brief Security code common to both the Trust Center and the normal node.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/framework/security/af-security.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/framework/cli/security-cli.h"

//------------------------------------------------------------------------------
bool emberAfClearLinkKeyTableUponFormingOrJoining = true;

const EmberAfSecurityProfileData emAfSecurityProfileData[] = {
  #include "security-profile-data.h"
};

static const EmberKeyData unSetKey = DUMMY_KEY;

// This routine sets the keys from values previously set on the CLI.
// If none are set via the CLI, then the default keys for the security profile
// are used.
static void getKeyFromCli(EmberKeyData* returnData, bool linkKey)
{
  uint8_t* keyPtr = (linkKey
                     ? emberKeyContents(&cliPreconfiguredLinkKey)
                     : emberKeyContents(&cliNetworkKey));
  if (0 != MEMCOMPARE(keyPtr,
                      emberKeyContents(&unSetKey),
                      EMBER_ENCRYPTION_KEY_SIZE)) {
    MEMMOVE(emberKeyContents(returnData), keyPtr, EMBER_ENCRYPTION_KEY_SIZE);
  }
}

void getLinkKeyFromCli(EmberKeyData* returnData)
{
  getKeyFromCli(returnData, true);
}

void getNetworkKeyFromCli(EmberKeyData* returnData)
{
  getKeyFromCli(returnData, false);
}

void emAfClearLinkKeyTable(void)
{
#if EMBER_KEY_TABLE_SIZE
  if (emberAfClearLinkKeyTableUponFormingOrJoining) {
    emberClearKeyTable();
  }
  emberAfClearLinkKeyTableUponFormingOrJoining = true;
#endif
}

const EmberAfSecurityProfileData *emAfGetCurrentSecurityProfileData(void)
{
  uint8_t i;
  for (i = 0; i < COUNTOF(emAfSecurityProfileData); i++) {
    if (emAfProIsCurrentNetwork()
        && (emAfCurrentZigbeeProNetwork->securityProfile
            == emAfSecurityProfileData[i].securityProfile)) {
      return &emAfSecurityProfileData[i];
    }
  }
  return NULL;
}

bool emberAfIsCurrentSecurityProfileSmartEnergy(void)
{
#ifdef EMBER_AF_HAS_SECURITY_PROFILE_SE
  return (emAfProIsCurrentNetwork()
          && ((emAfCurrentZigbeeProNetwork->securityProfile
               == EMBER_AF_SECURITY_PROFILE_SE_TEST)
              || (emAfCurrentZigbeeProNetwork->securityProfile
                  == EMBER_AF_SECURITY_PROFILE_SE_FULL)));
#else
  return false;
#endif
}
