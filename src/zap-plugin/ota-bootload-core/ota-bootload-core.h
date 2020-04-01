/***************************************************************************//**
 * @file
 * @brief ZCL OTA Bootload Core API
 ******************************************************************************/

#ifndef __OTA_BOOTLOAD_CORE_H__
#define __OTA_BOOTLOAD_CORE_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_ZCL_CORE

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
 * @sa EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE
 * @sa EmberZclOtaBootloadFileHeaderInfo_t
 */
#define EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER 0x0BEEF11E

/**
 * This is the size of the magic 32-bit number that appears at the beginning of
 * every OTA file.
 *
 * @sa EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER
 * @sa EmberZclOtaBootloadFileHeaderInfo_t
 */
#define EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER_SIZE 4

/**
 * This is the version of OTA files that work with ZCLIP.
 *
 * @sa EmberZclOtaBootloadFileHeaderInfo_t
 */
#define EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION 0x0200

/**
 * This is the size of the OTA file header string in bytes. This size includes
 * the byte for the NUL-terminator, which must be included in a header string.
 *
 * @sa EmberZclOtaBootloadFileHeaderInfo_t
 */
#define EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE 32

/**
 * This is the maximum number of bytes contained in an OTA file header.
 *
 * These fields are required.
 * -  4-byte file identifier (see ::EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER)
 * -  2-byte header version
 * -  2-byte header length
 * -  2-byte header field control
 * -  2-byte manufacturer code
 * -  2-byte file type
 * -  4-byte file version
 * -  2-byte communication stack version
 * - 32-byte header string (see ::EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE)
 * -  4-byte file size
 * -  1-byte security credential version
 *
 * These fields are optional.
 * - 32-byte destination UID
 * -  2-byte minimum hardware version
 * -  2-byte maximum hardware version
 *
 * @sa EmberZclOtaBootloadFileHeaderInfo_t
 */
#define EMBER_ZCL_OTA_BOOTLOAD_HEADER_MAX_SIZE 93

#ifndef DOXYGEN_SHOULD_SKIP_THIS
// This is a the upgrade URI used for downloading images. See also the
// coapDispatch setup block in the ota-bootload-server plugin.properties file.
#define EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI "ota"
#define EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH 3
// full URI format: "<BASE_URI>/MMMM-HHHH-VVVVVVVV"
#define EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_FORMAT "%s/%04x-%04x-%08x"
#define EM_ZCL_OTA_BOOTLOAD_UPGRADE_FULL_URI_LENGTH (EM_ZCL_OTA_BOOTLOAD_UPGRADE_BASE_URI_LENGTH + 1 + 4 + 1 + 4 + 1 + 8)
#endif

// -----------------------------------------------------------------------------
// Types

// TODO: should this type be generated from from the ZCL types XML data?
// OTA file header field control.
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclOtaBootloadFileHeaderFieldControl_t
#else
typedef uint16_t EmberZclOtaBootloadFileHeaderFieldControl_t;
enum
#endif
{
  // Security Credential Version Present
  EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_SECURITY_CREDENTIAL = 0x0001,
  // OTA file header contains destination field.
  EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_DESTINATION         = 0x0002,
  // OTA file header contains minimum and maximum valid hardware versions.
  EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_HARDWARE_VERSION    = 0x0004,

  // Distinguished value that represents a null (invalid) OTA file header field control.
  EMBER_ZCL_OTA_BOOTLOAD_FILE_HEADER_FIELD_CONTROL_NULL = ((EmberZclOtaBootloadFileHeaderFieldControl_t)-1),
};

// TODO: should this type be generated from from the ZCL types XML data?
/** OTA file type. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclOtaBootloadFileType_t
#else
typedef uint16_t EmberZclOtaBootloadFileType_t;
enum
#endif
{
  /** This is the maximum value for a manufacturer-specific file type. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_MANUFACTURER_SPECIFIC_MAXIMUM = 0xFFBF,

  /** OTA file is security credentials. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_SECURITY_CREDENTIALS = 0xFFC0,
  /** OTA file is a configuration. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_CONFIGURATION        = 0xFFC1,
  /** OTA file is a log. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_LOG                  = 0xFFC2,
  /** OTA file is a picture. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_PICTURE              = 0xFFC3,
  /** OTA file is unspecified. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_TYPE_WILDCARD             = 0xFFFF,
};

/** OTA file version. */
typedef uint32_t EmberZclOtaBootloadFileVersion_t;
/** Distinguished value that represents a null (invalid) OTA file version. */
#define EMBER_ZCL_OTA_BOOTLOAD_FILE_VERSION_NULL ((EmberZclOtaBootloadFileVersion_t)-1)

