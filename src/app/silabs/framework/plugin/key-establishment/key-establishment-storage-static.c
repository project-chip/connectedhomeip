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
 * @brief This file implements the routines for storing temporary data that
 * is needed for key establishment.  This is data is completely
 * public and is sent over-the-air and thus not required to be
 * closely protected.
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the zcl-util
#include "../../util/common.h"

#include "key-establishment-storage.h"

#ifndef EZSP_HOST
  #include "stack/include/cbke-crypto-engine.h"
#endif

//------------------------------------------------------------------------------
// Globals

static EmberCertificateData partnerCert;
static EmberPublicKeyData partnerPublicKey;
static EmberCertificate283k1Data partnerCert283k1;
static EmberPublicKey283k1Data partnerPublicKey283k1;
static EmberSmacData storedSmac;

//------------------------------------------------------------------------------

bool storePublicPartnerData163k1(bool isCertificate,
                                 uint8_t* data)
{
  uint8_t* ptr = (isCertificate
                  ? emberCertificateContents(&partnerCert)
                  : emberPublicKeyContents(&partnerPublicKey));
  uint8_t size = (isCertificate
                  ? EMBER_CERTIFICATE_SIZE
                  : EMBER_PUBLIC_KEY_SIZE);
  MEMMOVE(ptr, data, size);
  return true;
}

bool retrieveAndClearPublicPartnerData163k1(EmberCertificateData* partnerCertificate,
                                            EmberPublicKeyData* partnerEphemeralPublicKey)
{
  if ( partnerCertificate != NULL ) {
    MEMMOVE(partnerCertificate,
            &partnerCert,
            EMBER_CERTIFICATE_SIZE);
  }
  if ( partnerEphemeralPublicKey != NULL ) {
    MEMMOVE(partnerEphemeralPublicKey,
            &partnerPublicKey,
            EMBER_PUBLIC_KEY_SIZE);
  }
  MEMSET(&partnerCert, 0, EMBER_CERTIFICATE_SIZE);
  MEMSET(&partnerPublicKey, 0, EMBER_PUBLIC_KEY_SIZE);
  return true;
}

bool storePublicPartnerData283k1(bool isCertificate,
                                 uint8_t* data)
{
  uint8_t* ptr = (isCertificate
                  ? emberCertificate283k1Contents(&partnerCert283k1)
                  : emberPublicKey283k1Contents(&partnerPublicKey283k1));
  uint8_t size = (isCertificate
                  ? EMBER_CERTIFICATE_283K1_SIZE
                  : EMBER_PUBLIC_KEY_283K1_SIZE);
  MEMMOVE(ptr, data, size);
  return true;
}

bool retrieveAndClearPublicPartnerData283k1(EmberCertificate283k1Data* partnerCertificate,
                                            EmberPublicKey283k1Data* partnerEphemeralPublicKey)
{
  if ( partnerCertificate != NULL ) {
    MEMMOVE(partnerCertificate,
            &partnerCert283k1,
            EMBER_CERTIFICATE_283K1_SIZE);
  }
  if ( partnerEphemeralPublicKey != NULL ) {
    MEMMOVE(partnerEphemeralPublicKey,
            &partnerPublicKey283k1,
            EMBER_PUBLIC_KEY_283K1_SIZE);
  }
  MEMSET(&partnerCert283k1, 0, EMBER_CERTIFICATE_283K1_SIZE);
  MEMSET(&partnerPublicKey283k1, 0, EMBER_PUBLIC_KEY_283K1_SIZE);
  return true;
}

bool storeSmac(EmberSmacData* smac)
{
  MEMMOVE(&storedSmac, smac, EMBER_SMAC_SIZE);
  return true;
}

bool getSmacPointer(EmberSmacData** smacPtr)
{
  *smacPtr = &storedSmac;
  return true;
}

void clearAllTemporaryPublicData(void)
{
  MEMSET(&storedSmac, 0, EMBER_SMAC_SIZE);
  retrieveAndClearPublicPartnerData(NULL, NULL);
  retrieveAndClearPublicPartnerData283k1(NULL, NULL);
}
