/******************************************************************************

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************
 
 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/


#ifndef RTOS_PLATFORM_H_
#define RTOS_PLATFORM_H_

#include <stdint.h>
#include <openthread/instance.h>

#ifdef __cplusplus
extern "C" {
#endif

// Global OpenThread instance structure
extern otInstance *sInstance;

/**
 * Initialize the platform support modules.
 */
void otSysInit(int argc, char *argv[]);

/**
 * Process the platform drivers.
 */
void otSysProcessDrivers(otInstance * aInstance);

/**
 * Initialize the debug uart
 */
void platformDebugUartInit(void);

/**
 * This method initializes the alarm service used by OpenThread.
 *
 */
void platformAlarmInit(void);

/**
 * Signal the processing loop to process the alarm module.
 *
 */
void platformAlarmSignal(void);

/**
 * This method performs alarm driver processing.
 *
 */
void platformAlarmProcess(otInstance *aInstance);

/**
 * This method initializes the alarm service used by OpenThread.
 *
 */
void platformAlarmMicroInit(void);

/**
 * Signal the processing loop to process the alarm module.
 *
 */
void platformAlarmMicroSignal(void);

/**
 * This method performs alarm driver processing.
 *
 */
void platformAlarmMicroProcess(otInstance *aInstance);

/**
 * This method initializes the radio service used by OpenThread.
 *
 */
void platformRadioInit(void);

/**
 * Callback from the radio module indicating need for processing.
 */
void platformRadioSignal();

/**
 * This method performs radio driver processing.
 *
 */
void platformRadioProcess(otInstance *aInstance);

/**
 * This method initializes the random number service used by OpenThread.
 *
 */
void platformRandomInit(void);

/**
 * Callback from random module indicating need for processing.
 */
void platformRandomSignal(void);

/**
 * This method performs random driver processing.
 *
 */
void platformRandomProcess(void);

/**
 * Signal the processing loop to process the uart module.
 *
 */
void platformUartSignal();

/**
 * This method performs uart driver processing.
 *
 */
void platformUartProcess();

/**
 * Signal the processing loop to process the spi module.
 *
 */
void platformSpiSignal(void);

/**
 * This method performs spi driver processing.
 *
 */
void platformSpiProcess(void);

/**
 * This method performs UART Flush.
 *
 */
otError otPlatUartFlush(void);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // RTOS_PLATFORM_H_
