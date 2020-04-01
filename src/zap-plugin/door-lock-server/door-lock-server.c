/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include EMBER_AF_API_STACK
#ifdef EMBER_AF_API_DEBUG_PRINT
  #include EMBER_AF_API_DEBUG_PRINT
#endif
#include EMBER_AF_API_ZCL_CORE
#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #include EMBER_AF_API_ZCL_SCENES_SERVER
#endif
#include "thread-callbacks.h"
#include "door-lock-server.h"

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_DOOR_LOCK_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_DOOR_LOCK_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
EmZclDoorLockSceneSubTableEntry_t emZclPluginDoorLockServerSceneSubTable[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = emZclPluginDoorLockServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (emZclPluginDoorLockServerSceneSubTable[i] = entry)
  #endif
#endif

// TODO- ZCL server command handlers...

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void emZclDoorLockServerEraseSceneHandler(uint8_t tableIdx)
{
  EmZclDoorLockSceneSubTableEntry_t entry;

  entry.hasLockStateValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool emZclDoorLockServerAddSceneHandler(EmberZclClusterId_t clusterId,
                                        uint8_t tableIdx,
                                        const uint8_t *sceneData,
                                        uint8_t length)
{
  if (clusterId == EMBER_ZCL_CLUSTER_DOOR_LOCK) {
    if (length < 1) {
      return false; // ext field format error (lockStateValue byte must be present).
    }

    // Extract bytes from input data block and update scene subtable fields.
    EmZclDoorLockSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasLockStateValue = true;
    entry.lockStateValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void emZclDoorLockServerRecallSceneHandler(EmberZclEndpointId_t endpointId,
                                           uint8_t tableIdx,
                                           uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the door lock cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant door lock command handler to actually change the
  // hw state.

  EmZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasLockStateValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterDoorLockServerSpec,
                           EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&entry.lockStateValue,
                           sizeof(entry.lockStateValue));
  }
}

void emZclDoorLockServerStoreSceneHandler(EmberZclEndpointId_t endpointId,
                                          uint8_t tableIdx)
{
  EmZclDoorLockSceneSubTableEntry_t entry;

  entry.hasLockStateValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterDoorLockServerSpec,
                           EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&entry.lockStateValue,
                           sizeof(entry.lockStateValue)) == EMBER_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void emZclDoorLockServerCopySceneHandler(uint8_t srcTableIdx,
                                         uint8_t dstTableIdx)
{
  EmZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void emZclDoorLockServerViewSceneHandler(uint8_t tableIdx, uint8_t **ppExtFldData)
{
  EmZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasLockStateValue) {
    emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                EMBER_ZCL_CLUSTER_DOOR_LOCK);
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData, 1);  // length=1
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                               entry.lockStateValue);
  }
}

void emZclDoorLockServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  EmZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  emberAfCorePrint(" door lock:%x",
                   entry.lockStateValue);

  emberAfCoreFlush();
}
#endif

#ifdef DEFINETOKENS
// Defines for User table nv storage.
#define retrieveUserTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_DOOR_LOCK_USER_TABLE, i)
#define saveUserTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_DOOR_LOCK_USER_TABLE, i, &entry)
#else
// Defines for RAM based User table storage.
static EmberAfDoorLockUser_t userTable[EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_USER_TABLE_SIZE] = {};
#define retrieveUserTableEntry(entry, i) \
  (entry = userTable[i])
#define saveUserTableEntry(entry, i) \
  (userTable[i] = entry)
#endif

// Define some nice short macros.
#define USER_TABLE_SIZE      EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_USER_TABLE_SIZE
#define SCHEDULE_TABLE_SIZE  EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_SCHEDULE_TABLE_SIZE
#define MAX_PIN_LENGTH       EMBER_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_MAX_PIN_LENGTH

// Static variable storages.
static EmberAfDoorLockScheduleEntry_t schedule[SCHEDULE_TABLE_SIZE]; //TODO- provide an NV table for schedules?

