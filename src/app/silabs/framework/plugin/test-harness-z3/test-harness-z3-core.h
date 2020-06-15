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
 * @brief ZigBee 3.0 core test harness functionality
 *******************************************************************************
   ******************************************************************************/

// -----------------------------------------------------------------------------
// Constants

#define TEST_HARNESS_Z3_PRINT_NAME "TestHarnessZ3"

// -----------------------------------------------------------------------------
// Globals

enum {
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_NOT_ADDRESS_ASSIGNABLE     = 0x00,
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZR_ADDRESS_ASSIGNABLE         = 0x01,
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZED_NOT_ADDRESS_ASSIGNABLE    = 0x02,
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_ZED_ADDRESS_ASSIGNABLE        = 0x03,
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_SLEEPY_ZED_NOT_ADDRESS_ASSIGNABLE = 0x04,
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_SLEEPY_ZED_ADDRESS_ASSIGNABLE = 0x05,
};
typedef uint8_t EmAfPluginTestHarnessZ3DeviceMode;
extern EmAfPluginTestHarnessZ3DeviceMode emAfPluginTestHarnessZ3DeviceMode;
extern uint16_t emAfPluginTestHarnessZ3TouchlinkProfileId;

#define EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_MAX \
  EM_AF_PLUGIN_TEST_HARNESS_Z3_DEVICE_MODE_SLEEPY_ZED_ADDRESS_ASSIGNABLE
#define COMMAND_DATA_SIZE 128
#define EXTRA_COMMAND_BYTES 5

// -----------------------------------------------------------------------------
// Utility API

uint32_t emAfPluginTestHarnessZ3GetSignificantBit(uint8_t commandIndex);
