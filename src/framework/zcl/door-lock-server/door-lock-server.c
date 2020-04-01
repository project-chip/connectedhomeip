/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif
#include CHIP_AF_API_ZCL_CORE
#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #include CHIP_AF_API_ZCL_SCENES_SERVER
#endif
#include "thread-callbacks.h"
#include "door-lock-server.h"

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_DOOR_LOCK_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_DOOR_LOCK_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
ChZclDoorLockSceneSubTableEntry_t chZclPluginDoorLockServerSceneSubTable[CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = chZclPluginDoorLockServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (chZclPluginDoorLockServerSceneSubTable[i] = entry)
  #endif
#endif

// TODO- ZCL server command handlers...

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void chZclDoorLockServerEraseSceneHandler(uint8_t tableIdx)
{
  ChZclDoorLockSceneSubTableEntry_t entry;

  entry.hasLockStateValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool chZclDoorLockServerAddSceneHandler(ChipZclClusterId_t clusterId,
                                        uint8_t tableIdx,
                                        const uint8_t *sceneData,
                                        uint8_t length)
{
  if (clusterId == CHIP_ZCL_CLUSTER_DOOR_LOCK) {
    if (length < 1) {
      return false; // ext field format error (lockStateValue byte must be present).
    }

    // Extract bytes from input data block and update scene subtable fields.
    ChZclDoorLockSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasLockStateValue = true;
    entry.lockStateValue = chipZclPluginScenesServerGetUint8FromBuffer(&pData);

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void chZclDoorLockServerRecallSceneHandler(ChipZclEndpointId_t endpointId,
                                           uint8_t tableIdx,
                                           uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the door lock cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant door lock command handler to actually change the
  // hw state.

  ChZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasLockStateValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterDoorLockServerSpec,
                           CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&entry.lockStateValue,
                           sizeof(entry.lockStateValue));
  }
}

void chZclDoorLockServerStoreSceneHandler(ChipZclEndpointId_t endpointId,
                                          uint8_t tableIdx)
{
  ChZclDoorLockSceneSubTableEntry_t entry;

  entry.hasLockStateValue =
    (chipZclReadAttribute(endpointId,
                           &chipZclClusterDoorLockServerSpec,
                           CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&entry.lockStateValue,
                           sizeof(entry.lockStateValue)) == CHIP_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void chZclDoorLockServerCopySceneHandler(uint8_t srcTableIdx,
                                         uint8_t dstTableIdx)
{
  ChZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void chZclDoorLockServerViewSceneHandler(uint8_t tableIdx, uint8_t **ppExtFldData)
{
  ChZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasLockStateValue) {
    chipZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                CHIP_ZCL_CLUSTER_DOOR_LOCK);
    chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData, 1);  // length=1
    chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                               entry.lockStateValue);
  }
}

void chZclDoorLockServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  ChZclDoorLockSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  chipAfCorePrint(" door lock:%x",
                   entry.lockStateValue);

  chipAfCoreFlush();
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
static ChipAfDoorLockUser_t userTable[CHIP_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_USER_TABLE_SIZE] = {};
#define retrieveUserTableEntry(entry, i) \
  (entry = userTable[i])
#define saveUserTableEntry(entry, i) \
  (userTable[i] = entry)
#endif

// Define some nice short macros.
#define USER_TABLE_SIZE      CHIP_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_USER_TABLE_SIZE
#define SCHEDULE_TABLE_SIZE  CHIP_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_SCHEDULE_TABLE_SIZE
#define MAX_PIN_LENGTH       CHIP_AF_PLUGIN_DOOR_LOCK_SERVER_DOOR_LOCK_MAX_PIN_LENGTH

// Static variable storages.
static ChipAfDoorLockScheduleEntry_t schedule[SCHEDULE_TABLE_SIZE]; //TODO- provide an NV table for schedules?

// Static functions declarations.
static void printScheduleTable(void);
static void printUserTable(void);
static void printPin(const ChipAfDoorLockUser_t *entry);
static void printSuccessOrFailure(bool success);
static bool verifyPin(const ChipZclCommandContext_t *context,
                      const ChipZclStringType_t *pin,
                      uint8_t *userId);
static void opEventResponseHandler(ChipZclMessageStatus_t status,
                                   const ChipZclCommandContext_t *context,
                                   const ChipZclClusterDoorLockClientCommandOperationEventNotificationResponse_t *response);
static void prgEventResponseHandler(ChipZclMessageStatus_t status,
                                    const ChipZclCommandContext_t *context,
                                    const ChipZclClusterDoorLockClientCommandProgrammingEventNotificationResponse_t *response);
static void sendOperationEventToBoundDevice(const ChipZclCommandContext_t *context,
                                            uint8_t userId,
                                            const ChipZclStringType_t *pin,
                                            ChipAfDoorLockOperationEventCode_t eventCode);
static void sendProgrammingEventToBoundDevice(const ChipZclCommandContext_t *context,
                                              uint8_t userId,
                                              uint8_t userType,
                                              uint8_t userStatus,
                                              const ChipZclStringType_t *pin,
                                              ChipAfDoorLockProgrammingEventCode_t eventCode);
static void lockHandler(const ChipZclCommandContext_t *context,
                        const ChipZclStringType_t *pin);
static void unlockHandler(const ChipZclCommandContext_t *context,
                          const ChipZclStringType_t *pin);

// Static functions
static void printScheduleTable(void)
{
  chipAfPluginDoorLockServerPrintln("id uid dm strth strtm stph stpm");

  for (uint8_t i = 0; i < SCHEDULE_TABLE_SIZE; i++) {
    ChipAfDoorLockScheduleEntry_t *entry;
    entry = &schedule[i];
    chipAfPluginDoorLockServerPrintln("%x %x  %x %4x   %4x   %4x  %4x",
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
  chipAfPluginDoorLockServerPrintln("id st ty pl pin");
  for (uint8_t i = 0; i < USER_TABLE_SIZE; i++) {
    ChipAfDoorLockUser_t entry;
    retrieveUserTableEntry(entry, i);
    chipAfPluginDoorLockServerPrint("%x %x %x ", i, entry.status, entry.type);
    printPin(&entry);
    chipAfPluginDoorLockServerPrintln("");
  }
}

static void printPin(const ChipAfDoorLockUser_t *entry)
{
  if (entry->pinLength > 0) {
    chipAfPluginDoorLockServerPrint("(%x)", entry->pinLength);
    for (uint8_t i = 0; i < entry->pinLength; i++) {
      chipAfPluginDoorLockServerPrint(" %c", entry->pinData[i]);
    }
  }
}

static void printSuccessOrFailure(bool success)
{
  if (success) {
    chipAfPluginDoorLockServerPrintln("SUCCESS!");
  } else {
    chipAfPluginDoorLockServerPrintln("FAILURE!");
  }
}

static bool verifyPin(const ChipZclCommandContext_t *context,
                      const ChipZclStringType_t *pin,
                      uint8_t *userId)
{
  // Checks to see if a pin is required and, if it is and a pin is provided,
  // it validates the pin against those known in the user table.

  bool pinRequired = false;
  ChipZclStatus_t status;

  status =
    chipZclReadAttribute(context->endpointId,
                          &chipZclClusterDoorLockServerSpec,
                          CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_REQUIRE_PIN_FOR_RF_OPERATION,
                          (uint8_t *)&pinRequired,
                          sizeof(pinRequired));

  if ((status != CHIP_ZCL_STATUS_SUCCESS)
      || (!pinRequired)) {
    return true;
  } else if (pin == NULL) {
    return false;
  }

  for (uint8_t i = 0; i < USER_TABLE_SIZE; i++) {
    ChipAfDoorLockUser_t entry;
    retrieveUserTableEntry(entry, i);
    if ((entry.pinLength == pin->length)
        && (MEMCOMPARE(entry.pinData, pin->ptr, pin->length) == 0)) {
      *userId = i;
      return true;
    }
  }

  return false;
}

static void opEventResponseHandler(ChipZclMessageStatus_t status,
                                   const ChipZclCommandContext_t *context,
                                   const ChipZclClusterDoorLockClientCommandOperationEventNotificationResponse_t *response)
{
  chipAfPluginDoorLockServerPrintln("opEventResponseHandler Status = %d ",
                                     status);
}

static void prgEventResponseHandler(ChipZclMessageStatus_t status,
                                    const ChipZclCommandContext_t *context,
                                    const ChipZclClusterDoorLockClientCommandProgrammingEventNotificationResponse_t *response)
{
  chipAfPluginDoorLockServerPrintln("prgEventResponseHandler Status = %d",
                                     status);
}

static void sendOperationEventToBoundDevice(const ChipZclCommandContext_t *context,
                                            uint8_t userId,
                                            const ChipZclStringType_t *pin,
                                            ChipAfDoorLockOperationEventCode_t eventCode)
{
  uint16_t rfOperationEventMask = 0xffff; //will send events by default

  // Check if we should send event notification.
  chipZclReadAttribute(context->endpointId,
                        &chipZclClusterDoorLockServerSpec,
                        CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_RF_OPERATION_EVENT_MASK,
                        (uint8_t *)&rfOperationEventMask,
                        sizeof(rfOperationEventMask));

  // Possibly send operation event.
  if (rfOperationEventMask & BIT(eventCode)
      && (pin != NULL)) {
    // Find out the entry from the binding table for the cluster spec.
    for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
      ChipZclBindingEntry_t entry = { 0 };
      if (chipZclGetBinding(i, &entry)
          && chipZclCompareClusterSpec(&chipZclClusterDoorLockServerSpec,
                                        &entry.clusterSpec)) {
        ChipZclClusterDoorLockClientCommandOperationEventNotificationRequest_t request = { 0 };
        request.source = CHIP_ZCL_DOOR_LOCK_EVENT_SOURCE_RF;
        request.eventCode = eventCode;
        request.userId = userId;
        request.pin = *pin;
        request.timeStamp = 0xffffffff; // TODO -read from the local clock
        uint8_t data[] = { '1', '2', '3', '4', '5', '6', '7', '8' };
        request.data.length = sizeof(data);
        request.data.ptr = data;

        ChipStatus status =
          chipZclSendClusterDoorLockClientCommandOperationEventNotificationRequest((const ChipZclDestination_t *)&entry.destination,
                                                                                    &request,
                                                                                    opEventResponseHandler);

        chipAfPluginDoorLockServerPrintln("op eventCode=%d, ep=%d, callStatus=%d",
                                           request.eventCode,
                                           entry.destination.application.data.endpointId,
                                           status);

        status = 0; // fixes unused variable warning.
      }
    }
  }
}

static void sendProgrammingEventToBoundDevice(const ChipZclCommandContext_t *context,
                                              uint8_t userId,
                                              uint8_t userType,
                                              uint8_t userStatus,
                                              const ChipZclStringType_t *pin,
                                              ChipAfDoorLockProgrammingEventCode_t eventCode)
{
  uint16_t programmingEventMask = 0xffff; //will send events by default

  // Check if we should send event notification.
  chipZclReadAttribute(context->endpointId,
                        &chipZclClusterDoorLockServerSpec,
                        CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_RF_PROGRAMMING_EVENT_MASK,
                        (uint8_t *)&programmingEventMask,
                        sizeof(programmingEventMask));

  // Possibly send operation event
  if (programmingEventMask & BIT(eventCode)
      && (pin != NULL)) {
    // find out the entry from the binding table for the cluster spec
    for (ChipZclBindingId_t i = 0; i < CHIP_ZCL_BINDING_TABLE_SIZE; i++) {
      ChipZclBindingEntry_t entry = { 0 };
      if (chipZclGetBinding(i, &entry)
          && chipZclCompareClusterSpec(&chipZclClusterDoorLockServerSpec,
                                        &entry.clusterSpec)) {
        ChipZclClusterDoorLockClientCommandProgrammingEventNotificationRequest_t request = { 0 };

        request.source = CHIP_ZCL_DOOR_LOCK_EVENT_SOURCE_RF;
        request.eventCode = eventCode;
        request.userId = userId;
        request.pin = *pin;
        request.userType = userType;
        request.userStatus = userStatus;
        request.timeStamp = 0xffffffff; // TODO -read from the local clock
        uint8_t data[] = { '1', '2', '3', '4', '5', '6', '7', '8' };
        request.data.length = sizeof(data);
        request.data.ptr = data;

        ChipStatus status
          = chipZclSendClusterDoorLockClientCommandProgrammingEventNotificationRequest((const ChipZclDestination_t *)&entry.destination,
                                                                                        &request,
                                                                                        prgEventResponseHandler);

        chipAfPluginDoorLockServerPrintln("prg eventCode=%d, ep=%d, callStatus=%d",
                                           request.eventCode,
                                           entry.destination.application.data.endpointId,
                                           status);

        status = 0; // fixes unused variable warning.
      }
    }
  }
}

static void lockHandler(const ChipZclCommandContext_t *context,
                        const ChipZclStringType_t *pin)
{
  uint8_t userId = 0;
  bool doorLocked = false;

  bool pinVerified = verifyPin(context, pin, &userId);
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorLocked = chipAfPluginDoorLockServerActivateDoorLockCallback(true); // lock door
  }

  // Send cmd response.
  ChipZclClusterDoorLockServerCommandLockDoorResponse_t response;
  response.status = (doorLocked
                     ? CHIP_ZCL_STATUS_SUCCESS
                     : CHIP_ZCL_STATUS_FAILURE);
  chipZclSendClusterDoorLockServerCommandLockDoorResponse(context, &response);

  // Update attribute if success.
  if (doorLocked) {
    uint8_t lockState = CHIP_ZCL_DOOR_LOCK_STATE_LOCKED;
    chipZclWriteAttribute(context->endpointId,
                           &chipZclClusterDoorLockServerSpec,
                           CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&lockState,
                           sizeof(lockState));
  }

  // Send event to bound devices if is enabled and supported.
  ChipAfDoorLockOperationEventCode_t eventCode
    = (doorLocked
       ? CHIP_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_LOCK
       : CHIP_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_LOCK_INVALID_PIN_OR_ID);
  sendOperationEventToBoundDevice(context, userId, pin, eventCode);
}

static void unlockHandler(const ChipZclCommandContext_t *context,
                          const ChipZclStringType_t *pin)
{
  uint8_t userId = 0;
  bool doorUnlocked = false;

  bool pinVerified = verifyPin(context, pin, &userId);
  printSuccessOrFailure(pinVerified);

  if (pinVerified) {
    doorUnlocked = chipAfPluginDoorLockServerActivateDoorLockCallback(false);
  }

  // Send cmd response.
  ChipZclClusterDoorLockServerCommandUnlockDoorResponse_t response;
  response.status = (doorUnlocked
                     ? CHIP_ZCL_STATUS_SUCCESS
                     : CHIP_ZCL_STATUS_FAILURE);
  chipZclSendClusterDoorLockServerCommandUnlockDoorResponse(context, &response);

  // Update attribute if success.
  if (doorUnlocked) {
    uint8_t lockState = CHIP_ZCL_DOOR_LOCK_STATE_UNLOCKED;
    chipZclWriteAttribute(context->endpointId,
                           &chipZclClusterDoorLockServerSpec,
                           CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                           (uint8_t *)&lockState,
                           sizeof(lockState));
  }

  // Send event to bound devices if the it is enabled and supported.
  ChipAfDoorLockOperationEventCode_t eventCode
    = (doorUnlocked
       ? CHIP_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK
       : CHIP_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK_INVALID_PIN_OR_ID);
  sendOperationEventToBoundDevice(context, userId, pin, eventCode);
}

void chipZclClusterDoorLockServerCommandLockDoorRequestHandler(const ChipZclCommandContext_t *context,
                                                                const ChipZclClusterDoorLockServerCommandLockDoorRequest_t *request)
{
  chipAfCorePrintln("RX: LockDoor ");

  lockHandler(context, &request->PIN);
}

void chipZclClusterDoorLockServerCommandUnlockDoorRequestHandler(const ChipZclCommandContext_t *context,
                                                                  const ChipZclClusterDoorLockServerCommandUnlockDoorRequest_t *request)
{
  chipAfCorePrintln("RX: UnlockDoor ");

  unlockHandler(context, &request->PIN);
}

void chipZclClusterDoorLockServerCommandToggleRequestHandler(const ChipZclCommandContext_t *context,
                                                              const ChipZclClusterDoorLockServerCommandToggleRequest_t *request)
{
  chipAfCorePrintln("RX: ToggleLockState");

  uint8_t lockState = 0;

  chipZclReadAttribute(context->endpointId,
                        &chipZclClusterDoorLockServerSpec,
                        CHIP_ZCL_CLUSTER_DOOR_LOCK_SERVER_ATTRIBUTE_LOCK_STATE,
                        (uint8_t *)&lockState,
                        sizeof(lockState));

  switch (lockState) {
    case CHIP_ZCL_DOOR_LOCK_STATE_LOCKED:
      unlockHandler(context, &request->pin); // Locked --> unlock it
      break;
    case CHIP_ZCL_DOOR_LOCK_STATE_UNLOCKED:
      lockHandler(context, &request->pin); // Unlocked --> Lock it
      break;
    default:
      break;  // in a state that cannot serve the toggle.
  }
}

void chipZclClusterDoorLockServerCommandClearAllPinsRequestHandler(const ChipZclCommandContext_t *context,
                                                                    const ChipZclClusterDoorLockServerCommandClearAllPinsRequest_t *request)
{
  chipAfCorePrintln("RX: ClearAllPins");

  for (uint8_t i = 0; i < USER_TABLE_SIZE; i++) {
    ChipAfDoorLockUser_t entry = { 0 };
    saveUserTableEntry(entry, i);
  }

  ChipZclClusterDoorLockServerCommandClearAllPinsResponse_t response = { 0 };
  response.status = CHIP_ZCL_STATUS_SUCCESS;

  chipZclSendClusterDoorLockServerCommandClearAllPinsResponse(context,
                                                               &response); // Send response.
}

void chipZclClusterDoorLockServerCommandSetPinRequestHandler(const ChipZclCommandContext_t *context,
                                                              const ChipZclClusterDoorLockServerCommandSetPinRequest_t *request)
{
  chipAfCorePrintln("RX: SetPin");

  ChipZclClusterDoorLockServerCommandSetPinResponse_t response = { 0 };

  if ((request->userId < USER_TABLE_SIZE)
      && (request->pin.length < MAX_PIN_LENGTH)) {
    ChipAfDoorLockUser_t entry;
    entry.status = request->userStatus;
    entry.type = request->userType;
    MEMCOPY(entry.pinData, request->pin.ptr, request->pin.length);
    entry.pinLength = request->pin.length;

    saveUserTableEntry(entry, request->userId);
    printUserTable();

    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  chipZclSendClusterDoorLockServerCommandSetPinResponse(context, &response); // Send response.

  if (response.status == CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS) {
    // Send event notifications.
    sendProgrammingEventToBoundDevice(context,
                                      request->userId,
                                      request->userType,
                                      request->userStatus,
                                      &request->pin,
                                      CHIP_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_ADDED);
  }
}

void chipZclClusterDoorLockServerCommandGetPinRequestHandler(const ChipZclCommandContext_t *context,
                                                              const ChipZclClusterDoorLockServerCommandGetPinRequest_t *request)
{
  chipAfCorePrintln("RX: GetPin");

  if (request->userId < USER_TABLE_SIZE) {
    ChipAfDoorLockUser_t entry;
    retrieveUserTableEntry(entry, request->userId);
    ChipZclClusterDoorLockServerCommandGetPinResponse_t response = { 0 };
    response.userId = request->userId;
    response.userStatus = entry.status;
    response.userType = entry.type;
    response.pin.length = entry.pinLength;
    response.pin.ptr = entry.pinData;
    chipZclSendClusterDoorLockServerCommandGetPinResponse(context, &response); // Send response.
  } else {
    chipZclSendDefaultResponse(context, CHIP_ZCL_STATUS_INSUFFICIENT_SPACE);
  }
}

void chipZclClusterDoorLockServerCommandClearPinRequestHandler(const ChipZclCommandContext_t *context,
                                                                const ChipZclClusterDoorLockServerCommandClearPinRequest_t *request)
{
  chipAfCorePrintln("RX: ClearPin");

  ChipZclClusterDoorLockServerCommandClearPinResponse_t response = { 0 };

  if (request->userId < USER_TABLE_SIZE) {
    ChipAfDoorLockUser_t entry = { 0 };
    saveUserTableEntry(entry, request->userId);
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  chipZclSendClusterDoorLockServerCommandClearPinResponse(context, &response); // Send response.

  if (response.status == CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS) {
    // Send event notifications.
    ChipZclStringType_t userPin = { 0 }; //zero length string
    sendProgrammingEventToBoundDevice(context,
                                      request->userId,
                                      0,
                                      0,
                                      &userPin,
                                      CHIP_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_PIN_DELETED);
  }
}

void chipZclClusterDoorLockServerCommandSetWeekdayScheduleRequestHandler(const ChipZclCommandContext_t *context,
                                                                          const ChipZclClusterDoorLockServerCommandSetWeekdayScheduleRequest_t *request)
{
  chipAfCorePrintln("RX: SetWeekdaySchedule");

  ChipZclClusterDoorLockServerCommandSetWeekdayScheduleResponse_t response = { 0 };

  if ((request->userId < USER_TABLE_SIZE)
      && (request->scheduleId < SCHEDULE_TABLE_SIZE)) {
    ChipAfDoorLockScheduleEntry_t *entry = &schedule[request->scheduleId];
    entry->userId = request->userId;
    entry->daysMask = request->daysMask;
    entry->startHour = request->startHour;
    entry->startMinute = request->startMinute;
    entry->stopHour = request->endHour;
    entry->stopMinute = request->endMinute;
    printScheduleTable();
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  chipZclSendClusterDoorLockServerCommandSetWeekdayScheduleResponse(context,
                                                                     &response); // Send response.

  if (response.status == CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS) {
    ChipZclStringType_t userPin = { 0 }; //zero length string
    sendProgrammingEventToBoundDevice(context,
                                      request->userId,
                                      0,
                                      0,
                                      &userPin,
                                      CHIP_ZCL_DOOR_LOCK_PROGRAMMING_EVENT_CODE_UNKNOWN_OR_MFG_SPECIFIC);
  }
}

void chipZclClusterDoorLockServerCommandGetWeekdayScheduleRequestHandler(const ChipZclCommandContext_t *context,
                                                                          const ChipZclClusterDoorLockServerCommandGetWeekdayScheduleRequest_t *request)
{
  chipAfCorePrintln("RX: GetWeekdaySchedule");

  ChipZclClusterDoorLockServerCommandGetWeekdayScheduleResponse_t response = { 0 };

  if ((request->userId < USER_TABLE_SIZE)
      && (request->scheduleId < SCHEDULE_TABLE_SIZE)) {
    ChipAfDoorLockScheduleEntry_t *entry = &schedule[request->scheduleId];
    response.scheduleId = request->scheduleId;
    response.userId = request->userId;
    response.daysMask = entry->daysMask;
    response.startHour = entry->startHour;
    response.startMinute = entry->startMinute;
    response.endHour = entry->stopHour;
    response.endMinute = entry->stopMinute;
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
  } else {
    response.status = CHIP_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
  }
  chipZclSendClusterDoorLockServerCommandGetWeekdayScheduleResponse(context,
                                                                     &response); // Send response.
}
