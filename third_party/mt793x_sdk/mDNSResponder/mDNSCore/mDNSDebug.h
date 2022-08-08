/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2003 Apple Computer, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __mDNSDebug_h
#define __mDNSDebug_h

// Set MDNS_DEBUGMSGS to 0 to optimize debugf() calls out of the compiled code
// Set MDNS_DEBUGMSGS to 1 to generate normal debugging messages
// Set MDNS_DEBUGMSGS to 2 to generate verbose debugging messages
// MDNS_DEBUGMSGS is normally set in the project options (or makefile) but can also be set here if desired
// (If you edit the file here to turn on MDNS_DEBUGMSGS while you're debugging some code, be careful
// not to accidentally check-in that change by mistake when you check in your other changes.)

#undef MDNS_DEBUGMSGS
//#define MDNS_DEBUGMSGS 2
#define MDNS_DEBUGMSGS 0

// Set MDNS_CHECK_PRINTF_STYLE_FUNCTIONS to 1 to enable extra GCC compiler warnings
// Note: You don't normally want to do this, because it generates a bunch of
// spurious warnings for the following custom extensions implemented by mDNS_vsnprintf:
//    warning: `#' flag used with `%s' printf format    (for %#s              -- pascal string format)
//    warning: repeated `#' flag in format              (for %##s             -- DNS name string format)
//    warning: double format, pointer arg (arg 2)       (for %.4a, %.16a, %#a -- IP address formats)
#define MDNS_CHECK_PRINTF_STYLE_FUNCTIONS 1

#ifdef DEBUG_MDNS
#include "syslog.h"
#include <stdio.h>
// #define mdnslogInfo(M, ...)       LOG_I(mdns, M, ##__VA_ARGS__)
// #define mdnslogWarning(M, ...) LOG_W(mdns, M, ##__VA_ARGS__)
// #define mdnslogError(M, ...) LOG_E(mdns, M, ##__VA_ARGS__)
// #define mdnslogDebug(M, ...) LOG_I(mdns, M, ##__VA_ARGS__)
#define mdnslogInfo(M, ...) printf(M "\n", ##__VA_ARGS__)
#define mdnslogWarning(M, ...) printf(M "\n", ##__VA_ARGS__)
#define mdnslogError(M, ...) printf(M "\n", ##__VA_ARGS__)
#define mdnslogDebug(M, ...) printf(M "\n", ##__VA_ARGS__)
#else
#define mdnslogInfo(M, ...)
#define mdnslogWarning(M, ...)
#define mdnslogError(M, ...)
#define mdnslogDebug(M, ...)
#endif

typedef enum
{
    MDNS_LOG_MSG,
    MDNS_LOG_OPERATION,
    MDNS_LOG_SPS,
    MDNS_LOG_INFO,
    MDNS_LOG_DEBUG,
} mDNSLogLevel_t;

// Set this symbol to 1 to answer remote queries for our Address, reverse mapping PTR, and HINFO records
#define ANSWER_REMOTE_HOSTNAME_QUERIES 0

// Set this symbol to 1 to do extra debug checks on malloc() and free()
// Set this symbol to 2 to write a log message for every malloc() and free()
//#define MACOSX_MDNS_MALLOC_DEBUGGING 1

//#define ForceAlerts 1
//#define LogTimeStamps 1

// Developer-settings section ends here

#if MDNS_CHECK_PRINTF_STYLE_FUNCTIONS
#define IS_A_PRINTF_STYLE_FUNCTION(F, A) __attribute__((format(printf, F, A)))
#else
#define IS_A_PRINTF_STYLE_FUNCTION(F, A)
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Variable argument macro support. Use ANSI C99 __VA_ARGS__ where possible. Otherwise, use the next best thing.

#if (defined(__GNUC__))
#if ((__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 2)))
#define MDNS_C99_VA_ARGS 1
#define MDNS_GNU_VA_ARGS 0
#else
#define MDNS_C99_VA_ARGS 0
#define MDNS_GNU_VA_ARGS 1
#endif
#define MDNS_HAS_VA_ARG_MACROS 1
#elif (_MSC_VER >= 1400) // Visual Studio 2005 and later
#define MDNS_C99_VA_ARGS 1
#define MDNS_GNU_VA_ARGS 0
#define MDNS_HAS_VA_ARG_MACROS 1
#elif (defined(__MWERKS__))
#define MDNS_C99_VA_ARGS 1
#define MDNS_GNU_VA_ARGS 0
#define MDNS_HAS_VA_ARG_MACROS 1
#else
#define MDNS_C99_VA_ARGS 0
#define MDNS_GNU_VA_ARGS 0
#define MDNS_HAS_VA_ARG_MACROS 0
#endif

