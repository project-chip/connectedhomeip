/**
  ******************************************************************************
  * @file    pka_ctrl.c
  * @author  MCD Application Team
  * @brief   Source file for PKA controller module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"
#include "stm32wbaxx_ll_bus.h"
#include "stm32wbaxx_ll_pka.h"
#include "pka_ctrl.h"

/* Private defines -----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
enum
{
  PKACTRL_IDLE = 0,
  PKACTRL_P256_KEY_GEN,
  PKACTRL_RANGE_X_CHECK,
  PKACTRL_RANGE_Y_CHECK,
  PKACTRL_POINT_CHECK,
  PKACTRL_DH_KEY_GEN,
};

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t PKA_Run = 0;

static uint8_t PKACTRL_state;
static uint8_t PKACTRL_error;
static uint32_t PKACTRL_buffer[24];

static const uint32_t PKA_P256_gfp[8] =
{
  0xFFFFFFFF, /* LSB */
  0xFFFFFFFF,
  0xFFFFFFFF,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000001,
  0xFFFFFFFF,
};

static const uint32_t PKA_P256_r2[8] =
{
  0x00000002, /* LSB */
  0x00000005,
  0x00000003,
  0xFFFFFFFE,
  0xFFFFFFF9,
  0xFFFFFFFB,
  0xFFFFFFFC,
  0xFFFFFFFC,
};

static const uint32_t PKA_P256_p_x[8] =
{
  0xD898C296, /* LSB */
  0xF4A13945,
  0x2DEB33A0,
  0x77037D81,
  0x63A440F2,
  0xF8BCE6E5,
  0xE12C4247,
  0x6B17D1F2,
};

static const uint32_t PKA_P256_p_y[8] =
{
  0x37BF51F5, /* LSB */
  0xCBB64068,
  0x6B315ECE,
  0x2BCE3357,
  0x7C0F9E16,
  0x8EE7EB4A,
  0xFE1A7F9B,
  0x4FE342E2,
};

static const uint32_t PKA_P256_a[8] =
{
  0x00000003, /* LSB */
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
};

static const uint32_t PKA_P256_b[8] =
{
  0x27D2604B, /* LSB */
  0x3BCE3C3E,
  0xCC53B0F6,
  0x651D06B0,
  0x769886BC,
  0xB3EBBD55,
  0xAA3A93E7,
  0x5AC635D8,
};

static const uint32_t PKA_P256_n[8] =
{
  0XFC632551, /* LSB */
  0XF3B9CAC2,
  0XA7179E84,
  0XBCE6FAAD,
  0XFFFFFFFF,
  0XFFFFFFFF,
  0X00000000,
  0XFFFFFFFF
};

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
extern void Error_Handler(void);
static int PKACTRL_Process(void);

static int PKA_Enable(void);
static void PKA_Disable(void);
static void PKA_WriteSingleInput(uint32_t index, uint32_t word);
static void PKA_WriteOperand(uint32_t index, int size, const uint32_t* in);
static void PKA_Start(uint32_t mode);
static int PKA_EndOfOperation(void);
static uint32_t PKA_ReadSingleOutput(uint32_t index);
static void PKA_ReadResult(uint32_t index, int size, uint32_t* out);

static void PKA_P256_StartRangeCheck(const uint32_t* coord);
static uint32_t PKA_P256_IsRangeCheckOk(void);
static void PKA_P256_StartPointCheck(const uint32_t* x,
                                     const uint32_t* y);
static uint32_t PKA_P256_IsPointCheckOk(void);
static void PKA_P256_StartEccScalarMul(const uint32_t* k,
                                       const uint32_t* p_x,
                                       const uint32_t* p_y);
static void PKA_P256_ReadEccScalarMul(uint32_t* p_x,
                                      uint32_t* p_y);

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Reset the PKA controller module
 * @param  None
 * @retval None
 */
