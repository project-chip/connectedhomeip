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
 * @brief Definitions for the WWAH Connectivity Manager plugin, which implements
 *        the WWAH rejoining algorithm as defined in the WWAH specifications.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "wwah-connectivity-manager.h"

#ifdef EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE
 #include "app/framework/plugin/trust-center-keepalive/trust-center-keepalive.h"
#endif // EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE

#ifdef EMBER_AF_PLUGIN_POLL_CONTROL_SERVER
 #include "app/framework/plugin/poll-control-server/poll-control-server.h"
#endif // EMBER_AF_PLUGIN_POLL_CONTROL_SERVER

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
 #include "app/framework/plugin/zll-commissioning-common/zll-commissioning-common.h"
#endif

//------------------------------------------------------------------------------
// Notes
/*
   This file implements the rejoin algorithm per the Zigbee WWAH Requirements
   and WWAH ZCL documents. The algorithm is shown below.
   1.  If either of the following conditions is met, skip to step 5:
   a) device is a Router
   b) device is a Non-Sleepy End Device
   c) device is an Sleepy End Device and current rejoin was caused by Poll
     Control check-in failure described above
   2.  Secure Rejoin on the Current channel first
   3.  Trust Center rejoin on current channel
   4.  Optional: Trust Center rejoin on preferred channel list
   5.  Trust Center Rejoin on all channels
   6.  If Trust Center rejoin succeeds, connectivity is restored. No more steps are
    executed.
   7.  Otherwise, device continues to use its previous network parameters and waits
    for X seconds or until additional triggers (example: user button push)
   a.  For Routers and Non-Sleepy End Devices, X seconds will be equal to the
      Keepalive cluster value or 24 hours if Keepalive cluster is not
      implemented.
   b.  For Sleepy End devices, if the WWAH rejoin algorithm is enabled then the
      wait will be determined by the WWAH rejoin algorithm. If the WWAH rejoin
      algorithm is NOT enabled then the wait will be 15 minutes.
   8.  Routers and Non-Sleepy End Devices shall go back to step 3
   9.  Optional:  Secure Rejoins on preferred channel list
   10. Secure Rejoin on all channels
   11. If Secure Rejoin succeeds, parent connectivity is restored.
   a.  Hub connectivity may be tested at this point.
   b.  No more steps are executed.
   12. Otherwise, device sleeps for X seconds or until additional triggers
    (example:  user button push)
   a.  Routers and Non-Sleepy End Devices should not reach this point (return to
      step 3 after step 8).
   b.  For Sleepy End devices, if the WWAH rejoin algorithm is enabled then the
      wait will be determined by the WWAH rejoin algorithm. If the WWAH rejoin
      algorithm is NOT enabled then the wait will be 15 minutes.
   13. Goto step 1
 */

//------------------------------------------------------------------------------
// Globals

enum {
  CONNECTION_LOST_TO_PARENT                             = 0x01,
  CONNECTION_LOST_TO_HUB_DUE_TO_TC_KEEPALIVE            = 0x02,
  CONNECTION_LOST_TO_HUB_DUE_TO_POLL_CONTROL_CHECKIN    = 0x04
};

static uint8_t wwahConnectionErrorsBitmask = 0;

enum {
  STATE_NONE,
  STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL,
  STATE_TC_REJOIN_ON_CURRENT_CHANNEL,
  STATE_TC_REJOIN_ON_PREFERRED_CHANNELS,
  STATE_TC_REJOIN_ON_ALL_CHANNELS,
  STATE_WAIT_FOR_TRIGGER,
  STATE_SECURE_REJOIN_ON_PREFERRED_CHANNELS,
  STATE_SECURE_REJOIN_ON_ALL_CHANNELS,
  STATE_WAIT_FOR_SECOND_TRIGGER
};

static char* wwahStateStrings[] = {
  "None",
  "Secure Rejoin on Current Channel",
  "TC Rejoin on Current Channel",
  "TC Rejoin on Preferred Channels",
  "TC Rejoin on All Channels",
  "Wait for Trigger",
  "Secure Rejoin on Preferred Channels",
  "Secure Rejoin on All Channels",
  "Wait for Second Trigger",
};

#ifdef EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_USE_PREFERRED_CHANNEL_MASK
 #define STATE_AFTER_TC_REJOIN_ON_CURRENT_CHANNEL \
  STATE_TC_REJOIN_ON_PREFERRED_CHANNELS
 #define STATE_AFTER_WAIT_FOR_TRIGGER \
  STATE_SECURE_REJOIN_ON_PREFERRED_CHANNELS
#else
 #define STATE_AFTER_TC_REJOIN_ON_CURRENT_CHANNEL \
  STATE_TC_REJOIN_ON_ALL_CHANNELS
 #define STATE_AFTER_WAIT_FOR_TRIGGER \
  STATE_SECURE_REJOIN_ON_ALL_CHANNELS
#endif

static uint8_t wwahConnectionRecoveryState = STATE_NONE;

EmberEventControl emberAfPluginWwahConnectivityManagerStateTransitionEventControl;
EmberEventControl emberAfPluginWwahConnectivityManagerLongUptimeEventControl;
EmberEventControl emberAfPluginWwahConnectivityManagerFastRejoinEventControl;
EmberEventControl emberAfPluginWwahConnectivityManagerStateMachineEventControl;
// the following event is handled independent of the state machine
EmberEventControl emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl;

#ifdef EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE
 #define WAIT_TIME_BASE_SEC   emAfPluginTrustCenterKeepaliveGetBaseTimeSeconds()
 #define WAIT_TIME_JITTER_SEC emAfPluginTrustCenterKeepaliveGetJitterTimeSeconds()
#else // EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE
// From Zigbee WWAH Requirements doc:
// For Routers and Non-Sleepy End Devices, X seconds will be equal to the
// Keepalive cluster value or 24 hours if Keepalive cluster is not implemented
 #define WAIT_TIME_BASE_SEC   (24 * 60 * 60)
 #define WAIT_TIME_JITTER_SEC (60)