// Static functions declarations.
static void printScheduleTable(void);
static void printUserTable(void);
static void printPin(const EmberAfDoorLockUser_t *entry);
static void printSuccessOrFailure(bool success);
static bool verifyPin(const EmberZclCommandContext_t *context,
                      const EmberZclStringType_t *pin,
                      uint8_t *userId);
static void opEventResponseHandler(EmberZclMessageStatus_t status,
                                   const EmberZclCommandContext_t *context,
                                   const EmberZclClusterDoorLockClientCommandOperationEventNotificationResponse_t *response);
static void prgEventResponseHandler(EmberZclMessageStatus_t status,
                                    const EmberZclCommandContext_t *context,
                                    const EmberZclClusterDoorLockClientCommandProgrammingEventNotificationResponse_t *response);
static void sendOperationEventToBoundDevice(const EmberZclCommandContext_t *context,
                                            uint8_t userId,
                                            const EmberZclStringType_t *pin,
                                            EmberAfDoorLockOperationEventCode_t eventCode);
static void sendProgrammingEventToBoundDevice(const EmberZclCommandContext_t *context,
                                              uint8_t userId,
                                              uint8_t userType,
                                              uint8_t userStatus,
                                              const EmberZclStringType_t *pin,
                                              EmberAfDoorLockProgrammingEventCode_t eventCode);
static void lockHandler(const EmberZclCommandContext_t *context,
                        const EmberZclStringType_t *pin);
static void unlockHandler(const EmberZclCommandContext_t *context,
                          const EmberZclStringType_t *pin);

// Static functions
static void printScheduleTable(void)
{
  emberAfPluginDoorLockServerPrintln("id uid dm strth strtm stph stpm");

  for (uint8_t i = 0; i < SCHEDULE_TABLE_SIZE; i++) {
    EmberAfDoorLockScheduleEntry_t *entry;
    entry = &schedule[i];
    emberAfPluginDoorLockServerPrintln("%x %x  %x %4x   %4x   %4x  %4x",
                                       i,
                                       entry->userId,
                                       entry->daysMask,
                                       entry->startHour,
                                       entry->stopHour,
                                       entry->stopMinute);
  }
}

static void printUserTable(void)
{
  emberAfPluginDoorLockServerPrintln("id st ty pl pin");
  for (uint8_t i = 0; i < USER_TABLE_SIZE; i++) {
    EmberAfDoorLockUser_t entry;
    retrieveUserTableEntry(entry, i);
    emberAfPluginDoorLockServerPrint("%x %x %x ", i, entry.status, entry.type);
    printPin(&entry);
    emberAfPluginDoorLockServerPrintln("");
  }
}

static void printPin(const EmberAfDoorLockUser_t *entry)
{
  if (entry->pinLength > 0) {
    emberAfPluginDoorLockServerPrint("(%x)", entry->pinLength);
    for (uint8_t i = 0; i < entry->pinLength; i++) {
      emberAfPluginDoorLockServerPrint(" %c", entry->pinData[i]);
    }
  }
}

static void printSuccessOrFailure(bool success)
{
  if (success) {
    emberAfPluginDoorLockServerPrintln("SUCCESS!");
  } else {
    emberAfPluginDoorLockServerPrintln("FAILURE!");
  }
}

static bool verifyPin(const EmberZclCommandContext_t *context,
                      const EmberZclStringType_t *pin,
                      uint8_t *userId)
{
  // Checks to see if a pin is required and, if it is and a pin is provided,
  // it validates the pin against those known in the user table.

  bool pinRequired = false;
  EmberZclStatus_t status;

  status =
    emberZclReadAttribute(context->endpointId,
                          &emberZclClusterDoorLockServerSpec,
                          EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_REQUIRE_PIN_FOR_RF_OPERATION,
                          (uint8_t *)&pinRequired,
                          sizeof(pinRequired));

  if ((status != EMBER_ZCL_STATUS_SUCCESS)
      || (!pinRequired)) {
    return true;
  } else if (pin == NULL) {
    return false;
  }

  for (uint8_t i = 0; i < USER_TABLE_SIZE; i++) {
    EmberAfDoorLockUser_t entry;
    retrieveUserTableEntry(entry, i);
    if ((entry.pinLength == pin->length)
        && (MEMCOMPARE(entry.pinData, pin->ptr, pin->length) == 0)) {
      *userId = i;
      return true;
    }
  }

  return false;
}

