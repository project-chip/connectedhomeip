/*
 * SPDX-FileCopyrightText: 2009-2022 Arm Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * This file is derivative of CMSIS V5.9.0 system_ARMCM55.h
 * Git SHA: 2b7495b8535bdcb306dac29b9ded4cfb679d7e5c
 */

#ifndef __SYSTEM_CORE_INIT_H__
#define __SYSTEM_CORE_INIT_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t SystemCoreClock; /*!< System Clock Frequency (Core Clock)  */
extern uint32_t PeripheralClock; /*!< Peripheral Clock Frequency */

/**
  \brief Exception / Interrupt Handler Function Prototype
*/
typedef void (*VECTOR_TABLE_Type)(void);

/**
  \brief Setup the microcontroller system.
   Initialize the System and update the SystemCoreClock variable.
 */
extern void SystemInit(void);

/**
  \brief  Update SystemCoreClock variable.
   Updates the SystemCoreClock with current core Clock retrieved from cpu registers.
 */
extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CORE_INIT_H__ */
