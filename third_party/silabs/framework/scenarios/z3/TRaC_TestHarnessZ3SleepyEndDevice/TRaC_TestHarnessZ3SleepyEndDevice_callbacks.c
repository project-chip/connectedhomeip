/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 * @brief
 *******************************************************************************
   ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

#include "app/framework/include/af.h"
#include EMBER_AF_API_NETWORK_STEERING
#include PLATFORM_HEADER

/**
 * Custom CLI.  This command tree is executed by typing "custom <command>"
 * See app/util/serial/command-interpreter2.h for more detail on writing commands.
 **/
/*  Example sub-menu */
//  extern void doSomethingFunction(void);
//  static EmberCommandEntry customSubMenu[] = {
//    emberCommandEntryAction("do-something", doSomethingFunction, "", "Do something description"),
//    emberCommandEntryTerminator()
//  };
void customLookupNeighbourTable(void);
void customResetBootloader(void);
//  extern void actionFunction(void);
EmberCommandEntry emberAfCustomCommands[] = {
  /* Sample Custom CLI commands */
  // emberCommandEntrySubMenu("sub-menu", customSubMenu, "Sub menu of custom commands"),
  emberCommandEntryAction("lookup", customLookupNeighbourTable, "b", "Custom command to look up the shortaddress given an IEEE address"),
  emberCommandEntryAction("resetBootloader", customResetBootloader, "", "Invokes seriel bootloader"),
  //emberCommandEntryAction("version", customVersion, "b", "Custom command to look up version"),
  emberCommandEntryTerminator()
};

void customLookupNeighbourTable(void)
{
  EmberEUI64 eui64;
  emberCopyBigEndianEui64Argument(0, eui64);
  EmberNodeId nodeId = emberLookupNodeIdByEui64(eui64);
  if (EMBER_NULL_NODE_ID == nodeId ) {
    emberAfAppPrintln("shortaddress:EUI64 unknown");
  } else {
    emberAfAppPrintln("shortaddress:0x%2X", nodeId);
  }
}

void customResetBootloader(void)
{
  halLaunchStandaloneBootloader(0); //STANDALONE_BOOTLOADER_RECOVERY_MODE
}

/** @brief Alarms Cluster Reset Alarm
 *
 *
 *
 * @param alarmCode   Ver.: always
 * @param clusterId   Ver.: always
 */
boolean emberAfAlarmClusterResetAlarmCallback(int8u alarmCode,
                                              int16u clusterId)
{
  return TRUE;
}

/** @brief Alarms Cluster Reset All Alarms
 *
 *
 *
 */
boolean emberAfAlarmClusterResetAllAlarmsCallback(void)
{
  return TRUE;
}

/** @brief Alarms Cluster Alarm
 *
 *
 *
 * @param alarmCode   Ver.: always
 * @param clusterId   Ver.: always
 */
boolean emberAfAlarmClusterAlarmCallback(int8u alarmCode,
                                         int16u clusterId)
{
  return TRUE;
}

/** @brief Groups Cluster Add Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
boolean emberAfGroupsClusterAddGroupResponseCallback(int8u status,
                                                     int16u groupId)
{
  return TRUE;
}

/** @brief Groups Cluster View Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param groupName   Ver.: always
 */
boolean emberAfGroupsClusterViewGroupResponseCallback(int8u status,
                                                      int16u groupId,
                                                      int8u* groupName)
{
  return TRUE;
}

/** @brief Groups Cluster Get Group Membership Response
 *
 *
 *
 * @param capacity   Ver.: always
 * @param groupCount   Ver.: always
 * @param groupList   Ver.: always
 */
boolean emberAfGroupsClusterGetGroupMembershipResponseCallback(int8u capacity,
                                                               int8u groupCount,
                                                               int8u* groupList)
{
  return TRUE;
}

/** @brief Groups Cluster Remove Group Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
boolean emberAfGroupsClusterRemoveGroupResponseCallback(int8u status,
                                                        int16u groupId)
{
  return FALSE;
}

/** @brief Scenes Cluster Add Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
boolean emberAfScenesClusterAddSceneResponseCallback(int8u status,
                                                     int16u groupId,
                                                     int8u sceneId)
{
  return TRUE;
}

/** @brief Scenes Cluster View Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 * @param transitionTime   Ver.: always
 * @param sceneName   Ver.: always
 * @param extensionFieldSets   Ver.: always
 */
boolean emberAfScenesClusterViewSceneResponseCallback(int8u status,
                                                      int16u groupId,
                                                      int8u sceneId,
                                                      int16u transitionTime,
                                                      int8u* sceneName,
                                                      int8u* extensionFieldSets)
{
  return TRUE;
}

/** @brief Scenes Cluster Remove Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
boolean emberAfScenesClusterRemoveSceneResponseCallback(int8u status,
                                                        int16u groupId,
                                                        int8u sceneId)
{
  return TRUE;
}

/** @brief Scenes Cluster Remove All Scenes Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 */
boolean emberAfScenesClusterRemoveAllScenesResponseCallback(int8u status,
                                                            int16u groupId)
{
  return TRUE;
}

