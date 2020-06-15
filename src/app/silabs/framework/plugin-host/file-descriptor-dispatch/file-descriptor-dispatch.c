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
 * @brief This plugin provides support for registering a Unix file descriptor with
 * a callback.  This allows this code to call select() on a set of file
 * descriptors with knowledge of exactly what functions to call when
 * the file descriptor is ready for reading or writing.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "file-descriptor-dispatch.h"

#include <stdlib.h>     // for malloc()
#include <sys/time.h>   // for select()
#include <sys/types.h>  // ""
#include <unistd.h>     // ""
#include <errno.h>      // ""

//=============================================================================
// Globals

struct LinkListItemStruct {
  EmberAfFileDescriptorDispatchStruct dispatchStruct;
  struct LinkListItemStruct* next;
  struct LinkListItemStruct* prev;
  bool markedForRemoval;
  bool badFd;
};
typedef struct LinkListItemStruct LinkListItem;

static bool executingCallbacks = false;

static LinkListItem* theList = NULL;

const char emAfFileDescriptorDispatchPluginName[] = "FD Dispatch";
#define PLUGIN_NAME emAfFileDescriptorDispatchPluginName

//#define DEBUG_ON
#if defined(DEBUG_ON)
#define debugPrint(...) emberAfCorePrintln(__VA_ARGS__)
#else
#define debugPrint(...)
#endif

//=============================================================================
// Forward Declarations

LinkListItem* findDispatchByFileDescriptor(int fileDescriptor);
LinkListItem* addItem(LinkListItem* previous,
                      EmberAfFileDescriptorDispatchStruct* dispatchStruct);
static void scanForBadFds(void);
//=============================================================================

EmberStatus emberAfPluginFileDescriptorDispatchAdd(EmberAfFileDescriptorDispatchStruct* dispatchStruct)
{
  if (NULL != findDispatchByFileDescriptor(dispatchStruct->fileDescriptor)) {
    emberAfCorePrintln("%p dispatch for fileDescriptor %d already exists",
                       PLUGIN_NAME,
                       dispatchStruct->fileDescriptor);
    return EMBER_INVALID_CALL;
  }

  if ((dispatchStruct->operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_NONE)
      || (dispatchStruct->operation >= EMBER_AF_FILE_DESCRIPTOR_OPERATION_MAX)) {
    emberAfCorePrintln("Error: %p bad file descriptor operation of %d", dispatchStruct->operation);
    return EMBER_INVALID_CALL;
  }

  LinkListItem* iterator = theList;

  while (iterator != NULL && iterator->next != NULL) {
    iterator = iterator->next;
  }

  iterator = addItem(iterator, dispatchStruct);
  if (NULL == iterator) {
    return EMBER_ERR_FATAL;
  }

  if (theList == NULL) {
    theList = iterator;
  }
  return EMBER_SUCCESS;
}

#if defined(DEBUG_ON)
void debugPrintList(void)
{
  LinkListItem* iterator = theList;
  emberAfCorePrint("Current FD List:");
  while (iterator != NULL) {
    emberAfCorePrint("%d ", iterator->dispatchStruct.fileDescriptor);
    iterator = iterator->next;
    if (iterator != NULL) {
      emberAfCorePrint("-> ");
    }
  }
  emberAfCorePrintln("");
}
#else
#define debugPrintList()
#endif

LinkListItem* addItem(LinkListItem* previous, EmberAfFileDescriptorDispatchStruct* dispatchStruct)
{
  LinkListItem* new = malloc(sizeof(LinkListItem));
  if (new == NULL) {
    emberAfCorePrintln("Error: %p failed to allocate memory for dispatch list element.",
                       PLUGIN_NAME);
    return NULL;
  }

  debugPrint("%p Previous FD in list is %d",
             PLUGIN_NAME,
             (previous == NULL
              ? -1
              : previous->dispatchStruct.fileDescriptor));

  MEMMOVE(&(new->dispatchStruct), dispatchStruct, sizeof(EmberAfFileDescriptorDispatchStruct));
  new->markedForRemoval = false;
  new->badFd = false;
  new->next = NULL;
  new->prev = previous;
  if (previous) {
    previous->next = new;
  }
  debugPrint("%p added dispatch for FD %d", PLUGIN_NAME, new->dispatchStruct.fileDescriptor);
  debugPrintList();
  return new;
}

