/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
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

#include <stdio.h> // Needed for fopen() etc.
//#include <unistd.h>           // Needed for close()
#include <errno.h>  // Needed for errno etc.
#include <string.h> // Needed for strlen() etc.
//#include <sys/socket.h>			// Needed for socket() etc.
#include <lwip/sockets.h>
//#include <netinet/in.h>			// Needed for sockaddr_in
//#include <syslog.h>
#include "os.h"

#include "DNSCommon.h"
#include "PlatformCommon.h"
#include "mDNSEmbeddedAPI.h" // Defines the interface provided to the client layer above

// Stanley Added
/*        static void syslog( int priority, const char * message, ...)
                {
                         switch(priority)
                        {
                                case 0:  printf("LOG_WARNING:%s\n",message);
                                         break;
                                case 1:  printf("LOG_INFO:%s\n", message);
                                         break;
                                default: printf("LOG_OTHERS:%s\n", message);
                                         break;
                        }
                }
*/

#ifdef NOT_HAVE_SOCKLEN_T
typedef unsigned int socklen_t;
#endif

// Bind a UDP socket to find the source address to a destination
mDNSexport void mDNSPlatformSourceAddrForDest(mDNSAddr * const src, const mDNSAddr * const dst)
{
    union
    {
        struct sockaddr s;
        struct sockaddr_in a4;
        struct sockaddr_in6 a6;
    } addr;
    socklen_t len       = sizeof(addr);
    socklen_t inner_len = 0;
    int sock            = socket(AF_INET, SOCK_DGRAM, 0);
    src->type           = mDNSAddrType_None;
    if (sock == -1)
        return;
    if (dst->type == mDNSAddrType_IPv4)
    {
        inner_len = sizeof(addr.a4);
#ifndef NOT_HAVE_SA_LEN
        addr.a4.sin_len = inner_len;
#endif
        addr.a4.sin_family      = AF_INET;
        addr.a4.sin_port        = 1; // Not important, any port will do
        addr.a4.sin_addr.s_addr = dst->ip.v4.NotAnInteger;
    }
    else if (dst->type == mDNSAddrType_IPv6)
    {
        inner_len = sizeof(addr.a6);
#ifndef NOT_HAVE_SA_LEN
        addr.a6.sin6_len = inner_len;
#endif
        addr.a6.sin6_family   = AF_INET6;
        addr.a6.sin6_flowinfo = 0;
        addr.a6.sin6_port     = 1; // Not important, any port will do
        addr.a6.sin6_addr     = *(struct in6_addr *) &dst->ip.v6;
        // FIXED        addr.a6.sin6_scope_id = 0;
    }
    else
        return;

    if ((connect(sock, &addr.s, inner_len)) < 0)
    {
        LogMsg("mDNSPlatformSourceAddrForDest: connect %#a failed errno %d (%s)", dst, errno, strerror(errno));
        goto exit;
    }

    if ((getsockname(sock, &addr.s, &len)) < 0)
    {
        LogMsg("mDNSPlatformSourceAddrForDest: getsockname failed errno %d (%s)", errno, strerror(errno));
        goto exit;
    }

    src->type = dst->type;
    if (dst->type == mDNSAddrType_IPv4)
        src->ip.v4.NotAnInteger = addr.a4.sin_addr.s_addr;
    else
        src->ip.v6 = *(mDNSv6Addr *) &addr.a6.sin6_addr;
exit:
    close(sock);
}

// dst must be at least MAX_ESCAPED_DOMAIN_NAME bytes, and option must be less than 32 bytes in length
mDNSlocal mDNSBool GetConfigOption(char * dst, const char * option, FILE * f)
{
    char buf[32 + 1 + MAX_ESCAPED_DOMAIN_NAME]; // Option name, one space, option value
    unsigned int len = os_strlen(option);
    if (len + 1 + MAX_ESCAPED_DOMAIN_NAME > sizeof(buf) - 1)
    {
        LogMsg("GetConfigOption: option %s too long", option);
        return mDNSfalse;
    }
    fseek(f, 0, SEEK_SET);             // set position to beginning of stream
    while (fgets(buf, sizeof(buf), f)) // Read at most sizeof(buf)-1 bytes from file, and append '\0' C-string terminator
    {
        if (!os_strncmp(buf, option, len))
        {
            os_strncpy(dst, buf + len + 1, MAX_ESCAPED_DOMAIN_NAME - 1);
            if (dst[MAX_ESCAPED_DOMAIN_NAME - 1])
                dst[MAX_ESCAPED_DOMAIN_NAME - 1] = '\0';
            len = os_strlen(dst);
            if (len && dst[len - 1] == '\n')
                dst[len - 1] = '\0'; // chop newline
            return mDNStrue;
        }
    }
    debugf("Option %s not set", option);
    return mDNSfalse;
}

