/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

/* Includes */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>

/* Variables */
// #undef errno
extern int errno;
extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

register char * stack_ptr asm("sp");

char * __env[1] = { 0 };
char ** environ = __env;

/* Functions */
void initialise_monitor_handles() {}

int _getpid(void)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

void _exit(int status)
{
    _kill(status, -1);
    while (1)
    {
    } /* Make sure we hang here */
}

__attribute__((weak)) int _read(int file, char * ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        *ptr++ = __io_getchar();
    }

    return len;
}

__attribute__((weak)) int _write(int file, char * ptr, int len)
{
    int DataIdx;

    for (DataIdx = 0; DataIdx < len; DataIdx++)
    {
        __io_putchar(*ptr++);
    }
    return len;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat * st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int file)
{
    return 1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(char * path, int flags, ...)
{
    /* Pretend like we always fail */
    return -1;
}

int _wait(int * status)
{
    errno = ECHILD;
    return -1;
}

int _unlink(char * name)
{
    errno = ENOENT;
    return -1;
}

int _times(struct tms * buf)
{
    return -1;
}

int _stat(char * file, struct stat * st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _link(char * old, char * new)
{
    errno = EMLINK;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _execve(char * name, char ** argv, char ** env)
{
    errno = ENOMEM;
    return -1;
}

int _gettimeofday(struct timeval * tv, void * tzvp)
{
    errno = ENOMEM;
    return -1;
}

int _getentropy(void * buffer, size_t length)
{
    errno = ENOMEM;
    return -1;
}
