/*
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

 /*
 *  Terminal
 */

// Standard includes
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "UartHelper.h"


//*****************************************************************************
//                          LOCAL DEFINES
//*****************************************************************************


//*****************************************************************************
//                 GLOBAL VARIABLES
//*****************************************************************************
static UART2_Handle uartHandle;

//*****************************************************************************
//
//! Initialization
//!
//! This function
//!        1. Configures the UART to be used.
//!
//! \param  none
//!
//! \return none
//
//*****************************************************************************
UART2_Handle InitUart(void)
{
    UART2_Params uartParams;

    UART2_Params_init(&uartParams);

    uartParams.readReturnMode = UART2_ReadReturnMode_FULL;
    uartParams.baudRate = 115200;

    uartHandle = UART2_open(CONFIG_UART2_0, &uartParams);

    return uartHandle;
}


//*****************************************************************************
//
//! Get a json struct string from UART - no processing of special characters
//!
//! \param[in]  pBuffer   - is the command store to which command will be
//!                           populated
//! \param[in]  BufLen    - is the length of buffer store available
//!
//! \return Length of the bytes received. -1 if buffer length exceeded.
//!
//*****************************************************************************

int32_t UartGetJsonStruct(char *pBuffer, int32_t BufLen)
{
    char cChar = '\0';
    int16_t depth = 0;
    int32_t buffIdx = 0; 
    uint8_t complete = 0;
    while (1) 
    {
        cChar = getch();
        if (cChar == '{') 
        {
            depth++;
        } 
        else if (cChar == '}') 
        {
            depth--;
            if (depth == 0) 
            {
                //got to the closing brackets from the current structure.
                //exit loop and return
                complete = 1;
                //break;
            } 
            else if (depth < 0) 
            {
                //malformed json structure
                //reset depth and continue looking for a '{'
                depth = 0;
                buffIdx = 0;
                return -2;
            }
        } 
        //once we have found a json structure, start storing it in the buffer
        if(depth > 0 || complete)
        {
            if(buffIdx < (BufLen - 1))
            {
                pBuffer[buffIdx++] = cChar;                
                if(complete)
                {
                    pBuffer[buffIdx] = '\0';
                    break;
                }
            }
            else
            {
                //structure is too long, buffer is full
                return -1;
            }
        }
    }

    return buffIdx;
}



//*****************************************************************************
//
//! Read a character from the console
//!
//! \param none
//!
//! \return Character
//
//*****************************************************************************
char getch(void)
{
    char ch;

    UART2_read(uartHandle, &ch, 1, NULL);
    return(ch);
}

