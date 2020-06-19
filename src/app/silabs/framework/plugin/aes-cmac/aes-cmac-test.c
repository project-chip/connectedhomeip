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
 * @brief Unit test for AES CMAC code.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER
#include <stdlib.h>
#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "core/parcel.h"
#include "core/scripted-stub.h"

#include "aes-cmac.h"

static uint8_t key[16] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6,
                           0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

extern void oneBitLeftShift(uint8_t *x, uint8_t *out);

static void leftShiftTest(void)
{
  uint8_t i;
  uint8_t x[16];
  uint8_t out[16];
  MEMSET(x, 0x91, 16);

  fprintf(stderr, ".");
  oneBitLeftShift(x, out);

  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    if (i < 15) {
      assert(out[i] == 0x23);
    }
    // least significant byte
    else if (i == 15) {
      assert(out[i] == 0x22);
    }
  }
}

extern void padding(uint8_t *x, uint8_t length, uint8_t *out);

static void paddingTest(void)
{
  uint8_t i;
  uint8_t x[16];
  uint8_t out[16];

  MEMSET(x, 0xFF, 16);
  padding(x, 5, out);
  for (i = 0; i < 16; i++) {
    uint8_t expectedValue;

    if (i < 5) {
      expectedValue = 0xFF;
    } else if (i == 5) {
      expectedValue = 0x80;
    } else {
      expectedValue = 0x00;
    }

    fprintf(stderr, ".");
    assert(out[i] == expectedValue);
  }

  MEMSET(x, 0xFF, 16);
  padding(x, 0, out);
  for (i = 0; i < 16; i++) {
    uint8_t expectedValue;

    if (i == 0) {
      expectedValue = 0x80;
    } else {
      expectedValue = 0x00;
    }

    fprintf(stderr, ".");
    assert(out[i] == expectedValue);
  }

  MEMSET(x, 0xFF, 16);
  padding(x, 16, out);
  for (i = 0; i < 16; i++) {
    uint8_t expectedValue = 0xFF;

    fprintf(stderr, ".");
    assert(out[i] == expectedValue);
  }
}

extern void aesEncrypt(uint8_t *block, uint8_t *key);
extern void generateSubKey(uint8_t *key, uint8_t *outKey1, uint8_t *outKey2);

static void generateSubkeyTest(void)
{
  uint8_t i;
  uint8_t expectedOutKey1[16] = { 0xFB, 0xEE, 0xD6, 0x18, 0x35, 0x71, 0x33, 0x66,
                                  0x7C, 0x85, 0xE0, 0x8F, 0x72, 0x36, 0xA8, 0xDE };
  uint8_t expectedOutKey2[16] = { 0xF7, 0xDD, 0xAC, 0x30, 0x6A, 0xE2, 0x66, 0xCC,
                                  0xF9, 0x0B, 0xC1, 0x1E, 0xE4, 0x6D, 0x51, 0x3B };
  uint8_t outKey1[16];
  uint8_t outKey2[16];
  uint8_t zeroCypher[16];
  uint8_t expectedZeroCypher[16] = { 0x7D, 0xF7, 0x6B, 0x0C, 0x1A, 0xB8, 0x99, 0xB3,
                                     0x3E, 0x42, 0xF0, 0x47, 0xB9, 0x1B, 0x54, 0x6F };
  MEMSET(zeroCypher, 0x00, 16);

  aesEncrypt(zeroCypher, key);

  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    assert(zeroCypher[i] == expectedZeroCypher[i]);
  }

  generateSubKey(key, outKey1, outKey2);

  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    assert(outKey1[i] == expectedOutKey1[i]);
    fprintf(stderr, ".");
    assert(outKey2[i] == expectedOutKey2[i]);
  }
}

