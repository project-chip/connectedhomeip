/***************************************************************************//**
 * @brief Zigbee Source Route component configuration header.
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

// <h>Zigbee Source Route Library configuration

// <o EMBER_SOURCE_ROUTE_TABLE_SIZE> Source route table Size (SoC or NCP) <2-255>
// <i> Default: 7
// <i> The size of the source route table for storing source routes on the SOC or NCP.
#define EMBER_SOURCE_ROUTE_TABLE_SIZE   7

// <o EMBER_MAX_SOURCE_ROUTE_RELAY_COUNT> Max source route relay counts <11-40>
// <i> Default: 11
// <i> Max source route hops accepted or/and inserted in the source route table/header.
#define EMBER_MAX_SOURCE_ROUTE_RELAY_COUNT   11

// </h>

// <<< end of configuration section >>>
