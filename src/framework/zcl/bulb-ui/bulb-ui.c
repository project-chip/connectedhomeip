/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#include PLATFORM_HEADER
#include CONFIGURATION_HEADER
#include CHIP_AF_API_STACK
#include CHIP_AF_API_BULB_PWM_DRIVER
#include CHIP_AF_API_CONNECTION_MANAGER
#include CHIP_AF_API_HAL
#include CHIP_AF_API_ZCL_CORE
#ifdef CHIP_AF_API_DEBUG_PRINT
  #include CHIP_AF_API_DEBUG_PRINT
#endif

#define BULB_UI_SHORT_REBOOT_TIMEOUT_MS  (1 * MILLISECOND_TICKS_PER_SECOND)

// When flash attribute storage is supported, level will be retained from state
// before poweron.  For now, if the device is set to power on with the bulb on,
// this value will be used for the default level.
#define DEFAULT_LEVEL                             0x20

// Number of times to blink and duration of blink to signal various network
// states
#define EZ_MODE_BLINK_NUMBER                      2
#define EZ_MODE_BLINK_TIME_MS                     200
#define EZ_MODE_BLINK_REPEAT_TIME_MS              (4 * MILLISECOND_TICKS_PER_SECOND)
#define NEW_BIND_BLINK_NUMBER                     3
#define NEW_BIND_BLINK_TIME_MS                    500
#define STACK_LEFT_BLINK_NUMBER                   5
#define STACK_LEFT_BLINK_TIME_MS                  100

#define DEFAULT_IDENTIFY_TIME_S      180

enum {
  REBOOT_EVENT_STATE_INITIAL_CHECK       = 0,
  REBOOT_EVENT_STATE_SHORT_REBOOT_TIME   = 1,
};

static void incrementShortRebootMonitor(void);
static void resetBindingsAndAttributes(void);
static void enableIdentify(void);
static void joinNetwork(void);
static void leaveNetwork(void);
static size_t getNumberOfUsedBinds(void);

ChipEventControl emBulbUiEzModeControl;
ChipEventControl emBulbUiEzBlinkControl;
ChipEventControl emBulbUiRebootControl;

static uint8_t rebootEventState;
static bool initiateEzModeOnAttach;
static size_t numBindingsStartEzMode = 0;
static ChipZclEndpointId_t endpoint = CHIP_AF_PLUGIN_BULB_UI_LIGHT_ENDPOINT;

void emBulbUiInitHandler(void)
{
  ChipZclEndpointIndex_t endpointIndex;
  rebootEventState = REBOOT_EVENT_STATE_INITIAL_CHECK;
  initiateEzModeOnAttach = false;

  endpointIndex = chipZclEndpointIdToIndex(CHIP_AF_PLUGIN_BULB_UI_LIGHT_ENDPOINT,
                                            &chipZclClusterOnOffServerSpec);
  if (endpointIndex == CHIP_ZCL_ENDPOINT_INDEX_NULL) {
    chipAfCorePrintln("####ERROR!! Invalid endpoint selected for bulb UI Light Endpoint option!  Bulb UI will not function properly!!\n");
    endpoint = chZclEndpointTable[0].endpointId;
  }

  chipEventControlSetActive(emBulbUiRebootControl);
}

