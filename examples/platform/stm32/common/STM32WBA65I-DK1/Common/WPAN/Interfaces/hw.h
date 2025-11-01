/**
  ******************************************************************************
  * @file    hw.h
  * @author  MCD Application Team
  * @brief   This file contains the interface of STM32 HW drivers.
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

#ifndef HW_H__
#define HW_H__

#include "stm32wbaxx.h"

/* ---------------------------------------------------------------------------
 *                               General
 * ---------------------------------------------------------------------------
 */

#ifndef CFG_HW_ERROR_OFFSET
#define CFG_HW_ERROR_OFFSET 0
#endif

/* Return values definition */
enum
{
  HW_OK     = 0,
  HW_BUSY   = 1
};

/*
 * HW_Init
 *
 * This function must be called once after reset before calling any of the
 * other HW functions.
 */
extern void HW_Init( void );

/*
 * HW_Delay
 *
 * This function is used internally for minimum delays.
 * The input is given in microseconds and must be strictly higher than 0 (> 0)
 * and lower than 16000000 (= 16 s).
 * Be careful that the actual delay can be higher than the one programmed
 * if the function is interrupted.
 * The function is declared "weak" and can be overloaded by the user.
 */
extern void HW_Delay( uint32_t delay_us );

/*
 * HW_GetPackageType
 *
 * Returns the package type (cf Package Data Register in STM32WB UM)
 */
extern uint32_t HW_GetPackageType( void );

/*
 * HW_Config_HSE
 */
void HW_Config_HSE( uint8_t hsetune );

/* ---------------------------------------------------------------------------
 *                                 AES
 * ---------------------------------------------------------------------------
 */

/* Mode definitions used for HW_AES_SetKey() function */
enum
{
  HW_AES_DEC     = 0,
  HW_AES_ENC     = 1,
  HW_AES_REV     = 2,
  HW_AES_SWAP    = 4
};

/*
 * HW_AES_Enable
 *
 * Enables the AES hardware block.
 * If the AES was already in use, the function does nothing and returns 0;
 * otherwise it returns 1.
 * Be careful: no re-entrance is ensured for the HW_AES functions
 */
extern int HW_AES_Enable( void );

/*
 * HW_AES_SetKey
 *
 * Sets the key used for encryption/decryption.
 * The "mode" parameter must be set to HW_AES_ENC for encryption and to
 * HW_AES_DEC for decryption. It can be or-ed with HW_AES_REV for a reveresd
 * oreder of key bytes, and with HW_AES_SWAP to use data byte swapping mode.
 */
extern void HW_AES_SetKey( uint32_t mode,
                           const uint8_t* key );

/*
 * HW_AES_Crypt
 *
 * Encrypts/decrypts the 16-byte input data ("input"). Result is written in the
 * 16-byte buffer ("output") allocated by the user.
 */
extern void HW_AES_Crypt( const uint32_t* input,
                          uint32_t* output );

/*
 * HW_AES_Crypt
 *
 * Encrypts/decrypts the 16-byte input data ("input").
 * Result is written in the 16-byte buffer ("output") allocated by the user.
 *
 * Note : input & output are 8 bits aligned.
 */
extern void HW_AES_Crypt8( const uint8_t* input, uint8_t* output );

/*
 * HW_AES_Disable
 *
 * Disables the AES hardware block.
 */
extern void HW_AES_Disable( void );

/*
 * HW_AES_InitCcm
 *
 * Initilaizes AES for CCM encryption (decrypt = 0) or decryption (decrypt = 1)
 * Note: B0 and B1 4-word blocks must be provided by user.
 *
 */
extern void HW_AES_InitCcm( uint8_t decrypt,
                            const uint8_t* key,
                            const uint32_t* b0,
                            const uint32_t* b1 );

/*
 * HW_AES_EndCcm
 *
 * Completes CCM processing by computing the authentication tag
 *
 */
