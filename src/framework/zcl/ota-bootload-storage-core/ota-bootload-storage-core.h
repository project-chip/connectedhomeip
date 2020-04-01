/***************************************************************************//**
 * @file
 * @brief ZCL OTA Bootload Storage Core API
 ******************************************************************************/

#ifndef __OTA_BOOTLOAD_STORAGE_CORE_H__
#define __OTA_BOOTLOAD_STORAGE_CORE_H__

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_ZCL_CORE
#include CHIP_AF_API_ZCL_OTA_BOOTLOAD_CORE

/**
 * @addtogroup ZCLIP
 *
 * @{
 */

/**
 * @addtogroup OTA_Bootload
 * @{
 */

/**
 * @addtogroup OTA_Bootload_Types
 * @{
 */

// -----------------------------------------------------------------------------
// Types

/**
 * OTA storage module information.
 *
 * This is the current information about the OTA storage module.
 */
typedef struct {
  /** The maximum size of an OTA file allowed by a storage module. */
  size_t maximumFileSize;
  /** The number of OTA files in the storage module. */
  size_t fileCount;
} ChipZclOtaBootloadStorageInfo_t;

/**
 * OTA file information.
 *
 * This is the information about an OTA file in the storage module.
 */
typedef struct {
  /** The size of the OTA file, in bytes. */
  size_t size;
} ChipZclOtaBootloadStorageFileInfo_t;

/** OTA storage status. */
#ifdef DOXYGEN_SHOULD_SKIP_THIS
enum ChipZclOtaBootloadStorageStatus_t
#else
typedef uint8_t ChipZclOtaBootloadStorageStatus_t;
enum
#endif
{
  /** An operation has succeeded. */
  CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS      = 0x00,
  /** An operation has failed. */
  CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED       = 0x01,
  /** An operation is outside a valid range. */
  CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE = 0x02,
  /** An operation is specified on a nonexistent file. */
  CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE = 0x03,
  /** An operation is outside valid space constraints. */
  CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE = 0x04,

  /** Distinguished value that represents a null (invalid) status value. */
  CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_NULL = 0xFF,
};

typedef void (*ChipZclOtaBootloadStorageDeleteCallback)(ChipZclOtaBootloadStorageStatus_t);

/** @} end addtogroup */

/**
 * @addtogroup OTA_Bootload_API
 * @{
 */

// -----------------------------------------------------------------------------
// API

/**************************************************************************//**
 * This function gets information about the current storage module.
 *
 * @param info Returned information about the current storage module.
 * @param returnedFiles The returned list of files that exist in this module.
 *                      This parameter can be set to NULL to be ignored
 *                      by the implementation.
 * @param returnedFilesMaxCount The maximum number of entries in the provided
 *                              returnedFiles parameter.
 *
 * @note The implementation of this function must adhere to the following
 *       specifications:
 *       - The implementation of this function must return successfully. If, for
 *         any reason, the storage module cannot successfully return the
 *         necessary information in the implementation of this function, it
 *         should raise the error by calling ::assert(false).
 *       - The implementation of this function must be safe and idempotent,
 *         i.e., contiguous calls to this function must result in the same
 *         returned information and no change in the state of the storage
 *         module.
 *       - The implementation of this function does not need to be thread-safe.
 *****************************************************************************/
void chipZclOtaBootloadStorageGetInfo(ChipZclOtaBootloadStorageInfo_t *info,
                                       ChipZclOtaBootloadFileSpec_t *returnedFiles,
                                       size_t returnedFilesMaxCount);

/**************************************************************************//**
 * This function finds a file in the storage module.
 *
 * @param fileSpec A file specification describing the file to be found
 * @param fileInfo Returned information about the file to be found; only valid
 *                 if the function returns
 *                 ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS
 * @return One of the following status values.
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS if the file was
 *           successfully found
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE if the file
 *           does not exist
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED if any other failure
 *           occured
 *
 * @note The implementation of this function must adhere to the following
 *       specifications:
 *       - The implementation of this function must be safe and idempotent,
 *         i.e., contiguous calls to this function with equal fileSpec
 *         parameters (as determined by ::chipZclOtaBootloadFileSpecsAreEqual)
 *         must result in the same returned information and no change in the
 *         state of the storage module.
 *       - The implementation of this function does not need to be thread-safe.
 *****************************************************************************/
ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageFind(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  ChipZclOtaBootloadStorageFileInfo_t *fileInfo);