/** @brief Scenes Cluster Store Scene Response
 *
 *
 *
 * @param status   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneId   Ver.: always
 */
boolean emberAfScenesClusterStoreSceneResponseCallback(int8u status,
                                                       int16u groupId,
                                                       int8u sceneId)
{
  return TRUE;
}

/** @brief Scenes Cluster Get Scene Membership Response
 *
 *
 *
 * @param status   Ver.: always
 * @param capacity   Ver.: always
 * @param groupId   Ver.: always
 * @param sceneCount   Ver.: always
 * @param sceneList   Ver.: always
 */
boolean emberAfScenesClusterGetSceneMembershipResponseCallback(int8u status,
                                                               int8u capacity,
                                                               int16u groupId,
                                                               int8u sceneCount,
                                                               int8u* sceneList)
{
  return TRUE;
}

/** @brief Thermostat Cluster Current Weekly Schedule
 *
 *
 *
 * @param numberOfTransitionsForSequence   Ver.: always
 * @param dayOfWeekForSequence   Ver.: always
 * @param modeForSequence   Ver.: always
 * @param payload   Ver.: always
 */
boolean emberAfThermostatClusterCurrentWeeklyScheduleCallback(int8u numberOfTransitionsForSequence,
                                                              int8u dayOfWeekForSequence,
                                                              int8u modeForSequence,
                                                              int8u* payload)
{
  return TRUE;
}

/** @brief Thermostat Cluster Relay Status Log
 *
 *
 *
 * @param timeOfDay   Ver.: always
 * @param relayStatus   Ver.: always
 * @param localTemperature   Ver.: always
 * @param humidityInPercentage   Ver.: always
 * @param setpoint   Ver.: always
 * @param unreadEntries   Ver.: always
 */
boolean emberAfThermostatClusterRelayStatusLogCallback(int16u timeOfDay,
                                                       int16u relayStatus,
                                                       int16s localTemperature,
                                                       int8u humidityInPercentage,
                                                       int16s setpoint,
                                                       int16u unreadEntries)
{
  return TRUE;
}

/** @brief Thermostat Cluster Setpoint Raise Lower
 *
 *
 *
 * @param mode   Ver.: always
 * @param amount   Ver.: always
 */
boolean emberAfThermostatClusterSetpointRaiseLowerCallback(int8u mode,
                                                           int8s amount)
{
  return TRUE;
}

/** @brief Thermostat Cluster Set Weekly Schedule
 *
 *
 *
 * @param numberOfTransitionsForSequence   Ver.: always
 * @param dayOfWeekForSequence   Ver.: always
 * @param modeForSequence   Ver.: always
 * @param payload   Ver.: always
 */
boolean emberAfThermostatClusterSetWeeklyScheduleCallback(int8u numberOfTransitionsForSequence,
                                                          int8u dayOfWeekForSequence,
                                                          int8u modeForSequence,
                                                          int8u* payload)
{
  return TRUE;
}

/** @brief Thermostat Cluster Get Weekly Schedule
 *
 *
 *
 * @param daysToReturn   Ver.: always
 * @param modeToReturn   Ver.: always
 */
boolean emberAfThermostatClusterGetWeeklyScheduleCallback(int8u daysToReturn,
                                                          int8u modeToReturn)
{
  return TRUE;
}

/** @brief Thermostat Cluster Clear Weekly Schedule
 *
 *
 *
 */
boolean emberAfThermostatClusterClearWeeklyScheduleCallback(void)
{
  return TRUE;
}

/** @brief Thermostat Cluster Get Relay Status Log
 *
 *
 *
 */
boolean emberAfThermostatClusterGetRelayStatusLogCallback(void)
{
  return TRUE;
}

/** @brief IAS Zone Cluster Zone Status Change Notification
 *
 *
 *
 * @param zoneStatus   Ver.: always
 * @param extendedStatus   Ver.: always
 * @param zoneId   Ver.: since ha-1.2-05-3520-29
 * @param delay   Ver.: since ha-1.2-05-3520-29
 */
boolean emberAfIasZoneClusterZoneStatusChangeNotificationCallback(int16u zoneStatus,
                                                                  int8u extendedStatus,
                                                                  int8u zoneId,
                                                                  int16u delay)
{
  return TRUE;
}

/** @brief IAS Zone Cluster Zone Enroll Request
 *
 *
 *
 * @param zoneType   Ver.: always
 * @param manufacturerCode   Ver.: always
 */
boolean emberAfIasZoneClusterZoneEnrollRequestCallback(int16u zoneType,
                                                       int16u manufacturerCode)
{
  return TRUE;
}

