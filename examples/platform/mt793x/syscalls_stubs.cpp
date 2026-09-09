/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
    This file is only used to implement weak syscall stubs that
    gcc-arm-none-eabi 12.2.1 expects to link when using Libc
    (newlib/libc_nano).  Without these stubs the newlib stubs themselves
    are linked and emit "_xxx is not implemented and will always fail"
    warnings, which become fatal under -Wl,--fatal-warnings.

    The stubs below are declared __attribute__((weak)) so the existing
    link_wrapper.c overrides (e.g. -Wl,--wrap=_write) continue to win.
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

int _open(char * path, int flags, ...);
int _close(int file);
int _fstat(int file, struct stat * st);
int _isatty(int file);
int _lseek(int file, int ptr, int dir);
int _read(int file, char * ptr, int len);
int _write(int file, const char * ptr, int len);

int __attribute__((weak)) _open(char * path, int flags, ...)
{
    (void) path;
    (void) flags;
    return -1;
}

int __attribute__((weak)) _close(int file)
{
    (void) file;
    return 0;
}

void __attribute__((weak)) _exit(int status)
{
    (void) status;
    while (1)
    {
    }
}

int __attribute__((weak)) _fstat(int file, struct stat * st)
{
    (void) file;
    (void) st;
    return 0;
}

int __attribute__((weak)) _getpid(void)
{
    return 1;
}

int __attribute__((weak)) _isatty(int file)
{
    (void) file;
    return 1;
}

int __attribute__((weak)) _kill(int pid, int sig)
{
    (void) pid;
    (void) sig;
    return -1;
}

int __attribute__((weak)) _lseek(int file, int ptr, int dir)
{
    (void) file;
    (void) ptr;
    (void) dir;
    return 0;
}

int __attribute__((weak)) _read(int file, char * ptr, int len)
{
    (void) file;
    (void) ptr;
    (void) len;
    return 0;
}

int __attribute__((weak)) _write(int file, const char * ptr, int len)
{
    (void) file;
    (void) ptr;
    return len;
}

/*
 * newlib-nano omits these reent/stdio symbols, but the prebuilt
 * matter_sdk.a (built against the full newlib) references them from
 * FreeRTOS tasks.c (prvInitialiseNewTask) and mt7933 hal_nvic.c
 * (isrC_main).  Provide weak zero-initialized stubs so the link
 * succeeds under --specs=nano.specs.  lighting-app does not
 * exercise per-task stdio, so the stubs are never actually read;
 * if a stronger definition ever appears (e.g. from a future SDK
 * rebuild against nano libc) it will override these.
 */
__attribute__((weak)) struct _reent * _global_impure_ptr = 0;
__attribute__((weak)) FILE __sf_fake_stdin               = { 0 };
__attribute__((weak)) FILE __sf_fake_stdout              = { 0 };
__attribute__((weak)) FILE __sf_fake_stderr              = { 0 };

void __attribute__((weak)) exit(int status)
{
    /*
     * exit() is not meaningful in a bare-metal RTOS image.  Silently
     * resetting the chip via __NVIC_SystemReset() would make any stray
     * exit() invocation indistinguishable from a watchdog reboot.
     * Hang here instead so the watchdog (or an attached debugger)
     * surfaces the call site.
     */
    (void) status;
    while (1)
    {
    }
}

#ifdef __cplusplus
}
#endif