#endif // EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
  #define ZLL_RECOVERY_RETRY_DEFAULT      (1000)     // 1000 ms
#endif

#define BAD_PARENT_RECOVERY_RETRY_DEFAULT (24 * 60)  // 24 hours in min

#ifdef EMBER_TEST
  #ifdef EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_LONG_UPTIME_THRESHOLD
    #undef EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_LONG_UPTIME_THRESHOLD
  #endif // EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_LONG_UPTIME_THRESHOLD
  #define EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_LONG_UPTIME_THRESHOLD 10  // min
#endif // EMBER_TEST

// State information
static bool issuedRejoin = false;
static bool inFastRejoin = false;
static bool fastRejoinCompleted = false;
static bool backoffBetweenFastRejoinsCompleted = false;
static uint16_t numConsecutiveFastRejoinsCompleted = 0;

/*
 * parentRssiCheckStats keeps stats of how many times we have checked
 * the parent connectivity since the last NETWORK_UP or badParentRecovery enabled event
 */
static uint32_t parentRssiCheckStats = 0;

/*
 * attemptedParentRejoinStats keeps general stats about the number of parent rejoin
 * attempts becasue of bad parent connectivity.
 * This number is reset everytime badParentRecovery is disabled/enabled.
 */
static uint16_t attemptedParentRejoinStats = 0;

/*
 * attempt parent rejoin for disconnected parent once every badParentRecoveryRetry period
 * the effect especially kicks in if rejoinAlgoritm itself is not enabled
 */
static bool parentRecoveryRequired = false;

// // Configurable parameters
static EmberAfConnectionManagerTokenStruct connectionManagerTokenStruct;
static uint16_t badParentRecoveryRetryPeriod = BAD_PARENT_RECOVERY_RETRY_DEFAULT;//24 hours by default

// WWAH server endpoint as conveyed via wwah-server-silabs plugin during init.
static uint8_t wwahServerEndpoint = 0;

//------------------------------------------------------------------------------
// Prototypes

static uint8_t getNextState(void);
static uint32_t getTriggerStateWaitTimeMs(void);
static uint32_t calculateWaitTimeForNextStateMs(void);
static void wwahRunConnectivityRecoveryStateMachine(void);
static void clearState(void);
static void rejoinOnChannelMask(uint32_t channelMask, bool secureRejoin);
static void setTrustCenterConnectivity();

#define rejoinOnCurrentChannel(secure)  rejoinOnChannelMask(0, secure)
#define rejoinOnAllChannels(secure) \
  rejoinOnChannelMask(EMBER_ALL_802_15_4_CHANNELS_MASK, secure)
#ifdef EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_USE_PREFERRED_CHANNEL_MASK
 #define rejoinOnPreferredChannels(secure) \
  rejoinOnChannelMask(EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_PREFERRED_CHANNEL_MASK, secure)
#else // EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_USE_PREFERRED_CHANNEL_MASK
 #define rejoinOnPreferredChannels(secure)
#endif // EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_USE_PREFERRED_CHANNEL_MASK

//------------------------------------------------------------------------------
// Implemented callbacks

void emberAfPluginWwahConnectivityManagerInitCallback(uint8_t endpoint)
{
  //Restore connectivity manager state
  halCommonGetToken(&connectionManagerTokenStruct,
                    TOKEN_PLUGIN_CONNECTION_MANAGER_STATE);

  // Restore WWAH rejoin algorithm enabled attribute
  bool rejoinAlgorithmEnabled = (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE);
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_WWAH_REJOIN_ENABLED_ATTRIBUTE_ID,
                                      "WWAH rejoin",
                                      (uint8_t *)&rejoinAlgorithmEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // Restore WWAH bad parent recovery enabled attribute
  bool badParentRecoveryEnabled = (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY);
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_WWAH_BAD_PARENT_RECOVERY_ENABLED_ATTRIBUTE_ID,
                                      "WWAH bad parent",
                                      (uint8_t *)&badParentRecoveryEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // Arm the long uptime timer
  emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerLongUptimeEventControl,
                                       EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_LONG_UPTIME_THRESHOLD * 60000); // convert minutes to ms

  bool periodicRouterCheckinEnabled = (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE);
  emberWriteWwahServerSilabsAttribute(endpoint,
                                      ZCL_SL_ROUTER_CHECKIN_ENABLED_ATTRIBUTE_ID,
                                      "router checkin enabled",
                                      (uint8_t *)&periodicRouterCheckinEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  // Save WWAH endpoint for use by Bad Parent Recovery CLI command.
  wwahServerEndpoint = endpoint;
}

void emberAfPluginWwahConnectivityManagerStackStatusCallback(EmberStatus status)
{
  if (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE) {
    issuedRejoin = false;
    if (status == EMBER_NETWORK_UP) {
      clearState();
    }
  }
  // The BadParentRecovery timer should anyways reset in case of attempted rejoins
  if (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY) {
    if (status == EMBER_NETWORK_UP
        || status == EMBER_NETWORK_DOWN /* we need to reschule another rejoin attempt*/) {
      emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl);
      emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl,
                                           badParentRecoveryRetryPeriod * MILLISECOND_TICKS_PER_MINUTE);
      parentRecoveryRequired = false;
      parentRssiCheckStats = 0;
    }
  }
}

bool emberAfPluginTrustCenterKeepaliveTimeoutCallback(void)
{
  emberSetHubConnectivity(false);
  setTrustCenterConnectivity();

  bool keepAliveFailure = wwahConnectionErrorsBitmask
                          & CONNECTION_LOST_TO_HUB_DUE_TO_TC_KEEPALIVE;

  if (!(connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE)) {
    return false;
  } else if (!keepAliveFailure) {
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: lost connection to %s "
                                "due to %s failures",
                                "hub",
                                "trust center keepalive");

    wwahConnectionErrorsBitmask |= CONNECTION_LOST_TO_HUB_DUE_TO_TC_KEEPALIVE;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
    if (emberAfZllTouchLinkInProgress()) {
      // Using ZLL for connectivity recovery
      emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerStateMachineEventControl,
                                           ZLL_RECOVERY_RETRY_DEFAULT);
      return true;
    }