/** @brief IAS Zone Cluster Zone Enroll Response
 *
 *
 *
 * @param enrollResponseCode   Ver.: always
 * @param zoneId   Ver.: always
 */
boolean emberAfIasZoneClusterZoneEnrollResponseCallback(int8u enrollResponseCode,
                                                        int8u zoneId)
{
  return TRUE;
}

/** @brief Commissioning Cluster Restart Device Response
 *
 *
 *
 * @param status   Ver.: always
 */
boolean emberAfCommissioningClusterRestartDeviceResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Commissioning Cluster Save Startup Parameters Response
 *
 *
 *
 * @param status   Ver.: always
 */
boolean emberAfCommissioningClusterSaveStartupParametersResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Commissioning Cluster Restore Startup Parameters Response
 *
 *
 *
 * @param status   Ver.: always
 */
boolean emberAfCommissioningClusterRestoreStartupParametersResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Commissioning Cluster Reset Startup Parameters Response
 *
 *
 *
 * @param status   Ver.: always
 */
boolean emberAfCommissioningClusterResetStartupParametersResponseCallback(int8u status)
{
  return TRUE;
}

/** @brief Commissioning Cluster Restart Device
 *
 *
 *
 * @param options   Ver.: always
 * @param delay   Ver.: always
 * @param jitter   Ver.: always
 */
boolean emberAfCommissioningClusterRestartDeviceCallback(int8u options,
                                                         int8u delay,
                                                         int8u jitter)
{
  return TRUE;
}

/** @brief Commissioning Cluster Reset Startup Parameters
 *
 *
 *
 * @param options   Ver.: always
 * @param index   Ver.: always
 */
boolean emberAfCommissioningClusterResetStartupParametersCallback(int8u options,
                                                                  int8u index)
{
  return TRUE;
}

/** @brief Trust Center Join
 *
 * This callback is called from within the application framework's
 * implementation of emberTrustCenterJoinHandler or ezspTrustCenterJoinHandler.
 * This callback provides the same arguments passed to the
 * TrustCenterJoinHandler. For more information about the TrustCenterJoinHandler
 * please see documentation included in stack/include/trust-center.h.
 *
 * @param newNodeId   Ver.: always
 * @param newNodeEui64   Ver.: always
 * @param parentOfNewNode   Ver.: always
 * @param status   Ver.: always
 * @param decision   Ver.: always
 */
void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
  if (EMBER_STANDARD_SECURITY_SECURED_REJOIN == status
      || EMBER_STANDARD_SECURITY_UNSECURED_JOIN == status
      || EMBER_STANDARD_SECURITY_UNSECURED_REJOIN == status) {
    emberAfAddAddressTableEntry(newNodeEui64, newNodeId);
  }
}
/** @brief Ok To Sleep
 *
 * This function is called by the Idle/Sleep plugin before sleeping. It is
 * called with interrupts disabled. The application should return true if the
 * device may sleep or false otherwise.
 *
 * @param durationMs The maximum duration in milliseconds that the device will
 * sleep. Ver.: always
 */
#include "app/util/serial/command-interpreter2.h"
bool sleepEnable = false;
bool emberAfPluginIdleSleepOkToSleepCallback(uint32_t durationMs)
{
  // if sleep enabled
  if (!sleepEnable) {
    return false;
  }

  // if no serial input pending
  if (emberSerialReadAvailable(APP_SERIAL) != 0) {
    return false;
  }

  // if not in middle of reading a command
  if (emberCommandInterpreterBusy()) {
    return false;
  }

  // for now, always return false to disable actual sleeping, since we
  // always lose the first character on a wake-up - this won't really
  // matter for a test harness, since we only really need to be Rx
  // Off When Idle.
  return false;
}

/** @brief Wake Up
 *
 * This function is called by the Idle/Sleep plugin after sleeping.
 *
 * @param durationMs The duration in milliseconds that the device slept.
 * Ver.: always
 */
void emberAfPluginIdleSleepWakeUpCallback(uint32_t durationMs)
{
}

/** @brief Main Init
 *
 * This function is called from the application's main function. It gives the
 * application a chance to do any initialization required at system startup. Any
 * code that you would normally put into the top of the application's main()
 * routine should be put into this function. This is called before the clusters,
 * plugins, and the network are initialized so some functionality is not yet
 * available.
        Note: No callback in the Application Framework is
 * associated with resource cleanup. If you are implementing your application on
 * a Unix host where resource cleanup is a consideration, we expect that you
 * will use the standard Posix system calls, including the use of atexit() and
 * handlers for signals such as SIGTERM, SIGINT, SIGCHLD, SIGPIPE and so on. If
 * you use the signal() function to register your signal handler, please mind
 * the returned value which may be an Application Framework function. If the
 * return value is non-null, please make sure that you call the returned
 * function from your handler to avoid negating the resource cleanup of the
 * Application Framework itself.
 *
 */
void emberAfMainInitCallback(void)
{
  sleepEnable = true;
}