static void opEventResponseHandler(EmberZclMessageStatus_t status,
                                   const EmberZclCommandContext_t *context,
                                   const EmberZclClusterDoorLockClientCommandOperationEventNotificationResponse_t *response)
{
  emberAfPluginDoorLockServerPrintln("opEventResponseHandler Status = %d ",
                                     status);
}

static void prgEventResponseHandler(EmberZclMessageStatus_t status,
                                    const EmberZclCommandContext_t *context,
                                    const EmberZclClusterDoorLockClientCommandProgrammingEventNotificationResponse_t *response)
{
  emberAfPluginDoorLockServerPrintln("prgEventResponseHandler Status = %d",
                                     status);
}

static void sendOperationEventToBoundDevice(const EmberZclCommandContext_t *context,
                                            uint8_t userId,
                                            const EmberZclStringType_t *pin,
                                            EmberAfDoorLockOperationEventCode_t eventCode)
{
  uint16_t rfOperationEventMask = 0xffff; //will send events by default

  // Check if we should send event notification.
  emberZclReadAttribute(context->endpointId,
                        &emberZclClusterDoorLockServerSpec,
                        EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_RF_OPERATION_EVENT_MASK,
                        (uint8_t *)&rfOperationEventMask,
                        sizeof(rfOperationEventMask));

  // Possibly send operation event.
  if (rfOperationEventMask & BIT(eventCode)
      && (pin != NULL)) {
    // Find out the entry from the binding table for the cluster spec.
    for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
      EmberZclBindingEntry_t entry = { 0 };
      if (emberZclGetBinding(i, &entry)
          && emberZclCompareClusterSpec(&emberZclClusterDoorLockServerSpec,
                                        &entry.clusterSpec)) {
        EmberZclClusterDoorLockClientCommandOperationEventNotificationRequest_t request = { 0 };
        request.source = EMBER_ZCL_DOOR_LOCK_EVENT_SOURCE_RF;
        request.eventCode = eventCode;
        request.userId = userId;
        request.pin = *pin;
        request.timeStamp = 0xffffffff; // TODO -read from the local clock
        uint8_t data[] = { '1', '2', '3', '4', '5', '6', '7', '8' };
        request.data.length = sizeof(data);
        request.data.ptr = data;

        EmberStatus status =
          emberZclSendClusterDoorLockClientCommandOperationEventNotificationRequest((const EmberZclDestination_t *)&entry.destination,
                                                                                    &request,
                                                                                    opEventResponseHandler);

        emberAfPluginDoorLockServerPrintln("op eventCode=%d, ep=%d, callStatus=%d",
                                           request.eventCode,
                                           entry.destination.application.data.endpointId,
                                           status);

        status = 0; // fixes unused variable warning.
      }
    }
  }
}

