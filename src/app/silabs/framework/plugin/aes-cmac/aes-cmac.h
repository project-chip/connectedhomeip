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
 * @brief Implementation of the Cipher-based Message Authentication Code (CMAC)
 * specified in the IETF memo "The AES-CMAC Algorithm".
 *******************************************************************************
   ******************************************************************************/

// Returns the most significant bit of a bit string.
#define MSB(x) (x[0] >> 7)

#define initToConstZero(x) \
  do { MEMSET(x, 0x00, 16); } while (0)

#define initToConstRb(x) \
  do { MEMSET(x, 0x00, 15); x[15] = 0x87; } while (0)

/** @brief Computes the Cipher-based Message Authentication Code (CMAC)
 * corresponding to the passed message and the passed key.
 *
 * @param key           The key to be used to produce the cipher-based
 * authentication code.
 *
 * @param message       A pointer to the message for which a cipher-based
 * authentication code should be computed.
 *
 * @param messageLength The length in bytes of the message to be authenticated.
 *
 * @out                 A pointer to a 4-byte area memory in which the resulting
 * cipher-based authentication code will be stored.
 */
void emberAfPluginAesMacAuthenticate(const uint8_t *key,
                                     const uint8_t *message,
                                     uint8_t messageLength,
                                     uint8_t *out);
