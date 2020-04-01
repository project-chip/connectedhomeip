/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_COLOR_CONTROL_SERVER_H
#define ZCL_COLOR_CONTROL_SERVER_H

#include EMBER_AF_API_ZCL_CORE

// Define Color Control plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasCurrentXValue;
  uint16_t currentXValue;
  bool hasCurrentYValue;
  uint16_t currentYValue;
  bool hasEnhancedCurrentHueValue;
  uint16_t enhancedCurrentHueValue;
  bool hasCurrentSaturationValue;
  uint8_t currentSaturationValue;
  bool hasColorLoopActiveValue;
  uint8_t colorLoopActiveValue;
  bool hasColorLoopDirectionValue;
  uint8_t colorLoopDirectionValue;
  bool hasColorLoopTimeValue;
  uint16_t colorLoopTimeValue;
} EmZclColorControlSceneSubTableEntry_t;

void emberZclColorControlServerSetColorControl(EmberZclEndpointId_t endpointId, bool value);

#endif // ZCL_COLOR_CONTROL_SERVER