#endif

    // Kick the state machine only if we're not already trying to restore
    // connectivity
    if (wwahConnectionRecoveryState == STATE_NONE) {
      wwahConnectionRecoveryState = getNextState();
      wwahRunConnectivityRecoveryStateMachine();
    }
  }

  return true;
}

bool emberAfPluginTrustCenterKeepaliveServerlessIsSupportedCallback(void)
{
  return true;
}

#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE)
bool emberAfTrustCenterKeepaliveOverwriteDefaultTimingCallback(uint16_t *baseTimeSeconds, uint16_t *jitterTimeSeconds)
{
  if (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE) {
    // Set up TC Keep Alive timing based on Periodic Router Check-In requirements.
    uint16_t checkinIntervalSeconds;
    halCommonGetToken(&checkinIntervalSeconds,
                      TOKEN_PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_INTERVAL);

    // Note, that periodic router check in supposes no more retries after checking in,
    // meaning that TC Keep Alive has to reach its failure limit (3 consecutive failures)
    // by that time.
    *baseTimeSeconds = (uint16_t)(checkinIntervalSeconds / EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE_FAILURE_LIMIT);
    // Also note, that while Periodic Router Check In does not assume a random jitter
    // added to check-in interval, however, TC Keep Alive code adds that (0-7 seconds).
    *jitterTimeSeconds = 0;

    return true;
  }

  return false;
}
#endif

bool emberAfTrustCenterKeepaliveServerlessIsEnabledCallback(void)
{
  return
    (connectionManagerTokenStruct.connectionManagerFlags
     & PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE)
    ? true : false;
}

EmberStatus emberAfPluginWwahConnectivityManagerEnablePeriodicRouterCheckIns(uint16_t checkInInterval)
{
#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE) && !defined(EMBER_AF_HAS_SLEEPY_NETWORK)
  connectionManagerTokenStruct.connectionManagerFlags |= PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE;

  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);

  halCommonSetToken(TOKEN_PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_INTERVAL,
                    &checkInInterval);

  bool periodicRouterCheckinEnabled = (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE);
  emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                      ZCL_SL_ROUTER_CHECKIN_ENABLED_ATTRIBUTE_ID,
                                      "WWAH rejoin",
                                      (uint8_t *)&periodicRouterCheckinEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  emberAfPluginTrustCenterKeepaliveEnable();
  return EMBER_SUCCESS;
#else
  emberAfSlWwahClusterPrintln("WWAH Periodic Router Checkin is not implemented.");
  return EMBER_ERR_FATAL;
#endif
}

EmberStatus emberAfPluginWwahConnectivityManagerDisablePeriodicRouterCheckIns(void)
{
#if defined(EMBER_AF_PLUGIN_TRUST_CENTER_KEEPALIVE) && !defined(EMBER_AF_HAS_SLEEPY_NETWORK)
  connectionManagerTokenStruct.connectionManagerFlags &= ~PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE;

  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);

  // Do not disable Trust Center Keep Alive if the TC has the Trust Center Server
  // Cluster available. Doing this so that we don't unintentionally disable both
  // WWAH Periodic Router Check In *and* Trust Center Keep Alive at the same time.
  if (!emAfPluginTrustCenterKeepaliveTcHasServerCluster()) {
    emberAfPluginTrustCenterKeepaliveDisable();
  }

  bool periodicRouterCheckinEnabled = (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_PERIODIC_ROUTER_CHECKIN_ENABLED_STATE);
  emberWriteWwahServerSilabsAttribute(wwahServerEndpoint,
                                      ZCL_SL_ROUTER_CHECKIN_ENABLED_ATTRIBUTE_ID,
                                      "WWAH rejoin",
                                      (uint8_t *)&periodicRouterCheckinEnabled,
                                      ZCL_BOOLEAN_ATTRIBUTE_TYPE);

  return EMBER_SUCCESS;
#else
  emberAfSlWwahClusterPrintln("WWAH Periodic Router Checkin is not implemented.");
  return EMBER_ERR_FATAL;
#endif
}

void emberAfPluginPollControlServerCheckInTimeoutCallback(void)
{
  bool pollControlFailure = wwahConnectionErrorsBitmask
                            & CONNECTION_LOST_TO_HUB_DUE_TO_POLL_CONTROL_CHECKIN;

  if ((connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE)
      && (!pollControlFailure)) {
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: lost connection to %s "
                                "due to %s failures",
                                "hub",
                                "poll control checkin");

    wwahConnectionErrorsBitmask |=
      CONNECTION_LOST_TO_HUB_DUE_TO_POLL_CONTROL_CHECKIN;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
    if (emberAfZllTouchLinkInProgress()) {
      // Using ZLL for connectivity recovery
      emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerStateMachineEventControl,
                                           ZLL_RECOVERY_RETRY_DEFAULT);
      return;
    }
#endif

    // Kick the state machine only if we're not already trying to restore
    // connectivity
    if (wwahConnectionRecoveryState == STATE_NONE) {
      wwahConnectionRecoveryState = getNextState();
      wwahRunConnectivityRecoveryStateMachine();
    }
  } else if ((connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY)
             && !(connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE)) {
    // require a parent recovery rejoin and trigger the event to rejoin immediately
    parentRecoveryRequired = true;
    emberAfNetworkEventControlSetActive(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl);
  } else {
    // The connectivity manager isn't enabled, and we aren't performing bad parent recovery,
    // do nothing
  }
  return;
}

static void setTrustCenterConnectivity()
{
  if (emberIsHubConnected()) {
    connectionManagerTokenStruct.connectionManagerFlags |= PLUGIN_WWAH_CONNECTION_MANAGER_TRUST_CENTER_CONNECTIVITY;
  } else {
    connectionManagerTokenStruct.connectionManagerFlags &= ~PLUGIN_WWAH_CONNECTION_MANAGER_TRUST_CENTER_CONNECTIVITY;
  }
  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);
}

