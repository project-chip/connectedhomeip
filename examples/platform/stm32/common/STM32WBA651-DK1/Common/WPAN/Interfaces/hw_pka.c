/**
  ******************************************************************************
  * @file    hw_pka.c
  * @author  MCD Application Team
  * @brief   This file contains the PKA driver for STM32WBA
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

#include "app_common.h"
#include "stm32wbaxx_ll_bus.h"
#include "stm32wbaxx_ll_pka.h"

/*****************************************************************************/

typedef struct
{
  uint8_t  run;
} HW_PKA_VAR_T;

/*****************************************************************************/

static HW_PKA_VAR_T HW_PKA_var;

/*****************************************************************************/

int HW_PKA_Enable( void )
{
  HW_PKA_VAR_T* pv = &HW_PKA_var;

  /* Test if the driver is not already in use */

  if ( pv->run )
  {
    return FALSE;
  }

  pv->run = TRUE;

  /* Enable the RNG clock as it is needed.
   * See PKA chapter in IUM: the RNG peripheral must be clocked.
   */
  HW_RNG_EnableClock( 2 );

  UTILS_ENTER_CRITICAL_SECTION( );

  /* Enable the PKA clock */
  LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_PKA );

  UTILS_EXIT_CRITICAL_SECTION( );

  /* Enable the PKA block */
  LL_PKA_Enable( PKA );

  /* Wait for PKA initialization OK */
  while ( !(PKA->SR & PKA_SR_INITOK) );

  /* Reset any pending flag */
  SET_BIT(PKA->CLRFR, (PKA_CLRFR_PROCENDFC | PKA_CLRFR_RAMERRFC |
                       PKA_CLRFR_ADDRERRFC | PKA_CLRFR_OPERRFC));

  /* Disable the RNG clock as it is no more needed ???
   */
  HW_RNG_DisableClock( 2 );

  return TRUE;
}

/*****************************************************************************/

void HW_PKA_WriteSingleInput( uint32_t index, uint32_t word )
{
  /* Write the single word into PKA RAM */

  PKA->RAM[index] = word;
}

/*****************************************************************************/

void HW_PKA_WriteOperand( uint32_t index, int size, const uint32_t* in )
{
  uint32_t* pka_ram = (uint32_t*)&PKA->RAM[index];

  /* Write the input data into PKA RAM */

  for ( ; size > 0; size-- )
  {
    *pka_ram++ = *in++;
  }

  /* Write extra zeros into PKA RAM */

  *pka_ram = 0;
}

/*****************************************************************************/

void HW_PKA_Start( uint32_t mode )
{
  /* Set the configuration */
  LL_PKA_Config( PKA, mode );

  /* Start the PKA processing */
  LL_PKA_ClearFlag_PROCEND( PKA );
  LL_PKA_Start( PKA );
}

/*****************************************************************************/

int HW_PKA_EndOfOperation( void )
{
  /* Return 0 if the processing is still active */
  return LL_PKA_IsActiveFlag_PROCEND( PKA );
}

/*****************************************************************************/

uint32_t HW_PKA_ReadSingleOutput( uint32_t index )
{
  /* Read a single word from PKA RAM */

  return PKA->RAM[index];
}

/*****************************************************************************/

void HW_PKA_ReadResult( uint32_t index, int size, uint32_t* out )
{
  uint32_t* pka_ram = (uint32_t*)&PKA->RAM[index];

  /* Read from PKA RAM */

  for ( ; size > 0; size-- )
  {
    *out++ = *pka_ram++;
  }
}

/*****************************************************************************/

void HW_PKA_Disable( void )
{
  HW_PKA_VAR_T* pv = &HW_PKA_var;

  if ( pv->run )
  {
    /* Disable the PKA block */
    LL_PKA_Disable( PKA );

    UTILS_ENTER_CRITICAL_SECTION( );

    /* Disable the PKA clock */
    LL_AHB2_GRP1_DisableClock( LL_AHB2_GRP1_PERIPH_PKA );

    UTILS_EXIT_CRITICAL_SECTION( );

    pv->run = FALSE;
  }
}

/*****************************************************************************/
