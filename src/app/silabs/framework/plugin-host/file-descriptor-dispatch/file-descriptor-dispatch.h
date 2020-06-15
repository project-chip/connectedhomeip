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
 * @brief Definitions for the File Descriptor Dispatch plugin.
 *******************************************************************************
   ******************************************************************************/

typedef enum {
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_NONE   = 0x00,
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_READ   = 0x01,
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_WRITE  = 0x02,
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_EXCEPT = 0x03,
  EMBER_AF_FILE_DESCRIPTOR_OPERATION_MAX
} EmberAfFileDescriptorOperation;

typedef void (*EmberAfFileDescriptorReadyCallback)(void* data, EmberAfFileDescriptorOperation operation);

typedef struct {
  EmberAfFileDescriptorReadyCallback callback;
  void* dataPassedToCallback;
  EmberAfFileDescriptorOperation operation;
  int fileDescriptor;
} EmberAfFileDescriptorDispatchStruct;

EmberStatus emberAfPluginFileDescriptorDispatchAdd(EmberAfFileDescriptorDispatchStruct* dispatchStruct);
EmberStatus emberAfPluginFileDescriptorDispatchWaitForEvents(uint32_t timeoutMs);
bool emberAfPluginFileDescriptorDispatchRemove(int fileDescriptor);
