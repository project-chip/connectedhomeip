/***************************************************************************//**
 * @brief Zigbee Security Link Keys component configuration header.
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

// <h>Zigbee Security Link Keys Library configuration

// <o EMBER_KEY_TABLE_SIZE> Link Key Table Size <1-127>
// <i> Default: 6
// <i> The maximum number of link key table entries supported by the stack.
#define EMBER_KEY_TABLE_SIZE   12

// <o EMBER_REQUEST_KEY_TIMEOUT> Request Key Timeout <0-10>
// <i> Default: 0
// <i> The length of time that a node will wait for a trust center to answer its Application Link Key request.
#define EMBER_REQUEST_KEY_TIMEOUT   0

// </h>

// <<< end of configuration section >>>