void PKACTRL_Reset(void)
{
  /* Disable PKA hardware */
  PKA_Disable();

  PKACTRL_state = PKACTRL_IDLE;
}

/**
 * @brief  Start P-256 public key generation
 * @param  local_private_key: table of 8 x 32-bit words that contains the
 *         P-256 private key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_StartP256Key(const uint32_t* local_private_key)
{
  /* Enable PKA hardware */
  if (PKA_Enable() != PKACTRL_OK)
    return PKACTRL_BUSY;

  /* Call the PKA scalar multiplication with the local private key
     as k and the standard point as starting point,
     in order to compute the local public key */
  PKA_P256_StartEccScalarMul(local_private_key, NULL, NULL);

  PKACTRL_state = PKACTRL_P256_KEY_GEN;
  PKACTRL_error = 1;

  PKACTRL_CB_Process();

  return PKACTRL_OK;
}

/**
 * @brief  Get result of P-256 public key generation
 * @param  local_public_key: table of 32 x 32-bit words that is filled by the
 *         function with the generated P-256 public key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_ReadP256Key(uint32_t* local_public_key)
{
  if (PKACTRL_error != 0)
    return PKACTRL_EOF;

  /* Get local public key from buffer */
  memcpy(local_public_key, PKACTRL_buffer, 64);

  return PKACTRL_OK;
}

/**
 * @brief  Start DH key computation
 * @param  local_private_key: table of 8 x 32-bit words that contains the
 *         local P-256 private key (Little Endian format)
 * @param  remote_public_key: table of 32 x 32-bit words that contains the
 *         remote P-256 public key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_StartDhKey(const uint32_t* local_private_key,
                       const uint32_t* remote_public_key)
{
  /* Enable PKA hardware */
  if (PKA_Enable() != PKACTRL_OK)
    return PKACTRL_BUSY;

  /* Call the PKA range check operation for public key X coordinate */
  PKA_P256_StartRangeCheck(remote_public_key);

  /* Save input data */
  memcpy(PKACTRL_buffer, local_private_key, 32);
  memcpy(PKACTRL_buffer + 8, remote_public_key, 64);

  PKACTRL_state = PKACTRL_RANGE_X_CHECK;
  PKACTRL_error = 1;

  PKACTRL_CB_Process();

  return PKACTRL_OK;
}

/**
 * @brief  Get result of DH key computation
 * @param  dh_key: table of 8 x 32-bit words that is filled by the function
 *         with the generated DH key (Little Endian format)
 * @retval status (PKACTRL_XX)
 */
int PKACTRL_ReadDhKey(uint32_t* dh_key)
{
  if (PKACTRL_error)
    return PKACTRL_EOF;

  /* Get DH key from buffer */
  memcpy(dh_key, PKACTRL_buffer, 32);

  return PKACTRL_OK;
}

/**
 * @brief  PKA controller background task
 * @param  None
 * @retval None
 */
void PKACTRL_BG_Process(void)
{
  if(PKACTRL_Process() != 0u)
  {
    PKACTRL_CB_Process();
  }
}

/**
 * @brief  Callback to call when a PKA Process has ended
 * @param  None
 * @retval None
 */
void PKACTRL_EndOfProcessCb(void)
{
  /* Release the end of operation semaphore */
  if (PKACTRL_ReleaseSemEndOfOperation() != 0)
  {
    Error_Handler();
  }
}

/* Private function Definition -----------------------------------------------*/

