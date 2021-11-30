/***************************************************************************//**
 * @file
 * @brief Tokens for the Illuminance Measurement Server plugin.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
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

/**
 * Custom Application Tokens
 */
#define CREATOR_SI1141_MULTIPLIER  (0x000F)
#define NVM3KEY_SI1141_MULTIPLIER  (NVM3KEY_DOMAIN_USER | 0x000F)

#ifdef DEFINETOKENS
DEFINE_BASIC_TOKEN(SI1141_MULTIPLIER, uint8_t, 0)
#endif //DEFINETOKENS
