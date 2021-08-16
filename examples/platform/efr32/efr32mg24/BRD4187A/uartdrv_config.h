/***************************************************************************//**
 * @file
 * @brief UARTDRV configuration file.
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#ifndef __SILICON_LABS_UARTDRV_CONFIG_H__
#define __SILICON_LABS_UARTDRV_CONFIG_H__

/***************************************************************************//**
 * @addtogroup uartdrv
 * @{
 ******************************************************************************/
/// Size of the receive operation queue.
/// @details
///   The maximum number of receive operations that can be queued up for one
///   driver instance before @ref UARTDRV_Receive() returns
///   @ref ECODE_EMDRV_UARTDRV_QUEUE_FULL.
/// @note
///   This macro is not used by the UARTDRV itself, but is intended to be used
///   with the @ref DEFINE_BUF_QUEUE macro by the user of the driver to allocate
///   instances of the @ref UARTDRV_Buffer_FifoQueue_t struct.
#if !defined(EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS)
#define EMDRV_UARTDRV_MAX_CONCURRENT_RX_BUFS    6
#endif

/// Size of the transmit operation queue.
/// @details
///   The maximum number of transmit operations that can be queued up for one
///   driver instance before @ref UARTDRV_Transmit() returns
///   @ref ECODE_EMDRV_UARTDRV_QUEUE_FULL.
/// @note
///   This macro is not used by the UARTDRV itself, but is intended to be used
///   with the @ref DEFINE_BUF_QUEUE macro by the user of the driver to allocate
///   instances of the @ref UARTDRV_Buffer_FifoQueue_t struct.
#if !defined(EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS)
#define EMDRV_UARTDRV_MAX_CONCURRENT_TX_BUFS    6
#endif

// <<< Use Configuration Wizard in Context Menu >>>
// <h> UARTDRV Settings

/// Set to 1 to include flow control support
#if !defined(EMDRV_UARTDRV_FLOW_CONTROL_ENABLE)
// <o EMDRV_UARTDRV_FLOW_CONTROL_ENABLE>  Flow control support
// <1=> Enable
// <0=> Disable
// <i> Default: 1
#define EMDRV_UARTDRV_FLOW_CONTROL_ENABLE       1
#endif

/// Maximum number of driver instances.
#if !defined(EMDRV_UARTDRV_MAX_DRIVER_INSTANCES)
// <o EMDRV_UARTDRV_MAX_DRIVER_INSTANCES> Maximum number of driver instances
// <i> This maximum only applies when UARTDRV_FLOW_CONTROL_ENABLE = 1
// <i> Default: 4
#define EMDRV_UARTDRV_MAX_DRIVER_INSTANCES      4
#endif

/// UART software flow control code: request peer to start TX
#if !defined(UARTDRV_FC_SW_XON)
// <o UARTDRV_FC_SW_XON> UART software flow control code: request peer to start TX
// <i> Default: 0x11
#define UARTDRV_FC_SW_XON                       0x11
#endif

/// UART software flow control code: request peer to stop TX
#if !defined(UARTDRV_FC_SW_XOFF)
// <o UARTDRV_FC_SW_XOFF> UART software flow control code: request peer to stop TX
// <i> Default: 0x13
#define UARTDRV_FC_SW_XOFF                      0x13
#endif

/// UART enable reception when sleeping.
#if !defined(UARTDRV_RESTRICT_ENERGY_MODE_TO_ALLOW_RECEPTION)
// <o UARTDRV_RESTRICT_ENERGY_MODE_TO_ALLOW_RECEPTION>  Enable reception when sleeping
// <i> Enable reception when sleeping will use the power manager and add EM1
// <i> requirement during receive operations that use DMA.
// <1=> Enable
// <0=> Disable
// <i> Default: 1
#define UARTDRV_RESTRICT_ENERGY_MODE_TO_ALLOW_RECEPTION         1
#endif

//</h>

// <<< end of configuration section >>>

/** @} (end addtogroup uartdrv) */

#endif /* __SILICON_LABS_UARTDRV_CONFIG_H__ */