static int PKACTRL_Process(void)
{
  /* This function implements the offline key computation using the PKA
   */
  if (PKACTRL_state == PKACTRL_IDLE)
  {
    return PKACTRL_OK;
  }

  /* Check if the current operation is finished */
  if (PKA_EndOfOperation() != 0u)
    return PKACTRL_BUSY;

  switch (PKACTRL_state)
  {
  case PKACTRL_P256_KEY_GEN:

    /* Read the PKA scalar multiplication result which is the local public
       key */
    PKA_P256_ReadEccScalarMul(PKACTRL_buffer, PKACTRL_buffer + 8);

    PKACTRL_error = 0;

    break;

  case PKACTRL_RANGE_X_CHECK:

    /* Test result of range check operation for public key X coordinate */
    if (PKA_P256_IsRangeCheckOk() == 0x00000000)
      break;

    /* Call the PKA range check operation for public key Y coordinate */
    PKA_P256_StartRangeCheck(PKACTRL_buffer + 16);

    PKACTRL_state = PKACTRL_RANGE_Y_CHECK;

    return PKACTRL_BUSY;

  case PKACTRL_RANGE_Y_CHECK:

    /* Test result of range check operation for public key Y coordinate */
    if (PKA_P256_IsRangeCheckOk() == 0x00000000)
      break;

    /* Call the PKA point check operation for remote public key */
    PKA_P256_StartPointCheck(PKACTRL_buffer + 8,
                             PKACTRL_buffer + 16);

    PKACTRL_state = PKACTRL_POINT_CHECK;

    return PKACTRL_BUSY;

  case PKACTRL_POINT_CHECK:

    /* Test result of point check operation for remote public key */
    if (PKA_P256_IsPointCheckOk() == 0x00000000)
      break;

    /* Call the PKA scalar multiplication with the local private key
       as k and the remote public key as starting point,
       in order to compute the DH key */
    PKA_P256_StartEccScalarMul(PKACTRL_buffer,
                               PKACTRL_buffer + 8,
                               PKACTRL_buffer + 16);

    PKACTRL_state = PKACTRL_DH_KEY_GEN;

    return PKACTRL_BUSY;

  case PKACTRL_DH_KEY_GEN:

    /* Read the PKA scalar multiplication result which is the DH key */
    PKA_P256_ReadEccScalarMul(PKACTRL_buffer, NULL);

    PKACTRL_error = 0;

    break;
  }

  /* Callback to inform the BLE stack of the completion of PKA operation */
  PKACTRL_CB_Complete();

  /* End of process: reset the PKA module */
  PKACTRL_Reset();

  return PKACTRL_OK;
}

static int PKA_Enable(void)
{
  /* Test if the driver is not already in use */

  if (PKA_Run == 1)
  {
    return PKACTRL_BUSY;
  }

  if (PKACTRL_MutexTake() != 0)
  {
    return PKACTRL_BUSY;
  }

  PKA_Run = 1;

  /* Enable the RNG clock as it is needed.
   * See PKA chapter in IUM: the RNG peripheral must be clocked.
   */
  HW_RNG_EnableClock(2);

  UTILS_ENTER_CRITICAL_SECTION();

  /* Enable the PKA clock */
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_PKA);

  UTILS_EXIT_CRITICAL_SECTION();

  /* Enable the PKA block */
  LL_PKA_Enable(PKA);

  /* Wait for PKA initialization OK */
  while ( !(PKA->SR & PKA_SR_INITOK) );

  /* Reset any pending flag */
  SET_BIT(PKA->CLRFR, (PKA_CLRFR_PROCENDFC | PKA_CLRFR_RAMERRFC |
                       PKA_CLRFR_ADDRERRFC | PKA_CLRFR_OPERRFC));

  /* Set priority and Enable PKA interrupt */
  /* PKA_INTR_PRIO_PROCEND should be defined at application level */
  NVIC_SetPriority(PKA_IRQn, PKA_INTR_PRIO_PROCEND);
  HAL_NVIC_EnableIRQ(PKA_IRQn);

  /* Take the end of operation semaphore before operation start */
  if (PKACTRL_TakeSemEndOfOperation() != 0)
  {
    Error_Handler();
  }

  /* Disable the RNG clock as it is no more needed ???
   */
  HW_RNG_DisableClock(2);

  return PKACTRL_OK;
}

