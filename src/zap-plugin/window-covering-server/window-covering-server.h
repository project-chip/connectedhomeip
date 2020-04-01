/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_WINDOW_COVERING_SERVER_H
#define ZCL_WINDOW_COVERING_SERVER_H

#include EMBER_AF_API_ZCL_CORE

// Define Window Covering plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasCurrentPositionLiftPercentageValue;
  uint8_t currentPositionLiftPercentageValue;
  bool hasCurrentPositionTiltPercentageValue;
  uint8_t currentPositionTiltPercentageValue;
} EmZclWindowCoveringSceneSubTableEntry_t;

#endif // ZCL_WINDOW_COVERING_SERVER_H
