/***************************************************************************//**
 * @brief Zigbee Green Power component configuration header.
 *\n*******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

// <h>Zigbee Green Power Library configuration

// <o EMBER_GP_PROXY_TABLE_SIZE> Green Power Proxy Table Size <1-126>
// <i> Default: 5
// <i> The maximum number of Green Power proxy table entries supported by the stack.
#define EMBER_GP_PROXY_TABLE_SIZE   5

// <o EMBER_GP_PROXY_TABLE_TOKEN_SIZE> Green Power Proxy Table Token Size <1-126>
// <i> Default: 5
// <i> The maximum number of pairings stored in proxy table non-volatile memory,between 0 and EMBER_GP_PROXY_TABLE_SIZE.
#define EMBER_GP_PROXY_TABLE_TOKEN_SIZE   5

// <o EMBER_GP_SINK_TABLE_SIZE> Green Power Sink Table Size <0-126>
// <i> Default: 0
// <i> The maximum number of Green Power sink table entries supported by the stack.
#define EMBER_GP_SINK_TABLE_SIZE   0

// <o EMBER_GP_SINK_TABLE_TOKEN_SIZE> Green Power Sink Table Token Size <0-126>
// <i> Default: 0
// <i> The maximum number of pairings stored in sink table non-volatile memory,between 0 and EMBER_GP_SINK_TABLE_SIZE.
#define EMBER_GP_SINK_TABLE_TOKEN_SIZE   0

// <o EMBER_GP_INCOMING_FC_TOKEN_TIMEOUT> Green Power incoming FC (as part of proxy table) Token timeout <0-60>
// <i> Default: 0
// <i> The timeout (seconds) to restore the GPD incoming security frame counter in the Flash (0 being never).
#define EMBER_GP_INCOMING_FC_TOKEN_TIMEOUT   7

// <o EMBER_GP_INCOMING_FC_TOKEN_TABLE_SIZE> Green Power incoming FC (as part of proxy table) table token table size <0-126>
// <i> Default: 0
// <i> The maximum table size to restore the GPD incoming security frame counter in the Flash (being either 0 or EMBER_GP_PROXY_TABLE_SIZE).
#define EMBER_GP_INCOMING_FC_TOKEN_TABLE_SIZE   0

// <o EMBER_GP_INCOMING_FC_IN_SINK_TOKEN_TIMEOUT> Green Power incoming FC (as part of sink table) token table token timeout <0-60>
// <i> Default: 0
// <i> The timeout (seconds) to restore the GPD incoming security frame counter in the Flash (0 being never).
#define EMBER_GP_INCOMING_FC_IN_SINK_TOKEN_TIMEOUT   7

// <o EMBER_GP_INCOMING_FC_IN_SINK_TOKEN_TABLE_SIZE> Green Power incoming FC (as part of sink table)Token table size <0-126>
// <i> Default: 0
// <i> The maximum table size to restore the GPD incoming security frame counter in the Flash (being either 0 or EMBER_GP_SINK_TABLE_SIZE).
#define EMBER_GP_INCOMING_FC_IN_SINK_TOKEN_TABLE_SIZE   0

// </h>

// <<< end of configuration section >>>
