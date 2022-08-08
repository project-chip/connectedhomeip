/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2003-2013 Apple Computer, Inc. All rights reserved.
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

#if defined(_WIN32)
#include <process.h>
#define usleep(X) Sleep(((X) + 999) / 1000)
#else
//#include <fcntl.h>
#include <errno.h>
//#include <sys/ioctl.h>
#include <lwip/sockets.h>
//#include <sys/types.h>
//#include <sys/time.h>
//#include <timer.h>
//#include <sys/resource.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "DNSCommon.h"
#include "mDNSEmbeddedAPI.h"
#include "type_def.h" //TCM
#include "uDNS.h"
#include "uds_daemon.h"

// Normally we append search domains only for queries with a single label that are not
// fully qualified. This can be overridden to apply search domains for queries (that are
// not fully qualified) with any number of labels e.g., moon, moon.cs, moon.cs.be, etc.
mDNSBool AlwaysAppendSearchDomains = mDNSfalse;

// Apple-specific functionality, not required for other platforms
#if APPLE_OSX_mDNSResponder
#include <sys/ucred.h>
#ifndef PID_FILE
#define PID_FILE ""
#endif
#endif

#ifdef LOCAL_PEERPID
#include <libproc.h>       // for proc_pidinfo()
#include <sys/proc_info.h> // for struct proc_bsdshortinfo
#include <sys/socket.h>    // for getsockopt
#include <sys/un.h>        // for LOCAL_PEERPID
#endif                     // LOCAL_PEERPID
// upto 16 characters of process name (defined in <sys/proc.h> but we do not want to include that file)
#define MAXCOMLEN 16

#if APPLE_OSX_mDNSResponder
#include <WebFilterDNS/WebFilterDNS.h>

#if !NO_WCF

int WCFIsServerRunning(WCFConnection * conn) __attribute__((weak_import));
int WCFNameResolvesToAddr(WCFConnection * conn, char * domainName, struct sockaddr * address, uid_t userid)
    __attribute__((weak_import));
int WCFNameResolvesToName(WCFConnection * conn, char * fromName, char * toName, uid_t userid) __attribute__((weak_import));

// Do we really need to define a macro for "if"?
#define CHECK_WCF_FUNCTION(X) if (X)
#endif // ! NO_WCF

#else
#define NO_WCF 1
#endif // APPLE_OSX_mDNSResponder

// User IDs 0-500 are system-wide processes, not actual users in the usual sense
// User IDs for real user accounts start at 501 and count up from there
#define SystemUID(X) ((X) <= 500)

//#define MAX_ANONYMOUS_DATA      256
#define MAX_ANONYMOUS_DATA 64

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Types and Data Structures
#endif

typedef enum
{
    t_uninitialized,
    t_morecoming,
    t_complete,
    t_error,
    t_terminated
} transfer_state;

typedef struct request_state request_state;

typedef void (*req_termination_fn)(request_state * request);

typedef struct registered_record_entry
{
    struct registered_record_entry * next;
    mDNSu32 key;
    client_context_t regrec_client_context;
    request_state * request;
    mDNSBool external_advertise;
    mDNSInterfaceID origInterfaceID;
    AuthRecord * rr; // Pointer to variable-sized AuthRecord (Why a pointer? Why not just embed it here?)
} registered_record_entry;

// A single registered service: ServiceRecordSet + bookkeeping
// Note that we duplicate some fields from parent service_info object
// to facilitate cleanup, when instances and parent may be deallocated at different times.
typedef struct service_instance
{
    struct service_instance * next;
    request_state * request;
    AuthRecord * subtypes;
    mDNSBool renameonmemfree;    // Set on config change when we deregister original name
    mDNSBool clientnotified;     // Has client been notified of successful registration yet?
    mDNSBool default_local;      // is this the "local." from an empty-string registration?
    mDNSBool external_advertise; // is this is being advertised externally?
    domainname domain;
    ServiceRecordSet srs; // note -- variable-sized object -- must be last field in struct
} service_instance;

// for multi-domain default browsing
typedef struct browser_t
{
    struct browser_t * next;
    domainname domain;
    DNSQuestion q;
} browser_t;

#ifdef _WIN32
typedef unsigned int pid_t;
typedef unsigned int socklen_t;
#endif

struct request_state
{
    request_state * next;
    request_state * primary; // If this operation is on a shared socket, pointer to primary
                             // request_state for the original DNSServiceCreateConnection() operation
    dnssd_sock_t sd;
    pid_t process_id;         // Client's PID value
    char pid_name[MAXCOMLEN]; // Client's process name
    char uuid[UUID_SIZE];
    mDNSBool validUUID;
    dnssd_sock_t errsd;
    mDNSu32 uid;
    void * platform_data;

    // Note: On a shared connection these fields in the primary structure, including hdr, are re-used
    // for each new request. This is because, until we've read the ipc_msg_hdr to find out what the
    // operation is, we don't know if we're going to need to allocate a new request_state or not.
    transfer_state ts;
    mDNSu32 hdr_bytes; // bytes of header already read
    ipc_msg_hdr hdr;
    mDNSu32 data_bytes;  // bytes of message data already read
    char * msgbuf;       // pointer to data storage to pass to free()
    const char * msgptr; // pointer to data to be read from (may be modified)
    char * msgend;       // pointer to byte after last byte of message

    // reply, termination, error, and client context info
    int no_reply;         // don't send asynchronous replies to client
    mDNSs32 time_blocked; // record time of a blocked client
    int unresponsiveness_reports;
    struct reply_state * replies; // corresponding (active) reply list
    req_termination_fn terminate;
    DNSServiceFlags flags;

    union
    {
        registered_record_entry * reg_recs; // list of registrations for a connection-oriented request
        struct
        {
            mDNSInterfaceID interface_id;
            mDNSBool default_domain;
            mDNSBool ForceMCast;
            domainname regtype;
            browser_t * browsers;
            const mDNSu8 * AnonData;
        } browser;
        struct
        {
            mDNSInterfaceID InterfaceID;
            mDNSu16 txtlen;
            void * txtdata;
            mDNSIPPort port;
            domainlabel name;
            char type_as_string[MAX_ESCAPED_DOMAIN_NAME];
            domainname type;
            mDNSBool default_domain;
            domainname host;
            mDNSBool autoname;         // Set if this name is tied to the Computer Name
            mDNSBool autorename;       // Set if this client wants us to automatically rename on conflict
            mDNSBool allowremotequery; // Respond to unicast queries from outside the local link?
            int num_subtypes;
            mDNSBool AnonData;
            service_instance * instances;
        } servicereg;
        struct
        {
            mDNSInterfaceID interface_id;
            mDNSu32 flags;
            mDNSu32 protocol;
            DNSQuestion q4;
            DNSQuestion * q42;
            DNSQuestion q6;
            DNSQuestion * q62;
            mDNSu8 v4ans;
            mDNSu8 v6ans;
        } addrinfo;
        struct
        {
            mDNSIPPort ReqExt; // External port we originally requested, for logging purposes
#ifndef REMOVE_NAT_FUCTIONALITY
            NATTraversalInfo NATinfo;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
        } pm;
        struct
        {
            DNSServiceFlags flags;
            DNSQuestion q_all;
            DNSQuestion q_default;
        } enumeration;
        struct
        {
            DNSQuestion q;
            DNSQuestion * q2;
            mDNSu8 ans;
        } queryrecord;
        struct
        {
            DNSQuestion qtxt;
            DNSQuestion qsrv;
            const ResourceRecord * txt;
            const ResourceRecord * srv;
            mDNSs32 ReportTime;
            mDNSBool external_advertise;
        } resolve;
    } u;
};

// struct physically sits between ipc message header and call-specific fields in the message buffer
typedef struct
{
    DNSServiceFlags flags;     // Note: This field is in NETWORK byte order
    mDNSu32 ifi;               // Note: This field is in NETWORK byte order
    DNSServiceErrorType error; // Note: This field is in NETWORK byte order
} reply_hdr;

typedef struct reply_state
{
    struct reply_state * next; // If there are multiple unsent replies
    mDNSu32 totallen;
    mDNSu32 nwriten;
    ipc_msg_hdr mhdr[1];
    reply_hdr rhdr[1];
} reply_state;

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Globals
#endif

// globals
// mDNSexport mDNS mDNSStorage;
#if defined(MTK_MDNS_STORAGE_IN_TCM)
#if defined(__ICCARM__)
BSS_IN_TCM mDNSexport mDNS mDNSStorage;
#else
mDNSexport mDNS BSS_IN_TCM mDNSStorage;
#endif
#else
mDNSexport mDNS mDNSStorage;
#endif

mDNSexport const char ProgramName[] = "mDNSResponder";

static dnssd_sock_t listenfd        = dnssd_InvalidSocket;
static request_state * all_requests = NULL;
#ifdef LOCAL_PEERPID
struct proc_bsdshortinfo proc;
#endif // LOCAL_PEERPID
mDNSlocal void set_peer_pid(request_state * request);
mDNSlocal void LogMcastClientInfo(request_state * req);
mDNSlocal void GetMcastClients(request_state * req);
static mDNSu32 mcount;     // tracks the current active mcast operations for McastLogging
static mDNSu32 i_mcount;   // sets mcount when McastLogging is enabled(PROF signal is sent)
static mDNSu32 n_mrecords; // tracks the current active mcast records for McastLogging
static mDNSu32 n_mquests;  // tracks the current active mcast questions for McastLogging

// Note asymmetry here between registration and browsing.
// For service registrations we only automatically register in domains that explicitly appear in local configuration data
// (so AutoRegistrationDomains could equally well be called SCPrefRegDomains)
// For service browsing we also learn automatic browsing domains from the network, so for that case we have:
// 1. SCPrefBrowseDomains (local configuration data)
// 2. LocalDomainEnumRecords (locally-generated local-only PTR records -- equivalent to slElem->AuthRecs in uDNS.c)
// 3. AutoBrowseDomains, which is populated by tracking add/rmv events in AutomaticBrowseDomainChange, the callback function for our
// mDNS_GetDomains call. By creating and removing our own LocalDomainEnumRecords, we trigger AutomaticBrowseDomainChange callbacks
// just like domains learned from the network would.

mDNSexport DNameListElem * AutoRegistrationDomains; // Domains where we automatically register for empty-string registrations

static DNameListElem *
    SCPrefBrowseDomains; // List of automatic browsing domains read from SCPreferences for "empty string" browsing
static ARListElem * LocalDomainEnumRecords;   // List of locally-generated PTR records to augment those we learn from the network
mDNSexport DNameListElem * AutoBrowseDomains; // List created from those local-only PTR records plus records we get from the network

#define MSG_PAD_BYTES                                                                                                              \
    5 // pad message buffer (read from client) with n zero'd bytes to guarantee
      // n get_string() calls w/o buffer overrun
// initialization, setup/teardown functions

// If a platform specifies its own PID file name, we use that
#ifndef PID_FILE
#define PID_FILE "/var/run/mDNSResponder.pid"
#endif

mDNSlocal char * AnonDataToString(const mDNSu8 * ad, int adlen, char * adstr, int adstrlen);

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - General Utility Functions
#endif

mDNSlocal mDNSu32 dnssd_htonl(mDNSu32 l)
{
    mDNSu32 ret;
    char * data = (char *) &ret;
    put_uint32(l, &data);
    return ret;
}

#ifndef REMOVE_UDS_DAEMON_ERROR_MESSAGING
mDNSlocal void FatalError(char * errmsg)
{
    char * ptr = NULL;
    LogMsg("%s: %s", errmsg, dnssd_strerror(dnssd_errno));
    *ptr = 0; // On OS X abort() doesn't generate a crash log, but writing to zero does
    // FIXED abort();    // On platforms where writing to zero doesn't generate an exception, abort instead
}

// hack to search-replace perror's to LogMsg's
mDNSlocal void my_perror(char * errmsg)
{
    LogMsg("%s: %d (%s)", errmsg, dnssd_errno, dnssd_strerror(dnssd_errno));
}

// Throttled version of my_perror: Logs once every 250 msgs
mDNSlocal void my_throttled_perror(char * err_msg)
{
    static int uds_throttle_count = 0;
    if ((uds_throttle_count++ % 250) == 0)
        my_perror(err_msg);
}

// LogMcastQuestion/LogMcastQ should be called after the DNSQuestion struct is initialized(especially for q->TargetQID)
// Hence all calls are made after mDNS_StartQuery()/mDNS_StopQuery()/mDNS_StopBrowse() is called.
mDNSlocal void LogMcastQuestion(mDNS * const m, const DNSQuestion * const q, request_state * req, q_state status)
{
    if (mDNSOpaque16IsZero(q->TargetQID)) // Check for Mcast Query
    {
        mDNSBool mflag = mDNSfalse;
        if (status == q_start)
        {
            if (++mcount == 1)
                mflag = mDNStrue;
        }
        else
        {
            mcount--;
        }
        LogMcast("%s: %##s  (%s) (%s)  Client(%d)[%s]", status ? "+Question" : "-Question", q->qname.c, DNSTypeName(q->qtype),
                 q->InterfaceID == mDNSInterface_LocalOnly ? "lo"
                     : q->InterfaceID == mDNSInterface_P2P ? "p2p"
                     : q->InterfaceID == mDNSInterface_Any ? "any"
                                                           : InterfaceNameForID(m, q->InterfaceID),
                 req->process_id, req->pid_name);
        LogMcastStateInfo(m, mflag, mDNSfalse, mDNSfalse);
    }
    return;
}

// LogMcastService/LogMcastS should be called after the AuthRecord struct is initialized
// Hence all calls are made after mDNS_Register()/ just before mDNS_Deregister()
mDNSlocal void LogMcastService(mDNS * const m, const AuthRecord * const ar, request_state * req, reg_state status)
{
    if (!AuthRecord_uDNS(ar)) // Check for Mcast Service
    {
        mDNSBool mflag = mDNSfalse;
        if (status == reg_start)
        {
            if (++mcount == 1)
                mflag = mDNStrue;
        }
        else
        {
            mcount--;
        }
        LogMcast("%s: %##s  (%s)  (%s)  Client(%d)[%s]", status ? "+Service" : "-Service", ar->resrec.name->c,
                 DNSTypeName(ar->resrec.rrtype),
                 ar->resrec.InterfaceID == mDNSInterface_LocalOnly ? "lo"
                     : ar->resrec.InterfaceID == mDNSInterface_P2P ? "p2p"
                     : ar->resrec.InterfaceID == mDNSInterface_Any ? "all"
                                                                   : InterfaceNameForID(m, ar->resrec.InterfaceID),
                 req->process_id, req->pid_name);
        LogMcastStateInfo(m, mflag, mDNSfalse, mDNSfalse);
    }
    return;
}

// For complete Mcast State Log, pass mDNStrue to mstatelog in LogMcastStateInfo()
mDNSexport void LogMcastStateInfo(mDNS * const m, mDNSBool mflag, mDNSBool start, mDNSBool mstatelog)
{
    if (!mstatelog)
    {
        if (!all_requests)
        {
            LogMcastNoIdent("<None>");
        }
        else
        {
            request_state *req, *r;
            for (req = all_requests; req; req = req->next)
            {
                if (req->primary) // If this is a subbordinate operation, check that the parent is in the list
                {
                    for (r = all_requests; r && r != req; r = r->next)
                        if (r == req->primary)
                            goto foundpar;
                }
                // For non-subbordinate operations, and subbordinate operations that have lost their parent, write out their info
#ifndef REMOVE_LOG_FUNCTIONS
                GetMcastClients(req);
#endif // #ifndef REMOVE_LOG_FUNCTIONS
            foundpar:;
            }
            LogMcastNoIdent("--- MCAST RECORDS COUNT[%d] MCAST QUESTIONS COUNT[%d] ---", n_mrecords, n_mquests);
            n_mrecords = n_mquests = 0; // Reset the values
        }
    }
    else
    {
        static mDNSu32 i_mpktnum;
        i_mcount = 0;
        if (start)
            mcount = 0;
        // mcount is initialized to 0 when the PROF signal is sent since mcount could have
        // wrong value if MulticastLogging is disabled and then re-enabled
        LogMcastNoIdent("--- START MCAST STATE LOG ---");
        if (!all_requests)
        {
            mcount = 0;
            LogMcastNoIdent("<None>");
        }
        else
        {
            request_state *req, *r;
            for (req = all_requests; req; req = req->next)
            {
                if (req->primary) // If this is a subbordinate operation, check that the parent is in the list
                {
                    for (r = all_requests; r && r != req; r = r->next)
                        if (r == req->primary)
                            goto foundparent;
                    LogMcastNoIdent("%3d: Orphan operation; parent not found in request list", req->sd);
                }
                // For non-subbordinate operations, and subbordinate operations that have lost their parent, write out their info
#ifndef REMOVE_LOG_FUNCTIONS
                LogMcastClientInfo(req);
#endif //#ifndef REMOVE_LOG_FUNCTIONS
            foundparent:;
            }
            if (!mcount) // To initially set mcount
                mcount = i_mcount;
        }
        if (mcount == 0)
        {
            i_mpktnum = m->MPktNum;
            LogMcastNoIdent("--- MCOUNT[%d]: IMPKTNUM[%d] ---", mcount, i_mpktnum);
        }
        if (mflag)
            LogMcastNoIdent("--- MCOUNT[%d]: CMPKTNUM[%d] - IMPKTNUM[%d] = [%d]PKTS ---", mcount, m->MPktNum, i_mpktnum,
                            (m->MPktNum - i_mpktnum));
        LogMcastNoIdent("--- END MCAST STATE LOG ---");
    }
}
#else
#define FatalError(...) ((void) 0)
#define my_perror(...) ((void) 0)
#define my_throttled_perror(...) ((void) 0)
#define LogMcastQuestion(...) ((void) 0)
#define LogMcastService(...) ((void) 0)
#define LogMcastStateInfo(...) ((void) 0)
#endif // #ifndef REMOVE_UDS_DAEMON_ERROR_MESSAGING

mDNSlocal void abort_request(request_state * req)
{
    if (req->terminate == (req_termination_fn) ~0)
    {
        LogMsg("abort_request: ERROR: Attempt to abort operation %p with req->terminate %p", req, req->terminate);
        return;
    }

    // First stop whatever mDNSCore operation we were doing
    // If this is actually a shared connection operation, then its req->terminate function will scan
    // the all_requests list and terminate any subbordinate operations sharing this file descriptor
    if (req->terminate)
        req->terminate(req);

    if (!dnssd_SocketValid(req->sd))
    {
        LogMsg("abort_request: ERROR: Attempt to abort operation %p with invalid fd %d", req, req->sd);
        return;
    }

    // Now, if this request_state is not subordinate to some other primary, close file descriptor and discard replies
    if (!req->primary)
    {
        if (req->errsd != req->sd)
            LogOperation("%3d: Removing FD and closing errsd %d", req->sd, req->errsd);
        else
            LogOperation("%3d: Removing FD", req->sd);
        udsSupportRemoveFDFromEventLoop(req->sd, req->platform_data); // Note: This also closes file descriptor req->sd for us
        if (req->errsd != req->sd)
        {
            dnssd_close(req->errsd);
            req->errsd = req->sd;
        }

        while (req->replies) // free pending replies
        {
            reply_state * ptr = req->replies;
            req->replies      = req->replies->next;
            free(ptr);
        }
    }

    // Set req->sd to something invalid, so that udsserver_idle knows to unlink and free this structure
#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING
    // Don't use dnssd_InvalidSocket (-1) because that's the sentinel value MACOSX_MDNS_MALLOC_DEBUGGING uses
    // for detecting when the memory for an object is inadvertently freed while the object is still on some list
    req->sd = req->errsd = -2;
#else
    req->sd = req->errsd = dnssd_InvalidSocket;
#endif
    // We also set req->terminate to a bogus value so we know if abort_request() gets called again for this request
    req->terminate = (req_termination_fn) ~0;
}

mDNSlocal void AbortUnlinkAndFree(request_state * req)
{
    request_state ** p = &all_requests;
    abort_request(req);
    while (*p && *p != req)
        p = &(*p)->next;
    if (*p)
    {
        *p = req->next;
        free(req);
    }
    else
        LogMsg("AbortUnlinkAndFree: ERROR: Attempt to abort operation %p not in list", req);
}

mDNSlocal reply_state * create_reply(const reply_op_t op, const size_t datalen, request_state * const request)
{
    reply_state * reply;

    if ((unsigned) datalen < sizeof(reply_hdr))
    {
        LogMsg("ERROR: create_reply - data length less than length of required fields");
        return NULL;
    }

    // reply = mallocL("reply_state", sizeof(reply_state) + datalen - sizeof(reply_hdr));
    reply = malloc(sizeof(reply_state) + datalen - sizeof(reply_hdr));
    if (!reply)
        FatalError("ERROR: malloc");

    reply->next     = mDNSNULL;
    reply->totallen = (mDNSu32) datalen + sizeof(ipc_msg_hdr);
    reply->nwriten  = 0;

    reply->mhdr->version        = VERSION;
    reply->mhdr->datalen        = (mDNSu32) datalen;
    reply->mhdr->ipc_flags      = 0;
    reply->mhdr->op             = op;
    reply->mhdr->client_context = request->hdr.client_context;
    reply->mhdr->reg_index      = 0;

    return reply;
}

// Append a reply to the list in a request object
// If our request is sharing a connection, then we append our reply_state onto the primary's list
mDNSlocal void append_reply(request_state * req, reply_state * rep)
{
    request_state * r  = req->primary ? req->primary : req;
    reply_state ** ptr = &r->replies;
    while (*ptr)
        ptr = &(*ptr)->next;
    *ptr      = rep;
    rep->next = NULL;
}

// Generates a response message giving name, type, domain, plus interface index,
// suitable for a browse result or service registration result.
// On successful completion rep is set to point to a malloc'd reply_state struct
mDNSlocal mStatus GenerateNTDResponse(const domainname * const servicename, const mDNSInterfaceID id, request_state * const request,
                                      reply_state ** const rep, reply_op_t op, DNSServiceFlags flags, mStatus err)
{
    domainlabel name;
    domainname type, dom;
    *rep = NULL;
    if (!DeconstructServiceName(servicename, &name, &type, &dom))
        return kDNSServiceErr_Invalid;
    else
    {
        char namestr[MAX_DOMAIN_LABEL + 1];
        char typestr[MAX_ESCAPED_DOMAIN_NAME];
        char domstr[MAX_ESCAPED_DOMAIN_NAME];
        int len;
        char * data;

        ConvertDomainLabelToCString_unescaped(&name, namestr);
        ConvertDomainNameToCString(&type, typestr);
        ConvertDomainNameToCString(&dom, domstr);

        // Calculate reply data length
        len = sizeof(DNSServiceFlags);
        len += sizeof(mDNSu32); // if index
        len += sizeof(DNSServiceErrorType);
        len += (int) (strlen(namestr) + 1);
        len += (int) (strlen(typestr) + 1);
        len += (int) (strlen(domstr) + 1);

        // Build reply header
        *rep                = create_reply(op, len, request);
        (*rep)->rhdr->flags = dnssd_htonl(flags);
        (*rep)->rhdr->ifi   = dnssd_htonl(mDNSPlatformInterfaceIndexfromInterfaceID(&mDNSStorage, id, mDNSfalse));
        (*rep)->rhdr->error = dnssd_htonl(err);

        // Build reply body
        data = (char *) &(*rep)->rhdr[1];
        put_string(namestr, &data);
        put_string(typestr, &data);
        put_string(domstr, &data);

        return mStatus_NoError;
    }
}

// Special support to enable the DNSServiceBrowse call made by Bonjour Browser
// Remove after Bonjour Browser is updated to use DNSServiceQueryRecord instead of DNSServiceBrowse
mDNSlocal void GenerateBonjourBrowserResponse(const domainname * const servicename, const mDNSInterfaceID id,
                                              request_state * const request, reply_state ** const rep, reply_op_t op,
                                              DNSServiceFlags flags, mStatus err)
{
    char namestr[MAX_DOMAIN_LABEL + 1];
    char typestr[MAX_ESCAPED_DOMAIN_NAME];
    static const char domstr[] = ".";
    int len;
    char * data;

    *rep = NULL;

    // 1. Put first label in namestr
    ConvertDomainLabelToCString_unescaped((const domainlabel *) servicename, namestr);

    // 2. Put second label and "local" into typestr
    mDNS_snprintf(typestr, sizeof(typestr), "%#s.local.", SecondLabel(servicename));

    // Calculate reply data length
    len = sizeof(DNSServiceFlags);
    len += sizeof(mDNSu32); // if index
    len += sizeof(DNSServiceErrorType);
    len += (int) (strlen(namestr) + 1);
    len += (int) (strlen(typestr) + 1);
    len += (int) (strlen(domstr) + 1);

    // Build reply header
    *rep                = create_reply(op, len, request);
    (*rep)->rhdr->flags = dnssd_htonl(flags);
    (*rep)->rhdr->ifi   = dnssd_htonl(mDNSPlatformInterfaceIndexfromInterfaceID(&mDNSStorage, id, mDNSfalse));
    (*rep)->rhdr->error = dnssd_htonl(err);

    // Build reply body
    data = (char *) &(*rep)->rhdr[1];
    put_string(namestr, &data);
    put_string(typestr, &data);
    put_string(domstr, &data);
}

// Returns a resource record (allocated w/ malloc) containing the data found in an IPC message
// Data must be in the following format: flags, interfaceIndex, name, rrtype, rrclass, rdlen, rdata, (optional) ttl
// (ttl only extracted/set if ttl argument is non-zero). Returns NULL for a bad-parameter error
mDNSlocal AuthRecord * read_rr_from_ipc_msg(request_state * request, int GetTTL, int validate_flags)
{
    DNSServiceFlags flags  = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex = get_uint32(&request->msgptr, request->msgend);
    // char name[256];
    char name[64];
    int str_err        = get_string(&request->msgptr, request->msgend, name, sizeof(name));
    mDNSu16 type       = get_uint16(&request->msgptr, request->msgend);
    mDNSu16 class      = get_uint16(&request->msgptr, request->msgend);
    mDNSu16 rdlen      = get_uint16(&request->msgptr, request->msgend);
    const char * rdata = get_rdata(&request->msgptr, request->msgend, rdlen);
    mDNSu32 ttl        = GetTTL ? get_uint32(&request->msgptr, request->msgend) : 0;
    int storage_size   = rdlen > sizeof(RDataBody) ? rdlen : sizeof(RDataBody);
    AuthRecord * rr;
    mDNSInterfaceID InterfaceID;
    AuthRecType artype;

    request->flags = flags;

    if (str_err)
    {
        LogMsg("ERROR: read_rr_from_ipc_msg - get_string");
        return NULL;
    }

    if (!request->msgptr)
    {
        LogMsg("Error reading Resource Record from client");
        return NULL;
    }

    if (validate_flags && !((flags & kDNSServiceFlagsShared) == kDNSServiceFlagsShared) &&
        !((flags & kDNSServiceFlagsUnique) == kDNSServiceFlagsUnique))
    {
        LogMsg("ERROR: Bad resource record flags (must be kDNSServiceFlagsShared or kDNSServiceFlagsUnique)");
        return NULL;
    }

    // rr = mallocL("AuthRecord/read_rr_from_ipc_msg", sizeof(AuthRecord) - sizeof(RDataBody) + storage_size);
    rr = malloc(sizeof(AuthRecord) - sizeof(RDataBody) + storage_size);
    if (!rr)
        FatalError("ERROR: malloc");

    InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    if (InterfaceID == mDNSInterface_LocalOnly)
        artype = AuthRecordLocalOnly;
    else if (InterfaceID == mDNSInterface_P2P)
        artype = AuthRecordP2P;
    else if ((InterfaceID == mDNSInterface_Any) && (flags & kDNSServiceFlagsIncludeP2P) && (flags & kDNSServiceFlagsIncludeAWDL))
        artype = AuthRecordAnyIncludeAWDLandP2P;
    else if ((InterfaceID == mDNSInterface_Any) && (flags & kDNSServiceFlagsIncludeP2P))
        artype = AuthRecordAnyIncludeP2P;
    else if ((InterfaceID == mDNSInterface_Any) && (flags & kDNSServiceFlagsIncludeAWDL))
        artype = AuthRecordAnyIncludeAWDL;
    else
        artype = AuthRecordAny;

    mDNS_SetupResourceRecord(rr, mDNSNULL, InterfaceID, type, 0,
                             (mDNSu8) ((flags & kDNSServiceFlagsShared) ? kDNSRecordTypeShared : kDNSRecordTypeUnique), artype,
                             mDNSNULL, mDNSNULL);

    if (!MakeDomainNameFromDNSNameString(&rr->namestorage, name))
    {
        LogMsg("ERROR: bad name: %s", name);
        free(rr);
        return NULL;
    }

    if (flags & kDNSServiceFlagsAllowRemoteQuery)
        rr->AllowRemoteQuery = mDNStrue;
    rr->resrec.rrclass            = class;
    rr->resrec.rdlength           = rdlen;
    rr->resrec.rdata->MaxRDLength = rdlen;
    mDNSPlatformMemCopy(rr->resrec.rdata->u.data, rdata, rdlen);
    if (GetTTL)
        rr->resrec.rroriginalttl = ttl;
    rr->resrec.namehash = DomainNameHashValue(rr->resrec.name);
    SetNewRData(&rr->resrec, mDNSNULL, 0); // Sets rr->rdatahash for us
    return rr;
}

mDNSlocal int build_domainname_from_strings(domainname * srv, char * name, char * regtype, char * domain)
{
    domainlabel n;
    domainname d, t;

    if (!MakeDomainLabelFromLiteralString(&n, name))
        return -1;
    if (!MakeDomainNameFromDNSNameString(&t, regtype))
        return -1;
    if (!MakeDomainNameFromDNSNameString(&d, domain))
        return -1;
    if (!ConstructServiceName(srv, &n, &t, &d))
        return -1;
    return 0;
}

mDNSlocal void send_all(dnssd_sock_t s, const char * ptr, int len)
{
    int n = send(s, ptr, len, 0);
    // On a freshly-created Unix Domain Socket, the kernel should *never* fail to buffer a small write for us
    // (four bytes for a typical error code return, 12 bytes for DNSServiceGetProperty(DaemonVersion)).
    // If it does fail, we don't attempt to handle this failure, but we do log it so we know something is wrong.
    if (n < len)
        mdnslogInfo("ERROR: send_all(%d) wrote %d of %d errno %d (%s)", s, n, len, dnssd_errno, dnssd_strerror(dnssd_errno));
}

#if 0
mDNSlocal mDNSBool AuthorizedDomain(const request_state * const request, const domainname * const d, const DNameListElem * const doms)
{
    const DNameListElem   *delem = mDNSNULL;
    int bestDelta   = -1;                           // the delta of the best match, lower is better
    int dLabels     = 0;
    mDNSBool allow       = mDNSfalse;

    if (SystemUID(request->uid)) return mDNStrue;

    dLabels = CountLabels(d);
    for (delem = doms; delem; delem = delem->next)
    {
        if (delem->uid)
        {
            int delemLabels = CountLabels(&delem->name);
            int delta       = dLabels - delemLabels;
            if ((bestDelta == -1 || delta <= bestDelta) && SameDomainName(&delem->name, SkipLeadingLabels(d, delta)))
            {
                bestDelta = delta;
                allow = (allow || (delem->uid == request->uid));
            }
        }
    }

    return bestDelta == -1 ? mDNStrue : allow;
}
#endif

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - external helpers
#endif

mDNSlocal mDNSBool callExternalHelpers(mDNSInterfaceID InterfaceID, const domainname * const domain, DNSServiceFlags flags)
{
#if APPLE_OSX_mDNSResponder

    if (((InterfaceID == mDNSInterface_Any) && (flags & (kDNSServiceFlagsIncludeP2P | kDNSServiceFlagsIncludeAWDL)) &&
         IsLocalDomain(domain)) ||
        mDNSPlatformInterfaceIsD2D(InterfaceID))
    {
        return mDNStrue;
    }
    else
        return mDNSfalse;

#else
    (void) InterfaceID;
    (void) domain;
    (void) flags;

    return mDNSfalse;
#endif // APPLE_OSX_mDNSResponder
}

mDNSlocal void external_start_advertising_helper(service_instance * const instance)
{
    AuthRecord * st = instance->subtypes;
    ExtraResourceRecord * e;
    int i;

    if (mDNSIPPortIsZero(instance->request->u.servicereg.port))
    {
        LogInfo("external_start_advertising_helper: Not registering service with port number zero");
        return;
    }

    if (instance->external_advertise)
        LogMsg("external_start_advertising_helper: external_advertise already set!");

    for (i = 0; i < instance->request->u.servicereg.num_subtypes; i++)
        external_start_advertising_service(&st[i].resrec, instance->request->flags);

    external_start_advertising_service(&instance->srs.RR_PTR.resrec, instance->request->flags);
    external_start_advertising_service(&instance->srs.RR_SRV.resrec, instance->request->flags);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    external_start_advertising_service(&instance->srs.RR_TXT.resrec, instance->request->flags);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    for (e = instance->srs.Extras; e; e = e->next)
        external_start_advertising_service(&e->r.resrec, instance->request->flags);

    instance->external_advertise = mDNStrue;
}

