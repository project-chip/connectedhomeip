/***************************************************************************//**
 * @file
 * @brief
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

#define CREATOR_SHORT_REBOOT_MONITOR (0x000c)
#define NVM3KEY_SHORT_REBOOT_MONITOR (NVM3KEY_DOMAIN_USER | 0x000c)

#ifdef DEFINETOKENS

DEFINE_COUNTER_TOKEN(SHORT_REBOOT_MONITOR, uint32_t, 0)

#endif //DEFINETOKENS
