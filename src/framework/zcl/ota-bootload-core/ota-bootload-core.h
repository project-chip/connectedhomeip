/***************************************************************************//**
 * @file
 * @brief ZCL OTA Bootload Core API
 ******************************************************************************/

#ifndef __OTA_BOOTLOAD_CORE_H__
#define __OTA_BOOTLOAD_CORE_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup OTA_Bootload OTA Bootload
 *
 * The following image shows bootloading accomplished using Over-The-Air (OTA), that is through the wireless network.
 * \image html bootloading_ota.jpg
 *
 * @{
 */

/**
 * @addtogroup OTA_Bootload_Types OTA Bootload Types
 * @{
 */

// -----------------------------------------------------------------------------
// Constants

/**
 * This is the magic 32-bit number that appears at the beginning of every OTA
 * file.
 *
 * @sa CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE
 * @sa ChipZclOtaBootloadFileHeaderInfo_t
 */
#define CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER 0x0BEEF11E

/**
 * This is the size of the magic 32-bit number that appears at the beginning of
 * every OTA file.
 *
 * @sa CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER
 * @sa ChipZclOtaBootloadFileHeaderInfo_t
 */
#define CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE 4

/**
 * This is the version of OTA files that work with ZCLIP.
 *
 * @sa ChipZclOtaBootloadFileHeaderInfo_t
 */
#define CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION 0x0200

/**
 * This is the size of the OTA file header string in bytes. This size includes
 * the byte for the NUL-terminator, which must be included in a header string.
 *
 * @sa ChipZclOtaBootloadFileHeaderInfo_t
 */
#define CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE 32

/**
 * This is the maximum number of bytes contained in an OTA file header.
 *
 * These fields are required.
 * -  4-byte file identifier (see ::CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER)
 * -  2-byte header version
 * -  2-byte header length
 * -  2-byte header field control
 * -  2-byte manufacturer code
 * -  2-byte file type
 * -  4-byte file version
 * -  2-byte communication stack version
 * - 32-byte header string (see ::CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE)
 * -  4-byte file size
 * -  1-byte security credential version
 *
 * These fields are optional.
 * - 32-byte destination UID
 * -  2-byte minimum hardware version
 * -  2-byte maximum hardware version
 *
 * @sa ChipZclOtaBootloadFileHeaderInfo_t
 */
#define CHIP_ZCL_OTA_BOOTLOAD_HEADER_MAX_SIZE 93

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// This is a the upgrade URI used for downloading images. See also the
// coapDispatch setup block in the ota-bootload-server plugin.properties file.
#define CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI "ota"
#define CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH 3
// full URI format: "<BASE_URI>/MMMM-HHHH-VVVVVVVV"
#define CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_FORMAT "%s/%04x-%04x-%08x"
#define CH_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH (CH_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH + 1 + 4 + 1 + 4 + 1 + 8)
#endif

// -----------------------------------------------------------------------------
// Types

// TODO: should this type be generated from from the ZCL types XML data?
// OTA file header field control.
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclOtaBootloadFileHeaderFieldControl_t
#else
typedef uint16_t ChipZclOtaBootloadFileHeaderFieldControl_t;
enum
#endif
{
  // Security Credential Version Present
  CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_SECURITY_CREDENTIAL = 0x0001,
  // OTA file header contains destination field.
  CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION         = 0x0002,
  // OTA file header contains minimum and maximum valid hardware versions.
  CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION    = 0x0004,

  // Distinguished value that represents a null (invalid) OTA file header field control.
  CHIP_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_NULL = ((ChipZclOtaBootloadFileHeaderFieldControl_t)-1),
};

// TODO: should this type be generated from from the ZCL types XML data?
/** OTA file type. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclOtaBootloadFileType_t
#else
typedef uint16_t ChipZclOtaBootloadFileType_t;
enum
#endif
{
  /** This is the maximum value for a manufacturer-specific file type. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_MANUFACTURER_SPECIFIC_MAXIMUM = 0xFFBF,

  /** OTA file is security credentials. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_SECURITY_CREDENTIALS = 0xFFC0,
  /** OTA file is a configuration. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_CONFIGURATION        = 0xFFC1,
  /** OTA file is a log. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_LOG                  = 0xFFC2,
  /** OTA file is a picture. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_PICTURE              = 0xFFC3,
  /** OTA file is unspecified. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD             = 0xFFFF,
};

/** OTA file version. */
typedef uint32_t ChipZclOtaBootloadFileVersion_t;
/** Distinguished value that represents a null (invalid) OTA file version. */
#define CHIP_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL ((ChipZclOtaBootloadFileVersion_t)-1)

// TODO: should this type be generated from from the ZCL types XML data?
/** OTA file stack version. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclOtaBootloadStackVersion_t
#else
typedef uint16_t ChipZclOtaBootloadStackVersion_t;
enum
#endif
{
  /** OTA file is for an IP stack. */
  CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP = 0x0004,

  /** Distinguished value that represents a null (invalid) OTA file stack version. */
  CHIP_ZCL_OTA_BOOTLOAD_STACK_VERSION_NONE = ((ChipZclOtaBootloadStackVersion_t)-1),
};