// TODO: should this type be generated from from the ZCL types XML data?
/** OTA file stack version. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclOtaBootloadStackVersion_t
#else
typedef uint16_t EmberZclOtaBootloadStackVersion_t;
enum
#endif
{
  /** OTA file is for an IP stack. */
  EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_IP = 0x0004,

  /** Distinguished value that represents a null (invalid) OTA file stack version. */
  EMBER_ZCL_OTA_BOOTLOAD_STACK_VERSION_NONE = ((EmberZclOtaBootloadStackVersion_t)-1),
};

// TODO: should this type be generated from the ZCL types XML data?
/** OTA file security credential version. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclOtaBootloadSecurityCredentialVersion_t
#else
typedef uint8_t EmberZclOtaBootloadSecurityCredentialVersion_t;
enum
#endif
{
  /** OTA file uses IP security credentials. */
  EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_IP = 0x03,

  /** Distinguished value that represents a null (invalid) OTA file security credential version. */
  EMBER_ZCL_OTA_BOOTLOAD_SECURITY_CREDENTIAL_VERSION_NULL = ((EmberZclOtaBootloadSecurityCredentialVersion_t)-1),
};

/** OTA file hardware version. */
typedef uint16_t EmberZclOtaBootloadHardwareVersion_t;
/** Distinguished value that represents a null (invalid) OTA file hardware version. */
#define EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL ((EmberZclOtaBootloadHardwareVersion_t)-1)

/** This structure holds an OTA file hardware version range. */
typedef struct {
  /** Minimum OTA file hardware version. */
  EmberZclOtaBootloadHardwareVersion_t minimum;
  /** Maximum OTA file hardware version. */
  EmberZclOtaBootloadHardwareVersion_t maximum;
} EmberZclOtaBootloadHardwareVersionRange_t;

/**
 * This structure holds an OTA file specification.
 *
 * This file specification identifies a single OTA file, distinguishing it from
 * another OTA file.
 *
 * @sa emberZclOtaBootloadFileSpecNull
 */
typedef struct {
  /** Manufacturer code. */
  EmberZclManufacturerCode_t manufacturerCode;
  /** OTA file type. */
  EmberZclOtaBootloadFileType_t type;
  /** OTA file version. */
  EmberZclOtaBootloadFileVersion_t version;
} EmberZclOtaBootloadFileSpec_t;

/**
 * This is a distinguished value that represents a null (invalid) OTA file specification.
 *
 * This is provided as a utility to applications that wish to use a file
 * specification value that is \e uninitialized or \e invalid.
 */
extern const EmberZclOtaBootloadFileSpec_t emberZclOtaBootloadFileSpecNull;

/** OTA file status. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum EmberZclOtaBootloadFileStatus_t
#else
typedef uint8_t EmberZclOtaBootloadFileStatus_t;
enum
#endif
{
  /** OTA file is valid. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_VALID                               = 0x00,
  /** OTA file has invalid magic number. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_MAGIC_NUMBER                = 0x01,
  /** OTA file has invalid version. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_VERSION                     = 0x02,
  /** OTA file has invalid header size. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_HEADER_SIZE                 = 0x03,
  /** OTA file has invalid stack version. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_STACK_VERSION               = 0x04,
  /** OTA file has invalid security credential version. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_INVALID_SECURITY_CREDENTIAL_VERSION = 0x05,

  /** Distinguished value that represents a null (invalid) OTA file status. */
  EMBER_ZCL_OTA_BOOTLOAD_FILE_STATUS_NULL = 0xFF,
};

/**
 * This structure holds information about an OTA file header.
 *
 * This type contains all of the same information in the header of an actual
 * OTA file, except for the magic number
 * (::EMBER_ZCL_OTA_BOOTLOAD_FILE_MAGIC_NUMBER), which is constant, and the
 * field control. The field control is replaced by the use of invalid values
 * for the destination and hardwareVersionRange members.
 * Member                        | Invalid Value
 * ----------------------------- | ----------------------------------------------------------
 * destination                   | All 0xFF bytes
 * hardwareVersionRange.minimum  | ::EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL
 * hardwareVersionRange.maximum  | ::EMBER_ZCL_OTA_BOOTLOAD_HARDWARE_VERSION_NULL
 *
 * @sa EmberZclOtaBootloadFileHeaderFieldControl_t
 */