LinkListItem* findDispatchByFileDescriptor(int fileDescriptor)
{
  LinkListItem* iterator = theList;
  while (iterator != NULL) {
    if (iterator->dispatchStruct.fileDescriptor == fileDescriptor) {
      return iterator;
    }
    iterator = iterator->next;
  }

  return NULL;
}

static void removeItem(LinkListItem* toRemove)
{
  if (toRemove == theList) {
    theList = toRemove->next;
  } else if (toRemove->prev != NULL) {
    toRemove->prev->next = toRemove->next;
  }
  if (toRemove->next != NULL) {
    toRemove->next->prev = toRemove->prev;
  }

  debugPrint("%p removed dispatch for FD %d", PLUGIN_NAME, toRemove->dispatchStruct.fileDescriptor);
  free(toRemove);
  debugPrintList();
}

bool emberAfPluginFileDescriptorDispatchRemove(int fileDescriptor)
{
  LinkListItem* toRemove = findDispatchByFileDescriptor(fileDescriptor);

  if (toRemove == NULL) {
    emberAfCorePrintln("Error: %p could not find fileDescriptor %d to remove dispatch.",
                       PLUGIN_NAME,
                       fileDescriptor);
    return false;
  }

  // We must prevent modification of the list while we are iterating through it.
  // Instead mark the item for removal at a later date.
  if (executingCallbacks) {
    toRemove->markedForRemoval = true;
    debugPrint("%p marked FD %d for removal later.", PLUGIN_NAME, fileDescriptor);
    return true;
  }

  removeItem(toRemove);
  return true;
}

static void cleanupItemsMarkedForRemoval(void)
{
  LinkListItem* iterator = theList;
  while (iterator != NULL) {
    if (iterator->markedForRemoval == true) {
      removeItem(iterator);
    }
    iterator = iterator->next;
  }
}

