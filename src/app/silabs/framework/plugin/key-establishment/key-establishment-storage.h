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
 * @brief  * API for the storage of public temporary partner data.
 * - Partner Certificate
 * - Partner Ephemeral Public Key
 * - A single SMAC
 *******************************************************************************
   ******************************************************************************/

// If isCertificate is false, data is a public key.
bool storePublicPartnerData(bool isCertificate,
                            uint8_t* data);
bool storePublicPartnerData163k1(bool isCertificate,
                                 uint8_t* data);
bool storePublicPartnerData283k1(bool isCertificate,
                                 uint8_t* data);
bool retrieveAndClearPublicPartnerData163k1(EmberCertificateData* partnerCertificate,
                                            EmberPublicKeyData* partnerEphemeralPublicKey);

bool retrieveAndClearPublicPartnerData(EmberCertificate283k1Data* partnerCertificate,
                                       EmberPublicKey283k1Data* partnerEphemeralPublicKey);
bool retrieveAndClearPublicPartnerData283k1(EmberCertificate283k1Data* partnerCertificate,
                                            EmberPublicKey283k1Data* partnerEphemeralPublicKey);
bool storeSmac(EmberSmacData* smac);
bool getSmacPointer(EmberSmacData** smacPtr);

void clearAllTemporaryPublicData(void);
