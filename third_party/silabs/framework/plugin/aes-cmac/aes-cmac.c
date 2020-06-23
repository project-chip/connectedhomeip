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
 * @brief Implementation of the Cipher-based Message Authentication Code (CMAC)
 * specified in the IETF memo "The AES-CMAC Algorithm".
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER

#include "aes-cmac.h"

#if defined(EMBER_SCRIPTED_TEST)
#define HIDDEN
#else
#define HIDDEN static
#endif

// Exclusive-Or operation. For two equal length strings, x and y, x XOR y is
// their bit-wise exclusive-OR.
HIDDEN void xor128(const uint8_t *x, const uint8_t *y, uint8_t *out)
{
  uint8_t i;

  for (i = 0; i < 16; i++) {
    out[i] = x[i] ^ y[i];
  }
}

// Left-shift of the string x by 1 but. The most significant bit disappears, and
// a zero comes into the least significant bit.
HIDDEN void oneBitLeftShift(const uint8_t *x, uint8_t *out)
{
  int8_t i;
  uint8_t overflow = 0x00;

  for (i = 15; i >= 0; i--) {
    out[i] = x[i] << 1;
    out[i] |= overflow;
    overflow = (x[i] & 0x80) ? 0x01 : 0x00;
  }
}

// 10^i padded output of input x. Is the concatenation of x and a single '1'
// followed by the minimum number of '0's, so that the total length is equal
// to 128 bits.
HIDDEN void padding(const uint8_t *x, uint8_t length, uint8_t *out)
{
  uint8_t i;

  for (i = 0; i < 16; i++) {
    if (i < length) {
      out[i] = x[i];
    } else if (i == length) {
      out[i] = 0x80;
    } else {
      out[i] = 0x00;
    }
  }
}

extern void emGetKeyFromCore(uint8_t* key);
extern void emLoadKeyIntoCore(const uint8_t* key);
extern void emStandAloneEncryptBlock(uint8_t *block);

HIDDEN void aesEncrypt(uint8_t *block, const uint8_t *key)
{
  uint8_t temp[16];

  ATOMIC(
    emGetKeyFromCore(temp);
    emLoadKeyIntoCore((uint8_t*)key);
    emStandAloneEncryptBlock(block);
    emLoadKeyIntoCore(temp);
    )
}

HIDDEN void generateSubKey(const uint8_t *key, uint8_t *outKey1, uint8_t *outKey2)
{
  uint8_t L[16];
  uint8_t constRb[16];

  initToConstRb(constRb);

  // Step 1
  initToConstZero(L);
  aesEncrypt(L, key);

  // Step 2
  oneBitLeftShift(L, outKey1); // // K1:= L << 1;

  if (MSB(L)) { // K1:= (L << 1) XOR const_Rb;
    xor128(outKey1, constRb, outKey1);
  }
  // Step 3
  oneBitLeftShift(outKey1, outKey2); // // K2 := K1 << 1;

  if (MSB(outKey1)) { // K2 := (K1 << 1) XOR const_Rb;
    xor128(outKey2, constRb, outKey2);
  }
}

void emberAfPluginAesMacAuthenticate(const uint8_t *key,
                                     const uint8_t *message,
                                     uint8_t messageLength,
                                     uint8_t *out)
{
  uint8_t key1[16];
  uint8_t key2[16];
  uint8_t lastBlock[16];
  uint8_t blockNum;
  bool isLastBlockComplete;
  uint8_t i;

  // Step 1
  generateSubKey(key, key1, key2);

  // Step 2 (we perform ceil(x/y) by doing: (x + y - 1) / y).
  blockNum = (messageLength + 15) / 16;

  // Step 3
  if (blockNum == 0) {
    blockNum = 1;
    isLastBlockComplete = false;
  } else {
    isLastBlockComplete = ((messageLength % 16) == 0);
  }

  // Step 4
  if (isLastBlockComplete) {
    xor128(message + (blockNum - 1) * 16, key1, lastBlock);
  } else {
    padding(message + (blockNum - 1) * 16, messageLength % 16, lastBlock);
    xor128(lastBlock, key2, lastBlock);
  }

  // Step 5
  initToConstZero(out);

  // Step 6
  for (i = 0; i < blockNum - 1; i++) {
    xor128(out, message + i * 16, out);
    aesEncrypt(out, key);
  }

  xor128(out, lastBlock, out);
  aesEncrypt(out, key);
}
