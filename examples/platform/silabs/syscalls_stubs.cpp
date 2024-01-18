/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/*
    This file is only used to implement weak syscall stubs
    that gcc-arm-none-eabi 12.2.1 expect to link when using Libc
    (newlib/libc_nano)
*/

#include <reent.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef __cplusplus
extern "C" {
#endif

#if SILABS_LOG_OUT_UART
#include "uart.h"
#endif

int _close(int file);
int _fstat(int file, struct stat * st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _read(int file, char * ptr, int len);
int _write(int file, const char * ptr, int len);

/**************************************************************************
 * @brief
 *  Close a file.
 *
 * @param[in] file
 *  File you want to close.
 *
 * @return
 *  Returns 0 when the file is closed.
 **************************************************************************/
int __attribute__((weak)) _close(int file)
{
    (void) file;
    return 0;
}

/**************************************************************************
 * @brief Exit the program.
 * @param[in] status The value to return to the parent process as the
 *            exit status (not used).
 **************************************************************************/
void __attribute__((weak)) _exit(int status)
{
    (void) status;
    while (1)
    {
    } /* Hang here forever... */
}

/*************************************************************************
 * @brief
 *  Status of an open file.
 *
 * @param[in] file
 *  Check status for this file.
 *
 * @param[in] st
 *  Status information.
 *
 * @return
 *  Returns 0 when st_mode is set to character special.
 ************************************************************************/
int __attribute__((weak)) _fstat(int file, struct stat * st)
{
    (void) file;
    (void) st;
    return 0;
}

/**************************************************************************
 * @brief Get process ID.
 *************************************************************************/
int __attribute__((weak)) _getpid(void)
{
    return 1;
}

/**************************************************************************
 * @brief
 *  Query whether output stream is a terminal.
 *
 * @param[in] file
 *  Descriptor for the file.
 *
 * @return
 *  Returns 1 when query is done.
 **************************************************************************/
int __attribute__((weak)) _isatty(int file)
{
    (void) file;
    return 1;
}

/**************************************************************************
 * @brief Send signal to process.
 * @param[in] pid Process id (not used).
 * @param[in] sig Signal to send (not used).
 *************************************************************************/
int __attribute__((weak)) _kill(int pid, int sig)
{
    (void) pid;
    (void) sig;
    return -1;
}

/**************************************************************************
 * @brief
 *  Set position in a file.
 *
 * @param[in] file
 *  Descriptor for the file.
 *
 * @param[in] ptr
 *  Poiter to the argument offset.
 *
 * @param[in] dir
 *  Directory whence.
 *
 * @return
 *  Returns 0 when position is set.
 *************************************************************************/
int __attribute__((weak)) _lseek(int file, int ptr, int dir)
{
    (void) file;
    (void) ptr;
    (void) dir;
    return 0;
}

/**************************************************************************
 * @brief
 *  Read from a file.
 *
 * @param[in] file
 *  Descriptor for the file you want to read from.
 *
 * @param[in] ptr
 *  Pointer to the chacaters that are beeing read.
 *
 * @param[in] len
 *  Number of characters to be read.
 *
 * @return
 *  Number of characters that have been read.
 *************************************************************************/
int __attribute__((weak)) _read(int file, char * ptr, int len)
{
    (void) file;
#if SILABS_LOG_OUT_UART
    return uartConsoleRead(ptr, len);
#else
    (void) ptr;
    (void) len;
#endif
    return 0;
}

/**************************************************************************
 * @brief
 *  Write to a file.
 *
 * @param[in] file
 *  Descriptor for the file you want to write to.
 *
 * @param[in] ptr
 *  Pointer to the text you want to write
 *
 * @param[in] len
 *  Number of characters to be written.
 *
 * @return
 *  Number of characters that have been written.
 **************************************************************************/
int __attribute__((weak)) _write(int file, const char * ptr, int len)
{
    (void) file;
#if SILABS_LOG_OUT_UART
    uartConsoleWrite(ptr, len);
#else
    (void) ptr;
#endif

    return len;
}

#ifdef __cplusplus
}
#endif
