/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_SCENES_SERVER_H
#define ZCL_SCENES_SERVER_H

#include CHIP_AF_API_ZCL_CORE

#define CHIP_ZCL_SCENE_TABLE_NULL_INDEX       0xFF
#define CHIP_ZCL_SCENE_NULL                   ((uint8_t)-1)

#define ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH        16
#define ZCL_SCENES_CLUSTER_MAXIMUM_EXT_FIELD_LENGTH   200

#define ZCL_SCENES_GLOBAL_SCENE_GROUP_ID         0x0000
#define ZCL_SCENES_GLOBAL_SCENE_SCENE_ID         0x00

// Define Scenes structure type
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  ChipZclEndpointId_t endpointId;
  ChipZclGroupId_t groupId;
  uint8_t sceneId;
#ifdef CHIP_AF_PLUGIN_SCENES_SERVER_NAME_SUPPORT
  char name[ZCL_SCENES_CLUSTER_MAXIMUM_NAME_LENGTH + 1];
#endif
  uint16_t transitionTime;      // in seconds
  uint8_t transitionTime100ms;  // in tenths of a seconds (0..9)
  //
  // NOTE: Scene cluster data is now stored in scene sub-tables that reside in
  // each cluster plug-in. Cluster plugins must register handlers to provide
  // scenes functionality.
  //
} ChipZclSceneEntry_t;

// Public functions.
ChipZclStatus_t chipZclClusterScenesServerSetSceneCountAttribute(ChipZclEndpointId_t endpointId, uint8_t newCount);
ChipZclStatus_t chipZclClusterScenesServerMakeValid(ChipZclEndpointId_t endpointId, uint8_t sceneId, uint16_t groupId);
ChipZclStatus_t chipZclClusterScenesServerMakeInvalidCallback(ChipZclEndpointId_t endpointId);
void chipZclClusterScenesServerPrintInfo(void);
void chipZclClusterScenesServerClearSceneTable(ChipZclEndpointId_t endpointId);

// Scenes utilities.
uint8_t chipZclPluginScenesServerGetUint8FromBuffer(uint8_t **ptr);
void chipZclPluginScenesServerPutUint8InBuffer(uint8_t **ptr, uint8_t value);
uint16_t chipZclPluginScenesServerGetUint16FromBuffer(uint8_t **ptr);
void chipZclPluginScenesServerPutUint16InBuffer(uint8_t **ptr, uint16_t value);

#endif // ZCL_SCENES_SERVER_H
