/**
  ******************************************************************************
  * @file    hw_aes.c
  * @author  MCD Application Team
  * @brief   This file contains the AES driver for STM32WBA
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

/*****************************************************************************/

#define HW_AESX AES

#define HW_AES_CLOCK_ENABLE( )    LL_AHB2_GRP1_EnableClock( LL_AHB2_GRP1_PERIPH_AES )
#define HW_AES_CLOCK_DISABLE( )   LL_AHB2_GRP1_DisableClock( LL_AHB2_GRP1_PERIPH_AES )

#define HW_AES_CLOCK_IS_ENABLE( ) LL_AHB2_GRP1_IsEnabledClock( LL_AHB2_GRP1_PERIPH_AES )

#define AES_BLOCK_SIZE_WORD       (4U)
#define AES_BLOCK_SIZE_BYTE       (16U)

/*****************************************************************************/

typedef struct
{
  uint8_t  run;
} HW_AES_VAR_T;

/*****************************************************************************/

static HW_AES_VAR_T HW_AES_var;

/*****************************************************************************/

int HW_AES_Enable( void )
{
  HW_AES_VAR_T* av = &HW_AES_var;

  /* Test if the driver is not already in use */

  if ( HW_AES_CLOCK_IS_ENABLE() )
  {
    return FALSE;
  }
  av->run = TRUE;

  UTILS_ENTER_CRITICAL_SECTION( );

  /* Enable AES clock */
  HW_AES_CLOCK_ENABLE( );

  UTILS_EXIT_CRITICAL_SECTION( );

  return TRUE;
}

/*****************************************************************************/

void HW_AES_SetKey( uint32_t mode,
                    const uint8_t* key )
{
  uint32_t tmp[4];

  /* Retrieve all bytes of key */
  memcpy( tmp, key, 16 );

  /* Initialize the AES peripheral with default values:
     - Processing:     disabled
     - Data type:      32-bit
     - Operating mode: encryption
     - Chaining mode:  ECB
     - Key size:       128-bit
  */
  HW_AESX->CR = 0;

  /* Copy key bytes to the AES registers */

  if ( mode & HW_AES_REV )
  {
    HW_AESX->KEYR0 = tmp[0];
    HW_AESX->KEYR1 = tmp[1];
    HW_AESX->KEYR2 = tmp[2];
    HW_AESX->KEYR3 = tmp[3];
  }
  else
  {
    HW_AESX->KEYR3 = __REV( tmp[0] );
    HW_AESX->KEYR2 = __REV( tmp[1] );
    HW_AESX->KEYR1 = __REV( tmp[2] );
    HW_AESX->KEYR0 = __REV( tmp[3] );
  }

  if ( !(mode & HW_AES_ENC) )
  {
    /* Set key preparation mode */
    HW_AESX->CR = AES_CR_MODE_0;

    /* Enable AES processing */
    HW_AESX->CR |= AES_CR_EN;

    /* Wait for CCF flag to be raised */
    while ( ! (HW_AESX->SR & AES_SR_CCF) );

    /* Clear CCF Flag */
    HW_AESX->ICR |= AES_ICR_CCF;

    /* Set decryption mode */
    HW_AESX->CR = AES_CR_MODE_1;
  }

  /* Enable byte swapping if needed */
  if ( mode & HW_AES_SWAP )
    HW_AESX->CR |= AES_CR_DATATYPE_1;

  /* Wait until KEYVALID is set */
  while ( !(HW_AESX->SR & AES_SR_KEYVALID) );

  /* Enable AES processing */
  HW_AESX->CR |= AES_CR_EN;
}

/*****************************************************************************/

void HW_AES_Crypt( const uint32_t* input,
                   uint32_t* output )
{
  /* Write the input block into the input FIFO */
  HW_AESX->DINR = input[0];
  HW_AESX->DINR = input[1];
  HW_AESX->DINR = input[2];
  HW_AESX->DINR = input[3];

  /* Wait for CCF flag to be raised */
  while ( !(HW_AESX->SR & AES_SR_CCF) );

  /* Read the output block from the output FIFO */
  output[0] = HW_AESX->DOUTR;
  output[1] = HW_AESX->DOUTR;
  output[2] = HW_AESX->DOUTR;
  output[3] = HW_AESX->DOUTR;

  /* Clear CCF Flag */
  HW_AESX->ICR |= AES_ICR_CCF;
}

/*****************************************************************************/

void HW_AES_Disable( void )
{
  HW_AES_VAR_T* av = &HW_AES_var;

  if ( av->run )
  {
    /* Disable AES processing */
    HW_AESX->CR = 0;

    UTILS_ENTER_CRITICAL_SECTION( );

    /* Disable AES clock */
    HW_AES_CLOCK_DISABLE( );

    UTILS_EXIT_CRITICAL_SECTION( );

    av->run = FALSE;
  }
}

