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
 * @brief Header used for testing the event mechanism
 *******************************************************************************
   ******************************************************************************/

#define EMBER_AF_GENERATED_EVENT_CODE extern void testEventControlServerCallback(uint8_t endpoint); \
  extern void testEventControlClientCallback(uint8_t endpoint);                                     \
  EmberEventControl testEventControl1;                                                              \
  EmberEventControl testEventControl2;                                                              \
  void testEventControlFunction1(void) { testEventControlServerCallback(0x01); }                    \
  void testEventControlFunction2(void) { testEventControlClientCallback(0x02); }

#define EMBER_AF_GENERATED_EVENTS                    \
  { &testEventControl1, testEventControlFunction1 }, \
  { &testEventControl2, testEventControlFunction2 },

#define EMBER_AF_GENERATED_EVENT_STRINGS \
  "Test 1",                              \
  "Test 2",

#define EMBER_AF_EVENT_CONTEXT_LENGTH 2

#define EMBER_AF_GENERATED_EVENT_CONTEXT { 0x01, 0xabba, false, EMBER_AF_LONG_POLL, EMBER_AF_OK_TO_SLEEP, &testEventControl1 }, \
  { 0x02, 0xabba, true, EMBER_AF_LONG_POLL, EMBER_AF_OK_TO_SLEEP, &testEventControl2 }
