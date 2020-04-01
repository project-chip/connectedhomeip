/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE
#include "ota-bootload-core.h"

#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#else
  #define chipAfPluginOtaBootloadCorePrint(...)
  #define chipAfPluginOtaBootloadCorePrintln(...)
  #define chipAfPluginOtaBootloadCoreFlush()
  #define chipAfPluginOtaBootloadCoreDebugExec(x)
  #define chipAfPluginOtaBootloadCorePrintBuffer(buffer, len, withSpace)
  #define chipAfPluginOtaBootloadCorePrintString(buffer)
#endif

// -----------------------------------------------------------------------------
// Types

const ChipZclOtaBootloadFileSpec_t chipZclOtaBootloadFileSpecNull = {
  .manufacturerCode = CHIP_ZCL_MANUFACTURER_CODE_NULL,
  .type             = CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD,
  .version          = CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL,
};

// -----------------------------------------------------------------------------
// API

bool chipZclOtaBootloadIsWildcard(uint8_t *bytes, size_t size)
{
  for (size_t i = 0; i < size; i++) {
    if (bytes[i] != 0xFF) {
      return false;
    }
  }
  return true;
}

bool chipZclOtaBootloadFileSpecsAreEqual(const ChipZclOtaBootloadFileSpec_t *s1,
                                          const ChipZclOtaBootloadFileSpec_t *s2)
{
  return (s1->manufacturerCode == s2->manufacturerCode
          && s1->type == s2->type
          && s1->version == s2->version);
}

size_t chipZclOtaBootloadFetchFileSpec(const uint8_t *data,
                                        ChipZclOtaBootloadFileSpec_t *fileSpec)
{
  const uint8_t *finger = data;
  fileSpec->manufacturerCode = chipFetchLowHighInt16u(finger);
  finger += sizeof(fileSpec->manufacturerCode);
  fileSpec->type = chipFetchLowHighInt16u(finger);
  finger += sizeof(fileSpec->type);
  fileSpec->version = chipFetchLowHighInt32u(finger);
  finger += sizeof(fileSpec->version);
  return finger - data;
}

size_t chipZclOtaBootloadStoreFileSpec(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                        uint8_t *data)
{
  uint8_t *finger = data;
  chipStoreLowHighInt16u(finger, fileSpec->manufacturerCode);
  finger += sizeof(fileSpec->manufacturerCode);
  chipStoreLowHighInt16u(finger, fileSpec->type);
  finger += sizeof(fileSpec->type);
  chipStoreLowHighInt32u(finger, fileSpec->version);
  finger += sizeof(fileSpec->version);
  return finger - data;
}

void chipZclOtaBootloadInitFileHeaderInfo(ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo)
{
  fileHeaderInfo->version = CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION;
  fileHeaderInfo->spec = chipZclOtaBootloadFileSpecNull;
  fileHeaderInfo->stackVersion = CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP;
  MEMCOPY(fileHeaderInfo->string, "Silicon Labs ZCLIP Image File  ", CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE);
  fileHeaderInfo->securityCredentialVersion = CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP;
  MEMSET(&fileHeaderInfo->destination, 0xFF, sizeof(fileHeaderInfo->destination));
  fileHeaderInfo->hardwareVersionRange.minimum = CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  fileHeaderInfo->hardwareVersionRange.maximum = CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
}