void emberAfPluginTrustCenterKeepaliveConnectivityEstablishedCallback(void)
{
  emberSetHubConnectivity(true);
  setTrustCenterConnectivity();
}

bool emberAfPluginEndDeviceSupportLostParentConnectivityCallback(void)
{
  bool pollControlFailure = wwahConnectionErrorsBitmask
                            & CONNECTION_LOST_TO_PARENT;

  if ((connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE)
      && (!pollControlFailure)) {
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: lost connection to %s",
                                "parent");

    inFastRejoin = true;
    emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerFastRejoinEventControl,
                                         connectionManagerTokenStruct.fastRejoinTimeoutSec * MILLISECOND_TICKS_PER_SECOND);

    wwahConnectionErrorsBitmask |= CONNECTION_LOST_TO_PARENT;

#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
    if (emberAfZllTouchLinkInProgress()) {
      // Using ZLL for connectivity recovery
      emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerStateMachineEventControl,
                                           ZLL_RECOVERY_RETRY_DEFAULT);
      return true;
    }
#endif

    // Kick the state machine only if we're not already trying to restore
    // connectivity
    if (wwahConnectionRecoveryState == STATE_NONE) {
      wwahConnectionRecoveryState = getNextState();
      wwahRunConnectivityRecoveryStateMachine();
    }
  } else if ((connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY) && !(connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE)) {
    parentRecoveryRequired = true;
  } else {
    // The connectivity manager isn't enabled, so let fall back
    // to the non WWAH move code.
    return false;
  }
  return true;
}

bool emberAfPluginEndDeviceSupportPreNetworkMoveCallback(void)
{
  // Various plugins try to issue Network Moves when certain events happen. They
  // call a callback which is defined by end-device-move.c, who begins a process
  // of network rejoins. To stop it from issuing rejoins, we simply return true
  // here if either the rejoin algorithm or bad parent recovery are eanbled.
  // There is nothing else we need to catch or do here since our other
  // connectivity callbacks take care of it.
  return ((connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE) || (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY));
}

//------------------------------------------------------------------------------
// Event Handlers

void emberAfPluginWwahConnectivityManagerStateTransitionEventHandler(void)
{
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerStateTransitionEventControl);
  wwahConnectionRecoveryState = getNextState();
  wwahRunConnectivityRecoveryStateMachine();
}

void emberAfPluginWwahConnectivityManagerLongUptimeEventHandler(void)
{
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerLongUptimeEventControl);
  emberSetLongUpTime(true);
}

void emberAfPluginWwahConnectivityManagerFastRejoinEventHandler(void)
{
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerFastRejoinEventControl);
  numConsecutiveFastRejoinsCompleted++;
  inFastRejoin = false;
  fastRejoinCompleted = true;
}

void emberAfPluginWwahConnectivityManagerStateMachineEventHandler(void)
{
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerStateMachineEventControl);
#ifdef EMBER_AF_PLUGIN_ZLL_COMMISSIONING_COMMON
  if (!emberAfZllTouchLinkInProgress()) {
    // If the touch link fails, we need to recover the connectivity
    if ((wwahConnectionRecoveryState == STATE_NONE)
        && (emberAfNetworkState() != EMBER_JOINED_NETWORK)) {
      wwahConnectionRecoveryState = getNextState();
      wwahRunConnectivityRecoveryStateMachine();
    }
  } else {
    // Touch link is still in progress, check it next time
    emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerStateMachineEventControl,
                                         ZLL_RECOVERY_RETRY_DEFAULT);
  }
#endif
}

//this is handled independent of the state machine
void emberAfPluginWwahConnectivityManagerBadParentRecoveryEventHandler(void)
{
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl);
  if (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY) {
    EmberStatus status;
    EmberNodeType nodeType;
    status = emberGetNodeType(&nodeType);

    if ((status == EMBER_SUCCESS)
        && (nodeType != EMBER_ROUTER)) {
      if (wwahConnectionRecoveryState == STATE_NONE) {
        ++parentRssiCheckStats;
        if (parentRecoveryRequired) {
          rejoinOnAllChannels(true);
          ++attemptedParentRejoinStats;
          return;
        } else {
          EmberBeaconClassificationParams param;
          emberGetBeaconClassificationParams(&param);
          if (param.beaconClassificationMask
              & BAD_PARENT_CONNECTIVITY ) {
            rejoinOnCurrentChannel(true);
            ++attemptedParentRejoinStats;
            return;
          }
        }
      }
    }
    emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl,
                                         badParentRecoveryRetryPeriod * MILLISECOND_TICKS_PER_MINUTE);
  }
}

//------------------------------------------------------------------------------
// Internal functions
#if !defined(EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_ALLOW_REJOINS_WITH_WELL_KNOWN_LINK_KEY)
static bool checkForWellKnownTrustCenterLinkKey(void)
{
  EmberKeyStruct keyStruct;
  EmberStatus status = emberGetKey(EMBER_TRUST_CENTER_LINK_KEY, &keyStruct);

  const EmberKeyData smartEnergyWellKnownTestKey = SE_SECURITY_TEST_LINK_KEY;
  const EmberKeyData zigbeeAlliance09Key = ZIGBEE_PROFILE_INTEROPERABILITY_LINK_KEY;

  if (status != EMBER_SUCCESS) {
    // Assume by default we have a well-known key if we failed to retrieve it.
    // This will prevent soliciting a TC rejoin that might expose the network
    // key such that a passive attacker can obtain the key.  Better to be
    // conservative in this circumstance.
    return true;
  }

  if ((0 == MEMCOMPARE(emberKeyContents(&(keyStruct.key)),
                       emberKeyContents(&(smartEnergyWellKnownTestKey)),
                       EMBER_ENCRYPTION_KEY_SIZE))
      || (0 == MEMCOMPARE(emberKeyContents(&(keyStruct.key)),
                          emberKeyContents(&(zigbeeAlliance09Key)),
                          EMBER_ENCRYPTION_KEY_SIZE))) {
    return true;
  }

  return false;
}
#endif

