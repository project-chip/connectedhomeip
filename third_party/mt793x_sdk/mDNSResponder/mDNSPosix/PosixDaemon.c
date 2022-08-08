/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2004 Apple Computer, Inc. All rights reserved.
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

    File:		daemon.c

    Contains:	main & associated Application layer for mDNSResponder on Linux.

 */

#if __APPLE__
// In Mac OS X 10.5 and later trying to use the daemon function gives a 鈥溾�榙aemon鈥� is deprecated鈥�
// error, which prevents compilation because we build with "-Werror".
// Since this is supposed to be portable cross-platform code, we don't care that daemon is
// deprecated on Mac OS X 10.5, so we use this preprocessor trick to eliminate the error message.
#define daemon yes_we_know_that_daemon_is_deprecated_in_os_x_10_5_thankyou
#endif

#include "mdns.h"

#include <stdio.h>
#include <string.h>

#include <stdlib.h>
//#include <signal.h>
#include <errno.h>
//#include <fcntl.h>
#include "FreeRTOS.h"
#include "task.h"

#if __APPLE__
#undef daemon
extern int daemon(int, int);
#endif

#include "PlatformCommon.h"
#include "mDNSEmbeddedAPI.h"
#include "mDNSPosix.h"
#include "mDNSUNP.h" // For daemon()
#include "uds_daemon.h"

#define CONFIG_FILE "/etc/mdnsd.conf"
static domainname DynDNSZone; // Default wide-area zone for service registration
static domainname DynDNSHostname;

#ifdef REDUCE_RR_CACHE_SIZE
#define RR_CACHE_SIZE 10
#else
#define RR_CACHE_SIZE 500
#endif
static CacheEntity gRRCache[RR_CACHE_SIZE];
static mDNS_PlatformSupport PlatformStorage;

void mDNS_UpdateInterface()
{
    mDNSPlatformPosixRefreshInterfaceList(&mDNSStorage);
}

mDNSlocal void mDNS_StatusCallback(mDNS * const m, mStatus result)
{
    (void) m; // Unused
    if (result == mStatus_NoError)
    {
        // On successful registration of dot-local mDNS host name, daemon may want to check if
        // any name conflict and automatic renaming took place, and if so, record the newly negotiated
        // name in persistent storage for next time. It should also inform the user of the name change.
        // On Mac OS X we store the current dot-local mDNS host name in the SCPreferences store,
        // and notify the user with a CFUserNotification.
    }
    else if (result == mStatus_ConfigChanged)
    {
        udsserver_handle_configchange(m);
    }
    else if (result == mStatus_GrowCache)
    {
        // Allocate another chunk of cache storage
        CacheEntity * storage = malloc(sizeof(CacheEntity) * RR_CACHE_SIZE);
        if (storage)
            mDNS_GrowCache(m, storage, RR_CACHE_SIZE);
    }
}

// %%% Reconfigure() probably belongs in the platform support layer (mDNSPosix.c), not the daemon cde
// -- all client layers running on top of mDNSPosix.c need to handle network configuration changes,
// not only the Unix Domain Socket Daemon

static void Reconfigure(mDNS * m)
{
    mDNSAddr DynDNSIP;
    const mDNSAddr dummy = { mDNSAddrType_IPv4, { { { 1, 1, 1, 1 } } } };
    ;
    mDNS_SetPrimaryInterfaceInfo(m, NULL, NULL, NULL);
    //    if (ParseDNSServers(m, uDNS_SERVERS_FILE) < 0)
    //        LogMsg("Unable to parse DNS server list. Unicast DNS-SD unavailable");
    ReadDDNSSettingsFromConfFile(m, CONFIG_FILE, &DynDNSHostname, &DynDNSZone, NULL);
    mDNSPlatformSourceAddrForDest(&DynDNSIP, &dummy);
    if (DynDNSHostname.c[0])
        mDNS_AddDynDNSHostName(m, &DynDNSHostname, NULL, NULL);
    if (DynDNSIP.type)
        mDNS_SetPrimaryInterfaceInfo(m, &DynDNSIP, NULL, NULL);
    mDNS_ConfigChanged(m);
}