static void sendProgrammingEventToBoundDevice(const EmberZclCommandContext_t *context,
                                              uint8_t userId,
                                              uint8_t userType,
                                              uint8_t userStatus,
                                              const EmberZclStringType_t *pin,
                                              EmberAfDoorLockProgrammingEventCode_t eventCode)
{
  uint16_t programmingEventMask = 0xffff; //will send events by default

  // Check if we should send event notification.
  emberZclReadAttribute(context->endpointId,
                        &emberZclClusterDoorLockServerSpec,
                        EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_RF_PROGRAMMING_EVENT_MASK,
                        (uint8_t *)&programmingEventMask,
                        sizeof(programmingEventMask));

  // Possibly send operation event
  if (programmingEventMask & BIT(eventCode)
      && (pin != NULL)) {
    // find out the entry from the binding table for the cluster spec
    for (EmberZclBindingId_t i = 0; i < EMBER_ZCL_BINDING_TABLE_SIZE; i++) {
      EmberZclBindingEntry_t entry = { 0 };
      if (emberZclGetBinding(i, &entry)
          && emberZclCompareClusterSpec(&emberZclClusterDoorLockServerSpec,
                                        &entry.clusterSpec)) {
        EmberZclClusterDoorLockClientCommandProgrammingEventNotificationRequest_t request = { 0 };

        request.source = EMBER_ZCL_DOOR_LOCK_EVENT_SOURCE_RF;
        request.eventCode = eventCode;
        request.userId = userId;
        request.pin = *pin;
        request.userType = userType;
        request.userStatus = userStatus;
        request.timeStamp = 0xffffffff; // TODO -read from the local clock
        uint8_t data[] = { '1', '2', '3', '4', '5', '6', '7', '8' };
        request.data.length = sizeof(data);
        request.data.ptr = data;

        EmberStatus status
          = emberZclSendClusterDoorLockClientCommandProgrammingEventNotificationRequest((const EmberZclDestination_t *)&entry.destination,
                                                                                        &request,
                                                                                        prgEventResponseHandler);

        emberAfPluginDoorLockServerPrintln("prg eventCode=%d, ep=%d, callStatus=%d",
                                           request.eventCode,
                                           entry.destination.application.data.endpointId,
                                           status);

        status = 0; // fixes unused variable warning.
      }
    }
  }
}

static void lockHandler(const EmberZclCommandContext_t *context,
                        const EmberZclStringType_t *pin)
{
  uint8_t userId = 0;
  bool doorLocked = false;

  bool pinVerified = verifyPin(context, pin, &userId);
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorLocked = emberAfPluginDoorLockServerActivateDoorLockCallback(true); // lock door
  }

  // Send cmd response.
  EmberZclClusterDoorLockServerCommandLockDoorResponse_t response;
  response.status = (doorLocked
                     ? EMBER_ZCL_STATUS_SUCCESS
                     : EMBER_ZCL_STATUS_FAILURE);
  emberZclSendClusterDoorLockServerCommandLockDoorResponse(context, &response);

  // Update attribute if success.
  if (doorLocked) {
    uint8_t lockState = EMBER_ZCL_DOOR_LOCK_STATE_LOCKED;
    emberZclWriteAttribute(context->endpointId,
                           &emberZclClusterDoorLockServerSpec,
                           EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&lockState,
                           sizeof(lockState));
  }

  // Send event to bound devices if is enabled and supported.
  EmberAfDoorLockOperationEventCode_t eventCode
    = (doorLocked
       ? EMBER_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_LOCK
       : EMBER_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_LOCK_INVALID_PIN_OR_ID);
  sendOperationEventToBoundDevice(context, userId, pin, eventCode);
}

static void unlockHandler(const EmberZclCommandContext_t *context,
                          const EmberZclStringType_t *pin)
{
  uint8_t userId = 0;
  bool doorUnlocked = false;

  bool pinVerified = verifyPin(context, pin, &userId);
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorUnlocked = emberAfPluginDoorLockServerActivateDoorLockCallback(false);
  }

  // Send cmd response.
  EmberZclClusterDoorLockServerCommandUnlockDoorResponse_t response;
  response.status = (doorUnlocked
                     ? EMBER_ZCL_STATUS_SUCCESS
                     : EMBER_ZCL_STATUS_FAILURE);
  emberZclSendClusterDoorLockServerCommandUnlockDoorResponse(context, &response);

  // Update attribute if success.
  if (doorUnlocked) {
    uint8_t lockState = EMBER_ZCL_DOOR_LOCK_STATE_UNLOCKED;
    emberZclWriteAttribute(context->endpointId,
                           &emberZclClusterDoorLockServerSpec,
                           EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&lockState,
                           sizeof(lockState));
  }

  // Send event to bound devices if the it is enabled and supported.
  EmberAfDoorLockOperationEventCode_t eventCode
    = (doorUnlocked
       ? EMBER_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK
       : EMBER_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK_INVALID_PIN_OR_ID);
  sendOperationEventToBoundDevice(context, userId, pin, eventCode);
}