static uint8_t getNextState(void)
{
  uint8_t nextState;
  EmberStatus status;
  EmberNodeType nodeType;
  bool skipToTcRejoinOnAllChannels = false;

  status = emberGetNodeType(&nodeType);
  if (EMBER_SUCCESS != status) {
    return STATE_NONE;
  }

  // If we're a router, non sleepy, or sleepy with a failed poll control
  // checkin, we skip directly to TC Rejoin on All Channels since we know TC
  // connectivity is lost
  if ((nodeType == EMBER_ROUTER)
      || (nodeType == EMBER_END_DEVICE)
      || (wwahConnectionErrorsBitmask
          & CONNECTION_LOST_TO_HUB_DUE_TO_POLL_CONTROL_CHECKIN)) {
    skipToTcRejoinOnAllChannels = true;
  }

  switch (wwahConnectionRecoveryState) {
    case STATE_NONE:
    case STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL:
      nextState = skipToTcRejoinOnAllChannels ? STATE_TC_REJOIN_ON_ALL_CHANNELS
                  : (wwahConnectionRecoveryState + 1);
      break;

    case STATE_TC_REJOIN_ON_CURRENT_CHANNEL:
      nextState = skipToTcRejoinOnAllChannels ? STATE_TC_REJOIN_ON_ALL_CHANNELS
                  : STATE_AFTER_TC_REJOIN_ON_CURRENT_CHANNEL;
      break;

    case STATE_TC_REJOIN_ON_PREFERRED_CHANNELS:
      nextState = STATE_TC_REJOIN_ON_ALL_CHANNELS;
      break;

    case STATE_TC_REJOIN_ON_ALL_CHANNELS:
      nextState = STATE_WAIT_FOR_TRIGGER;
      break;

    case STATE_WAIT_FOR_TRIGGER:
      // Routers and Non-Sleepy End Devices shall go back to step 3
      if ((nodeType == EMBER_ROUTER) || (nodeType == EMBER_END_DEVICE)) {
        nextState = STATE_TC_REJOIN_ON_CURRENT_CHANNEL;
      } else {
        nextState = STATE_AFTER_WAIT_FOR_TRIGGER;
      }
      break;

    case STATE_SECURE_REJOIN_ON_PREFERRED_CHANNELS:
      nextState = STATE_SECURE_REJOIN_ON_ALL_CHANNELS;
      break;

    case STATE_SECURE_REJOIN_ON_ALL_CHANNELS:
      nextState = STATE_WAIT_FOR_SECOND_TRIGGER;
      break;

    case STATE_WAIT_FOR_SECOND_TRIGGER:
      nextState = skipToTcRejoinOnAllChannels ? STATE_TC_REJOIN_ON_ALL_CHANNELS
                  : STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL;
      break;

    default:
      nextState = STATE_NONE;
      break;
  }

  return nextState;
}

// In two steps of the rejoin algorithm, we delay a while before continuing
// rejoins. This function gets that conditional delay amount
static uint32_t getTriggerStateWaitTimeMs(void)
{
  uint32_t waitTimeMs;
  uint32_t waitTimeBaseSec;
  uint16_t waitTimeJitterSec;
  EmberStatus status;
  EmberNodeType nodeType;

  status = emberGetNodeType(&nodeType);
  if (EMBER_SUCCESS != status) {
    return STATE_NONE;
  }

  // For Routers and Non-Sleepy End Devices, X seconds will be equal to the
  // Keepalive cluster value or 24 hours if Keepalive cluster is not implemented
  // For Sleepy End devices, if the WWAH rejoin algorithm is enabled then the
  // wait will be determined by the WWAH rejoin algorithm. If the WWAH rejoin
  // algorithm is NOT enabled then the wait will be 15 minutes.
  if ((wwahConnectionRecoveryState == STATE_WAIT_FOR_TRIGGER)
      || (wwahConnectionRecoveryState == STATE_WAIT_FOR_SECOND_TRIGGER)) {
    waitTimeBaseSec = WAIT_TIME_BASE_SEC;
    waitTimeJitterSec = WAIT_TIME_JITTER_SEC;
  } else {
    waitTimeBaseSec = EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_NON_FAST_REJOIN_BACKOFF_MIN * 60;
    waitTimeJitterSec = EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_NON_FAST_REJOIN_BACKOFF_JITTER_SEC;
  }

  // Setup wait time, pay special attention to jitter as that can be zero.
  waitTimeMs = (waitTimeBaseSec * MILLISECOND_TICKS_PER_SECOND);
  waitTimeMs += (waitTimeJitterSec)
                ? ((emberGetPseudoRandomNumber() % waitTimeJitterSec)
                   * MILLISECOND_TICKS_PER_SECOND)
                : 0;

  return waitTimeMs;
}

static void rejoinOnChannelMask(uint32_t channelMask, bool secureRejoin)
{
  EmberStatus status;
#if !defined(EMBER_AF_PLUGIN_END_DEVICE_SUPPORT_ALLOW_REJOINS_WITH_WELL_KNOWN_LINK_KEY)
  if (checkForWellKnownTrustCenterLinkKey() && !secureRejoin) {
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: Cancelling TC rejoin attempt with well known key");
    return;
  }
#endif

  status = emberFindAndRejoinNetworkWithReason(secureRejoin,
                                               channelMask,
                                               EMBER_AF_REJOIN_DUE_TO_WWAH_CONNECTIVITY_MANAGER);

  issuedRejoin = true;

  emberAfSlWwahClusterPrint("WWAH Connectivity Mgr: issued %s rejoin on ",
                            secureRejoin ? "secure" : "insecure");
  if (channelMask == 0) {
    emberAfSlWwahClusterPrint("current channel: ");
  } else {
    emberAfSlWwahClusterPrint("%s channels (mask 0x%4X): ",
                              (EMBER_ALL_802_15_4_CHANNELS_MASK == channelMask)
                              ? "all" : "preferred",
                              channelMask);
  }
  emberAfSlWwahClusterPrintln("0x%X", status);
}

