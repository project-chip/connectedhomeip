/***************************************************************************//**
 *
 *    <COPYRIGHT>
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
 *
 ******************************************************************************
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE
#include "ota-bootload-core.h"

#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#else
  #define emberAfPluginOtaBootloadCorePrint(...)
  #define emberAfPluginOtaBootloadCorePrintln(...)
  #define emberAfPluginOtaBootloadCoreFlush()
  #define emberAfPluginOtaBootloadCoreDebugExec(x)
  #define emberAfPluginOtaBootloadCorePrintBuffer(buffer, len, withSpace)
  #define emberAfPluginOtaBootloadCorePrintString(buffer)
#endif

// -----------------------------------------------------------------------------
// Types

const EmberZclOtaBootloadFileSpec_t emberZclOtaBootloadFileSpecNull = {
  .manufacturerCode = EMBER_ZCL_MANUFACTURER_CODE_NULL,
  .type             = EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD,
  .version          = EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL,
};

// -----------------------------------------------------------------------------
// API

bool emberZclOtaBootloadIsWildcard(uint8_t *bytes, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    if (bytes[i] != 0xFF) {
      return false;
    }
  }
  return true;
}

bool emberZclOtaBootloadFileSpecsAreEqual(const EmberZclOtaBootloadFileSpec_t *s1,
                                          const EmberZclOtaBootloadFileSpec_t *s2)
{
  return (s1->manufacturerCode == s2->manufacturerCode
          && s1->type == s2->type
          && s1->version == s2->version);
}

size_t emberZclOtaBootloadFetchFileSpec(const uint8_t *data,
                                        EmberZclOtaBootloadFileSpec_t *fileSpec)
{
  const uint8_t *finger = data;
  fileSpec->manufacturerCode = emberFetchLowHighInt16u(finger);
  finger += sizeof(fileSpec->manufacturerCode);
  fileSpec->type = emberFetchLowHighInt16u(finger);
  finger += sizeof(fileSpec->type);
  fileSpec->version = emberFetchLowHighInt32u(finger);
  finger += sizeof(fileSpec->version);
  return finger - data;
}

size_t emberZclOtaBootloadStoreFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                        uint8_t *data)
{
  uint8_t *finger = data;
  emberStoreLowHighInt16u(finger, fileSpec->manufacturerCode);
  finger += sizeof(fileSpec->manufacturerCode);
  emberStoreLowHighInt16u(finger, fileSpec->type);
  finger += sizeof(fileSpec->type);
  emberStoreLowHighInt32u(finger, fileSpec->version);
  finger += sizeof(fileSpec->version);
  return finger - data;
}

void emberZclOtaBootloadInitFileHeaderInfo(EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo)
{
  fileHeaderInfo->version = EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION;
  fileHeaderInfo->spec = emberZclOtaBootloadFileSpecNull;
  fileHeaderInfo->stackVersion = EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP;
  MEMCOPY(fileHeaderInfo->string, "Silicon Labs ZCLIP Image File  ", EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE);
  fileHeaderInfo->securityCredentialVersion = EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP;
  MEMSET(&fileHeaderInfo->destination, 0xFF, sizeof(fileHeaderInfo->destination));
  fileHeaderInfo->hardwareVersionRange.minimum = EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  fileHeaderInfo->hardwareVersionRange.maximum = EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
}

EmberZclOtaBootloadFileStatus_t emberZclOtaBootloadFetchFileHeaderInfo(const uint8_t *data,
                                                                       EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo)
{
  const uint8_t *finger = data;

  // Magic number
  uint32_t magicNumber = emberFetchLowHighInt32u(finger);
  if (magicNumber != EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER) {
    emberAfPluginOtaBootloadCorePrintln("Invalid magic number (0x%X != 0x%X)", magicNumber, EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER);
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_MAGIC_NUMBER;
  }
  finger += EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE;

  // Header version
  fileHeaderInfo->version = emberFetchLowHighInt16u(finger);
  if (fileHeaderInfo->version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION) {
    emberAfPluginOtaBootloadCorePrintln("Invalid file version (%d != %d)", fileHeaderInfo->version, EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION);
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION;
  }
  finger += sizeof(fileHeaderInfo->version);

  // Header length
  fileHeaderInfo->headerSize = emberFetchLowHighInt16u(finger);
  finger += sizeof(fileHeaderInfo->headerSize);

  // Header Field Control
  EmberZclOtaBootloadFileHeaderFieldControl_t fieldControl = emberFetchLowHighInt16u(finger);
  finger += sizeof(fieldControl);

  // File spec
  finger += emberZclOtaBootloadFetchFileSpec(finger, &fileHeaderInfo->spec);

  // Stack version
  fileHeaderInfo->stackVersion = emberFetchLowHighInt16u(finger);
  if (fileHeaderInfo->stackVersion != EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP) {
    emberAfPluginOtaBootloadCorePrintln("Invalid stack version (%d != %d)", fileHeaderInfo->stackVersion, EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION;
  }
  finger += sizeof(fileHeaderInfo->stackVersion);

  // Header string
  MEMMOVE(fileHeaderInfo->string, finger, EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE);
  finger += EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE;

  // Total file size
  fileHeaderInfo->fileSize = emberFetchLowHighInt32u(finger);
  finger += sizeof(fileHeaderInfo->fileSize);

  if (READBITS(fieldControl, EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_SECURITY_CREDENTIAL)) {
    // Security credential
    fileHeaderInfo->securityCredentialVersion = *finger;
    if (fileHeaderInfo->securityCredentialVersion != EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP) {
      emberAfPluginOtaBootloadCorePrintln("Invalid security credential version (%d != %d)", fileHeaderInfo->securityCredentialVersion, EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP);
      return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION;
    }
    finger += sizeof(fileHeaderInfo->securityCredentialVersion);
  } else { // This field is mandatory in Thread
    emberAfPluginOtaBootloadCorePrintln("Missing security credential version");
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION;
  }

  if (READBITS(fieldControl, EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION)) {
    MEMMOVE(fileHeaderInfo->destination.bytes, finger, EMBER_ZCL_UID_SIZE);
    finger += EMBER_ZCL_UID_SIZE;
  } else {
    MEMSET(&fileHeaderInfo->destination, 0xFF, sizeof(fileHeaderInfo->destination));
  }

  if (READBITS(fieldControl, EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION)) {
    fileHeaderInfo->hardwareVersionRange.minimum = emberFetchLowHighInt16u(finger);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.minimum);
    fileHeaderInfo->hardwareVersionRange.maximum = emberFetchLowHighInt16u(finger);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.maximum);
  } else {
    fileHeaderInfo->hardwareVersionRange.minimum = EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
    fileHeaderInfo->hardwareVersionRange.maximum = EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  }

  if (fileHeaderInfo->headerSize != finger - data) {
    emberAfPluginOtaBootloadCorePrintln("Invalid header size (%d != %d)", fileHeaderInfo->headerSize, finger - data);
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_HEADER_SIZE;
  }

  return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID;
}

EmberZclOtaBootloadFileStatus_t emberZclOtaBootloadStoreFileHeaderInfo(uint8_t *headerData,
                                                                       EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo,
                                                                       size_t imageDataSize)
{
  // The OTA file is little-endian.
  uint8_t *finger = headerData;

  // Magic number.
  emberStoreLowHighInt32u(finger, EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER);
  finger += sizeof(uint32_t);

  // Header version.
  if (fileHeaderInfo->version != EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION) {
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION;
  }
  emberStoreLowHighInt16u(finger, EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION);
  finger += sizeof(uint16_t);

  // Header length (filled in later).
  uint8_t *headerSizeLocation = finger;
  finger += sizeof(uint16_t);

  // Header field control (no optional fields).
  uint8_t fieldControl = 0;
  if (!emberZclOtaBootloadIsWildcard(fileHeaderInfo->destination.bytes, sizeof(fileHeaderInfo->destination.bytes))) {
    fieldControl |= EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION;
  }
  if ((fileHeaderInfo->hardwareVersionRange.minimum != EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL)
      || (fileHeaderInfo->hardwareVersionRange.maximum != EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL)) {
    fieldControl |= EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION;
  }
  emberStoreLowHighInt16u(finger, fieldControl);
  finger += sizeof(uint16_t);

  // File spec.
  finger += emberZclOtaBootloadStoreFileSpec(&(fileHeaderInfo->spec), finger);

  // Stack version.
  if (fileHeaderInfo->stackVersion != EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP) {
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION;
  }
  emberStoreLowHighInt16u(finger, EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
  finger += sizeof(uint16_t);

  // Header string.
  MEMMOVE(finger, fileHeaderInfo->string, EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE);
  finger += EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE;

  // Total file size (filled in later).
  uint8_t *fileSizeLocation = finger;
  finger += sizeof(uint32_t);

  // Security credential.
  if (fileHeaderInfo->securityCredentialVersion != EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP) {
    return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION;
  }
  *finger++ = EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP;

  // Add fieldControl blocks
  if (READBITS(fieldControl, EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION)) {
    MEMMOVE(finger, fileHeaderInfo->destination.bytes, EMBER_ZCL_UID_SIZE);
    finger += EMBER_ZCL_UID_SIZE;
  }

  if (READBITS(fieldControl, EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION)) {
    emberStoreLowHighInt16u(finger, fileHeaderInfo->hardwareVersionRange.minimum);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.minimum);
    emberStoreLowHighInt16u(finger, fileHeaderInfo->hardwareVersionRange.maximum);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.maximum);
  }

  size_t headerSize = finger - headerData;
  assert(headerSize <= UINT16_MAX);
  fileHeaderInfo->headerSize = headerSize;
  emberStoreLowHighInt16u(headerSizeLocation, (uint16_t) headerSize);
  assert(headerSize + imageDataSize <= UINT32_MAX);
  fileHeaderInfo->fileSize = (uint32_t) (headerSize + imageDataSize);
  emberStoreLowHighInt32u(fileSizeLocation, fileHeaderInfo->fileSize);

  return EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID;
}