void emberZclClusterDoorLockServerCommandLockDoorRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterDoorLockServerCommandLockDoorRequest_t *request)
{
  emberAfCorePrintln("RX: LockDoor ");

  lockHandler(context, &request->PIN);
}

void emberZclClusterDoorLockServerCommandUnlockDoorRequestHandler(const EmberZclCommandContext_t *context,
                                                                  const EmberZclClusterDoorLockServerCommandUnlockDoorRequest_t *request)
{
  emberAfCorePrintln("RX: UnlockDoor ");

  unlockHandler(context, &request->PIN);
}

void emberZclClusterDoorLockServerCommandToggleRequestHandler(const EmberZclCommandContext_t *context,
                                                              const EmberZclClusterDoorLockServerCommandToggleRequest_t *request)
{
  emberAfCorePrintln("RX: ToggleLockState");

  uint8_t lockState = 0;

  emberZclReadAttribute(context->endpointId,
                        &emberZclClusterDoorLockServerSpec,
                        EMBER_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                        (uint8_t *)&lockState,
                        sizeof(lockState));

  switch (lockState) {
    case EMBER_ZCL_DOOR_LOCK_STATE_LOCKED:
      unlockHandler(context, &request->pin); // Locked --> unlock it
      break;
    case EMBER_ZCL_DOOR_LOCK_STATE_UNLOCKED:
      lockHandler(context, &request->pin); // Unlocked --> Lock it
      break;
    default:
      break;  // in a state that cannot serve the toggle.
  }
}

void emberZclClusterDoorLockServerCommandClearAllPinsRequestHandler(const EmberZclCommandContext_t *context,
                                                                    const EmberZclClusterDoorLockServerCommandClearAllPinsRequest_t *request)
{
  emberAfCorePrintln("RX: ClearAllPins");

  for (uint8_t i = 0; i < USER_TABLE_SIZE; i++) {
    EmberAfDoorLockUser_t entry = { 0 };
    saveUserTableEntry(entry, i);
  }

  EmberZclClusterDoorLockServerCommandClearAllPinsResponse_t response = { 0 };
  response.status = EMBER_ZCL_STATUS_SUCCESS;

  emberZclSendClusterDoorLockServerCommandClearAllPinsResponse(context,
                                                               &response); // Send response.
}