typedef struct {
  /** OTA file metadata version. */
  uint16_t version;
  /** OTA file header size, in bytes. */
  uint16_t headerSize;
  /** OTA file specification. */
  EmberZclOtaBootloadFileSpec_t spec;
  /** OTA file intended stack version. */
  EmberZclOtaBootloadStackVersion_t stackVersion;
  /** OTA file header string (must be NUL-terminated). */
  uint8_t string[EMBER_ZCL_OTA_BOOTLOAD_HEADER_STRING_SIZE];
  /** OTA file size, in bytes. */
  uint32_t fileSize;
  /** OTA file security credential version. */
  EmberZclOtaBootloadSecurityCredentialVersion_t securityCredentialVersion;
  /** OTA file destination identifier. */
  EmberZclUid_t destination;
  /** OTA file earliest and latest valid hardware versions. */
  EmberZclOtaBootloadHardwareVersionRange_t hardwareVersionRange;
} EmberZclOtaBootloadFileHeaderInfo_t;

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
bool emberZclOtaBootloadIsWildcard(uint8_t *bytes, size_t size);

/**************************************************************************//**
 * This function initializes the EmberZclOtaBootloadFileHeaderInfo_t structure.
 *
 * @param headerInfo Structure to be initialized
 *****************************************************************************/
void emberZclOtaBootloadInitFileHeaderInfo(EmberZclOtaBootloadFileHeaderInfo_t *headerInfo);

/**************************************************************************//**
 * This function compares two OTA file specifications.
 *
 * @param s1 OTA file specification to be compared
 * @param s2 OTA file specification to be compared
 * @return `true` if both OTA file specifications are equal, `false` otherwise.
 *****************************************************************************/
bool emberZclOtaBootloadFileSpecsAreEqual(const EmberZclOtaBootloadFileSpec_t *s1,
                                          const EmberZclOtaBootloadFileSpec_t *s2);

/**************************************************************************//**
 * This function reads an ::EmberZclOtaBootloadFileSpec_t from a flat buffer (little-endian).
 *
 * @param data Flat buffer from which to read a file specification struct
 * @param fileSpec File specification struct to be populated from a flat buffer
 * @return The number of bytes read from the flat buffer.
 *****************************************************************************/
size_t emberZclOtaBootloadFetchFileSpec(const uint8_t *data,
                                        EmberZclOtaBootloadFileSpec_t *fileSpec);

/**************************************************************************//**
 * This function writes an ::EmberZclOtaBootloadFileSpec_t to a flat buffer (little-endian).
 *
 * @param fileSpec File specification struct to be written to a flat buffer
 * @param data Flat buffer to which the file specification struct will be
 *             written
 * @return The number of bytes written to the flat buffer.
 *****************************************************************************/
size_t emberZclOtaBootloadStoreFileSpec(const EmberZclOtaBootloadFileSpec_t *fileSpec,
                                        uint8_t *data);

/**************************************************************************//**
 * This function reads an ::EmberZclOtaBootloadFileHeaderInfo_t from a flat buffer
 * (little-endian).
 *
 * @param data Flat buffer from which to read a file header info structure
 * @param fileHeaderInfo File header info struct to be populated from a flat
 *                       buffer
 * @return An ::EmberZclOtaBootloadFileStatus_t value describing if the data is
 *         a valid or invalid dotdot OTA data.
 *****************************************************************************/
EmberZclOtaBootloadFileStatus_t emberZclOtaBootloadFetchFileHeaderInfo(const uint8_t *data,
                                                                       EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo);

/**************************************************************************//**
 * This function writes an ::EmberZclOtaBootloadFileHeaderInfo_t to a flat buffer
 * (little-endian). The fileHeaderInfo->headerSize and fileHeaderInfo->fileSize
 * variables are updated to reflect the stored sizes.
 *
 * @param data Flat buffer to which to write a file header info structure
 * @param fileHeaderInfo File header info struct to be written
 * @return An ::EmberZclOtaBootloadFileStatus_t value describing if the data is
 *         a valid or invalid dotdot OTA data.
 *****************************************************************************/
EmberZclOtaBootloadFileStatus_t emberZclOtaBootloadStoreFileHeaderInfo(uint8_t *data,
                                                                       EmberZclOtaBootloadFileHeaderInfo_t *fileHeaderInfo,
                                                                       size_t imageDataSize);

/** @} end addtogroup */

/** @} end addtogroup */

/** @} end addtogroup ZCLIP */

#endif // __OTA_BOOTLOAD_CORE_H__
