/***************************************************************************//**
 * @brief Zigbee Gateway Support component configuration header.
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

// <h>Zigbee Gateway Support configuration

// <o EMBER_AF_PLUGIN_GATEWAY_MAX_FDS> Max File Descriptors to Monitor <3-255>
// <i> Default: 10
// <i> The maximum number of file descriptors that the gateway application can monitor for activity with select().
#define EMBER_AF_PLUGIN_GATEWAY_MAX_FDS   10

// <o EMBER_AF_PLUGIN_GATEWAY_TCP_PORT_OFFSET> TCP Port Offset <1-65535>
// <i> Default: 4900
// <i> The gateway application supports remote CLI connections via TCP.  This option defines the starting TCP port on the local system where the gateway will accept connections.  The first port X (i.e. 4900 by default) will be used for the CLI, while the X+1 port (i.e. 4901 by default) will be used for the raw connection.  The raw port is used to send/receive binary data.
#define EMBER_AF_PLUGIN_GATEWAY_TCP_PORT_OFFSET   4900

// <o EMBER_AF_PLUGIN_GATEWAY_MAX_WAIT_FOR_EVENT_TIMEOUT_MS> Maximum Time to Wait for Events (milliseconds) <1-4294967295>
// <i> Default: 0xFFFFFFFF
// <i> Maximum amount of time the Gateway Support plugin will allow the framework to yield the processor.  In other words, the max timeout to select().  Normally the framework limits its yield time due to whatever App Framework events and user created events are scheduled to run.  This further limits it to allow the main loop to run more frequently at the expense of CPU cycles.  This is a simpler way of insuring the main loop periodically runs.  The recommended way is to schedule an event through the framework.
#define EMBER_AF_PLUGIN_GATEWAY_MAX_WAIT_FOR_EVENT_TIMEOUT_MS   0xFFFFFFFF

// </h>

// <<< end of configuration section >>>