void emberZclClusterDoorLockServerCommandSetPinRequestHandler(const EmberZclCommandContext_t *context,
                                                              const EmberZclClusterDoorLockServerCommandSetPinRequest_t *request)
{
  emberAfCorePrintln("RX: SetPin");

  EmberZclClusterDoorLockServerCommandSetPinResponse_t response = { 0 };

  if ((request->userId < USER_TABLE_SIZE)
      && (request->pin.length < MAX_PIN_LENGTH)) {
    EmberAfDoorLockUser_t entry;
    entry.status = request->userStatus;
    entry.type = request->userType;
    MEMCOPY(entry.pinData, request->pin.ptr, request->pin.length);
    entry.pinLength = request->pin.length;

    saveUserTableEntry(entry, request->userId);
    printUserTable();

    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  emberZclSendClusterDoorLockServerCommandSetPinResponse(context, &response); // Send response.

  if (response.status == EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS) {
    // Send event notifications.
    sendProgrammingEventToBoundDevice(context,
                                      request->userId,
                                      request->userType,
                                      request->userStatus,
                                      &request->pin,
                                      EMBER_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_ADDED);
  }
}

void emberZclClusterDoorLockServerCommandGetPinRequestHandler(const EmberZclCommandContext_t *context,
                                                              const EmberZclClusterDoorLockServerCommandGetPinRequest_t *request)
{
  emberAfCorePrintln("RX: GetPin");

  if (request->userId < USER_TABLE_SIZE) {
    EmberAfDoorLockUser_t entry;
    retrieveUserTableEntry(entry, request->userId);
    EmberZclClusterDoorLockServerCommandGetPinResponse_t response = { 0 };
    response.userId = request->userId;
    response.userStatus = entry.status;
    response.userType = entry.type;
    response.pin.length = entry.pinLength;
    response.pin.ptr = entry.pinData;
    emberZclSendClusterDoorLockServerCommandGetPinResponse(context, &response); // Send response.
  } else {
    emberZclSendDefaultResponse(context, EMBER_ZCL_STATUS_INSUFFICIENT_SPACE);
  }
}

void emberZclClusterDoorLockServerCommandClearPinRequestHandler(const EmberZclCommandContext_t *context,
                                                                const EmberZclClusterDoorLockServerCommandClearPinRequest_t *request)
{
  emberAfCorePrintln("RX: ClearPin");

  EmberZclClusterDoorLockServerCommandClearPinResponse_t response = { 0 };

  if (request->userId < USER_TABLE_SIZE) {
    EmberAfDoorLockUser_t entry = { 0 };
    saveUserTableEntry(entry, request->userId);
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  emberZclSendClusterDoorLockServerCommandClearPinResponse(context, &response); // Send response.

  if (response.status == EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS) {
    // Send event notifications.
    EmberZclStringType_t userPin = { 0 }; //zero length string
    sendProgrammingEventToBoundDevice(context,
                                      request->userId,
                                      0,
                                      0,
                                      &userPin,
                                      EMBER_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_DELETED);
  }
}

void emberZclClusterDoorLockServerCommandSetWeekdayScheduleRequestHandler(const EmberZclCommandContext_t *context,
                                                                          const EmberZclClusterDoorLockServerCommandSetWeekdayScheduleRequest_t *request)
{
  emberAfCorePrintln("RX: SetWeekdaySchedule");

  EmberZclClusterDoorLockServerCommandSetWeekdayScheduleResponse_t response = { 0 };

  if ((request->userId < USER_TABLE_SIZE)
      && (request->scheduleId < SCHEDULE_TABLE_SIZE)) {
    EmberAfDoorLockScheduleEntry_t *entry = &schedule[request->scheduleId];
    entry->userId = request->userId;
    entry->daysMask = request->daysMask;
    entry->startHour = request->startHour;
    entry->startMinute = request->startMinute;
    entry->stopHour = request->endHour;
    entry->stopMinute = request->endMinute;
    printScheduleTable();
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  emberZclSendClusterDoorLockServerCommandSetWeekdayScheduleResponse(context,
                                                                     &response); // Send response.

  if (response.status == EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS) {
    EmberZclStringType_t userPin = { 0 }; //zero length string
    sendProgrammingEventToBoundDevice(context,
                                      request->userId,
                                      0,
                                      0,
                                      &userPin,
                                      EMBER_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC);
  }
}

void emberZclClusterDoorLockServerCommandGetWeekdayScheduleRequestHandler(const EmberZclCommandContext_t *context,
                                                                          const EmberZclClusterDoorLockServerCommandGetWeekdayScheduleRequest_t *request)
{
  emberAfCorePrintln("RX: GetWeekdaySchedule");

  EmberZclClusterDoorLockServerCommandGetWeekdayScheduleResponse_t response = { 0 };

  if ((request->userId < USER_TABLE_SIZE)
      && (request->scheduleId < SCHEDULE_TABLE_SIZE)) {
    EmberAfDoorLockScheduleEntry_t *entry = &schedule[request->scheduleId];
    response.scheduleId = request->scheduleId;
    response.userId = request->userId;
    response.daysMask = entry->daysMask;
    response.startHour = entry->startHour;
    response.startMinute = entry->startMinute;
    response.endHour = entry->stopHour;
    response.endMinute = entry->stopMinute;
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = EMBER_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  emberZclSendClusterDoorLockServerCommandGetWeekdayScheduleResponse(context,
                                                                     &response); // Send response.
}