EmberStatus emberAfPluginFileDescriptorDispatchWaitForEvents(uint32_t timeoutMs)
{
  fd_set readSet;
  fd_set writeSet;
  fd_set exceptSet;
  // Set to -2 so highestFd + 1 = -1, which is still an invalid value
  int highestFd = -2;
  LinkListItem* iterator = theList;

  static bool firstRun = true;
  if (firstRun) {
    firstRun = false;
    debugPrint("%p first run, not waiting for data.", PLUGIN_NAME);
    return EMBER_SUCCESS;
  }

  // If timeoutMs is 0, then don't bother calling select.
  // Just return here. Bug: EMAPPFWKV2-1196.
  if (timeoutMs == 0) {
    return EMBER_SUCCESS;
  }

  FD_ZERO(&readSet);
  FD_ZERO(&writeSet);
  FD_ZERO(&exceptSet);

  while (iterator != NULL && iterator->badFd == false) {
    if (iterator->dispatchStruct.operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_READ) {
      debugPrint("%p added read FD %d",
                 PLUGIN_NAME,
                 iterator->dispatchStruct.fileDescriptor);
      FD_SET(iterator->dispatchStruct.fileDescriptor,
             &readSet);
    }
    if (iterator->dispatchStruct.operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_WRITE) {
      debugPrint("%p added write FD %d",
                 PLUGIN_NAME,
                 iterator->dispatchStruct.fileDescriptor);
      FD_SET(iterator->dispatchStruct.fileDescriptor,
             &writeSet);
    }
    if (iterator->dispatchStruct.operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_EXCEPT) {
      debugPrint("%p added except FD %d",
                 PLUGIN_NAME,
                 iterator->dispatchStruct.fileDescriptor);
      FD_SET(iterator->dispatchStruct.fileDescriptor,
             &exceptSet);
    }
    if (iterator->dispatchStruct.fileDescriptor > highestFd) {
      highestFd = iterator->dispatchStruct.fileDescriptor;
    }
    iterator = iterator->next;
  }
  struct timeval timeoutStruct = {
    timeoutMs / 1000,           // seconds
    (timeoutMs % 1000) * 1000,  // micro seconds
  };

  debugPrint("%p select() called, highestFd %d, timeout %d ms", PLUGIN_NAME, highestFd, timeoutMs);

  int status = 0;
  if (highestFd >= 0) {
    status = select(highestFd + 1,   // per select() manpage
                    &readSet,
                    &writeSet,
                    &exceptSet,
                    (timeoutMs != MAX_INT32U_VALUE
                     ? &timeoutStruct
                     : NULL));
  }
  if (status < 0) {
    emberAfCorePrintln("%p select() failed: %p", PLUGIN_NAME, strerror(errno));

    // EMZIGBEE-2126 Try to isolate the bad file descriptor,
    if (errno == EBADF) {
      // Individually scan each FD and mark any that are in error to exclude
      // from further select polls.
      scanForBadFds();
    }
  }

  if (status < 0 || highestFd < 0) {
    // For error status or no descriptors to be polled, impose the intended
    // select timeout delay to prevent CPU spin on repeated select error.
    if (timeoutMs != MAX_INT32U_VALUE) {
      usleep((long)(timeoutMs * 1000));
    } else {
      sleep(1);
    }

    return EMBER_SUCCESS;
  }

  debugPrint("%p select() returned %d", PLUGIN_NAME, status);

  if (status == 0) {
    // timeout, no FDs set.
    return EMBER_SUCCESS;
  }

  iterator = theList;
  executingCallbacks = true;
  while (iterator != NULL) {
    if (iterator->dispatchStruct.callback != NULL) {
      // NOTE: We must check markedForRemoval/badFd before each callback since a
      // previous callback may have modified the state
      if (!iterator->markedForRemoval && !iterator->badFd) {
        if (FD_ISSET(iterator->dispatchStruct.fileDescriptor, &readSet)) {
          (iterator->dispatchStruct.callback)(iterator->dispatchStruct.dataPassedToCallback,
                                              EMBER_AF_FILE_DESCRIPTOR_OPERATION_READ);
        }
        if (FD_ISSET(iterator->dispatchStruct.fileDescriptor, &writeSet)) {
          (iterator->dispatchStruct.callback)(iterator->dispatchStruct.dataPassedToCallback,
                                              EMBER_AF_FILE_DESCRIPTOR_OPERATION_WRITE);
        }
        if (FD_ISSET(iterator->dispatchStruct.fileDescriptor, &exceptSet)) {
          (iterator->dispatchStruct.callback)(iterator->dispatchStruct.dataPassedToCallback,
                                              EMBER_AF_FILE_DESCRIPTOR_OPERATION_EXCEPT);
        }
      }
    }
    iterator = iterator->next;
  }
  executingCallbacks = false;

  cleanupItemsMarkedForRemoval();

  return EMBER_SUCCESS;
}

// If above select() returns a EBADF error, scan each file descriptor to
// identify and exclude the faulty file descriptor from further select polling.
// Invoke callback to notify application and possibly prompt recovery.
static void scanForBadFds(void)
{
  fd_set readSet;
  fd_set writeSet;
  fd_set exceptSet;
  LinkListItem* iterator = theList;

  while (iterator != NULL && iterator->badFd == false) {
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);
    FD_ZERO(&exceptSet);

    if (iterator->dispatchStruct.operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_READ) {
      FD_SET(iterator->dispatchStruct.fileDescriptor, &readSet);
    }
    if (iterator->dispatchStruct.operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_WRITE) {
      FD_SET(iterator->dispatchStruct.fileDescriptor, &writeSet);
    }
    if (iterator->dispatchStruct.operation == EMBER_AF_FILE_DESCRIPTOR_OPERATION_EXCEPT) {
      FD_SET(iterator->dispatchStruct.fileDescriptor, &exceptSet);
    }

    struct timeval zeroTime = {
      0, // seconds
      0, // micro seconds
    };

    int status = select(iterator->dispatchStruct.fileDescriptor + 1,
                        &readSet,
                        &writeSet,
                        &exceptSet,
                        &zeroTime);

    if (status < 0) {
      if (errno == EBADF) {
        emberAfCorePrintln("%p bad file descriptor: %d", PLUGIN_NAME,
                           iterator->dispatchStruct.fileDescriptor);
        emberAfPluginFileDescriptorDispatchBadFileDescriptorCallback(iterator->dispatchStruct.fileDescriptor);
        iterator->badFd = true;
      }
    }

    iterator = iterator->next;
  }
}
