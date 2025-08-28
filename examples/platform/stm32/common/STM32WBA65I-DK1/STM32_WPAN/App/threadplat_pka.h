/**
  ******************************************************************************
  * @file    threadplat_pka.h
  * @author  MCD Application Team
  * @brief
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

#ifndef THREADPLAT_PKA_H_
#define THREADPLAT_PKA_H_

#include <stdint.h>
#include "mbedtls/bignum.h"

//define
#define OTPLAT_PKA_IT 0


#if (OTPLAT_PKA_IT == 0)
/*
 * otPlatPkaProccessLoop
 *
 * Call this function in loop task when polling for PKA end of operation
 *
 */    
void otPlatPkaProccessLoop(void);


/*
 * This function is called to schedule otPlatPkaProccessLoop()
 *
 *
 */
void APP_THREAD_SchedulePka(void);

/*
 * This function is called to wait for Pka end of operation without blocking shceduling
 *  Semaphore, wait event
 *
 *
 */
void APP_THREAD_WaitPkaEndOfOperation(void);

/*
 * This function is called to post Pka end of operation 
 *  Semaphore, wait event
 *
 *
 */
void APP_THREAD_PostPkaEndOfOperation(void);
#endif


void otPlatPkaProccessLoop(void);


/*
 * otPlatPkaEnable
 *
 * Enables the PKA hardware block.
 * If the driver is already in used, the function returns 0 immediately.
 * If the PKA semaphore is available, this function locks the PKA semaphore,
 * otherwise it returns 0.
 * Then, when PKA semaphore is locked, the function enables PKA security,
 * PKA clock and the block itself.
 * This function must not be directly called when using P-256 elliptic curve
 * dedicated functions.
 * Good example :
 * otPlatPkaEnable -> otPlatPkaStart -> otPlatPkaEndOfOperation -> otPlatPkaDisable
 * Be careful: no re-entrance is ensured for the otPlatPka functions
 */
extern int otPlatPkaEnable(void);

/*
 * otPlatPkaP256StartEccScalarMul
 *
 * Starts the PKA scalar multiplication using the P-256 elliptic curve.
 *
 * This function sets the parameters in PKA memory and then starts the
 * processing. The PKA has to be enabled before with otPlatEnable( ).
 * The user must poll on the result availability by calling the
 * otPlatEndOfOperation() function.
 *
 * The input parameter is the starting point P defined by its 2 coordinates
 * p_x and p_y, and the scalar k. Each parameter must be a vector of 8 x 32-bit
 * words (32 bytes).
 */
extern void otPlatPkaP256StartEccScalarMul (const uint32_t* k,
                                            const uint32_t* p_x,
                                            const uint32_t* p_y);

/*
 * otPlatEndOfOperation, must be called after otPlatEnable()
 *
 * Returns 0 if the PKA processing is still active.
 * Returns a value different from 0 when the PKA processing is complete.
 */
extern int otPlatEndOfOperation(void);


/*
 * otPlatPkaP256ReadEccScalarMul
 *
 * Reads the result of PKA scalar multiplication using the P-256 elliptic
 * curve. This function must only be called when otPlatEndOfOperation()
 * returns a non-zero value.
 *
 * This function retrieves the result from PKA memory: coordinates of point P,
 * p_x and p_y, 8 x 32-bit words each (2 times 32 bytes).
 * 
 */
extern void otPlatPkaP256ReadEccScalarMul(uint32_t* p_x,
                                          uint32_t* p_y);

/*
 * otPlatPkaMpiInvMod
 *
 * Use PKA to calculate modular inversion X = A^-1 mod N
 * X output
 * A first operand
 * N modulus
 */
int otPlatPkaMpiInvMod(mbedtls_mpi* X,const mbedtls_mpi* A,const mbedtls_mpi* N);

/*
 * otPlatDisable
 *
 * Disables the PKA hardware block.
 * This function disables also PKA clock, PKA security and PKA interrupt.
 * It then releases the PKA semaphore.
 */
extern void otPlatPkaDisable(void);
#endif  // THREADPLAT_PKA_H_
