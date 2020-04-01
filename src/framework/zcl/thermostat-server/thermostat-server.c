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
#include "thermostat-server.h"

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
  #ifdef DEFINETOKENS
// Token based storage.
    #define retrieveSceneSubTableEntry(entry, i) \
  halCommonGetIndexedToken(&entry, TOKEN_ZCL_CORE_THERMOSTAT_SCENE_SUBTABLE, i)
    #define saveSceneSubTableEntry(entry, i) \
  halCommonSetIndexedToken(TOKEN_ZCL_CORE_THERMOSTAT_SCENE_SUBTABLE, i, &entry)
  #else
// RAM based storage.
ChZclThermostatSceneSubTableEntry_t chZclPluginThermostatServerSceneSubTable[CHIP_AF_PLUGIN_SCENES_SERVER_TABLE_SIZE] = { { 0 } };
    #define retrieveSceneSubTableEntry(entry, i) \
  (entry = chZclPluginThermostatServerSceneSubTable[i])
    #define saveSceneSubTableEntry(entry, i) \
  (chZclPluginThermostatServerSceneSubTable[i] = entry)
  #endif
#endif

#ifdef CHIP_AF_API_ZCL_SCENES_SERVER
// Scenes callback handlers...

void chZclThermostatServerEraseSceneHandler(uint8_t tableIdx)
{
  ChZclThermostatSceneSubTableEntry_t entry;

  entry.hasOccupiedCoolingSetpointValue = false;
  entry.hasOccupiedHeatingSetpointValue = false;
  entry.hasSystemModeValue = false;

  saveSceneSubTableEntry(entry, tableIdx);
}

bool chZclThermostatServerAddSceneHandler(ChipZclClusterId_t clusterId,
                                          uint8_t tableIdx,
                                          const uint8_t *sceneData,
                                          uint8_t length)
{
  if (clusterId == CHIP_ZCL_CLUSTER_THERMOSTAT) {
    if (length < 2) {
      return false; // ext field format error (occupiedCoolingSetpointValue bytes must be present, other bytes optional).
    }

    // Extract bytes from input data block and update scene subtable fields.
    ChZclThermostatSceneSubTableEntry_t entry = { 0 };
    uint8_t *pData = (uint8_t *)sceneData;

    entry.hasOccupiedCoolingSetpointValue = true;
    entry.occupiedCoolingSetpointValue = chipZclPluginScenesServerGetUint16FromBuffer(&pData);
    length -= 2;
    if (length >= 2) {
      entry.hasOccupiedHeatingSetpointValue = true;
      entry.occupiedHeatingSetpointValue = chipZclPluginScenesServerGetUint16FromBuffer(&pData);
      length -= 2;
      if (length >= 1) {
        entry.hasSystemModeValue = true;
        entry.systemModeValue = chipZclPluginScenesServerGetUint8FromBuffer(&pData);
      }
    }

    saveSceneSubTableEntry(entry, tableIdx);

    return true;
  }

  return false;
}

void chZclThermostatServerRecallSceneHandler(ChipZclEndpointId_t endpointId,
                                             uint8_t tableIdx,
                                             uint32_t transitionTime100mS)
{
  // Handles the recallScene command for the thermostat cluster.
  // Note- this handler presently just updates (writes) the relevant cluster
  // attribute(s), in a production system this could be replaced by a call
  // to the relevant thermostat command handler to actually change the
  // hw state.

  ChZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if (entry.hasOccupiedCoolingSetpointValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterThermostatServerSpec,
                           CHIP_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_COOLING_SETPOINT,
                           (uint8_t *)&entry.occupiedCoolingSetpointValue,
                           sizeof(entry.occupiedCoolingSetpointValue));
  }
  if (entry.hasOccupiedHeatingSetpointValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterThermostatServerSpec,
                           CHIP_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_HEATING_SETPOINT,
                           (uint8_t *)&entry.occupiedHeatingSetpointValue,
                           sizeof(entry.occupiedHeatingSetpointValue));
  }
  if (entry.hasSystemModeValue) {
    chipZclWriteAttribute(endpointId,
                           &chipZclClusterThermostatServerSpec,
                           CHIP_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_SYSTEM_MODE,
                           (uint8_t *)&entry.systemModeValue,
                           sizeof(entry.systemModeValue));
  }
}