static void PKA_Disable(void)
{
  if (PKA_Run == 1)
  {
    /* Disable the PKA IRQ */
    HAL_NVIC_DisableIRQ(PKA_IRQn);

    /* Disable the PKA block */
    LL_PKA_Disable(PKA);

    UTILS_ENTER_CRITICAL_SECTION();

    /* Disable the PKA clock */
    LL_AHB2_GRP1_DisableClock(LL_AHB2_GRP1_PERIPH_PKA);

    UTILS_EXIT_CRITICAL_SECTION();

    /* Release the end of operation semaphore */
    if (PKACTRL_ReleaseSemEndOfOperation() != 0)
    {
      Error_Handler();
    }

    if (PKACTRL_MutexRelease() != 0)
    {
      Error_Handler();
    }

    PKA_Run = 0;
  }
}

static void PKA_WriteSingleInput(uint32_t index, uint32_t word)
{
  /* Write the single word into PKA RAM */
  PKA->RAM[index] = word;
}

static void PKA_WriteOperand(uint32_t index, int size, const uint32_t* in)
{
  uint32_t* pka_ram = (uint32_t*)&PKA->RAM[index];

  /* Write the input data into PKA RAM */
  for (; size > 0; size--)
  {
    *pka_ram++ = *in++;
  }

  /* Write extra zeros into PKA RAM */

  *pka_ram = 0;
}

static void PKA_Start(uint32_t mode)
{
  /* Set the configuration */
  LL_PKA_Config(PKA, mode);

  /* Setup IT end of process */
  LL_PKA_ClearFlag_PROCEND(PKA);
  LL_PKA_EnableIT_PROCEND(PKA);
  while (LL_PKA_IsEnabledIT_PROCEND(PKA) != 1);

  /* Start the PKA processing */
  LL_PKA_Start(PKA);
}

static int PKA_EndOfOperation(void)
{
  /* Check if operation is still in progress */
  if (LL_PKA_IsActiveFlag_PROCEND(PKA) != 1)
  {
    /* Wait until end of process semaphore is released */
    while (PKACTRL_TakeSemEndOfOperation() != 0);

    /* Disable the process end IT */
    LL_PKA_DisableIT_PROCEND(PKA);
  }

  return 0;
}

static uint32_t PKA_ReadSingleOutput(uint32_t index)
{
  /* Read a single word from PKA RAM */
  return PKA->RAM[index];
}

static void PKA_ReadResult(uint32_t index, int size, uint32_t* out)
{
  uint32_t* pka_ram = (uint32_t*)&PKA->RAM[index];

  /* Read from PKA RAM */
  for (; size > 0; size--)
  {
    *out++ = *pka_ram++;
  }
}

static void PKA_P256_StartRangeCheck(const uint32_t* coord)
{
  /* Set the muber of bits of P */
  PKA_WriteSingleInput(PKA_COMPARISON_IN_OP_NB_BITS, 256);

  /* Set the coordinate */
  PKA_WriteOperand(PKA_COMPARISON_IN_OP1, 8, coord);

  /* Set the modulus value p */
  PKA_WriteOperand(PKA_COMPARISON_IN_OP2, 8, PKA_P256_gfp);

  /* Start PKA hardware */
  PKA_Start(LL_PKA_MODE_COMPARISON);
}

static uint32_t PKA_P256_IsRangeCheckOk(void)
{
  return (PKA_ReadSingleOutput(PKA_COMPARISON_OUT_RESULT) == 0x916AUL);
}