extern void HW_AES_EndCcm( uint8_t tag_length,
                           uint8_t* tag );

/*
 * HW_AES_SetLast
 *
 * Function used in CCM processing to indicate the last block of data in
 * case of decryption
 *
 */
extern void HW_AES_SetLast( uint8_t left_length );

/* ---------------------------------------------------------------------------
 *                                 PKA
 * ---------------------------------------------------------------------------
 */

/*
 * HW_PKA_Enable
 *
 * Enables the PKA hardware block.
 * If the driver is already in used, the function returns 0 immediately.
 * If the PKA semaphore is available, this function locks the PKA semaphore,
 * otherwise it returns 0.
 * Then, when PKA semaphore is locked, the function enables PKA security,
 * PKA clock and the block itself.
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 * Be careful: no re-entrance is ensured for the HW_PKA functions
 */
extern int HW_PKA_Enable( void );

/*
 * HW_PKA_WriteSingleInput
 *
 * Writes one single word into the PKA memory.
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 */
extern void HW_PKA_WriteSingleInput( uint32_t index,
                                     uint32_t word );

/*
 * HW_PKA_WriteOperand
 *
 * Writes one operand of size 'n' 32-bit words into the PKA memory.
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 */
extern void HW_PKA_WriteOperand( uint32_t index,
                                 int size,
                                 const uint32_t* in );

/*
 * HW_PKA_Start
 *
 * Starts the PKA operation with mode defined by the parameter "mode".
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 *
 * "mode" can be one of the LL_PKA_MODE...  definitions that can be found
 * in "stm32wbxx_ll_pka.h" file.
 */
extern void HW_PKA_Start( uint32_t mode );

/*
 * HW_PKA_EndOfOperation
 *
 * Returns 0 if the PKA processing is still active.
 * Returns a value different from 0 when the PKA processing is complete.
 */
extern int HW_PKA_EndOfOperation( void );

/*
 * HW_PKA_ReadSingleOutput
 *
 * Reads one 32-bit word result from the PKA memory.
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 */
extern uint32_t HW_PKA_ReadSingleOutput( uint32_t index );

/*
 * HW_PKA_ReadResult
 *
 * Reads one multi-word result ("size" x 32-bit words) from the PKA memory.
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 */
extern void HW_PKA_ReadResult( uint32_t index,
                               int size,
                               uint32_t* out );

/*
 * HW_PKA_Disable
 *
 * Disables the PKA hardware block.
 * This function disables also the PKA clock and the PKA security.
 * It then releases the PKA semaphore.
 */
extern void HW_PKA_Disable( void );

/*
 * Notes:
 *
 * - this driver uses a semaphore to handle access to the PKA. The index of
 * the semaphore must be configured with CFG_HW_PKA_SEMID.
 */

/* ---------------------------------------------------------------------------
 *                               PKA_P256
 * ---------------------------------------------------------------------------
 */

/*
 * HW_PKA_P256_StartRangeCheck
 *
 * Starts the range check of a point coordinate for P-256 elliptic curve.
 *
 * This function sets the parameters in PKA memory and then starts the
 * processing. The PKA has to be enabled before with HW_PKA_Enable( ).
 * The user must poll on the result availability by calling the
 * HW_PKA_EndOfOperation() function.
 *
 * The input parameter is one the point coordinate. It must be a vector of
 * 8 x 32-bit words (32 bytes).
 *
 * The check result is retrieved by calling HW_PKA_P256_IsRangeCheckOk().
 */
extern void HW_PKA_P256_StartRangeCheck( const uint32_t* coord );

/*
 * HW_PKA_P256_IsRangeCheckOk
 *
 * Reads the result of P-256 range check. This function must only be called
 * when HW_PKA_EndOfOperation() returns a non-zero value.
 *
 * Returns 0 if the check fails ; 1 otherwise.
 */
