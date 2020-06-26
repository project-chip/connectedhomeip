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
 * @brief The Simple Storage Module driver interface.  In other words, primitives
 * for reading / writing and storing data about the OTA file that is stored,
 * or is in the process of being downloaded and stored.
 *******************************************************************************
   ******************************************************************************/

bool emberAfCustomStorageInitCallback(void);

bool emberAfCustomStorageReadCallback(uint32_t offset,
                                      uint32_t length,
                                      uint8_t* returnData);

bool emberAfCustomStorageWriteCallback(const uint8_t* dataToWrite,
                                       uint32_t offset,
                                       uint32_t length);

// TODO: put this gating back in once we have that mechanism in place for
// the generated CLI
//#if defined(EMBER_TEST)
void emAfOtaLoadFileCommand(void);
//#endif
