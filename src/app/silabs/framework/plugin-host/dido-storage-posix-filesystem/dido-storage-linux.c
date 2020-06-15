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
 * @brief A means to store DIDO data on a Linux system.
 *******************************************************************************
   ******************************************************************************/

// this file contains all the common includes for clusters in the util
#include "app/framework/include/af.h"
#include "app/framework/util/common.h"
#include "dido-storage-linux.h"

#if defined (IMAGE_BUILDER)
// For our PC tool, we use a simpler #define to turn on this code.
  #define GATEWAY_APP
#endif

#if defined(GATEWAY_APP)
// These includes are wrapped inside the #ifdef because they are platform
// specific.

#define _GNU_SOURCE     // strnlen
#include <string.h>     // ""

#include <stdio.h>      // fopen, fread, fwrite, fclose, fseek, fprintf
#include <sys/types.h>  // stat
#include <sys/stat.h>   // ""
#include <unistd.h>     // ""
#include <stdarg.h>     // va_start, va_end

#include <stdlib.h>     // malloc, free
#include <errno.h>      // errno, strerror

#include <dirent.h>     // opendir, readdir
#include <time.h>       // timestamp

#ifdef __APPLE__
#define strnlen(string, n) strlen((string))
#endif

//------------------------------------------------------------------------------
// Globals

// This will always end with a '/'.  The code will append one if not present.
static char* storageDevice = NULL;
static bool storageDeviceIsDirectory = false;

static const char* tempStorageFile = "dido-report";
static bool firstInit = TRUE;

#define MAX_FILEPATH_LENGTH  256

#if defined(WIN32)
  #define portableMkdir(x) _mkdir(x)
#else
  #define portableMkdir(x) \
  mkdir((x), S_IRUSR | S_IWUSR | S_IXUSR)   /* permissions (o=rwx) */
#endif

static const char defaultStorageDirectory[] = "dido-reports";

// Debug
static int allocations = 0;

// #defines to enable printing / debugging of File and Memory operations.
#define FILE_DEBUG  false
#define MEMORY_DEBUG false

static EmberAfDidoStorageStatus setDidoStorageDevice(const char* directoryOrFile);
static EmberAfDidoStorageStatus createDefaultStorageDirectory(void);
static char *getNewDidoReportFilepath(void);
static EmberAfDidoStorageStatus writeRawData(const char* filepath,
                                             const uint8_t* data,
                                             uint32_t length);
static void freeReportFilepath(char *filepath);

// Internal Debug Functions
static void* myMalloc(size_t size, const char* allocName);
static void myFree(void* ptr);
static void error(const char* formatString, ...);
static void note(const char* formatString, ...);
static void debug(bool debugOn, const char* formatString, ...);

//==============================================================================
// Public API
//==============================================================================
void emberAfPluginDidoStorageInitCallback(void)
{
  createDefaultStorageDirectory();
  firstInit = FALSE;
}

EmberAfDidoStorageStatus emberAfDidoStorageWriteReport(uint8_t* data,
                                                       uint32_t length)
{
  EmberAfDidoStorageStatus status;
  char *reportFilepath;

  if (firstInit) {
    // Ensure storage directories are created before writing.
    emberAfPluginDidoStorageInitCallback();
  }
  debug(FILE_DEBUG,
        "Write DIDO report, length=%d\n",
        length);
  reportFilepath = getNewDidoReportFilepath();
  if (reportFilepath == NULL) {
    return EMBER_AF_DIDO_STORAGE_ERROR;
  }
  status = writeRawData(reportFilepath, data, length);
  freeReportFilepath(reportFilepath);
  return status;
}

//==============================================================================
// Internal Functions
//==============================================================================

static EmberAfDidoStorageStatus createDefaultStorageDirectory(void)
{
  struct stat statInfo;
  int returnValue = stat(defaultStorageDirectory, &statInfo);
  if (returnValue == 0) {
    if (!S_ISDIR(statInfo.st_mode)) {
      error("Default storage directory '%s' is not a directory!\n",
            defaultStorageDirectory);
      return EMBER_AF_DIDO_STORAGE_ERROR;
    }

    debug(FILE_DEBUG,
          "Default storage directory already exists '%s'\n",
          defaultStorageDirectory);
  } else {
    //   Does not exist, therefore we must create it.
    debug(FILE_DEBUG,
          "Creating default storage directory '%s'\n",
          defaultStorageDirectory);

    int status = portableMkdir(defaultStorageDirectory);

    if (status != 0) {
      error("Could not create default directory '%s': %s\n",
            defaultStorageDirectory,
            strerror(errno));
      return EMBER_AF_DIDO_STORAGE_ERROR;
    }
  }
  return setDidoStorageDevice(defaultStorageDirectory);
}