extern uint32_t HW_PKA_P256_IsRangeCheckOk( void );

/*
 * HW_PKA_P256_StartPointCheck
 *
 * Starts the check of a point for P-256 elliptic curve.
 *
 * This function sets the parameters in PKA memory and then starts the
 * processing. The PKA has to be enabled before with HW_PKA_Enable( ).
 * The user must poll on the result availability by calling the
 * HW_PKA_EndOfOperation() function.
 *
 * The input parameters are the point coordinates. Each parameter must be a
 * vector of 8 x 32-bit words (32 bytes).
 *
 * The check result is retrieved by calling HW_PKA_P256_IsPointCheckOk().
 */
extern void HW_PKA_P256_StartPointCheck( const uint32_t* x,
                                         const uint32_t* y );

/*
 * HW_PKA_P256_IsPointCheckOk
 *
 * Reads the result of P-256 point check. This function must only be called
 * when HW_PKA_EndOfOperation() returns a non-zero value.
 *
 * Returns 0 if the check fails ; 1 otherwise.
 */
extern uint32_t HW_PKA_P256_IsPointCheckOk( void );

/*
 * HW_PKA_P256_StartEccScalarMul
 *
 * Starts the PKA scalar multiplication using the P-256 elliptic curve.
 *
 * This function sets the parameters in PKA memory and then starts the
 * processing. The PKA has to be enabled before with HW_PKA_Enable( ).
 * The user must poll on the result availability by calling the
 * HW_PKA_EndOfOperation() function.
 *
 * The input parameter is the starting point P defined by its 2 coordinates
 * p_x and p_y, and the scalar k. Each parameter must be a vector of 8 x 32-bit
 * words (32 bytes).
 */
extern void HW_PKA_P256_StartEccScalarMul( const uint32_t* k,
                                           const uint32_t* p_x,
                                           const uint32_t* p_y );

/*
 * HW_PKA_P256_ReadEccScalarMul
 *
 * Reads the result of PKA scalar multiplication using the P-256 elliptic
 * curve. This function must only be called when HW_PKA_EndOfOperation()
 * returns a non-zero value.
 *
 * This function retrieves the result from PKA memory: coordinates of point P,
 * p_x and p_y, 8 x 32-bit words each (2 times 32 bytes).
 * Before returning, it disables the PKA block, releasing the PKA semaphore.
 */
extern void HW_PKA_P256_ReadEccScalarMul( uint32_t* p_x,
                                          uint32_t* p_y );

/* ---------------------------------------------------------------------------
 *                                 RNG
 * ---------------------------------------------------------------------------
 */

/*
 * The RNG driver is made to generate the random numbers in background instead
 * of generating them each time they are needed by the application.
 * Thus, the function HW_RNG_Process() must be called regularly in background
 * loop to generate a pool of random numbers. The function HW_RNG_Get() reads
 * the random numbers from the pool.
 * The size of the pool must be configured with CFG_HW_RNG_POOL_SIZE.
 */

/* Error codes definition for HW_RNG return values */
enum
{
  HW_RNG_CLOCK_ERROR = CFG_HW_ERROR_OFFSET + 0x101,
  HW_RNG_NOISE_ERROR = CFG_HW_ERROR_OFFSET + 0x102,
  HW_RNG_UFLOW_ERROR = CFG_HW_ERROR_OFFSET + 0x103,
};

/* RNG_KERNEL_CLK_ON
 *
 * Enable RNG kernel clock.
 */
void RNG_KERNEL_CLK_ON(void);

/* RNG_KERNEL_CLK_OFF
 *
 * Called when RNG kernel clock may be disabled.
 * Weak function to be implemented by user.
 */
void RNG_KERNEL_CLK_OFF(void);

/* HW_RNG_Disable
 *
 * Disables the RNG peripheral and switch off its clock in RCC.
 */
extern void HW_RNG_Disable( void );

