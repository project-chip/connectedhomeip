/***************************************************************************//**
 * @file
 * @brief Device initialization for clocks.
 *******************************************************************************
 * # License
 * <b>Copyright 2019 Silicon Laboratories Inc. www.silabs.com</b>
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
#include "sl_device_init_clocks.h"

#include "em_cmu.h"

sl_status_t sl_device_init_clocks(void)
{
  CMU_ClockSelectSet(cmuClock_SYSCLK, cmuSelect_HFXO);
#if defined(_CMU_EM01GRPACLKCTRL_MASK)
  CMU_ClockSelectSet(cmuClock_EM01GRPACLK, cmuSelect_HFXO);
#endif
#if defined(_CMU_EM01GRPBCLKCTRL_MASK)
  CMU_ClockSelectSet(cmuClock_EM01GRPBCLK, cmuSelect_HFXO);
#endif
  CMU_ClockSelectSet(cmuClock_EM23GRPACLK, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_EM4GRPACLK, cmuSelect_LFXO);
#if defined(RTCC_PRESENT)
  CMU_ClockSelectSet(cmuClock_RTCC, cmuSelect_LFXO);
#endif
#if defined(SYSRTC_PRESENT)
  CMU_ClockSelectSet(cmuClock_SYSRTC, cmuSelect_LFXO);
#endif
  CMU_ClockSelectSet(cmuClock_WDOG0, cmuSelect_LFXO);
#if WDOG_COUNT > 1
  CMU_ClockSelectSet(cmuClock_WDOG1, cmuSelect_LFXO);
#endif

  return SL_STATUS_OK;
}