static void PKA_P256_StartPointCheck(const uint32_t* x,
                                     const uint32_t* y)
{
  /* Set the muber of bits of p */
  PKA_WriteSingleInput(PKA_POINT_CHECK_IN_MOD_NB_BITS, 256);

  /* Set the coefficient a sign */
  PKA_WriteSingleInput(PKA_POINT_CHECK_IN_A_COEFF_SIGN, 1);

  /* Set the coefficient |a| */
  PKA_WriteOperand(PKA_POINT_CHECK_IN_A_COEFF, 8, PKA_P256_a);

  /* Set the coefficient b */
  PKA_WriteOperand(PKA_POINT_CHECK_IN_B_COEFF, 8, PKA_P256_b);

  /* Set the modulus value p */
  PKA_WriteOperand(PKA_POINT_CHECK_IN_MOD_GF, 8, PKA_P256_gfp);

  /* Set the point coordinate x */
  PKA_WriteOperand(PKA_POINT_CHECK_IN_INITIAL_POINT_X, 8, x);

  /* Set the point coordinate y */
  PKA_WriteOperand(PKA_POINT_CHECK_IN_INITIAL_POINT_Y, 8, y);

  /* Set the Montgomery parameter */
  PKA_WriteOperand(PKA_POINT_CHECK_IN_MONTGOMERY_PARAM,
                   8, PKA_P256_r2);

  /* Start PKA hardware */
  PKA_Start(LL_PKA_MODE_POINT_CHECK);
}

static uint32_t PKA_P256_IsPointCheckOk(void)
{
  return (PKA_ReadSingleOutput(PKA_POINT_CHECK_OUT_ERROR) == 0xD60DUL);
}

static void PKA_P256_StartEccScalarMul(const uint32_t* k,
                                       const uint32_t* p_x,
                                       const uint32_t* p_y)
{
  /* Set the scalar multiplier k length */
  PKA_WriteSingleInput(PKA_ECC_SCALAR_MUL_IN_EXP_NB_BITS, 256);

  /* Set the modulus length */
  PKA_WriteSingleInput(PKA_ECC_SCALAR_MUL_IN_OP_NB_BITS, 256);

  /* Set the coefficient a sign */
  PKA_WriteSingleInput(PKA_ECC_SCALAR_MUL_IN_A_COEFF_SIGN, 1);

  /* Set the coefficient |a| */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_A_COEFF, 8, PKA_P256_a);

  /* Set the coefficient b */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_B_COEFF, 8, PKA_P256_b);

  /* Set the modulus value p */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_MOD_GF, 8, PKA_P256_gfp);

  /* Set the scalar multiplier k */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_K, 8, k);

  /* Set the point P coordinate x */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_INITIAL_POINT_X,
                   8, p_x ? p_x : PKA_P256_p_x);

  /* Set the point P coordinate y */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_INITIAL_POINT_Y,
                   8, p_y ? p_y : PKA_P256_p_y);

  /* Set the prime order n */
  PKA_WriteOperand(PKA_ECC_SCALAR_MUL_IN_N_PRIME_ORDER,
                   8, PKA_P256_n);

  /* Start PKA hardware */
  PKA_Start(LL_PKA_MODE_ECC_MUL);
}

static void PKA_P256_ReadEccScalarMul(uint32_t* p_x,
                                      uint32_t* p_y)
{
  /* Read the output point X */
  if (p_x)
  {
    PKA_ReadResult(PKA_ECC_SCALAR_MUL_OUT_RESULT_X, 8, p_x);
  }

  /* Read the output point Y as the second half of the result */
  if (p_y)
  {
    PKA_ReadResult(PKA_ECC_SCALAR_MUL_OUT_RESULT_Y, 8, p_y);
  }
}

/* Weak function Definition --------------------------------------------------*/
__WEAK int PKACTRL_MutexTake(void)
{
  return 0; /* This shall be implemented by user */
}

__WEAK int PKACTRL_MutexRelease(void)
{
  return 0; /* This shall be implemented by user */
}

__WEAK int PKACTRL_TakeSemEndOfOperation(void)
{
  return 0; /* This shall be implemented by user */
}

__WEAK int PKACTRL_ReleaseSemEndOfOperation(void)
{
  return 0; /* This shall be implemented by user */
}

__WEAK void PKACTRL_CB_Complete( void )
{
}

__WEAK void PKACTRL_CB_Process( void )
{
}