/*****************************************************************************/

void HW_AES_Crypt8( const uint8_t * pInput, uint8_t * pOutput )
{
  uint32_t    pTemp[AES_BLOCK_SIZE_WORD];

  // Transfer 8 -> 32  bits */
  memcpy( pTemp, pInput, AES_BLOCK_SIZE_BYTE );

  /*  Write the input block into the input FIFO */
  HW_AESX->DINR = __REV( pTemp[0] );
  HW_AESX->DINR = __REV( pTemp[1] );
  HW_AESX->DINR = __REV( pTemp[2] );
  HW_AESX->DINR = __REV( pTemp[3] );

  // -- Wait for CCF flag to be raised /
  while ( (HW_AESX->SR & AES_SR_CCF) == 0x00u )
    { }

  /* Read the output block from the output FIFO */
  pTemp[0] = __REV( HW_AESX->DOUTR );
  pTemp[1] = __REV( HW_AESX->DOUTR );
  pTemp[2] = __REV( HW_AESX->DOUTR );
  pTemp[3] = __REV( HW_AESX->DOUTR );

  /* Transfer 32 -> 8  bits */
  memcpy( pOutput, pTemp, AES_BLOCK_SIZE_BYTE );

  /* Clear CCF Flag */
  HW_AESX->ICR |= AES_ICR_CCF;
}

/*****************************************************************************/

void HW_AES_InitCcm( uint8_t decrypt,
                     const uint8_t* key,
                     const uint32_t* b0,
                     const uint32_t* b1 )
{
  uint32_t tmp[4], mode = decrypt ? AES_CR_MODE_1 : 0;

  /* CCM init phase */
  HW_AESX->CR = AES_CR_CHMOD_2 | mode;

  /* Copy key bytes to the AES registers */
  memcpy( tmp, key, 16 );
  HW_AESX->KEYR0 = tmp[0];
  HW_AESX->KEYR1 = tmp[1];
  HW_AESX->KEYR2 = tmp[2];
  HW_AESX->KEYR3 = tmp[3];

  /* Copy B0 bytes to the AES registers */
  HW_AESX->IVR3 = __REV( b0[0] );
  HW_AESX->IVR2 = __REV( b0[1] );
  HW_AESX->IVR1 = __REV( b0[2] );
  HW_AESX->IVR0 = __REV( b0[3] );

  /* Enable AES processing */
  HW_AESX->CR |= AES_CR_EN;

  /* Wait for CCF flag to be raised */
  while ( ! (HW_AESX->SR & AES_SR_CCF) );

  /* Clear CCF Flag */
  HW_AESX->ICR |= AES_ICR_CCF;

  /* CCM header phase */
  HW_AESX->CR = AES_CR_CHMOD_2 | AES_CR_GCMPH_0 | AES_CR_DATATYPE_1;

  /* Enable AES processing */
  HW_AESX->CR |= AES_CR_EN;

  /* Write the header block B1 into the input FIFO */
  HW_AESX->DINR = b1[0];
  HW_AESX->DINR = b1[1];
  HW_AESX->DINR = b1[2];
  HW_AESX->DINR = b1[3];

  /* Wait for CCF flag to be raised */
  while ( !(HW_AESX->SR & AES_SR_CCF) );

  /* Clear CCF Flag */
  HW_AESX->ICR |= AES_ICR_CCF;

  /* CCM payload  phase */
  HW_AESX->CR = (AES_CR_EN | AES_CR_CHMOD_2 |
                 AES_CR_GCMPH_1 | AES_CR_DATATYPE_1 | mode);
}

/*****************************************************************************/

void HW_AES_EndCcm( uint8_t tag_length,
                    uint8_t* tag )
{
  uint32_t tmp[4];

  /* CCM final phase */
  HW_AESX->CR = (AES_CR_EN | AES_CR_CHMOD_2 |
                 AES_CR_GCMPH_0 | AES_CR_GCMPH_1 | AES_CR_DATATYPE_1);

  /* Wait for CCF flag to be raised */
  while ( !(HW_AESX->SR & AES_SR_CCF) );

  /* Read the output block from the output FIFO */
  tmp[0] = HW_AESX->DOUTR;
  tmp[1] = HW_AESX->DOUTR;
  tmp[2] = HW_AESX->DOUTR;
  tmp[3] = HW_AESX->DOUTR;
  memcpy( tag, tmp, tag_length );

  /* Clear CCF Flag */
  HW_AESX->ICR |= AES_ICR_CCF;
}

/*****************************************************************************/

void HW_AES_SetLast( uint8_t left_length )
{
  HW_AESX->CR |= (16UL - left_length) << AES_CR_NPBLB_Pos;
}

/*****************************************************************************/