// TODO: should this type be generated from the ZCL types XML data?
/** OTA file security credential version. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclOtaBootloadSecurityCredentialVersion_t
#else
typedef uint8_t ChipZclOtaBootloadSecurityCredentialVersion_t;
enum
#endif
{
  /** OTA file uses IP security credentials. */
  CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP = 0x03,

  /** Distinguished value that represents a null (invalid) OTA file security credential version. */
  CHIP_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_NULL = ((ChipZclOtaBootloadSecurityCredentialVersion_t)-1),
};

/** OTA file hardware version. */
typedef uint16_t ChipZclOtaBootloadHardwareVersion_t;
/** Distinguished value that represents a null (invalid) OTA file hardware version. */
#define CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL ((ChipZclOtaBootloadHardwareVersion_t)-1)

/** This structure holds an OTA file hardware version range. */
typedef struct {
  /** Minimum OTA file hardware version. */
  ChipZclOtaBootloadHardwareVersion_t minimum;
  /** Maximum OTA file hardware version. */
  ChipZclOtaBootloadHardwareVersion_t maximum;
} ChipZclOtaBootloadHardwareVersionRange_t;

/**
 * This structure holds an OTA file specification.
 *
 * This file specification identifies a single OTA file, distinguishing it from
 * another OTA file.
 *
 * @sa chipZclOtaBootloadFileSpecNull
 */
typedef struct {
  /** Manufacturer code. */
  ChipZclManufacturerCode_t manufacturerCode;
  /** OTA file type. */
  ChipZclOtaBootloadFileType_t type;
  /** OTA file version. */
  ChipZclOtaBootloadFileVersion_t version;
} ChipZclOtaBootloadFileSpec_t;

/**
 * This is a distinguished value that represents a null (invalid) OTA file specification.
 *
 * This is provided as a utility to applications that wish to use a file
 * specification value that is \e uninitialized or \e invalid.
 */
extern const ChipZclOtaBootloadFileSpec_t chipZclOtaBootloadFileSpecNull;

/** OTA file status. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclOtaBootloadFileStatus_t
#else
typedef uint8_t ChipZclOtaBootloadFileStatus_t;
enum
#endif
{
  /** OTA file is valid. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID                               = 0x00,
  /** OTA file has invalid magic number. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_MAGIC_NUMBER                = 0x01,
  /** OTA file has invalid version. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION                     = 0x02,
  /** OTA file has invalid header size. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_HEADER_SIZE                 = 0x03,
  /** OTA file has invalid stack version. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION               = 0x04,
  /** OTA file has invalid security credential version. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION = 0x05,

  /** Distinguished value that represents a null (invalid) OTA file status. */
  CHIP_ZCL_OTA_BOOTLOAD_FILE_STATUS_NULL = 0xFF,
};

/**
 * This structure holds information about an OTA file header.
 *
 * This type contains all of the same information in the header of an actual
 * OTA file, except for the magic number
 * (::CHIP_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER), which is constant, and the
 * field control. The field control is replaced by the use of invalid values
 * for the destination and hardwareVersionRange mchips.
 * Mchip                        | Invalid Value
 * ----------------------------- | ----------------------------------------------------------
 * destination                   | All 0xFF bytes
 * hardwareVersionRange.minimum  | ::CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL
 * hardwareVersionRange.maximum  | ::CHIP_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL
 *
 * @sa ChipZclOtaBootloadFileHeaderFieldControl_t
 */