mDNSexport void ReadDDNSSettingsFromConfFile(mDNS * const m, const char * const filename, domainname * const hostname,
                                             domainname * const domain, mDNSBool * DomainDiscoveryDisabled)
{
    char buf[MAX_ESCAPED_DOMAIN_NAME] = "";
    mStatus err;
    FILE * f = fopen(filename, "r");

    if (hostname)
        hostname->c[0] = 0;
    if (domain)
        domain->c[0] = 0;
    if (DomainDiscoveryDisabled)
        *DomainDiscoveryDisabled = mDNSfalse;

    if (f)
    {
        if (DomainDiscoveryDisabled && GetConfigOption(buf, "DomainDiscoveryDisabled", f) && !os_strcasecmp(buf, "true"))
            *DomainDiscoveryDisabled = mDNStrue;
        if (hostname && GetConfigOption(buf, "hostname", f) && !MakeDomainNameFromDNSNameString(hostname, buf))
            goto badf;
        if (domain && GetConfigOption(buf, "zone", f) && !MakeDomainNameFromDNSNameString(domain, buf))
            goto badf;
        buf[0] = 0;
        GetConfigOption(buf, "secret-64", f); // failure means no authentication
        fclose(f);
        f = NULL;
    }
    else
    {
        //        if (errno != ENOENT) LogMsg("ERROR: Config file exists, but cannot be opened.");
        return;
    }

    if (domain && domain->c[0] && buf[0])
    {
        DomainAuthInfo * info = (DomainAuthInfo *) mDNSPlatformMemAllocate(sizeof(*info));
        // for now we assume keyname = service reg domain and we use same key for service and hostname registration
        err = mDNS_SetSecretForDomain(m, info, domain, domain, buf, NULL, 0, mDNSfalse);
        if (err)
            LogMsg("ERROR: mDNS_SetSecretForDomain returned %d for domain %##s", err, domain->c);
    }

    return;

badf:
    LogMsg("ERROR: malformatted config file");
    if (f)
        fclose(f);
}
#ifndef REMOVE_LOG_FUNCTIONS
#if MDNS_DEBUGMSGS
mDNSexport void mDNSPlatformWriteDebugMsg(const char * msg)
{
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
}
#endif

mDNSexport void mDNSPlatformWriteLogMsg(const char * ident, const char * buffer, mDNSLogLevel_t loglevel)
{
#if APPLE_OSX_mDNSResponder && LogTimeStamps
    extern mDNS mDNSStorage;
    extern mDNSu32 mDNSPlatformClockDivisor;
    mDNSs32 t = mDNSStorage.timenow ? mDNSStorage.timenow : mDNSPlatformClockDivisor ? mDNS_TimeNow_NoLock(&mDNSStorage) : 0;
    int ms    = ((t < 0) ? -t : t) % 1000;
#endif

    if (mDNS_DebugMode) // In debug mode we write to stderr
    {
#if APPLE_OSX_mDNSResponder && LogTimeStamps
        if (ident && ident[0] && mDNSPlatformClockDivisor)
            fprintf(stderr, "%8d.%03d: %s\n", (int) (t / 1000), ms, buffer);
        else
#endif
            fprintf(stderr, "%s\n", buffer);
        fflush(stderr);
    }
    else // else, in production mode, we write to syslog
    {
        static int log_inited = 0;

        // int syslog_level = LOG_ERR;
        int syslog_level = 1;
        switch (loglevel)
        {
        // case MDNS_LOG_MSG:       syslog_level = LOG_ERR;     break;
        case MDNS_LOG_MSG:
            syslog_level = 1;
            break;
        // case MDNS_LOG_OPERATION: syslog_level = LOG_WARNING; break;
        case MDNS_LOG_OPERATION:
            syslog_level = 2;
            break;
        case MDNS_LOG_SPS:
            syslog_level = 3;
            break;
        // case MDNS_LOG_SPS:       syslog_level = LOG_NOTICE;  break;
        case MDNS_LOG_INFO:
            syslog_level = 4;
            break;
        // case MDNS_LOG_INFO:      syslog_level = LOG_INFO;    break;
        case MDNS_LOG_DEBUG:
            syslog_level = 5;
            break;
            // case MDNS_LOG_DEBUG:     syslog_level = LOG_DEBUG;   break;
        default:
            fprintf(stderr, "Unknown loglevel %d, assuming LOG_ERR\n", loglevel);
            fflush(stderr);
        }

        // if (!log_inited) { openlog(ident, LOG_CONS, LOG_DAEMON); log_inited++; }

#if APPLE_OSX_mDNSResponder && LogTimeStamps
        if (ident && ident[0] && mDNSPlatformClockDivisor)
            syslog(syslog_level, "%8d.%03d: %s", (int) (t / 1000), ms, buffer);
        else
#elif APPLE_OSX_mDNSResponder
        mDNSPlatformLogToFile(syslog_level, buffer);
#else
        // syslog(syslog_level, "%s", buffer);
        printf("from PlatformCommon.c\n");
        switch (syslog_level)
        {
        case 1:
            printf("LOG_ERR: %s\n", buffer);
            break;
        case 2:
            printf("LOG_WARNING: %s\n", buffer);
            break;
        case 3:
            printf("LOG_NOTICE: %s\n", buffer);
            break;
        case 4:
            printf("LOG_INFO: %s\n", buffer);
            break;
        case 5:
            printf("LOG_DEBUG: %s\n", buffer);
            break;
        default:
            printf("LOG_OTHER: %s\n", buffer);
            break;
        }
#endif
    }
}
#endif // #ifndef REMOVE_LOG_FUNCTIONS