/* HW_RNG_Start
 *
 * Starts the generation of random numbers using the RNG IP. This function has
 * to be called only once at reset before calling HW_RNG_Get() to retrieve
 * the generated random values.
 */
extern void HW_RNG_Start( void );

/*
 * HW_RNG_Get
 *
 * Retrieves "n" random 32-bit words.
 * "n" must be in the range [1, CFG_HW_RNG_POOL_SIZE].
 * The random numbers are written in memory from "val" pointer.
 * "val" must point to a sufficient memory buffer allocated by the caller.
 */
extern void HW_RNG_Get( uint8_t n,
                        uint32_t* val );

/*
 * HW_RNG_Process
 *
 * This function must be called in a separate task or in "background" loop.
 * It implements a simple state machine that enables the RNG block,
 * generates random numbers and then disables the RNG.
 * It returns 0 (HW_OK) if the low power mode can be entered.
 * It returns HW_BUSY as long as this function must be called.
 * In error conditions, it returns one of the following error codes:
 * - HW_RNG_CLOCK_ERROR for clock error,
 * - HW_RNG_NOISE_ERROR for noise source error;
 * the hardware must then be reset.
 * - HW_RNG_UFLOW_ERROR in case of pool underflow error.
 */
extern int HW_RNG_Process( void );

/*
 * HW_RNG_EnableClock
 *
 * This function enables the RNG clock for "other user" than RNG driver itself
 */
extern void HW_RNG_EnableClock( uint8_t user_mask );

/*
 * HW_RNG_DisableClock
 *
 * This function disables the RNG clock for "other user" than RNG driver itself
 */
extern void HW_RNG_DisableClock( uint8_t user_mask );

extern void HWCB_RNG_Process( void );

/* ---------------------------------------------------------------------------
 *                               GPIO
 * ---------------------------------------------------------------------------
 */

/* Index definitions used for all GPIO functions */
enum
{
  HW_GPIO_DBG          =  0,
  HW_GPIO_GREEN_LED    = 13,
  HW_GPIO_RED_LED      = 14,
  HW_GPIO_BLUE_LED     = 15,
};

/*
 * HW_GPIO_Init
 *
 * This function initilaizes the GPIO pins used for debug.
 */
extern void HW_GPIO_Init( const uint32_t* dbg_pins );

/*
 * HW_GPIO_Read
 *
 * This function reads the output pin which index is given in parameter.
 * It returns 0 if the pin is low, 1 if the pin is high.
 */
extern uint8_t HW_GPIO_Read( uint8_t index );

/*
 * HW_GPIO_Set
 *
 * This function sets to high level the output pin which index is given
 * in parameter.
 */
extern void HW_GPIO_Set( uint8_t index );

/*
 * HW_GPIO_Reset
 *
 * This function resets to low level the output pin which index is given
 * in parameter.
 */
extern void HW_GPIO_Reset( uint8_t index );

extern void GPIO_SetDebug( int gpio_pin,
                           int pin_value );

/* ---------------------------------------------------------------------------
 *                             OTP
 * ---------------------------------------------------------------------------
 */

typedef __PACKED_STRUCT
{
  uint8_t additional_data[8]; /*!< 64 bits of data to fill OTP slot */
  uint8_t bd_address[6];      /*!< Bluetooth Device Address*/
  uint8_t hsetune;            /*!< Load capacitance to be applied on HSE pad */
  uint8_t index;              /*!< Structure index */
} HW_OTP_data_t;

/*
 * HW_OTP_Read
 *
 * Read the OTP
 *
 */
int HW_OTP_Read( uint8_t index,
                 HW_OTP_data_t** data );

/*
 * HW_OTP_Write
 *
 * ReadWrite the OTP
 *
 */
int HW_OTP_Write( uint8_t* additional_data,
                  uint8_t* bd_address,
                  uint8_t hsetune,
                  uint8_t index );

#endif /* HW_H__ */