// Do appropriate things at startup with command line arguments. Calls exit() if unhappy.
#if 0
mDNSlocal void ParseCmdLinArgs(int argc, char **argv)
{
    if (argc > 1)
    {
        if (0 == strcmp(argv[1], "-debug")) mDNS_DebugMode = mDNStrue;
        else printf("Usage: %s [-debug]\n", argv[0]);
    }

    if (!mDNS_DebugMode)
    {
        int result = daemon(0, 0);
        if (result != 0) { LogMsg("Could not run as daemon - exiting"); exit(result); }
#if __APPLE__
        LogMsg("The POSIX mdnsd should only be used on OS X for testing - exiting");
        exit(-1);
#endif
    }
}
#endif

#ifndef REMOVE_LOG_FUNCTIONS
mDNSlocal void DumpStateLog(mDNS * const m)
// Dump a little log of what we've been up to.
{
    LogMsg("---- BEGIN STATE LOG ----");
    udsserver_info(m);
    LogMsg("----  END STATE LOG  ----");
}
#endif // #ifndef REMOVE_LOG_FUNCTIONS

// for mdnsd_stop
static int mStoped = 0;
void setStoped()
{
    mStoped = 1;
}

mDNSlocal mStatus MainLoop(mDNS * m) // Loop until we quit.
{
    //    sigset_t signals;
    mDNSBool gotData = mDNSfalse;

    //    mDNSPosixListenForSignalInEventLoop(SIGINT);
    //    mDNSPosixListenForSignalInEventLoop(SIGTERM);
    //    mDNSPosixListenForSignalInEventLoop(SIGUSR1);
    //    mDNSPosixListenForSignalInEventLoop(SIGPIPE);
    //    mDNSPosixListenForSignalInEventLoop(SIGHUP) ;

    for (;;)
    {
        // Work out how long we expect to sleep before the next scheduled task
        struct timeval timeout;
        mDNSs32 ticks;

        // Only idle if we didn't find any data the last time around
        if (!gotData)
        {
            mDNSs32 nextTimerEvent = mDNS_Execute(m);
            nextTimerEvent         = udsserver_idle(nextTimerEvent);
            ticks                  = nextTimerEvent - mDNS_TimeNow(m);
            if (ticks < 1)
                ticks = 1;
        }
        else // otherwise call EventLoop again with 0 timemout
            ticks = 0;

        // FIX for WSAP00000869 : LWIP select() in mDNSPosixRunEventLoopOnce
        // will be stuck if idle for too long
        if (ticks > 10000)
            ticks = 10000;

        timeout.tv_sec  = ticks / mDNSPlatformOneSecond;
        timeout.tv_usec = (ticks % mDNSPlatformOneSecond) * 1000000 / mDNSPlatformOneSecond;

        //        (void) mDNSPosixRunEventLoopOnce(m, &timeout, &signals, &gotData);
        (void) mDNSPosixRunEventLoopOnce(m, &timeout, NULL, &gotData);

#if 0
        if (sigismember(&signals, SIGHUP )) Reconfigure(m);
        if (sigismember(&signals, SIGUSR1)) DumpStateLog(m);
        // SIGPIPE happens when we try to write to a dead client; death should be detected soon in request_callback() and cleaned up.
        if (sigismember(&signals, SIGPIPE)) LogMsg("Received SIGPIPE - ignoring");
        if (sigismember(&signals, SIGINT) || sigismember(&signals, SIGTERM)) break;
#endif
        if (mStoped == 1)
        {
            break;
        }
    }
    mStoped = 0;
    //    return EINTR;
    return 0;
}