ChipZclOtaBootloadFileStatus_t chipZclOtaBootloadFetchFileHeaderInfo(const uint8_t *data,
                                                                       ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo)
{
  const uint8_t *finger = data;

  // Magic number
  uint32_t magicNumber = chipFetchLowHighInt32u(finger);
  if (magicNumber != CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER) {
    chipAfPluginOtaBootloadCorePrintln("Invalid magic number (0x%X != 0x%X)", magicNumber, CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER);
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_MAGIC_NUMBER;
  }
  finger += CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE;

  // Header version
  fileHeaderInfo->version = chipFetchLowHighInt16u(finger);
  if (fileHeaderInfo->version != CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION) {
    chipAfPluginOtaBootloadCorePrintln("Invalid file version (%d != %d)", fileHeaderInfo->version, CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION);
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION;
  }
  finger += sizeof(fileHeaderInfo->version);

  // Header length
  fileHeaderInfo->headerSize = chipFetchLowHighInt16u(finger);
  finger += sizeof(fileHeaderInfo->headerSize);

  // Header Field Control
  ChipZclOtaBootloadFileHeaderFieldControl_t fieldControl = chipFetchLowHighInt16u(finger);
  finger += sizeof(fieldControl);

  // File spec
  finger += chipZclOtaBootloadFetchFileSpec(finger, &fileHeaderInfo->spec);

  // Stack version
  fileHeaderInfo->stackVersion = chipFetchLowHighInt16u(finger);
  if (fileHeaderInfo->stackVersion != CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP) {
    chipAfPluginOtaBootloadCorePrintln("Invalid stack version (%d != %d)", fileHeaderInfo->stackVersion, CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION;
  }
  finger += sizeof(fileHeaderInfo->stackVersion);

  // Header string
  MEMMOVE(fileHeaderInfo->string, finger, CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE);
  finger += CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE;

  // Total file size
  fileHeaderInfo->fileSize = chipFetchLowHighInt32u(finger);
  finger += sizeof(fileHeaderInfo->fileSize);

  if (READBITS(fieldControl, CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_SECURITY_CREDENTIAL)) {
    // Security credential
    fileHeaderInfo->securityCredentialVersion = *finger;
    if (fileHeaderInfo->securityCredentialVersion != CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP) {
      chipAfPluginOtaBootloadCorePrintln("Invalid security credential version (%d != %d)", fileHeaderInfo->securityCredentialVersion, CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP);
      return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION;
    }
    finger += sizeof(fileHeaderInfo->securityCredentialVersion);
  } else { // This field is mandatory in Thread
    chipAfPluginOtaBootloadCorePrintln("Missing security credential version");
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION;
  }

  if (READBITS(fieldControl, CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION)) {
    MEMMOVE(fileHeaderInfo->destination.bytes, finger, CHIP_ZCL_UID_SIZE);
    finger += CHIP_ZCL_UID_SIZE;
  } else {
    MEMSET(&fileHeaderInfo->destination, 0xFF, sizeof(fileHeaderInfo->destination));
  }

  if (READBITS(fieldControl, CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION)) {
    fileHeaderInfo->hardwareVersionRange.minimum = chipFetchLowHighInt16u(finger);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.minimum);
    fileHeaderInfo->hardwareVersionRange.maximum = chipFetchLowHighInt16u(finger);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.maximum);
  } else {
    fileHeaderInfo->hardwareVersionRange.minimum = CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
    fileHeaderInfo->hardwareVersionRange.maximum = CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL;
  }

  if (fileHeaderInfo->headerSize != finger - data) {
    chipAfPluginOtaBootloadCorePrintln("Invalid header size (%d != %d)", fileHeaderInfo->headerSize, finger - data);
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_HEADER_SIZE;
  }

  return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID;
}

ChipZclOtaBootloadFileStatus_t chipZclOtaBootloadStoreFileHeaderInfo(uint8_t *headerData,
                                                                       ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo,
                                                                       size_t imageDataSize)
{
  // The OTA file is little-endian.
  uint8_t *finger = headerData;

  // Magic number.
  chipStoreLowHighInt32u(finger, CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER);
  finger += sizeof(uint32_t);

  // Header version.
  if (fileHeaderInfo->version != CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION) {
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION;
  }
  chipStoreLowHighInt16u(finger, CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION);
  finger += sizeof(uint16_t);

  // Header length (filled in later).
  uint8_t *headerSizeLocation = finger;
  finger += sizeof(uint16_t);

  // Header field control (no optional fields).
  uint8_t fieldControl = 0;
  if (!chipZclOtaBootloadIsWildcard(fileHeaderInfo->destination.bytes, sizeof(fileHeaderInfo->destination.bytes))) {
    fieldControl |= CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION;
  }
  if ((fileHeaderInfo->hardwareVersionRange.minimum != CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL)
      || (fileHeaderInfo->hardwareVersionRange.maximum != CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL)) {
    fieldControl |= CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION;
  }
  chipStoreLowHighInt16u(finger, fieldControl);
  finger += sizeof(uint16_t);

  // File spec.
  finger += chipZclOtaBootloadStoreFileSpec(&(fileHeaderInfo->spec), finger);

  // Stack version.
  if (fileHeaderInfo->stackVersion != CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP) {
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION;
  }
  chipStoreLowHighInt16u(finger, CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP);
  finger += sizeof(uint16_t);

  // Header string.
  MEMMOVE(finger, fileHeaderInfo->string, CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE);
  finger += CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE;

  // Total file size (filled in later).
  uint8_t *fileSizeLocation = finger;
  finger += sizeof(uint32_t);

  // Security credential.
  if (fileHeaderInfo->securityCredentialVersion != CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP) {
    return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION;
  }
  *finger++ = CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP;

  // Add fieldControl blocks
  if (READBITS(fieldControl, CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION)) {
    MEMMOVE(finger, fileHeaderInfo->destination.bytes, CHIP_ZCL_UID_SIZE);
    finger += CHIP_ZCL_UID_SIZE;
  }

  if (READBITS(fieldControl, CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION)) {
    chipStoreLowHighInt16u(finger, fileHeaderInfo->hardwareVersionRange.minimum);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.minimum);
    chipStoreLowHighInt16u(finger, fileHeaderInfo->hardwareVersionRange.maximum);
    finger += sizeof(fileHeaderInfo->hardwareVersionRange.maximum);
  }

  size_t headerSize = finger - headerData;
  assert(headerSize <= UINT16_MAX);
  fileHeaderInfo->headerSize = headerSize;
  chipStoreLowHighInt16u(headerSizeLocation, (uint16_t) headerSize);
  assert(headerSize + imageDataSize <= UINT32_MAX);
  fileHeaderInfo->fileSize = (uint32_t) (headerSize + imageDataSize);
  chipStoreLowHighInt32u(fileSizeLocation, fileHeaderInfo->fileSize);

  return CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID;
}