// On boot, increment the counter token and perform any bulb initialization.
// Then, wait the short reboot timeout.  If the device power cycles before that
// amount of time passes, the counter will be incremented again, and so on
// until the user has power cycled enough times to cause the desired activity
// to take place.  If the reboot event does activate a second time, it means
// that the user is done short power cycling, so action should occur based on
// the following table:
//
// 0-2 reboots: No action
// 3-4 reboots: Begin EZ Mode commissioning
// 5-6 reboots: Enter identify mode
// 7-9 reboots: Reset the existing binding table
// 10+ reboots: Perform a network reset
void emBulbUiRebootHandler(void)
{
  uint32_t shortRebootMonitor;

  halCommonGetToken(&shortRebootMonitor, TOKEN_SHORT_REBOOT_MONITOR);

  switch (rebootEventState) {
    case REBOOT_EVENT_STATE_INITIAL_CHECK:
      chipAfDebugPrintln("Bulb UI event Reset Reason:  %x %2x %p %p",
                          halGetResetInfo(),
                          halGetExtendedResetInfo(),
                          halGetResetString(),
                          halGetExtendedResetString());

      incrementShortRebootMonitor();
      rebootEventState = REBOOT_EVENT_STATE_SHORT_REBOOT_TIME;
      chipEventControlSetDelayMS(emBulbUiRebootControl,
                                  BULB_UI_SHORT_REBOOT_TIMEOUT_MS);

      break;

    case REBOOT_EVENT_STATE_SHORT_REBOOT_TIME:
      chipEventControlSetInactive(emBulbUiRebootControl);
      chipAfCorePrintln("Short reboot timer ended, %d reboots detected",
                         shortRebootMonitor);

      if (shortRebootMonitor >= 10) {
        leaveNetwork();
        // Try again to join a network, it could be that we are detaching from
        // an old network and joining a new one.
        joinNetwork();
      } else if (shortRebootMonitor >= 7) {
        resetBindingsAndAttributes();
      } else if (shortRebootMonitor >= 5) {
        enableIdentify();
      } else if (shortRebootMonitor >= 3) {
        // If the user wants to enter EZ Mode and the device is already attached
        // to the network, do so immediately.  Otherwise, set the initEzMode flag
        // so that the device will enter ez mode on the next attach.
        if (chipNetworkStatus() == CHIP_JOINED_NETWORK_ATTACHED) {
          initiateEzModeOnAttach = false;
          chipEventControlSetActive(emBulbUiEzModeControl);
        } else {
          initiateEzModeOnAttach = true;
        }
      } else if (shortRebootMonitor == 1) {
        // If the status is no network, then also try to join one now since we
        // are just now booting up
        if (chipNetworkStatus() == CHIP_NO_NETWORK) {
          joinNetwork();
        }
      }

      // Once the short reboot sequence is complete, reset the short reboot count
      // to zero and return to normal operation.
      shortRebootMonitor = 0;
      halCommonSetToken(TOKEN_SHORT_REBOOT_MONITOR, &shortRebootMonitor);
      break;

    default:
      assert(0);
  }
}

