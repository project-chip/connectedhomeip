/*
 * SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/****************************************************************************
 * @file
 * @brief Routines for the Identify Client plugin, the
 *        client implementation of the Identify cluster.
 *******************************************************************************
 ******************************************************************************/

#include <app/CommandHandler.h>
#include <app/util/af.h>

using namespace chip;

bool emberAfIdentifyClusterIdentifyQueryResponseCallback(app::CommandHandler * commandObj, uint16_t timeout)
{
    emberAfIdentifyClusterPrintln("RX: IdentifyQueryResponse 0x%4x", timeout);
    emberAfSendImmediateDefaultResponse(EMBER_ZCL_STATUS_SUCCESS);
    return true;
}
