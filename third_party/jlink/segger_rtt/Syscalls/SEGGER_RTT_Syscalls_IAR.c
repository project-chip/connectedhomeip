/*********************************************************************
*                    SEGGER Microcontroller GmbH                     *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*            (c) 1995 - 2019 SEGGER Microcontroller GmbH             *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
*                                                                    *
*       SEGGER RTT * Real Time Transfer for embedded targets         *
*                                                                    *
**********************************************************************
*                                                                    *
* All rights reserved.                                               *
*                                                                    *
* SEGGER strongly recommends to not make any changes                 *
* to or modify the source code of this software in order to stay     *
* compatible with the RTT protocol and J-Link.                       *
*                                                                    *
* Redistribution and use in source and binary forms, with or         *
* without modification, are permitted provided that the following    *
* condition is met:                                                  *
*                                                                    *
* o Redistributions of source code must retain the above copyright   *
*   notice, this condition and the following disclaimer.             *
*                                                                    *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND             *
* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,        *
* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF           *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           *
* DISCLAIMED. IN NO EVENT SHALL SEGGER Microcontroller BE LIABLE FOR *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR           *
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT  *
* OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;    *
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT          *
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE  *
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH   *
* DAMAGE.                                                            *
*                                                                    *
**********************************************************************
---------------------------END-OF-HEADER------------------------------
File    : SEGGER_RTT_Syscalls_IAR.c
Purpose : Low-level functions for using printf() via RTT in IAR.
          To use RTT for printf output, include this file in your
          application and set the Library Configuration to Normal.
Revision: $Rev: 17697 $
----------------------------------------------------------------------
*/
#ifdef __IAR_SYSTEMS_ICC__

//
// Since IAR EWARM V8 and EWRX V4, yfuns.h is considered as deprecated and LowLevelIOInterface.h
// shall be used instead. To not break any compatibility with older compiler versions, we have a
// version check in here.
//
#if ((defined __ICCARM__) && (__VER__ >= 8000000)) || ((defined __ICCRX__) && (__VER__ >= 400))
#include <LowLevelIOInterface.h>
#else
#include <yfuns.h>
#endif

#include "SEGGER_RTT.h"
#pragma module_name = "?__write"

/*********************************************************************
 *
 *       Function prototypes
 *
 **********************************************************************
 */
size_t __write(int handle, const unsigned char * buffer, size_t size);

/*********************************************************************
 *
 *       Global functions
 *
 **********************************************************************
 */
/*********************************************************************
 *
 *       __write()
 *
 * Function description
 *   Low-level write function.
 *   Standard library subroutines will use this system routine
 *   for output to all files, including stdout.
 *   Write data via RTT.
 */
size_t __write(int handle, const unsigned char * buffer, size_t size)
{
    (void) handle; /* Not used, avoid warning */
    SEGGER_RTT_Write(0, (const char *) buffer, size);
    return size;
}

/*********************************************************************
 *
 *       __write_buffered()
 *
 * Function description
 *   Low-level write function.
 *   Standard library subroutines will use this system routine
 *   for output to all files, including stdout.
 *   Write data via RTT.
 */
size_t __write_buffered(int handle, const unsigned char * buffer, size_t size)
{
    (void) handle; /* Not used, avoid warning */
    SEGGER_RTT_Write(0, (const char *) buffer, size);
    return size;
}

#endif
/****** End Of File *************************************************/