void emBulbUiNetworkStatusHandler(ChipNetworkStatus newNetworkStatus,
                                  ChipNetworkStatus oldNetworkStatus,
                                  ChipJoinFailureReason reason)
{
  if ((newNetworkStatus == CHIP_JOINED_NETWORK_ATTACHED)
      && (initiateEzModeOnAttach)) {
    // Ez Mode should only start on the first attach, not every time the bulb
    // attaches / reattaches for the life of the bulb
    initiateEzModeOnAttach = false;
    chipEventControlSetDelayMS(emBulbUiEzModeControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else if ((newNetworkStatus == CHIP_NO_NETWORK)
             && (oldNetworkStatus == CHIP_JOINED_NETWORK_ATTACHED)) {
    chipAfCorePrintln("Lost network");
    halBulbPwmDriverLedBlink(STACK_LEFT_BLINK_NUMBER,
                             STACK_LEFT_BLINK_TIME_MS);
  }
}

// Before starting EZ Mode, the system will determine how many binds are
// present in the binding table (as there is no callback generated when a bind
// is created).  A call to chipZclStartEzMode will then initiate EZ mode
// operation, and the bulb will start to blink the EZ Mode search pattern.  The
// ezModeBlink event will then be used to poll the number of binds created
// every few seconds to see if any new entries have been generated.
void emBulbUiEzModeHandler(void)
{
  ChipStatus status;

  chipEventControlSetInactive(emBulbUiEzModeControl);

  status = chipZclStartEzMode();

  if (status == CHIP_SUCCESS) {
    chipAfCorePrintln("starting ez mode\n");
    numBindingsStartEzMode = getNumberOfUsedBinds();
    halBulbPwmDriverLedBlink(EZ_MODE_BLINK_NUMBER,
                             EZ_MODE_BLINK_TIME_MS);
    chipEventControlSetDelayMS(emBulbUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else {
    chipAfCorePrintln("Unable to start EZ mode: %d", status);
  }
}

void emBulbUiEzBlinkHandler(void)
{
  size_t numBindings;
  bool blinkForNewBind = false;

  numBindings = getNumberOfUsedBinds();

  if (numBindings != numBindingsStartEzMode) {
    chipAfCorePrintln("%d new bindings created",
                       numBindings - numBindingsStartEzMode);
    blinkForNewBind = true;

    numBindingsStartEzMode = numBindings;
    chipEventControlSetDelayMS(emBulbUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  }

  if (chipZclEzModeIsActive()) {
    if (blinkForNewBind) {
      halBulbPwmDriverLedBlink(NEW_BIND_BLINK_NUMBER,
                               NEW_BIND_BLINK_TIME_MS);
    } else {
      halBulbPwmDriverLedBlink(EZ_MODE_BLINK_NUMBER,
                               EZ_MODE_BLINK_TIME_MS);
    }
    chipEventControlSetDelayMS(emBulbUiEzBlinkControl,
                                EZ_MODE_BLINK_REPEAT_TIME_MS);
  } else {
    chipEventControlSetInactive(emBulbUiEzBlinkControl);
  }
}

static size_t getNumberOfUsedBinds(void)
{
  ChipZclBindingId_t numberOfBinds = 0;
  ChipZclBindingId_t currentBindIndex;
  ChipZclBindingEntry_t currentBind;

  for (currentBindIndex = 0;
       currentBindIndex < CHIP_ZCL_BINDING_TABLE_SIZE;
       currentBindIndex++) {
    // Check to see if the binding table entry is active
    if (chipZclGetBinding(currentBindIndex, &currentBind)) {
      numberOfBinds++;
    }
  }
  return numberOfBinds;
}

static void resetBindingsAndAttributes(void)
{
  chipAfCorePrintln("Clearing binding table and resetting all attributes\n");
  chipZclRemoveAllBindings();
  chipZclResetAttributes(endpoint);
}

static void incrementShortRebootMonitor(void)
{
  halCommonIncrementCounterToken(TOKEN_SHORT_REBOOT_MONITOR);
}

static void enableIdentify(void)
{
  uint16_t identifyTimeS = DEFAULT_IDENTIFY_TIME_S;
  ChipZclStatus_t status;
  ChipZclEndpointId_t endpoint;
  ChipZclEndpointIndex_t i;

  chipAfCorePrintln("bulb-ui: identify mode enabled for %d seconds", identifyTimeS);

  for (i = 0; i < chZclEndpointCount; i++) {
    endpoint = chipZclEndpointIndexToId(i,
                                         &chipZclClusterIdentifyServerSpec);
    if (endpoint != CHIP_ZCL_ENDPOINT_NULL) {
      status = chipZclWriteAttribute(endpoint,
                                      &chipZclClusterIdentifyServerSpec,
                                      CHIP_ZCL_CLUSTER_IDENTIFY_SERVER_ATTRIBUTE_IDENTIFY_TIME,
                                      &identifyTimeS,
                                      sizeof(identifyTimeS));
      if (status != CHIP_ZCL_STATUS_SUCCESS) {
        chipAfCorePrintln("End node UI unable to identify on endpoint %d!",
                           i);
      } else {
        chipAfCorePrintln("Identifying for %d seconds on endpoint %d",
                           identifyTimeS,
                           i);
      }
    }
  }

  return;
}

static void leaveNetwork(void)
{
  chipAfCorePrintln("Attempting to reset the network connection");
  chipConnectionManagerLeaveNetwork();
}

static void joinNetwork(void)
{
  chipAfCorePrintln("Attempting to join a network");
  chipConnectionManagerStartConnect();
}

void chipZclIdentifyServerStartIdentifyingCallback(uint16_t identifyTimeS)
{
  // This callback is called whenever the endpoint should identify itself.  The
  // identification procedure is application specific, and could be implemented
  // by blinking an LED, playing a sound, or displaying a message.

  chipAfCorePrintln("Identifying...");
  halBulbPwmDriverLedBlink(HAL_BULB_PWM_DRIVER_BLINK_FOREVER, 1000);
}

void chipZclIdentifyServerStopIdentifyingCallback(void)
{
  // This callback is called whenever the endpoint should stop identifying
  // itself.

  chipAfCorePrintln("Identify complete");
  halBulbPwmDriverLedOn(0);
}
