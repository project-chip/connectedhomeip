/***************************************************************************//**
 * @file
 * @brief Configuration header of Bootloader Interface
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
#ifndef BTL_INTERFACE_CFG_H
#define BTL_INTERFACE_CFG_H

#if !defined(BOOTLOADER_APPLOADER)

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_1)
#include "btl_interface_cfg_s2c1.h"
#endif

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_2)
#include "btl_interface_cfg_s2c2.h"
#endif

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_3)
#include "btl_interface_cfg_s2c3.h"
#endif

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_4)
#include "btl_interface_cfg_s2c4.h"
#endif

#if defined(_SILICON_LABS_32B_SERIES_2_CONFIG_5)
#define BOOTLOADER_DISABLE_OLD_BOOTLOADER_MITIGATION 1
#endif

#endif // !BOOTLOADER_APPLOADER

#endif // BTL_INTERFACE_CFG_H