static EmberAfDidoStorageStatus setDidoStorageDevice(const char* directoryOrFile)
{
  static bool initDone = false;
  struct stat statInfo;
  int length;
  int returnValue;

  if (initDone) {
    return EMBER_AF_DIDO_STORAGE_ERROR;
  }

  length = strnlen(directoryOrFile, MAX_FILEPATH_LENGTH + 1);
  if (MAX_FILEPATH_LENGTH < length) {
    error("Storage directory path too long (max = %d)!\n", MAX_FILEPATH_LENGTH);
    return EMBER_AF_DIDO_STORAGE_ERROR;
  }

  // Add 1 for '\0' and 1 for '/'.  This may or may not be necessary
  // because the path already has it or it is only a file.
  storageDevice = myMalloc(length + 2,
                           "emAfSetStorageDevice(): storageDevice");
  if (storageDevice == NULL) {
    error("Could not allocate %d bytes!\n", length);
    return EMBER_AF_DIDO_STORAGE_ERROR;
  }
  if (directoryOrFile[length - 1] == '/') {
    // We don't want to copy the '/' yet, since stat() will
    // complain if we pass it in.
    length--;
  }
  memset(storageDevice, 0, length + 2);
  strncpy(storageDevice, directoryOrFile, length);

  returnValue = stat(storageDevice, &statInfo);
  debug(FILE_DEBUG,
        "Checking for existence of '%s'\n",
        storageDevice);
  if (returnValue != 0) {
    error("Could not read storage device '%s'. %s\n",
          directoryOrFile,
          strerror(errno));
    myFree(storageDevice);
    storageDevice = NULL;
    return EMBER_AF_DIDO_STORAGE_ERROR;
  }

  if (S_ISDIR(statInfo.st_mode)) {
    storageDeviceIsDirectory = true;
    storageDevice[length] = '/';
  }
  debug(FILE_DEBUG, "Storage device set to '%s'.\n", storageDevice);
  initDone = true;
  return EMBER_AF_DIDO_STORAGE_SUCCESS;
}

static char *getNewDidoReportFilepath()
{
  #define TIMESTAMP_SIZE  16    // -YYYYMMDD-hhmmss
  #define EXTENSION_SIZE   4    // .log
  EmberAfDidoStorageStatus status = EMBER_AF_DIDO_STORAGE_ERROR;
  int reportFilepathLength;
  char *reportFile = NULL;
  struct tm *myTime;
  time_t now;

  if (!storageDeviceIsDirectory) {
    error("Cannot create temp. OTA data because storage device is a file, not a directory.\n");
    return NULL;
  }

  if (storageDevice == NULL) {
    error("No storage device defined!");
    return NULL;
  }

  // Add 1 to make sure we have room for a NULL terminating character
  reportFilepathLength = (strlen(storageDevice)
                          + strlen(tempStorageFile) + 1
                          + TIMESTAMP_SIZE + EXTENSION_SIZE);
  if (reportFilepathLength > MAX_FILEPATH_LENGTH) {
    return NULL;
  }
  reportFile = myMalloc(reportFilepathLength,
                        "didoStoragePrepareDidoReport(): reportFile");
  if (reportFile == NULL) {
    return NULL;
  }
  now = time(NULL);
  myTime = localtime(&now);
  snprintf(reportFile,
           reportFilepathLength,
           "%s%s-%.4d%.2d%.2d-%.2d%.2d%.2d.log",
           storageDevice,
           tempStorageFile,
           myTime->tm_year + 1900,
           myTime->tm_mon + 1,
           myTime->tm_mday,
           myTime->tm_hour,
           myTime->tm_min,
           myTime->tm_sec);

  return reportFile;
}

static void freeReportFilepath(char *filepath)
{
  if (filepath != NULL) {
    myFree(filepath);
  }
}

static EmberAfDidoStorageStatus writeRawData(const char* filepath,
                                             const uint8_t* data,
                                             uint32_t length)
{
  // Windows requires the 'b' (binary) as part of the mode so that line endings
  // are not truncated.  POSIX ignores this.
  EmberAfDidoStorageStatus status;
  FILE* fileHandle = fopen(filepath, "wb");
  size_t written;

  if (fileHandle == NULL) {
    error("Could not open file '%s' for writing: %s\n",
          filepath,
          strerror(errno));
    return EMBER_AF_DIDO_STORAGE_ERROR;
  }
  written = fwrite(data, 1, length, fileHandle);
  if (written != length) {
    error("Tried to write %d bytes but wrote %d\n", length, written);
    status = EMBER_AF_DIDO_STORAGE_ERROR;
  } else {
    status = EMBER_AF_DIDO_STORAGE_SUCCESS;
  }
  fclose(fileHandle);
  return status;
}

//------------------------------------------------------------------------------
// DEBUG

static void* myMalloc(size_t size, const char* allocName)
{
  void* returnValue = malloc(size);
  if (returnValue != NULL) {
    allocations++;
    debug(MEMORY_DEBUG,
          "[myMalloc] %s, %d bytes (0x%08X)\n",
          allocName, size, returnValue);
  }
  return returnValue;
}

static void myFree(void* ptr)
{
  debug(MEMORY_DEBUG, "[myFree] 0x%08X\n", ptr);
  free(ptr);
  allocations--;
}

//------------------------------------------------------------------------------
// Print routines.

static void message(FILE* stream,
                    bool error,
                    const char* formatString,
                    va_list ap)
{
  if (error) {
    fprintf(stream, "Error: ");
  }
  vfprintf(stream, formatString, ap);
  fflush(stream);
}

static void note(const char* formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  message(stdout, false, formatString, ap);
  va_end(ap);
}

static void debug(bool debugOn, const char* formatString, ...)
{
  if (debugOn) {
    va_list ap = { 0 };
    va_start(ap, formatString);
    message(stdout, false, formatString, ap);
    va_end(ap);
  }
}

static void error(const char* formatString, ...)
{
  va_list ap = { 0 };
  va_start(ap, formatString);
  message(stderr, true, formatString, ap);
  va_end(ap);
}

#endif  // #ifdef GATEWAY_APP
