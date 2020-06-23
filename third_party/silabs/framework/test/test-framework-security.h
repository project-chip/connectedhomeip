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
 * @brief Security test code and stubs for unit tests of the Afv2 code.
 *******************************************************************************
   ******************************************************************************/

extern EmberKeyStruct testFrameworkCurrentNwkKey;
extern EmberKeyStruct testFrameworkNextNwkKey;
extern EmberKeyStruct testFrameworkTrustCenterLinkKey;

extern EmberKeyStruct testFrameworkKeyTable[];
extern bool testFrameworkGenerateKeyFails;
extern EmberKeyData testFrameworkRandomlyGeneratedKey;

extern EmberKeyData testFrameworkZeroKey;

extern EmberNodeId testFrameworkKeyTableNodeId[];

#define TEST_FRAMEWORK_KEY_TABLE_SIZE 4

extern bool emberKeySwitchExpected;
extern bool emberKeySwitchReceived;
