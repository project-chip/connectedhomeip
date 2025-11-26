/**
  ******************************************************************************
  * @file    utilities_common.h
  * @author  MCD Application Team
  * @brief   Common file to utilities
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UTILITIES_COMMON_H
#define UTILITIES_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "app_conf.h"

  /* -------------------------------- *
   *  Basic definitions               *
   * -------------------------------- */

#undef NULL
#define NULL                    0

#undef FALSE
#define FALSE                   0

#undef TRUE
#define TRUE                    (!0)

  /* -------------------------------- *
   *  Critical Section definition     *
   * -------------------------------- */
#undef BACKUP_PRIMASK
#define BACKUP_PRIMASK()    uint32_t primask_bit= __get_PRIMASK()

#undef DISABLE_IRQ
#define DISABLE_IRQ()       __disable_irq()

#undef RESTORE_PRIMASK
#define RESTORE_PRIMASK()   __set_PRIMASK(primask_bit)

  /* -------------------------------- *
   *  Macro delimiters                *
   * -------------------------------- */
#undef M_BEGIN
#define M_BEGIN     do {

#undef  M_END
#define M_END       } while(0)

  /* -------------------------------- *
   *  Some useful macro definitions   *
   * -------------------------------- */
#undef MAX
#define MAX( x, y )          (((x)>(y))?(x):(y))

#undef MIN
#define MIN( x, y )          (((x)<(y))?(x):(y))

#undef MODINC
#define MODINC( a, m )       M_BEGIN  (a)++;  if ((a)>=(m)) (a)=0;  M_END

#undef MODDEC
#define MODDEC( a, m )       M_BEGIN  if ((a)==0) (a)=(m);  (a)--;  M_END

#undef MODADD
#define MODADD( a, b, m )    M_BEGIN  (a)+=(b);  if ((a)>=(m)) (a)-=(m);  M_END

#undef MODSUB
#define MODSUB( a, b, m )    MODADD( a, (m)-(b), m )

#undef ALIGN
#ifdef WIN32
#define ALIGN(n)
#else
#define ALIGN(n)             __attribute__((aligned(n)))
#endif

#undef PAUSE
#define PAUSE( t )           M_BEGIN \
                               volatile int _i; \
                               for ( _i = t; _i > 0; _i -- ); \
                             M_END
#undef DIVF
#define DIVF( x, y )         ((x)/(y))

#undef DIVC
#define DIVC( x, y )         (((x)+(y)-1)/(y))

#undef DIVR
#define DIVR( x, y )         (((x)+((y)/2))/(y))

#undef SHRR
#define SHRR( x, n )         ((((x)>>((n)-1))+1)>>1)

#undef BITN
#define BITN( w, n )         (((w)[(n)/32] >> ((n)%32)) & 1)

#undef BITNSET
#define BITNSET( w, n, b )   M_BEGIN (w)[(n)/32] |= ((U32)(b))<<((n)%32); M_END

/* -------------------------------- *
 *  Section attribute               *
 * -------------------------------- */
#define PLACE_IN_SECTION( __x__ )  __attribute__((section (__x__)))

#ifdef __cplusplus
}
#endif

#define SYS_WAITING_CYCLES_25() do {\
            __asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");\
            __asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");\
            __asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");\
            __asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");\
            __asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");__asm("mov r0, r0");\
           } while(0)

#define SYS_WAITING_RNGCLK_DEACTIVATION() do {\
            for(volatile unsigned int cpt = 178 ; cpt!=0 ; --cpt);\
           } while(0)

#define STM32WBA5x_DEFAULT_SCA_RANGE          (0)
#define STM32WBA5x_REV_ID_A_SCA_RANGE         (0)
#define STM32WBA5x_REV_ID_B_SCA_RANGE         (0)
#ifdef STM32WBA65xx
#define STM32WBA6x_SCA_RANGE                  (0)
#endif

/* Macro helper for optimizing by speed specific functions.
 * For IAR only: The functions with this definition will be optimized
 * by speed only if the project uses a High optimisation level.
 */
#if defined(__IAR_SYSTEMS_ICC__)
#define OPTIMIZED _Pragma("optimize=speed")
#elif defined(__clang__)
#define OPTIMIZED   _Pragma("pragma Ofast")
#elif defined(__GNUC__)
#define OPTIMIZED   __attribute__((optimize("Ofast")))
#endif

#define UTIL_UNUSED(X) (void)X      /* To avoid gcc/g++ warnings */

#endif /* UTILITIES_COMMON_H */

