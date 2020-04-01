/***************************************************************************//**
 * @file
 * @brief
 ******************************************************************************/

#ifndef ZCL_ON_OFF_SERVER_H
#define ZCL_ON_OFF_SERVER_H

// Define OnOff plugin Scenes sub-table structure.
// NOTE: When modifying this structure take into account NVM token space and
// backward compatibility considerations
typedef struct {
  bool hasOnOffValue;
  bool onOffValue;
} EmZclOnOffSceneSubTableEntry_t;

#endif // ZCL_ON_OFF_SERVER_H
