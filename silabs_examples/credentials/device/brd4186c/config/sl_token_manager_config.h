/***************************************************************************//**
 * @file
 * @brief Token Manager Configurations
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc.  Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement.  This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// <<< Use Configuration Wizard in Context Menu >>>

#ifndef SL_TOKEN_MANAGER_CONFIG_H
#define SL_TOKEN_MANAGER_CONFIG_H

// <h> TOKEN MANAGER Configurations

// <e SL_TOKEN_MANAGER_CUSTOM_TOKENS_PRESENT> Enable Custom Tokens
// <i> Default: 0
#define SL_TOKEN_MANAGER_CUSTOM_TOKENS_PRESENT  0

// <s SL_TOKEN_MANAGER_CUSTOM_TOKEN_HEADER> File containing custom tokens
// <i> Default: "sl_custom_token_header.h"
// <i> Header file containing custom tokens located at project_root/config
#define SL_TOKEN_MANAGER_CUSTOM_TOKEN_HEADER "sl_custom_token_header.h"

// </e>

// <e SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKENS_PRESENT> Enable Custom Manufacturing Tokens
// <i> Default: 0
#define SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKENS_PRESENT  0

// <s SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKEN_HEADER> File containing custom manufacturing tokens
// <i> Default: "sl_custom_manufacturing_token_header.h"
// <i> Header file containing custom tokens located at project_root/config
#define SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKEN_HEADER "sl_custom_manufacturing_token_header.h"

// </e>
// </h>

#if (SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKENS_PRESENT)
#define APPLICATION_MFG_TOKEN_HEADER SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKEN_HEADER
#endif // SL_TOKEN_MANAGER_CUSTOM_MANUFACTURING_TOKENS_PRESENT

#endif // SL_TOKEN_MANAGER_CONFIG_H

// <<< end of configuration section >>>