void chZclThermostatServerStoreSceneHandler(ChipZclEndpointId_t endpointId,
                                            uint8_t tableIdx)
{
  ChZclThermostatSceneSubTableEntry_t entry;

  entry.hasOccupiedCoolingSetpointValue =
    (chipZclReadAttribute(endpointId,
                           &chipZclClusterThermostatServerSpec,
                           CHIP_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_COOLING_SETPOINT,
                           (uint8_t *)&entry.occupiedCoolingSetpointValue,
                           sizeof(entry.occupiedCoolingSetpointValue)) == CHIP_ZCL_STATUS_SUCCESS);
  entry.hasOccupiedHeatingSetpointValue =
    (chipZclReadAttribute(endpointId,
                           &chipZclClusterThermostatServerSpec,
                           CHIP_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_OCCUPIED_HEATING_SETPOINT,
                           (uint8_t *)&entry.occupiedHeatingSetpointValue,
                           sizeof(entry.occupiedHeatingSetpointValue)) == CHIP_ZCL_STATUS_SUCCESS);
  entry.hasSystemModeValue =
    (chipZclReadAttribute(endpointId,
                           &chipZclClusterThermostatServerSpec,
                           CHIP_ZCL_CLUSTER_THERMOSTAT_SERVER_ATTRIBUTE_SYSTEM_MODE,
                           (uint8_t *)&entry.systemModeValue,
                           sizeof(entry.systemModeValue)) == CHIP_ZCL_STATUS_SUCCESS);

  saveSceneSubTableEntry(entry, tableIdx);
}

void chZclThermostatServerCopySceneHandler(uint8_t srcTableIdx,
                                           uint8_t dstTableIdx)
{
  ChZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, srcTableIdx);

  saveSceneSubTableEntry(entry, dstTableIdx);
}

void chZclThermostatServerViewSceneHandler(uint8_t tableIdx,
                                           uint8_t **ppExtFldData)
{
  ChZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  if ((entry.hasOccupiedCoolingSetpointValue)
      || (entry.hasOccupiedHeatingSetpointValue)
      || (entry.hasSystemModeValue)) {
    chipZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                CHIP_ZCL_CLUSTER_THERMOSTAT);

    uint8_t *pLength = *ppExtFldData;  // Save pointer to length byte.
    *pLength = 0;
    chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData, *pLength); // Insert temporary length value.

    if (entry.hasOccupiedCoolingSetpointValue) {
      chipZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.occupiedCoolingSetpointValue);
    }
    if (entry.hasOccupiedHeatingSetpointValue) {
      chipZclPluginScenesServerPutUint16InBuffer(ppExtFldData,
                                                  entry.occupiedHeatingSetpointValue);
    }
    if (entry.hasSystemModeValue) {
      chipZclPluginScenesServerPutUint8InBuffer(ppExtFldData,
                                                 entry.systemModeValue);
    }

    // Update length byte value.
    *pLength = *ppExtFldData - pLength - 1;
  }
}

void chZclThermostatServerPrintInfoSceneHandler(uint8_t tableIdx)
{
  ChZclThermostatSceneSubTableEntry_t entry;
  retrieveSceneSubTableEntry(entry, tableIdx);

  chipAfCorePrint(" thermostat:%2x %2x %x",
                   entry.occupiedCoolingSetpointValue,
                   entry.occupiedHeatingSetpointValue,
                   entry.systemModeValue);

  chipAfCoreFlush();
}
#endif