/**************************************************************************//**
 * This function creates a file in the storage module.
 *
 * @param fileSpec A file specification for the file to be created
 * @return One of the following status values.
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS if the file was
 *           successfully created
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE if the file
 *           already exists
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED if any other failure
 *           occured
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_SPACE if there is
 *           not enough space to create this file
 *
 * @note The implementation of this function must adhere to the following
 *       specifications:
 *       - The implementation of this function must be atomic.
 *       - The implementation of this function does not need to be thread-safe.
 *****************************************************************************/
ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageCreate(const ChipZclOtaBootloadFileSpec_t *fileSpec);

/**************************************************************************//**
 * This function reads contiguous bytes from a file in the storage module.
 *
 * @param fileSpec A file specification for the file to be read
 * @param offset The offset into the file at which to start reading bytes
 * @param data The buffer into which the bytes will be read
 * @param dataLength The number of bytes to read from the file
 * @return One of the following status values.
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS if dataLength
 *           number of bytes were successfully read
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE if reading
 *           dataLength number of bytes starting at offset would result in
 *           reading past the end of the file
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE if the file
 *           does not exist in the storage module
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED if any other failure
 *           occured
 *
 * @note The implementation of this function must adhere to the following
 *       specifications:
 *       - The implementation of this function must be atomic.
 *       - The implementation of this function must support random read access.
 *       - The implementation of this function must allow for random access
 *         reading, i.e., reading from any valid offset must be supported.
 *       - The implementation of this function does not need to be thread-safe.
 *****************************************************************************/
ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageRead(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                  size_t offset,
                                                                  void *data,
                                                                  size_t dataLength);

/**************************************************************************//**
 * This function writes contiguous bytes to a file in the storage module.
 *
 * @param fileSpec A file specification for the file to be written
 * @param offset The offset into the file at which to start writing bytes
 * @param data The bytes to be written
 * @param dataLength The number of bytes to written to the file
 * @return One of the following status values.
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS if dataLength
 *           number of bytes were successfully written
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_OUT_OF_RANGE if writing
 *           dataLength number of bytes starting at offset would result in
 *           writing past the end of the file
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE if the file
 *           does not exist in the storage module
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED if any other failure
 *           occured
 *
 * @note The implementation of this function must adhere to the following
 *       specifications:
 *       - The implementation of this function must be atomic.
 *       - The implementation of this function must support sequential write
 *         access, but does not need to support random write access.
 *       - The implementation of this function does not need to be thread-safe.
 *****************************************************************************/
ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageWrite(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                   size_t offset,
                                                                   const void *data,
                                                                   size_t dataLength);

/**************************************************************************//**
 * This function deletes one or all files in the storage module asynchronously.
 *
 * @param fileSpec A file specification for the file to be deleted, or
 *                 ::chipZclOtaBootloadFileSpecNull for all files to be deleted
 * @param callback A callback to be called upon completion of this deletion
 *                 operation
 * @return One of the following status values.
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_SUCCESS if deletion was
 *           successfully started on the indicated file/s
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_INVALID_FILE if the file
 *           does not exist in the storage module
 *         - ::CHIP_ZCL_OTA_BOOTLOAD_STORAGE_STATUS_FAILED if any other failure
 *           occured
 *
 * @note The implementation of this function must adhere to the following
 *       specification:
 *       - The implementation of this function does not need to be thread-safe.
 *****************************************************************************/
ChipZclOtaBootloadStorageStatus_t chipZclOtaBootloadStorageDelete(const ChipZclOtaBootloadFileSpec_t *fileSpec,
                                                                    ChipZclOtaBootloadStorageDeleteCallback callback);

/** @} end addtogroup */

/** @} end addtogroup */

/** @} end addtogroup ZCLIP */

#endif // __OTA_BOOTLOAD_STORAGE_CORE_H__