static void transitionStateAfterDelay(uint32_t delayMs)
{
  emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: moving state after %d ms",
                              delayMs);

  emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerStateTransitionEventControl,
                                       delayMs);
}

static uint32_t calculateWaitTimeForNextStateMs(void)
{
  EmberStatus status;
  EmberNodeType nodeType;

  emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: calculating delay until "
                              "next state (inFastRejoin %s, "
                              "fastRejoinCompleted %s, "
                              "numConsecutiveFastRejoinsCompleted %d, "
                              "backoffBetweenFastRejoinsCompleted %s)",
                              inFastRejoin ? "Y" : "N",
                              fastRejoinCompleted ? "Y" : "N",
                              numConsecutiveFastRejoinsCompleted,
                              backoffBetweenFastRejoinsCompleted ? "Y" : "N");

  // If we just completed a fast rejoin backoff, we're in fast rejoin again
  if (backoffBetweenFastRejoinsCompleted) {
    inFastRejoin = true;
    emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerFastRejoinEventControl,
                                         connectionManagerTokenStruct.fastRejoinTimeoutSec * MILLISECOND_TICKS_PER_SECOND);
    backoffBetweenFastRejoinsCompleted = false;
  }

  status = emberGetNodeType(&nodeType);

  // The backoff retry algorithm is designed to help end devices recover and
  // reconnect to the network without issuing too many rejoins which would drain
  // battery. If we're a router or always-on end device, we rejoin at a faster
  // pace
  if ((status != EMBER_SUCCESS)
      || (nodeType == EMBER_ROUTER)
      || (nodeType == EMBER_END_DEVICE)) {
    return getTriggerStateWaitTimeMs();
  }

  // At this point on, we're a sleepy
#ifdef EMBER_AF_HAS_SLEEPY_NETWORK
  uint32_t waitTimeMs;
  uint32_t maxWaitTimeMs;

  if (inFastRejoin) {
    waitTimeMs = connectionManagerTokenStruct.durationBetweenEachRejoinSec * MILLISECOND_TICKS_PER_SECOND;
  } else if (fastRejoinCompleted) {
    if (connectionManagerTokenStruct.fastRejoinFirstBackoffTimeSec == 0) {
      return 0; // no backoff/retry
    } else {
      waitTimeMs = connectionManagerTokenStruct.fastRejoinFirstBackoffTimeSec * MILLISECOND_TICKS_PER_SECOND;

      // In between each fast rejoin iteration, we delay by a constantly
      // doubling connectionManagerTokenStruct.fastRejoinFirstBackoffTimeSec seconds
      // When we've done connectionManagerTokenStruct.maxBackoffIter number of fast rejoins, the
      // backoff amount is reset back to the first backoff time
      if (connectionManagerTokenStruct.maxBackoffIter + 1 == numConsecutiveFastRejoinsCompleted) {
        numConsecutiveFastRejoinsCompleted = 1;
      }

      // each backoff between fast rejoins is double the last
      // as numConsecutiveFastRejoinsCompleted will increase 1 after each fast rejoin completed
      if (numConsecutiveFastRejoinsCompleted > 0) {
        waitTimeMs = waitTimeMs << (numConsecutiveFastRejoinsCompleted - 1);
      }

      // Ensure that the backoff does not exceed connectionManagerTokenStruct.maxBackoffTimeSec
      maxWaitTimeMs = connectionManagerTokenStruct.maxBackoffTimeSec * MILLISECOND_TICKS_PER_SECOND;
      if (waitTimeMs > maxWaitTimeMs) {
        waitTimeMs = maxWaitTimeMs;
      }

      // Update state for the next fast rejoin attempt
      fastRejoinCompleted = false;
      // The following is acted upon when the state machine next runs
      backoffBetweenFastRejoinsCompleted = true;
    }
  } else {
    waitTimeMs = (EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_NON_FAST_REJOIN_BACKOFF_MIN
                  * MILLISECOND_TICKS_PER_MINUTE);
    waitTimeMs += ((emberGetPseudoRandomNumber() % EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_NON_FAST_REJOIN_BACKOFF_JITTER_SEC)
                   * MILLISECOND_TICKS_PER_SECOND);
  }

  return waitTimeMs;
#else
  return 0;
#endif // EMBER_AF_HAS_SLEEPY_NETWORK
}

static void wwahRunConnectivityRecoveryStateMachine(void)
{
  uint32_t waitTimeMs;

  // If the algorithm is disabled or if connectivity is totally restored to both
  // hub and parent, reset state and exit
  if (!(connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE) || (0 == wwahConnectionErrorsBitmask)) {
    wwahConnectionRecoveryState = STATE_NONE;
    return;
  }

  emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: running state %s "
                              "(connection error mask 0x%X, fast rejoin: %s)",
                              wwahStateStrings[wwahConnectionRecoveryState],
                              wwahConnectionErrorsBitmask,
                              inFastRejoin ? "Y" : "N");

  switch (wwahConnectionRecoveryState) {
    case STATE_NONE:
      break;

    case STATE_SECURE_REJOIN_ON_CURRENT_CHANNEL:
      rejoinOnCurrentChannel(true); // secure rejoin
      break;

    case STATE_TC_REJOIN_ON_CURRENT_CHANNEL:
      rejoinOnCurrentChannel(false); // insecure rejoin
      break;

    case STATE_TC_REJOIN_ON_PREFERRED_CHANNELS:
      rejoinOnPreferredChannels(false); // insecure rejoin
      break;

    case STATE_TC_REJOIN_ON_ALL_CHANNELS:
      rejoinOnAllChannels(false); // insecure rejoin
      break;

    case STATE_WAIT_FOR_TRIGGER:
#ifdef EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_ALLOW_TRIGGER_STATE_SEND_REJOIN
      // EMZIGBEE-4387: This rejoin attempt is to accommodate the discrepancy between
      // "17-01066-029-Zigbee_WWAH_Requirements" and "WWAH Test Cases - 191002"
      // for the trigger waiting states. The "16-02828-005-PRO-BDB-Specification-v1.0-Errata"
      // removes the trigger waiting states for the rejoin algorithm. If the future
      // "Zigbee_WWAH_Requirements" adopts the new rejoin algorithm we should remove this.
      rejoinOnAllChannels(false); // insecure rejoin
#endif
      break;

    case STATE_SECURE_REJOIN_ON_PREFERRED_CHANNELS:
      rejoinOnPreferredChannels(true); // secure rejoin
      break;

    case STATE_SECURE_REJOIN_ON_ALL_CHANNELS:
      rejoinOnAllChannels(true);  // secure rejoin
      break;

    case STATE_WAIT_FOR_SECOND_TRIGGER:
#ifdef EMBER_AF_PLUGIN_WWAH_CONNECTIVITY_MANAGER_ALLOW_TRIGGER_STATE_SEND_REJOIN
      // EMZIGBEE-4387: This rejoin attempt is to accommodate the discrepancy between
      // "17-01066-029-Zigbee_WWAH_Requirements" and "WWAH Test Cases - 191002"
      // for the trigger waiting states. The "16-02828-005-PRO-BDB-Specification-v1.0-Errata"
      // removes the trigger waiting states for the rejoin algorithm. If the future
      // "Zigbee_WWAH_Requirements" adopts the new rejoin algorithm we should remove this.
      rejoinOnAllChannels(true);  // secure rejoin
#endif
      break;

    default:
      break;
  }

  waitTimeMs = calculateWaitTimeForNextStateMs();
  if (waitTimeMs != 0) {
    transitionStateAfterDelay(waitTimeMs);
  }
}

