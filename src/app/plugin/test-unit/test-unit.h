#ifndef CHIP_ZCL_TEST_UNIT
#define CHIP_ZCL_TEST_UNIT

#include "chip-zcl-codec.h"
#include "chip-zcl.h"
#include "gen.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int testClusterServerBasic();
int testClusterServerIdentify();
int testClusterServerLevelControl();
int testClusterServerOnOff();
int testCodecSimple();
int testCoreDataModel();
int testCoreMessageDispatch();

#endif // CHIP_ZCL_TEST_UNIT