// int main(int argc, char **argv)
int MainDaemonTask(int argc, char ** argv)
{
    mStatus err;

    //    ParseCmdLinArgs(argc, argv);

    LogMsg("%s starting", mDNSResponderVersionString);

    //    err = mDNS_Init(&mDNSStorage, &PlatformStorage, gRRCache, RR_CACHE_SIZE, mDNS_Init_AdvertiseLocalAddresses,
    //                    mDNS_StatusCallback, mDNS_Init_NoInitCallbackContext);

    err = mDNS_Init(&mDNSStorage, &PlatformStorage, NULL, 0, mDNS_Init_AdvertiseLocalAddresses, NULL,
                    mDNS_Init_NoInitCallbackContext);

    if (mStatus_NoError == err)
        err = udsserver_init(mDNSNULL, 0);

    //    Reconfigure(&mDNSStorage);

    // Now that we're finished with anything privileged, switch over to running as "nobody"
    /* FIXED    if (mStatus_NoError == err)
        {
            const struct passwd *pw = getpwnam("nobody");
            if (pw != NULL)
                setuid(pw->pw_uid);
            else
                LogMsg("WARNING: mdnsd continuing as root because user \"nobody\" does not exist");
        }
            */

    if (mStatus_NoError == err)
        err = MainLoop(&mDNSStorage);

    LogMsg("err return is %d\n", err);
    LogMsg("%s stopping", mDNSResponderVersionString);

    // mDNS_Close(&mDNSStorage);
    mDNS_StartExit(&mDNSStorage);
    mDNS_FinalExit(&mDNSStorage);

    if (udsserver_exit() < 0)
        LogMsg("ExitCallback: udsserver_exit failed");

#if MDNS_DEBUGMSGS > 0
    printf("mDNSResponder exiting normally with %ld\n", err);
#endif

    return err;
}

//		uds_daemon support		////////////////////////////////////////////////////////////

mStatus udsSupportAddFDToEventLoop(int fd, udsEventCallback callback, void * context, void ** platform_data)
/* Support routine for uds_daemon.c */
{
    // Depends on the fact that udsEventCallback == mDNSPosixEventCallback
    (void) platform_data;
    mdnslogInfo("udsSupportAddFDToEventLoop fd=%d", fd);
    return mDNSPosixAddFDToEventLoop(fd, callback, context);
}

int udsSupportReadFD(dnssd_sock_t fd, char * buf, int len, int flags, void * platform_data)
{
    (void) platform_data;
    return recv(fd, buf, len, flags);
}

mStatus udsSupportRemoveFDFromEventLoop(int fd, void * platform_data) // Note: This also CLOSES the file descriptor
{
    mStatus err = mDNSPosixRemoveFDFromEventLoop(fd);
    mdnslogInfo("udsSupportRemoveFDFromEventLoop fd=%d err=%d", fd, err);
    (void) platform_data;
    close(fd);
    return err;
}

mDNSexport void RecordUpdatedNiceLabel(mDNS * const m, mDNSs32 delay)
{
    (void) m;
    (void) delay;
    // No-op, for now
}

// mDNS API Implement in mdns.h
void mdnsd_start()
{
    mdnslogInfo("mdnsd_start");
    MainDaemonTask(0, NULL);
}

void mdnsd_stop()
{
    mdnslogInfo("mdnsd_stop");
    setStoped();
    for (int i = 0; i < 20; i++)
    {
        if (mStoped == 0)
        {
            break;
        }
        vTaskDelay(500 / portTICK_RATE_MS);
    }
}

void mdns_update_interface()
{
    mdnslogInfo("mdns_update_interface");
    mDNS_UpdateInterface();
}

#if _BUILDING_XCODE_PROJECT_
// If the process crashes, then this string will be magically included in the automatically-generated crash log
const char * __crashreporter_info__ = mDNSResponderVersionString_SCCS + 5;
asm(".desc ___crashreporter_info__, 0x10");
#endif

// For convenience when using the "strings" command, this is the last thing in the file
#if mDNSResponderVersion > 1
mDNSexport const char mDNSResponderVersionString_SCCS[] =
    "@(#) mDNSResponder-" STRINGIFY(mDNSResponderVersion) " (" __DATE__ " " __TIME__ ")";
#elif MDNS_VERSIONSTR_NODTS
mDNSexport const char mDNSResponderVersionString_SCCS[] = "@(#) mDNSResponder (Engineering Build)";
#else
mDNSexport const char mDNSResponderVersionString_SCCS[] = "@(#) mDNSResponder (Engineering Build) (" __DATE__ " " __TIME__ ")";
#endif