static void aesCmacTest(void)
{
  uint8_t i;
  uint8_t out[16];
  uint8_t *example1 = NULL;
  uint8_t example1Length = 0;
  uint8_t expectedExample1Out[16] = { 0xBB, 0x1D, 0x69, 0x29, 0xE9, 0x59, 0x37, 0x28,
                                      0x7F, 0xA3, 0x7D, 0x12, 0x9B, 0x75, 0x67, 0x46 };
  uint8_t example2[16] = { 0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
                           0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A };
  uint8_t example2Length = 16;
  uint8_t expectedExample2Out[16] = { 0x07, 0x0A, 0x16, 0xB4, 0x6B, 0x4D, 0x41, 0x44,
                                      0xF7, 0x9B, 0xDD, 0x9D, 0xD0, 0x4A, 0x28, 0x7C };
  uint8_t example3[40] = { 0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
                           0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
                           0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C,
                           0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51,
                           0x30, 0xC8, 0x1C, 0x46, 0xA3, 0x5C, 0xE4, 0x11 };
  uint8_t example3Length = 40;
  uint8_t expectedExample3Out[16] = { 0xDF, 0xA6, 0x67, 0x47, 0xDE, 0x9A, 0xE6, 0x30,
                                      0x30, 0xCA, 0x32, 0x61, 0x14, 0x97, 0xC8, 0x27 };
  uint8_t example4[64] = { 0x6B, 0xC1, 0xBE, 0xE2, 0x2E, 0x40, 0x9F, 0x96,
                           0xE9, 0x3D, 0x7E, 0x11, 0x73, 0x93, 0x17, 0x2A,
                           0xAE, 0x2D, 0x8A, 0x57, 0x1E, 0x03, 0xAC, 0x9C,
                           0x9E, 0xB7, 0x6F, 0xAC, 0x45, 0xAF, 0x8E, 0x51,
                           0x30, 0xC8, 0x1C, 0x46, 0xA3, 0x5C, 0xE4, 0x11,
                           0xE5, 0xFB, 0xC1, 0x19, 0x1A, 0x0A, 0x52, 0xEF,
                           0xF6, 0x9F, 0x24, 0x45, 0xDF, 0x4F, 0x9B, 0x17,
                           0xAD, 0x2B, 0x41, 0x7B, 0xE6, 0x6C, 0x37, 0x10 };
  uint8_t example4Length = 64;
  uint8_t expectedExample4Out[16] = { 0x51, 0xF0, 0xBE, 0xBF, 0x7E, 0x3B, 0x9D, 0x92,
                                      0xFC, 0x49, 0x74, 0x17, 0x79, 0x36, 0x3C, 0xFE };

  emberAfPluginAesMacAuthenticate(key, example1, example1Length, out);
  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    assert(out[i] == expectedExample1Out[i]);
  }

  emberAfPluginAesMacAuthenticate(key, example2, example2Length, out);
  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    assert(out[i] == expectedExample2Out[i]);
  }

  emberAfPluginAesMacAuthenticate(key, example3, example3Length, out);
  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    assert(out[i] == expectedExample3Out[i]);
  }

  emberAfPluginAesMacAuthenticate(key, example4, example4Length, out);
  for (i = 0; i < 16; i++) {
    fprintf(stderr, ".");
    assert(out[i] == expectedExample4Out[i]);
  }
}

static Test tests[] =
{
  { "left-shift", leftShiftTest },
  { "padding", paddingTest },
  { "generate-subkey", generateSubkeyTest },
  { "aes-cmac", aesCmacTest },
  { NULL, NULL }                        // terminator
};

int main(int argc, char **argv)
{
  Thunk test = parseTestArgument(argc, argv, tests);

  //----------------
  // Run the actual test.
  test();

  fprintf(stderr, " done]\n");

  return 0;
}

bool scriptDebug;

Thunk parseTestArgument(int argc, char **argv, Test *tests)
{
  int i;
  int errors = 0;
  bool disabled = false;
  char *testName = NULL;
  char *me = *argv;   // Save program name.
  argv++; argc--;   // Skip program name.

  for (; argc > 0; argc--, argv++) {
    if (argv[0][0] == '-') {
      switch (argv[0][1]) {
        case '-': {
          if (strcmp(argv[0], "--debug") == 0) {
            scriptDebug = true;
          } else if (strcmp(argv[0], "--disable") == 0) {
            disabled = true;
          } else if (strcmp(argv[0], "--help") == 0) {
            errors++;
            break;
          } else {
            fprintf(stderr, "Invalid argument: %s\n", *argv);
            errors++;
          }
          break;
        }
        default:
          fprintf(stderr, "Invalid argument: %s\n", *argv);
          errors++;
      }
    } else if (testName == NULL) {
      testName = argv[0];
    } else {
      fprintf(stderr, "Too man test names: %s and %s\n", testName, *argv);
      errors++;
      break;
    }
  }

  if (testName == NULL) {
    errors += 1;
  }

  if (errors == 0) {
    for (i = 0;; i++) {
      if (tests[i].name == NULL) {
        fprintf(stderr, "Unrecognized test: '%s'\n", testName);
        errors += 1;
        break;
      } else if (strcmp(testName, tests[i].name) == 0) {
        if (disabled) {
          fprintf(stderr, "[***Test %s is disabled***]\n", testName);
          exit(0);
        } else {
          fprintf(stderr, "[Testing %s ", testName);
          return tests[i].test;
        }
      }
    }
  }

  if (errors != 0) {
    fprintf(stderr, "Usage: %s [--debug] [--disable] test\n", me);
    for (i = 0; tests[i].name != NULL; i++) {
      fprintf(stderr, "%s %s\n",
              i == 0 ? "Tests:" : "      ",
              tests[i].name);
    }
    exit(1);
  }
  return NULL;
}
