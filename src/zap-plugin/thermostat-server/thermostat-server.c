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
#include "thermostat-server.h"

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_THERMOSTAT_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_THERMOSTAT_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
EmZclThermostatSceneSubTableEntry_t emZclPluginThermostatServerSceneSubTable[EMBER_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = emZclPluginThermostatServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (emZclPluginThermostatServerSceneSubTable[i] = entry)
  #endif
#endif

#ifdef EMBER_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void emZclThermostatServerEraseSceneHandler(uint8_t tableIdx)
{
  EmZclThermostatSceneSubTableEntry_t entry;

  entry.hasOccupiedCoolingSetpointValue = false;
  entry.hasOccupiedHeatingSetpointValue = false;
  entry.hasSystemModeValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool emZclThermostatServerAddSceneHandler(EmberZclClusterId_t clusterId,
                                          uint8_t tableIdx,
                                          const uint8_t *sceneData,
                                          uint8_t length)
{
  if (clusterId == EMBER_ZCL_CLUSTER_THERMOSTAT) {
    if (length < 2) {
      return false; // ext field format error (occupiedCoolingSetpointValue bytes must be present, other bytes optional).
    }

    // Extract bytes from input data block and update scene subtable fields.
    EmZclThermostatSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasOccupiedCoolingSetpointValue = true;
    entry.occupiedCoolingSetpointValue = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
    length -= 2;
    if (length >= 2) {
      entry.hasOccupiedHeatingSetpointValue = true;
      entry.occupiedHeatingSetpointValue = emberZclPluginScenesServerGetUint16FromBuffer(&pData);
      length -= 2;
      if (length >= 1) {
        entry.hasSystemModeValue = true;
        entry.systemModeValue = emberZclPluginScenesServerGetUint8FromBuffer(&pData);
      }
    }

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void emZclThermostatServerRecallSceneHandler(EmberZclEndpointId_t endpointId,
                                             uint8_t tableIdx,
                                             uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the thermostat cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant thermostat command handler to actually change the
  // hw state.

  EmZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasOccupiedCoolingSetpointValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterThermostatServerSpec,
                           EMBER_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_COOLING_SETPOINT,
                           (uint8_t *)&entry.occupiedCoolingSetpointValue,
                           sizeof(entry.occupiedCoolingSetpointValue));
  }
  if (entry.hasOccupiedHeatingSetpointValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterThermostatServerSpec,
                           EMBER_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_HEATING_SETPOINT,
                           (uint8_t *)&entry.occupiedHeatingSetpointValue,
                           sizeof(entry.occupiedHeatingSetpointValue));
  }
  if (entry.hasSystemModeValue) {
    emberZclWriteAttribute(endpointId,
                           &emberZclClusterThermostatServerSpec,
                           EMBER_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_SYSTEM_MODE,
                           (uint8_t *)&entry.systemModeValue,
                           sizeof(entry.systemModeValue));
  }
}

void emZclThermostatServerStoreSceneHandler(EmberZclEndpointId_t endpointId,
                                            uint8_t tableIdx)
{
  EmZclThermostatSceneSubTableEntry_t entry;

  entry.hasOccupiedCoolingSetpointValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterThermostatServerSpec,
                           EMBER_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_COOLING_SETPOINT,
                           (uint8_t *)&entry.occupiedCoolingSetpointValue,
                           sizeof(entry.occupiedCoolingSetpointValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasOccupiedHeatingSetpointValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterThermostatServerSpec,
                           EMBER_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_HEATING_SETPOINT,
                           (uint8_t *)&entry.occupiedHeatingSetpointValue,
                           sizeof(entry.occupiedHeatingSetpointValue)) == EMBER_ZCL_STATUS_SUCCESS);
  entry.hasSystemModeValue =
    (emberZclReadAttribute(endpointId,
                           &emberZclClusterThermostatServerSpec,
                           EMBER_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_SYSTEM_MODE,
                           (uint8_t *)&entry.systemModeValue,
                           sizeof(entry.systemModeValue)) == EMBER_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void emZclThermostatServerCopySceneHandler(uint8_t srcTableIdx,
                                           uint8_t dstTableIdx)
{
  EmZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void emZclThermostatServerViewSceneHandler(uint8_t tableIdx,
                                           uint8_t **ppExtFldData)
{
  EmZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if ((entry.hasOccupiedCoolingSetpointValue)
      || (entry.hasOccupiedHeatingSetpointValue)
      || (entry.hasSystemModeValue)) {
    emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                EMBER_ZCL_CLUSTER_THERMOSTAT);

    uint8_t *pLength = *ppExtFldData;  // Save pointer to length byte.
    *pLength = 0;
    emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData, *pLength); // Insert temporary length value.

    if (entry.hasOccupiedCoolingSetpointValue) {
      emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.occupiedCoolingSetpointValue);
    }
    if (entry.hasOccupiedHeatingSetpointValue) {
      emberZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.occupiedHeatingSetpointValue);
    }
    if (entry.hasSystemModeValue) {
      emberZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.systemModeValue);
    }

    // Update length byte value.
    *pLength = *ppExtFldData - pLength - 1;
  }
}

void emZclThermostatServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  EmZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  emberAfCorePrint(" thermostat:%2x %2x %x",
                   entry.occupiedCoolingSetpointValue,
                   entry.occupiedHeatingSetpointValue,
                   entry.systemModeValue);

  emberAfCoreFlush();
}
#endif