void wwahConnectivityManagerPrintInfo(void)
{
  bool parentConnectivityFailure = wwahConnectionErrorsBitmask
                                   & CONNECTION_LOST_TO_PARENT;
  bool tcKeepAliveFailure = wwahConnectionErrorsBitmask
                            & CONNECTION_LOST_TO_HUB_DUE_TO_TC_KEEPALIVE;
  bool pollControlFailure = wwahConnectionErrorsBitmask
                            & CONNECTION_LOST_TO_HUB_DUE_TO_POLL_CONTROL_CHECKIN;
  bool hubConnectivityFailure = tcKeepAliveFailure || pollControlFailure;
  EmberBeaconClassificationParams param;
  emberGetBeaconClassificationParams(&param);

  emberAfSlWwahClusterPrintln("WWAH Connectivity Manager Info");
  emberAfSlWwahClusterPrintln("------------------------------------------------"
                              "--------------------");
  emberAfSlWwahClusterPrintln("Enabled:                         %s",
                              (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE) ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("Connection Error Mask:           0x%X",
                              wwahConnectionErrorsBitmask);
  emberAfSlWwahClusterPrintln("  Parent Connection Lost:        %s",
                              parentConnectivityFailure ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("  Trust Center Connectivity:     %s",
                              connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_TRUST_CENTER_CONNECTIVITY ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("  Hub Connection Lost:           %s",
                              hubConnectivityFailure ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("    TC Keepalive Failure:        %s",
                              tcKeepAliveFailure ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("    Poll Control Server Failure: %s",
                              pollControlFailure ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("State:                           %s",
                              wwahStateStrings[wwahConnectionRecoveryState]);
  emberAfSlWwahClusterPrintln("  Issued Rejoin:                 %s",
                              issuedRejoin ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("  In Fast Rejoin:                %s",
                              inFastRejoin ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("  Consecutive Fast Rejoins Done: %d",
                              numConsecutiveFastRejoinsCompleted);
  emberAfSlWwahClusterPrintln("  Long Uptime:                   %s",
                              emberIsUpTimeLong() ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("Bad Parent Recovery Enabled:     %s",
                              (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY) ? "Yes" : "No");
  emberAfSlWwahClusterPrintln("  Quality checks:                %d",
                              parentRssiCheckStats);
  emberAfSlWwahClusterPrintln("  Measurement period(Min):       %d",
                              badParentRecoveryRetryPeriod);
  emberAfSlWwahClusterPrintln("  Attempted rejoins:             %d",
                              attemptedParentRejoinStats);
  emberAfSlWwahClusterPrintln("minRssiForReceivingPackets:      %d",
                              param.minRssiForReceivingPkts);
}

void wwahConnectivityManagerShow(void)
{
  emberAfSlWwahClusterPrintln("Fast Rejoin Timeout:             %d s",
                              connectionManagerTokenStruct.fastRejoinTimeoutSec);
  emberAfSlWwahClusterPrintln("Duration Between Each Rejoin:    %d s",
                              connectionManagerTokenStruct.durationBetweenEachRejoinSec);
  emberAfSlWwahClusterPrintln("Fast Rejoin First Backoff Time:  %d s",
                              connectionManagerTokenStruct.fastRejoinFirstBackoffTimeSec);
  emberAfSlWwahClusterPrintln("Max Backoff Timeout:             %d s",
                              connectionManagerTokenStruct.maxBackoffTimeSec);
  emberAfSlWwahClusterPrintln("Max Backoff Iterations:          %d",
                              connectionManagerTokenStruct.maxBackoffIter);
}

static void clearState(void)
{
  wwahConnectionRecoveryState = STATE_NONE;
  wwahConnectionErrorsBitmask = 0;
  issuedRejoin = false;
  inFastRejoin = false;
  fastRejoinCompleted = false;
  backoffBetweenFastRejoinsCompleted = false;
  numConsecutiveFastRejoinsCompleted = 0;
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerStateTransitionEventControl);
  emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerFastRejoinEventControl);
}
//------------------------------------------------------------------------------
// Public APIs

bool emberAfPluginWwahConnectivityManagerIsRejoinAlgorithmEnabled(void)
{
  return connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE;
}

void emberAfPluginWwahConnectivityManagerEnableRejoinAlgorithm(uint8_t endpoint)
{
  if (!(connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE)) {
    parentRecoveryRequired = false;
    connectionManagerTokenStruct.connectionManagerFlags |= PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE;
    halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                      &connectionManagerTokenStruct);
    bool rejoinAlgorithmEnabled = connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE;
    emberWriteWwahServerSilabsAttribute(endpoint,
                                        ZCL_SL_WWAH_REJOIN_ENABLED_ATTRIBUTE_ID,
                                        "WWAH rejoin",
                                        (uint8_t *)&rejoinAlgorithmEnabled,
                                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  }
}

void emberAfPluginWwahConnectivityManagerDisableRejoinAlgorithm(uint8_t endpoint)
{
  if (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE) {
    connectionManagerTokenStruct.connectionManagerFlags &= ~PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE;
    halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                      &connectionManagerTokenStruct);
    clearState();
    bool rejoinAlgorithmEnabled = connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_ENABLED_STATE;
    emberWriteWwahServerSilabsAttribute(endpoint,
                                        ZCL_SL_WWAH_REJOIN_ENABLED_ATTRIBUTE_ID,
                                        "WWAH rejoin",
                                        (uint8_t *)&rejoinAlgorithmEnabled,
                                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);
  }
}

// Invoked by CLI command handler for Bad Parent Recovery.
void emberAfPluginWwahConnectivityManagerEnableBadParentRecovery(uint16_t badParentRejoinPeriod)
{
  emAfPluginWwahConnectivityManagerEnableBadParentRecovery(wwahServerEndpoint, badParentRejoinPeriod);
}

// Common handler for ZCL and CLI that conveys endpoint.
void emAfPluginWwahConnectivityManagerEnableBadParentRecovery(uint8_t endpoint, uint16_t badParentRejoinPeriod)
{
  if (!(connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY)) {
    parentRecoveryRequired = false;
    parentRssiCheckStats = 0;
    EmberStatus status;
    EmberNodeType nodeType;
    status = emberGetNodeType(&nodeType);

    if ((status == EMBER_SUCCESS)
        && (nodeType != EMBER_ROUTER)) {
      connectionManagerTokenStruct.connectionManagerFlags |= PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY;
      halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                        &connectionManagerTokenStruct);
      uint8_t badParentRecoveryEnabled = 1; // ZCL boolean true
      emberWriteWwahServerSilabsAttribute(endpoint,
                                          ZCL_SL_WWAH_BAD_PARENT_RECOVERY_ENABLED_ATTRIBUTE_ID,
                                          "WWAH bad parent",
                                          (uint8_t *)&badParentRecoveryEnabled,
                                          ZCL_BOOLEAN_ATTRIBUTE_TYPE);
      if (badParentRejoinPeriod) {
        badParentRecoveryRetryPeriod = badParentRejoinPeriod;  //in min
      } else {
        badParentRecoveryRetryPeriod = BAD_PARENT_RECOVERY_RETRY_DEFAULT;
      }
      status = emberAfNetworkEventControlSetDelayMS(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl,
                                                    badParentRecoveryRetryPeriod * MILLISECOND_TICKS_PER_MINUTE);
      if (status != EMBER_SUCCESS) {
        emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: retry period is too large, falling back on default");
        badParentRecoveryRetryPeriod = BAD_PARENT_RECOVERY_RETRY_DEFAULT;
      } else {
        // MISRA
      }
      emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: Enabled bad parent recovery checking parent quality every %d minutes", badParentRecoveryRetryPeriod);
    }
  } else {
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: bad parent recovery already enabled. Checking parent quality every %d minutes", badParentRecoveryRetryPeriod);
  }
}

// Invoked by CLI command handler for Bad Parent Recovery.
void emberAfPluginWwahConnectivityManagerDisableBadParentRecovery(void)
{
  emAfPluginWwahConnectivityManagerDisableBadParentRecovery(wwahServerEndpoint);
}

// Common handler for ZCL and CLI that conveys endpoint.
void emAfPluginWwahConnectivityManagerDisableBadParentRecovery(uint8_t endpoint)
{
  if (connectionManagerTokenStruct.connectionManagerFlags & PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY) {
    parentRecoveryRequired = false;
    connectionManagerTokenStruct.connectionManagerFlags &= ~PLUGIN_WWAH_CONNECTION_MANAGER_BAD_PARENT_RECOVERY;
    halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                      &connectionManagerTokenStruct);
    uint8_t badParentRecoveryEnabled = 0; // ZCL boolean false
    emberWriteWwahServerSilabsAttribute(endpoint,
                                        ZCL_SL_WWAH_BAD_PARENT_RECOVERY_ENABLED_ATTRIBUTE_ID,
                                        "WWAH bad parent",
                                        (uint8_t *)&badParentRecoveryEnabled,
                                        ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    emberAfNetworkEventControlSetInactive(&emberAfPluginWwahConnectivityManagerBadParentRecoveryEventControl);
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: Disabled bad parent recovery");
    attemptedParentRejoinStats = 0;
    parentRssiCheckStats = 0;
  } else {
    emberAfSlWwahClusterPrintln("WWAH Connectivity Mgr: bad parent recovery already disabled.");
  }
}

void emberAfPluginWwahConnectivityManagerSetRejoinParameters(uint16_t fastRejoinTimeoutSeconds,
                                                             uint16_t durationBetweenRejoinsSeconds,
                                                             uint16_t fastRejoinFirstBackoffSeconds,
                                                             uint16_t maxBackoffTimeSeconds,
                                                             uint16_t maxBackoffIterations)
{
  connectionManagerTokenStruct.fastRejoinTimeoutSec = fastRejoinTimeoutSeconds;
  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);

  connectionManagerTokenStruct.durationBetweenEachRejoinSec = durationBetweenRejoinsSeconds;
  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);

  connectionManagerTokenStruct.fastRejoinFirstBackoffTimeSec = fastRejoinFirstBackoffSeconds;
  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);

  connectionManagerTokenStruct.maxBackoffTimeSec = maxBackoffTimeSeconds;
  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);

  connectionManagerTokenStruct.maxBackoffIter = maxBackoffIterations;
  halCommonSetToken(TOKEN_PLUGIN_CONNECTION_MANAGER_STATE,
                    &connectionManagerTokenStruct);
}