typedef struct {
  /** OTA file metadata version. */
  uint16_t version;
  /** OTA file header size, in bytes. */
  uint16_t headerSize;
  /** OTA file specification. */
  ChipZclOtaBootloadFileSpec_t spec;
  /** OTA file intended stack version. */
  ChipZclOtaBootloadStackVersion_t stackVersion;
  /** OTA file header string (must be NUL-terminated). */
  uint8_t string[CHIP_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE];
  /** OTA file size, in bytes. */
  uint32_t fileSize;
  /** OTA file security credential version. */
  ChipZclOtaBootloadSecurityCredentialVersion_t securityCredentialVersion;
  /** OTA file destination identifier. */
  ChipZclUid_t destination;
  /** OTA file earliest and latest valid hardware versions. */
  ChipZclOtaBootloadHardwareVersionRange_t hardwareVersionRange;
} ChipZclOtaBootloadFileHeaderInfo_t;

/** @} end addtogroup */

/**
 * @addtogroup OTA_Bootload_API OTA Bootload API
 * @{
 */

// -----------------------------------------------------------------------------
// API

/**************************************************************************//**
 * This function initializes checks if all bytes are 0xFF
 *
 * @param bytes Array to be checked
 * @param size Length of the array
 *****************************************************************************/
bool chipZclOtaBootloadIsWildcard(uint8_t *bytes, size_t size);

/**************************************************************************//**
 * This function initializes the ChipZclOtaBootloadFileHeaderInfo_t structure.
 *
 * @param headerInfo Structure to be initialized
 *****************************************************************************/
void chipZclOtaBootloadInitFileHeaderInfo(ChipZclOtaBootloadFileHeaderInfo_t *headerInfo);

/**************************************************************************//**
 * This function compares two OTA file specifications.
 *
 * @param s1 OTA file specification to be compared
 * @param s2 OTA file specification to be compared
 * @return `true` if both OTA file specifications are equal, `false` otherwise.
 *****************************************************************************/
bool chipZclOtaBootloadFileSpecsAreEqual(const ChipZclOtaBootloadFileSpec_t *s1,
                                          const ChipZclOtaBootloadFileSpec_t *s2);

/**************************************************************************//**
 * This function reads an ::ChipZclOtaBootloadFileSpec_t from a flat buffer (little-endian).
 *
 * @param data Flat buffer from which to read a file specification struct
 * @param fileSpec File specification struct to be populated from a flat buffer
 * @return The number of bytes read from the flat buffer.
 *****************************************************************************/
size_t chipZclOtaBootloadFetchFileSpec(const uint8_t *data,
                                        ChipZclOtaBootloadFileSpec_t *fileSpec);

/**************************************************************************//**
 * This function writes an ::ChipZclOtaBootloadFileSpec_t to a flat buffer (little-endian).
 *
 * @param fileSpec File specification struct to be written to a flat buffer
 * @param data Flat buffer to which the file specification struct will be
 *             written
 * @return The number of bytes written to the flat buffer.
 *****************************************************************************/
size_t chipZclOtaBootloadStoreFileSpec(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                        uint8_t *data);

/**************************************************************************//**
 * This function reads an ::ChipZclOtaBootloadFileHeaderInfo_t from a flat buffer
 * (little-endian).
 *
 * @param data Flat buffer from which to read a file header info structure
 * @param fileHeaderInfo File header info struct to be populated from a flat
 *                       buffer
 * @return An ::ChipZclOtaBootloadFileStatus_t value describing if the data is
 *         a valid or invalid dotdot OTA data.
 *****************************************************************************/
ChipZclOtaBootloadFileStatus_t chipZclOtaBootloadFetchFileHeaderInfo(const uint8_t *data,
                                                                       ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo);

/**************************************************************************//**
 * This function writes an ::ChipZclOtaBootloadFileHeaderInfo_t to a flat buffer
 * (little-endian). The fileHeaderInfo->headerSize and fileHeaderInfo->fileSize
 * variables are updated to reflect the stored sizes.
 *
 * @param data Flat buffer to which to write a file header info structure
 * @param fileHeaderInfo File header info struct to be written
 * @return An ::ChipZclOtaBootloadFileStatus_t value describing if the data is
 *         a valid or invalid dotdot OTA data.
 *****************************************************************************/
ChipZclOtaBootloadFileStatus_t chipZclOtaBootloadStoreFileHeaderInfo(uint8_t *data,
                                                                       ChipZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo,
                                                                       size_t imageDataSize);

/** @} end addtogroup */

/** @} end addtogroup */

/** @} end addtogroup ZCLIP */

#endif // __OTA_BOOTLOAD_CORE_H__
