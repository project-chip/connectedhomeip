/* See Project CHIP LICENSE file for licensing information. */

/****************************************************************************
 * @file
 * @brief Routines for the Door Lock Client plugin, the
 *        client implementation of the Door Lock cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af.h>

using namespace chip;

bool emberAfDoorLockClusterLockDoorResponseCallback(uint8_t status)
{
    emberAfDoorLockClusterPrintln("RX: LockDoorResponse 0x%x", status);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}

bool emberAfDoorLockClusterUnlockDoorResponseCallback(uint8_t status)
{
    emberAfDoorLockClusterPrintln("RX: UnlockDoorResponse 0x%x", status);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
