/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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

    File:		mDNSDebug.c

    Contains:	Implementation of debugging utilities. Requires a POSIX environment.

    Version:	1.0

 */
#include "mDNSEmbeddedAPI.h"

#ifdef DEBUG_MDNS
    #include "syslog.h"
    log_create_module(mdns, PRINT_LEVEL_INFO);
#endif
#ifndef REMOVE_LOG_FUNCTIONS

#include "mDNSDebug.h"

#include <stdio.h>

#if defined(WIN32) || defined(EFI32) || defined(EFI64) || defined(EFIX64)
// Need to add Windows/EFI syslog support here
#define LOG_PID 0x01
#define LOG_CONS 0x02
#define LOG_PERROR 0x20
#else
//#include <syslog.h>
#endif



mDNSexport int mDNS_LoggingEnabled       = 0;
mDNSexport int mDNS_PacketLoggingEnabled = 0;
mDNSexport int mDNS_McastLoggingEnabled  = 0;
mDNSexport int mDNS_McastTracingEnabled  = 0; 

#if MDNS_DEBUGMSGS
mDNSexport int mDNS_DebugMode = mDNStrue;
#else
mDNSexport int mDNS_DebugMode = mDNSfalse;
#endif

// Note, this uses mDNS_vsnprintf instead of standard "vsnprintf", because mDNS_vsnprintf knows
// how to print special data types like IP addresses and length-prefixed domain names
#if MDNS_DEBUGMSGS > 1
mDNSexport void verbosedebugf_(const char *format, ...)
{
    char buffer[512];
    va_list ptr;
    va_start(ptr,format);
    buffer[mDNS_vsnprintf(buffer, sizeof(buffer), format, ptr)] = 0;
    va_end(ptr);
    mDNSPlatformWriteDebugMsg(buffer);
}
#endif

// Log message with default "mDNSResponder" ident string at the start
mDNSlocal void LogMsgWithLevelv(mDNSLogLevel_t logLevel, const char *format, va_list ptr)
{
    char buffer[512];
    buffer[mDNS_vsnprintf((char *)buffer, sizeof(buffer), format, ptr)] = 0;
    mDNSPlatformWriteLogMsg(ProgramName, buffer, logLevel);
}

#define LOG_HELPER_BODY(L) \
    { \
        va_list ptr; \
        va_start(ptr,format); \
        LogMsgWithLevelv(L, format, ptr); \
        va_end(ptr); \
    }

// see mDNSDebug.h
#if !MDNS_HAS_VA_ARG_MACROS
void LogMsg_(const char *format, ...)       LOG_HELPER_BODY(MDNS_LOG_MSG)
void LogOperation_(const char *format, ...) LOG_HELPER_BODY(MDNS_LOG_OPERATION)
void LogSPS_(const char *format, ...)       LOG_HELPER_BODY(MDNS_LOG_SPS)
void LogInfo_(const char *format, ...)      LOG_HELPER_BODY(MDNS_LOG_INFO)
#endif

#if MDNS_DEBUGMSGS
void debugf_(const char *format, ...)       LOG_HELPER_BODY(MDNS_LOG_DEBUG)
#endif

// Log message with default "mDNSResponder" ident string at the start
mDNSexport void LogMsgWithLevel(mDNSLogLevel_t logLevel, const char *format, ...)
LOG_HELPER_BODY(logLevel)

#else
	mDNSexport int mDNS_PacketLoggingEnabled = 0;
	mDNSexport int mDNS_McastLoggingEnabled  = 0;
#endif //#ifndef REMOVE_LOG_FUNCTIONS


