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
 * @brief A sample of Secure EZSP protocol.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "app/util/serial/command-interpreter2.h"
#include "app/util/ezsp/secure-ezsp-host.h"

#ifdef EMBER_AF_LEGACY_CLI
  #error The Secure EZSP plugin is not compatible with the legacy CLI.
#endif

#if defined(EMBER_AF_GENERATE_CLI) || defined(EMBER_AF_API_COMMAND_INTERPRETER2)

// plugin secure-ezsp set_security_key [securityType:4]
void emberAfPluginSecureEzspSetSecurityKeyCommand(void)
{
  EzspStatus status;
  EmberKeyData securityKey;
  SecureEzspSecurityType securityType;
  securityType = (SecureEzspSecurityType)emberUnsignedCommandArgument(0);

  status = emberAfGenerateRandomKey(&securityKey);
  if (status != EMBER_SUCCESS) {
    emberAfCorePrint("Random key generation failed, status:0x%x", status);
    assert(0);
  }

  status = emberSecureEzspSetSecurityKey(&securityKey, securityType);

  if (status == EZSP_SUCCESS) {
    emberAfCorePrint("Security Key set { ");
    emberAfCorePrintBuffer(emberKeyContents(&securityKey), EMBER_ENCRYPTION_KEY_SIZE, TRUE);
    emberAfCorePrintln("}");
  } else {
    emberAfCorePrintln("Failed to set Security Key, status:0x%x", status);
  }
}

// plugin secure-ezsp set_security_parameters [securityLevel:4]
void emberAfPluginSecureEzspSetSecurityParametersCommand(void)
{
  EzspStatus status;
  SecureEzspSecurityLevel securityLevel;
  static SecureEzspRandomNumber randomNumber;

  securityLevel = (SecureEzspSecurityLevel)emberUnsignedCommandArgument(0);

  status = emberAfGenerateRandomData(SecureEzspRandomNumberContents(&randomNumber),
                                     SECURE_EZSP_RANDOM_NUMBER_SIZE);

  if (status != EMBER_SUCCESS) {
    emberAfCorePrint("Random data generation failed, status:0x%x", status);
    assert(0);
  }

  status = emberSecureEzspSetSecurityParameters(securityLevel, &randomNumber);

  if (status == EZSP_SUCCESS) {
    emberAfCorePrint("Security Parameters set { ");
    emberAfCorePrintBuffer(randomNumber.contents, SECURE_EZSP_RANDOM_NUMBER_SIZE, TRUE);
    emberAfCorePrintln("}");
  } else {
    emberAfCorePrintln("Failed to set Security Parameters, status:0x%x", status);
  }
}

// plugin secure-ezsp reset_to_factory_defaults
void emberAfPluginSecureEzspResetToFactoryDefaultsCommand(void)
{
  EzspStatus status = emberSecureEzspResetToFactoryDefaults();

  if (status == EZSP_SUCCESS) {
    emberAfCorePrintln("Reset Security");
  } else {
    emberAfCorePrintln("Failed to Reset Security, status:0x%x", status);
  }
}

//------------------------------------------------------------------------------
// Callbacks

void emberSecureEzspInitCallback(EzspStatus ncpSecurityKeyStatus,
                                 EzspStatus hostSecurityKeyStatus,
                                 SecureEzspSecurityType ncpSecurityType)
{
  EzspStatus status = EMBER_INVALID_CALL;
  SecureEzspSecurityLevel securityLevel = 5;
  bool newKeySet = false;
  static SecureEzspRandomNumber randomNumber;

  if (ncpSecurityKeyStatus == EZSP_ERROR_SECURITY_KEY_ALREADY_SET
      && hostSecurityKeyStatus == EZSP_ERROR_SECURITY_KEY_NOT_SET) {
    // User should decide what happens in this case
    assert(0);
  }

  if (ncpSecurityKeyStatus == EZSP_ERROR_SECURITY_KEY_NOT_SET) {
    #ifdef EMBER_AF_PLUGIN_SECURE_EZSP_SETUP_SECURITY_ON_INIT
    EmberKeyData securityKey;
    status = emberAfGenerateRandomKey(&securityKey);
    if (status != EMBER_SUCCESS) {
      emberAfCorePrint("Random key generation failed, status:0x%x", status);
      assert(0);
    }
      #ifdef EMBER_AF_PLUGIN_SECURE_EZSP_PERMANENT_SECURITY_KEY
    status = emberSecureEzspSetSecurityKey(&securityKey,
                                           SECURE_EZSP_SECURITY_TYPE_PERMANENT);
      #else
    status = emberSecureEzspSetSecurityKey(&securityKey,
                                           SECURE_EZSP_SECURITY_TYPE_TEMPORARY);
      #endif
    if (status == EZSP_SUCCESS) {
      emberAfCorePrint("Security Key set { ");
      emberAfCorePrintBuffer(emberKeyContents(&securityKey), EMBER_ENCRYPTION_KEY_SIZE, TRUE);
      emberAfCorePrintln("}");
      newKeySet = true;
    } else {
      emberAfCorePrintln("Failed to set Security Key, status:0x%x", status);
      return;
    }
    #else
    return;
    #endif
  }

  if (ncpSecurityKeyStatus == EZSP_ERROR_SECURITY_KEY_ALREADY_SET
      || newKeySet) {
    emberAfCorePrintln("Security Key Already Set, type:0x%x", ncpSecurityType);
    status = emberAfGenerateRandomData(SecureEzspRandomNumberContents(&randomNumber),
                                       SECURE_EZSP_RANDOM_NUMBER_SIZE);

    if (status != EMBER_SUCCESS) {
      emberAfCorePrint("Random data generation failed, status:0x%x", status);
      assert(0);
    }
    status = emberSecureEzspSetSecurityParameters(securityLevel,
                                                  &randomNumber);
  }

  if (status == EZSP_SUCCESS) {
    emberAfCorePrint("Security Parameters set { ");
    emberAfCorePrintBuffer(randomNumber.contents, SECURE_EZSP_RANDOM_NUMBER_SIZE, TRUE);
    emberAfCorePrintln("}");
  } else {
    emberAfCorePrintln("Failed to set Security Parameters, status:0x%x", status);
  }
}

#endif
