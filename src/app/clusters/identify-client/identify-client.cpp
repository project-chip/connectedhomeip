/* See Project CHIP LICENSE file for licensing information. */


/****************************************************************************
 * @file
 * @brief Routines for the Identify Client plugin, the
 *        client implementation of the Identify cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/util/af.h>

using namespace chip;

bool emberAfIdentifyClusterIdentifyQueryResponseCallback(uint16_t timeout)
{
    emberAfIdentifyClusterPrintln("RX: IdentifyQueryResponse 0x%4x", timeout);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