mDNSlocal void external_stop_advertising_helper(service_instance * const instance)
{
    AuthRecord * st = instance->subtypes;
    ExtraResourceRecord * e;
    int i;

    if (!instance->external_advertise)
        return;

    LogInfo("external_stop_advertising_helper: calling external_stop_advertising_service");

    for (i = 0; i < instance->request->u.servicereg.num_subtypes; i++)
        external_stop_advertising_service(&st[i].resrec, instance->request->flags);

    external_stop_advertising_service(&instance->srs.RR_PTR.resrec, instance->request->flags);
    external_stop_advertising_service(&instance->srs.RR_SRV.resrec, instance->request->flags);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    external_stop_advertising_service(&instance->srs.RR_TXT.resrec, instance->request->flags);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    for (e = instance->srs.Extras; e; e = e->next)
        external_stop_advertising_service(&e->r.resrec, instance->request->flags);

    instance->external_advertise = mDNSfalse;
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceRegister
#endif

mDNSexport void FreeExtraRR(mDNS * const m, AuthRecord * const rr, mStatus result)
{
    ExtraResourceRecord * extra = (ExtraResourceRecord *) rr->RecordContext;
    (void) m; // Unused

    if (result != mStatus_MemFree)
    {
        LogMsg("Error: FreeExtraRR invoked with unexpected error %d", result);
        return;
    }

    LogInfo("     FreeExtraRR %s", RRDisplayString(m, &rr->resrec));

    if (rr->resrec.rdata != &rr->rdatastorage)
        free(rr->resrec.rdata);
    free(extra);
}

mDNSlocal void unlink_and_free_service_instance(service_instance * srv)
{
    ExtraResourceRecord *e = srv->srs.Extras, *tmp;

    external_stop_advertising_helper(srv);

    // clear pointers from parent struct
    if (srv->request)
    {
        service_instance ** p = &srv->request->u.servicereg.instances;
        while (*p)
        {
            if (*p == srv)
            {
                *p = (*p)->next;
                break;
            }
            p = &(*p)->next;
        }
    }

    while (e)
    {
        e->r.RecordContext = e;
        tmp                = e;
        e                  = e->next;
        FreeExtraRR(&mDNSStorage, &tmp->r, mStatus_MemFree);
    }

#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (srv->srs.RR_TXT.resrec.rdata != &srv->srs.RR_TXT.rdatastorage)
        free(srv->srs.RR_TXT.resrec.rdata);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (srv->subtypes)
    {
        free(srv->subtypes);
        srv->subtypes = NULL;
    }
    if (srv->srs.AnonData)
    {
        free((void *) srv->srs.AnonData);
        srv->srs.AnonData = NULL;
    }
    free(srv);
}

// Count how many other service records we have locally with the same name, but different rdata.
// For auto-named services, we can have at most one per machine -- if we allowed two auto-named services of
// the same type on the same machine, we'd get into an infinite autoimmune-response loop of continuous renaming.
mDNSexport int CountPeerRegistrations(mDNS * const m, ServiceRecordSet * const srs)
{
    int count          = 0;
    ResourceRecord * r = &srs->RR_SRV.resrec;
    AuthRecord * rr;

    for (rr = m->ResourceRecords; rr; rr = rr->next)
        if (rr->resrec.rrtype == kDNSType_SRV && SameDomainName(rr->resrec.name, r->name) &&
            !IdenticalSameNameRecord(&rr->resrec, r))
            count++;

    verbosedebugf("%d peer registrations for %##s", count, r->name->c);
    return (count);
}

mDNSexport int CountExistingRegistrations(domainname * srv, mDNSIPPort port)
{
    int count = 0;
    AuthRecord * rr;
    for (rr = mDNSStorage.ResourceRecords; rr; rr = rr->next)
        if (rr->resrec.rrtype == kDNSType_SRV && mDNSSameIPPort(rr->resrec.rdata->u.srv.port, port) &&
            SameDomainName(rr->resrec.name, srv))
            count++;
    return (count);
}

mDNSlocal void SendServiceRemovalNotification(ServiceRecordSet * const srs)
{
    reply_state * rep;
    service_instance * instance = srs->ServiceContext;
    if (GenerateNTDResponse(srs->RR_SRV.resrec.name, srs->RR_SRV.resrec.InterfaceID, instance->request, &rep, reg_service_reply_op,
                            0, mStatus_NoError) != mStatus_NoError)
        LogMsg("%3d: SendServiceRemovalNotification: %##s is not valid DNS-SD SRV name", instance->request->sd,
               srs->RR_SRV.resrec.name->c);
    else
    {
        append_reply(instance->request, rep);
        instance->clientnotified = mDNSfalse;
    }
}

// service registration callback performs three duties - frees memory for deregistered services,
// handles name conflicts, and delivers completed registration information to the client
mDNSlocal void regservice_callback(mDNS * const m, ServiceRecordSet * const srs, mStatus result)
{
    mStatus err;
    mDNSBool SuppressError = mDNSfalse;
    service_instance * instance;
    reply_state * rep;
    (void) m; // Unused

    if (!srs)
    {
        LogMsg("regservice_callback: srs is NULL %d", result);
        return;
    }

    instance = srs->ServiceContext;
    if (!instance)
    {
        LogMsg("regservice_callback: srs->ServiceContext is NULL %d", result);
        return;
    }

    // don't send errors up to client for wide-area, empty-string registrations
    if (instance->request && instance->request->u.servicereg.default_domain && !instance->default_local)
        SuppressError = mDNStrue;
#ifndef REMOVE_mDNS_LoggingEnabled
    if (mDNS_LoggingEnabled)
    {
        const char * const fmt = (result == mStatus_NoError) ? "%s DNSServiceRegister(%##s, %u) REGISTERED"
            : (result == mStatus_MemFree)                    ? "%s DNSServiceRegister(%##s, %u) DEREGISTERED"
            : (result == mStatus_NameConflict)               ? "%s DNSServiceRegister(%##s, %u) NAME CONFLICT"
                                                             : "%s DNSServiceRegister(%##s, %u) %s %d";
        char prefix[16]        = "---:";
        if (instance->request)
            mDNS_snprintf(prefix, sizeof(prefix), "%3d:", instance->request->sd);
        LogOperation(fmt, prefix, srs->RR_SRV.resrec.name->c, mDNSVal16(srs->RR_SRV.resrec.rdata->u.srv.port),
                     SuppressError ? "suppressed error" : "CALLBACK", result);
    }
#endif // #ifndef REMOVE_mDNS_LoggingEnabled

    if (!instance->request && result != mStatus_MemFree)
    {
        LogMsg("regservice_callback: instance->request is NULL %d", result);
        return;
    }

    if (result == mStatus_NoError)
    {
        if (instance->request->u.servicereg.allowremotequery)
        {
            ExtraResourceRecord * e;
            srs->RR_ADV.AllowRemoteQuery = mDNStrue;
            srs->RR_PTR.AllowRemoteQuery = mDNStrue;
            srs->RR_SRV.AllowRemoteQuery = mDNStrue;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
            srs->RR_TXT.AllowRemoteQuery = mDNStrue;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
            for (e = instance->srs.Extras; e; e = e->next)
                e->r.AllowRemoteQuery = mDNStrue;
        }

        if (GenerateNTDResponse(srs->RR_SRV.resrec.name, srs->RR_SRV.resrec.InterfaceID, instance->request, &rep,
                                reg_service_reply_op, kDNSServiceFlagsAdd, result) != mStatus_NoError)
            LogMsg("%3d: regservice_callback: %##s is not valid DNS-SD SRV name", instance->request->sd,
                   srs->RR_SRV.resrec.name->c);
        else
        {
            append_reply(instance->request, rep);
            instance->clientnotified = mDNStrue;
        }

        if (callExternalHelpers(instance->request->u.servicereg.InterfaceID, &instance->domain, instance->request->flags))
        {
            LogInfo("regservice_callback: calling external_start_advertising_helper()");
            external_start_advertising_helper(instance);
        }
        if (instance->request->u.servicereg.autoname && CountPeerRegistrations(m, srs) == 0)
            RecordUpdatedNiceLabel(m, 0); // Successfully got new name, tell user immediately
    }
    else if (result == mStatus_MemFree)
    {
        if (instance->request && instance->renameonmemfree)
        {
            external_stop_advertising_helper(instance);
            instance->renameonmemfree = 0;
            err                       = mDNS_RenameAndReregisterService(m, srs, &instance->request->u.servicereg.name);
            if (err)
                LogMsg("ERROR: regservice_callback - RenameAndReregisterService returned %d", err);
            // error should never happen - safest to log and continue
        }
        else
            unlink_and_free_service_instance(instance);
    }
    else if (result == mStatus_NameConflict)
    {
        if (instance->request->u.servicereg.autorename)
        {
            external_stop_advertising_helper(instance);
            if (instance->request->u.servicereg.autoname && CountPeerRegistrations(m, srs) == 0)
            {
                // On conflict for an autoname service, rename and reregister *all* autoname services
                IncrementLabelSuffix(&m->nicelabel, mDNStrue);
                mDNS_ConfigChanged(m); // Will call back into udsserver_handle_configchange()
            }
            else // On conflict for a non-autoname service, rename and reregister just that one service
            {
                if (instance->clientnotified)
                    SendServiceRemovalNotification(srs);
                mDNS_RenameAndReregisterService(m, srs, mDNSNULL);
            }
        }
        else
        {
            if (!SuppressError)
            {
                if (GenerateNTDResponse(srs->RR_SRV.resrec.name, srs->RR_SRV.resrec.InterfaceID, instance->request, &rep,
                                        reg_service_reply_op, kDNSServiceFlagsAdd, result) != mStatus_NoError)
                    LogMsg("%3d: regservice_callback: %##s is not valid DNS-SD SRV name", instance->request->sd,
                           srs->RR_SRV.resrec.name->c);
                else
                {
                    append_reply(instance->request, rep);
                    instance->clientnotified = mDNStrue;
                }
            }
            unlink_and_free_service_instance(instance);
        }
    }
    else // Not mStatus_NoError, mStatus_MemFree, or mStatus_NameConflict
    {
        if (!SuppressError)
        {
            if (GenerateNTDResponse(srs->RR_SRV.resrec.name, srs->RR_SRV.resrec.InterfaceID, instance->request, &rep,
                                    reg_service_reply_op, kDNSServiceFlagsAdd, result) != mStatus_NoError)
                LogMsg("%3d: regservice_callback: %##s is not valid DNS-SD SRV name", instance->request->sd,
                       srs->RR_SRV.resrec.name->c);
            else
            {
                append_reply(instance->request, rep);
                instance->clientnotified = mDNStrue;
            }
        }
    }
}

mDNSlocal void regrecord_callback(mDNS * const m, AuthRecord * rr, mStatus result)
{
    (void) m;               // Unused
    if (!rr->RecordContext) // parent struct already freed by termination callback
    {
        if (result == mStatus_NoError)
            LogMsg("Error: regrecord_callback: successful registration of orphaned record %s", ARDisplayString(m, rr));
        else
        {
            if (result != mStatus_MemFree)
                LogMsg("regrecord_callback: error %d received after parent termination", result);

            // We come here when the record is being deregistered either from DNSServiceRemoveRecord or connection_termination.
            // If the record has been updated, we need to free the rdata. Everytime we call mDNS_Update, it calls update_callback
            // with the old rdata (so that we can free it) and stores the new rdata in "rr->resrec.rdata". This means, we need
            // to free the latest rdata for which the update_callback was never called with.
            if (rr->resrec.rdata != &rr->rdatastorage)
                free(rr->resrec.rdata);
            free(rr);
        }
    }
    else
    {
        registered_record_entry * re = rr->RecordContext;
        request_state * request      = re->request;
#ifndef REMOVE_mDNS_LoggingEnabled
        if (mDNS_LoggingEnabled)
        {
            char * fmt = (result == mStatus_NoError) ? "%3d: DNSServiceRegisterRecord(%u %s) REGISTERED"
                : (result == mStatus_MemFree)        ? "%3d: DNSServiceRegisterRecord(%u %s) DEREGISTERED"
                : (result == mStatus_NameConflict)   ? "%3d: DNSServiceRegisterRecord(%u %s) NAME CONFLICT"
                                                     : "%3d: DNSServiceRegisterRecord(%u %s) %d";
            LogOperation(fmt, request->sd, re->key, RRDisplayString(m, &rr->resrec), result);
        }
#endif // #ifndef REMOVE_mDNS_LoggingEnabled
        if (result != mStatus_MemFree)
        {
            int len                     = sizeof(DNSServiceFlags) + sizeof(mDNSu32) + sizeof(DNSServiceErrorType);
            reply_state * reply         = create_reply(reg_record_reply_op, len, request);
            reply->mhdr->client_context = re->regrec_client_context;
            reply->rhdr->flags          = dnssd_htonl(0);
            reply->rhdr->ifi   = dnssd_htonl(mDNSPlatformInterfaceIndexfromInterfaceID(m, rr->resrec.InterfaceID, mDNSfalse));
            reply->rhdr->error = dnssd_htonl(result);
            append_reply(request, reply);
        }

        if (result)
        {
            // If this is a callback to a keepalive record, do not free it.
            if (result == mStatus_BadStateErr)
            {
                LogInfo("regrecord_callback: Callback with error code mStatus_BadStateErr - not freeing the record.");
            }
            else
            {
                // unlink from list, free memory
                registered_record_entry ** ptr = &request->u.reg_recs;
                while (*ptr && (*ptr) != re)
                    ptr = &(*ptr)->next;
                if (!*ptr)
                {
                    LogMsg("regrecord_callback - record not in list!");
                    return;
                }
                *ptr = (*ptr)->next;
                free(re->rr);
                free(re);
            }
        }
        else
        {
            if (re->external_advertise)
                LogMsg("regrecord_callback: external_advertise already set!");

            if (callExternalHelpers(re->origInterfaceID, &rr->namestorage, request->flags))
            {
                LogInfo("regrecord_callback: calling external_start_advertising_service");
                external_start_advertising_service(&rr->resrec, request->flags);
                re->external_advertise = mDNStrue;
            }
        }
    }
}

// set_peer_pid() is called after mem is allocated for each new request in NewRequest()
// This accounts for 2 places (connect_callback, request_callback)
mDNSlocal void set_peer_pid(request_state * request)
{
    pid_t p              = (pid_t) -1;
    socklen_t len        = sizeof(p);
    request->pid_name[0] = '\0';
    request->process_id  = -1;
#ifdef LOCAL_PEERPID
    if (request->sd < 0)
        return;
    // to extract the pid value
    if (getsockopt(request->sd, SOL_LOCAL, LOCAL_PEERPID, &p, &len) != 0)
        return;
    // to extract the process name from the pid value
    if (proc_pidinfo(p, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0)
        return;
    mDNSPlatformStrCopy(request->pid_name, proc.pbsi_comm);
    request->process_id = p;
#else  // !LOCAL_PEERPID
    len = 0;
    if (request->sd < 0)
        return;
    LogInfo("set_peer_pid: Not Supported on this version of OS");
#endif // LOCAL_PEERPID
}

mDNSlocal void connection_termination(request_state * request)
{
    // When terminating a shared connection, we need to scan the all_requests list
    // and terminate any subbordinate operations sharing this file descriptor
    request_state ** req = &all_requests;

    LogOperation("%3d: DNSServiceCreateConnection STOP PID[%d](%s)", request->sd, request->process_id, request->pid_name);

    while (*req)
    {
        if ((*req)->primary == request)
        {
            // Since we're already doing a list traversal, we unlink the request directly instead of using AbortUnlinkAndFree()
            request_state * tmp = *req;
            if (tmp->primary == tmp)
                LogMsg("connection_termination ERROR (*req)->primary == *req for %p %d", tmp, tmp->sd);
            if (tmp->replies)
                LogMsg("connection_termination ERROR How can subordinate req %p %d have replies queued?", tmp, tmp->sd);
            abort_request(tmp);
            *req = tmp->next;
            free(tmp);
        }
        else
            req = &(*req)->next;
    }

    while (request->u.reg_recs)
    {
        registered_record_entry * ptr = request->u.reg_recs;
        LogOperation("%3d: DNSServiceRegisterRecord(%u %s) STOP PID[%d](%s)", request->sd, ptr->key,
                     RRDisplayString(&mDNSStorage, &ptr->rr->resrec), request->process_id, request->pid_name);
        request->u.reg_recs    = request->u.reg_recs->next;
        ptr->rr->RecordContext = NULL;
        if (ptr->external_advertise)
        {
            ptr->external_advertise = mDNSfalse;
            external_stop_advertising_service(&ptr->rr->resrec, request->flags);
        }
        LogMcastS(&mDNSStorage, ptr->rr, request, reg_stop);
        mDNS_Deregister(&mDNSStorage, ptr->rr); // Will free ptr->rr for us
        free(ptr);
    }
}

mDNSlocal void handle_cancel_request(request_state * request)
{
    request_state ** req = &all_requests;
    LogOperation("%3d: Cancel %08X %08X", request->sd, request->hdr.client_context.u32[1], request->hdr.client_context.u32[0]);
    while (*req)
    {
        if ((*req)->primary == request && (*req)->hdr.client_context.u32[0] == request->hdr.client_context.u32[0] &&
            (*req)->hdr.client_context.u32[1] == request->hdr.client_context.u32[1])
        {
            // Since we're already doing a list traversal, we unlink the request directly instead of using AbortUnlinkAndFree()
            request_state * tmp = *req;
            abort_request(tmp);
            *req = tmp->next;
            free(tmp);
        }
        else
            req = &(*req)->next;
    }
}

mDNSlocal mStatus handle_regrecord_request(request_state * request)
{
    mStatus err     = mStatus_BadParamErr;
    AuthRecord * rr = read_rr_from_ipc_msg(request, 1, 1);
    if (rr)
    {
        registered_record_entry * re;
        // Don't allow non-local domains to be regsitered as LocalOnly. Allowing this would permit
        // clients to register records such as www.bigbank.com A w.x.y.z to redirect Safari.
        if (rr->resrec.InterfaceID == mDNSInterface_LocalOnly && !IsLocalDomain(rr->resrec.name) &&
            rr->resrec.rrclass == kDNSClass_IN &&
            (rr->resrec.rrtype == kDNSType_A || rr->resrec.rrtype == kDNSType_AAAA || rr->resrec.rrtype == kDNSType_CNAME))
        {
            free(rr);
            return (mStatus_BadParamErr);
        }
        // allocate registration entry, link into list
        // re = mallocL("registered_record_entry", sizeof(registered_record_entry));
        re = malloc(sizeof(registered_record_entry));
        if (!re)
            FatalError("ERROR: malloc");
        re->key                   = request->hdr.reg_index;
        re->rr                    = rr;
        re->regrec_client_context = request->hdr.client_context;
        re->request               = request;
        re->external_advertise    = mDNSfalse;
        rr->RecordContext         = re;
        rr->RecordCallback        = regrecord_callback;

        re->origInterfaceID = rr->resrec.InterfaceID;
        if (rr->resrec.InterfaceID == mDNSInterface_P2P)
            rr->resrec.InterfaceID = mDNSInterface_Any;
#if 0
        if (!AuthorizedDomain(request, rr->resrec.name, AutoRegistrationDomains)) return (mStatus_NoError);
#endif
        if (rr->resrec.rroriginalttl == 0)
            rr->resrec.rroriginalttl = DefaultTTLforRRType(rr->resrec.rrtype);

        LogOperation("%3d: DNSServiceRegisterRecord(%u %s) START PID[%d](%s)", request->sd, re->key,
                     RRDisplayString(&mDNSStorage, &rr->resrec), request->process_id, request->pid_name);

        err = mDNS_Register(&mDNSStorage, rr);
        if (err)
        {
            LogOperation("%3d: DNSServiceRegisterRecord(%u %s) ERROR (%d)", request->sd, re->key,
                         RRDisplayString(&mDNSStorage, &rr->resrec), err);
            free(re);
            free(rr);
        }
        else
        {
            LogMcastS(&mDNSStorage, rr, request, reg_start);
            re->next            = request->u.reg_recs;
            request->u.reg_recs = re;
        }
    }
    return (err);
}

mDNSlocal void UpdateDeviceInfoRecord(mDNS * const m);

mDNSlocal void regservice_termination_callback(request_state * request)
{
    if (!request)
    {
        LogMsg("regservice_termination_callback context is NULL");
        return;
    }
    while (request->u.servicereg.instances)
    {
        service_instance * p            = request->u.servicereg.instances;
        request->u.servicereg.instances = request->u.servicereg.instances->next;
        // only safe to free memory if registration is not valid, i.e. deregister fails (which invalidates p)
        LogOperation("%3d: DNSServiceRegister(%##s, %u) STOP PID[%d](%s)", request->sd, p->srs.RR_SRV.resrec.name->c,
                     mDNSVal16(p->srs.RR_SRV.resrec.rdata->u.srv.port), request->process_id, request->pid_name);

        external_stop_advertising_helper(p);

        // Clear backpointer *before* calling mDNS_DeregisterService/unlink_and_free_service_instance
        // We don't need unlink_and_free_service_instance to cut its element from the list, because we're already advancing
        // request->u.servicereg.instances as we work our way through the list, implicitly cutting one element at a time
        // We can't clear p->request *after* the calling mDNS_DeregisterService/unlink_and_free_service_instance
        // because by then we might have already freed p
        p->request = NULL;
        LogMcastS(&mDNSStorage, &p->srs.RR_SRV, request, reg_stop);
        if (mDNS_DeregisterService(&mDNSStorage, &p->srs))
        {
            unlink_and_free_service_instance(p);
            // Don't touch service_instance *p after this -- it's likely to have been freed already
        }
    }
    if (request->u.servicereg.txtdata)
    {
        free(request->u.servicereg.txtdata);
        request->u.servicereg.txtdata = NULL;
    }
    if (request->u.servicereg.autoname)
    {
        // Clear autoname before calling UpdateDeviceInfoRecord() so it doesn't mistakenly include this in its count of active
        // autoname registrations
        request->u.servicereg.autoname = mDNSfalse;
        UpdateDeviceInfoRecord(&mDNSStorage);
    }
}

mDNSlocal request_state * LocateSubordinateRequest(request_state * request)
{
    request_state * req;
    for (req = all_requests; req; req = req->next)
        if (req->primary == request && req->hdr.client_context.u32[0] == request->hdr.client_context.u32[0] &&
            req->hdr.client_context.u32[1] == request->hdr.client_context.u32[1])
            return (req);
    return (request);
}

mDNSlocal mStatus add_record_to_service(request_state * request, service_instance * instance, mDNSu16 rrtype, mDNSu16 rdlen,
                                        const char * rdata, mDNSu32 ttl)
{
    ServiceRecordSet * srs = &instance->srs;
    mStatus result;
    mDNSu32 coreFlags = 0; // translate to corresponding mDNSCore flag definitions
    int size          = rdlen > sizeof(RDataBody) ? rdlen : sizeof(RDataBody);
    // ExtraResourceRecord *extra = mallocL("ExtraResourceRecord", sizeof(*extra) - sizeof(RDataBody) + size);
    ExtraResourceRecord * extra = malloc(sizeof(*extra) - sizeof(RDataBody) + size);
    if (!extra)
    {
        my_perror("ERROR: malloc");
        return mStatus_NoMemoryErr;
    }

    mDNSPlatformMemZero(extra, sizeof(ExtraResourceRecord)); // OK if oversized rdata not zero'd
    extra->r.resrec.rrtype            = rrtype;
    extra->r.rdatastorage.MaxRDLength = (mDNSu16) size;
    extra->r.resrec.rdlength          = rdlen;
    mDNSPlatformMemCopy(&extra->r.rdatastorage.u.data, rdata, rdlen);
    // use InterfaceID value from DNSServiceRegister() call that created the original service
    extra->r.resrec.InterfaceID = request->u.servicereg.InterfaceID;

    if (request->flags & kDNSServiceFlagsIncludeP2P)
        coreFlags |= coreFlagIncludeP2P;
    if (request->flags & kDNSServiceFlagsIncludeAWDL)
        coreFlags |= coreFlagIncludeAWDL;

    result = mDNS_AddRecordToService(&mDNSStorage, srs, extra, &extra->r.rdatastorage, ttl, coreFlags);
    if (result)
    {
        free(extra);
        return result;
    }
    LogMcastS(&mDNSStorage, &srs->RR_PTR, request, reg_start);

    extra->ClientID = request->hdr.reg_index;
    if (instance->external_advertise && callExternalHelpers(request->u.servicereg.InterfaceID, &instance->domain, request->flags))
    {
        LogInfo("add_record_to_service: calling external_start_advertising_service");
        external_start_advertising_service(&extra->r.resrec, request->flags);
    }
    return result;
}

mDNSlocal mStatus handle_add_request(request_state * request)
{
    service_instance * i;
    mStatus result        = mStatus_UnknownErr;
    DNSServiceFlags flags = get_flags(&request->msgptr, request->msgend);
    mDNSu16 rrtype        = get_uint16(&request->msgptr, request->msgend);
    mDNSu16 rdlen         = get_uint16(&request->msgptr, request->msgend);
    const char * rdata    = get_rdata(&request->msgptr, request->msgend, rdlen);
    mDNSu32 ttl           = get_uint32(&request->msgptr, request->msgend);
    if (!ttl)
        ttl = DefaultTTLforRRType(rrtype);
    (void) flags; // Unused

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceAddRecord(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    // If this is a shared connection, check if the operation actually applies to a subordinate request_state object
    if (request->terminate == connection_termination)
        request = LocateSubordinateRequest(request);

    if (request->terminate != regservice_termination_callback)
    {
        LogMsg("%3d: DNSServiceAddRecord(not a registered service ref)", request->sd);
        return (mStatus_BadParamErr);
    }

    // For a service registered with zero port, don't allow adding records. This mostly happens due to a bug
    // in the application. See radar://9165807.
    if (mDNSIPPortIsZero(request->u.servicereg.port))
    {
        LogMsg("%3d: DNSServiceAddRecord: adding record to a service registered with zero port", request->sd);
        return (mStatus_BadParamErr);
    }

    LogOperation("%3d: DNSServiceAddRecord(%X, %##s, %s, %d)", request->sd, flags,
                 (request->u.servicereg.instances) ? request->u.servicereg.instances->srs.RR_SRV.resrec.name->c : NULL,
                 DNSTypeName(rrtype), rdlen);

    for (i = request->u.servicereg.instances; i; i = i->next)
    {
        result = add_record_to_service(request, i, rrtype, rdlen, rdata, ttl);
        if (result && i->default_local)
            break;
        else
            result = mStatus_NoError; // suppress non-local default errors
    }

    return (result);
}

mDNSlocal void update_callback(mDNS * const m, AuthRecord * const rr, RData * oldrd, mDNSu16 oldrdlen)
{
    mDNSBool external_advertise = (rr->UpdateContext) ? *((mDNSBool *) rr->UpdateContext) : mDNSfalse;
    (void) m; // Unused

    // There are three cases.
    //
    // 1. We have updated the primary TXT record of the service
    // 2. We have updated the TXT record that was added to the service using DNSServiceAddRecord
    // 3. We have updated the TXT record that was registered using DNSServiceRegisterRecord
    //
    // external_advertise is set if we have advertised at least once during the initial addition
    // of the record in all of the three cases above. We should have checked for InterfaceID/LocalDomain
    // checks during the first time and hence we don't do any checks here
    if (external_advertise)
    {
        ResourceRecord ext    = rr->resrec;
        DNSServiceFlags flags = 0;

        // Since we don't have a copy of the flags value used when the record was registered,
        // we'll have to derive it from the ARType field.
        if (rr->ARType == AuthRecordAnyIncludeP2P)
            flags |= kDNSServiceFlagsIncludeP2P;
        else if (rr->ARType == AuthRecordAnyIncludeAWDL)
            flags |= kDNSServiceFlagsIncludeAWDL;

        if (ext.rdlength == oldrdlen && mDNSPlatformMemSame(&ext.rdata->u, &oldrd->u, oldrdlen))
            goto exit;
        SetNewRData(&ext, oldrd, oldrdlen);
        external_stop_advertising_service(&ext, flags);
        LogInfo("update_callback: calling external_start_advertising_service");
        external_start_advertising_service(&rr->resrec, flags);
    }
exit:
    if (oldrd != &rr->rdatastorage)
        free(oldrd);
}

mDNSlocal mStatus update_record(AuthRecord * rr, mDNSu16 rdlen, const char * rdata, mDNSu32 ttl,
                                const mDNSBool * const external_advertise)
{
    mStatus result;
    const int rdsize = rdlen > sizeof(RDataBody) ? rdlen : sizeof(RDataBody);
    // RData *newrd = mallocL("RData/update_record", sizeof(RData) - sizeof(RDataBody) + rdsize);
    RData * newrd = malloc(sizeof(RData) - sizeof(RDataBody) + rdsize);
    if (!newrd)
        FatalError("ERROR: malloc");
    newrd->MaxRDLength = (mDNSu16) rdsize;
    mDNSPlatformMemCopy(&newrd->u, rdata, rdlen);

    // BIND named (name daemon) doesn't allow TXT records with zero-length rdata. This is strictly speaking correct,
    // since RFC 1035 specifies a TXT record as "One or more <character-string>s", not "Zero or more <character-string>s".
    // Since some legacy apps try to create zero-length TXT records, we'll silently correct it here.
    if (rr->resrec.rrtype == kDNSType_TXT && rdlen == 0)
    {
        rdlen             = 1;
        newrd->u.txt.c[0] = 0;
    }

    if (external_advertise)
        rr->UpdateContext = (void *) external_advertise;

    result = mDNS_Update(&mDNSStorage, rr, ttl, rdlen, newrd, update_callback);
    if (result)
    {
        LogMsg("update_record: Error %d for %s", (int) result, ARDisplayString(&mDNSStorage, rr));
        free(newrd);
    }
    return result;
}

mDNSlocal mStatus handle_update_request(request_state * request)
{
    const ipc_msg_hdr * const hdr = &request->hdr;
    mStatus result                = mStatus_BadReferenceErr;
    service_instance * i;
    AuthRecord * rr = NULL;

    // get the message data
    DNSServiceFlags flags = get_flags(&request->msgptr, request->msgend); // flags unused
    mDNSu16 rdlen         = get_uint16(&request->msgptr, request->msgend);
    const char * rdata    = get_rdata(&request->msgptr, request->msgend, rdlen);
    mDNSu32 ttl           = get_uint32(&request->msgptr, request->msgend);
    (void) flags; // Unused

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceUpdateRecord(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    // If this is a shared connection, check if the operation actually applies to a subordinate request_state object
    if (request->terminate == connection_termination)
        request = LocateSubordinateRequest(request);

    if (request->terminate == connection_termination)
    {
        // update an individually registered record
        registered_record_entry * reptr;
        for (reptr = request->u.reg_recs; reptr; reptr = reptr->next)
        {
            if (reptr->key == hdr->reg_index)
            {
                result = update_record(reptr->rr, rdlen, rdata, ttl, &reptr->external_advertise);
                LogOperation("%3d: DNSServiceUpdateRecord(%##s, %s)", request->sd, reptr->rr->resrec.name->c,
                             reptr->rr ? DNSTypeName(reptr->rr->resrec.rrtype) : "<NONE>");
                goto end;
            }
        }
        result = mStatus_BadReferenceErr;
        goto end;
    }

    if (request->terminate != regservice_termination_callback)
    {
        LogMsg("%3d: DNSServiceUpdateRecord(not a registered service ref)", request->sd);
        return (mStatus_BadParamErr);
    }

    // For a service registered with zero port, only SRV record is initialized. Don't allow any updates.
    if (mDNSIPPortIsZero(request->u.servicereg.port))
    {
        LogMsg("%3d: DNSServiceUpdateRecord: updating the record of a service registered with zero port", request->sd);
        return (mStatus_BadParamErr);
    }
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // update the saved off TXT data for the service
    if (hdr->reg_index == TXT_RECORD_INDEX)
    {
        if (request->u.servicereg.txtdata)
        {
            free(request->u.servicereg.txtdata);
            request->u.servicereg.txtdata = NULL;
        }
        if (rdlen > 0)
        {
            // request->u.servicereg.txtdata = mallocL("service_info txtdata", rdlen);
            request->u.servicereg.txtdata = malloc(rdlen);
            if (!request->u.servicereg.txtdata)
                FatalError("ERROR: handle_update_request - malloc");
            mDNSPlatformMemCopy(request->u.servicereg.txtdata, rdata, rdlen);
        }
        request->u.servicereg.txtlen = rdlen;
    }
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

    // update a record from a service record set
    for (i = request->u.servicereg.instances; i; i = i->next)
    {
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
        if (hdr->reg_index == TXT_RECORD_INDEX)
            rr = &i->srs.RR_TXT;
        else
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
        {
            ExtraResourceRecord * e;
            for (e = i->srs.Extras; e; e = e->next)
                if (e->ClientID == hdr->reg_index)
                {
                    rr = &e->r;
                    break;
                }
        }
        if (!rr)
        {
            result = mStatus_BadReferenceErr;
            goto end;
        }
        result = update_record(rr, rdlen, rdata, ttl, &i->external_advertise);
        if (result && i->default_local)
            goto end;
        else
            result = mStatus_NoError; // suppress non-local default errors
    }

end:
    if (request->terminate == regservice_termination_callback)
        LogOperation("%3d: DNSServiceUpdateRecord(%##s, %s)", request->sd,
                     (request->u.servicereg.instances) ? request->u.servicereg.instances->srs.RR_SRV.resrec.name->c : NULL,
                     rr ? DNSTypeName(rr->resrec.rrtype) : "<NONE>");

    return (result);
}

// remove a resource record registered via DNSServiceRegisterRecord()
mDNSlocal mStatus remove_record(request_state * request)
{
    mStatus err = mStatus_UnknownErr;
    registered_record_entry *e, **ptr = &request->u.reg_recs;

    while (*ptr && (*ptr)->key != request->hdr.reg_index)
        ptr = &(*ptr)->next;
    if (!*ptr)
    {
        LogMsg("%3d: DNSServiceRemoveRecord(%u) not found", request->sd, request->hdr.reg_index);
        return mStatus_BadReferenceErr;
    }
    e    = *ptr;
    *ptr = e->next; // unlink

    LogOperation("%3d: DNSServiceRemoveRecord(%u %s)", request->sd, e->key, RRDisplayString(&mDNSStorage, &e->rr->resrec));
    e->rr->RecordContext = NULL;
    if (e->external_advertise)
    {
        external_stop_advertising_service(&e->rr->resrec, request->flags);
        e->external_advertise = mDNSfalse;
    }
    LogMcastS(&mDNSStorage, e->rr, request, reg_stop);
    err = mDNS_Deregister(&mDNSStorage, e->rr); // Will free e->rr for us; we're responsible for freeing e
    if (err)
    {
        LogMsg("ERROR: remove_record, mDNS_Deregister: %d", err);
        free(e->rr);
    }
    free(e);
    return err;
}

mDNSlocal mStatus remove_extra(const request_state * const request, service_instance * const serv, mDNSu16 * const rrtype)
{
    mStatus err = mStatus_BadReferenceErr;
    ExtraResourceRecord * ptr;

    for (ptr = serv->srs.Extras; ptr; ptr = ptr->next)
    {
        if (ptr->ClientID == request->hdr.reg_index) // found match
        {
            *rrtype = ptr->r.resrec.rrtype;
            if (serv->external_advertise)
                external_stop_advertising_service(&ptr->r.resrec, request->flags);
            err = mDNS_RemoveRecordFromService(&mDNSStorage, &serv->srs, ptr, FreeExtraRR, ptr);
            break;
        }
    }
    return err;
}

mDNSlocal mStatus handle_removerecord_request(request_state * request)
{
    mStatus err = mStatus_BadReferenceErr;
    get_flags(&request->msgptr, request->msgend); // flags unused

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceRemoveRecord(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    // If this is a shared connection, check if the operation actually applies to a subordinate request_state object
    if (request->terminate == connection_termination)
        request = LocateSubordinateRequest(request);

    if (request->terminate == connection_termination)
        err = remove_record(request); // remove individually registered record
    else if (request->terminate != regservice_termination_callback)
    {
        LogMsg("%3d: DNSServiceRemoveRecord(not a registered service ref)", request->sd);
        return (mStatus_BadParamErr);
    }
    else
    {
        service_instance * i;
        mDNSu16 rrtype = 0;
        LogOperation("%3d: DNSServiceRemoveRecord(%##s, %s)", request->sd,
                     (request->u.servicereg.instances) ? request->u.servicereg.instances->srs.RR_SRV.resrec.name->c : NULL,
                     rrtype ? DNSTypeName(rrtype) : "<NONE>");
        for (i = request->u.servicereg.instances; i; i = i->next)
        {
            err = remove_extra(request, i, &rrtype);
            if (err && i->default_local)
                break;
            else
                err = mStatus_NoError; // suppress non-local default errors
        }
    }

    return (err);
}

// If there's a comma followed by another character,
// FindFirstSubType overwrites the comma with a nul and returns the pointer to the next character.
// Otherwise, it returns a pointer to the final nul at the end of the string
mDNSlocal char * FindFirstSubType(char * p, char ** AnonData)
{
    while (*p)
    {
        if (p[0] == '\\' && p[1])
        {
            p += 2;
        }
        else if (p[0] == ',' && p[1])
        {
            *p++ = 0;
            return (p);
        }
        else if (p[0] == ':' && p[1])
        {
            *p++      = 0;
            *AnonData = p;
        }
        else
        {
            p++;
        }
    }
    return (p);
}

// If there's a comma followed by another character,
// FindNextSubType overwrites the comma with a nul and returns the pointer to the next character.
// If it finds an illegal unescaped dot in the subtype name, it returns mDNSNULL
// Otherwise, it returns a pointer to the final nul at the end of the string
mDNSlocal char * FindNextSubType(char * p)
{
    while (*p)
    {
        if (p[0] == '\\' && p[1]) // If escape character
            p += 2;               // ignore following character
        else if (p[0] == ',')     // If we found a comma
        {
            if (p[1])
                *p++ = 0;
            return (p);
        }
        else if (p[0] == '.')
            return (mDNSNULL);
        else
            p++;
    }
    return (p);
}

// Returns -1 if illegal subtype found
mDNSexport mDNSs32 ChopSubTypes(char * regtype, char ** AnonData)
{
    mDNSs32 NumSubTypes = 0;
    char * stp          = FindFirstSubType(regtype, AnonData);
    while (stp && *stp) // If we found a comma...
    {
        if (*stp == ',')
            return (-1);
        NumSubTypes++;
        stp = FindNextSubType(stp);
    }
    if (!stp)
        return (-1);
    return (NumSubTypes);
}

mDNSexport AuthRecord * AllocateSubTypes(mDNSs32 NumSubTypes, char * p, char ** AnonData)
{
    AuthRecord * st = mDNSNULL;
    //
    // "p" is pointing at the regtype e.g., _http._tcp followed by ":<AnonData>" indicated
    // by AnonData being non-NULL which is in turn follwed by ",<SubTypes>" indicated by
    // NumSubTypes being non-zero. We need to skip the initial regtype to get to the actual
    // data that we want. When we come here, ChopSubTypes has null terminated like this e.g.,
    //
    // _http._tcp<NULL><AnonData><NULL><SubType1><NULL><SubType2><NULL> etc.
    //
    // 1. If we have Anonymous data and subtypes, skip the regtype (e.g., "_http._tcp")
    //    to get the AnonData and then skip the AnonData to get to the SubType.
    //
    // 2. If we have only SubTypes, skip the regtype to get to the SubType data.
    //
    // 3. If we have only AnonData, skip the regtype to get to the AnonData.
    //
    // 4. If we don't have AnonData or NumStypes, it is a noop.
    //
    if (AnonData)
    {
        int len;

        // Skip the regtype
        while (*p)
            p++;
        p++;

        len = strlen(p) + 1;
        //*AnonData = mallocL("Anonymous", len);
        *AnonData = malloc(len);
        if (!(*AnonData))
        {
            return (mDNSNULL);
        }
        mDNSPlatformMemCopy(*AnonData, p, len);
    }
    if (NumSubTypes)
    {
        mDNSs32 i;
        // st = mallocL("ServiceSubTypes", NumSubTypes * sizeof(AuthRecord));
        st = malloc(NumSubTypes * sizeof(AuthRecord));
        if (!st)
            return (mDNSNULL);
        for (i = 0; i < NumSubTypes; i++)
        {
            mDNS_SetupResourceRecord(&st[i], mDNSNULL, mDNSInterface_Any, kDNSQType_ANY, kStandardTTL, 0, AuthRecordAny, mDNSNULL,
                                     mDNSNULL);
            // First time through we skip the regtype or AnonData. Subsequently, the
            // previous subtype.
            while (*p)
                p++;
            p++;
            if (!MakeDomainNameFromDNSNameString(&st[i].namestorage, p))
            {
                free(st);
                if (*AnonData)
                    free(*AnonData);
                return (mDNSNULL);
            }
        }
    }
    // If NumSubTypes is zero and AnonData is non-NULL, we still return NULL but AnonData has been
    // initialized. The caller knows how to handle this.
    return (st);
}

mDNSlocal mStatus register_service_instance(request_state * request, const domainname * domain)
{
    service_instance **ptr, *instance;
    const int extra_size =
        (request->u.servicereg.txtlen > sizeof(RDataBody)) ? (request->u.servicereg.txtlen - sizeof(RDataBody)) : 0;
    const mDNSBool DomainIsLocal = SameDomainName(domain, &localdomain);
    mStatus result;
    mDNSInterfaceID interfaceID = request->u.servicereg.InterfaceID;
    mDNSu32 coreFlags           = 0;

    if (request->flags & kDNSServiceFlagsIncludeP2P)
        coreFlags |= coreFlagIncludeP2P;
    if (request->flags & kDNSServiceFlagsIncludeAWDL)
        coreFlags |= coreFlagIncludeAWDL;

    // Client guarantees that record names are unique, so we can skip sending out initial
    // probe messages.  Standard name conflict resolution is still done if a conflict is discovered.
    if (request->flags & kDNSServiceFlagsKnownUnique)
        coreFlags |= coreFlagKnownUnique;

    if (request->flags & kDNSServiceFlagsWakeOnlyService)
        coreFlags |= coreFlagWakeOnly;

    // If the client specified an interface, but no domain, then we honor the specified interface for the "local" (mDNS)
    // registration but for the wide-area registrations we don't (currently) have any concept of a wide-area unicast
    // registrations scoped to a specific interface, so for the automatic domains we add we must *not* specify an interface.
    // (Specifying an interface with an apparently wide-area domain (i.e. something other than "local")
    // currently forces the registration to use mDNS multicast despite the apparently wide-area domain.)
    if (request->u.servicereg.default_domain && !DomainIsLocal)
        interfaceID = mDNSInterface_Any;

    for (ptr = &request->u.servicereg.instances; *ptr; ptr = &(*ptr)->next)
    {
        if (SameDomainName(&(*ptr)->domain, domain))
        {
            LogMsg("register_service_instance: domain %##s already registered for %#s.%##s", domain->c, &request->u.servicereg.name,
                   &request->u.servicereg.type);
            return mStatus_AlreadyRegistered;
        }
    }

    // instance = mallocL("service_instance", sizeof(*instance) + extra_size);
    instance = malloc(sizeof(*instance) + extra_size);
    if (!instance)
    {
        my_perror("ERROR: malloc");
        return mStatus_NoMemoryErr;
    }

    instance->next               = mDNSNULL;
    instance->request            = request;
    instance->renameonmemfree    = 0;
    instance->clientnotified     = mDNSfalse;
    instance->default_local      = (request->u.servicereg.default_domain && DomainIsLocal);
    instance->external_advertise = mDNSfalse;
    AssignDomainName(&instance->domain, domain);

    instance->srs.AnonData = mDNSNULL;
    if (!request->u.servicereg.AnonData)
    {
        instance->subtypes = AllocateSubTypes(request->u.servicereg.num_subtypes, request->u.servicereg.type_as_string, mDNSNULL);
    }
    else
    {
        char * AnonData    = mDNSNULL;
        instance->subtypes = AllocateSubTypes(request->u.servicereg.num_subtypes, request->u.servicereg.type_as_string, &AnonData);
        if (AnonData)
            instance->srs.AnonData = (const mDNSu8 *) AnonData;
    }

    if (request->u.servicereg.num_subtypes && !instance->subtypes)
    {
        unlink_and_free_service_instance(instance);
        instance = NULL;
        FatalError("ERROR: malloc");
    }

    result = mDNS_RegisterService(&mDNSStorage, &instance->srs, &request->u.servicereg.name, &request->u.servicereg.type, domain,
                                  request->u.servicereg.host.c[0] ? &request->u.servicereg.host : NULL, request->u.servicereg.port,
                                  request->u.servicereg.txtdata, request->u.servicereg.txtlen, instance->subtypes,
                                  request->u.servicereg.num_subtypes, interfaceID, regservice_callback, instance, coreFlags);

    if (!result)
    {
        *ptr = instance; // Append this to the end of our request->u.servicereg.instances list
        LogOperation("%3d: DNSServiceRegister(%##s, %u) ADDED", instance->request->sd, instance->srs.RR_SRV.resrec.name->c,
                     mDNSVal16(request->u.servicereg.port));
        LogMcastS(&mDNSStorage, &instance->srs.RR_SRV, request, reg_start);
    }
    else
    {
        LogMsg("register_service_instance %#s.%##s%##s error %d", &request->u.servicereg.name, &request->u.servicereg.type,
               domain->c, result);
        unlink_and_free_service_instance(instance);
    }

    return result;
}

mDNSlocal void udsserver_default_reg_domain_changed(const DNameListElem * const d, const mDNSBool add)
{
    request_state * request;

#if APPLE_OSX_mDNSResponder
    machserver_automatic_registration_domain_changed(&d->name, add);
#endif // APPLE_OSX_mDNSResponder

    LogMsg("%s registration domain %##s", add ? "Adding" : "Removing", d->name.c);
    for (request = all_requests; request; request = request->next)
    {
        if (request->terminate != regservice_termination_callback)
            continue;
        if (!request->u.servicereg.default_domain)
            continue;
        if (!d->uid || SystemUID(request->uid) || request->uid == d->uid)
        {
            service_instance ** ptr = &request->u.servicereg.instances;
            while (*ptr && !SameDomainName(&(*ptr)->domain, &d->name))
                ptr = &(*ptr)->next;
            if (add)
            {
                // If we don't already have this domain in our list for this registration, add it now
                if (!*ptr)
                    register_service_instance(request, &d->name);
                else
                    debugf("udsserver_default_reg_domain_changed %##s already in list, not re-adding", &d->name);
            }
            else
            {
                // Normally we should not fail to find the specified instance
                // One case where this can happen is if a uDNS update fails for some reason,
                // and regservice_callback then calls unlink_and_free_service_instance and disposes of that instance.
                if (!*ptr)
                    LogMsg("udsserver_default_reg_domain_changed domain %##s not found for service %#s type %s", &d->name,
                           request->u.servicereg.name.c, request->u.servicereg.type_as_string);
                else
                {
                    DNameListElem * p;
                    for (p = AutoRegistrationDomains; p; p = p->next)
                        if (!p->uid || SystemUID(request->uid) || request->uid == p->uid)
                            if (SameDomainName(&d->name, &p->name))
                                break;
                    if (p)
                        debugf("udsserver_default_reg_domain_changed %##s still in list, not removing", &d->name);
                    else
                    {
                        mStatus err;
                        service_instance * si = *ptr;
                        *ptr                  = si->next;
                        if (si->clientnotified)
                            SendServiceRemovalNotification(&si->srs); // Do this *before* clearing si->request backpointer
                        // Now that we've cut this service_instance from the list, we MUST clear the si->request backpointer.
                        // Otherwise what can happen is this: While our mDNS_DeregisterService is in the
                        // process of completing asynchronously, the client cancels the entire operation, so
                        // regservice_termination_callback then runs through the whole list deregistering each
                        // instance, clearing the backpointers, and then disposing the parent request_state object.
                        // However, because this service_instance isn't in the list any more, regservice_termination_callback
                        // has no way to find it and clear its backpointer, and then when our mDNS_DeregisterService finally
                        // completes later with a mStatus_MemFree message, it calls unlink_and_free_service_instance() with
                        // a service_instance with a stale si->request backpointer pointing to memory that's already been freed.
                        si->request = NULL;
                        err         = mDNS_DeregisterService(&mDNSStorage, &si->srs);
                        if (err)
                        {
                            LogMsg("udsserver_default_reg_domain_changed err %d", err);
                            unlink_and_free_service_instance(si);
                        }
                    }
                }
            }
        }
    }
}

// Don't allow normal and anonymous registration to coexist.
mDNSlocal mDNSBool CheckForMixedRegistrations(domainname * regtype, domainname * domain, mDNSBool AnonData)
{
    request_state * request;

    // We only care about local domains where the anonymous extension is
    // implemented.
    if (!SameDomainName(domain,
                        (const domainname *) "\x5"
                                             "local"))
    {
        return mDNStrue;
    }

    for (request = all_requests; request; request = request->next)
    {
        service_instance * ptr;

        if (request->terminate != regservice_termination_callback)
            continue;
        for (ptr = request->u.servicereg.instances; ptr; ptr = ptr->next)
        {
            if (!SameDomainName(&ptr->domain,
                                (const domainname *) "\x5"
                                                     "local") ||
                !SameDomainName(&request->u.servicereg.type, regtype))
            {
                continue;
            }

            // If we are about to register a anonymous registraion, we dont't want to
            // allow the regular ones and vice versa.
            if (AnonData)
            {
                if (!ptr->srs.AnonData)
                {
                    LogMsg("CheckForMixedRegistrations: Normal registration already exists for %##s", regtype->c);
                    return mDNSfalse;
                }
            }
            else
            {
                // Allow multiple regular registrations
                if (ptr->srs.AnonData)
                {
                    LogMsg("CheckForMixedRegistrations: Anonymous registration already exists for %##s", regtype->c);
                    return mDNSfalse;
                }
            }
        }
    }
    return mDNStrue;
}

mDNSlocal mStatus handle_regservice_request(request_state * request)
{
    // char name[256]; // Lots of spare space for extra-long names that we'll auto-truncate down to 63 bytes
    char name[64]; // Lots of spare space for extra-long names that we'll auto-truncate down to 63 bytes
    char domain[MAX_ESCAPED_DOMAIN_NAME], host[MAX_ESCAPED_DOMAIN_NAME];
    char type_as_string[MAX_ESCAPED_DOMAIN_NAME];
    domainname d, srv;
    mStatus err;
    char * AnonData = mDNSNULL;

    DNSServiceFlags flags  = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex = get_uint32(&request->msgptr, request->msgend);
    mDNSInterfaceID InterfaceID;

    // Map kDNSServiceInterfaceIndexP2P to kDNSServiceInterfaceIndexAny with the
    // kDNSServiceFlagsIncludeP2P flag set.
    if (interfaceIndex == kDNSServiceInterfaceIndexP2P)
    {
        LogOperation("handle_regservice_request: mapping kDNSServiceInterfaceIndexP2P to kDNSServiceInterfaceIndexAny + "
                     "kDNSServiceFlagsIncludeP2P");
        flags |= kDNSServiceFlagsIncludeP2P;
        interfaceIndex = kDNSServiceInterfaceIndexAny;
    }

    InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    if (interfaceIndex && !InterfaceID)
    {
        LogMsg("ERROR: handle_regservice_request - Couldn't find interfaceIndex %d", interfaceIndex);
        goto bad_param;
    }

    if (get_string(&request->msgptr, request->msgend, name, sizeof(name)) < 0 ||
        get_string(&request->msgptr, request->msgend, type_as_string, MAX_ESCAPED_DOMAIN_NAME) < 0 ||
        get_string(&request->msgptr, request->msgend, domain, MAX_ESCAPED_DOMAIN_NAME) < 0 ||
        get_string(&request->msgptr, request->msgend, host, MAX_ESCAPED_DOMAIN_NAME) < 0)
    {
        LogMsg("ERROR: handle_regservice_request - Couldn't read name/regtype/domain");
        return (mStatus_BadParamErr);
    }

    request->flags                    = flags;
    request->u.servicereg.InterfaceID = InterfaceID;
    request->u.servicereg.instances   = NULL;
    request->u.servicereg.txtlen      = 0;
    request->u.servicereg.txtdata     = NULL;
    mDNSPlatformStrCopy(request->u.servicereg.type_as_string, type_as_string);

    if (request->msgptr + 2 > request->msgend)
        request->msgptr = NULL;
    else
    {
        request->u.servicereg.port.b[0] = *request->msgptr++;
        request->u.servicereg.port.b[1] = *request->msgptr++;
    }

    request->u.servicereg.txtlen = get_uint16(&request->msgptr, request->msgend);
    if (request->u.servicereg.txtlen)
    {
        // request->u.servicereg.txtdata = mallocL("service_info txtdata", request->u.servicereg.txtlen);
        request->u.servicereg.txtdata = malloc(request->u.servicereg.txtlen);
        if (!request->u.servicereg.txtdata)
            FatalError("ERROR: handle_regservice_request - malloc");
        mDNSPlatformMemCopy(request->u.servicereg.txtdata,
                            get_rdata(&request->msgptr, request->msgend, request->u.servicereg.txtlen),
                            request->u.servicereg.txtlen);
    }

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceRegister(unreadable parameters)", request->sd);
        goto bad_param;
    }

    // Check for sub-types after the service type
    request->u.servicereg.num_subtypes =
        ChopSubTypes(request->u.servicereg.type_as_string, &AnonData); // Note: Modifies regtype string to remove trailing subtypes
    if (request->u.servicereg.num_subtypes < 0)
    {
        LogMsg("ERROR: handle_regservice_request - ChopSubTypes failed %s", request->u.servicereg.type_as_string);
        goto bad_param;
    }
    if (AnonData)
    {
        int AnonDataLen = strlen(AnonData);
        if (AnonDataLen > MAX_ANONYMOUS_DATA)
        {
            LogMsg("ERROR: handle_regservice_request: AnonDataLen %d", AnonDataLen);
            goto bad_param;
        }
        request->u.servicereg.AnonData = mDNStrue;
    }
    else
    {
        request->u.servicereg.AnonData = mDNSfalse;
    }

    // Don't try to construct "domainname t" until *after* ChopSubTypes has worked its magic
    if (!*request->u.servicereg.type_as_string ||
        !MakeDomainNameFromDNSNameString(&request->u.servicereg.type, request->u.servicereg.type_as_string))
    {
        LogMsg("ERROR: handle_regservice_request - type_as_string bad %s", request->u.servicereg.type_as_string);
        goto bad_param;
    }

    if (!name[0])
    {
        request->u.servicereg.name     = mDNSStorage.nicelabel;
        request->u.servicereg.autoname = mDNStrue;
    }
    else
    {
        // If the client is allowing AutoRename, then truncate name to legal length before converting it to a DomainLabel
        if ((flags & kDNSServiceFlagsNoAutoRename) == 0)
        {
            int newlen   = TruncateUTF8ToLength((mDNSu8 *) name, mDNSPlatformStrLen(name), MAX_DOMAIN_LABEL);
            name[newlen] = 0;
        }
        if (!MakeDomainLabelFromLiteralString(&request->u.servicereg.name, name))
        {
            LogMsg("ERROR: handle_regservice_request - name bad %s", name);
            goto bad_param;
        }
        request->u.servicereg.autoname = mDNSfalse;
    }

    if (*domain)
    {
        request->u.servicereg.default_domain = mDNSfalse;
        if (!MakeDomainNameFromDNSNameString(&d, domain))
        {
            LogMsg("ERROR: handle_regservice_request - domain bad %s", domain);
            goto bad_param;
        }
    }
    else
    {
        request->u.servicereg.default_domain = mDNStrue;
        MakeDomainNameFromDNSNameString(&d, "local.");
    }

    // We don't allow the anonymous and the regular ones to coexist
    if (!CheckForMixedRegistrations(&request->u.servicereg.type, &d, request->u.servicereg.AnonData))
    {
        goto bad_param;
    }

    if (!ConstructServiceName(&srv, &request->u.servicereg.name, &request->u.servicereg.type, &d))
    {
        LogMsg("ERROR: handle_regservice_request - Couldn't ConstructServiceName from");
        goto bad_param;
    }

    if (!MakeDomainNameFromDNSNameString(&request->u.servicereg.host, host))
    {
        LogMsg("ERROR: handle_regservice_request - host bad %s", host);
        goto bad_param;
    }
    request->u.servicereg.autorename       = (flags & kDNSServiceFlagsNoAutoRename) == 0;
    request->u.servicereg.allowremotequery = (flags & kDNSServiceFlagsAllowRemoteQuery) != 0;

    // Some clients use mDNS for lightweight copy protection, registering a pseudo-service with
    // a port number of zero. When two instances of the protected client are allowed to run on one
    // machine, we don't want to see misleading "Bogus client" messages in syslog and the console.
    if (!mDNSIPPortIsZero(request->u.servicereg.port))
    {
        int count = CountExistingRegistrations(&srv, request->u.servicereg.port);
        if (count)
            LogMsg("Client application[%d](%s) registered %d identical instances of service %##s port %u.", request->process_id,
                   request->pid_name, count + 1, srv.c, mDNSVal16(request->u.servicereg.port));
    }

    LogOperation("%3d: DNSServiceRegister(%X, %d, \"%s\", \"%s\", \"%s\", \"%s\", %u) START PID[%d](%s)", request->sd, flags,
                 interfaceIndex, name, request->u.servicereg.type_as_string, domain, host, mDNSVal16(request->u.servicereg.port),
                 request->process_id, request->pid_name);

    // We need to unconditionally set request->terminate, because even if we didn't successfully
    // start any registrations right now, subsequent configuration changes may cause successful
    // registrations to be added, and we'll need to cancel them before freeing this memory.
    // We also need to set request->terminate first, before adding additional service instances,
    // because the uds_validatelists uses the request->terminate function pointer to determine
    // what kind of request this is, and therefore what kind of list validation is required.
    request->terminate = regservice_termination_callback;

    err = register_service_instance(request, &d);

#if 0
    err = AuthorizedDomain(request, &d, AutoRegistrationDomains) ? register_service_instance(request, &d) : mStatus_NoError;
#endif
    if (!err)
    {
        if (request->u.servicereg.autoname)
            UpdateDeviceInfoRecord(&mDNSStorage);

        if (!*domain)
        {
            DNameListElem * ptr;
            // Note that we don't report errors for non-local, non-explicit domains
            for (ptr = AutoRegistrationDomains; ptr; ptr = ptr->next)
                if (!ptr->uid || SystemUID(request->uid) || request->uid == ptr->uid)
                    register_service_instance(request, &ptr->name);
        }
    }

    return (err);

bad_param:
    free(request->u.servicereg.txtdata);
    request->u.servicereg.txtdata = NULL;
    return mStatus_BadParamErr;
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceBrowse
#endif

mDNSlocal void FoundInstance(mDNS * const m, DNSQuestion * question, const ResourceRecord * const answer, QC_result AddRecord)
{
    DNSServiceFlags flags = AddRecord ? kDNSServiceFlagsAdd : 0;
    request_state * req   = question->QuestionContext;
    reply_state * rep;
    (void) m; // Unused

    if (answer->rrtype != kDNSType_PTR)
    {
        LogMsg("%3d: FoundInstance: Should not be called with rrtype %d (not a PTR record)", req->sd, answer->rrtype);
        return;
    }

    if (mDNSOpaque16IsZero(question->TargetQID) && (question->BrowseThreshold > 0) &&
        (question->CurrentAnswers >= question->BrowseThreshold))
    {
        flags |= kDNSServiceFlagsThresholdReached;
    }

    if (GenerateNTDResponse(&answer->rdata->u.name, answer->InterfaceID, req, &rep, browse_reply_op, flags, mStatus_NoError) !=
        mStatus_NoError)
    {
        if (SameDomainName(&req->u.browser.regtype, (const domainname *) "\x09_services\x07_dns-sd\x04_udp"))
        {
            // Special support to enable the DNSServiceBrowse call made by Bonjour Browser
            // Remove after Bonjour Browser is updated to use DNSServiceQueryRecord instead of DNSServiceBrowse
            GenerateBonjourBrowserResponse(&answer->rdata->u.name, answer->InterfaceID, req, &rep, browse_reply_op, flags,
                                           mStatus_NoError);
            goto bonjourbrowserhack;
        }

        LogMsg("%3d: FoundInstance: %##s PTR %##s received from network is not valid DNS-SD service pointer", req->sd,
               answer->name->c, answer->rdata->u.name.c);
        return;
    }

bonjourbrowserhack:

    LogOperation("%3d: DNSServiceBrowse(%##s, %s) RESULT %s %d: %s", req->sd, question->qname.c, DNSTypeName(question->qtype),
                 AddRecord ? "Add" : "Rmv", mDNSPlatformInterfaceIndexfromInterfaceID(m, answer->InterfaceID, mDNSfalse),
                 RRDisplayString(m, answer));

    append_reply(req, rep);
}

mDNSlocal mStatus add_domain_to_browser(request_state * info, const domainname * d)
{
    browser_t *b, *p;
    mStatus err;

    for (p = info->u.browser.browsers; p; p = p->next)
    {
        if (SameDomainName(&p->domain, d))
        {
            debugf("add_domain_to_browser %##s already in list", d->c);
            return mStatus_AlreadyRegistered;
        }
    }

    // b = mallocL("browser_t", sizeof(*b));
    b = malloc(sizeof(*b));
    if (!b)
        return mStatus_NoMemoryErr;
    AssignDomainName(&b->domain, d);
    err = mDNS_StartBrowse(&mDNSStorage, &b->q, &info->u.browser.regtype, d, info->u.browser.AnonData, info->u.browser.interface_id,
                           info->flags, info->u.browser.ForceMCast, (info->flags & kDNSServiceFlagsBackgroundTrafficClass) != 0,
                           FoundInstance, info);
    if (err)
    {
        LogMsg("mDNS_StartBrowse returned %d for type %##s domain %##s", err, info->u.browser.regtype.c, d->c);
        free(b);
    }
    else
    {
        b->next                  = info->u.browser.browsers;
        info->u.browser.browsers = b;
        LogOperation("%3d: DNSServiceBrowse(%##s) START PID[%d](%s)", info->sd, b->q.qname.c, info->process_id, info->pid_name);
        LogMcastQ(&mDNSStorage, &b->q, info, q_start);
        if (callExternalHelpers(info->u.browser.interface_id, &b->domain, info->flags))
        {
            domainname tmp;
            ConstructServiceName(&tmp, NULL, &info->u.browser.regtype, &b->domain);
            LogInfo("add_domain_to_browser: calling external_start_browsing_for_service()");
            external_start_browsing_for_service(info->u.browser.interface_id, &tmp, kDNSType_PTR, info->flags);
        }
    }
    return err;
}
#ifndef REMOVE_BROWSE_REQUEST
mDNSlocal void browse_termination_callback(request_state * info)
{
    if (info->u.browser.default_domain)
    {
        // Stop the domain enumeration queries to discover the WAB legacy browse domains
        LogInfo("%3d: DNSServiceBrowse Cancel WAB PID[%d](%s)", info->sd, info->process_id, info->pid_name);
        uDNS_StopWABQueries(&mDNSStorage, UDNS_WAB_LBROWSE_QUERY);
    }
    if (info->u.browser.AnonData)
        free((void *) info->u.browser.AnonData);
    while (info->u.browser.browsers)
    {
        browser_t * ptr = info->u.browser.browsers;

        if (callExternalHelpers(info->u.browser.interface_id, &ptr->domain, info->flags))
        {
            domainname tmp;
            ConstructServiceName(&tmp, NULL, &info->u.browser.regtype, &ptr->domain);
            LogInfo("browse_termination_callback: calling external_stop_browsing_for_service()");
            external_stop_browsing_for_service(info->u.browser.interface_id, &tmp, kDNSType_PTR, info->flags);
        }

        info->u.browser.browsers = ptr->next;
        LogOperation("%3d: DNSServiceBrowse(%##s) STOP PID[%d](%s)", info->sd, ptr->q.qname.c, info->process_id, info->pid_name);
        mDNS_StopBrowse(&mDNSStorage, &ptr->q); // no need to error-check result
        LogMcastQ(&mDNSStorage, &ptr->q, info, q_stop);
        free(ptr);
    }
}

mDNSlocal void udsserver_automatic_browse_domain_changed(const DNameListElem * const d, const mDNSBool add)
{
    request_state * request;
    debugf("udsserver_automatic_browse_domain_changed: %s default browse domain %##s", add ? "Adding" : "Removing", d->name.c);

#if APPLE_OSX_mDNSResponder
    machserver_automatic_browse_domain_changed(&d->name, add);
#endif // APPLE_OSX_mDNSResponder

    for (request = all_requests; request; request = request->next)
    {
#ifndef REMOVE_BROWSE_REQUEST
        if (request->terminate != browse_termination_callback)
            continue; // Not a browse operation
        if (!request->u.browser.default_domain)
            continue; // Not an auto-browse operation
#endif                // #ifndef REMOVE_BROWSE_REQUEST
        if (!d->uid || SystemUID(request->uid) || request->uid == d->uid)
        {
            browser_t ** ptr = &request->u.browser.browsers;
            while (*ptr && !SameDomainName(&(*ptr)->domain, &d->name))
                ptr = &(*ptr)->next;
            if (add)
            {
                // If we don't already have this domain in our list for this browse operation, add it now
                if (!*ptr)
                    add_domain_to_browser(request, &d->name);
                else
                    debugf("udsserver_automatic_browse_domain_changed %##s already in list, not re-adding", &d->name);
            }
            else
            {
                if (!*ptr)
                    LogMsg("udsserver_automatic_browse_domain_changed ERROR %##s not found", &d->name);
                else
                {
                    DNameListElem * p;
                    for (p = AutoBrowseDomains; p; p = p->next)
                        if (!p->uid || SystemUID(request->uid) || request->uid == p->uid)
                            if (SameDomainName(&d->name, &p->name))
                                break;
                    if (p)
                        debugf("udsserver_automatic_browse_domain_changed %##s still in list, not removing", &d->name);
                    else
                    {
                        browser_t * rem = *ptr;
                        *ptr            = (*ptr)->next;
                        mDNS_StopQueryWithRemoves(&mDNSStorage, &rem->q);
                        free(rem);
                    }
                }
            }
        }
    }
}
#endif // #ifndef REMOVE_BROWSE_REQUEST
mDNSlocal void FreeARElemCallback(mDNS * const m, AuthRecord * const rr, mStatus result)
{
    (void) m; // unused
    if (result == mStatus_MemFree)
    {
        // On shutdown, mDNS_Close automatically deregisters all records
        // Since in this case no one has called DeregisterLocalOnlyDomainEnumPTR to cut the record
        // from the LocalDomainEnumRecords list, we do this here before we free the memory.
        // (This should actually no longer be necessary, now that we do the proper cleanup in
        // udsserver_exit. To confirm this, we'll log an error message if we do find a record that
        // hasn't been cut from the list yet. If these messages don't appear, we can delete this code.)
        ARListElem ** ptr = &LocalDomainEnumRecords;
        while (*ptr && &(*ptr)->ar != rr)
            ptr = &(*ptr)->next;
        if (*ptr)
        {
            *ptr = (*ptr)->next;
            LogMsg("FreeARElemCallback: Have to cut %s", ARDisplayString(m, rr));
        }
        mDNSPlatformMemFree(rr->RecordContext);
    }
}

// RegisterLocalOnlyDomainEnumPTR and DeregisterLocalOnlyDomainEnumPTR largely duplicate code in
// "FoundDomain" in uDNS.c for creating and destroying these special mDNSInterface_LocalOnly records.
// We may want to turn the common code into a subroutine.

mDNSlocal void RegisterLocalOnlyDomainEnumPTR(mDNS * m, const domainname * d, int type)
{
    // allocate/register legacy and non-legacy _browse PTR record
    mStatus err;
    ARListElem * ptr = mDNSPlatformMemAllocate(sizeof(*ptr));

    debugf("Incrementing %s refcount for %##s",
           (type == mDNS_DomainTypeBrowse)                ? "browse domain   "
               : (type == mDNS_DomainTypeRegistration)    ? "registration dom"
               : (type == mDNS_DomainTypeBrowseAutomatic) ? "automatic browse"
                                                          : "?",
           d->c);

    mDNS_SetupResourceRecord(&ptr->ar, mDNSNULL, mDNSInterface_LocalOnly, kDNSType_PTR, 7200, kDNSRecordTypeShared,
                             AuthRecordLocalOnly, FreeARElemCallback, ptr);
    MakeDomainNameFromDNSNameString(&ptr->ar.namestorage, mDNS_DomainTypeNames[type]);
    AppendDNSNameString(&ptr->ar.namestorage, "local");
    AssignDomainName(&ptr->ar.resrec.rdata->u.name, d);
    err = mDNS_Register(m, &ptr->ar);
    if (err)
    {
        LogMsg("SetSCPrefsBrowseDomain: mDNS_Register returned error %d", err);
        mDNSPlatformMemFree(ptr);
    }
    else
    {
        ptr->next              = LocalDomainEnumRecords;
        LocalDomainEnumRecords = ptr;
    }
}

mDNSlocal void DeregisterLocalOnlyDomainEnumPTR(mDNS * m, const domainname * d, int type)
{
    ARListElem ** ptr = &LocalDomainEnumRecords;
    domainname lhs; // left-hand side of PTR, for comparison

    debugf("Decrementing %s refcount for %##s",
           (type == mDNS_DomainTypeBrowse)                ? "browse domain   "
               : (type == mDNS_DomainTypeRegistration)    ? "registration dom"
               : (type == mDNS_DomainTypeBrowseAutomatic) ? "automatic browse"
                                                          : "?",
           d->c);

    MakeDomainNameFromDNSNameString(&lhs, mDNS_DomainTypeNames[type]);
    AppendDNSNameString(&lhs, "local");

    while (*ptr)
    {
        if (SameDomainName(&(*ptr)->ar.resrec.rdata->u.name, d) && SameDomainName((*ptr)->ar.resrec.name, &lhs))
        {
            ARListElem * rem = *ptr;
            *ptr             = (*ptr)->next;
            mDNS_Deregister(m, &rem->ar);
            return;
        }
        else
            ptr = &(*ptr)->next;
    }
}
#ifndef REMOVE_BROWSE_REQUEST
mDNSlocal void AddAutoBrowseDomain(const mDNSu32 uid, const domainname * const name)
{
    DNameListElem * new = mDNSPlatformMemAllocate(sizeof(DNameListElem));
    if (!new)
    {
        LogMsg("ERROR: malloc");
        return;
    }
    AssignDomainName(&new->name, name);
    new->uid          = uid;
    new->next         = AutoBrowseDomains;
    AutoBrowseDomains = new;
    udsserver_automatic_browse_domain_changed(new, mDNStrue);
}

mDNSlocal void RmvAutoBrowseDomain(const mDNSu32 uid, const domainname * const name)
{
    DNameListElem ** p = &AutoBrowseDomains;
    while (*p && (!SameDomainName(&(*p)->name, name) || (*p)->uid != uid))
        p = &(*p)->next;
    if (!*p)
        LogMsg("RmvAutoBrowseDomain: Got remove event for domain %##s not in list", name->c);
    else
    {
        DNameListElem * ptr = *p;
        *p                  = ptr->next;
        udsserver_automatic_browse_domain_changed(ptr, mDNSfalse);
        mDNSPlatformMemFree(ptr);
    }
}

mDNSlocal void SetPrefsBrowseDomains(mDNS * m, DNameListElem * browseDomains, mDNSBool add)
{
    DNameListElem * d;
    for (d = browseDomains; d; d = d->next)
    {
        if (add)
        {
            RegisterLocalOnlyDomainEnumPTR(m, &d->name, mDNS_DomainTypeBrowse);
            AddAutoBrowseDomain(d->uid, &d->name);
        }
        else
        {
            DeregisterLocalOnlyDomainEnumPTR(m, &d->name, mDNS_DomainTypeBrowse);
            RmvAutoBrowseDomain(d->uid, &d->name);
        }
    }
}
#endif //#ifndef REMOVE_BROWSE_REQUEST

#if APPLE_OSX_mDNSResponder

mDNSlocal void UpdateDeviceInfoRecord(mDNS * const m)
{
    int num_autoname = 0;
    request_state * req;
    for (req = all_requests; req; req = req->next)
        if (req->terminate == regservice_termination_callback && req->u.servicereg.autoname)
            num_autoname++;

    // If DeviceInfo record is currently registered, see if we need to deregister it
    if (m->DeviceInfo.resrec.RecordType != kDNSRecordTypeUnregistered)
        if (num_autoname == 0 || !SameDomainLabelCS(m->DeviceInfo.resrec.name->c, m->nicelabel.c))
        {
            LogOperation("UpdateDeviceInfoRecord Deregister %##s", m->DeviceInfo.resrec.name);
            mDNS_Deregister(m, &m->DeviceInfo);
        }

    // If DeviceInfo record is not currently registered, see if we need to register it
    if (m->DeviceInfo.resrec.RecordType == kDNSRecordTypeUnregistered)
        if (num_autoname > 0)
        {
            mDNS_SetupResourceRecord(&m->DeviceInfo, mDNSNULL, mDNSNULL, kDNSType_TXT, kStandardTTL, kDNSRecordTypeAdvisory,
                                     AuthRecordAny, mDNSNULL, mDNSNULL);
            ConstructServiceName(&m->DeviceInfo.namestorage, &m->nicelabel, &DeviceInfoName, &localdomain);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
            m->DeviceInfo.resrec.rdlength = initializeDeviceInfoTXT(m, m->DeviceInfo.resrec.rdata->u.data);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
            LogOperation("UpdateDeviceInfoRecord   Register %##s", m->DeviceInfo.resrec.name);
            mDNS_Register(m, &m->DeviceInfo);
        }
}
#else  // APPLE_OSX_mDNSResponder
mDNSlocal void UpdateDeviceInfoRecord(mDNS * const m)
{
    (void) m; // unused
}
#endif // APPLE_OSX_mDNSResponder

mDNSexport void udsserver_handle_configchange(mDNS * const m)
{
    request_state * req;
    service_instance * ptr;
    DNameListElem * RegDomains    = NULL;
    DNameListElem * BrowseDomains = NULL;
    DNameListElem * p;

    UpdateDeviceInfoRecord(m);

    // For autoname services, see if the default service name has changed, necessitating an automatic update
    for (req = all_requests; req; req = req->next)
        if (req->terminate == regservice_termination_callback)
            if (req->u.servicereg.autoname && !SameDomainLabelCS(req->u.servicereg.name.c, m->nicelabel.c))
            {
                req->u.servicereg.name = m->nicelabel;
                for (ptr = req->u.servicereg.instances; ptr; ptr = ptr->next)
                {
                    ptr->renameonmemfree = 1;
                    if (ptr->clientnotified)
                        SendServiceRemovalNotification(&ptr->srs);
                    LogInfo("udsserver_handle_configchange: Calling deregister for Service %##s", ptr->srs.RR_PTR.resrec.name->c);
                    if (mDNS_DeregisterService_drt(m, &ptr->srs, mDNS_Dereg_rapid))
                        regservice_callback(m, &ptr->srs,
                                            mStatus_MemFree); // If service deregistered already, we can re-register immediately
                }
            }

    // Let the platform layer get the current DNS information
    mDNS_Lock(m);
    mDNSPlatformSetDNSConfig(m, mDNSfalse, mDNSfalse, mDNSNULL, &RegDomains, &BrowseDomains, mDNSfalse);
    mDNS_Unlock(m);

#ifndef REMOVE_BROWSE_REQUEST
    // Any automatic registration domains are also implicitly automatic browsing domains
    if (RegDomains)
        SetPrefsBrowseDomains(m, RegDomains, mDNStrue); // Add the new list first
    if (AutoRegistrationDomains)
        SetPrefsBrowseDomains(m, AutoRegistrationDomains, mDNSfalse); // Then clear the old list
#endif

    // Add any new domains not already in our AutoRegistrationDomains list
    for (p = RegDomains; p; p = p->next)
    {
        DNameListElem ** pp = &AutoRegistrationDomains;
        while (*pp && ((*pp)->uid != p->uid || !SameDomainName(&(*pp)->name, &p->name)))
            pp = &(*pp)->next;
        if (!*pp) // If not found in our existing list, this is a new default registration domain
        {
            RegisterLocalOnlyDomainEnumPTR(m, &p->name, mDNS_DomainTypeRegistration);
            udsserver_default_reg_domain_changed(p, mDNStrue);
        }
        else // else found same domainname in both old and new lists, so no change, just delete old copy
        {
            DNameListElem * del = *pp;
            *pp                 = (*pp)->next;
            mDNSPlatformMemFree(del);
        }
    }

    // Delete any domains in our old AutoRegistrationDomains list that are now gone
    while (AutoRegistrationDomains)
    {
        DNameListElem * del     = AutoRegistrationDomains;
        AutoRegistrationDomains = AutoRegistrationDomains->next; // Cut record from list FIRST,
        DeregisterLocalOnlyDomainEnumPTR(m, &del->name, mDNS_DomainTypeRegistration);
        udsserver_default_reg_domain_changed(del, mDNSfalse); // before calling udsserver_default_reg_domain_changed()
        mDNSPlatformMemFree(del);
    }

    // Now we have our new updated automatic registration domain list
    AutoRegistrationDomains = RegDomains;

#ifndef REMOVE_BROWSE_REQUEST
    // Add new browse domains to internal list
    if (BrowseDomains)
        SetPrefsBrowseDomains(m, BrowseDomains, mDNStrue);

    // Remove old browse domains from internal list
    if (SCPrefBrowseDomains)
    {
        SetPrefsBrowseDomains(m, SCPrefBrowseDomains, mDNSfalse);
        while (SCPrefBrowseDomains)
        {
            DNameListElem * fptr = SCPrefBrowseDomains;
            SCPrefBrowseDomains  = SCPrefBrowseDomains->next;
            mDNSPlatformMemFree(fptr);
        }
    }
#endif //#ifndef REMOVE_BROWSE_REQUEST

    // Replace the old browse domains array with the new array
    SCPrefBrowseDomains = BrowseDomains;
}
#ifndef REMOVE_BROWSE_REQUEST
mDNSlocal void AutomaticBrowseDomainChange(mDNS * const m, DNSQuestion * q, const ResourceRecord * const answer,
                                           QC_result AddRecord)
{
    (void) m; // unused;
    (void) q; // unused

    LogOperation("AutomaticBrowseDomainChange: %s automatic browse domain %##s", AddRecord ? "Adding" : "Removing",
                 answer->rdata->u.name.c);

    if (AddRecord)
        AddAutoBrowseDomain(0, &answer->rdata->u.name);
    else
        RmvAutoBrowseDomain(0, &answer->rdata->u.name);
}

mDNSlocal mStatus handle_browse_request(request_state * request)
{
    char regtype[MAX_ESCAPED_DOMAIN_NAME], domain[MAX_ESCAPED_DOMAIN_NAME];
    domainname typedn, d, temp;
    mDNSs32 NumSubTypes;
    char * AnonData = mDNSNULL;
    mStatus err     = mStatus_NoError;
    int AnonDataLen;

    DNSServiceFlags flags       = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex      = get_uint32(&request->msgptr, request->msgend);
    mDNSInterfaceID InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    if (interfaceIndex && !InterfaceID)
        return (mStatus_BadParamErr);

    if (get_string(&request->msgptr, request->msgend, regtype, MAX_ESCAPED_DOMAIN_NAME) < 0 ||
        get_string(&request->msgptr, request->msgend, domain, MAX_ESCAPED_DOMAIN_NAME) < 0)
        return (mStatus_BadParamErr);

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceBrowse(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    request->flags = flags;
    typedn.c[0]    = 0;
    NumSubTypes    = ChopSubTypes(regtype, &AnonData); // Note: Modifies regtype string to remove trailing subtypes
    if (NumSubTypes < 0 || NumSubTypes > 1)
        return (mStatus_BadParamErr);
    AnonDataLen = 0;
    if (AnonData)
    {
        AnonDataLen = strlen(AnonData);
        if (AnonDataLen > MAX_ANONYMOUS_DATA)
        {
            LogMsg("handle_browse_request: AnonDataLen %d", AnonDataLen);
            return (mStatus_BadParamErr);
        }
        // Account for the null byte
        AnonDataLen += 1;
    }
    if (NumSubTypes == 1)
    {
        if (!AppendDNSNameString(&typedn, regtype + strlen(regtype) + 1 + AnonDataLen))
            return (mStatus_BadParamErr);
    }

    if (!regtype[0] || !AppendDNSNameString(&typedn, regtype))
        return (mStatus_BadParamErr);

    if (!MakeDomainNameFromDNSNameString(&temp, regtype))
        return (mStatus_BadParamErr);
    // For over-long service types, we only allow domain "local"
    if (temp.c[0] > 15 && domain[0] == 0)
        mDNSPlatformStrCopy(domain, "local.");

    // Set up browser info
    request->u.browser.ForceMCast   = (flags & kDNSServiceFlagsForceMulticast) != 0;
    request->u.browser.interface_id = InterfaceID;
    AssignDomainName(&request->u.browser.regtype, &typedn);
    request->u.browser.default_domain = !domain[0];
    request->u.browser.browsers       = NULL;

    LogOperation("%3d: DNSServiceBrowse(%X, %d, \"%##s\", \"%s\") START PID[%d](%s)", request->sd, request->flags, interfaceIndex,
                 request->u.browser.regtype.c, domain, request->process_id, request->pid_name);

    if (request->u.browser.default_domain)
    {
        // Start the domain enumeration queries to discover the WAB browse domains
        LogInfo("%3d: DNSServiceBrowse Start WAB PID[%d](%s)", request->sd, request->process_id, request->pid_name);
        uDNS_StartWABQueries(&mDNSStorage, UDNS_WAB_LBROWSE_QUERY);
    }
    request->u.browser.AnonData = mDNSNULL;
    if (AnonData)
    {
        int len = strlen(AnonData) + 1;
        // request->u.browser.AnonData = mallocL("Anonymous", len);
        request->u.browser.AnonData = malloc(len);
        if (!request->u.browser.AnonData)
            return mStatus_NoMemoryErr;
        else
            mDNSPlatformMemCopy((void *) request->u.browser.AnonData, AnonData, len);
    }
    // We need to unconditionally set request->terminate, because even if we didn't successfully
    // start any browses right now, subsequent configuration changes may cause successful
    // browses to be added, and we'll need to cancel them before freeing this memory.
    request->terminate = browse_termination_callback;

    if (domain[0])
    {
        if (!MakeDomainNameFromDNSNameString(&d, domain))
            return (mStatus_BadParamErr);
        err = add_domain_to_browser(request, &d);
    }
    else
    {
        DNameListElem * sdom;
        for (sdom = AutoBrowseDomains; sdom; sdom = sdom->next)
            if (!sdom->uid || SystemUID(request->uid) || request->uid == sdom->uid)
            {
                err = add_domain_to_browser(request, &sdom->name);
                if (err)
                {
                    if (SameDomainName(&sdom->name, &localdomain))
                        break;
                    else
                        err = mStatus_NoError; // suppress errors for non-local "default" domains
                }
            }
    }

    return (err);
}
#endif //#ifndef REMOVE_BROWSE_REQUEST
// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceResolve
#endif

mDNSlocal void resolve_result_callback(mDNS * const m, DNSQuestion * question, const ResourceRecord * const answer,
                                       QC_result AddRecord)
{
    size_t len = 0;
    char fullname[MAX_ESCAPED_DOMAIN_NAME], target[MAX_ESCAPED_DOMAIN_NAME];
    char * data;
    reply_state * rep;
    request_state * req = question->QuestionContext;
    (void) m; // Unused

    LogOperation("%3d: DNSServiceResolve(%##s) %s %s", req->sd, question->qname.c, AddRecord ? "ADD" : "RMV",
                 RRDisplayString(m, answer));

    if (!AddRecord)
    {
        if (req->u.resolve.srv == answer)
            req->u.resolve.srv = mDNSNULL;
        if (req->u.resolve.txt == answer)
            req->u.resolve.txt = mDNSNULL;
        return;
    }

    if (answer->rrtype == kDNSType_SRV)
        req->u.resolve.srv = answer;
    if (answer->rrtype == kDNSType_TXT)
        req->u.resolve.txt = answer;

    if (!req->u.resolve.txt || !req->u.resolve.srv)
        return; // only deliver result to client if we have both answers

    ConvertDomainNameToCString(answer->name, fullname);
    ConvertDomainNameToCString(&req->u.resolve.srv->rdata->u.srv.target, target);

    // calculate reply length
    len += sizeof(DNSServiceFlags);
    len += sizeof(mDNSu32); // interface index
    len += sizeof(DNSServiceErrorType);
    len += strlen(fullname) + 1;
    len += strlen(target) + 1;
    len += 2 * sizeof(mDNSu16); // port, txtLen
    len += req->u.resolve.txt->rdlength;

    // allocate/init reply header
    rep              = create_reply(resolve_reply_op, len, req);
    rep->rhdr->flags = dnssd_htonl(0);
    rep->rhdr->ifi   = dnssd_htonl(mDNSPlatformInterfaceIndexfromInterfaceID(m, answer->InterfaceID, mDNSfalse));
    rep->rhdr->error = dnssd_htonl(kDNSServiceErr_NoError);

    data = (char *) &rep->rhdr[1];

    // write reply data to message
    put_string(fullname, &data);
    put_string(target, &data);
    *data++ = req->u.resolve.srv->rdata->u.srv.port.b[0];
    *data++ = req->u.resolve.srv->rdata->u.srv.port.b[1];
    put_uint16(req->u.resolve.txt->rdlength, &data);
    put_rdata(req->u.resolve.txt->rdlength, req->u.resolve.txt->rdata->u.data, &data);

    LogOperation("%3d: DNSServiceResolve(%s) RESULT   %s:%d", req->sd, fullname, target,
                 mDNSVal16(req->u.resolve.srv->rdata->u.srv.port));
    append_reply(req, rep);
}

mDNSlocal void resolve_termination_callback(request_state * request)
{
    LogOperation("%3d: DNSServiceResolve(%##s) STOP PID[%d](%s)", request->sd, request->u.resolve.qtxt.qname.c, request->process_id,
                 request->pid_name);
    mDNS_StopQuery(&mDNSStorage, &request->u.resolve.qtxt);
    mDNS_StopQuery(&mDNSStorage, &request->u.resolve.qsrv);
    LogMcastQ(&mDNSStorage, &request->u.resolve.qsrv, request, q_stop);
    if (request->u.resolve.external_advertise)
        external_stop_resolving_service(request->u.resolve.qsrv.InterfaceID, &request->u.resolve.qsrv.qname, request->flags);
}

mDNSlocal mStatus handle_resolve_request(request_state * request)
{
    // char name[256], regtype[MAX_ESCAPED_DOMAIN_NAME], domain[MAX_ESCAPED_DOMAIN_NAME];
    char name[64], regtype[MAX_ESCAPED_DOMAIN_NAME], domain[MAX_ESCAPED_DOMAIN_NAME];
    domainname fqdn;
    mStatus err;

    // extract the data from the message
    DNSServiceFlags flags  = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex = get_uint32(&request->msgptr, request->msgend);
    mDNSInterfaceID InterfaceID;

    // Map kDNSServiceInterfaceIndexP2P to kDNSServiceInterfaceIndexAny with the kDNSServiceFlagsIncludeP2P
    // flag set so that the resolve will run over P2P interfaces that are not yet created.
    if (interfaceIndex == kDNSServiceInterfaceIndexP2P)
    {
        LogOperation("handle_resolve_request: mapping kDNSServiceInterfaceIndexP2P to kDNSServiceInterfaceIndexAny + "
                     "kDNSServiceFlagsIncludeP2P");
        flags |= kDNSServiceFlagsIncludeP2P;
        interfaceIndex = kDNSServiceInterfaceIndexAny;
    }

    InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    if (interfaceIndex && !InterfaceID)
    {
        LogMsg("ERROR: handle_resolve_request bad interfaceIndex %d", interfaceIndex);
        return (mStatus_BadParamErr);
    }

    // if (get_string(&request->msgptr, request->msgend, name, 256) < 0 ||
    if (get_string(&request->msgptr, request->msgend, name, 64) < 0 ||
        get_string(&request->msgptr, request->msgend, regtype, MAX_ESCAPED_DOMAIN_NAME) < 0 ||
        get_string(&request->msgptr, request->msgend, domain, MAX_ESCAPED_DOMAIN_NAME) < 0)
    {
        LogMsg("ERROR: handle_resolve_request - Couldn't read name/regtype/domain");
        return (mStatus_BadParamErr);
    }

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceResolve(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    if (build_domainname_from_strings(&fqdn, name, regtype, domain) < 0)
    {
        LogMsg("ERROR: handle_resolve_request bad %s %s %s", name, regtype, domain);
        return (mStatus_BadParamErr);
    }

    mDNSPlatformMemZero(&request->u.resolve, sizeof(request->u.resolve));

    request->flags = flags;

    // format questions
    request->u.resolve.qsrv.InterfaceID = InterfaceID;
    request->u.resolve.qsrv.flags       = flags;
    request->u.resolve.qsrv.Target      = zeroAddr;
    AssignDomainName(&request->u.resolve.qsrv.qname, &fqdn);
    request->u.resolve.qsrv.qtype                  = kDNSType_SRV;
    request->u.resolve.qsrv.qclass                 = kDNSClass_IN;
    request->u.resolve.qsrv.LongLived              = (flags & kDNSServiceFlagsLongLivedQuery) != 0;
    request->u.resolve.qsrv.ExpectUnique           = mDNStrue;
    request->u.resolve.qsrv.ForceMCast             = (flags & kDNSServiceFlagsForceMulticast) != 0;
    request->u.resolve.qsrv.ReturnIntermed         = (flags & kDNSServiceFlagsReturnIntermediates) != 0;
    request->u.resolve.qsrv.SuppressUnusable       = mDNSfalse;
    request->u.resolve.qsrv.SearchListIndex        = 0;
    request->u.resolve.qsrv.AppendSearchDomains    = 0;
    request->u.resolve.qsrv.RetryWithSearchDomains = mDNSfalse;
    request->u.resolve.qsrv.TimeoutQuestion        = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    request->u.resolve.qsrv.WakeOnResolve = (flags & kDNSServiceFlagsWakeOnResolve) != 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    request->u.resolve.qsrv.UseBackgroundTrafficClass = (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    request->u.resolve.qsrv.ValidationRequired        = 0;
    request->u.resolve.qsrv.ValidatingResponse        = 0;
    request->u.resolve.qsrv.ProxyQuestion             = 0;
    request->u.resolve.qsrv.qnameOrig                 = mDNSNULL;
    request->u.resolve.qsrv.AnonInfo                  = mDNSNULL;
    request->u.resolve.qsrv.pid                       = request->process_id;
    request->u.resolve.qsrv.QuestionCallback          = resolve_result_callback;
    request->u.resolve.qsrv.QuestionContext           = request;

    request->u.resolve.qtxt.InterfaceID = InterfaceID;
    request->u.resolve.qtxt.flags       = flags;
    request->u.resolve.qtxt.Target      = zeroAddr;
    AssignDomainName(&request->u.resolve.qtxt.qname, &fqdn);
    request->u.resolve.qtxt.qtype                  = kDNSType_TXT;
    request->u.resolve.qtxt.qclass                 = kDNSClass_IN;
    request->u.resolve.qtxt.LongLived              = (flags & kDNSServiceFlagsLongLivedQuery) != 0;
    request->u.resolve.qtxt.ExpectUnique           = mDNStrue;
    request->u.resolve.qtxt.ForceMCast             = (flags & kDNSServiceFlagsForceMulticast) != 0;
    request->u.resolve.qtxt.ReturnIntermed         = (flags & kDNSServiceFlagsReturnIntermediates) != 0;
    request->u.resolve.qtxt.SuppressUnusable       = mDNSfalse;
    request->u.resolve.qtxt.SearchListIndex        = 0;
    request->u.resolve.qtxt.AppendSearchDomains    = 0;
    request->u.resolve.qtxt.RetryWithSearchDomains = mDNSfalse;
    request->u.resolve.qtxt.TimeoutQuestion        = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    request->u.resolve.qtxt.WakeOnResolve = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    request->u.resolve.qtxt.UseBackgroundTrafficClass = (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    request->u.resolve.qtxt.ValidationRequired        = 0;
    request->u.resolve.qtxt.ValidatingResponse        = 0;
    request->u.resolve.qtxt.ProxyQuestion             = 0;
    request->u.resolve.qtxt.qnameOrig                 = mDNSNULL;
    request->u.resolve.qtxt.AnonInfo                  = mDNSNULL;
    request->u.resolve.qtxt.pid                       = request->process_id;
    request->u.resolve.qtxt.QuestionCallback          = resolve_result_callback;
    request->u.resolve.qtxt.QuestionContext           = request;

    request->u.resolve.ReportTime = NonZeroTime(mDNS_TimeNow(&mDNSStorage) + 130 * mDNSPlatformOneSecond);

    request->u.resolve.external_advertise = mDNSfalse;

#if 0
    if (!AuthorizedDomain(request, &fqdn, AutoBrowseDomains)) return(mStatus_NoError);
#endif

    // ask the questions
    LogOperation("%3d: DNSServiceResolve(%X %d %##s) START PID[%d](%s)", request->sd, flags, interfaceIndex,
                 request->u.resolve.qsrv.qname.c, request->process_id, request->pid_name);
    err = mDNS_StartQuery(&mDNSStorage, &request->u.resolve.qsrv);

    if (!err)
    {
        err = mDNS_StartQuery(&mDNSStorage, &request->u.resolve.qtxt);
        if (err)
        {
            mDNS_StopQuery(&mDNSStorage, &request->u.resolve.qsrv);
        }
        else
        {
            request->terminate = resolve_termination_callback;
            LogMcastQ(&mDNSStorage, &request->u.resolve.qsrv, request, q_start);
            if (callExternalHelpers(InterfaceID, &fqdn, flags))
            {
                request->u.resolve.external_advertise = mDNStrue;
                LogInfo("handle_resolve_request: calling external_start_resolving_service()");
                external_start_resolving_service(InterfaceID, &fqdn, flags);
            }
        }
    }

    return (err);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceQueryRecord
#endif

// mDNS operation functions. Each operation has 3 associated functions - a request handler that parses
// the client's request and makes the appropriate mDNSCore call, a result handler (passed as a callback
// to the mDNSCore routine) that sends results back to the client, and a termination routine that aborts
// the mDNSCore operation if the client dies or closes its socket.

// Returns -1 to tell the caller that it should not try to reissue the query anymore
// Returns 1 on successfully appending a search domain and the caller should reissue the new query
// Returns 0 when there are no more search domains and the caller should reissue the query
mDNSlocal int AppendNewSearchDomain(mDNS * const m, DNSQuestion * question)
{
    domainname * sd;
    mStatus err;

    // Sanity check: The caller already checks this. We use -1 to indicate that we have searched all
    // the domains and should try the single label query directly on the wire.
    if (question->SearchListIndex == -1)
    {
        LogMsg("AppendNewSearchDomain: question %##s (%s) SearchListIndex is -1", question->qname.c, DNSTypeName(question->qtype));
        return -1;
    }

    if (!question->AppendSearchDomains)
    {
        LogMsg("AppendNewSearchDomain: question %##s (%s) AppendSearchDoamins is 0", question->qname.c,
               DNSTypeName(question->qtype));
        return -1;
    }

    // Save the original name, before we modify them below.
    if (!question->qnameOrig)
    {
        // question->qnameOrig =  mallocL("AppendNewSearchDomain", sizeof(domainname));
        question->qnameOrig = malloc(sizeof(domainname));
        if (!question->qnameOrig)
        {
            LogMsg("AppendNewSearchDomain: ERROR!!  malloc failure");
            return -1;
        }
        question->qnameOrig->c[0] = 0;
        AssignDomainName(question->qnameOrig, &question->qname);
        LogInfo("AppendSearchDomain: qnameOrig %##s", question->qnameOrig->c);
    }

    sd = uDNS_GetNextSearchDomain(m, question->InterfaceID, &question->SearchListIndex, !question->AppendLocalSearchDomains);
    // We use -1 to indicate that we have searched all the domains and should try the single label
    // query directly on the wire. uDNS_GetNextSearchDomain should never return a negative value
    if (question->SearchListIndex == -1)
    {
        LogMsg("AppendNewSearchDomain: ERROR!! uDNS_GetNextSearchDomain returned -1");
        return -1;
    }

    // Not a common case. Perhaps, we should try the next search domain if it exceeds ?
    if (sd && (DomainNameLength(question->qnameOrig) + DomainNameLength(sd)) > MAX_DOMAIN_NAME)
    {
        LogMsg("AppendNewSearchDomain: ERROR!! exceeding max domain length for %##s (%s) SearchDomain %##s length %d, Question "
               "name length %d",
               question->qnameOrig->c, DNSTypeName(question->qtype), sd->c, DomainNameLength(question->qnameOrig),
               DomainNameLength(sd));
        return -1;
    }

    // if there are no more search domains and we have already tried this question
    // without appending search domains, then we are done.
    if (!sd && !ApplySearchDomainsFirst(question))
    {
        LogInfo("AppnedNewSearchDomain: No more search domains for question with name %##s (%s), not trying anymore",
                question->qname.c, DNSTypeName(question->qtype));
        return -1;
    }

    // Stop the question before changing the name as negative cache entries could be pointing at this question.
    // Even if we don't change the question in the case of returning 0, the caller is going to restart the
    // question.
    err = mDNS_StopQuery(&mDNSStorage, question);
    if (err)
    {
        LogMsg("AppendNewSearchDomain: ERROR!! %##s %s mDNS_StopQuery: %d, while retrying with search domains", question->qname.c,
               DNSTypeName(question->qtype), (int) err);
    }

    AssignDomainName(&question->qname, question->qnameOrig);
    if (sd)
    {
        AppendDomainName(&question->qname, sd);
        LogInfo("AppnedNewSearchDomain: Returning question with name %##s, SearchListIndex %d", question->qname.c,
                question->SearchListIndex);
        return 1;
    }

    // Try the question as single label
    LogInfo("AppnedNewSearchDomain: No more search domains for question with name %##s (%s), trying one last time",
            question->qname.c, DNSTypeName(question->qtype));
    return 0;
}

#if APPLE_OSX_mDNSResponder

mDNSlocal mDNSBool DomainInSearchList(const domainname * domain, mDNSBool excludeLocal)
{
    const SearchListElem * s;
    int qcount, scount;

    qcount = CountLabels(domain);
    for (s = SearchList; s; s = s->next)
    {
        if (excludeLocal && SameDomainName(&s->domain, &localdomain))
            continue;
        scount = CountLabels(&s->domain);
        if (qcount >= scount)
        {
            // Note: When qcount == scount, we do a complete match of the domain
            // which is expected by the callers.
            const domainname * d = SkipLeadingLabels(domain, (qcount - scount));
            if (SameDomainName(&s->domain, d))
            {
                return mDNStrue;
            }
        }
    }
    return mDNSfalse;
}

// The caller already checks that this is a dotlocal question.
mDNSlocal mDNSBool ShouldDeliverNegativeResponse(mDNS * const m, DNSQuestion * question)
{
    mDNSu16 qtype;

    // If the question matches the search domain exactly or the search domain is a
    // subdomain of the question, it is most likely a valid unicast domain and hence
    // don't suppress negative responses.
    //
    // If the user has configured ".local" as a search domain, we don't want
    // to deliver a negative response for names ending in ".local" as that would
    // prevent bonjour discovery. Passing mDNStrue for the last argument excludes
    // ".local" search domains.
    if (DomainInSearchList(&question->qname, mDNStrue))
    {
        LogOperation("ShouldDeliverNegativeResponse: Question %##s (%s) in SearchList", question->qname.c,
                     DNSTypeName(question->qtype));
        return mDNStrue;
    }

    // Deliver negative response for A/AAAA if there was a positive response for AAAA/A respectively.
    if (question->qtype != kDNSType_A && question->qtype != kDNSType_AAAA)
    {
        LogOperation(
            "ShouldDeliverNegativeResponse: Question %##s (%s) not answering local question with negative unicast response",
            question->qname.c, DNSTypeName(question->qtype));
        return mDNSfalse;
    }
    qtype = (question->qtype == kDNSType_A ? kDNSType_AAAA : kDNSType_A);
    if (!mDNS_CheckForCacheRecord(m, question, qtype))
    {
        LogOperation("ShouldDeliverNegativeResponse:Question %##s (%s) not answering local question with negative unicast response"
                     " (can't find positive record)",
                     question->qname.c, DNSTypeName(question->qtype));
        return mDNSfalse;
    }
    LogOperation(
        "ShouldDeliverNegativeResponse:Question %##s (%s) answering local with negative unicast response (found positive record)",
        question->qname.c, DNSTypeName(question->qtype));
    return mDNStrue;
}

// Workaround for networks using Microsoft Active Directory using "local" as a private internal
// top-level domain
mDNSlocal mStatus SendAdditionalQuery(DNSQuestion * q, request_state * request, mStatus err)
{
#ifndef UNICAST_DISABLED
    extern domainname ActiveDirectoryPrimaryDomain;
    DNSQuestion ** question2;
#define VALID_MSAD_SRV_TRANSPORT(T)                                                                                                \
    (SameDomainLabel((T)->c, (const mDNSu8 *) "\x4_tcp") || SameDomainLabel((T)->c, (const mDNSu8 *) "\x4_udp"))
#define VALID_MSAD_SRV(Q) ((Q)->qtype == kDNSType_SRV && VALID_MSAD_SRV_TRANSPORT(SecondLabel(&(Q)->qname)))

    question2 = mDNSNULL;
    if (request->hdr.op == query_request)
        question2 = &request->u.queryrecord.q2;
    else if (request->hdr.op == addrinfo_request)
    {
        if (q->qtype == kDNSType_A)
            question2 = &request->u.addrinfo.q42;
        else if (q->qtype == kDNSType_AAAA)
            question2 = &request->u.addrinfo.q62;
    }
    if (!question2)
    {
        LogMsg("SendAdditionalQuery: question2 NULL for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        return mStatus_BadParamErr;
    }

    // Sanity check: If we already sent an additonal query, we don't need to send one more.
    //
    // 1. When the application calls DNSServiceQueryRecord or DNSServiceGetAddrInfo with a .local name, this function
    // is called to see whether a unicast query should be sent or not.
    //
    // 2. As a result of appending search domains, the question may be end up with a .local suffix even though it
    // was not a .local name to start with. In that case, queryrecord_result_callback calls this function to
    // send the additional query.
    //
    // Thus, it should not be called more than once.
    if (*question2)
    {
        LogInfo("SendAdditionalQuery: question2 already sent for %##s (%s), no more q2", q->qname.c, DNSTypeName(q->qtype));
        return err;
    }

    if (!q->ForceMCast && SameDomainLabel(LastLabel(&q->qname), (const mDNSu8 *) &localdomain))
        if (q->qtype == kDNSType_A || q->qtype == kDNSType_AAAA || VALID_MSAD_SRV(q))
        {
            DNSQuestion * q2;
            int labels = CountLabels(&q->qname);
            // q2 = mallocL("DNSQuestion", sizeof(DNSQuestion));
            q2 = malloc(sizeof(DNSQuestion));
            if (!q2)
                FatalError("ERROR: SendAdditionalQuery malloc");
            *question2       = q2;
            *q2              = *q;
            q2->InterfaceID  = mDNSInterface_Unicast;
            q2->ExpectUnique = mDNStrue;
            // Always set the QuestionContext to indicate that this question should be stopped
            // before freeing. Don't rely on "q".
            q2->QuestionContext = request;
            // If the query starts as a single label e.g., somehost, and we have search domains with .local,
            // queryrecord_result_callback calls this function when .local is appended to "somehost".
            // At that time, the name in "q" is pointing at somehost.local and its qnameOrig pointing at
            // "somehost". We need to copy that information so that when we retry with a different search
            // domain e.g., mycompany.local, we get "somehost.mycompany.local".
            if (q->qnameOrig)
            {
                //(*question2)->qnameOrig =  mallocL("SendAdditionalQuery", DomainNameLength(q->qnameOrig));
                (*question2)->qnameOrig = malloc(DomainNameLength(q->qnameOrig));
                if (!(*question2)->qnameOrig)
                {
                    LogMsg("SendAdditionalQuery: ERROR!!  malloc failure");
                    return mStatus_NoMemoryErr;
                }
                (*question2)->qnameOrig->c[0] = 0;
                AssignDomainName((*question2)->qnameOrig, q->qnameOrig);
                LogInfo("SendAdditionalQuery: qnameOrig %##s", (*question2)->qnameOrig->c);
            }
            // For names of the form "<one-or-more-labels>.bar.local." we always do a second unicast query in parallel.
            // For names of the form "<one-label>.local." it's less clear whether we should do a unicast query.
            // If the name being queried is exactly the same as the name in the DHCP "domain" option (e.g. the DHCP
            // "domain" is my-small-company.local, and the user types "my-small-company.local" into their web browser)
            // then that's a hint that it's worth doing a unicast query. Otherwise, we first check to see if the
            // site's DNS server claims there's an SOA record for "local", and if so, that's also a hint that queries
            // for names in the "local" domain will be safely answered privately before they hit the root name servers.
            // Note that in the "my-small-company.local" example above there will typically be an SOA record for
            // "my-small-company.local" but *not* for "local", which is why the "local SOA" check would fail in that case.
            // We need to check against both ActiveDirectoryPrimaryDomain and SearchList. If it matches against either
            // of those, we don't want do the SOA check for the local
            if (labels == 2 && !SameDomainName(&q->qname, &ActiveDirectoryPrimaryDomain) &&
                !DomainInSearchList(&q->qname, mDNSfalse))
            {
                AssignDomainName(&q2->qname, &localdomain);
                q2->qtype          = kDNSType_SOA;
                q2->LongLived      = mDNSfalse;
                q2->ForceMCast     = mDNSfalse;
                q2->ReturnIntermed = mDNStrue;
                // Don't append search domains for the .local SOA query
                q2->AppendSearchDomains      = 0;
                q2->AppendLocalSearchDomains = 0;
                q2->RetryWithSearchDomains   = mDNSfalse;
                q2->SearchListIndex          = 0;
                q2->TimeoutQuestion          = 0;
                q2->AnonInfo                 = mDNSNULL;
                q2->pid                      = request->process_id;
            }
            LogOperation("%3d: DNSServiceQueryRecord(%##s, %s) unicast", request->sd, q2->qname.c, DNSTypeName(q2->qtype));
            err = mDNS_StartQuery(&mDNSStorage, q2);
            if (err)
                LogMsg("%3d: ERROR: DNSServiceQueryRecord %##s %s mDNS_StartQuery: %d", request->sd, q2->qname.c,
                       DNSTypeName(q2->qtype), (int) err);
        }
    return (err);
#else  // !UNICAST_DISABLED
    (void) q;
    (void) request;
    (void) err;

    return mStatus_NoError;
#endif // !UNICAST_DISABLED
}
#endif // APPLE_OSX_mDNSResponder

// This function tries to append a search domain if valid and possible. If so, returns true.
mDNSlocal mDNSBool RetryQuestionWithSearchDomains(mDNS * const m, DNSQuestion * question, request_state * req, QC_result AddRecord)
{
    int result;
    // RetryWithSearchDomains tells the core to call us back so that we can retry with search domains if there is no
    // answer in the cache or /etc/hosts. In the first call back from the core, we clear RetryWithSearchDomains so
    // that we don't get called back repeatedly. If we got an answer from the cache or /etc/hosts, we don't touch
    // RetryWithSearchDomains which may or may not be set.
    //
    // If we get e.g., NXDOMAIN and the query is neither suppressed nor exhausted the domain search list and
    // is a valid question for appending search domains, retry by appending domains

    if ((AddRecord != QC_suppressed) && question->SearchListIndex != -1 && question->AppendSearchDomains)
    {
        question->RetryWithSearchDomains = 0;
        result                           = AppendNewSearchDomain(m, question);
        // As long as the result is either zero or 1, we retry the question. If we exahaust the search
        // domains (result is zero) we try the original query (as it was before appending the search
        // domains) as such on the wire as a last resort if we have not tried them before. For queries
        // with more than one label, we have already tried them before appending search domains and
        // hence don't retry again
        if (result != -1)
        {
            mStatus err;
            err = mDNS_StartQuery(m, question);
            if (!err)
            {
                LogOperation("%3d: RetryQuestionWithSearchDomains(%##s, %s), retrying after appending search domain", req->sd,
                             question->qname.c, DNSTypeName(question->qtype));
                // If the result was zero, it meant that there are no search domains and we just retried the question
                // as a single label and we should not retry with search domains anymore.
                if (!result)
                    question->SearchListIndex = -1;
                return mDNStrue;
            }
            else
            {
                LogMsg("%3d: ERROR: RetryQuestionWithSearchDomains %##s %s mDNS_StartQuery: %d, while retrying with search domains",
                       req->sd, question->qname.c, DNSTypeName(question->qtype), (int) err);
                // We have already stopped the query and could not restart. Reset the appropriate pointers
                // so that we don't call stop again when the question terminates
                question->QuestionContext = mDNSNULL;
            }
        }
    }
    else
    {
        LogInfo("%3d: RetryQuestionWithSearchDomains: Not appending search domains - SuppressQuery %d, SearchListIndex %d, "
                "AppendSearchDomains %d",
                req->sd, AddRecord, question->SearchListIndex, question->AppendSearchDomains);
    }
    return mDNSfalse;
}

mDNSlocal void queryrecord_result_reply(mDNS * const m, request_state * req, DNSQuestion * question,
                                        const ResourceRecord * const answer, QC_result AddRecord, DNSServiceErrorType error)
{
    char name[MAX_ESCAPED_DOMAIN_NAME];
    size_t len;
    DNSServiceFlags flags = 0;
    reply_state * rep;
    char * data;

    ConvertDomainNameToCString(answer->name, name);

    LogOperation("%3d: %s(%##s, %s) %s %s", req->sd,
                 req->hdr.op == query_request ? "DNSServiceQueryRecord" : "DNSServiceGetAddrInfo", question->qname.c,
                 DNSTypeName(question->qtype), AddRecord ? "ADD" : "RMV", RRDisplayString(m, answer));

    len = sizeof(DNSServiceFlags); // calculate reply data length
    len += sizeof(mDNSu32);        // interface index
    len += sizeof(DNSServiceErrorType);
    len += strlen(name) + 1;
    len += 3 * sizeof(mDNSu16); // type, class, rdlen
    len += answer->rdlength;
    len += sizeof(mDNSu32); // TTL

    rep = create_reply(req->hdr.op == query_request ? query_reply_op : addrinfo_reply_op, len, req);

    if (AddRecord)
        flags |= kDNSServiceFlagsAdd;
    if (question->ValidationStatus != 0)
    {
        error = kDNSServiceErr_NoError;
        if (question->ValidationRequired && question->ValidationState == DNSSECValDone)
        {
            switch (question->ValidationStatus) // Set the dnssec flags to be passed on to the Apps here
            {
            case DNSSEC_Secure:
                flags |= kDNSServiceFlagsSecure;
                break;
            case DNSSEC_Insecure:
                flags |= kDNSServiceFlagsInsecure;
                break;
            case DNSSEC_Indeterminate:
                flags |= kDNSServiceFlagsIndeterminate;
                break;
            case DNSSEC_Bogus:
                flags |= kDNSServiceFlagsBogus;
                break;
            default:
                LogMsg("queryrecord_result_reply unknown status %d for %##s", question->ValidationStatus, question->qname.c);
            }
        }
    }

    rep->rhdr->flags = dnssd_htonl(flags);
    // Call mDNSPlatformInterfaceIndexfromInterfaceID, but suppressNetworkChange (last argument). Otherwise, if the
    // InterfaceID is not valid, then it simulates a "NetworkChanged" which in turn makes questions
    // to be stopped and started including  *this* one. Normally the InterfaceID is valid. But when we
    // are using the /etc/hosts entries to answer a question, the InterfaceID may not be known to the
    // mDNS core . Eventually, we should remove the calls to "NetworkChanged" in
    // mDNSPlatformInterfaceIndexfromInterfaceID when it can't find InterfaceID as ResourceRecords
    // should not have existed to answer this question if the corresponding interface is not valid.
    rep->rhdr->ifi   = dnssd_htonl(mDNSPlatformInterfaceIndexfromInterfaceID(m, answer->InterfaceID, mDNStrue));
    rep->rhdr->error = dnssd_htonl(error);

    data = (char *) &rep->rhdr[1];

    put_string(name, &data);
    put_uint16(answer->rrtype, &data);
    put_uint16(answer->rrclass, &data);
    put_uint16(answer->rdlength, &data);
    // We need to use putRData here instead of the crude put_rdata function, because the crude put_rdata
    // function just does a blind memory copy without regard to structures that may have holes in them.
    if (answer->rdlength)
        if (!putRData(mDNSNULL, (mDNSu8 *) data, (mDNSu8 *) rep->rhdr + len, answer))
            LogMsg("queryrecord_result_reply putRData failed %d", (mDNSu8 *) rep->rhdr + len - (mDNSu8 *) data);
    data += answer->rdlength;
    put_uint32(AddRecord ? answer->rroriginalttl : 0, &data);

    append_reply(req, rep);
    // Stop the question, if we just timed out
    if (error == kDNSServiceErr_Timeout)
    {
        mDNS_StopQuery(m, question);
        // Reset the pointers so that we don't call stop on termination
        question->QuestionContext = mDNSNULL;
    }
    else if ((AddRecord == QC_add) && req->hdr.op == addrinfo_request)
    {
        // Note: We count all answers including LocalOnly e.g., /etc/hosts. If we
        // exclude that, v4ans/v6ans will be zero and we would wrongly think that
        // we did not answer questions and setup the status to deliver triggers.
        if (question->qtype == kDNSType_A)
            req->u.addrinfo.v4ans = 1;
        if (question->qtype == kDNSType_AAAA)
            req->u.addrinfo.v6ans = 1;
    }
    else if ((AddRecord == QC_add) && req->hdr.op == query_request)
    {
        if (question->qtype == kDNSType_A || question->qtype == kDNSType_AAAA)
            req->u.queryrecord.ans = 1;
    }

#if APPLE_OSX_mDNSResponder
#if !NO_WCF
    CHECK_WCF_FUNCTION(WCFIsServerRunning)
    {
        struct xucred x;
        socklen_t xucredlen = sizeof(x);

        if (WCFIsServerRunning((WCFConnection *) m->WCF) && answer->rdlength != 0)
        {
            if (getsockopt(req->sd, 0, LOCAL_PEERCRED, &x, &xucredlen) >= 0 && (x.cr_version == XUCRED_VERSION))
            {
                struct sockaddr_storage addr;
                const RDataBody2 * const rdb = (RDataBody2 *) answer->rdata->u.data;
                addr.ss_len                  = 0;
                if (answer->rrtype == kDNSType_A || answer->rrtype == kDNSType_AAAA)
                {
                    if (answer->rrtype == kDNSType_A)
                    {
                        struct sockaddr_in * sin = (struct sockaddr_in *) &addr;
                        sin->sin_port            = 0;
                        if (!putRData(mDNSNULL, (mDNSu8 *) &sin->sin_addr, (mDNSu8 *) (&sin->sin_addr + sizeof(rdb->ipv4)), answer))
                            LogMsg("queryrecord_result_reply: WCF AF_INET putRData failed");
                        else
                        {
                            addr.ss_len    = sizeof(struct sockaddr_in);
                            addr.ss_family = AF_INET;
                        }
                    }
                    else if (answer->rrtype == kDNSType_AAAA)
                    {
                        struct sockaddr_in6 * sin6 = (struct sockaddr_in6 *) &addr;
                        sin6->sin6_port            = 0;
                        if (!putRData(mDNSNULL, (mDNSu8 *) &sin6->sin6_addr, (mDNSu8 *) (&sin6->sin6_addr + sizeof(rdb->ipv6)),
                                      answer))
                            LogMsg("queryrecord_result_reply: WCF AF_INET6 putRData failed");
                        else
                        {
                            addr.ss_len    = sizeof(struct sockaddr_in6);
                            addr.ss_family = AF_INET6;
                        }
                    }
                    if (addr.ss_len)
                    {
                        debugf("queryrecord_result_reply: Name %s, uid %u, addr length %d", name, x.cr_uid, addr.ss_len);
                        CHECK_WCF_FUNCTION((WCFConnection *) WCFNameResolvesToAddr)
                        {
                            WCFNameResolvesToAddr(m->WCF, name, (struct sockaddr *) &addr, x.cr_uid);
                        }
                    }
                }
                else if (answer->rrtype == kDNSType_CNAME)
                {
                    domainname cname;
                    char cname_cstr[MAX_ESCAPED_DOMAIN_NAME];
                    if (!putRData(mDNSNULL, cname.c, (mDNSu8 *) (cname.c + MAX_DOMAIN_NAME), answer))
                        LogMsg("queryrecord_result_reply: WCF CNAME putRData failed");
                    else
                    {
                        ConvertDomainNameToCString(&cname, cname_cstr);
                        CHECK_WCF_FUNCTION((WCFConnection *) WCFNameResolvesToAddr)
                        {
                            WCFNameResolvesToName(m->WCF, name, cname_cstr, x.cr_uid);
                        }
                    }
                }
            }
            else
                my_perror("queryrecord_result_reply: ERROR: getsockopt LOCAL_PEERCRED");
        }
    }
#endif
#endif
}

mDNSlocal void queryrecord_result_callback(mDNS * const m, DNSQuestion * question, const ResourceRecord * const answer,
                                           QC_result AddRecord)
{
    request_state * req       = question->QuestionContext;
    DNSServiceErrorType error = kDNSServiceErr_NoError;
    DNSQuestion * q           = mDNSNULL;

#if APPLE_OSX_mDNSResponder
    {
        // Sanity check: QuestionContext is set to NULL after we stop the question and hence we should not
        // get any callbacks from the core after this.
        if (!req)
        {
            LogMsg("queryrecord_result_callback: ERROR!! QuestionContext NULL for %##s (%s)", question->qname.c,
                   DNSTypeName(question->qtype));
            return;
        }
        if (req->hdr.op == query_request && question == req->u.queryrecord.q2)
            q = &req->u.queryrecord.q;
        else if (req->hdr.op == addrinfo_request && question == req->u.addrinfo.q42)
            q = &req->u.addrinfo.q4;
        else if (req->hdr.op == addrinfo_request && question == req->u.addrinfo.q62)
            q = &req->u.addrinfo.q6;

        if (q && question->qtype != q->qtype && !SameDomainName(&question->qname, &q->qname))
        {
            mStatus err;
            domainname * orig = question->qnameOrig;

            LogInfo("queryrecord_result_callback: Stopping q2 local %##s", question->qname.c);
            mDNS_StopQuery(m, question);
            question->QuestionContext = mDNSNULL;

            // We got a negative response for the SOA record indicating that .local does not exist.
            // But we might have other search domains (that does not end in .local) that can be
            // appended to this question. In that case, we want to retry the question. Otherwise,
            // we don't want to try this question as unicast.
            if (answer->RecordType == kDNSRecordTypePacketNegative && !q->AppendSearchDomains)
            {
                LogInfo("queryrecord_result_callback: question %##s AppendSearchDomains zero", q->qname.c);
                return;
            }

            // If we got a non-negative answer for our "local SOA" test query, start an additional parallel unicast query
            //
            // Note: When we copy the original question, we copy everything including the AppendSearchDomains,
            // RetryWithSearchDomains except for qnameOrig which can be non-NULL if the original question is
            // e.g., somehost and then we appended e.g., ".local" and retried that question. See comment in
            // SendAdditionalQuery as to how qnameOrig gets initialized.
            *question              = *q;
            question->InterfaceID  = mDNSInterface_Unicast;
            question->ExpectUnique = mDNStrue;
            question->qnameOrig    = orig;

            LogOperation("%3d: DNSServiceQueryRecord(%##s, %s) unicast, context %p", req->sd, question->qname.c,
                         DNSTypeName(question->qtype), question->QuestionContext);

            // If the original question timed out, its QuestionContext would already be set to NULL and that's what we copied above.
            // Hence, we need to set it explicitly here.
            question->QuestionContext = req;
            err                       = mDNS_StartQuery(m, question);
            if (err)
                LogMsg("%3d: ERROR: queryrecord_result_callback %##s %s mDNS_StartQuery: %d", req->sd, question->qname.c,
                       DNSTypeName(question->qtype), (int) err);

            // If we got a positive response to local SOA, then try the .local question as unicast
            if (answer->RecordType != kDNSRecordTypePacketNegative)
                return;

            // Fall through and get the next search domain. The question is pointing at .local
            // and we don't want to try that. Try the next search domain. Don't try with local
            // search domains for the unicast question anymore.
            //
            // Note: we started the question above which will be stopped immediately (never sent on the wire)
            // before we pick the next search domain below. RetryQuestionWithSearchDomains assumes that the
            // question has already started.
            question->AppendLocalSearchDomains = 0;
        }

        if (q && AddRecord && AddRecord != QC_dnssec && (question->InterfaceID == mDNSInterface_Unicast) && !answer->rdlength)
        {
            // If we get a negative response to the unicast query that we sent above, retry after appending search domains
            // Note: We could have appended search domains below (where do it for regular unicast questions) instead of doing it
            // here. As we ignore negative unicast answers below, we would never reach the code where the search domains are
            // appended. To keep things simple, we handle unicast ".local" separately here.
            LogInfo("queryrecord_result_callback: Retrying .local question %##s (%s) as unicast after appending search domains",
                    question->qname.c, DNSTypeName(question->qtype));
            if (RetryQuestionWithSearchDomains(m, question, req, AddRecord))
                return;
            if (question->AppendSearchDomains && !question->AppendLocalSearchDomains && IsLocalDomain(&question->qname))
            {
                // If "local" is the last search domain, we need to stop the question so that we don't send the "local"
                // question on the wire as we got a negative response for the local SOA. But, we can't stop the question
                // yet as we may have to timeout the question (done by the "core") for which we need to leave the question
                // in the list. We leave it disabled so that it does not hit the wire.
                LogInfo("queryrecord_result_callback: Disabling .local question %##s (%s)", question->qname.c,
                        DNSTypeName(question->qtype));
                question->ThisQInterval = 0;
            }
        }
        // If we are here it means that either "question" is not "q2" OR we got a positive response for "q2" OR we have no more
        // search domains to append for "q2". In all cases, fall through and deliver the response
    }
#endif // APPLE_OSX_mDNSResponder

    // If a query is being suppressed for some reason, we don't have to do any other
    // processing.
    //
    // Note: We don't check for "SuppressQuery" and instead use QC_suppressed because
    // the "core" needs to temporarily turn off SuppressQuery to answer this query.
    if (AddRecord == QC_suppressed)
    {
        LogInfo("queryrecord_result_callback: Suppressed question %##s (%s)", question->qname.c, DNSTypeName(question->qtype));
        queryrecord_result_reply(m, req, question, answer, AddRecord, kDNSServiceErr_NoSuchRecord);
        return;
    }

    if (answer->RecordType == kDNSRecordTypePacketNegative)
    {
        // If this question needs to be timed out and we have reached the stop time, mark
        // the error as timeout. It is possible that we might get a negative response from an
        // external DNS server at the same time when this question reaches its stop time. We
        // can't tell the difference as there is no indication in the callback. This should
        // be okay as we will be timing out this query anyway.
        mDNS_Lock(m);
        if (question->TimeoutQuestion)
        {
            if ((m->timenow - question->StopTime) >= 0)
            {
                LogInfo("queryrecord_result_callback:Question %##s (%s) timing out, InterfaceID %p", question->qname.c,
                        DNSTypeName(question->qtype), question->InterfaceID);
                error = kDNSServiceErr_Timeout;
            }
        }
        mDNS_Unlock(m);
        // When we're doing parallel unicast and multicast queries for dot-local names (for supporting Microsoft
        // Active Directory sites) we need to ignore negative unicast answers. Otherwise we'll generate negative
        // answers for just about every single multicast name we ever look up, since the Microsoft Active Directory
        // server is going to assert that pretty much every single multicast name doesn't exist.
        //
        // If we are timing out this query, we need to deliver the negative answer to the application
        if (error != kDNSServiceErr_Timeout)
        {
            if (!answer->InterfaceID && IsLocalDomain(answer->name))
            {
                // Sanity check: "q" will be set only if "question" is the .local unicast query.
                if (!q)
                {
                    LogMsg("queryrecord_result_callback: ERROR!! answering multicast question %s with unicast cache record",
                           RRDisplayString(m, answer));
                    return;
                }
#if APPLE_OSX_mDNSResponder
                if (!ShouldDeliverNegativeResponse(m, question))
                {
                    return;
                }
#endif // APPLE_OSX_mDNSResponder
                LogInfo("queryrecord_result_callback:Question %##s (%s) answering local with negative unicast response",
                        question->qname.c, DNSTypeName(question->qtype));
            }
            error = kDNSServiceErr_NoSuchRecord;
        }
    }
    // If we get a negative answer, try appending search domains. Don't append search domains
    // - if we are timing out this question
    // - if the negative response was received as a result of a multicast query
    // - if this is an additional query (q2), we already appended search domains above (indicated by "!q" below)
    // - if this response is forced e.g., dnssec validation result
    if (error != kDNSServiceErr_Timeout)
    {
        if (!q && !answer->InterfaceID && !answer->rdlength && AddRecord && AddRecord != QC_dnssec)
        {
            // If the original question did not end in .local, we did not send an SOA query
            // to figure out whether we should send an additional unicast query or not. If we just
            // appended .local, we need to see if we need to send an additional query. This should
            // normally happen just once because after we append .local, we ignore all negative
            // responses for .local above.
            LogInfo("queryrecord_result_callback: Retrying question %##s (%s) after appending search domains", question->qname.c,
                    DNSTypeName(question->qtype));
            if (RetryQuestionWithSearchDomains(m, question, req, AddRecord))
            {
                // Note: We need to call SendAdditionalQuery every time after appending a search domain as .local could
                // be anywhere in the search domain list.
#if APPLE_OSX_mDNSResponder
                mStatus err = mStatus_NoError;
                err         = SendAdditionalQuery(question, req, err);
                if (err)
                    LogMsg("queryrecord_result_callback: Sending .local SOA query failed, after appending domains");
#endif // APPLE_OSX_mDNSResponder
                return;
            }
        }
    }
    queryrecord_result_reply(m, req, question, answer, AddRecord, error);
}

mDNSlocal void queryrecord_termination_callback(request_state * request)
{
    LogOperation("%3d: DNSServiceQueryRecord(%##s, %s) STOP PID[%d](%s)", request->sd, request->u.queryrecord.q.qname.c,
                 DNSTypeName(request->u.queryrecord.q.qtype), request->process_id, request->pid_name);
    if (request->u.queryrecord.q.QuestionContext)
    {
        mDNS_StopQuery(&mDNSStorage, &request->u.queryrecord.q); // no need to error check
        LogMcastQ(&mDNSStorage, &request->u.queryrecord.q, request, q_stop);
        request->u.queryrecord.q.QuestionContext = mDNSNULL;
    }
    else
    {
        DNSQuestion * question = &request->u.queryrecord.q;
        LogInfo("queryrecord_termination_callback: question %##s (%s) already stopped, InterfaceID %p", question->qname.c,
                DNSTypeName(question->qtype), question->InterfaceID);
    }

    if (request->u.queryrecord.q.qnameOrig)
    {
        free(request->u.queryrecord.q.qnameOrig);
        request->u.queryrecord.q.qnameOrig = mDNSNULL;
    }

    if (callExternalHelpers(request->u.queryrecord.q.InterfaceID, &request->u.queryrecord.q.qname, request->flags))
    {
        LogInfo("queryrecord_termination_callback: calling external_stop_browsing_for_service()");
        external_stop_browsing_for_service(request->u.queryrecord.q.InterfaceID, &request->u.queryrecord.q.qname,
                                           request->u.queryrecord.q.qtype, request->flags);
    }
    if (request->u.queryrecord.q2)
    {
        if (request->u.queryrecord.q2->QuestionContext)
        {
            LogInfo("queryrecord_termination_callback: Stopping q2 %##s", request->u.queryrecord.q2->qname.c);
            mDNS_StopQuery(&mDNSStorage, request->u.queryrecord.q2);
            LogMcastQ(&mDNSStorage, request->u.queryrecord.q2, request, q_stop);
        }
        else
        {
            DNSQuestion * question = request->u.queryrecord.q2;
            LogInfo("queryrecord_termination_callback: q2 %##s (%s) already stopped, InterfaceID %p", question->qname.c,
                    DNSTypeName(question->qtype), question->InterfaceID);
        }
        if (request->u.queryrecord.q2->qnameOrig)
        {
            LogInfo("queryrecord_termination_callback: freeing q2 qnameOrig %##s", request->u.queryrecord.q2->qnameOrig->c);
            free(request->u.queryrecord.q2->qnameOrig);
            request->u.queryrecord.q2->qnameOrig = mDNSNULL;
        }
        free(request->u.queryrecord.q2);
        request->u.queryrecord.q2 = mDNSNULL;
    }
#if APPLE_OSX_mDNSResponder
    {
        if (request->u.queryrecord.ans)
        {
            DNSQuestion *v4q, *v6q;
            // If we are receiving poisitive answers, provide the hint to the
            // upper layer.
            v4q = v6q = mDNSNULL;
            if (request->u.queryrecord.q.qtype == kDNSType_A)
                v4q = &request->u.queryrecord.q;
            else if (request->u.queryrecord.q.qtype == kDNSType_AAAA)
                v6q = &request->u.queryrecord.q;
            mDNSPlatformTriggerDNSRetry(&mDNSStorage, v4q, v6q);
        }
    }
#endif // APPLE_OSX_mDNSResponder
}

mDNSlocal void SetQuestionPolicy(DNSQuestion * q, request_state * req)
{
    int i;

    // The policy is either based on pid or UUID. Pass a zero pid
    // to the "core" if the UUID is valid. If we always pass the pid,
    // then the "core" needs to determine whether the uuid is valid
    // by examining all the 16 bytes at the time of the policy
    // check and also when setting the delegate socket option. Also, it
    // requires that we zero out the uuid wherever the question is
    // initialized to make sure that it is not interpreted as valid.
    // To prevent these intrusive changes, just pass a zero pid to indicate
    // that pid is not valid when uuid is valid. In future if we need the
    // pid in the question, we will reevaluate this strategy.
    if (req->validUUID)
    {
        for (i = 0; i < UUID_SIZE; i++)
        {
            q->uuid[i] = req->uuid[i];
        }
        q->pid = 0;
    }
    else
    {
        q->pid = req->process_id;
    }
}

mDNSlocal mStatus handle_queryrecord_request(request_state * request)
{
    DNSQuestion * const q = &request->u.queryrecord.q;
    // char name[256];
    char name[64];
    mDNSu16 rrtype, rrclass;
    mStatus err;

    DNSServiceFlags flags       = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex      = get_uint32(&request->msgptr, request->msgend);
    mDNSInterfaceID InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    if (interfaceIndex && !InterfaceID)
        return (mStatus_BadParamErr);

    // if (get_string(&request->msgptr, request->msgend, name, 256) < 0) return(mStatus_BadParamErr);
    if (get_string(&request->msgptr, request->msgend, name, 64) < 0)
        return (mStatus_BadParamErr);
    rrtype  = get_uint16(&request->msgptr, request->msgend);
    rrclass = get_uint16(&request->msgptr, request->msgend);

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceQueryRecord(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    request->flags = flags;
    mDNSPlatformMemZero(&request->u.queryrecord, sizeof(request->u.queryrecord));

    q->InterfaceID = InterfaceID;
    q->flags       = flags;
    q->Target      = zeroAddr;
    if (!MakeDomainNameFromDNSNameString(&q->qname, name))
        return (mStatus_BadParamErr);
#if 0
    if (!AuthorizedDomain(request, &q->qname, AutoBrowseDomains)) return (mStatus_NoError);
#endif
    q->qtype            = rrtype;
    q->qclass           = rrclass;
    q->LongLived        = (flags & kDNSServiceFlagsLongLivedQuery) != 0;
    q->ExpectUnique     = mDNSfalse;
    q->ForceMCast       = (flags & kDNSServiceFlagsForceMulticast) != 0;
    q->ReturnIntermed   = (flags & kDNSServiceFlagsReturnIntermediates) != 0;
    q->SuppressUnusable = (flags & kDNSServiceFlagsSuppressUnusable) != 0;
    q->TimeoutQuestion  = (flags & kDNSServiceFlagsTimeout) != 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    q->WakeOnResolve = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    q->UseBackgroundTrafficClass = (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    if ((flags & kDNSServiceFlagsValidate) != 0)
        q->ValidationRequired = DNSSEC_VALIDATION_SECURE;
    else if ((flags & kDNSServiceFlagsValidateOptional) != 0)
        q->ValidationRequired = DNSSEC_VALIDATION_SECURE_OPTIONAL;
    q->ValidatingResponse = 0;
    q->ProxyQuestion      = 0;
    q->AnonInfo           = mDNSNULL;
    q->QuestionCallback   = queryrecord_result_callback;
    q->QuestionContext    = request;
    q->SearchListIndex    = 0;

    q->DNSSECAuthInfo  = mDNSNULL;
    q->DAIFreeCallback = mDNSNULL;

    // Turn off dnssec validation for local domains and Question Types: RRSIG/ANY(ANY Type is not supported yet)
    if ((IsLocalDomain(&q->qname)) || (q->qtype == kDNSServiceType_RRSIG) || (q->qtype == kDNSServiceType_ANY))
        q->ValidationRequired = 0;

    // Don't append search domains for fully qualified domain names including queries
    // such as e.g., "abc." that has only one label. We convert all names to FQDNs as internally
    // we only deal with FQDNs. Hence, we cannot look at qname to figure out whether we should
    // append search domains or not.  So, we record that information in AppendSearchDomains.
    //
    // We append search domains only for queries that are a single label. If overriden using command line
    // argument "AlwaysAppendSearchDomains", then we do it for any query which is not fully qualified.
    // For DNSSEC questions, append search domains only if kDNSServiceFlagsValidateOptional is set.

    if ((!(q->ValidationRequired == DNSSEC_VALIDATION_SECURE)) && (!(q->ValidationRequired == DNSSEC_VALIDATION_INSECURE)) &&
        (rrtype == kDNSType_A || rrtype == kDNSType_AAAA) && name[strlen(name) - 1] != '.' &&
        (AlwaysAppendSearchDomains || CountLabels(&q->qname) == 1))
    {
        q->AppendSearchDomains      = 1;
        q->AppendLocalSearchDomains = 1;
    }
    else
    {
        q->AppendSearchDomains      = 0;
        q->AppendLocalSearchDomains = 0;
    }

    // For single label queries that are not fully qualified, look at /etc/hosts, cache and try
    // search domains before trying them on the wire as a single label query. RetryWithSearchDomains
    // tell the core to call back into the UDS layer if there is no valid response in /etc/hosts or
    // the cache
    q->RetryWithSearchDomains = ApplySearchDomainsFirst(q) ? 1 : 0;
    q->qnameOrig              = mDNSNULL;
    SetQuestionPolicy(q, request);

    LogOperation("%3d: DNSServiceQueryRecord(%X, %d, %##s, %s) START PID[%d](%s)", request->sd, flags, interfaceIndex, q->qname.c,
                 DNSTypeName(q->qtype), request->process_id, request->pid_name);
    err = mDNS_StartQuery(&mDNSStorage, q);

    if (err)
        LogMsg("%3d: ERROR: DNSServiceQueryRecord %##s %s mDNS_StartQuery: %d", request->sd, q->qname.c, DNSTypeName(q->qtype),
               (int) err);
    else
    {
        request->terminate = queryrecord_termination_callback;
        LogMcastQ(&mDNSStorage, q, request, q_start);
        if (callExternalHelpers(q->InterfaceID, &q->qname, flags))
        {
            LogInfo("handle_queryrecord_request: calling external_start_browsing_for_service()");
            external_start_browsing_for_service(q->InterfaceID, &q->qname, q->qtype, flags);
        }
    }

#if APPLE_OSX_mDNSResponder
    err = SendAdditionalQuery(q, request, err);
#endif // APPLE_OSX_mDNSResponder

    return (err);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceEnumerateDomains
#endif

mDNSlocal reply_state * format_enumeration_reply(request_state * request, const char * domain, DNSServiceFlags flags, mDNSu32 ifi,
                                                 DNSServiceErrorType err)
{
    size_t len;
    reply_state * reply;
    char * data;

    len = sizeof(DNSServiceFlags);
    len += sizeof(mDNSu32);
    len += sizeof(DNSServiceErrorType);
    len += strlen(domain) + 1;

    reply              = create_reply(enumeration_reply_op, len, request);
    reply->rhdr->flags = dnssd_htonl(flags);
    reply->rhdr->ifi   = dnssd_htonl(ifi);
    reply->rhdr->error = dnssd_htonl(err);
    data               = (char *) &reply->rhdr[1];
    put_string(domain, &data);
    return reply;
}

mDNSlocal void enum_termination_callback(request_state * request)
{
#ifndef REMOVE_BROWSE_REQUEST
    // Stop the domain enumeration queries to discover the WAB Browse/Registration domains
    if (request->u.enumeration.flags & kDNSServiceFlagsRegistrationDomains)
    {
        LogInfo("%3d: DNSServiceEnumeration Cancel WAB Registration PID[%d](%s)", request->sd, request->process_id,
                request->pid_name);
        uDNS_StopWABQueries(&mDNSStorage, UDNS_WAB_REG_QUERY);
    }
    else
    {
        LogInfo("%3d: DNSServiceEnumeration Cancel WAB Browse PID[%d](%s)", request->sd, request->process_id, request->pid_name);
        uDNS_StopWABQueries(&mDNSStorage, UDNS_WAB_BROWSE_QUERY);
    }
#endif //#ifndef REMOVE_BROWSE_REQUEST
    mDNS_StopGetDomains(&mDNSStorage, &request->u.enumeration.q_all);
    mDNS_StopGetDomains(&mDNSStorage, &request->u.enumeration.q_default);
}

mDNSlocal void enum_result_callback(mDNS * const m, DNSQuestion * const question, const ResourceRecord * const answer,
                                    QC_result AddRecord)
{
    char domain[MAX_ESCAPED_DOMAIN_NAME];
    request_state * request = question->QuestionContext;
    DNSServiceFlags flags   = 0;
    reply_state * reply;
    (void) m; // Unused

    if (answer->rrtype != kDNSType_PTR)
        return;

#if 0
    if (!AuthorizedDomain(request, &answer->rdata->u.name, request->u.enumeration.flags ? AutoRegistrationDomains : AutoBrowseDomains)) return;
#endif

    // We only return add/remove events for the browse and registration lists
    // For the default browse and registration answers, we only give an "ADD" event
    if (question == &request->u.enumeration.q_default && !AddRecord)
        return;

    if (AddRecord)
    {
        flags |= kDNSServiceFlagsAdd;
        if (question == &request->u.enumeration.q_default)
            flags |= kDNSServiceFlagsDefault;
    }

    ConvertDomainNameToCString(&answer->rdata->u.name, domain);
    // Note that we do NOT propagate specific interface indexes to the client - for example, a domain we learn from
    // a machine's system preferences may be discovered on the LocalOnly interface, but should be browsed on the
    // network, so we just pass kDNSServiceInterfaceIndexAny
    reply = format_enumeration_reply(request, domain, flags, kDNSServiceInterfaceIndexAny, kDNSServiceErr_NoError);
    if (!reply)
    {
        LogMsg("ERROR: enum_result_callback, format_enumeration_reply");
        return;
    }

    LogOperation("%3d: DNSServiceEnumerateDomains(%#2s) RESULT %s: %s", request->sd, question->qname.c, AddRecord ? "Add" : "Rmv",
                 domain);

    append_reply(request, reply);
}

mDNSlocal mStatus handle_enum_request(request_state * request)
{
    mStatus err;
    DNSServiceFlags flags       = get_flags(&request->msgptr, request->msgend);
    DNSServiceFlags reg         = flags & kDNSServiceFlagsRegistrationDomains;
    mDNS_DomainType t_all       = reg ? mDNS_DomainTypeRegistration : mDNS_DomainTypeBrowse;
    mDNS_DomainType t_default   = reg ? mDNS_DomainTypeRegistrationDefault : mDNS_DomainTypeBrowseDefault;
    mDNSu32 interfaceIndex      = get_uint32(&request->msgptr, request->msgend);
    mDNSInterfaceID InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    if (interfaceIndex && !InterfaceID)
        return (mStatus_BadParamErr);

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceEnumerateDomains(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    // mark which kind of enumeration we're doing so that we know what domain enumeration queries to stop
    request->u.enumeration.flags = reg;

    // enumeration requires multiple questions, so we must link all the context pointers so that
    // necessary context can be reached from the callbacks
    request->u.enumeration.q_all.QuestionContext     = request;
    request->u.enumeration.q_default.QuestionContext = request;

    // if the caller hasn't specified an explicit interface, we use local-only to get the system-wide list.
    if (!InterfaceID)
        InterfaceID = mDNSInterface_LocalOnly;

    // make the calls
    LogOperation("%3d: DNSServiceEnumerateDomains(%X=%s)", request->sd, flags,
                 (flags & kDNSServiceFlagsBrowseDomains)             ? "kDNSServiceFlagsBrowseDomains"
                     : (flags & kDNSServiceFlagsRegistrationDomains) ? "kDNSServiceFlagsRegistrationDomains"
                                                                     : "<<Unknown>>");
    err = mDNS_GetDomains(&mDNSStorage, &request->u.enumeration.q_all, t_all, NULL, InterfaceID, enum_result_callback, request);
    if (!err)
    {
        err = mDNS_GetDomains(&mDNSStorage, &request->u.enumeration.q_default, t_default, NULL, InterfaceID, enum_result_callback,
                              request);
        if (err)
            mDNS_StopGetDomains(&mDNSStorage, &request->u.enumeration.q_all);
        else
            request->terminate = enum_termination_callback;
    }
#ifndef REMOVE_BROWSE_REQUEST
    if (!err)
    {
        // Start the domain enumeration queries to discover the WAB Browse/Registration domains
        if (reg)
        {
            LogInfo("%3d: DNSServiceEnumerateDomains Start WAB Registration PID[%d](%s)", request->sd, request->process_id,
                    request->pid_name);
            uDNS_StartWABQueries(&mDNSStorage, UDNS_WAB_REG_QUERY);
        }
        else
        {
            LogInfo("%3d: DNSServiceEnumerateDomains Start WAB Browse PID[%d](%s)", request->sd, request->process_id,
                    request->pid_name);
            uDNS_StartWABQueries(&mDNSStorage, UDNS_WAB_BROWSE_QUERY);
        }
    }
#endif //#ifndef REMOVE_BROWSE_REQUEST
    return (err);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceReconfirmRecord & Misc
#endif

mDNSlocal mStatus handle_reconfirm_request(request_state * request)
{
    mStatus status  = mStatus_BadParamErr;
    AuthRecord * rr = read_rr_from_ipc_msg(request, 0, 0);
    if (rr)
    {
        status = mDNS_ReconfirmByValue(&mDNSStorage, &rr->resrec);
        LogOperation((status == mStatus_NoError) ? "%3d: DNSServiceReconfirmRecord(%s) interface %d initiated"
                                                 : "%3d: DNSServiceReconfirmRecord(%s) interface %d failed: %d",
                     request->sd, RRDisplayString(&mDNSStorage, &rr->resrec),
                     mDNSPlatformInterfaceIndexfromInterfaceID(&mDNSStorage, rr->resrec.InterfaceID, mDNSfalse), status);
        free(rr);
    }
    return (status);
}

#if APPLE_OSX_mDNSResponder

mDNSlocal mStatus handle_release_request(request_state * request)
{
    mStatus err = 0;
    // char name[256], regtype[MAX_ESCAPED_DOMAIN_NAME], domain[MAX_ESCAPED_DOMAIN_NAME];
    char name[64], regtype[MAX_ESCAPED_DOMAIN_NAME], domain[MAX_ESCAPED_DOMAIN_NAME];
    domainname instance;

    // extract the data from the message
    DNSServiceFlags flags = get_flags(&request->msgptr, request->msgend);

    // if (get_string(&request->msgptr, request->msgend, name, 256) < 0 ||
    if (get_string(&request->msgptr, request->msgend, name, 64) < 0 ||
        get_string(&request->msgptr, request->msgend, regtype, MAX_ESCAPED_DOMAIN_NAME) < 0 ||
        get_string(&request->msgptr, request->msgend, domain, MAX_ESCAPED_DOMAIN_NAME) < 0)
    {
        LogMsg("ERROR: handle_release_request - Couldn't read name/regtype/domain");
        return (mStatus_BadParamErr);
    }

    if (!request->msgptr)
    {
        LogMsg("%3d: PeerConnectionRelease(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    if (build_domainname_from_strings(&instance, name, regtype, domain) < 0)
    {
        LogMsg("ERROR: handle_release_request bad %s %s %s", name, regtype, domain);
        return (mStatus_BadParamErr);
    }

    LogOperation("%3d: PeerConnectionRelease(%X %##s) START PID[%d](%s)", request->sd, flags, instance.c, request->process_id,
                 request->pid_name);

    external_connection_release(&instance);
    return (err);
}

#else // APPLE_OSX_mDNSResponder

mDNSlocal mStatus handle_release_request(request_state * request)
{
    (void) request;
    return mStatus_UnsupportedErr;
}

#endif // APPLE_OSX_mDNSResponder

mDNSlocal mStatus handle_setdomain_request(request_state * request)
{
    char domainstr[MAX_ESCAPED_DOMAIN_NAME];
    domainname domain;
    DNSServiceFlags flags = get_flags(&request->msgptr, request->msgend);
    (void) flags; // Unused
    if (get_string(&request->msgptr, request->msgend, domainstr, MAX_ESCAPED_DOMAIN_NAME) < 0 ||
        !MakeDomainNameFromDNSNameString(&domain, domainstr))
    {
        LogMsg("%3d: DNSServiceSetDefaultDomainForUser(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    LogOperation("%3d: DNSServiceSetDefaultDomainForUser(%##s)", request->sd, domain.c);
    return (mStatus_NoError);
}

typedef packedstruct
{
    mStatus err;
    mDNSu32 len;
    mDNSu32 vers;
}
DaemonVersionReply;

mDNSlocal void handle_getproperty_request(request_state * request)
{
    const mStatus BadParamErr = dnssd_htonl((mDNSu32) mStatus_BadParamErr);
    // char prop[256];
    char prop[64];
    if (get_string(&request->msgptr, request->msgend, prop, sizeof(prop)) >= 0)
    {
        LogOperation("%3d: DNSServiceGetProperty(%s)", request->sd, prop);
        if (!strcmp(prop, kDNSServiceProperty_DaemonVersion))
        {
            DaemonVersionReply x = { 0, dnssd_htonl(4), dnssd_htonl(_DNS_SD_H) };
            send_all(request->sd, (const char *) &x, sizeof(x));
            return;
        }
    }

    // If we didn't recogize the requested property name, return BadParamErr
    send_all(request->sd, (const char *) &BadParamErr, sizeof(BadParamErr));
}

#ifdef APPLE_OSX_mDNSResponder
// The caller can specify either the pid or the uuid. If the pid is not specified,
// update the effective uuid. Don't overwrite the pid which is used for debugging
// purposes and initialized when the socket is opened.
mDNSlocal void handle_connection_delegate_request(request_state * request)
{
    mDNSs32 pid;
    socklen_t len;

    len = 0;
    pid = get_uint32(&request->msgptr, request->msgend);
#ifdef LOCAL_PEEREPID
    if (pid)
    {
        len = sizeof(pid);
        if (getsockopt(request->sd, SOL_LOCAL, LOCAL_PEEREPID, &request->process_id, &len) != 0)
            return;
        // to extract the process name from the pid value
        if (proc_pidinfo(request->process_id, PROC_PIDT_SHORTBSDINFO, 1, &proc, PROC_PIDT_SHORTBSDINFO_SIZE) == 0)
            return;
        mDNSPlatformStrCopy(request->pid_name, proc.pbsi_comm);
        // LogMsg("handle_connection_delegate_request: process id %d, name %s", request->process_id, request->pid_name);
    }
#endif
#ifdef LOCAL_PEEREUUID
    if (!pid)
    {
        len = UUID_SIZE;
        if (getsockopt(request->sd, SOL_LOCAL, LOCAL_PEEREUUID, request->uuid, &len) != 0)
            return;
        request->validUUID = mDNStrue;
    }
#endif
}
#else
mDNSlocal void handle_connection_delegate_request(request_state * request)
{
    (void) request;
}
#endif

typedef packedstruct
{
    mStatus err;
    mDNSs32 pid;
}
PIDInfo;

mDNSlocal void handle_getpid_request(request_state * request)
{
    const request_state * req;
    mDNSs32 pid           = -1;
    mDNSu16 srcport       = get_uint16(&request->msgptr, request->msgend);
    const DNSQuestion * q = NULL;
    PIDInfo pi;

    LogOperation("%3d: DNSServiceGetPID START", request->sd);

    for (req = all_requests; req; req = req->next)
    {
        if (req->hdr.op == query_request)
            q = &req->u.queryrecord.q;
        else if (req->hdr.op == addrinfo_request)
            q = &req->u.addrinfo.q4;
        else if (req->hdr.op == addrinfo_request)
            q = &req->u.addrinfo.q6;

        if (q && q->LocalSocket != NULL)
        {
            mDNSu16 port = mDNSPlatformGetUDPPort(q->LocalSocket);
            if (port == srcport)
            {
                pid = req->process_id;
                LogInfo("DNSServiceGetPID: srcport %d, pid %d [%s] question %##s", htons(srcport), pid, req->pid_name, q->qname.c);
                break;
            }
        }
    }
    // If we cannot find in the client requests, look to see if this was
    // started by mDNSResponder.
    if (pid == -1)
    {
        for (q = mDNSStorage.Questions; q; q = q->next)
        {
            if (q && q->LocalSocket != NULL)
            {
                mDNSu16 port = mDNSPlatformGetUDPPort(q->LocalSocket);
                if (port == srcport)
                {
#if APPLE_OSX_mDNSResponder
                    pid = getpid();
#endif // APPLE_OSX_mDNSResponder
                    LogInfo("DNSServiceGetPID: srcport %d, pid %d [%s], question %##s", htons(srcport), pid, "_mDNSResponder",
                            q->qname.c);
                    break;
                }
            }
        }
    }

    pi.err = 0;
    pi.pid = pid;
    send_all(request->sd, (const char *) &pi, sizeof(PIDInfo));
    LogOperation("%3d: DNSServiceGetPID STOP", request->sd);
}
#ifndef REMOVE_NAT_FUCTIONALITY
// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceNATPortMappingCreate
#endif

#define DNSServiceProtocol(X)                                                                                                      \
    ((X) == NATOp_AddrRequest ? 0 : (X) == NATOp_MapUDP ? kDNSServiceProtocol_UDP : kDNSServiceProtocol_TCP)

mDNSlocal void port_mapping_termination_callback(request_state * request)
{
    LogOperation("%3d: DNSServiceNATPortMappingCreate(%X, %u, %u, %d) STOP PID[%d](%s)", request->sd,
                 DNSServiceProtocol(request->u.pm.NATinfo.Protocol), mDNSVal16(request->u.pm.NATinfo.IntPort),
                 mDNSVal16(request->u.pm.ReqExt), request->u.pm.NATinfo.NATLease, request->process_id, request->pid_name);
    mDNS_StopNATOperation(&mDNSStorage, &request->u.pm.NATinfo);
}

// Called via function pointer when we get a NAT Traversal (address request or port mapping) response
mDNSlocal void port_mapping_create_request_callback(mDNS * m, NATTraversalInfo * n)
{
    request_state * request = (request_state *) n->clientContext;
    reply_state * rep;
    int replyLen;
    char * data;

    if (!request)
    {
        LogMsg("port_mapping_create_request_callback called with unknown request_state object");
        return;
    }

    // calculate reply data length
    replyLen = sizeof(DNSServiceFlags);
    replyLen += 3 * sizeof(mDNSu32); // if index + addr + ttl
    replyLen += sizeof(DNSServiceErrorType);
    replyLen += 2 * sizeof(mDNSu16); // Internal Port + External Port
    replyLen += sizeof(mDNSu8);      // protocol

    rep = create_reply(port_mapping_reply_op, replyLen, request);

    rep->rhdr->flags = dnssd_htonl(0);
    rep->rhdr->ifi   = dnssd_htonl(mDNSPlatformInterfaceIndexfromInterfaceID(m, n->InterfaceID, mDNSfalse));
    rep->rhdr->error = dnssd_htonl(n->Result);

    data = (char *) &rep->rhdr[1];

    *data++ = request->u.pm.NATinfo.ExternalAddress.b[0];
    *data++ = request->u.pm.NATinfo.ExternalAddress.b[1];
    *data++ = request->u.pm.NATinfo.ExternalAddress.b[2];
    *data++ = request->u.pm.NATinfo.ExternalAddress.b[3];
    *data++ = DNSServiceProtocol(request->u.pm.NATinfo.Protocol);
    *data++ = request->u.pm.NATinfo.IntPort.b[0];
    *data++ = request->u.pm.NATinfo.IntPort.b[1];
    *data++ = request->u.pm.NATinfo.ExternalPort.b[0];
    *data++ = request->u.pm.NATinfo.ExternalPort.b[1];
    put_uint32(request->u.pm.NATinfo.Lifetime, &data);

    LogOperation("%3d: DNSServiceNATPortMappingCreate(%X, %u, %u, %d) RESULT %.4a:%u TTL %u", request->sd,
                 DNSServiceProtocol(request->u.pm.NATinfo.Protocol), mDNSVal16(request->u.pm.NATinfo.IntPort),
                 mDNSVal16(request->u.pm.ReqExt), request->u.pm.NATinfo.NATLease, &request->u.pm.NATinfo.ExternalAddress,
                 mDNSVal16(request->u.pm.NATinfo.ExternalPort), request->u.pm.NATinfo.Lifetime);

    append_reply(request, rep);
}
mDNSlocal mStatus handle_port_mapping_request(request_state * request)
{
    mDNSu32 ttl = 0;
    mStatus err = mStatus_NoError;

    DNSServiceFlags flags       = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex      = get_uint32(&request->msgptr, request->msgend);
    mDNSInterfaceID InterfaceID = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    mDNSu8 protocol             = (mDNSu8) get_uint32(&request->msgptr, request->msgend);
    (void) flags; // Unused
    if (interfaceIndex && !InterfaceID)
        return (mStatus_BadParamErr);
    if (request->msgptr + 8 > request->msgend)
        request->msgptr = NULL;
    else
    {
        request->u.pm.NATinfo.IntPort.b[0] = *request->msgptr++;
        request->u.pm.NATinfo.IntPort.b[1] = *request->msgptr++;
        request->u.pm.ReqExt.b[0]          = *request->msgptr++;
        request->u.pm.ReqExt.b[1]          = *request->msgptr++;
        ttl                                = get_uint32(&request->msgptr, request->msgend);
    }

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceNATPortMappingCreate(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    if (protocol == 0) // If protocol == 0 (i.e. just request public address) then IntPort, ExtPort, ttl must be zero too
    {
        if (!mDNSIPPortIsZero(request->u.pm.NATinfo.IntPort) || !mDNSIPPortIsZero(request->u.pm.ReqExt) || ttl)
            return (mStatus_BadParamErr);
    }
    else
    {
        if (mDNSIPPortIsZero(request->u.pm.NATinfo.IntPort))
            return (mStatus_BadParamErr);
        if (!(protocol & (kDNSServiceProtocol_UDP | kDNSServiceProtocol_TCP)))
            return (mStatus_BadParamErr);
    }

    request->u.pm.NATinfo.Protocol = !protocol  ? NATOp_AddrRequest
        : (protocol == kDNSServiceProtocol_UDP) ? NATOp_MapUDP
                                                : NATOp_MapTCP;
    //       u.pm.NATinfo.IntPort        = already set above
    request->u.pm.NATinfo.RequestedPort  = request->u.pm.ReqExt;
    request->u.pm.NATinfo.NATLease       = ttl;
    request->u.pm.NATinfo.clientCallback = port_mapping_create_request_callback;
    request->u.pm.NATinfo.clientContext  = request;

    LogOperation("%3d: DNSServiceNATPortMappingCreate(%X, %u, %u, %d) START PID[%d](%s)", request->sd, protocol,
                 mDNSVal16(request->u.pm.NATinfo.IntPort), mDNSVal16(request->u.pm.ReqExt), request->u.pm.NATinfo.NATLease,
                 request->process_id, request->pid_name);
    err = mDNS_StartNATOperation(&mDNSStorage, &request->u.pm.NATinfo);
    if (err)
        LogMsg("ERROR: mDNS_StartNATOperation: %d", (int) err);
    else
        request->terminate = port_mapping_termination_callback;

    return (err);
}
#endif //#ifndef REMOVE_NAT_FUCTIONALITY

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNSServiceGetAddrInfo
#endif

mDNSlocal void addrinfo_termination_callback(request_state * request)
{
    LogOperation("%3d: DNSServiceGetAddrInfo(%##s) STOP PID[%d](%s)", request->sd, request->u.addrinfo.q4.qname.c,
                 request->process_id, request->pid_name);

    if (request->u.addrinfo.q4.QuestionContext)
    {
        mDNS_StopQuery(&mDNSStorage, &request->u.addrinfo.q4);
        LogMcastQ(&mDNSStorage, &request->u.addrinfo.q4, request, q_stop);
        request->u.addrinfo.q4.QuestionContext = mDNSNULL;
    }
    if (request->u.addrinfo.q4.qnameOrig)
    {
        free(request->u.addrinfo.q4.qnameOrig);
        request->u.addrinfo.q4.qnameOrig = mDNSNULL;
    }
    if (request->u.addrinfo.q42)
    {
        if (request->u.addrinfo.q42->QuestionContext)
        {
            LogInfo("addrinfo_termination_callback: Stopping q42 %##s", request->u.addrinfo.q42->qname.c);
            mDNS_StopQuery(&mDNSStorage, request->u.addrinfo.q42);
            LogMcastQ(&mDNSStorage, request->u.addrinfo.q42, request, q_stop);
        }
        if (request->u.addrinfo.q42->qnameOrig)
        {
            LogInfo("addrinfo_termination_callback: freeing q42 qnameOrig %##s", request->u.addrinfo.q42->qnameOrig->c);
            free(request->u.addrinfo.q42->qnameOrig);
            request->u.addrinfo.q42->qnameOrig = mDNSNULL;
        }
        free(request->u.addrinfo.q42);
        request->u.addrinfo.q42 = mDNSNULL;
    }

    if (request->u.addrinfo.q6.QuestionContext)
    {
        mDNS_StopQuery(&mDNSStorage, &request->u.addrinfo.q6);
        LogMcastQ(&mDNSStorage, &request->u.addrinfo.q6, request, q_stop);
        request->u.addrinfo.q6.QuestionContext = mDNSNULL;
    }
    if (request->u.addrinfo.q6.qnameOrig)
    {
        free(request->u.addrinfo.q6.qnameOrig);
        request->u.addrinfo.q6.qnameOrig = mDNSNULL;
    }
    if (request->u.addrinfo.q62)
    {
        if (request->u.addrinfo.q62->QuestionContext)
        {
            LogInfo("addrinfo_termination_callback: Stopping q62 %##s", request->u.addrinfo.q62->qname.c);
            mDNS_StopQuery(&mDNSStorage, request->u.addrinfo.q62);
            LogMcastQ(&mDNSStorage, request->u.addrinfo.q62, request, q_stop);
        }
        if (request->u.addrinfo.q62->qnameOrig)
        {
            LogInfo("addrinfo_termination_callback: freeing q62 qnameOrig %##s", request->u.addrinfo.q62->qnameOrig->c);
            free(request->u.addrinfo.q62->qnameOrig);
            request->u.addrinfo.q62->qnameOrig = mDNSNULL;
        }
        free(request->u.addrinfo.q62);
        request->u.addrinfo.q62 = mDNSNULL;
    }
#if APPLE_OSX_mDNSResponder
    {
        DNSQuestion *v4q, *v6q;
        v4q = v6q = mDNSNULL;
        if (request->u.addrinfo.protocol & kDNSServiceProtocol_IPv4)
        {
            // If we are not delivering answers, we may be timing out prematurely.
            // Note down the current state so that we know to retry when we see a
            // valid response again.
            if (request->u.addrinfo.q4.TimeoutQuestion && !request->u.addrinfo.v4ans)
            {
                mDNSPlatformUpdateDNSStatus(&mDNSStorage, &request->u.addrinfo.q4);
            }
            // If we have a v4 answer and if we timed out prematurely before, provide
            // a trigger to the upper layer so that it can retry questions if needed.
            if (request->u.addrinfo.v4ans)
                v4q = &request->u.addrinfo.q4;
        }
        if (request->u.addrinfo.protocol & kDNSServiceProtocol_IPv6)
        {
            if (request->u.addrinfo.q6.TimeoutQuestion && !request->u.addrinfo.v6ans)
            {
                mDNSPlatformUpdateDNSStatus(&mDNSStorage, &request->u.addrinfo.q6);
            }
            if (request->u.addrinfo.v6ans)
                v6q = &request->u.addrinfo.q6;
        }
        mDNSPlatformTriggerDNSRetry(&mDNSStorage, v4q, v6q);
    }
#endif // APPLE_OSX_mDNSResponder
}

mDNSlocal mStatus handle_addrinfo_request(request_state * request)
{
    // char hostname[256];
    char hostname[64];
    domainname d;
    mStatus err = 0;

    DNSServiceFlags flags  = get_flags(&request->msgptr, request->msgend);
    mDNSu32 interfaceIndex = get_uint32(&request->msgptr, request->msgend);

    mDNSPlatformMemZero(&request->u.addrinfo, sizeof(request->u.addrinfo));
    request->u.addrinfo.interface_id = mDNSPlatformInterfaceIDfromInterfaceIndex(&mDNSStorage, interfaceIndex);
    request->u.addrinfo.flags        = flags;
    request->u.addrinfo.protocol     = get_uint32(&request->msgptr, request->msgend);

    if (interfaceIndex && !request->u.addrinfo.interface_id)
        return (mStatus_BadParamErr);
    if (request->u.addrinfo.protocol > (kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6))
        return (mStatus_BadParamErr);

    // if (get_string(&request->msgptr, request->msgend, hostname, 256) < 0) return(mStatus_BadParamErr);
    if (get_string(&request->msgptr, request->msgend, hostname, 64) < 0)
        return (mStatus_BadParamErr);

    if (!request->msgptr)
    {
        LogMsg("%3d: DNSServiceGetAddrInfo(unreadable parameters)", request->sd);
        return (mStatus_BadParamErr);
    }

    if (!MakeDomainNameFromDNSNameString(&d, hostname))
    {
        LogMsg("ERROR: handle_addrinfo_request: bad hostname: %s", hostname);
        return (mStatus_BadParamErr);
    }

#if 0
    if (!AuthorizedDomain(request, &d, AutoBrowseDomains)) return (mStatus_NoError);
#endif

    if (!request->u.addrinfo.protocol)
    {
        flags |= kDNSServiceFlagsSuppressUnusable;
        request->u.addrinfo.protocol = (kDNSServiceProtocol_IPv4 | kDNSServiceProtocol_IPv6);
    }

    request->u.addrinfo.q4.InterfaceID = request->u.addrinfo.q6.InterfaceID = request->u.addrinfo.interface_id;
    request->u.addrinfo.q4.flags = request->u.addrinfo.q6.flags = flags;
    request->u.addrinfo.q4.Target = request->u.addrinfo.q6.Target = zeroAddr;
    request->u.addrinfo.q4.qname = request->u.addrinfo.q6.qname = d;
    request->u.addrinfo.q4.qclass = request->u.addrinfo.q6.qclass = kDNSServiceClass_IN;
    request->u.addrinfo.q4.LongLived = request->u.addrinfo.q6.LongLived = (flags & kDNSServiceFlagsLongLivedQuery) != 0;
    request->u.addrinfo.q4.ExpectUnique = request->u.addrinfo.q6.ExpectUnique = mDNSfalse;
    request->u.addrinfo.q4.ForceMCast = request->u.addrinfo.q6.ForceMCast = (flags & kDNSServiceFlagsForceMulticast) != 0;
    request->u.addrinfo.q4.ReturnIntermed                                 = request->u.addrinfo.q6.ReturnIntermed =
        (flags & kDNSServiceFlagsReturnIntermediates) != 0;
    request->u.addrinfo.q4.SuppressUnusable = request->u.addrinfo.q6.SuppressUnusable =
        (flags & kDNSServiceFlagsSuppressUnusable) != 0;
    request->u.addrinfo.q4.TimeoutQuestion = request->u.addrinfo.q6.TimeoutQuestion = (flags & kDNSServiceFlagsTimeout) != 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    request->u.addrinfo.q4.WakeOnResolve = request->u.addrinfo.q6.WakeOnResolve = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    request->u.addrinfo.q4.UseBackgroundTrafficClass = request->u.addrinfo.q6.UseBackgroundTrafficClass =
        (flags & kDNSServiceFlagsBackgroundTrafficClass) != 0;
    if ((flags & kDNSServiceFlagsValidate) != 0)
        request->u.addrinfo.q4.ValidationRequired = request->u.addrinfo.q6.ValidationRequired = DNSSEC_VALIDATION_SECURE;
    else if ((flags & kDNSServiceFlagsValidateOptional) != 0)
        request->u.addrinfo.q4.ValidationRequired = request->u.addrinfo.q6.ValidationRequired = DNSSEC_VALIDATION_SECURE_OPTIONAL;
    request->u.addrinfo.q4.ValidatingResponse = request->u.addrinfo.q6.ValidatingResponse = 0;
    request->u.addrinfo.q4.ProxyQuestion = request->u.addrinfo.q6.ProxyQuestion = 0;
    request->u.addrinfo.q4.qnameOrig = request->u.addrinfo.q6.qnameOrig = mDNSNULL;
    request->u.addrinfo.q4.AnonInfo = request->u.addrinfo.q6.AnonInfo = mDNSNULL;

    SetQuestionPolicy(&request->u.addrinfo.q4, request);
    SetQuestionPolicy(&request->u.addrinfo.q6, request);

    request->u.addrinfo.q4.DNSSECAuthInfo = request->u.addrinfo.q6.DNSSECAuthInfo = mDNSNULL;
    request->u.addrinfo.q4.DAIFreeCallback = request->u.addrinfo.q6.DAIFreeCallback = mDNSNULL;

    // Turn off dnssec validation for local domains
    if (IsLocalDomain(&d))
        request->u.addrinfo.q4.ValidationRequired = request->u.addrinfo.q6.ValidationRequired = 0;

    if (request->u.addrinfo.protocol & kDNSServiceProtocol_IPv6)
    {
        request->u.addrinfo.q6.qtype           = kDNSServiceType_AAAA;
        request->u.addrinfo.q6.SearchListIndex = 0;
        // For DNSSEC questions, append search domains only if kDNSServiceFlagsValidateOptional is set
        if ((!(request->u.addrinfo.q6.ValidationRequired == DNSSEC_VALIDATION_SECURE)) &&
            (!(request->u.addrinfo.q6.ValidationRequired == DNSSEC_VALIDATION_INSECURE)) && hostname[strlen(hostname) - 1] != '.' &&
            (AlwaysAppendSearchDomains || CountLabels(&d) == 1))
        {
            request->u.addrinfo.q6.AppendSearchDomains      = 1;
            request->u.addrinfo.q6.AppendLocalSearchDomains = 1;
        }
        else
        {
            request->u.addrinfo.q6.AppendSearchDomains      = 0;
            request->u.addrinfo.q6.AppendLocalSearchDomains = 0;
        }
        request->u.addrinfo.q6.RetryWithSearchDomains = (ApplySearchDomainsFirst(&request->u.addrinfo.q6) ? 1 : 0);
        request->u.addrinfo.q6.QuestionCallback       = queryrecord_result_callback;
        request->u.addrinfo.q6.QuestionContext        = request;
        err                                           = mDNS_StartQuery(&mDNSStorage, &request->u.addrinfo.q6);
        if (err != mStatus_NoError)
        {
            LogMsg("ERROR: mDNS_StartQuery: %d", (int) err);
            request->u.addrinfo.q6.QuestionContext = mDNSNULL;
        }
#if APPLE_OSX_mDNSResponder
        err = SendAdditionalQuery(&request->u.addrinfo.q6, request, err);
#endif // APPLE_OSX_mDNSResponder
        if (!err)
        {
            request->terminate = addrinfo_termination_callback;
            LogMcastQ(&mDNSStorage, &request->u.addrinfo.q6, request, q_start);
        }
    }

    if (!err && (request->u.addrinfo.protocol & kDNSServiceProtocol_IPv4))
    {
        request->u.addrinfo.q4.qtype           = kDNSServiceType_A;
        request->u.addrinfo.q4.SearchListIndex = 0;

        // We append search domains only for queries that are a single label. If overriden using cmd line arg
        // "AlwaysAppendSearchDomains", then we do it for any query which is not fully qualified.
        // For DNSSEC questions, append search domains only if kDNSServiceFlagsValidateOptional is set.

        if ((!(request->u.addrinfo.q4.ValidationRequired == DNSSEC_VALIDATION_SECURE)) &&
            (!(request->u.addrinfo.q4.ValidationRequired == DNSSEC_VALIDATION_INSECURE)) && hostname[strlen(hostname) - 1] != '.' &&
            (AlwaysAppendSearchDomains || CountLabels(&d) == 1))
        {
            request->u.addrinfo.q4.AppendSearchDomains      = 1;
            request->u.addrinfo.q4.AppendLocalSearchDomains = 1;
        }
        else
        {
            request->u.addrinfo.q4.AppendSearchDomains      = 0;
            request->u.addrinfo.q4.AppendLocalSearchDomains = 0;
        }
        request->u.addrinfo.q4.RetryWithSearchDomains = (ApplySearchDomainsFirst(&request->u.addrinfo.q4) ? 1 : 0);
        request->u.addrinfo.q4.QuestionCallback       = queryrecord_result_callback;
        request->u.addrinfo.q4.QuestionContext        = request;
        err                                           = mDNS_StartQuery(&mDNSStorage, &request->u.addrinfo.q4);
        if (err != mStatus_NoError)
        {
            LogMsg("ERROR: mDNS_StartQuery: %d", (int) err);
            request->u.addrinfo.q4.QuestionContext = mDNSNULL;
            if (request->u.addrinfo.protocol & kDNSServiceProtocol_IPv6)
            {
                // If we started a query for IPv6, we need to cancel it
                mDNS_StopQuery(&mDNSStorage, &request->u.addrinfo.q6);
                request->u.addrinfo.q6.QuestionContext = mDNSNULL;
            }
        }
#if APPLE_OSX_mDNSResponder
        err = SendAdditionalQuery(&request->u.addrinfo.q4, request, err);
#endif // APPLE_OSX_mDNSResponder
        if (!err)
        {
            request->terminate = addrinfo_termination_callback;
            LogMcastQ(&mDNSStorage, &request->u.addrinfo.q4, request, q_start);
        }
    }

    LogOperation("%3d: DNSServiceGetAddrInfo(%X, %d, %d, %##s) START PID[%d](%s)", request->sd, flags, interfaceIndex,
                 request->u.addrinfo.protocol, d.c, request->process_id, request->pid_name);
    return (err);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Main Request Handler etc.
#endif

mDNSlocal request_state * NewRequest(void)
{
    request_state ** p = &all_requests;
    while (*p)
        p = &(*p)->next;
    //*p = mallocL("request_state", sizeof(request_state));
    *p = malloc(sizeof(request_state));
    if (!*p)
        FatalError("ERROR: malloc");
    mDNSPlatformMemZero(*p, sizeof(request_state));
    return (*p);
}

// read_msg may be called any time when the transfer state (req->ts) is t_morecoming.
// if there is no data on the socket, the socket will be closed and t_terminated will be returned
mDNSlocal void read_msg(request_state * req)
{
    int socket_errno = 0;
    if (req->ts == t_terminated || req->ts == t_error)
    {
        LogMsg("%3d: ERROR: read_msg called with transfer state terminated or error", req->sd);
        req->ts = t_error;
        return;
    }

    if (req->ts == t_complete) // this must be death or something is wrong
    {
        char buf[4]; // dummy for death notification
        int nread    = udsSupportReadFD(req->sd, buf, 4, 0, req->platform_data);
        socket_errno = dnssd_errno;
        if (!nread)
        {
            req->ts = t_terminated;
            return;
        }
        if (nread < 0)
            goto rerror;
        LogMsg("%3d: ERROR: read data from a completed request", req->sd);
        req->ts = t_error;
        return;
    }

    if (req->ts != t_morecoming)
    {
        LogMsg("%3d: ERROR: read_msg called with invalid transfer state (%d)", req->sd, req->ts);
        req->ts = t_error;
        return;
    }

    if (req->hdr_bytes < sizeof(ipc_msg_hdr))
    {
        mDNSu32 nleft = sizeof(ipc_msg_hdr) - req->hdr_bytes;
        int nread     = udsSupportReadFD(req->sd, (char *) &req->hdr + req->hdr_bytes, nleft, 0, req->platform_data);
        socket_errno  = dnssd_errno;
        if (nread == 0)
        {
            req->ts = t_terminated;
            return;
        }
        if (nread < 0)
            goto rerror;
        req->hdr_bytes += nread;
        if (req->hdr_bytes > sizeof(ipc_msg_hdr))
        {
            LogMsg("%3d: ERROR: read_msg - read too many header bytes", req->sd);
            req->ts = t_error;
            return;
        }

        // only read data if header is complete
        if (req->hdr_bytes == sizeof(ipc_msg_hdr))
        {
            ConvertHeaderBytes(&req->hdr);
            if (req->hdr.version != VERSION)
            {
                LogMsg("%3d: ERROR: client version 0x%08X daemon version 0x%08X", req->sd, req->hdr.version, VERSION);
                req->ts = t_error;
                return;
            }

            // Largest conceivable single request is a DNSServiceRegisterRecord() or DNSServiceAddRecord()
            // with 64kB of rdata. Adding 1009 byte for a maximal domain name, plus a safety margin
            // for other overhead, this means any message above 70kB is definitely bogus.
            if (req->hdr.datalen > 70000)
            {
                LogMsg("%3d: ERROR: read_msg: hdr.datalen %u (0x%X) > 70000", req->sd, req->hdr.datalen, req->hdr.datalen);
                req->ts = t_error;
                return;
            }
            // req->msgbuf = mallocL("request_state msgbuf", req->hdr.datalen + MSG_PAD_BYTES);
            req->msgbuf = malloc(req->hdr.datalen + MSG_PAD_BYTES);
            if (!req->msgbuf)
            {
                my_perror("ERROR: malloc");
                req->ts = t_error;
                return;
            }
            req->msgptr = req->msgbuf;
            req->msgend = req->msgbuf + req->hdr.datalen;
            mDNSPlatformMemZero(req->msgbuf, req->hdr.datalen + MSG_PAD_BYTES);
        }
    }

    // If our header is complete, but we're still needing more body data, then try to read it now
    // Note: For cancel_request req->hdr.datalen == 0, but there's no error return socket for cancel_request
    // Any time we need to get the error return socket we know we'll have at least one data byte
    // (even if only the one-byte empty C string placeholder for the old ctrl_path parameter)
    if (req->hdr_bytes == sizeof(ipc_msg_hdr) && req->data_bytes < req->hdr.datalen)
    {
        mDNSu32 nleft = req->hdr.datalen - req->data_bytes;
        int nread;
#if !defined(_WIN32)
#if 0
        struct iovec vec = { req->msgbuf + req->data_bytes, nleft };    // Tell recvmsg where we want the bytes put
        struct msghdr msg;
        struct cmsghdr *cmsg;
        char cbuf[CMSG_SPACE(4 * sizeof(dnssd_sock_t))];
        msg.msg_name       = 0;
        msg.msg_namelen    = 0;
        msg.msg_iov        = &vec;
        msg.msg_iovlen     = 1;
        msg.msg_control    = cbuf;
        msg.msg_controllen = sizeof(cbuf);
        msg.msg_flags      = 0;
        nread = recvmsg(req->sd, &msg, 0);
#endif
        // Stanley Added
        nread = udsSupportReadFD(req->sd, (char *) req->msgbuf + req->data_bytes, nleft, 0, req->platform_data);
#else
        nread = udsSupportReadFD(req->sd, (char *) req->msgbuf + req->data_bytes, nleft, 0, req->platform_data);
#endif
        socket_errno = dnssd_errno;
        if (nread == 0)
        {
            req->ts = t_terminated;
            return;
        }
        if (nread < 0)
            goto rerror;
        req->data_bytes += nread;
        if (req->data_bytes > req->hdr.datalen)
        {
            LogMsg("%3d: ERROR: read_msg - read too many data bytes", req->sd);
            req->ts = t_error;
            return;
        }
#if !defined(_WIN32)
//        cmsg = CMSG_FIRSTHDR(&msg);
#if DEBUG_64BIT_SCM_RIGHTS
        LogMsg("%3d: Expecting %d %d %d %d", req->sd, sizeof(cbuf), sizeof(cbuf), SOL_SOCKET, SCM_RIGHTS);
//        LogMsg("%3d: Got       %d %d %d %d", req->sd, msg.msg_controllen, cmsg->cmsg_len, cmsg->cmsg_level, cmsg->cmsg_type);
#endif // DEBUG_64BIT_SCM_RIGHTS
#if 0
        if (msg.msg_controllen != 0 &&
            cmsg->cmsg_level   == SOL_SOCKET &&
            cmsg->cmsg_type    == SCM_RIGHTS)
        {
#endif
#if APPLE_OSX_mDNSResponder
        // Strictly speaking BPF_fd belongs solely in the platform support layer, but because
        // of privilege separation on Mac OS X we need to get BPF_fd from mDNSResponderHelper,
        // and it's convenient to repurpose the existing fd-passing code here for that task
        if (req->hdr.op == send_bpf)
        {
            dnssd_sock_t x = *(dnssd_sock_t *) CMSG_DATA(cmsg);
            LogOperation("%3d: Got len %d, BPF %d", req->sd, cmsg->cmsg_len, x);
            mDNSPlatformReceiveBPF_fd(&mDNSStorage, x);
        }
        else
#endif // APPLE_OSX_mDNSResponder
// FIXME            req->errsd = *(dnssd_sock_t *)CMSG_DATA(cmsg);
#if DEBUG_64BIT_SCM_RIGHTS
            LogMsg("%3d: read req->errsd %d", req->sd, req->errsd);
#endif // DEBUG_64BIT_SCM_RIGHTS
        if (req->data_bytes < req->hdr.datalen)
        {
            LogMsg("%3d: Client(PID [%d](%s)) sent error socket %d via SCM_RIGHTS with req->data_bytes %d < req->hdr.datalen %d",
                   req->sd, req->process_id, req->pid_name, req->errsd, req->data_bytes, req->hdr.datalen);
            req->ts = t_error;
            return;
        }
//        }
#endif
    }

    // If our header and data are both complete, see if we need to make our separate error return socket
    if (req->hdr_bytes == sizeof(ipc_msg_hdr) && req->data_bytes == req->hdr.datalen)
    {
        if (req->terminate && req->hdr.op != cancel_request)
        {
            dnssd_sockaddr_t cliaddr;
#if defined(USE_TCP_LOOPBACK)
            mDNSOpaque16 port;
            u_long opt = 1;
            port.b[0]  = req->msgptr[0];
            port.b[1]  = req->msgptr[1];
            req->msgptr += 2;
            cliaddr.sin_family      = AF_INET;
            cliaddr.sin_port        = port.NotAnInteger;
            cliaddr.sin_addr.s_addr = inet_addr(MDNS_TCP_SERVERADDR);
#else
            char ctrl_path[MAX_CTLPATH];
            get_string(&req->msgptr, req->msgend, ctrl_path, MAX_CTLPATH); // path is first element in message buffer
            mDNSPlatformMemZero(&cliaddr, sizeof(cliaddr));
            cliaddr.sun_family = AF_LOCAL;
            mDNSPlatformStrCopy(cliaddr.sun_path, ctrl_path);
            // If the error return path UDS name is empty string, that tells us
            // that this is a new version of the library that's going to pass us
            // the error return path socket via sendmsg/recvmsg
            if (ctrl_path[0] == 0)
            {
                if (req->errsd == req->sd)
                {
                    LogMsg("%3d: read_msg: ERROR failed to get errsd via SCM_RIGHTS", req->sd);
                    req->ts = t_error;
                    return;
                }
                goto got_errfd;
            }
#endif

            req->errsd = socket(AF_DNSSD, SOCK_STREAM, 0);
            mdnslogInfo("read_msg socket req->errsd=%d", req->errsd);
            if (!dnssd_SocketValid(req->errsd))
            {
                my_throttled_perror("ERROR: socket");
                req->ts = t_error;
                return;
            }

            mdnslogInfo("read_msg connect socket req->errsd=%d", req->errsd);
            if (connect(req->errsd, (struct sockaddr *) &cliaddr, sizeof(cliaddr)) < 0)
            {
#if !defined(USE_TCP_LOOPBACK)
                struct stat sb;
                LogMsg("%3d: read_msg: Couldn't connect to error return path socket %s errno %d (%s)", req->sd, cliaddr.sun_path,
                       dnssd_errno, dnssd_strerror(dnssd_errno));
                if (stat(cliaddr.sun_path, &sb) < 0)
                    LogMsg("%3d: read_msg: stat failed %s errno %d (%s)", req->sd, cliaddr.sun_path, dnssd_errno,
                           dnssd_strerror(dnssd_errno));
                else
                    LogMsg("%3d: read_msg: file %s mode %o (octal) uid %d gid %d", req->sd, cliaddr.sun_path, sb.st_mode, sb.st_uid,
                           sb.st_gid);
#endif
                mdnslogInfo("read_msg connect socket=%d fail", req->errsd);
                req->ts = t_error;
                return;
            }

#if !defined(USE_TCP_LOOPBACK)
        got_errfd:
#endif
            LogOperation("%3d: Error socket %d created %08X %08X", req->sd, req->errsd, req->hdr.client_context.u32[1],
                         req->hdr.client_context.u32[0]);
#if defined(_WIN32)
            if (ioctlsocket(req->errsd, FIONBIO, &opt) != 0)
#else
            if (fcntl(req->errsd, F_SETFL, fcntl(req->errsd, F_GETFL, 0) | O_NONBLOCK) != 0)
#endif
            {
                LogMsg("%3d: ERROR: could not set control socket to non-blocking mode errno %d (%s)", req->sd, dnssd_errno,
                       dnssd_strerror(dnssd_errno));
                req->ts = t_error;
                return;
            }
        }

        req->ts = t_complete;
    }

    return;

rerror:
    if (dnssd_errno == dnssd_EWOULDBLOCK || dnssd_errno == dnssd_EINTR)
        return;
    // int socket_errno = lwip_socket_get_errno(req->sd);
    mdnslogInfo("%d read_msg ERROR: errno=%d not socket_errno=%d", req->sd, dnssd_errno, socket_errno);
    if (socket_errno == dnssd_EWOULDBLOCK || socket_errno == dnssd_EINTR)
        return;
    req->ts = t_error;
}

#define RecordOrientedOp(X)                                                                                                        \
    ((X) == reg_record_request || (X) == add_record_request || (X) == update_record_request || (X) == remove_record_request)

// The lightweight operations are the ones that don't need a dedicated request_state structure allocated for them
#define LightweightOp(X) (RecordOrientedOp(X) || (X) == cancel_request)

mDNSlocal void request_callback(int fd, short filter, void * info)
{
    mStatus err         = 0;
    request_state * req = info;
    mDNSs32 min_size    = sizeof(DNSServiceFlags);
    (void) fd;     // Unused
    (void) filter; // Unused

    for (;;)
    {
        read_msg(req);
        if (req->ts == t_morecoming)
            return;
        if (req->ts == t_terminated || req->ts == t_error)
        {
            AbortUnlinkAndFree(req);
            return;
        }
        if (req->ts != t_complete)
        {
            LogMsg("request_callback: req->ts %d != t_complete PID[%d][%s]", req->ts, req->process_id, req->pid_name);
            AbortUnlinkAndFree(req);
            return;
        }
        if (req->hdr.version != VERSION)
        {
            LogMsg("request_callback: ERROR: client IPC version %d incompatible with daemon IPC version %d PID[%d][%s]",
                   req->hdr.version, VERSION, req->process_id, req->pid_name);
            AbortUnlinkAndFree(req);
            return;
        }

        switch (req->hdr.op) //          Interface       + other data
        {
        case connection_request:
            min_size = 0;
            break;
        case connection_delegate_request:
            min_size = 4; /* pid */
            break;
        case reg_service_request:
            min_size += sizeof(mDNSu32) + 4 /* name, type, domain, host */ + 4 /* port, textlen */;
            break;
        case add_record_request:
            min_size += 4 /* type, rdlen */ + 4 /* ttl */;
            break;
        case update_record_request:
            min_size += 2 /* rdlen */ + 4 /* ttl */;
            break;
        case remove_record_request:
            break;
#ifndef REMOVE_BROWSE_REQUEST
        case browse_request:
            min_size += sizeof(mDNSu32) + 2 /* type, domain */;
            break;
#endif
        case resolve_request:
            min_size += sizeof(mDNSu32) + 3 /* type, type, domain */;
            break;
        case query_request:
            min_size += sizeof(mDNSu32) + 1 /* name */ + 4 /* type, class*/;
            break;
        case enumeration_request:
            min_size += sizeof(mDNSu32);
            break;
        case reg_record_request:
            min_size += sizeof(mDNSu32) + 1 /* name */ + 6 /* type, class, rdlen */ + 4 /* ttl */;
            break;
        case reconfirm_record_request:
            min_size += sizeof(mDNSu32) + 1 /* name */ + 6 /* type, class, rdlen */;
            break;
        case setdomain_request:
            min_size += 1 /* domain */;
            break;
        case getproperty_request:
            min_size = 2;
            break;
        case getpid_request:
            min_size = 2;
            break;
        case port_mapping_request:
            min_size += sizeof(mDNSu32) + 4 /* udp/tcp */ + 4 /* int/ext port */ + 4 /* ttl */;
            break;
        case addrinfo_request:
            min_size += sizeof(mDNSu32) + 4 /* v4/v6 */ + 1 /* hostname */;
            break;
        case send_bpf: // Same as cancel_request below
        case cancel_request:
            min_size = 0;
            break;
        case release_request:
            min_size += sizeof(mDNSu32) + 3 /* type, type, domain */;
            break;
        default:
            LogMsg("request_callback: ERROR: validate_message - unsupported req type: %d PID[%d][%s]", req->hdr.op, req->process_id,
                   req->pid_name);
            min_size = -1;
            break;
        }

        if ((mDNSs32) req->data_bytes < min_size)
        {
            LogMsg("request_callback: Invalid message %d bytes; min for %d is %d PID[%d][%s]", req->data_bytes, req->hdr.op,
                   min_size, req->process_id, req->pid_name);
            AbortUnlinkAndFree(req);
            return;
        }
        if (LightweightOp(req->hdr.op) && !req->terminate)
        {
            LogMsg("request_callback: Reg/Add/Update/Remove %d require existing connection PID[%d][%s]", req->hdr.op,
                   req->process_id, req->pid_name);
            AbortUnlinkAndFree(req);
            return;
        }

        // check if client wants silent operation
        if (req->hdr.ipc_flags & IPC_FLAGS_NOREPLY)
            req->no_reply = 1;

        // If req->terminate is already set, this means this operation is sharing an existing connection
        if (req->terminate && !LightweightOp(req->hdr.op))
        {
            request_state * newreq = NewRequest();
            newreq->primary        = req;
            newreq->sd             = req->sd;
            newreq->errsd          = req->errsd;
            newreq->uid            = req->uid;
            newreq->hdr            = req->hdr;
            newreq->msgbuf         = req->msgbuf;
            newreq->msgptr         = req->msgptr;
            newreq->msgend         = req->msgend;
            // if the parent request is a delegate connection, copy the
            // relevant bits
            if (req->validUUID)
            {
                int i;
                newreq->validUUID = mDNStrue;
                for (i = 0; i < UUID_SIZE; i++)
                {
                    newreq->uuid[i] = req->uuid[i];
                }
            }
            else
            {
                if (req->process_id)
                {
                    newreq->process_id = req->process_id;
                }
                else
                {
                    set_peer_pid(newreq);
                }
            }
            req = newreq;
        }

        // If we're shutting down, don't allow new client requests
        // We do allow "cancel" and "getproperty" during shutdown
        if (mDNSStorage.ShutdownTime && req->hdr.op != cancel_request && req->hdr.op != getproperty_request)
        {
            err = mStatus_ServiceNotRunning;
        }
        else
        {
            mdnslogInfo("request_callback req->hdr.op=%d", req->hdr.op);
            switch (req->hdr.op)
            {
            // These are all operations that have their own first-class request_state object
            case connection_request:
                LogOperation("%3d: DNSServiceCreateConnection START PID[%d](%s)", req->sd, req->process_id, req->pid_name);
                req->terminate = connection_termination;
                break;
            case connection_delegate_request:
                LogOperation("%3d: DNSServiceCreateDelegateConnection START PID[%d](%s)", req->sd, req->process_id, req->pid_name);
                req->terminate = connection_termination;
                handle_connection_delegate_request(req);
                break;
            case resolve_request:
                err = handle_resolve_request(req);
                break;
            case query_request:
                err = handle_queryrecord_request(req);
                break;
#ifndef REMOVE_BROWSE_REQUEST
            case browse_request:
                err = handle_browse_request(req);
                break;
#endif
            case reg_service_request:
                err = handle_regservice_request(req);
                break;
            case enumeration_request:
                err = handle_enum_request(req);
                break;
            case reconfirm_record_request:
                err = handle_reconfirm_request(req);
                break;
            case setdomain_request:
                err = handle_setdomain_request(req);
                break;
            case getproperty_request:
                handle_getproperty_request(req);
                break;
            case getpid_request:
                handle_getpid_request(req);
                break;
#ifndef REMOVE_NAT_FUCTIONALITY
            case port_mapping_request:
                err = handle_port_mapping_request(req);
                break;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
            case addrinfo_request:
                err = handle_addrinfo_request(req);
                break;
            case send_bpf: /* Do nothing for send_bpf */
                break;

            // These are all operations that work with an existing request_state object
            case reg_record_request:
                err = handle_regrecord_request(req);
                break;
            case add_record_request:
                err = handle_add_request(req);
                break;
            case update_record_request:
                err = handle_update_request(req);
                break;
            case remove_record_request:
                err = handle_removerecord_request(req);
                break;
            case cancel_request:
                handle_cancel_request(req);
                break;
            case release_request:
                err = handle_release_request(req);
                break;
            default:
                LogMsg("request_callback: %3d:ERROR: Unsupported UDS req:%d PID[%d][%s]", req->sd, req->hdr.op, req->process_id,
                       req->pid_name);
                break;
            }
        }
        // req->msgbuf may be NULL, e.g. for connection_request or remove_record_request
        if (req->msgbuf)
            free(req->msgbuf);
        mdnslogInfo("request_callback err=%d socket=%d", err, req->errsd);
        // There's no return data for a cancel request (DNSServiceRefDeallocate returns no result)
        // For a DNSServiceGetProperty call, the handler already generated the response, so no need to do it again here
        if (req->hdr.op != cancel_request && req->hdr.op != getproperty_request && req->hdr.op != send_bpf &&
            req->hdr.op != getpid_request)
        {
            const mStatus err_netorder = dnssd_htonl(err);
            mdnslogInfo("request_callback send_all req->errsd=%d", req->errsd);
            send_all(req->errsd, (const char *) &err_netorder, sizeof(err_netorder));
            if (req->errsd != req->sd)
            {
                mdnslogInfo("%3d: not socket %d closed  %08X %08X (%d)", req->sd, req->errsd, req->hdr.client_context.u32[1],
                            req->hdr.client_context.u32[0], err);
                dnssd_close(req->errsd);
                req->errsd = req->sd;
                // Also need to reset the parent's errsd, if this is a subordinate operation
                if (req->primary)
                    req->primary->errsd = req->primary->sd;
            }
        }

        // Reset ready to accept the next req on this pipe
        if (req->primary)
            req = req->primary;
        req->ts         = t_morecoming;
        req->hdr_bytes  = 0;
        req->data_bytes = 0;
        req->msgbuf     = mDNSNULL;
        req->msgptr     = mDNSNULL;
        req->msgend     = 0;
    }
}

mDNSlocal void connect_callback(int fd, short filter, void * info)
{
    dnssd_sockaddr_t cliaddr;
    dnssd_socklen_t len = (dnssd_socklen_t) sizeof(cliaddr);
    dnssd_sock_t sd     = accept(fd, (struct sockaddr *) &cliaddr, (socklen_t *) &len);
#if defined(SO_NOSIGPIPE) || defined(_WIN32)
    unsigned long optval = 1;
#endif

    (void) filter; // Unused
    (void) info;   // Unused

    if (!dnssd_SocketValid(sd))
    {
        if (dnssd_errno != dnssd_EWOULDBLOCK)
            my_throttled_perror("ERROR: accept");
        return;
    }

#ifdef SO_NOSIGPIPE
    // Some environments (e.g. OS X) support turning off SIGPIPE for a socket
    if (setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, &optval, sizeof(optval)) < 0)
        LogMsg("%3d: WARNING: setsockopt - SO_NOSIGPIPE %d (%s)", sd, dnssd_errno, dnssd_strerror(dnssd_errno));
#endif

#if defined(_WIN32)
    if (ioctlsocket(sd, FIONBIO, &optval) != 0)
#else
    if (fcntl(sd, F_SETFL, fcntl(sd, F_GETFL, 0) | O_NONBLOCK) != 0)
#endif
    {
        my_perror("ERROR: fcntl(sd, F_SETFL, O_NONBLOCK) - aborting client");
        dnssd_close(sd);
        return;
    }
    else
    {
        request_state * request = NewRequest();
        request->ts             = t_morecoming;
        request->sd             = sd;
        request->errsd          = sd;
        set_peer_pid(request);
#if APPLE_OSX_mDNSResponder
        struct xucred x;
        socklen_t xucredlen = sizeof(x);
        if (getsockopt(sd, 0, LOCAL_PEERCRED, &x, &xucredlen) >= 0 && x.cr_version == XUCRED_VERSION)
            request->uid = x.cr_uid;
        else
            my_perror("ERROR: getsockopt, LOCAL_PEERCRED");
        debugf("LOCAL_PEERCRED %d %u %u %d", xucredlen, x.cr_version, x.cr_uid, x.cr_ngroups);
#endif // APPLE_OSX_mDNSResponder
        LogOperation("%3d: Adding FD for uid %u", request->sd, request->uid);
        udsSupportAddFDToEventLoop(sd, request_callback, request, &request->platform_data);
    }
}

mDNSlocal mDNSBool uds_socket_setup(dnssd_sock_t skt)
{
#if defined(SO_NP_EXTENSIONS)
    struct so_np_extensions sonpx;
    socklen_t optlen = sizeof(struct so_np_extensions);
    sonpx.npx_flags  = SONPX_SETOPTSHUT;
    sonpx.npx_mask   = SONPX_SETOPTSHUT;
    if (setsockopt(skt, SOL_SOCKET, SO_NP_EXTENSIONS, &sonpx, optlen) < 0)
        my_perror("WARNING: could not set sockopt - SO_NP_EXTENSIONS");
#endif
#if defined(_WIN32)
    // SEH: do we even need to do this on windows?
    // This socket will be given to WSAEventSelect which will automatically set it to non-blocking
    u_long opt = 1;
    if (ioctlsocket(skt, FIONBIO, &opt) != 0)
#else
    if (fcntl(skt, F_SETFL, fcntl(skt, F_GETFL, 0) | O_NONBLOCK) != 0)
#endif
    {
        my_perror("ERROR: could not set listen socket to non-blocking mode");
        return mDNSfalse;
    }

    if (listen(skt, LISTENQ) != 0)
    {
        my_perror("ERROR: could not listen on listen socket");
        printf("uds_socket_setup: listen socket id %d non-blocking mode failed\n", skt);

        return mDNSfalse;
    }

    if (mStatus_NoError != udsSupportAddFDToEventLoop(skt, connect_callback, (void *) NULL, (void **) NULL))
    {
        my_perror("ERROR: could not add listen socket to event loop");
        return mDNSfalse;
    }
    else
    {
        LogMsg("%3d: Listening for incoming Unix Domain Socket client requests", skt);
        mDNSStorage.uds_listener_skt = skt;
    }
    return mDNStrue;
}

mDNSexport int udsserver_init(dnssd_sock_t skts[], mDNSu32 count)
{
    dnssd_sockaddr_t laddr;
    int ret;
    mDNSu32 i = 0;

    LogInfo("udsserver_init: %d %d", _DNS_SD_H, mDNSStorage.mDNS_plat);

    /* remove useless code for IAR
    // If a particular platform wants to opt out of having a PID file, define PID_FILE to be ""
    if (PID_FILE[0])
    {
        FILE *fp = fopen(PID_FILE, "w");
        if (fp != NULL)
        {
            // FIXED fprintf(fp, "%d\n", getpid());
            fclose(fp);
        }
    }
    */

    if (skts)
    {
        for (i = 0; i < count; i++)
            if (dnssd_SocketValid(skts[i]) && !uds_socket_setup(skts[i]))
                goto error;
    }
    else
    {
        listenfd = socket(AF_DNSSD, SOCK_STREAM, 0);
        if (!dnssd_SocketValid(listenfd))
        {
            my_perror("ERROR: socket(AF_DNSSD, SOCK_STREAM, 0); failed");
            goto error;
        }

        mDNSPlatformMemZero(&laddr, sizeof(laddr));

#if defined(USE_TCP_LOOPBACK)
        {
            laddr.sin_family      = AF_INET;
            laddr.sin_port        = htons(MDNS_TCP_SERVERPORT);
            laddr.sin_addr.s_addr = inet_addr(MDNS_TCP_SERVERADDR);
            ret                   = bind(listenfd, (struct sockaddr *) &laddr, sizeof(laddr));
            if (ret < 0)
            {
                my_perror("ERROR: bind(listenfd, (struct sockaddr *) &laddr, sizeof(laddr)); failed");
                goto error;
            }
        }
#else
        {
            mode_t mask = umask(0);
            // FIXED unlink(MDNS_UDS_SERVERPATH);  // OK if this fails
            laddr.sun_family = AF_LOCAL;
#ifndef NOT_HAVE_SA_LEN
            // According to Stevens (section 3.2), there is no portable way to
            // determine whether sa_len is defined on a particular platform.
            //            laddr.sun_len = sizeof(struct sockaddr_un);
#endif
#if 0
            if (strlen(MDNS_UDS_SERVERPATH) >= sizeof(laddr.sun_path))
            {
                LogMsg("ERROR: MDNS_UDS_SERVERPATH must be < %d characters", (int)sizeof(laddr.sun_path));
                goto error;
            }
            mDNSPlatformStrCopy(laddr.sun_path, MDNS_UDS_SERVERPATH);
#endif
            ret = bind(listenfd, (struct sockaddr *) &laddr, sizeof(laddr));
            umask(mask);
            if (ret < 0)
            {
                my_perror("ERROR: bind(listenfd, (struct sockaddr *) &laddr, sizeof(laddr)); failed");
                goto error;
            }
        }
#endif

        if (!uds_socket_setup(listenfd))
            goto error;
    }

#if !defined(PLATFORM_NO_RLIMIT)
    {
        // Set maximum number of open file descriptors
#define MIN_OPENFILES 10240
        //        struct rlimit maxfds, newfds;

        // Due to bugs in OS X (<rdar://problem/2941095>, <rdar://problem/3342704>, <rdar://problem/3839173>)
        // you have to get and set rlimits once before getrlimit will return sensible values
#if 0
        if (getrlimit(RLIMIT_NOFILE, &maxfds) < 0) { my_perror("ERROR: Unable to get file descriptor limit"); return 0; }
        if (setrlimit(RLIMIT_NOFILE, &maxfds) < 0) my_perror("ERROR: Unable to set maximum file descriptor limit");

        if (getrlimit(RLIMIT_NOFILE, &maxfds) < 0) { my_perror("ERROR: Unable to get file descriptor limit"); return 0; }
        newfds.rlim_max = (maxfds.rlim_max > MIN_OPENFILES) ? maxfds.rlim_max : MIN_OPENFILES;
        newfds.rlim_cur = (maxfds.rlim_cur > MIN_OPENFILES) ? maxfds.rlim_cur : MIN_OPENFILES;
        if (newfds.rlim_max != maxfds.rlim_max || newfds.rlim_cur != maxfds.rlim_cur)
            if (setrlimit(RLIMIT_NOFILE, &newfds) < 0) my_perror("ERROR: Unable to set maximum file descriptor limit");

        if (getrlimit(RLIMIT_NOFILE, &maxfds) < 0) { my_perror("ERROR: Unable to get file descriptor limit"); return 0; }
        debugf("maxfds.rlim_max %d", (long)maxfds.rlim_max);
        debugf("maxfds.rlim_cur %d", (long)maxfds.rlim_cur);
#endif
    }
#endif

#ifndef REMOVE_DNS_SD_DOMAINS
    // We start a "LocalOnly" query looking for Automatic Browse Domain records.
    // When Domain Enumeration in uDNS.c finds an "lb" record from the network, its "FoundDomain" routine
    // creates a "LocalOnly" record, which results in our AutomaticBrowseDomainChange callback being invoked
    mDNS_GetDomains(&mDNSStorage, &mDNSStorage.AutomaticBrowseDomainQ, mDNS_DomainTypeBrowseAutomatic, mDNSNULL,
                    mDNSInterface_LocalOnly, AutomaticBrowseDomainChange, mDNSNULL);

    // Add "local" as recommended registration domain ("dns-sd -E"), recommended browsing domain ("dns-sd -F"), and automatic
    // browsing domain
    RegisterLocalOnlyDomainEnumPTR(&mDNSStorage, &localdomain, mDNS_DomainTypeRegistration);
    RegisterLocalOnlyDomainEnumPTR(&mDNSStorage, &localdomain, mDNS_DomainTypeBrowse);
    AddAutoBrowseDomain(0, &localdomain);
#endif // #ifndef REMOVE_DNS_SD_DOMAINS

    udsserver_handle_configchange(&mDNSStorage);
    return 0;

error:

    my_perror("ERROR: udsserver_init");
    return -1;
}

mDNSexport int udsserver_exit(void)
{
    // Cancel all outstanding client requests
    while (all_requests)
        AbortUnlinkAndFree(all_requests);

    // Clean up any special mDNSInterface_LocalOnly records we created, both the entries for "local" we
    // created in udsserver_init, and others we created as a result of reading local configuration data
    while (LocalDomainEnumRecords)
    {
        ARListElem * rem       = LocalDomainEnumRecords;
        LocalDomainEnumRecords = LocalDomainEnumRecords->next;
        mDNS_Deregister(&mDNSStorage, &rem->ar);
    }

    // If the launching environment created no listening socket,
    // that means we created it ourselves, so we should clean it up on exit
    if (dnssd_SocketValid(listenfd))
    {
        dnssd_close(listenfd);
#if !defined(USE_TCP_LOOPBACK)
        // Currently, we're unable to remove /var/run/mdnsd because we've changed to userid "nobody"
        // to give up unnecessary privilege, but we need to be root to remove this Unix Domain Socket.
        // It would be nice if we could find a solution to this problem
// FIXED        if (unlink(MDNS_UDS_SERVERPATH))
//             debugf("Unable to remove %s", MDNS_UDS_SERVERPATH);
#endif
    }

    // FIXED    if (PID_FILE[0]) unlink(PID_FILE);

    return 0;
}

#ifndef REMOVE_LOG_FUNCTIONS
mDNSlocal void LogClientInfo(mDNS * const m, request_state * req)
{
    char prefix[16];
    if (req->primary)
        mDNS_snprintf(prefix, sizeof(prefix), " -> ");
    else
        mDNS_snprintf(prefix, sizeof(prefix), "%3d:", req->sd);

    if (!req->terminate)
        LogMsgNoIdent("%s No operation yet on this socket", prefix);
    else if (req->terminate == connection_termination)
    {
        int num_records = 0, num_ops = 0;
        const registered_record_entry * p;
        request_state * r;
        for (p = req->u.reg_recs; p; p = p->next)
            num_records++;
        for (r = req->next; r; r = r->next)
            if (r->primary == req)
                num_ops++;
        LogMsgNoIdent(
            "%s DNSServiceCreateConnection: %d registered record%s, %d kDNSServiceFlagsShareConnection operation%s PID[%d](%s)",
            prefix, num_records, num_records != 1 ? "s" : "", num_ops, num_ops != 1 ? "s" : "", req->process_id, req->pid_name);
        for (p = req->u.reg_recs; p; p = p->next)
            LogMsgNoIdent(" ->  DNSServiceRegisterRecord %3d %s PID[%d](%s)", p->key, ARDisplayString(m, p->rr), req->process_id,
                          req->pid_name);
        for (r = req->next; r; r = r->next)
            if (r->primary == req)
                LogClientInfo(m, r);
    }
#ifndef REMOVE_BROWSE_REQUEST
    else if (req->terminate == regservice_termination_callback)
    {
        service_instance * ptr;
        // char anonstr[256];
        char anonstr[64];
        for (ptr = req->u.servicereg.instances; ptr; ptr = ptr->next)
            LogMsgNoIdent("%s DNSServiceRegister         %##s%s %u/%u PID[%d](%s)",
                          (ptr == req->u.servicereg.instances) ? prefix : "    ", ptr->srs.RR_SRV.resrec.name->c,
                          AnonDataToString(ptr->srs.AnonData, 0, anonstr, sizeof(anonstr)), mDNSVal16(req->u.servicereg.port),
                          SRS_PORT(&ptr->srs), req->process_id, req->pid_name);
    }
    else if (req->terminate == browse_termination_callback)
    {
        browser_t * blist;
        // char anonstr[256];
        char anonstr[64];
        for (blist = req->u.browser.browsers; blist; blist = blist->next)
            LogMsgNoIdent("%s DNSServiceBrowse           %##s%s PID[%d](%s)", (blist == req->u.browser.browsers) ? prefix : "    ",
                          blist->q.qname.c, AnonDataToString(req->u.browser.AnonData, 0, anonstr, sizeof(anonstr)), req->process_id,
                          req->pid_name);
    }
#endif // #ifndef REMOVE_BROWSE_REQUEST
    else if (req->terminate == resolve_termination_callback)
        LogMsgNoIdent("%s DNSServiceResolve          %##s PID[%d](%s)", prefix, req->u.resolve.qsrv.qname.c, req->process_id,
                      req->pid_name);
    else if (req->terminate == queryrecord_termination_callback)
        LogMsgNoIdent("%s DNSServiceQueryRecord      %##s (%s) PID[%d](%s)", prefix, req->u.queryrecord.q.qname.c,
                      DNSTypeName(req->u.queryrecord.q.qtype), req->process_id, req->pid_name);
    else if (req->terminate == enum_termination_callback)
        LogMsgNoIdent("%s DNSServiceEnumerateDomains %##s PID[%d](%s)", prefix, req->u.enumeration.q_all.qname.c, req->process_id,
                      req->pid_name);
#ifndef REMOVE_NAT_FUCTIONALITY
    else if (req->terminate == port_mapping_termination_callback)
        LogMsgNoIdent("%s DNSServiceNATPortMapping   %s%s Int %5d Req %5d Ext %.4a:%5d Req TTL %5d Granted TTL %5d PID[%d](%s)",
                      prefix, req->u.pm.NATinfo.Protocol & NATOp_MapTCP ? "TCP" : "   ",
                      req->u.pm.NATinfo.Protocol & NATOp_MapUDP ? "UDP" : "   ", mDNSVal16(req->u.pm.NATinfo.IntPort),
                      mDNSVal16(req->u.pm.ReqExt), &req->u.pm.NATinfo.ExternalAddress, mDNSVal16(req->u.pm.NATinfo.ExternalPort),
                      req->u.pm.NATinfo.NATLease, req->u.pm.NATinfo.Lifetime, req->process_id, req->pid_name);
#endif
    else if (req->terminate == addrinfo_termination_callback)
        LogMsgNoIdent("%s DNSServiceGetAddrInfo      %s%s %##s PID[%d](%s)", prefix,
                      req->u.addrinfo.protocol & kDNSServiceProtocol_IPv4 ? "v4" : "  ",
                      req->u.addrinfo.protocol & kDNSServiceProtocol_IPv6 ? "v6" : "  ", req->u.addrinfo.q4.qname.c,
                      req->process_id, req->pid_name);
    else
        LogMsgNoIdent("%s Unrecognized operation %p", prefix, req->terminate);
}
mDNSlocal void GetMcastClients(request_state * req)
{
    if (req->terminate == connection_termination)
    {
        int num_records = 0, num_ops = 0;
        const registered_record_entry * p;
        request_state * r;
        for (p = req->u.reg_recs; p; p = p->next)
            num_records++;
        for (r = req->next; r; r = r->next)
            if (r->primary == req)
                num_ops++;
        for (p = req->u.reg_recs; p; p = p->next)
        {
            if (!AuthRecord_uDNS(p->rr))
                n_mrecords++;
        }
        for (r = req->next; r; r = r->next)
            if (r->primary == req)
                GetMcastClients(r);
    }
#ifndef REMOVE_BROWSE_REQUEST
    else if (req->terminate == regservice_termination_callback)
    {
        service_instance * ptr;
        for (ptr = req->u.servicereg.instances; ptr; ptr = ptr->next)
        {
            if (!AuthRecord_uDNS(&ptr->srs.RR_SRV))
                n_mrecords++;
        }
    }
    else if (req->terminate == browse_termination_callback)
    {
        browser_t * blist;
        for (blist = req->u.browser.browsers; blist; blist = blist->next)
        {
            if (mDNSOpaque16IsZero(blist->q.TargetQID))
                n_mquests++;
        }
    }
#endif // #ifndef REMOVE_BROWSE_REQUEST
    else if (req->terminate == resolve_termination_callback)
    {
        if ((mDNSOpaque16IsZero(req->u.resolve.qsrv.TargetQID)) && (req->u.resolve.qsrv.ThisQInterval > 0))
            n_mquests++;
    }
    else if (req->terminate == queryrecord_termination_callback)
    {
        if ((mDNSOpaque16IsZero(req->u.queryrecord.q.TargetQID)) && (req->u.queryrecord.q.ThisQInterval > 0))
            n_mquests++;
    }
    else if (req->terminate == addrinfo_termination_callback)
    {
        if ((mDNSOpaque16IsZero(req->u.addrinfo.q4.TargetQID)) && (req->u.addrinfo.q4.ThisQInterval > 0))
            n_mquests++;
    }
    else
    {
        return;
    }
}

mDNSlocal void LogMcastClientInfo(request_state * req)
{
    if (!req->terminate)
        LogMcastNoIdent("No operation yet on this socket");
    else if (req->terminate == connection_termination)
    {
        int num_records = 0, num_ops = 0;
        const registered_record_entry * p;
        request_state * r;
        for (p = req->u.reg_recs; p; p = p->next)
            num_records++;
        for (r = req->next; r; r = r->next)
            if (r->primary == req)
                num_ops++;
        for (p = req->u.reg_recs; p; p = p->next)
        {
            if (!AuthRecord_uDNS(p->rr))
                LogMcastNoIdent("R: ->  DNSServiceRegisterRecord:  %##s %s PID[%d](%s)", p->rr->resrec.name->c,
                                DNSTypeName(p->rr->resrec.rrtype), req->process_id, req->pid_name, i_mcount++);
        }
        for (r = req->next; r; r = r->next)
            if (r->primary == req)
                LogMcastClientInfo(r);
    }

#ifndef REMOVE_BROWSE_REQUEST
    else if (req->terminate == regservice_termination_callback)
    {
        service_instance * ptr;
        for (ptr = req->u.servicereg.instances; ptr; ptr = ptr->next)
        {
            if (!AuthRecord_uDNS(&ptr->srs.RR_SRV))
                LogMcastNoIdent("R: DNSServiceRegister:  %##s %u/%u PID[%d](%s)", ptr->srs.RR_SRV.resrec.name->c,
                                mDNSVal16(req->u.servicereg.port), SRS_PORT(&ptr->srs), req->process_id, req->pid_name, i_mcount++);
        }
    }
    else if (req->terminate == browse_termination_callback)
    {
        browser_t * blist;
        for (blist = req->u.browser.browsers; blist; blist = blist->next)
        {
            if (mDNSOpaque16IsZero(blist->q.TargetQID))
                LogMcastNoIdent("Q: DNSServiceBrowse  %##s %s PID[%d](%s)", blist->q.qname.c, DNSTypeName(blist->q.qtype),
                                req->process_id, req->pid_name, i_mcount++);
        }
    }
#endif // #ifndef REMOVE_BROWSE_REQUEST
    else if (req->terminate == resolve_termination_callback)
    {
        if ((mDNSOpaque16IsZero(req->u.resolve.qsrv.TargetQID)) && (req->u.resolve.qsrv.ThisQInterval > 0))
            LogMcastNoIdent("Q: DNSServiceResolve  %##s %s PID[%d](%s)", req->u.resolve.qsrv.qname.c,
                            DNSTypeName(req->u.resolve.qsrv.qtype), req->process_id, req->pid_name, i_mcount++);
    }
    else if (req->terminate == queryrecord_termination_callback)
    {
        if ((mDNSOpaque16IsZero(req->u.queryrecord.q.TargetQID)) && (req->u.queryrecord.q.ThisQInterval > 0))
            LogMcastNoIdent("Q: DNSServiceQueryRecord  %##s %s PID[%d](%s)", req->u.queryrecord.q.qname.c,
                            DNSTypeName(req->u.queryrecord.q.qtype), req->process_id, req->pid_name, i_mcount++);
    }
    else if (req->terminate == addrinfo_termination_callback)
    {
        if ((mDNSOpaque16IsZero(req->u.addrinfo.q4.TargetQID)) && (req->u.addrinfo.q4.ThisQInterval > 0))
            LogMcastNoIdent("Q: DNSServiceGetAddrInfo  %s%s %##s PID[%d](%s)",
                            req->u.addrinfo.protocol & kDNSServiceProtocol_IPv4 ? "v4" : "  ",
                            req->u.addrinfo.protocol & kDNSServiceProtocol_IPv6 ? "v6" : "  ", req->u.addrinfo.q4.qname.c,
                            req->process_id, req->pid_name, i_mcount++);
    }
    else
    {
        return;
    }
}
mDNSlocal char * RecordTypeName(mDNSu8 rtype)
{
    switch (rtype)
    {
    case kDNSRecordTypeUnregistered:
        return ("Unregistered ");
    case kDNSRecordTypeDeregistering:
        return ("Deregistering");
    case kDNSRecordTypeUnique:
        return ("Unique       ");
    case kDNSRecordTypeAdvisory:
        return ("Advisory     ");
    case kDNSRecordTypeShared:
        return ("Shared       ");
    case kDNSRecordTypeVerified:
        return ("Verified     ");
    case kDNSRecordTypeKnownUnique:
        return ("KnownUnique  ");
    default:
        return ("Unknown");
    }
}

mDNSlocal void LogEtcHosts(mDNS * const m)
{
    mDNSBool showheader = mDNStrue;
    const AuthRecord * ar;
    mDNSu32 slot;
    AuthGroup * ag;
    int count          = 0;
    int authslot       = 0;
    mDNSBool truncated = 0;

    for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
    {
        if (m->rrauth.rrauth_hash[slot])
            authslot++;
        for (ag = m->rrauth.rrauth_hash[slot]; ag; ag = ag->next)
            for (ar = ag->members; ar; ar = ar->next)
            {
                if (ar->RecordCallback != FreeEtcHosts)
                    continue;
                if (showheader)
                {
                    showheader = mDNSfalse;
                    LogMsgNoIdent("  State       Interface");
                }

                // Print a maximum of 50 records
                if (count++ >= 50)
                {
                    truncated = mDNStrue;
                    continue;
                }
                if (ar->ARType == AuthRecordLocalOnly)
                {
                    if (ar->resrec.InterfaceID == mDNSInterface_LocalOnly)
                        LogMsgNoIdent(" %s   LO %s", RecordTypeName(ar->resrec.RecordType), ARDisplayString(m, ar));
                    else
                    {
                        mDNSu32 scopeid = (mDNSu32) (uintptr_t) ar->resrec.InterfaceID;
                        LogMsgNoIdent(" %s   %u  %s", RecordTypeName(ar->resrec.RecordType), scopeid, ARDisplayString(m, ar));
                    }
                }
            }
    }

    if (showheader)
        LogMsgNoIdent("<None>");
    else if (truncated)
        LogMsgNoIdent("<Truncated: to 50 records, Total records %d, Total Auth Groups %d, Auth Slots %d>", count,
                      m->rrauth.rrauth_totalused, authslot);
}

mDNSlocal void LogLocalOnlyAuthRecords(mDNS * const m)
{
    mDNSBool showheader = mDNStrue;
    const AuthRecord * ar;
    mDNSu32 slot;
    AuthGroup * ag;

    for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
    {
        for (ag = m->rrauth.rrauth_hash[slot]; ag; ag = ag->next)
            for (ar = ag->members; ar; ar = ar->next)
            {
                if (ar->RecordCallback == FreeEtcHosts)
                    continue;
                if (showheader)
                {
                    showheader = mDNSfalse;
                    LogMsgNoIdent("  State       Interface");
                }

                // Print a maximum of 400 records
                if (ar->ARType == AuthRecordLocalOnly)
                    LogMsgNoIdent(" %s   LO %s", RecordTypeName(ar->resrec.RecordType), ARDisplayString(m, ar));
                else if (ar->ARType == AuthRecordP2P)
                    LogMsgNoIdent(" %s   PP %s", RecordTypeName(ar->resrec.RecordType), ARDisplayString(m, ar));
            }
    }

    if (showheader)
        LogMsgNoIdent("<None>");
}

mDNSlocal char * AnonInfoToString(AnonymousInfo * ai, char * anonstr, int anstrlen)
{
    anonstr[0] = 0;
    if (ai && ai->AnonData)
    {
        return (AnonDataToString(ai->AnonData, ai->AnonDataLen, anonstr, anstrlen));
    }
    return anonstr;
}

mDNSlocal void LogOneAuthRecord(mDNS * const m, const AuthRecord * ar, mDNSs32 now, const char * const ifname)
{
    // char anstr[256];
    char anstr[64];
    if (AuthRecord_uDNS(ar))
    {
        LogMsgNoIdent("%7d %7d %7d %7d %s", ar->ThisAPInterval / mDNSPlatformOneSecond,
                      (ar->LastAPTime + ar->ThisAPInterval - now) / mDNSPlatformOneSecond,
                      ar->expire ? (ar->expire - now) / mDNSPlatformOneSecond : 0, ar->state, ARDisplayString(m, ar));
    }
    else
    {
        LogMsgNoIdent("%7d %7d %7d %7s %s%s", ar->ThisAPInterval / mDNSPlatformOneSecond,
                      ar->AnnounceCount ? (ar->LastAPTime + ar->ThisAPInterval - now) / mDNSPlatformOneSecond : 0,
                      ar->TimeExpire ? (ar->TimeExpire - now) / mDNSPlatformOneSecond : 0, ifname ? ifname : "ALL",
                      ARDisplayString(m, ar), AnonInfoToString(ar->resrec.AnonInfo, anstr, sizeof(anstr)));
    }
}

mDNSlocal void LogAuthRecords(mDNS * const m, const mDNSs32 now, AuthRecord * ResourceRecords, int * proxy)
{
    mDNSBool showheader = mDNStrue;
    const AuthRecord * ar;
    OwnerOptData owner = zeroOwner;
    for (ar = ResourceRecords; ar; ar = ar->next)
    {
        const char * const ifname = InterfaceNameForID(m, ar->resrec.InterfaceID);
        if ((ar->WakeUp.HMAC.l[0] != 0) == (proxy != mDNSNULL))
        {
            if (showheader)
            {
                showheader = mDNSfalse;
                LogMsgNoIdent("    Int    Next  Expire   State");
            }
            if (proxy)
                (*proxy)++;
            if (!mDNSPlatformMemSame(&owner, &ar->WakeUp, sizeof(owner)))
            {
                owner = ar->WakeUp;
                if (owner.password.l[0])
                    LogMsgNoIdent("Proxying for H-MAC %.6a I-MAC %.6a Password %.6a seq %d", &owner.HMAC, &owner.IMAC,
                                  &owner.password, owner.seq);
                else if (!mDNSSameEthAddress(&owner.HMAC, &owner.IMAC))
                    LogMsgNoIdent("Proxying for H-MAC %.6a I-MAC %.6a seq %d", &owner.HMAC, &owner.IMAC, owner.seq);
                else
                    LogMsgNoIdent("Proxying for %.6a seq %d", &owner.HMAC, owner.seq);
            }
            if (AuthRecord_uDNS(ar))
            {
                LogOneAuthRecord(m, ar, now, ifname);
            }
            else if (ar->ARType == AuthRecordLocalOnly)
            {
                LogMsgNoIdent("                             LO %s", ARDisplayString(m, ar));
            }
            else if (ar->ARType == AuthRecordP2P)
            {
                LogMsgNoIdent("                             PP %s", ARDisplayString(m, ar));
            }
            else
            {
                LogOneAuthRecord(m, ar, now, ifname);
                if (ar->resrec.AnonInfo)
                {
                    ResourceRecord * nsec3 = ar->resrec.AnonInfo->nsec3RR;
                    // We just print the values from the AuthRecord to keep it nicely aligned though
                    // all we want here is the nsec3 information.
                    LogMsgNoIdent("%7d %7d %7d %7s %s", ar->ThisAPInterval / mDNSPlatformOneSecond,
                                  ar->AnnounceCount ? (ar->LastAPTime + ar->ThisAPInterval - now) / mDNSPlatformOneSecond : 0,
                                  ar->TimeExpire ? (ar->TimeExpire - now) / mDNSPlatformOneSecond : 0, ifname ? ifname : "ALL",
                                  RRDisplayString(m, nsec3));
                }
            }
        }
    }
    if (showheader)
        LogMsgNoIdent("<None>");
}

mDNSlocal void PrintOneCacheRecord(mDNS * const m, const CacheRecord * cr, mDNSu32 slot, const mDNSu32 remain, const char * ifname,
                                   mDNSu32 * CacheUsed)
{
    LogMsgNoIdent("%3d %s%8d %-7s%s %-6s%s", slot, cr->CRActiveQuestion ? "*" : " ", remain, ifname ? ifname : "-U-",
                  (cr->resrec.RecordType == kDNSRecordTypePacketNegative)        ? "-"
                      : (cr->resrec.RecordType & kDNSRecordTypePacketUniqueMask) ? " "
                                                                                 : "+",
                  DNSTypeName(cr->resrec.rrtype), CRDisplayString(m, cr));
    (*CacheUsed)++;
}

mDNSlocal void PrintCachedRecords(mDNS * const m, const CacheRecord * cr, mDNSu32 slot, const mDNSu32 remain, const char * ifname,
                                  mDNSu32 * CacheUsed)
{
    CacheRecord * nsec;
    CacheRecord * soa;
    nsec = cr->nsec;

    // The records that are cached under the main cache record like nsec, soa don't have
    // their own lifetime. If the main cache record expires, they also expire.
    while (nsec)
    {
        PrintOneCacheRecord(m, nsec, slot, remain, ifname, CacheUsed);
        nsec = nsec->next;
    }
    soa = cr->soa;
    if (soa)
    {
        PrintOneCacheRecord(m, soa, slot, remain, ifname, CacheUsed);
    }
    if (cr->resrec.AnonInfo)
    {
        ResourceRecord * nsec3 = cr->resrec.AnonInfo->nsec3RR;
        // Even though it is a resource record, we print the sameway
        // as a cache record so that it aligns properly.
        if (nsec3)
        {
            LogMsgNoIdent("%3d %s%8d %-7s%s %-6s%s", slot, " ", remain, ifname ? ifname : "-U-",
                          (nsec3->RecordType == kDNSRecordTypePacketNegative)        ? "-"
                              : (nsec3->RecordType & kDNSRecordTypePacketUniqueMask) ? " "
                                                                                     : "+",
                          DNSTypeName(nsec3->rrtype), RRDisplayString(m, nsec3));
        }
    }
}
mDNSlocal char * AnonDataToString(const mDNSu8 * ad, int adlen, char * adstr, int adstrlen)
{
    adstr[0] = 0;
    if (ad)
    {
        int len;
        char * orig = adstr;

        // If the caller is lazy to compute the length, we do it for them.
        if (!adlen)
            len = strlen((const char *) ad);
        else
            len = adlen;

        // Print the anondata within brackets. Hence, we need space for two
        // brackets and a NULL byte.
        if (len > (adstrlen - 3))
            len = adstrlen - 3;

        *adstr++ = '(';
        mDNSPlatformMemCopy(adstr, ad, len);
        adstr[len]     = ')';
        adstr[len + 1] = 0;
        return orig;
    }
    return adstr;
}
#ifndef REMOVE_UDSSERVER_INFO
mDNSexport void LogMDNSStatistics(mDNS * const m)
{
    LogMsgNoIdent("--- MDNS Statistics ---");

    LogMsgNoIdent("Name Conflicts                 %u", m->mDNSStats.NameConflicts);
    LogMsgNoIdent("KnownUnique Name Conflicts     %u", m->mDNSStats.KnownUniqueNameConflicts);
    LogMsgNoIdent("Duplicate Query Suppressions   %u", m->mDNSStats.DupQuerySuppressions);
    LogMsgNoIdent("KA Suppressions                %u", m->mDNSStats.KnownAnswerSuppressions);
    LogMsgNoIdent("KA Multiple Packets            %u", m->mDNSStats.KnownAnswerMultiplePkts);
    LogMsgNoIdent("Poof Cache Deletions           %u", m->mDNSStats.PoofCacheDeletions);
    LogMsgNoIdent("--------------------------------");

    LogMsgNoIdent("Multicast packets Sent         %u", m->MulticastPacketsSent);
    LogMsgNoIdent("Multicast packets Received     %u", m->MPktNum);
    LogMsgNoIdent("Remote Subnet packets          %u", m->RemoteSubnet);
    LogMsgNoIdent("QU questions  received         %u", m->mDNSStats.UnicastBitInQueries);
    LogMsgNoIdent("Normal multicast questions     %u", m->mDNSStats.NormalQueries);
    LogMsgNoIdent("Answers for questions          %u", m->mDNSStats.MatchingAnswersForQueries);
    LogMsgNoIdent("Unicast responses              %u", m->mDNSStats.UnicastResponses);
    LogMsgNoIdent("Multicast responses            %u", m->mDNSStats.MulticastResponses);
    LogMsgNoIdent("Unicast response Demotions     %u", m->mDNSStats.UnicastDemotedToMulticast);
    LogMsgNoIdent("--------------------------------");

    LogMsgNoIdent("Sleeps                         %u", m->mDNSStats.Sleeps);
    LogMsgNoIdent("Wakeups                        %u", m->mDNSStats.Wakes);
    LogMsgNoIdent("Interface UP events            %u", m->mDNSStats.InterfaceUp);
    LogMsgNoIdent("Interface UP Flap events       %u", m->mDNSStats.InterfaceUpFlap);
    LogMsgNoIdent("Interface Down events          %u", m->mDNSStats.InterfaceDown);
    LogMsgNoIdent("Interface DownFlap events      %u", m->mDNSStats.InterfaceDownFlap);
    LogMsgNoIdent("Cache refresh queries          %u", m->mDNSStats.CacheRefreshQueries);
    LogMsgNoIdent("Cache refreshed                %u", m->mDNSStats.CacheRefreshed);
    LogMsgNoIdent("Wakeup on Resolves             %u", m->mDNSStats.WakeOnResolves);
}

mDNSexport void udsserver_info(mDNS * const m)
{
    const mDNSs32 now = mDNS_TimeNow(m);
    mDNSu32 CacheUsed = 0, CacheActive = 0, slot;
    int ProxyA = 0, ProxyD = 0;
    const CacheGroup * cg;
    const CacheRecord * cr;
    const DNSQuestion * q;
    const DNameListElem * d;
    const SearchListElem * s;

    LogMsgNoIdent("Timenow 0x%08lX (%d)", (mDNSu32) now, now);

    LogMsgNoIdent("------------ Cache -------------");
    LogMsgNoIdent("Slt Q     TTL if     U Type rdlen");
    for (slot = 0; slot < CACHE_HASH_SLOTS; slot++)
    {
        for (cg = m->rrcache_hash[slot]; cg; cg = cg->next)
        {
            CacheUsed++; // Count one cache entity for the CacheGroup object
            for (cr = cg->members; cr; cr = cr->next)
            {
                const mDNSs32 remain = cr->resrec.rroriginalttl - (now - cr->TimeRcvd) / mDNSPlatformOneSecond;
                const char * ifname;
                mDNSInterfaceID InterfaceID = cr->resrec.InterfaceID;
                if (!InterfaceID && cr->resrec.rDNSServer && cr->resrec.rDNSServer->scoped)
                    InterfaceID = cr->resrec.rDNSServer->interface;
                ifname = InterfaceNameForID(m, InterfaceID);
                if (cr->CRActiveQuestion)
                    CacheActive++;
                PrintOneCacheRecord(m, cr, slot, remain, ifname, &CacheUsed);
                PrintCachedRecords(m, cr, slot, remain, ifname, &CacheUsed);
            }
        }
    }

    if (m->rrcache_totalused != CacheUsed)
        LogMsgNoIdent("Cache use mismatch: rrcache_totalused is %lu, true count %lu", m->rrcache_totalused, CacheUsed);
    if (m->rrcache_active != CacheActive)
        LogMsgNoIdent("Cache use mismatch: rrcache_active is %lu, true count %lu", m->rrcache_active, CacheActive);
    LogMsgNoIdent("Cache currently contains %lu entities; %lu referenced by active questions", CacheUsed, CacheActive);

    LogMsgNoIdent("--------- Auth Records ---------");
    LogAuthRecords(m, now, m->ResourceRecords, mDNSNULL);

    LogMsgNoIdent("--------- LocalOnly, P2P Auth Records ---------");
    LogLocalOnlyAuthRecords(m);

    LogMsgNoIdent("--------- /etc/hosts ---------");
    LogEtcHosts(m);

    LogMsgNoIdent("------ Duplicate Records -------");
    LogAuthRecords(m, now, m->DuplicateRecords, mDNSNULL);

    LogMsgNoIdent("----- Auth Records Proxied -----");
    LogAuthRecords(m, now, m->ResourceRecords, &ProxyA);

    LogMsgNoIdent("-- Duplicate Records Proxied ---");
    LogAuthRecords(m, now, m->DuplicateRecords, &ProxyD);

    LogMsgNoIdent("---------- Questions -----------");
    if (!m->Questions)
        LogMsgNoIdent("<None>");
    else
    {
        // char anonstr[256];
        char anonstr[64];
        CacheUsed   = 0;
        CacheActive = 0;
        LogMsgNoIdent("   Int  Next if     T  NumAns VDNS    Qptr     DupOf    SU SQ Type Name");
        for (q = m->Questions; q; q = q->next)
        {
            mDNSs32 i     = q->ThisQInterval / mDNSPlatformOneSecond;
            mDNSs32 n     = (NextQSendTime(q) - now) / mDNSPlatformOneSecond;
            char * ifname = InterfaceNameForID(m, q->InterfaceID);
            CacheUsed++;
            if (q->ThisQInterval)
                CacheActive++;
            LogMsgNoIdent("%6d%6d %-7s%s%s %5d 0x%x%x 0x%p 0x%p %1d %2d %-5s%##s%s%s", i, n,
                          ifname                                 ? ifname
                              : mDNSOpaque16IsZero(q->TargetQID) ? ""
                                                                 : "-U-",
                          mDNSOpaque16IsZero(q->TargetQID) ? (q->LongLived ? "l" : " ") : (q->LongLived ? "L" : "O"),
                          PrivateQuery(q)             ? "P"
                              : q->ValidationRequired ? "V"
                              : q->ValidatingResponse ? "R"
                                                      : " ",
                          q->CurrentAnswers, q->validDNSServers.l[1], q->validDNSServers.l[0], q, q->DuplicateOf,
                          q->SuppressUnusable, q->SuppressQuery, DNSTypeName(q->qtype), q->qname.c,
                          AnonInfoToString(q->AnonInfo, anonstr, sizeof(anonstr)), q->DuplicateOf ? " (dup)" : "");
        }
        LogMsgNoIdent("%lu question%s; %lu active", CacheUsed, CacheUsed > 1 ? "s" : "", CacheActive);
    }

    LogMsgNoIdent("----- Local-Only Questions -----");
    if (!m->LocalOnlyQuestions)
        LogMsgNoIdent("<None>");
    else
        for (q = m->LocalOnlyQuestions; q; q = q->next)
            LogMsgNoIdent("                       %5d  %-6s%##s%s", q->CurrentAnswers, DNSTypeName(q->qtype), q->qname.c,
                          q->DuplicateOf ? " (dup)" : "");

    LogMsgNoIdent("---- Active UDS Client Requests ----");
    if (!all_requests)
        LogMsgNoIdent("<None>");
    else
    {
        request_state *req, *r;
        for (req = all_requests; req; req = req->next)
        {
            if (req->primary) // If this is a subbordinate operation, check that the parent is in the list
            {
                for (r = all_requests; r && r != req; r = r->next)
                    if (r == req->primary)
                        goto foundparent;
                LogMsgNoIdent("%3d: Orhpan operation %p; parent %p not found in request list", req->sd);
            }
            // For non-subbordinate operations, and subbordinate operations that have lost their parent, write out their info
            LogClientInfo(m, req);
        foundparent:;
        }
    }

    LogMsgNoIdent("-------- NAT Traversals --------");
    LogMsgNoIdent("ExtAddress %.4a Retry %d Interval %d", &m->ExtAddress,
                  m->retryGetAddr ? (m->retryGetAddr - now) / mDNSPlatformOneSecond : 0,
                  m->retryIntervalGetAddr / mDNSPlatformOneSecond);
    if (m->NATTraversals)
    {
        const NATTraversalInfo * nat;
        for (nat = m->NATTraversals; nat; nat = nat->next)
        {
            LogMsgNoIdent("%p %s Int %5d %s Err %d Retry %5d Interval %5d Expire %5d Req %.4a:%d Ext %.4a:%d", nat,
                          nat->Protocol ? (nat->Protocol == NATOp_MapTCP ? "TCP" : "UDP") : "ADD", mDNSVal16(nat->IntPort),
                          (nat->lastSuccessfulProtocol == NATTProtocolNone          ? "None    "
                               : nat->lastSuccessfulProtocol == NATTProtocolNATPMP  ? "NAT-PMP "
                               : nat->lastSuccessfulProtocol == NATTProtocolUPNPIGD ? "UPnP/IGD"
                               : nat->lastSuccessfulProtocol == NATTProtocolPCP     ? "PCP     "
                                                                                    :
                                                                                /* else */ "Unknown "),
                          nat->Result, nat->retryPortMap ? (nat->retryPortMap - now) / mDNSPlatformOneSecond : 0,
                          nat->retryInterval / mDNSPlatformOneSecond,
                          nat->ExpiryTime ? (nat->ExpiryTime - now) / mDNSPlatformOneSecond : 0, &nat->NewAddress,
                          mDNSVal16(nat->RequestedPort), &nat->ExternalAddress, mDNSVal16(nat->ExternalPort));
        }
    }

    LogMsgNoIdent("--------- AuthInfoList ---------");
    if (!m->AuthInfoList)
        LogMsgNoIdent("<None>");
    else
    {
        const DomainAuthInfo * a;
        for (a = m->AuthInfoList; a; a = a->next)
        {
#ifndef REMOVE_TUNNELING
            LogMsgNoIdent("%##s %##s %##s %d %d %.16a%s", a->domain.c, a->keyname.c, a->hostname.c,
                          (a->port.b[0] << 8 | a->port.b[1]), (a->deltime ? (a->deltime - now) : 0), &a->AutoTunnelInnerAddress,
                          a->AutoTunnel ? " AutoTunnel" : "");
#endif // #ifndef REMOVE_TUNNELING
        }
    }

#if APPLE_OSX_mDNSResponder
    LogMsgNoIdent("--------- TunnelClients --------");
    if (!m->TunnelClients)
        LogMsgNoIdent("<None>");
    else
    {
        const ClientTunnel * c;
        for (c = m->TunnelClients; c; c = c->next)
            LogMsgNoIdent("%##s local %.16a %.4a %.16a remote %.16a %.4a %5d %.16a interval %d", c->dstname.c, &c->loc_inner,
                          &c->loc_outer, &c->loc_outer6, &c->rmt_inner, &c->rmt_outer, mDNSVal16(c->rmt_outer_port), &c->rmt_outer6,
                          c->q.ThisQInterval);
    }
#endif // APPLE_OSX_mDNSResponder

    LogMsgNoIdent("---------- Misc State ----------");

    LogMsgNoIdent("PrimaryMAC:   %.6a", &m->PrimaryMAC);

    LogMsgNoIdent("m->SleepState %d (%s) seq %d", m->SleepState,
                  m->SleepState == SleepState_Awake              ? "Awake"
                      : m->SleepState == SleepState_Transferring ? "Transferring"
                      : m->SleepState == SleepState_Sleeping     ? "Sleeping"
                                                                 : "?",
                  m->SleepSeqNum);

    if (!m->SPSSocket)
        LogMsgNoIdent("Not offering Sleep Proxy Service");
#ifndef SPC_DISABLED
    else
        LogMsgNoIdent("Offering Sleep Proxy Service: %#s", m->SPSRecords.RR_SRV.resrec.name->c);
#endif
    if (m->ProxyRecords == ProxyA + ProxyD)
        LogMsgNoIdent("ProxyRecords: %d + %d = %d", ProxyA, ProxyD, ProxyA + ProxyD);
    else
        LogMsgNoIdent("ProxyRecords: MISMATCH %d + %d = %d %d", ProxyA, ProxyD, ProxyA + ProxyD, m->ProxyRecords);

    LogMsgNoIdent("------ Auto Browse Domains -----");
    if (!AutoBrowseDomains)
        LogMsgNoIdent("<None>");
    else
        for (d = AutoBrowseDomains; d; d = d->next)
            LogMsgNoIdent("%##s", d->name.c);

    LogMsgNoIdent("--- Auto Registration Domains --");
    if (!AutoRegistrationDomains)
        LogMsgNoIdent("<None>");
    else
        for (d = AutoRegistrationDomains; d; d = d->next)
            LogMsgNoIdent("%##s", d->name.c);

    LogMsgNoIdent("--- Search Domains --");
    if (!SearchList)
        LogMsgNoIdent("<None>");
    else
    {
        for (s = SearchList; s; s = s->next)
        {
            char * ifname = InterfaceNameForID(m, s->InterfaceID);
            LogMsgNoIdent("%##s %s", s->domain.c, ifname ? ifname : "");
        }
    }
    LogInfo("--- Trust Anchors ---");
    if (!m->TrustAnchors)
    {
        LogInfo("<None>");
    }
    else
    {
        TrustAnchor * ta;
        mDNSu8 fromTimeBuf[64];
        mDNSu8 untilTimeBuf[64];

        for (ta = m->TrustAnchors; ta; ta = ta->next)
        {
            mDNSPlatformFormatTime((unsigned long) ta->validFrom, fromTimeBuf, sizeof(fromTimeBuf));
            mDNSPlatformFormatTime((unsigned long) ta->validUntil, untilTimeBuf, sizeof(untilTimeBuf));
            LogInfo("%##s %d %d %d %d %s %s", ta->zone.c, ta->rds.keyTag, ta->rds.alg, ta->rds.digestType, ta->digestLen,
                    fromTimeBuf, untilTimeBuf);
        }
    }

    LogInfo("--- DNSSEC Statistics ---");

    LogInfo("Next Stats Time                 %u", m->NextStatLogTime - mDNSPlatformUTC());
    LogMsgNoIdent("Unicast Cache size              %u", m->rrcache_totalused_unicast);
    LogInfo("DNSSEC  Cache size              %u", m->DNSSECStats.TotalMemUsed);
    if (m->rrcache_totalused_unicast)
        LogInfo("DNSSEC  usage percentage        %u",
                ((unsigned long) (m->DNSSECStats.TotalMemUsed * 100)) / m->rrcache_totalused_unicast);
    LogInfo("DNSSEC  Extra Packets (0 to 2)  %u", m->DNSSECStats.ExtraPackets0);
    LogInfo("DNSSEC  Extra Packets (3 to 6)  %u", m->DNSSECStats.ExtraPackets3);
    LogInfo("DNSSEC  Extra Packets (7 to 9)  %u", m->DNSSECStats.ExtraPackets7);
    LogInfo("DNSSEC  Extra Packets ( >= 10)  %u", m->DNSSECStats.ExtraPackets10);

    LogInfo("DNSSEC  Latency (0 to 4ms)      %u", m->DNSSECStats.Latency0);
    LogInfo("DNSSEC  Latency (4 to 9ms)      %u", m->DNSSECStats.Latency5);
    LogInfo("DNSSEC  Latency (10 to 19ms)    %u", m->DNSSECStats.Latency10);
    LogInfo("DNSSEC  Latency (20 to 49ms)    %u", m->DNSSECStats.Latency20);
    LogInfo("DNSSEC  Latency (50 to 99ms)    %u", m->DNSSECStats.Latency50);
    LogInfo("DNSSEC  Latency (   >=100ms)    %u", m->DNSSECStats.Latency100);

    LogInfo("DNSSEC  Secure Status           %u", m->DNSSECStats.SecureStatus);
    LogInfo("DNSSEC  Insecure Status         %u", m->DNSSECStats.InsecureStatus);
    LogInfo("DNSSEC  Indeterminate Status    %u", m->DNSSECStats.IndeterminateStatus);
    LogInfo("DNSSEC  Bogus Status            %u", m->DNSSECStats.BogusStatus);
    LogInfo("DNSSEC  NoResponse Status       %u", m->DNSSECStats.NoResponseStatus);
    LogInfo("DNSSEC  Probes sent             %u", m->DNSSECStats.NumProbesSent);
    LogInfo("DNSSEC  Msg Size (<=1024)       %u", m->DNSSECStats.MsgSize0);
    LogInfo("DNSSEC  Msg Size (<=2048)       %u", m->DNSSECStats.MsgSize1);
    LogInfo("DNSSEC  Msg Size (> 2048)       %u", m->DNSSECStats.MsgSize2);

    LogMDNSStatistics(m);

    LogMsgNoIdent("---- Task Scheduling Timers ----");

    if (!m->NewQuestions)
        LogMsgNoIdent("NewQuestion <NONE>");
    else
        LogMsgNoIdent("NewQuestion DelayAnswering %d %d %##s (%s)", m->NewQuestions->DelayAnswering,
                      m->NewQuestions->DelayAnswering - now, m->NewQuestions->qname.c, DNSTypeName(m->NewQuestions->qtype));

    if (!m->NewLocalOnlyQuestions)
        LogMsgNoIdent("NewLocalOnlyQuestions <NONE>");
    else
        LogMsgNoIdent("NewLocalOnlyQuestions %##s (%s)", m->NewLocalOnlyQuestions->qname.c,
                      DNSTypeName(m->NewLocalOnlyQuestions->qtype));

    if (!m->NewLocalRecords)
        LogMsgNoIdent("NewLocalRecords <NONE>");
    else
        LogMsgNoIdent("NewLocalRecords %02X %s", m->NewLocalRecords->resrec.RecordType, ARDisplayString(m, m->NewLocalRecords));

    LogMsgNoIdent("SPSProxyListChanged%s", m->SPSProxyListChanged ? "" : " <NONE>");
    LogMsgNoIdent("LocalRemoveEvents%s", m->LocalRemoveEvents ? "" : " <NONE>");
    LogMsgNoIdent("m->AutoTunnelRelayAddr %.16a", &m->AutoTunnelRelayAddr);
    LogMsgNoIdent("m->WABBrowseQueriesCount %d", m->WABBrowseQueriesCount);
    LogMsgNoIdent("m->WABLBrowseQueriesCount %d", m->WABLBrowseQueriesCount);
    LogMsgNoIdent("m->WABRegQueriesCount %d", m->WABRegQueriesCount);
    LogMsgNoIdent("m->mDNSOppCaching %d", m->mDNSOppCaching);
    LogMsgNoIdent("m->AutoTargetServices %d", m->AutoTargetServices);

#define LogTimer(MSG, T) LogMsgNoIdent(MSG " %08X %11d  %08X %11d", (T), (T), (T) -now, (T) -now)

    LogMsgNoIdent("                         ABS (hex)  ABS (dec)  REL (hex)  REL (dec)");
    LogMsgNoIdent("m->timenow               %08X %11d", now, now);
    LogMsgNoIdent("m->timenow_adjust        %08X %11d", m->timenow_adjust, m->timenow_adjust);
    LogTimer("m->NextScheduledEvent   ", m->NextScheduledEvent);

#ifndef UNICAST_DISABLED
    LogTimer("m->NextuDNSEvent        ", m->NextuDNSEvent);
    LogTimer("m->NextSRVUpdate        ", m->NextSRVUpdate);
    LogTimer("m->NextScheduledNATOp   ", m->NextScheduledNATOp);
    LogTimer("m->retryGetAddr         ", m->retryGetAddr);
#endif

    LogTimer("m->NextCacheCheck       ", m->NextCacheCheck);
    LogTimer("m->NextScheduledSPS     ", m->NextScheduledSPS);
    LogTimer("m->NextScheduledKA      ", m->NextScheduledKA);
    LogTimer("m->NextScheduledSPRetry ", m->NextScheduledSPRetry);
    LogTimer("m->DelaySleep           ", m->DelaySleep);

    LogTimer("m->NextScheduledQuery   ", m->NextScheduledQuery);
    LogTimer("m->NextScheduledProbe   ", m->NextScheduledProbe);
    LogTimer("m->NextScheduledResponse", m->NextScheduledResponse);

    LogTimer("m->SuppressSending      ", m->SuppressSending);
    LogTimer("m->SuppressProbes       ", m->SuppressProbes);
    LogTimer("m->ProbeFailTime        ", m->ProbeFailTime);
    LogTimer("m->DelaySleep           ", m->DelaySleep);
    LogTimer("m->SleepLimit           ", m->SleepLimit);
    LogTimer("m->NextScheduledStopTime ", m->NextScheduledStopTime);
}
#endif // #ifndef REMOVE_UDSSERVER_INFO

#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING
mDNSexport void uds_validatelists(void)
{
    const request_state *req, *p;
    for (req = all_requests; req; req = req->next)
    {
        if (req->next == (request_state *) ~0 || (req->sd < 0 && req->sd != -2))
            LogMemCorruption("UDS request list: %p is garbage (%d)", req, req->sd);

        if (req->primary == req)
            LogMemCorruption("UDS request list: req->primary should not point to self %p/%d", req, req->sd);

        if (req->primary && req->replies)
            LogMemCorruption("UDS request list: Subordinate request %p/%d/%p should not have replies (%p)", req, req->sd,
                             req->primary && req->replies);

        p = req->primary;
        if ((long) p & 3)
            LogMemCorruption("UDS request list: req %p primary %p is misaligned (%d)", req, p, req->sd);
        else if (p && (p->next == (request_state *) ~0 || (p->sd < 0 && p->sd != -2)))
            LogMemCorruption("UDS request list: req %p primary %p is garbage (%d)", req, p, p->sd);

        reply_state * rep;
        for (rep = req->replies; rep; rep = rep->next)
            if (rep->next == (reply_state *) ~0)
                LogMemCorruption("UDS req->replies: %p is garbage", rep);

        if (req->terminate == connection_termination)
        {
            registered_record_entry * r;
            for (r = req->u.reg_recs; r; r = r->next)
                if (r->next == (registered_record_entry *) ~0)
                    LogMemCorruption("UDS req->u.reg_recs: %p is garbage", r);
        }
#ifndef REMOVE_BROWSE_REQUEST
        else if (req->terminate == regservice_termination_callback)
        {
            service_instance * s;
            for (s = req->u.servicereg.instances; s; s = s->next)
                if (s->next == (service_instance *) ~0)
                    LogMemCorruption("UDS req->u.servicereg.instances: %p is garbage", s);
        }
        else if (req->terminate == browse_termination_callback)
        {
            browser_t * b;
            for (b = req->u.browser.browsers; b; b = b->next)
                if (b->next == (browser_t *) ~0)
                    LogMemCorruption("UDS req->u.browser.browsers: %p is garbage", b);
        }
    }

    DNameListElem * d;
    for (d = SCPrefBrowseDomains; d; d = d->next)
        if (d->next == (DNameListElem *) ~0 || d->name.c[0] > 63)
            LogMemCorruption("SCPrefBrowseDomains: %p is garbage (%d)", d, d->name.c[0]);

    ARListElem * b;
    for (b = LocalDomainEnumRecords; b; b = b->next)
        if (b->next == (ARListElem *) ~0 || b->ar.resrec.name->c[0] > 63)
            LogMemCorruption("LocalDomainEnumRecords: %p is garbage (%d)", b, b->ar.resrec.name->c[0]);

    for (d = AutoBrowseDomains; d; d = d->next)
        if (d->next == (DNameListElem *) ~0 || d->name.c[0] > 63)
            LogMemCorruption("AutoBrowseDomains: %p is garbage (%d)", d, d->name.c[0]);

    for (d = AutoRegistrationDomains; d; d = d->next)
        if (d->next == (DNameListElem *) ~0 || d->name.c[0] > 63)
            LogMemCorruption("AutoRegistrationDomains: %p is garbage (%d)", d, d->name.c[0]);
#endif // #ifndef REMOVE_BROWSE_REQUEST
}
#endif // #ifndef REMOVE_LOG_FUNCTIONS

#endif // APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING

mDNSlocal int send_msg(request_state * const req)
{
    reply_state * const rep = req->replies; // Send the first waiting reply
    ssize_t nwriten;
    if (req->no_reply)
        return (t_complete);

    ConvertHeaderBytes(rep->mhdr);
    nwriten = send(req->sd, (char *) &rep->mhdr + rep->nwriten, rep->totallen - rep->nwriten, 0);
    ConvertHeaderBytes(rep->mhdr);

    if (nwriten < 0)
    {
        if (dnssd_errno == dnssd_EINTR || dnssd_errno == dnssd_EWOULDBLOCK)
            nwriten = 0;
        else
        {
#if !defined(PLATFORM_NO_EPIPE)
            if (dnssd_errno == EPIPE)
                return (req->ts = t_terminated);
            else
#endif
            {
                LogMsg("send_msg ERROR: failed to write %d of %d bytes to fd %d errno %d (%s)", rep->totallen - rep->nwriten,
                       rep->totallen, req->sd, dnssd_errno, dnssd_strerror(dnssd_errno));
                return (t_error);
            }
        }
    }
    rep->nwriten += nwriten;
    return (rep->nwriten == rep->totallen) ? t_complete : t_morecoming;
}

mDNSexport mDNSs32 udsserver_idle(mDNSs32 nextevent)
{
    mDNSs32 now          = mDNS_TimeNow(&mDNSStorage);
    request_state ** req = &all_requests;

    while (*req)
    {
        request_state * const r = *req;

        if (r->terminate == resolve_termination_callback)
            if (r->u.resolve.ReportTime && now - r->u.resolve.ReportTime >= 0)
            {
                r->u.resolve.ReportTime = 0;
                LogMsgNoIdent("Client application bug PID[%d](%s) : DNSServiceResolve(%##s) active for over two minutes. "
                              "This places considerable burden on the network.",
                              r->process_id, r->pid_name, r->u.resolve.qsrv.qname.c);
            }

        // Note: Only primary req's have reply lists, not subordinate req's.
        while (r->replies) // Send queued replies
        {
            transfer_state result;
            if (r->replies->next)
                r->replies->rhdr->flags |= dnssd_htonl(kDNSServiceFlagsMoreComing);
            result = send_msg(r); // Returns t_morecoming if buffer full because client is not reading
            if (result == t_complete)
            {
                reply_state * fptr = r->replies;
                r->replies         = r->replies->next;
                free(fptr);
                r->time_blocked             = 0; // reset failure counter after successful send
                r->unresponsiveness_reports = 0;
                continue;
            }
            else if (result == t_terminated || result == t_error)
            {
                LogMsg("%3d: Could not write data to clientPID[%d](%s)  because of error - aborting connection", r->sd,
                       r->process_id, r->pid_name);
#ifndef REMOVE_LOG_FUNCTIONS
                LogClientInfo(&mDNSStorage, r);
#endif // #ifndef REMOVE_LOG_FUNCTIONS
                abort_request(r);
            }
            break;
        }

        if (r->replies) // If we failed to send everything, check our time_blocked timer
        {
            if (nextevent - now > mDNSPlatformOneSecond)
                nextevent = now + mDNSPlatformOneSecond;

            if (mDNSStorage.SleepState != SleepState_Awake)
                r->time_blocked = 0;
            else if (!r->time_blocked)
                r->time_blocked = NonZeroTime(now);
            else if (now - r->time_blocked >= 10 * mDNSPlatformOneSecond * (r->unresponsiveness_reports + 1))
            {
                int num                = 0;
                struct reply_state * x = r->replies;
                while (x)
                {
                    num++;
                    x = x->next;
                }
                LogMsg("%3d: Could not write data to client PID[%d](%s) after %ld seconds, %d repl%s waiting", r->sd, r->process_id,
                       r->pid_name, (now - r->time_blocked) / mDNSPlatformOneSecond, num, num == 1 ? "y" : "ies");
                if (++r->unresponsiveness_reports >= 60)
                {
                    LogMsg("%3d: Client PID[%d](%s) unresponsive; aborting connection", r->sd, r->process_id, r->pid_name);
#ifndef REMOVE_LOG_FUNCTIONS
                    LogClientInfo(&mDNSStorage, r);
#endif //#ifndef REMOVE_LOG_FUNCTIONS
                    abort_request(r);
                }
            }
        }

        if (!dnssd_SocketValid(r->sd)) // If this request is finished, unlink it from the list and free the memory
        {
            // Since we're already doing a list traversal, we unlink the request directly instead of using AbortUnlinkAndFree()
            *req = r->next;
            free(r);
        }
        else
            req = &r->next;
    }
    return nextevent;
}

struct CompileTimeAssertionChecks_uds_daemon
{
    // Check our structures are reasonable sizes. Including overly-large buffers, or embedding
    // other overly-large structures instead of having a pointer to them, can inadvertently
    // cause structure sizes (and therefore memory usage) to balloon unreasonably.
    char sizecheck_request_state[(sizeof(request_state) <= 2000) ? 1 : -1];
    char sizecheck_registered_record_entry[(sizeof(registered_record_entry) <= 60) ? 1 : -1];
    char sizecheck_service_instance[(sizeof(service_instance) <= 6552) ? 1 : -1];
    char sizecheck_browser_t[(sizeof(browser_t) <= 1096) ? 1 : -1];
    char sizecheck_reply_hdr[(sizeof(reply_hdr) <= 12) ? 1 : -1];
    char sizecheck_reply_state[(sizeof(reply_state) <= 64) ? 1 : -1];
};