//#define REMOVE_DEBUG_LogMsg
#ifndef REMOVE_DEBUG_LogMsg
#if (MDNS_HAS_VA_ARG_MACROS)
#if (MDNS_C99_VA_ARGS)
#define debug_noop(...) ((void) 0)
#define LogMsg(...) LogMsgWithLevel(MDNS_LOG_MSG, __VA_ARGS__)
#define LogOperation(...)                                                                                                          \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mDNS_LoggingEnabled)                                                                                                   \
            LogMsgWithLevel(MDNS_LOG_OPERATION, __VA_ARGS__);                                                                      \
    } while (0)
#define LogSPS(...)                                                                                                                \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mDNS_LoggingEnabled)                                                                                                   \
            LogMsgWithLevel(MDNS_LOG_SPS, __VA_ARGS__);                                                                            \
    } while (0)
#define LogInfo(...)                                                                                                               \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mDNS_LoggingEnabled)                                                                                                   \
            LogMsgWithLevel(MDNS_LOG_INFO, __VA_ARGS__);                                                                           \
    } while (0)
#elif (MDNS_GNU_VA_ARGS)
#define debug_noop(ARGS...) ((void) 0)
#define LogMsg(ARGS...) LogMsgWithLevel(MDNS_LOG_MSG, ARGS)
#define LogOperation(ARGS...)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mDNS_LoggingEnabled)                                                                                                   \
            LogMsgWithLevel(MDNS_LOG_OPERATION, ARGS);                                                                             \
    } while (0)
#define LogSPS(ARGS...)                                                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mDNS_LoggingEnabled)                                                                                                   \
            LogMsgWithLevel(MDNS_LOG_SPS, ARGS);                                                                                   \
    } while (0)
#define LogInfo(ARGS...)                                                                                                           \
    do                                                                                                                             \
    {                                                                                                                              \
        if (mDNS_LoggingEnabled)                                                                                                   \
            LogMsgWithLevel(MDNS_LOG_INFO, ARGS);                                                                                  \
    } while (0)
#else
#error Unknown variadic macros
#endif
#else
// If your platform does not support variadic macros, you need to define the following variadic functions.
// See mDNSShared/mDNSDebug.c for sample implementation
#define debug_noop 1 ? (void) 0 : (void)
#define LogMsg LogMsg_
#define LogOperation (mDNS_LoggingEnabled == 0) ? ((void) 0) : LogOperation_
#define LogSPS (mDNS_LoggingEnabled == 0) ? ((void) 0) : LogSPS_
#define LogInfo (mDNS_LoggingEnabled == 0) ? ((void) 0) : LogInfo_
extern void LogMsg_(const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(1, 2);
extern void LogOperation_(const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(1, 2);
extern void LogSPS_(const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(1, 2);
extern void LogInfo_(const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(1, 2);
#endif

#if MDNS_DEBUGMSGS
#define debugf debugf_
extern void debugf_(const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(1, 2);
#else
#define debugf debug_noop
#endif

#if MDNS_DEBUGMSGS > 1
#define verbosedebugf verbosedebugf_
extern void verbosedebugf_(const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(1, 2);
#else
#define verbosedebugf debug_noop
#endif
#else
#define debug_noop(...) ((void) 0)
#define LogMsg(...) ((void) 0)
#define LogOperation(...) ((void) 0)
#define LogSPS(...) ((void) 0)
#define LogInfo(...) ((void) 0)

#define debugf(...) ((void) 0)
#define verbosedebugf(...) ((void) 0)
#endif // #ifndef REMOVE_DEBUG_LogMsg

extern int mDNS_LoggingEnabled;
extern int mDNS_PacketLoggingEnabled;
extern int mDNS_McastLoggingEnabled;
extern int mDNS_McastTracingEnabled;
extern int mDNS_DebugMode; // If non-zero, LogMsg() writes to stderr instead of syslog
extern const char ProgramName[];

extern void LogMsgWithLevel(mDNSLogLevel_t logLevel, const char * format, ...) IS_A_PRINTF_STYLE_FUNCTION(2, 3);
// LogMsgNoIdent needs to be fixed so that it logs without the ident prefix like it used to
// (or completely overhauled to use the new "log to a separate file" facility)
#define LogMsgNoIdent LogMsg

#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING >= 1
extern void * mallocL(char * msg, unsigned int size);
extern void freeL(char * msg, void * x);
extern void LogMemCorruption(const char * format, ...);
extern void uds_validatelists(void);
extern void udns_validatelists(void * const v);
#else
#define mallocL(X, Y) malloc(Y)
#define freeL(X, Y) free(Y)
#endif

#ifdef __cplusplus
}
#endif

#endif
