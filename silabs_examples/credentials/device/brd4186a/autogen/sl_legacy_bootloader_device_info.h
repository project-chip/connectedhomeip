/***************************************************************************//**
 * @file
 * @brief Legacy Bootloader Interface Device Info
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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

enum {
  SL_FAMILY_UNKNOWN    = 0,
  SL_FAMILY_MGM1       = 1,
  SL_FAMILY_BGM1       = 1,
  SL_FAMILY_EFR32MG1P  = 16,
  SL_FAMILY_EFR32MG1B  = 17,
  SL_FAMILY_EFR32MG1V  = 18,
  SL_FAMILY_EFR32BG1P  = 19,
  SL_FAMILY_EFR32BG1B  = 20,
  SL_FAMILY_EFR32BG1V  = 21,
  SL_FAMILY_EFR32FG1P  = 25,
  SL_FAMILY_EFR32FG1V  = 27,
};

// CORTEXM3
#define SL_PLAT    4U

// CORTEXM3_EFR32
#define SL_MICRO   24U

#define SL_FAMILY  SL_FAMILY_EFR32MG24

// PHY_RAIL
#define SL_PHY     15U
