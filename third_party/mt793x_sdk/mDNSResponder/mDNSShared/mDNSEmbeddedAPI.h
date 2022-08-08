/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2002-2013 Apple Computer, Inc. All rights reserved.
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

   NOTE:
   If you're building an application that uses DNS Service Discovery
   this is probably NOT the header file you're looking for.
   In most cases you will want to use /usr/include/dns_sd.h instead.

   This header file defines the lowest level raw interface to mDNSCore,
   which is appropriate *only* on tiny embedded systems where everything
   runs in a single address space and memory is extremely constrained.
   All the APIs here are malloc-free, which means that the caller is
   responsible for passing in a pointer to the relevant storage that
   will be used in the execution of that call, and (when called with
   correct parameters) all the calls are guaranteed to succeed. There
   is never a case where a call can suffer intermittent failures because
   the implementation calls malloc() and sometimes malloc() returns NULL
   because memory is so limited that no more is available.
   This is primarily for devices that need to have precisely known fixed
   memory requirements, with absolutely no uncertainty or run-time variation,
   but that certainty comes at a cost of more difficult programming.

   For applications running on general-purpose desktop operating systems
   (Mac OS, Linux, Solaris, Windows, etc.) the API you should use is
   /usr/include/dns_sd.h, which defines the API by which multiple
   independent client processes communicate their DNS Service Discovery
   requests to a single "mdnsd" daemon running in the background.

   Even on platforms that don't run multiple independent processes in
   multiple independent address spaces, you can still use the preferred
   dns_sd.h APIs by linking in "dnssd_clientshim.c", which implements
   the standard "dns_sd.h" API calls, allocates any required storage
   using malloc(), and then calls through to the low-level malloc-free
   mDNSCore routines defined here. This has the benefit that even though
   you're running on a small embedded system with a single address space,
   you can still use the exact same client C code as you'd use on a
   general-purpose desktop system.

 */

#ifndef __mDNSEmbeddedAPI_h
#define __mDNSEmbeddedAPI_h

// For KEIL/IAR IDE
#include "mdns_porting.h"

#if defined(EFI32) || defined(EFI64) || defined(EFIX64)
// EFI doesn't have stdarg.h unless it's building with GCC.
#include "Tiano.h"
#if !defined(__GNUC__)
#define va_list VA_LIST
#define va_start(a, b) VA_START(a, b)
#define va_end(a) VA_END(a)
#define va_arg(a, b) VA_ARG(a, b)
#endif
#else
#include <stdarg.h> // stdarg.h is required for for va_list support for the mDNS_vsnprintf declaration
#endif

#include "mDNSDebug.h"
#if APPLE_OSX_mDNSResponder
#include <uuid/uuid.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

// ***************************************************************************
// Feature removal compile options & limited resource targets

// The following compile options are responsible for removing certain features from mDNSCore to reduce the
// memory footprint for use in embedded systems with limited resources.

// UNICAST_DISABLED - disables unicast DNS functionality, including Wide Area Bonjour
// ANONYMOUS_DISABLED - disables anonymous functionality
// DNSSEC_DISABLED - disables DNSSEC functionality
// SPC_DISABLED - disables Bonjour Sleep Proxy client
// IDLESLEEPCONTROL_DISABLED - disables sleep control for Bonjour Sleep Proxy clients

// In order to disable the above features pass the option to your compiler, e.g. -D UNICAST_DISABLED

// Additionally, the LIMITED_RESOURCES_TARGET compile option will eliminate caching and
// and reduce the maximum DNS message sizes.

#ifdef LIMITED_RESOURCES_TARGET
// Don't support jumbo frames
#define AbsoluteMaxDNSMessageData 1500
// StandardAuthRDSize is 264 (256+8), which is large enough to hold a maximum-sized SRV record (6 + 256 bytes)
#define MaximumRDSize 264
// Don't cache anything
#define AUTH_HASH_SLOTS 1
#define CACHE_HASH_SLOTS 1
#endif
// Memory reduction changes
#define ONLY_ONE_NetWakeResolve
#define REMOVE_RR_HINFO
#define REMOVE_NETWAKE
#ifdef REMOVE_NETWAKE
#define IDLESLEEPCONTROL_DISABLED
#endif
#define REMOVE_NON_PRIVILIGED_MODE
#define REDUCE_RR_CACHE_SIZE
#define REDUCE_AUTH_HASH_SLOTS
#define REMOVE_DNS_SD_DOMAINS
#define REMOVE_mDNS_LoggingEnabled
#define REMOVE_MDNS_TRACING
#define REMOVE_UDSSERVER_INFO
#define REMOVE_MACHINE_SLEEP
#define REMOVE_BROWSE_REQUEST
#define REMOVE_MPRINT
//#define REMOVE_LOG_FUNCTIONS
//#if 0
#define REMOVE_NAT_FUCTIONALITY
#define REMOVE_UNUSED_VARS
#define UNICAST_DISABLED
//#if 0
//#define REMOVE_WAKEUP_FUNCTIONALITY
#define REMOVE_TUNNELING
#if 0
#define REMOVE_QTXT_RECORD_TYPE_SUPPORT // mainly used for email service (source: wikipedia)
#endif
#define REMOVE_UDS_DAEMON_ERROR_MESSAGING
// ***************************************************************************
// Function scope indicators

// If you see "mDNSlocal" before a function name in a C file, it means the function is not callable outside this file
#ifndef mDNSlocal
#define mDNSlocal static
#endif
// If you see "mDNSexport" before a symbol in a C file, it means the symbol is exported for use by clients
// For every "mDNSexport" in a C file, there needs to be a corresponding "extern" declaration in some header file
// (When a C file #includes a header file, the "extern" declarations tell the compiler:
// "This symbol exists -- but not necessarily in this C file.")
#ifndef mDNSexport
#define mDNSexport
#endif

// Explanation: These local/export markers are a little habit of mine for signaling the programmers' intentions.
// When "mDNSlocal" is just a synonym for "static", and "mDNSexport" is a complete no-op, you could be
// forgiven for asking what purpose they serve. The idea is that if you see "mDNSexport" in front of a
// function definition it means the programmer intended it to be exported and callable from other files
// in the project. If you see "mDNSlocal" in front of a function definition it means the programmer
// intended it to be private to that file. If you see neither in front of a function definition it
// means the programmer forgot (so you should work out which it is supposed to be, and fix it).
// Using "mDNSlocal" instead of "static" makes it easier to do a textual searches for one or the other.
// For example you can do a search for "static" to find if any functions declare any local variables as "static"
// (generally a bad idea unless it's also "const", because static storage usually risks being non-thread-safe)
// without the results being cluttered with hundreds of matches for functions declared static.
// - Stuart Cheshire

// ***************************************************************************
// Structure packing macro

// If we're not using GNUC, it's not fatal.
// Most compilers naturally pack the on-the-wire structures correctly anyway, so a plain "struct" is usually fine.
// In the event that structures are not packed correctly, mDNS_Init() will detect this and report an error, so the
// developer will know what's wrong, and can investigate what needs to be done on that compiler to provide proper packing.
#ifndef packedstruct
#if ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 9)))
#define packedstruct struct __attribute__((__packed__))
#define packedunion union __attribute__((__packed__))
#else
#define packedstruct struct
#define packedunion union
#endif
#endif

// ***************************************************************************
#if 0
#pragma mark - DNS Resource Record class and type constants
#endif

typedef enum // From RFC 1035
{
    kDNSClass_IN   = 1,   // Internet
    kDNSClass_CS   = 2,   // CSNET
    kDNSClass_CH   = 3,   // CHAOS
    kDNSClass_HS   = 4,   // Hesiod
    kDNSClass_NONE = 254, // Used in DNS UPDATE [RFC 2136]

    kDNSClass_Mask        = 0x7FFF, // Multicast DNS uses the bottom 15 bits to identify the record class...
    kDNSClass_UniqueRRSet = 0x8000, // ... and the top bit indicates that all other cached records are now invalid

    kDNSQClass_ANY             = 255,   // Not a DNS class, but a DNS query class, meaning "all classes"
    kDNSQClass_UnicastResponse = 0x8000 // Top bit set in a question means "unicast response acceptable"
} DNS_ClassValues;

typedef enum // From RFC 1035
{
    kDNSType_A = 1,      //  1 Address
    kDNSType_NS,         //  2 Name Server
    kDNSType_MD,         //  3 Mail Destination
    kDNSType_MF,         //  4 Mail Forwarder
    kDNSType_CNAME,      //  5 Canonical Name
    kDNSType_SOA,        //  6 Start of Authority
    kDNSType_MB,         //  7 Mailbox
    kDNSType_MG,         //  8 Mail Group
    kDNSType_MR,         //  9 Mail Rename
    kDNSType_NULL,       // 10 NULL RR
    kDNSType_WKS,        // 11 Well-known-service
    kDNSType_PTR,        // 12 Domain name pointer
    kDNSType_HINFO,      // 13 Host information
    kDNSType_MINFO,      // 14 Mailbox information
    kDNSType_MX,         // 15 Mail Exchanger
    kDNSType_TXT,        // 16 Arbitrary text string
    kDNSType_RP,         // 17 Responsible person
    kDNSType_AFSDB,      // 18 AFS cell database
    kDNSType_X25,        // 19 X_25 calling address
    kDNSType_ISDN,       // 20 ISDN calling address
    kDNSType_RT,         // 21 Router
    kDNSType_NSAP,       // 22 NSAP address
    kDNSType_NSAP_PTR,   // 23 Reverse NSAP lookup (deprecated)
    kDNSType_SIG,        // 24 Security signature
    kDNSType_KEY,        // 25 Security key
    kDNSType_PX,         // 26 X.400 mail mapping
    kDNSType_GPOS,       // 27 Geographical position (withdrawn)
    kDNSType_AAAA,       // 28 IPv6 Address
    kDNSType_LOC,        // 29 Location Information
    kDNSType_NXT,        // 30 Next domain (security)
    kDNSType_EID,        // 31 Endpoint identifier
    kDNSType_NIMLOC,     // 32 Nimrod Locator
    kDNSType_SRV,        // 33 Service record
    kDNSType_ATMA,       // 34 ATM Address
    kDNSType_NAPTR,      // 35 Naming Authority PoinTeR
    kDNSType_KX,         // 36 Key Exchange
    kDNSType_CERT,       // 37 Certification record
    kDNSType_A6,         // 38 IPv6 Address (deprecated)
    kDNSType_DNAME,      // 39 Non-terminal DNAME (for IPv6)
    kDNSType_SINK,       // 40 Kitchen sink (experimental)
    kDNSType_OPT,        // 41 EDNS0 option (meta-RR)
    kDNSType_APL,        // 42 Address Prefix List
    kDNSType_DS,         // 43 Delegation Signer
    kDNSType_SSHFP,      // 44 SSH Key Fingerprint
    kDNSType_IPSECKEY,   // 45 IPSECKEY
    kDNSType_RRSIG,      // 46 RRSIG
    kDNSType_NSEC,       // 47 Denial of Existence
    kDNSType_DNSKEY,     // 48 DNSKEY
    kDNSType_DHCID,      // 49 DHCP Client Identifier
    kDNSType_NSEC3,      // 50 Hashed Authenticated Denial of Existence
    kDNSType_NSEC3PARAM, // 51 Hashed Authenticated Denial of Existence

    kDNSType_HIP = 55, // 55 Host Identity Protocol

    kDNSType_SPF = 99, // 99 Sender Policy Framework for E-Mail
    kDNSType_UINFO,    // 100 IANA-Reserved
    kDNSType_UID,      // 101 IANA-Reserved
    kDNSType_GID,      // 102 IANA-Reserved
    kDNSType_UNSPEC,   // 103 IANA-Reserved

    kDNSType_TKEY = 249, // 249 Transaction key
    kDNSType_TSIG,       // 250 Transaction signature
    kDNSType_IXFR,       // 251 Incremental zone transfer
    kDNSType_AXFR,       // 252 Transfer zone of authority
    kDNSType_MAILB,      // 253 Transfer mailbox records
    kDNSType_MAILA,      // 254 Transfer mail agent records
    kDNSQType_ANY        // Not a DNS type, but a DNS query type, meaning "all types"
} DNS_TypeValues;

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Simple types
#endif

// mDNS defines its own names for these common types to simplify portability across
// multiple platforms that may each have their own (different) names for these types.
typedef unsigned char mDNSBool;
typedef signed char mDNSs8;
typedef unsigned char mDNSu8;
typedef signed short mDNSs16;
typedef unsigned short mDNSu16;

// Source: http://www.unix.org/version2/whatsnew/lp64_wp.html
// http://software.intel.com/sites/products/documentation/hpc/mkl/lin/MKL_UG_structure/Support_for_ILP64_Programming.htm
// It can be safely assumed that int is 32bits on the platform
#if defined(_ILP64) || defined(__ILP64__)
typedef signed int32 mDNSs32;
typedef unsigned int32 mDNSu32;
#else
typedef signed int mDNSs32;
typedef unsigned int mDNSu32;
#endif

// To enforce useful type checking, we make mDNSInterfaceID be a pointer to a dummy struct
// This way, mDNSInterfaceIDs can be assigned, and compared with each other, but not with other types
// Declaring the type to be the typical generic "void *" would lack this type checking
typedef struct mDNSInterfaceID_dummystruct
{
    void * dummy;
} * mDNSInterfaceID;

// These types are for opaque two- and four-byte identifiers.
// The "NotAnInteger" fields of the unions allow the value to be conveniently passed around in a
// register for the sake of efficiency, and compared for equality or inequality, but don't forget --
// just because it is in a register doesn't mean it is an integer. Operations like greater than,
// less than, add, multiply, increment, decrement, etc., are undefined for opaque identifiers,
// and if you make the mistake of trying to do those using the NotAnInteger field, then you'll
// find you get code that doesn't work consistently on big-endian and little-endian machines.
#if defined(_WIN32)
#pragma pack(push, 2)
#endif
typedef union
{
    mDNSu8 b[2];
    mDNSu16 NotAnInteger;
} mDNSOpaque16;
typedef union
{
    mDNSu8 b[4];
    mDNSu32 NotAnInteger;
} mDNSOpaque32;
typedef packedunion
{
    mDNSu8 b[6];
    mDNSu16 w[3];
    mDNSu32 l[1];
}
mDNSOpaque48;
typedef union
{
    mDNSu8 b[8];
    mDNSu16 w[4];
    mDNSu32 l[2];
} mDNSOpaque64;
typedef union
{
    mDNSu8 b[16];
    mDNSu16 w[8];
    mDNSu32 l[4];
} mDNSOpaque128;
#if defined(_WIN32)
#pragma pack(pop)
#endif

typedef mDNSOpaque16 mDNSIPPort;  // An IP port is a two-byte opaque identifier (not an integer)
typedef mDNSOpaque32 mDNSv4Addr;  // An IP address is a four-byte opaque identifier (not an integer)
typedef mDNSOpaque128 mDNSv6Addr; // An IPv6 address is a 16-byte opaque identifier (not an integer)
typedef mDNSOpaque48 mDNSEthAddr; // An Ethernet address is a six-byte opaque identifier (not an integer)

// Bit operations for opaque 64 bit quantity. Uses the 32 bit quantity(l[2]) to set and clear bits
#define mDNSNBBY 8
#define bit_set_opaque64(op64, index)                                                                                              \
    (op64.l[((index)) / (sizeof(mDNSu32) * mDNSNBBY)] |= (1 << ((index) % (sizeof(mDNSu32) * mDNSNBBY))))
#define bit_clr_opaque64(op64, index)                                                                                              \
    (op64.l[((index)) / (sizeof(mDNSu32) * mDNSNBBY)] &= ~(1 << ((index) % (sizeof(mDNSu32) * mDNSNBBY))))
#define bit_get_opaque64(op64, index)                                                                                              \
    (op64.l[((index)) / (sizeof(mDNSu32) * mDNSNBBY)] & (1 << ((index) % (sizeof(mDNSu32) * mDNSNBBY))))

enum
{
    mDNSAddrType_None    = 0,
    mDNSAddrType_IPv4    = 4,
    mDNSAddrType_IPv6    = 6,
    mDNSAddrType_Unknown = ~0 // Special marker value used in known answer list recording
};

enum
{
    mDNSTransport_None = 0,
    mDNSTransport_UDP  = 1,
    mDNSTransport_TCP  = 2
};

typedef struct
{
    mDNSs32 type;
    union
    {
        mDNSv6Addr v6;
        mDNSv4Addr v4;
    } ip;
} mDNSAddr;

enum
{
    mDNSfalse = 0,
    mDNStrue  = 1
};

#define mDNSNULL 0L

enum
{
    mStatus_Waiting = 1,
    mStatus_NoError = 0,

    // mDNS return values are in the range FFFE FF00 (-65792) to FFFE FFFF (-65537)
    // The top end of the range (FFFE FFFF) is used for error codes;
    // the bottom end of the range (FFFE FF00) is used for non-error values;

    // Error codes:
    mStatus_UnknownErr        = -65537, // First value: 0xFFFE FFFF
    mStatus_NoSuchNameErr     = -65538,
    mStatus_NoMemoryErr       = -65539,
    mStatus_BadParamErr       = -65540,
    mStatus_BadReferenceErr   = -65541,
    mStatus_BadStateErr       = -65542,
    mStatus_BadFlagsErr       = -65543,
    mStatus_UnsupportedErr    = -65544,
    mStatus_NotInitializedErr = -65545,
    mStatus_NoCache           = -65546,
    mStatus_AlreadyRegistered = -65547,
    mStatus_NameConflict      = -65548,
    mStatus_Invalid           = -65549,
    mStatus_Firewall          = -65550,
    mStatus_Incompatible      = -65551,
    mStatus_BadInterfaceErr   = -65552,
    mStatus_Refused           = -65553,
    mStatus_NoSuchRecord      = -65554,
    mStatus_NoAuth            = -65555,
    mStatus_NoSuchKey         = -65556,
#ifndef REMOVE_NAT_FUCTIONALITY
    mStatus_NATTraversal = -65557,
    mStatus_DoubleNAT    = -65558,
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    mStatus_BadTime      = -65559,
    mStatus_BadSig       = -65560, // while we define this per RFC 2845, BIND 9 returns Refused for bad/missing signatures
    mStatus_BadKey       = -65561,
    mStatus_TransientErr = -65562, // transient failures, e.g. sending packets shortly after a network transition or wake from sleep
    mStatus_ServiceNotRunning = -65563, // Background daemon not running
#ifndef REMOVE_NAT_FUCTIONALITY
    mStatus_NATPortMappingUnsupported = -65564, // NAT doesn't support PCP, NAT-PMP or UPnP
    mStatus_NATPortMappingDisabled    = -65565, // NAT supports PCP, NAT-PMP or UPnP, but it's disabled by the administrator
#endif                                          //#ifndef REMOVE_NAT_FUCTIONALITY
    mStatus_NoRouter    = -65566,
    mStatus_PollingMode = -65567,
    mStatus_Timeout     = -65568,
    // -65568 to -65786 currently unused; available for allocation

    // tcp connection status
    mStatus_ConnPending     = -65787,
    mStatus_ConnFailed      = -65788,
    mStatus_ConnEstablished = -65789,

    // Non-error values:
    mStatus_GrowCache     = -65790,
    mStatus_ConfigChanged = -65791,
    mStatus_MemFree       = -65792 // Last value: 0xFFFE FF00
                                   // mStatus_MemFree is the last legal mDNS error code, at the end of the range allocated for mDNS
};

typedef mDNSs32 mStatus;
#define MaxIp 5 // Needs to be consistent with MaxInputIf in dns_services.h

typedef enum
{
    q_stop = 0,
    q_start
} q_state;
typedef enum
{
    reg_stop = 0,
    reg_start
} reg_state;

// RFC 1034/1035 specify that a domain label consists of a length byte plus up to 63 characters
#define MAX_DOMAIN_LABEL 63
//#define MAX_DOMAIN_LABEL 15
typedef struct
{
    mDNSu8 c[64];
} domainlabel; // One label: length byte and up to 63 characters
// typedef struct { mDNSu8 c[ 16]; } domainlabel;      // One label: length byte and up to 63 characters

// RFC 1034/1035/2181 specify that a domain name (length bytes and data bytes) may be up to 255 bytes long,
// plus the terminating zero at the end makes 256 bytes total in the on-the-wire format.
#define MAX_DOMAIN_NAME 256
//#define MAX_DOMAIN_NAME 64
typedef struct
{
    mDNSu8 c[256];
} domainname; // Up to 256 bytes of length-prefixed domainlabels
// typedef struct { mDNSu8 c[64]; } domainname;       // Up to 256 bytes of length-prefixed domainlabels

typedef struct
{
    mDNSu8 c[64];
} UTF8str255; // Null-terminated C string

// The longest legal textual form of a DNS name is 1009 bytes, including the C-string terminating NULL at the end.
// Explanation:
// When a native domainname object is converted to printable textual form using ConvertDomainNameToCString(),
// non-printing characters are represented in the conventional DNS way, as '\ddd', where ddd is a three-digit decimal number.
// The longest legal domain name is 256 bytes, in the form of four labels as shown below:
// Length byte, 63 data bytes, length byte, 63 data bytes, length byte, 63 data bytes, length byte, 62 data bytes, zero byte.
// Each label is encoded textually as characters followed by a trailing dot.
// If every character has to be represented as a four-byte escape sequence, then this makes the maximum textual form four labels
// plus the C-string terminating NULL as shown below:
// 63*4+1 + 63*4+1 + 63*4+1 + 62*4+1 + 1 = 1009.
// Note that MAX_ESCAPED_DOMAIN_LABEL is not normally used: If you're only decoding a single label, escaping is usually not
// required. It is for domain names, where dots are used as label separators, that proper escaping is vital.
#define MAX_ESCAPED_DOMAIN_LABEL 254
//#define MAX_ESCAPED_DOMAIN_LABEL 62
#define MAX_ESCAPED_DOMAIN_NAME 1009
//#define MAX_ESCAPED_DOMAIN_NAME 256

// MAX_REVERSE_MAPPING_NAME
// For IPv4: "123.123.123.123.in-addr.arpa."  30 bytes including terminating NUL
// For IPv6: "x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.x.ip6.arpa."  74 bytes including terminating NUL

#define MAX_REVERSE_MAPPING_NAME_V4 30
#define MAX_REVERSE_MAPPING_NAME_V6 74
#define MAX_REVERSE_MAPPING_NAME 74

// Most records have a TTL of 75 minutes, so that their 80% cache-renewal query occurs once per hour.
// For records containing a hostname (in the name on the left, or in the rdata on the right),
// like A, AAAA, reverse-mapping PTR, and SRV, we use a two-minute TTL by default, because we don't want
// them to hang around for too long in the cache if the host in question crashes or otherwise goes away.

#define kStandardTTL (3600UL * 100 / 80)
#define kHostNameTTL 120UL

// Some applications want to register their SRV records with a lower ttl so that in case the server
// using a dynamic port number restarts, the clients will not have stale information for more than
// 10 seconds

#define kHostNameSmallTTL 10UL

// Multicast DNS uses announcements (gratuitous responses) to update peer caches.
// This means it is feasible to use relatively larger TTL values than we might otherwise
// use, because we have a cache coherency protocol to keep the peer caches up to date.
// With Unicast DNS, once an authoritative server gives a record with a certain TTL value to a client
// or caching server, that client or caching server is entitled to hold onto the record until its TTL
// expires, and has no obligation to contact the authoritative server again until that time arrives.
// This means that whereas Multicast DNS can use announcements to pre-emptively update stale data
// before it would otherwise have expired, standard Unicast DNS (not using LLQs) has no equivalent
// mechanism, and TTL expiry is the *only* mechanism by which stale data gets deleted. Because of this,
// we currently limit the TTL to ten seconds in such cases where no dynamic cache updating is possible.
#define kStaticCacheTTL 10

#define DefaultTTLforRRType(X) (((X) == kDNSType_A || (X) == kDNSType_AAAA || (X) == kDNSType_SRV) ? kHostNameTTL : kStandardTTL)
#define mDNS_KeepaliveRecord(rr)                                                                                                   \
    ((rr)->rrtype == kDNSType_NULL && SameDomainLabel(SecondLabel((rr)->name)->c, (mDNSu8 *) "\x0A_keepalive"))

// Number of times keepalives are sent if no ACK is received before waking up the system
// this is analogous to net.inet.tcp.keepcnt
#define kKeepaliveRetryCount 10
// The frequency at which keepalives are retried if no ACK is received
#define kKeepaliveRetryInterval 30

typedef struct AuthRecord_struct AuthRecord;
typedef struct ServiceRecordSet_struct ServiceRecordSet;
typedef struct CacheRecord_struct CacheRecord;
typedef struct CacheGroup_struct CacheGroup;
typedef struct AuthGroup_struct AuthGroup;
typedef struct DNSQuestion_struct DNSQuestion;
typedef struct ZoneData_struct ZoneData;
typedef struct mDNS_struct mDNS;
typedef struct mDNS_PlatformSupport_struct mDNS_PlatformSupport;
#ifndef REMOVE_NAT_FUCTIONALITY
typedef struct NATTraversalInfo_struct NATTraversalInfo;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
typedef struct ResourceRecord_struct ResourceRecord;

// Structure to abstract away the differences between TCP/SSL sockets, and one for UDP sockets
// The actual definition of these structures appear in the appropriate platform support code
typedef struct TCPSocket_struct TCPSocket;
typedef struct UDPSocket_struct UDPSocket;

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - DNS Message structures
#endif

#define mDNS_numZones numQuestions
#define mDNS_numPrereqs numAnswers
#define mDNS_numUpdates numAuthorities

typedef packedstruct
{
    mDNSOpaque16 id;
    mDNSOpaque16 flags;
    mDNSu16 numQuestions;
    mDNSu16 numAnswers;
    mDNSu16 numAuthorities;
    mDNSu16 numAdditionals;
}
DNSMessageHeader;

// We can send and receive packets up to 9000 bytes (Ethernet Jumbo Frame size, if that ever becomes widely used)
// However, in the normal case we try to limit packets to 1500 bytes so that we don't get IP fragmentation on standard Ethernet
// 40 (IPv6 header) + 8 (UDP header) + 12 (DNS message header) + 1440 (DNS message body) = 1500 total
#ifndef AbsoluteMaxDNSMessageData
//#define AbsoluteMaxDNSMessageData 8940
#define AbsoluteMaxDNSMessageData 2940
#endif
#define NormalMaxDNSMessageData 1440
typedef packedstruct
{
    DNSMessageHeader h;                     // Note: Size 12 bytes
    mDNSu8 data[AbsoluteMaxDNSMessageData]; // 40 (IPv6) + 8 (UDP) + 12 (DNS header) + 8940 (data) = 9000
}
DNSMessage;

typedef struct tcpInfo_t
{
    mDNS * m;
    TCPSocket * sock;
    DNSMessage request;
    int requestLen;
    DNSQuestion * question; // For queries
    AuthRecord * rr;        // For record updates
    mDNSAddr Addr;
    mDNSIPPort Port;
    mDNSIPPort SrcPort;
    DNSMessage * reply;
    mDNSu16 replylen;
    unsigned long nread;
    int numReplies;
} tcpInfo_t;

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Other Packet Format Structures
#endif

typedef packedstruct
{
    mDNSEthAddr dst;
    mDNSEthAddr src;
    mDNSOpaque16 ethertype;
}
EthernetHeader; // 14 bytes

typedef packedstruct
{
    mDNSOpaque16 hrd;
    mDNSOpaque16 pro;
    mDNSu8 hln;
    mDNSu8 pln;
    mDNSOpaque16 op;
    mDNSEthAddr sha;
    mDNSv4Addr spa;
    mDNSEthAddr tha;
    mDNSv4Addr tpa;
}
ARP_EthIP; // 28 bytes

typedef packedstruct
{
    mDNSu8 vlen;
    mDNSu8 tos;
    mDNSu16 totlen;
    mDNSOpaque16 id;
    mDNSOpaque16 flagsfrags;
    mDNSu8 ttl;
    mDNSu8 protocol; // Payload type: 0x06 = TCP, 0x11 = UDP
    mDNSu16 checksum;
    mDNSv4Addr src;
    mDNSv4Addr dst;
}
IPv4Header; // 20 bytes

typedef packedstruct
{
    mDNSu32 vcf; // Version, Traffic Class, Flow Label
    mDNSu16 len; // Payload Length
    mDNSu8 pro;  // Type of next header: 0x06 = TCP, 0x11 = UDP, 0x3A = ICMPv6
    mDNSu8 ttl;  // Hop Limit
    mDNSv6Addr src;
    mDNSv6Addr dst;
}
IPv6Header; // 40 bytes

typedef packedstruct
{
    mDNSv6Addr src;
    mDNSv6Addr dst;
    mDNSOpaque32 len;
    mDNSOpaque32 pro;
}
IPv6PseudoHeader; // 40 bytes

typedef union
{
    mDNSu8 bytes[20];
    ARP_EthIP arp;
    IPv4Header v4;
    IPv6Header v6;
} NetworkLayerPacket;

typedef packedstruct
{
    mDNSIPPort src;
    mDNSIPPort dst;
    mDNSu32 seq;
    mDNSu32 ack;
    mDNSu8 offset;
    mDNSu8 flags;
    mDNSu16 window;
    mDNSu16 checksum;
    mDNSu16 urgent;
}
TCPHeader; // 20 bytes; IP protocol type 0x06

typedef struct
{
    mDNSInterfaceID IntfId;
    mDNSu32 seq;
    mDNSu32 ack;
    mDNSu16 window;
} mDNSTCPInfo;

typedef packedstruct
{
    mDNSIPPort src;
    mDNSIPPort dst;
    mDNSu16 len; // Length including UDP header (i.e. minimum value is 8 bytes)
    mDNSu16 checksum;
}
UDPHeader; // 8 bytes; IP protocol type 0x11

typedef packedstruct
{
    mDNSu8 type; // 0x87 == Neighbor Solicitation, 0x88 == Neighbor Advertisement
    mDNSu8 code;
    mDNSu16 checksum;
    mDNSu32 flags_res; // R/S/O flags and reserved bits
    mDNSv6Addr target;
    // Typically 8 bytes of options are also present
}
IPv6NDP; // 24 bytes or more; IP protocol type 0x3A

typedef struct
{
    mDNSAddr ipaddr;
    char ethaddr[18];
} IPAddressMACMapping;

#define NDP_Sol 0x87
#define NDP_Adv 0x88

#define NDP_Router 0x80
#define NDP_Solicited 0x40
#define NDP_Override 0x20

#define NDP_SrcLL 1
#define NDP_TgtLL 2

typedef union
{
    mDNSu8 bytes[20];
    TCPHeader tcp;
    UDPHeader udp;
    IPv6NDP ndp;
} TransportLayerPacket;

typedef packedstruct
{
    mDNSOpaque64 InitiatorCookie;
    mDNSOpaque64 ResponderCookie;
    mDNSu8 NextPayload;
    mDNSu8 Version;
    mDNSu8 ExchangeType;
    mDNSu8 Flags;
    mDNSOpaque32 MessageID;
    mDNSu32 Length;
}
IKEHeader; // 28 bytes

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Resource Record structures
#endif

// Authoritative Resource Records:
// There are four basic types: Shared, Advisory, Unique, Known Unique

// * Shared Resource Records do not have to be unique
// -- Shared Resource Records are used for DNS-SD service PTRs
// -- It is okay for several hosts to have RRs with the same name but different RDATA
// -- We use a random delay on responses to reduce collisions when all the hosts respond to the same query
// -- These RRs typically have moderately high TTLs (e.g. one hour)
// -- These records are announced on startup and topology changes for the benefit of passive listeners
// -- These records send a goodbye packet when deregistering
//
// * Advisory Resource Records are like Shared Resource Records, except they don't send a goodbye packet
//
// * Unique Resource Records should be unique among hosts within any given mDNS scope
// -- The majority of Resource Records are of this type
// -- If two entities on the network have RRs with the same name but different RDATA, this is a conflict
// -- Responses may be sent immediately, because only one host should be responding to any particular query
// -- These RRs typically have low TTLs (e.g. a few minutes)
// -- On startup and after topology changes, a host issues queries to verify uniqueness

// * Known Unique Resource Records are treated like Unique Resource Records, except that mDNS does
// not have to verify their uniqueness because this is already known by other means (e.g. the RR name
// is derived from the host's IP or Ethernet address, which is already known to be a unique identifier).

// Summary of properties of different record types:
// Probe?    Does this record type send probes before announcing?
// Conflict? Does this record type react if we observe an apparent conflict?
// Goodbye?  Does this record type send a goodbye packet on departure?
//
//               Probe? Conflict? Goodbye? Notes
// Unregistered                            Should not appear in any list (sanity check value)
// Shared         No      No       Yes     e.g. Service PTR record
// Deregistering  No      No       Yes     Shared record about to announce its departure and leave the list
// Advisory       No      No       No
// Unique         Yes     Yes      No      Record intended to be unique -- will probe to verify
// Verified       Yes     Yes      No      Record has completed probing, and is verified unique
// KnownUnique    No      Yes      No      Record is assumed by other means to be unique

// Valid lifecycle of a record:
// Unregistered ->                   Shared      -> Deregistering -(goodbye)-> Unregistered
// Unregistered ->                   Advisory                               -> Unregistered
// Unregistered -> Unique -(probe)-> Verified                               -> Unregistered
// Unregistered ->                   KnownUnique                            -> Unregistered

// Each Authoritative kDNSRecordType has only one bit set. This makes it easy to quickly see if a record
// is one of a particular set of types simply by performing the appropriate bitwise masking operation.

// Cache Resource Records (received from the network):
// There are four basic types: Answer, Unique Answer, Additional, Unique Additional
// Bit 7 (the top bit) of kDNSRecordType is always set for Cache Resource Records; always clear for Authoritative Resource Records
// Bit 6 (value 0x40) is set for answer records; clear for authority/additional records
// Bit 5 (value 0x20) is set for records received with the kDNSClass_UniqueRRSet

enum
{
    kDNSRecordTypeUnregistered  = 0x00, // Not currently in any list
    kDNSRecordTypeDeregistering = 0x01, // Shared record about to announce its departure and leave the list

    kDNSRecordTypeUnique = 0x02, // Will become a kDNSRecordTypeVerified when probing is complete

    kDNSRecordTypeAdvisory = 0x04, // Like Shared, but no goodbye packet
    kDNSRecordTypeShared   = 0x08, // Shared means record name does not have to be unique -- use random delay on responses

    kDNSRecordTypeVerified    = 0x10, // Unique means mDNS should check that name is unique (and then send immediate responses)
    kDNSRecordTypeKnownUnique = 0x20, // Known Unique means mDNS can assume name is unique without checking
                                      // For Dynamic Update records, Known Unique means the record must already exist on the server.
    kDNSRecordTypeUniqueMask       = (kDNSRecordTypeUnique | kDNSRecordTypeVerified | kDNSRecordTypeKnownUnique),
    kDNSRecordTypeActiveSharedMask = (kDNSRecordTypeAdvisory | kDNSRecordTypeShared),
    kDNSRecordTypeActiveUniqueMask = (kDNSRecordTypeVerified | kDNSRecordTypeKnownUnique),
    kDNSRecordTypeActiveMask       = (kDNSRecordTypeActiveSharedMask | kDNSRecordTypeActiveUniqueMask),

    kDNSRecordTypePacketAdd        = 0x80, // Received in the Additional  Section of a DNS Response
    kDNSRecordTypePacketAddUnique  = 0x90, // Received in the Additional  Section of a DNS Response with kDNSClass_UniqueRRSet set
    kDNSRecordTypePacketAuth       = 0xA0, // Received in the Authorities Section of a DNS Response
    kDNSRecordTypePacketAuthUnique = 0xB0, // Received in the Authorities Section of a DNS Response with kDNSClass_UniqueRRSet set
    kDNSRecordTypePacketAns        = 0xC0, // Received in the Answer      Section of a DNS Response
    kDNSRecordTypePacketAnsUnique  = 0xD0, // Received in the Answer      Section of a DNS Response with kDNSClass_UniqueRRSet set

    kDNSRecordTypePacketNegative = 0xF0, // Pseudo-RR generated to cache non-existence results like NXDomain

    kDNSRecordTypePacketUniqueMask =
        0x10 // True for PacketAddUnique, PacketAnsUnique, PacketAuthUnique, kDNSRecordTypePacketNegative
};

typedef packedstruct
{
    mDNSu16 priority;
    mDNSu16 weight;
    mDNSIPPort port;
    domainname target;
}
rdataSRV;
typedef packedstruct
{
    mDNSu16 preference;
    domainname exchange;
}
rdataMX;
typedef packedstruct
{
    domainname mbox;
    domainname txt;
}
rdataRP;
typedef packedstruct
{
    mDNSu16 preference;
    domainname map822;
    domainname mapx400;
}
rdataPX;

typedef packedstruct
{
    domainname mname;
    domainname rname;
    mDNSs32 serial;  // Modular counter; increases when zone changes
    mDNSu32 refresh; // Time in seconds that a slave waits after successful replication of the database before it attempts
                     // replication again
    mDNSu32
        retry; // Time in seconds that a slave waits after an unsuccessful replication attempt before it attempts replication again
    mDNSu32 expire; // Time in seconds that a slave holds on to old data while replication attempts remain unsuccessful
    mDNSu32 min;    // Nominally the minimum record TTL for this zone, in seconds; also used for negative caching.
}
rdataSOA;

// http://www.iana.org/assignments/dns-sec-alg-numbers/dns-sec-alg-numbers.xhtml
// Algorithm used for RRSIG, DS and DNS KEY
#define CRYPTO_RSA_SHA1 0x05
#define CRYPTO_DSA_NSEC3_SHA1 0x06
#define CRYPTO_RSA_NSEC3_SHA1 0x07
#define CRYPTO_RSA_SHA256 0x08
#define CRYPTO_RSA_SHA512 0x0A

#define CRYPTO_ALG_MAX 0x0B

// alg - same as in RRSIG, DNS KEY or DS.
// RFC 4034 defines SHA1
// RFC 4509 defines SHA256
// Note: NSEC3 also uses 1 for SHA1 and hence we will reuse for now till a new
// value is assigned.
//
#define SHA1_DIGEST_TYPE 1
#define SHA256_DIGEST_TYPE 2
#define DIGEST_TYPE_MAX 3

// We need support for base64 and base32 encoding for displaying KEY, NSEC3
// To make this platform agnostic, we define two types which the platform
// needs to support
#define ENC_BASE32 1
#define ENC_BASE64 2
#define ENC_ALG_MAX 3

#define DS_FIXED_SIZE 4
typedef packedstruct
{
    mDNSu16 keyTag;
    mDNSu8 alg;
    mDNSu8 digestType;
    mDNSu8 * digest;
}
rdataDS;

typedef struct TrustAnchor
{
    struct TrustAnchor * next;
    int digestLen;
    mDNSu32 validFrom;
    mDNSu32 validUntil;
    domainname zone;
    rdataDS rds;
} TrustAnchor;

// size of rdataRRSIG excluding signerName and signature (which are variable fields)
#define RRSIG_FIXED_SIZE 18
typedef packedstruct
{
    mDNSu16 typeCovered;
    mDNSu8 alg;
    mDNSu8 labels;
    mDNSu32 origTTL;
    mDNSu32 sigExpireTime;
    mDNSu32 sigInceptTime;
    mDNSu16 keyTag;
    mDNSu8 * signerName;
    // mDNSu8 *signature
}
rdataRRSig;

// RFC 4034: For DNS Key RR
// flags - the valid value for DNSSEC is 256 (Zone signing key - ZSK) and 257 (Secure Entry Point) which also
// includes the ZSK bit
//
#define DNSKEY_ZONE_SIGN_KEY 0x100
#define DNSKEY_SECURE_ENTRY_POINT 0x101

// proto - the only valid value for protocol is 3 (See RFC 4034)
#define DNSKEY_VALID_PROTO_VALUE 0x003

// alg - The only mandatory algorithm that we support is RSA/SHA-1
// DNSSEC_RSA_SHA1_ALG

#define DNSKEY_FIXED_SIZE 4
typedef packedstruct
{
    mDNSu16 flags;
    mDNSu8 proto;
    mDNSu8 alg;
    mDNSu8 * data;
}
rdataDNSKey;

#define NSEC3_FIXED_SIZE 5
#define NSEC3_FLAGS_OPTOUT 1
#define NSEC3_MAX_ITERATIONS 2500
typedef packedstruct
{
    mDNSu8 alg;
    mDNSu8 flags;
    mDNSu16 iterations;
    mDNSu8 saltLength;
    mDNSu8 * salt;
    // hashLength, nxt, bitmap
}
rdataNSEC3;

// In the multicast usage of NSEC3, we know the actual size of RData
// 4 bytes : HashAlg, Flags,Iterations
// 5 bytes : Salt Length 1 byte, Salt 4 bytes
// 21 bytes : HashLength 1 byte, Hash 20 bytes
// 34 bytes : Window number, Bitmap length, Type bit map to include the first 256 types
#define MCAST_NSEC3_RDLENGTH (4 + 5 + 21 + 34)
#define SHA1_HASH_LENGTH 20

// Base32 encoding takes 5 bytes of the input and encodes as 8 bytes of output.
// For example, SHA-1 hash of 20 bytes will be encoded as 20/5 * 8 = 32 base32
// bytes. For a max domain name size of 255 bytes of base32 encoding : (255/8)*5
// is the max hash length possible.
#define NSEC3_MAX_HASH_LEN 155
// In NSEC3, the names are hashed and stored in the first label and hence cannot exceed label
// size.
#define NSEC3_MAX_B32_LEN MAX_DOMAIN_LABEL

// We define it here instead of dnssec.h so that these values can be used
// in files without bringing in all of dnssec.h unnecessarily.
typedef enum
{
    DNSSEC_Secure = 1,    // Securely validated and has a chain up to the trust anchor
    DNSSEC_Insecure,      // Cannot build a chain up to the trust anchor
    DNSSEC_Indeterminate, // Not used currently
    DNSSEC_Bogus,         // failed to validate signatures
    DNSSEC_NoResponse     // No DNSSEC records to start with
} DNSSECStatus;

#define DNSSECRecordType(rrtype)                                                                                                   \
    (((rrtype) == kDNSType_RRSIG) || ((rrtype) == kDNSType_NSEC) || ((rrtype) == kDNSType_DNSKEY) || ((rrtype) == kDNSType_DS) ||  \
     ((rrtype) == kDNSType_NSEC3))

typedef enum
{
    platform_OSX = 1, // OSX Platform
    platform_iOS,     // iOS Platform
    platform_Atv,     // Atv Platform
    platform_NonApple // Non-Apple (Windows, POSIX) Platform
} Platform_t;

// EDNS Option Code registrations are recorded in the "DNS EDNS0 Options" section of
// <http://www.iana.org/assignments/dns-parameters>

#define kDNSOpt_LLQ 1
#define kDNSOpt_Lease 2
#define kDNSOpt_NSID 3
#define kDNSOpt_Owner 4
#define kDNSOpt_Trace 65001 // 65001-65534 Reserved for Local/Experimental Use

typedef struct
{
    mDNSu16 vers;
    mDNSu16 llqOp;
    mDNSu16 err; // Or UDP reply port, in setup request
    // Note: In the in-memory form, there's typically a two-byte space here, so that the following 64-bit id is word-aligned
    mDNSOpaque64 id;
    mDNSu32 llqlease;
} LLQOptData;

typedef struct
{
    mDNSu8 vers;           // Version number of this Owner OPT record
    mDNSs8 seq;            // Sleep/wake epoch
    mDNSEthAddr HMAC;      // Host's primary identifier (e.g. MAC of on-board Ethernet)
    mDNSEthAddr IMAC;      // Interface's MAC address (if different to primary MAC)
    mDNSOpaque48 password; // Optional password
} OwnerOptData;

typedef struct
{
    mDNSu8 platf;  // Running platform (see enum Platform_t)
    mDNSu32 mDNSv; // mDNSResponder Version (DNS_SD_H defined in dns_sd.h)
} TracerOptData;

// Note: rdataOPT format may be repeated an arbitrary number of times in a single resource record
typedef packedstruct
{
    mDNSu16 opt;
    mDNSu16 optlen;
    union
    {
        LLQOptData llq;
        mDNSu32 updatelease;
        OwnerOptData owner;
        TracerOptData tracer;
    } u;
}
rdataOPT;

// Space needed to put OPT records into a packet:
// Header         11  bytes (name 1, type 2, class 2, TTL 4, length 2)
// LLQ   rdata    18  bytes (opt 2, len 2, vers 2, op 2, err 2, id 8, lease 4)
// Lease rdata     8  bytes (opt 2, len 2, lease 4)
// Owner rdata 12-24  bytes (opt 2, len 2, owner 8-20)
// Trace rdata     9  bytes (opt 2, len 2, platf 1, mDNSv 4)

#define DNSOpt_Header_Space 11
#define DNSOpt_LLQData_Space (4 + 2 + 2 + 2 + 8 + 4)
#define DNSOpt_LeaseData_Space (4 + 4)
#define DNSOpt_OwnerData_ID_Space (4 + 2 + 6)
#define DNSOpt_OwnerData_ID_Wake_Space (4 + 2 + 6 + 6)
#define DNSOpt_OwnerData_ID_Wake_PW4_Space (4 + 2 + 6 + 6 + 4)
#define DNSOpt_OwnerData_ID_Wake_PW6_Space (4 + 2 + 6 + 6 + 6)
#define DNSOpt_TraceData_Space (4 + 1 + 4)

#define ValidOwnerLength(X)                                                                                                        \
    ((X) == DNSOpt_OwnerData_ID_Space - 4 || (X) == DNSOpt_OwnerData_ID_Wake_Space - 4 ||                                          \
     (X) == DNSOpt_OwnerData_ID_Wake_PW4_Space - 4 || (X) == DNSOpt_OwnerData_ID_Wake_PW6_Space - 4)

#define DNSOpt_Owner_Space(A, B) (mDNSSameEthAddress((A), (B)) ? DNSOpt_OwnerData_ID_Space : DNSOpt_OwnerData_ID_Wake_Space)

#define DNSOpt_Data_Space(O)                                                                                                       \
    ((O)->opt == kDNSOpt_LLQ         ? DNSOpt_LLQData_Space                                                                        \
         : (O)->opt == kDNSOpt_Lease ? DNSOpt_LeaseData_Space                                                                      \
         : (O)->opt == kDNSOpt_Trace ? DNSOpt_TraceData_Space                                                                      \
         : (O)->opt == kDNSOpt_Owner ? DNSOpt_Owner_Space(&(O)->u.owner.HMAC, &(O)->u.owner.IMAC)                                  \
                                     : 0x10000)

// NSEC record is defined in RFC 4034.
// 16 bit RRTYPE space is split into 256 windows and each window has 256 bits (32 bytes).
// If we create a structure for NSEC, it's size would be:
//
//   256 bytes domainname 'nextname'
// + 256 * 34 = 8704 bytes of bitmap data
// = 8960 bytes total
//
// This would be a waste, as types about 256 are not very common. But it would be odd, if we receive
// a type above 256 (.US zone had TYPE65534 when this code was written) and not able to handle it.
// Hence, we handle any size by not fixing a strucure in place. The following is just a placeholder
// and never used anywhere.
//
#define NSEC_MCAST_WINDOW_SIZE 32
typedef struct
{
    domainname * next; // placeholders are uncommented because C89 in Windows requires that a struct has at least a member.
    char bitmap[32];
} rdataNSEC;

// StandardAuthRDSize is 264 (256+8), which is large enough to hold a maximum-sized SRV record (6 + 256 bytes)
// MaximumRDSize is 8K the absolute maximum we support (at least for now)
#define StandardAuthRDSize 264
//#define StandardAuthRDSize 72
#ifndef MaximumRDSize
#define MaximumRDSize 8192
//#define MaximumRDSize 256
#endif

// InlineCacheRDSize is 68
// Records received from the network with rdata this size or less have their rdata stored right in the CacheRecord object
// Records received from the network with rdata larger than this have additional storage allocated for the rdata
// A quick unscientific sample from a busy network at Apple with lots of machines revealed this:
// 1461 records in cache
// 292 were one-byte TXT records
// 136 were four-byte A records
// 184 were sixteen-byte AAAA records
// 780 were various PTR, TXT and SRV records from 12-64 bytes
// Only 69 records had rdata bigger than 64 bytes
// Note that since CacheRecord object and a CacheGroup object are allocated out of the same pool, it's sensible to
// have them both be the same size. Making one smaller without making the other smaller won't actually save any memory.
#define InlineCacheRDSize 68

// The RDataBody union defines the common rdata types that fit into our 264-byte limit
typedef union
{
    mDNSu8 data[StandardAuthRDSize];
    mDNSv4Addr ipv4; // For 'A' record
    domainname name; // For PTR, NS, CNAME, DNAME
    UTF8str255 txt;
    rdataMX mx;
    mDNSv6Addr ipv6; // For 'AAAA' record
    rdataSRV srv;
    rdataOPT opt[2]; // For EDNS0 OPT record; RDataBody may contain multiple variable-length rdataOPT objects packed together
} RDataBody;

// The RDataBody2 union is the same as above, except it includes fields for the larger types like soa, rp, px
typedef union
{
    mDNSu8 data[StandardAuthRDSize];
    mDNSv4Addr ipv4; // For 'A' record
    domainname name; // For PTR, NS, CNAME, DNAME
    rdataSOA soa;    // This is large; not included in the normal RDataBody definition
    UTF8str255 txt;
    rdataMX mx;
    rdataRP rp;      // This is large; not included in the normal RDataBody definition
    rdataPX px;      // This is large; not included in the normal RDataBody definition
    mDNSv6Addr ipv6; // For 'AAAA' record
    rdataSRV srv;
    rdataOPT opt[2]; // For EDNS0 OPT record; RDataBody may contain multiple variable-length rdataOPT objects packed together
    rdataDS ds;
    rdataDNSKey key;
    rdataRRSig rrsig;
} RDataBody2;

typedef struct
{
    mDNSu16 MaxRDLength; // Amount of storage allocated for rdata (usually sizeof(RDataBody))
    mDNSu16 padding;     // So that RDataBody is aligned on 32-bit boundary
    RDataBody u;
} RData;

// sizeofRDataHeader should be 4 bytes
#define sizeofRDataHeader (sizeof(RData) - sizeof(RDataBody))

// RData_small is a smaller version of the RData object, used for inline data storage embedded in a CacheRecord_struct
typedef struct
{
    mDNSu16
        MaxRDLength; // Storage allocated for data (may be greater than InlineCacheRDSize if additional storage follows this object)
    mDNSu16 padding; // So that data is aligned on 32-bit boundary
    mDNSu8 data[InlineCacheRDSize];
} RData_small;

// Note: Within an mDNSRecordCallback mDNS all API calls are legal except mDNS_Init(), mDNS_Exit(), mDNS_Execute()
typedef void mDNSRecordCallback(mDNS * const m, AuthRecord * const rr, mStatus result);

// Note:
// Restrictions: An mDNSRecordUpdateCallback may not make any mDNS API calls.
// The intent of this callback is to allow the client to free memory, if necessary.
// The internal data structures of the mDNS code may not be in a state where mDNS API calls may be made safely.
typedef void mDNSRecordUpdateCallback(mDNS * const m, AuthRecord * const rr, RData * OldRData, mDNSu16 OldRDLen);

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - NAT Traversal structures and constants
#endif

#ifndef REMOVE_NAT_FUCTIONALITY

#define NATMAP_MAX_RETRY_INTERVAL ((mDNSPlatformOneSecond * 60) * 15) // Max retry interval is 15 minutes
#define NATMAP_MIN_RETRY_INTERVAL (mDNSPlatformOneSecond * 2)         // Min retry interval is 2 seconds
#define NATMAP_INIT_RETRY (mDNSPlatformOneSecond / 4)                 // start at 250ms w/ exponential decay
#define NATMAP_DEFAULT_LEASE (60 * 60 * 2)                            // 2 hour lease life in seconds
#define NATMAP_VERS 0

typedef enum
{
    NATOp_AddrRequest = 0,
    NATOp_MapUDP      = 1,
    NATOp_MapTCP      = 2,

    NATOp_AddrResponse   = 0x80 | 0,
    NATOp_MapUDPResponse = 0x80 | 1,
    NATOp_MapTCPResponse = 0x80 | 2,
} NATOp_t;

enum
{
    NATErr_None    = 0,
    NATErr_Vers    = 1,
    NATErr_Refused = 2,
    NATErr_NetFail = 3,
    NATErr_Res     = 4,
    NATErr_Opcode  = 5
};

typedef mDNSu16 NATErr_t;

typedef packedstruct
{
    mDNSu8 vers;
    mDNSu8 opcode;
}
NATAddrRequest;

typedef packedstruct
{
    mDNSu8 vers;
    mDNSu8 opcode;
    mDNSu16 err;
    mDNSu32 upseconds; // Time since last NAT engine reboot, in seconds
    mDNSv4Addr ExtAddr;
}
NATAddrReply;

typedef packedstruct
{
    mDNSu8 vers;
    mDNSu8 opcode;
    mDNSOpaque16 unused;
    mDNSIPPort intport;
    mDNSIPPort extport;
    mDNSu32 NATReq_lease;
}
NATPortMapRequest;

typedef packedstruct
{
    mDNSu8 vers;
    mDNSu8 opcode;
    mDNSu16 err;
    mDNSu32 upseconds; // Time since last NAT engine reboot, in seconds
    mDNSIPPort intport;
    mDNSIPPort extport;
    mDNSu32 NATRep_lease;
}
NATPortMapReply;

// PCP Support for IPv4 mappings

#define PCP_VERS 0x02
#define PCP_WAITSECS_AFTER_EPOCH_INVALID 5

typedef enum
{
    PCPOp_Announce = 0,
    PCPOp_Map      = 1
} PCPOp_t;

typedef enum
{
    PCPProto_All = 0,
    PCPProto_TCP = 6,
    PCPProto_UDP = 17
} PCPProto_t;

typedef enum
{
    PCPResult_Success         = 0,
    PCPResult_UnsuppVersion   = 1,
    PCPResult_NotAuthorized   = 2,
    PCPResult_MalformedReq    = 3,
    PCPResult_UnsuppOpcode    = 4,
    PCPResult_UnsuppOption    = 5,
    PCPResult_MalformedOption = 6,
    PCPResult_NetworkFailure  = 7,
    PCPResult_NoResources     = 8,
    PCPResult_UnsuppProtocol  = 9,
    PCPResult_UserExQuota     = 10,
    PCPResult_CantProvideExt  = 11,
    PCPResult_AddrMismatch    = 12,
    PCPResult_ExcesRemotePeer = 13
} PCPResult_t;

typedef packedstruct
{
    mDNSu8 version;
    mDNSu8 opCode;
    mDNSOpaque16 reserved;
    mDNSu32 lifetime;
    mDNSv6Addr clientAddr;
    mDNSu32 nonce[3];
    mDNSu8 protocol;
    mDNSu8 reservedMapOp[3];
    mDNSIPPort intPort;
    mDNSIPPort extPort;
    mDNSv6Addr extAddress;
}
PCPMapRequest;

typedef packedstruct
{
    mDNSu8 version;
    mDNSu8 opCode;
    mDNSu8 reserved;
    mDNSu8 result;
    mDNSu32 lifetime;
    mDNSu32 epoch;
    mDNSu32 clientAddrParts[3];
    mDNSu32 nonce[3];
    mDNSu8 protocol;
    mDNSu8 reservedMapOp[3];
    mDNSIPPort intPort;
    mDNSIPPort extPort;
    mDNSv6Addr extAddress;
}
PCPMapReply;

// LNT Support

typedef enum
{
    LNTDiscoveryOp     = 1,
    LNTExternalAddrOp  = 2,
    LNTPortMapOp       = 3,
    LNTPortMapDeleteOp = 4
} LNTOp_t;

#define LNT_MAXBUFSIZE 8192
//#define LNT_MAXBUFSIZE 256
typedef struct tcpLNTInfo_struct tcpLNTInfo;
struct tcpLNTInfo_struct
{
    tcpLNTInfo * next;
    mDNS * m;
    NATTraversalInfo * parentNATInfo; // pointer back to the parent NATTraversalInfo
    TCPSocket * sock;
    LNTOp_t op;       // operation performed using this connection
    mDNSAddr Address; // router address
    mDNSIPPort Port;  // router port
    mDNSu8 * Request; // xml request to router
    int requestLen;
    mDNSu8 * Reply; // xml reply from router
    int replyLen;
    unsigned long nread; // number of bytes read so far
    int retries;         // number of times we've tried to do this port mapping
};

typedef void (*NATTraversalClientCallback)(mDNS * m, NATTraversalInfo * n);

// if m->timenow <  ExpiryTime then we have an active mapping, and we'll renew halfway to expiry
// if m->timenow >= ExpiryTime then our mapping has expired, and we're trying to create one

typedef enum
{
    NATTProtocolNone    = 0,
    NATTProtocolNATPMP  = 1,
    NATTProtocolUPNPIGD = 2,
    NATTProtocolPCP     = 3,
} NATTProtocol;

struct NATTraversalInfo_struct
{
    // Internal state fields. These are used internally by mDNSCore; the client layer needn't be concerned with them.
    NATTraversalInfo * next;

    mDNSs32 ExpiryTime;                  // Time this mapping expires, or zero if no mapping
    mDNSs32 retryInterval;               // Current interval, between last packet we sent and the next one
    mDNSs32 retryPortMap;                // If Protocol is nonzero, time to send our next mapping packet
    mStatus NewResult;                   // New error code; will be copied to Result just prior to invoking callback
    NATTProtocol lastSuccessfulProtocol; // To send correct deletion request & update non-PCP external address operations
    mDNSBool sentNATPMP;                 // Whether we just sent a NAT-PMP packet, so we won't send another if
                                         //    we receive another NAT-PMP "Unsupported Version" packet

#ifdef _LEGACY_NAT_TRAVERSAL_
    tcpLNTInfo tcpInfo; // Legacy NAT traversal (UPnP) TCP connection
#endif

    // Result fields: When the callback is invoked these fields contain the answers the client is looking for
    // When the callback is invoked ExternalPort is *usually* set to be the same the same as RequestedPort, except:
    // (a) When we're behind a NAT gateway with port mapping disabled, ExternalPort is reported as zero to
    //     indicate that we don't currently have a working mapping (but RequestedPort retains the external port
    //     we'd like to get, the next time we meet an accomodating NAT gateway willing to give us one).
    // (b) When we have a routable non-RFC1918 address, we don't *need* a port mapping, so ExternalPort
    //     is reported as the same as our InternalPort, since that is effectively our externally-visible port too.
    //     Again, RequestedPort retains the external port we'd like to get the next time we find ourself behind a NAT gateway.
    // To improve stability of port mappings, RequestedPort is updated any time we get a successful
    // mapping response from the PCP, NAT-PMP or UPnP gateway. For example, if we ask for port 80, and
    // get assigned port 81, then thereafter we'll contine asking for port 81.
    mDNSInterfaceID InterfaceID;
    mDNSv4Addr ExternalAddress; // Initially set to onesIPv4Addr, until first callback
    mDNSv4Addr NewAddress;      // May be updated with actual value assigned by gateway
    mDNSIPPort ExternalPort;
    mDNSu32 Lifetime;
    mStatus Result;

    // Client API fields: The client must set up these fields *before* making any NAT traversal API calls
    mDNSu8 Protocol;          // NATOp_MapUDP or NATOp_MapTCP, or zero if just requesting the external IP address
    mDNSIPPort IntPort;       // Client's internal port number (doesn't change)
    mDNSIPPort RequestedPort; // Requested external port; may be updated with actual value assigned by gateway
    mDNSu32 NATLease;         // Requested lifetime in seconds (doesn't change)
    NATTraversalClientCallback clientCallback;
    void * clientContext;
};
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - DNSServer & McastResolver structures and constants
#endif

enum
{
    DNSServer_Untested = 0,
    DNSServer_Passed   = 1,
    DNSServer_Failed   = 2,
    DNSServer_Disabled = 3
};

enum
{
    DNSServer_FlagDelete = 1,
    DNSServer_FlagNew    = 2
};

enum
{
    McastResolver_FlagDelete = 1,
    McastResolver_FlagNew    = 2
};

typedef struct McastResolver
{
    struct McastResolver * next;
    mDNSInterfaceID interface;
    mDNSu32 flags; // Set when we're planning to delete this from the list
    domainname domain;
    mDNSu32 timeout; // timeout value for questions
} McastResolver;

// scoped values for DNSServer matching
enum
{
    kScopeNone        = 0, // DNS server used by unscoped questions
    kScopeInterfaceID = 1, // Scoped DNS server used only by scoped questions
    kScopeServiceID   = 2  // Service specific DNS server used only by questions
                           // have a matching serviceID
};

// Note: DNSSECAware is set if we are able to get a valid response to
// a DNSSEC question. In some cases it is possible that the proxy
// strips the EDNS0 option and we just get a plain response with no
// signatures. But we still mark DNSSECAware in that case. As DNSSECAware
// is only used to determine whether DNSSEC_VALIDATION_SECURE_OPTIONAL
// should be turned off or not, it is sufficient that we are getting
// responses back.
typedef struct DNSServer
{
    struct DNSServer * next;
    mDNSInterfaceID interface; // DNS requests should be sent on this interface
    mDNSs32 serviceID;
    mDNSAddr addr;
    mDNSIPPort port;
    mDNSOpaque16 testid;
    mDNSu32 flags;        // Set when we're planning to delete this from the list
    mDNSu32 teststate;    // Have we sent bug-detection query to this server?
    mDNSs32 lasttest;     // Time we sent last bug-detection query to this server
    domainname domain;    // name->server matching for "split dns"
    mDNSs32 penaltyTime;  // amount of time this server is penalized
    mDNSu32 scoped;       // See the scoped enum above
    mDNSu32 timeout;      // timeout value for questions
    mDNSBool cellIntf;    // Resolver from Cellular Interface ?
    mDNSu16 resGroupID;   // ID of the resolver group that contains this DNSServer
    mDNSBool req_A;       // If set, send v4 query (DNSConfig allows A queries)
    mDNSBool req_AAAA;    // If set, send v6 query (DNSConfig allows AAAA queries)
    mDNSBool req_DO;      // If set, okay to send DNSSEC queries (EDNS DO bit is supported)
    mDNSBool retransDO;   // Total Retransmissions for queries sent with DO option
    mDNSBool DNSSECAware; // set if we are able to receive a response to a request
                          // sent with DO option.
} DNSServer;

typedef struct
{
    mDNSu8 * AnonData;
    int AnonDataLen;
    mDNSu32 salt;
    ResourceRecord * nsec3RR;
    mDNSInterfaceID SendNow; // The interface ID that this record should be sent on
} AnonymousInfo;

struct ResourceRecord_struct
{
    mDNSu8 RecordType; // See enum above
    mDNSu16 rrtype;
    mDNSu16 rrclass;
    mDNSu32 rroriginalttl; // In seconds
    mDNSu16 rdlength;      // Size of the raw rdata, in bytes, in the on-the-wire format
                           // (In-memory storage may be larger, for structures containing 'holes', like SOA)
    mDNSu16 rdestimate;    // Upper bound on on-the-wire size of rdata after name compression
    mDNSu32 namehash;      // Name-based (i.e. case-insensitive) hash of name
    mDNSu32 rdatahash;     // For rdata containing domain name (e.g. PTR, SRV, CNAME etc.), case-insensitive name hash
                           // else, for all other rdata, 32-bit hash of the raw rdata
                           // Note: This requirement is important. Various routines like AddAdditionalsToResponseList(),
                           // ReconfirmAntecedents(), etc., use rdatahash as a pre-flight check to see
                           // whether it's worth doing a full SameDomainName() call. If the rdatahash
                           // is not a correct case-insensitive name hash, they'll get false negatives.

    // Grouping pointers together at the end of the structure improves the memory layout efficiency
    mDNSInterfaceID InterfaceID; // Set if this RR is specific to one interface
                                 // For records received off the wire, InterfaceID is *always* set to the receiving interface
                                 // For our authoritative records, InterfaceID is usually zero, except for those few records
                                 // that are interface-specific (e.g. address records, especially linklocal addresses)
    const domainname * name;
    RData * rdata;            // Pointer to storage for this rdata
    DNSServer * rDNSServer;   // Unicast DNS server authoritative for this entry;null for multicast
    AnonymousInfo * AnonInfo; // Anonymous Information
};

// Unless otherwise noted, states may apply to either independent record registrations or service registrations
typedef enum
{
    regState_Zero         = 0,
    regState_Pending      = 1, // update sent, reply not received
    regState_Registered   = 2, // update sent, reply received
    regState_DeregPending = 3, // dereg sent, reply not received
    regState_Unregistered = 4, // not in any list
    regState_Refresh      = 5, // outstanding refresh (or target change) message
#ifndef REMOVE_NAT_FUCTIONALITY
    regState_NATMap = 6,        // establishing NAT port mapping
#endif                          //#ifndef REMOVE_NAT_FUCTIONALITY
    regState_UpdatePending = 7, // update in flight as result of mDNS_Update call
    regState_NoTarget      = 8, // SRV Record registration pending registration of hostname
#ifndef REMOVE_NAT_FUCTIONALITY
    regState_NATError = 9 // unable to complete NAT traversal
#endif                    //#ifndef REMOVE_NAT_FUCTIONALITY
} regState_t;

enum
{
    Target_Manual   = 0,
    Target_AutoHost = 1,
#ifndef REMOVE_NAT_FUCTIONALITY
    Target_AutoHostAndNATMAP = 2
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
};

typedef enum
{
    mergeState_Zero      = 0,
    mergeState_DontMerge = 1 // Set on fatal error conditions to disable merging
} mergeState_t;

#define AUTH_GROUP_NAME_SIZE 128
struct AuthGroup_struct // Header object for a list of AuthRecords with the same name
{
    AuthGroup * next;          // Next AuthGroup object in this hash table bucket
    mDNSu32 namehash;          // Name-based (i.e. case insensitive) hash of name
    AuthRecord * members;      // List of CacheRecords with this same name
    AuthRecord ** rrauth_tail; // Tail end of that list
    domainname * name;         // Common name for all AuthRecords in this list
    AuthRecord * NewLocalOnlyRecords;
    mDNSu8 namestorage[AUTH_GROUP_NAME_SIZE];
};

#ifndef AUTH_HASH_SLOTS
#ifdef REDUCE_AUTH_HASH_SLOTS
#define AUTH_HASH_SLOTS 49
#else
#define AUTH_HASH_SLOTS 499
#endif
#endif
#define FORALL_AUTHRECORDS(SLOT, AG, AR)                                                                                           \
    for ((SLOT) = 0; (SLOT) < AUTH_HASH_SLOTS; (SLOT)++)                                                                           \
        for ((AG) = m->rrauth.rrauth_hash[(SLOT)]; (AG); (AG) = (AG)->next)                                                        \
            for ((AR) = (AG)->members; (AR); (AR) = (AR)->next)

typedef union AuthEntity_union AuthEntity;
union AuthEntity_union
{
    AuthEntity * next;
    AuthGroup ag;
};
typedef struct
{
    mDNSu32 rrauth_size;      // Total number of available auth entries
    mDNSu32 rrauth_totalused; // Number of auth entries currently occupied
    mDNSu32 rrauth_report;
    mDNSu8 rrauth_lock; // For debugging: Set at times when these lists may not be modified
    AuthEntity * rrauth_free;
    AuthGroup * rrauth_hash[AUTH_HASH_SLOTS];
} AuthHash;

// AuthRecordAny includes mDNSInterface_Any and interface specific auth records.
typedef enum
{
    AuthRecordAny,                  // registered for *Any, NOT including P2P interfaces
    AuthRecordAnyIncludeP2P,        // registered for *Any, including P2P interfaces
    AuthRecordAnyIncludeAWDL,       // registered for *Any, including AWDL interface
    AuthRecordAnyIncludeAWDLandP2P, // registered for *Any, including AWDL and P2P interfaces
    AuthRecordLocalOnly,
    AuthRecordP2P // discovered over D2D/P2P framework
} AuthRecType;

typedef enum
{
    AuthFlagsWakeOnly = 0x1 // WakeOnly service
} AuthRecordFlags;

struct AuthRecord_struct
{
    // For examples of how to set up this structure for use in mDNS_Register(),
    // see mDNS_AdvertiseInterface() or mDNS_RegisterService().
    // Basically, resrec and persistent metadata need to be set up before calling mDNS_Register().
    // mDNS_SetupResourceRecord() is avaliable as a helper routine to set up most fields to sensible default values for you

    AuthRecord * next; // Next in list; first element of structure for efficiency reasons
    // Field Group 1: Common ResourceRecord fields
    ResourceRecord resrec; // 36 bytes when compiling for 32-bit; 48 when compiling for 64-bit

    // Field Group 2: Persistent metadata for Authoritative Records
    AuthRecord * Additional1; // Recommended additional record to include in response (e.g. SRV for PTR record)
    AuthRecord * Additional2; // Another additional (e.g. TXT for PTR record)
    AuthRecord * DependentOn; // This record depends on another for its uniqueness checking
    AuthRecord * RRSet;       // This unique record is part of an RRSet
    mDNSRecordCallback *
        RecordCallback;      // Callback function to call for state changes, and to free memory asynchronously on deregistration
    void * RecordContext;    // Context parameter for the callback function
    mDNSu8 AutoTarget;       // Set if the target of this record (PTR, CNAME, SRV, etc.) is our host name
    mDNSu8 AllowRemoteQuery; // Set if we allow hosts not on the local link to query this record
    mDNSu8 ForceMCast;       // Set by client to advertise solely via multicast, even for apparently unicast names
    mDNSu8 AuthFlags;

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    OwnerOptData WakeUp;   // WakeUp.HMAC.l[0] nonzero indicates that this is a Sleep Proxy record
#endif                     //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    mDNSAddr AddressProxy; // For reverse-mapping Sleep Proxy PTR records, address in question
    mDNSs32 TimeRcvd;      // In platform time units
    mDNSs32 TimeExpire;    // In platform time units
    AuthRecType ARType;    // LocalOnly, P2P or Normal ?
    mDNSs32 KATimeExpire;  // In platform time units: time to send keepalive packet for the proxy record

    // Field Group 3: Transient state for Authoritative Records
    mDNSu8 Acknowledged;         // Set if we've given the success callback to the client
    mDNSu8 ProbeRestartCount;    // Number of times we have restarted probing
    mDNSu8 ProbeCount;           // Number of probes remaining before this record is valid (kDNSRecordTypeUnique)
    mDNSu8 AnnounceCount;        // Number of announcements remaining (kDNSRecordTypeShared)
    mDNSu8 RequireGoodbye;       // Set if this RR has been announced on the wire and will require a goodbye packet
    mDNSu8 AnsweredLocalQ;       // Set if this AuthRecord has been delivered to any local question (LocalOnly or mDNSInterface_Any)
    mDNSu8 IncludeInProbe;       // Set if this RR is being put into a probe right now
    mDNSu8 ImmedUnicast;         // Set if we may send our response directly via unicast to the requester
    mDNSInterfaceID SendNSECNow; // Set if we need to generate associated NSEC data for this rrname
    mDNSInterfaceID ImmedAnswer; // Someone on this interface issued a query we need to answer (all-ones for all interfaces)
#if MDNS_LOG_ANSWER_SUPPRESSION_TIMES
    mDNSs32 ImmedAnswerMarkTime;
#endif
    mDNSInterfaceID ImmedAdditional; // Hint that we might want to also send this record, just to be helpful
    mDNSInterfaceID SendRNow;        // The interface this query is being sent on right now
    mDNSv4Addr v4Requester;          // Recent v4 query for this record, or all-ones if more than one recent query
    mDNSv6Addr v6Requester;          // Recent v6 query for this record, or all-ones if more than one recent query
    AuthRecord * NextResponse;       // Link to the next element in the chain of responses to generate
    const mDNSu8 * NR_AnswerTo;      // Set if this record was selected by virtue of being a direct answer to a question
    AuthRecord * NR_AdditionalTo;    // Set if this record was selected by virtue of being additional to another
    mDNSs32 ThisAPInterval;          // In platform time units: Current interval for announce/probe
    mDNSs32 LastAPTime;              // In platform time units: Last time we sent announcement/probe
    mDNSs32 LastMCTime;              // Last time we multicast this record (used to guard against packet-storm attacks)
    mDNSInterfaceID LastMCInterface; // Interface this record was multicast on at the time LastMCTime was recorded
    RData * NewRData;                // Set if we are updating this record with new rdata
    mDNSu16 newrdlength;             // ... and the length of the new RData
    mDNSRecordUpdateCallback * UpdateCallback;
    mDNSu32 UpdateCredits;    // Token-bucket rate limiting of excessive updates
    mDNSs32 NextUpdateCredit; // Time next token is added to bucket
    mDNSs32 UpdateBlocked;    // Set if update delaying is in effect

    // Field Group 4: Transient uDNS state for Authoritative Records
    regState_t state;      // Maybe combine this with resrec.RecordType state? Right now it's ambiguous and confusing.
                           // e.g. rr->resrec.RecordType can be kDNSRecordTypeUnregistered,
                           // and rr->state can be regState_Unregistered
                           // What if we find one of those statements is true and the other false? What does that mean?
    mDNSBool uselease;     // dynamic update contains (should contain) lease option
    mDNSs32 expire;        // In platform time units: expiration of lease (-1 for static)
    mDNSBool Private;      // If zone is private, DNS updates may have to be encrypted to prevent eavesdropping
    mDNSOpaque16 updateid; // Identifier to match update request and response -- also used when transferring records to Sleep Proxy
    mDNSOpaque64 updateIntID; // Interface IDs (one bit per interface index)to which updates have been sent
    const domainname * zone;  // the zone that is updated
    ZoneData * nta;
    struct tcpInfo_t * tcp;
#ifndef REMOVE_NAT_FUCTIONALITY
    NATTraversalInfo NATinfo;
#endif                   //#ifndef REMOVE_NAT_FUCTIONALITY
    mDNSBool SRVChanged; // temporarily deregistered service because its SRV target or port changed
    mergeState_t mState; // Unicast Record Registrations merge state
    mDNSu8 refreshCount; // Number of refreshes to the server
    mStatus updateError; // Record update resulted in Error ?

    // uDNS_UpdateRecord support fields
    // Do we really need all these in *addition* to NewRData and newrdlength above?
    void * UpdateContext;  // Context parameter for the update callback function
    mDNSu16 OrigRDLen;     // previously registered, being deleted
    mDNSu16 InFlightRDLen; // currently being registered
    mDNSu16 QueuedRDLen;   // pending operation (re-transmitting if necessary) THEN register the queued update
    RData * OrigRData;
    RData * InFlightRData;
    RData * QueuedRData;

    // Field Group 5: Large data objects go at the end
    domainname namestorage;
    RData rdatastorage; // Normally the storage is right here, except for oversized records
    // rdatastorage MUST be the last thing in the structure -- when using oversized AuthRecords, extra bytes
    // are appended after the end of the AuthRecord, logically augmenting the size of the rdatastorage
    // DO NOT ADD ANY MORE FIELDS HERE
};

// IsLocalDomain alone is not sufficient to determine that a record is mDNS or uDNS. By default domain names within
// the "local" pseudo-TLD (and within the IPv4 and IPv6 link-local reverse mapping domains) are automatically treated
// as mDNS records, but it is also possible to force any record (even those not within one of the inherently local
// domains) to be handled as an mDNS record by setting the ForceMCast flag, or by setting a non-zero InterfaceID.
// For example, the reverse-mapping PTR record created in AdvertiseInterface sets the ForceMCast flag, since it points to
// a dot-local hostname, and therefore it would make no sense to register this record with a wide-area Unicast DNS server.
// The same applies to Sleep Proxy records, which we will answer for when queried via mDNS, but we never want to try
// to register them with a wide-area Unicast DNS server -- and we probably don't have the required credentials anyway.
// Currently we have no concept of a wide-area uDNS record scoped to a particular interface, so if the InterfaceID is
// nonzero we treat this the same as ForceMCast.
// Note: Question_uDNS(Q) is used in *only* one place -- on entry to mDNS_StartQuery_internal, to decide whether to set TargetQID.
// Everywhere else in the code, the determination of whether a question is unicast is made by checking to see if TargetQID is
// nonzero.
#define AuthRecord_uDNS(R) ((R)->resrec.InterfaceID == mDNSInterface_Any && !(R)->ForceMCast && !IsLocalDomain((R)->resrec.name))
#define Question_uDNS(Q)                                                                                                           \
    ((Q)->InterfaceID == mDNSInterface_Unicast || (Q)->ProxyQuestion ||                                                            \
     ((Q)->InterfaceID != mDNSInterface_LocalOnly && (Q)->InterfaceID != mDNSInterface_P2P && !(Q)->ForceMCast &&                  \
      !IsLocalDomain(&(Q)->qname)))

#define RRLocalOnly(rr) ((rr)->ARType == AuthRecordLocalOnly || (rr)->ARType == AuthRecordP2P)

#define RRAny(rr)                                                                                                                  \
    ((rr)->ARType == AuthRecordAny || (rr)->ARType == AuthRecordAnyIncludeP2P || (rr)->ARType == AuthRecordAnyIncludeAWDL ||       \
     (rr)->ARType == AuthRecordAnyIncludeAWDLandP2P)

// Question (A or AAAA) that is suppressed currently because IPv4 or IPv6 address
// is not available locally for A or AAAA question respectively. Also, if the
// query is disallowed for the "pid" that we are sending on behalf of, suppress it.
#define QuerySuppressed(Q) (((Q)->SuppressUnusable && (Q)->SuppressQuery) || ((Q)->DisallowPID))

#ifndef REMOVE_TUNNELING
#define PrivateQuery(Q) ((Q)->AuthInfo && (Q)->AuthInfo->AutoTunnel)
#else
#define PrivateQuery(Q) ((Q)->AuthInfo)
#endif

// Normally we always lookup the cache and /etc/hosts before sending the query on the wire. For single label
// queries (A and AAAA) that are unqualified (indicated by AppendSearchDomains), we want to append search
// domains before we try them as such
#define ApplySearchDomainsFirst(q) ((q)->AppendSearchDomains && (CountLabels(&((q)->qname))) == 1)

// Wrapper struct for Auth Records for higher-level code that cannot use the AuthRecord's ->next pointer field
typedef struct ARListElem
{
    struct ARListElem * next;
    AuthRecord ar; // Note: Must be last element of structure, to accomodate oversized AuthRecords
} ARListElem;

struct CacheRecord_struct
{
    CacheRecord * next;    // Next in list; first element of structure for efficiency reasons
    ResourceRecord resrec; // 36 bytes when compiling for 32-bit; 48 when compiling for 64-bit

    // Transient state for Cache Records
    CacheRecord * NextInKAList;     // Link to the next element in the chain of known answers to send
    mDNSs32 TimeRcvd;               // In platform time units
    mDNSs32 DelayDelivery;          // Set if we want to defer delivery of this answer to local clients
    mDNSs32 NextRequiredQuery;      // In platform time units
    mDNSs32 LastUsed;               // In platform time units
    DNSQuestion * CRActiveQuestion; // Points to an active question referencing this answer. Can never point to a NewQuestion.
    mDNSs32 LastUnansweredTime;     // In platform time units; last time we incremented UnansweredQueries
    mDNSu8 UnansweredQueries;       // Number of times we've issued a query for this record without getting an answer
    mDNSu8 CRDNSSECQuestion;        // Set to 1 if this was created in response to a DNSSEC question
    mDNSOpaque16 responseFlags;     // Second 16 bit in the DNS response
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
    mDNSu32 MPUnansweredQ;      // Multi-packet query handling: Number of times we've seen a query for this record
    mDNSs32 MPLastUnansweredQT; // Multi-packet query handling: Last time we incremented MPUnansweredQ
    mDNSu32 MPUnansweredKA;     // Multi-packet query handling: Number of times we've seen this record in a KA list
    mDNSBool MPExpectingKA;     // Multi-packet query handling: Set when we increment MPUnansweredQ; allows one KA
#endif
    CacheRecord * NextInCFList; // Set if this is in the list of records we just received with the cache flush bit set
    CacheRecord * nsec;         // NSEC records needed for non-existence proofs
    CacheRecord * soa;          // SOA record to return for proxy questions

    mDNSAddr sourceAddress; // node from which we received this record
    // Size to here is 76 bytes when compiling 32-bit; 104 bytes when compiling 64-bit
    RData_small smallrdatastorage; // Storage for small records is right here (4 bytes header + 68 bytes data = 72 bytes)
};

// Should match the CacheGroup_struct members, except namestorage[].  Only used to calculate
// the size of the namestorage array in CacheGroup_struct so that
// sizeof(CacheGroup) == sizeof(CacheRecord)
struct CacheGroup_base
{
    CacheGroup * next;
    mDNSu32 namehash;
    CacheRecord * members;
    CacheRecord ** rrcache_tail;
    domainname * name;
};

struct CacheGroup_struct // Header object for a list of CacheRecords with the same name
{
    CacheGroup * next;                                                        // Next CacheGroup object in this hash table bucket
    mDNSu32 namehash;                                                         // Name-based (i.e. case insensitive) hash of name
    CacheRecord * members;                                                    // List of CacheRecords with this same name
    CacheRecord ** rrcache_tail;                                              // Tail end of that list
    domainname * name;                                                        // Common name for all CacheRecords in this list
    mDNSu8 namestorage[sizeof(CacheRecord) - sizeof(struct CacheGroup_base)]; // match sizeof(CacheRecord)
};

// Storage sufficient to hold either a CacheGroup header or a CacheRecord
// -- for best efficiency (to avoid wasted unused storage) they should be the same size
typedef union CacheEntity_union CacheEntity;
union CacheEntity_union
{
    CacheEntity * next;
    CacheGroup cg;
    CacheRecord cr;
};

typedef struct
{
    CacheRecord r;
    mDNSu8 _extradata[MaximumRDSize - InlineCacheRDSize]; // Glue on the necessary number of extra bytes
    domainname namestorage;                               // Needs to go *after* the extra rdata bytes
} LargeCacheRecord;

typedef struct HostnameInfo
{
    struct HostnameInfo * next;
#ifndef REMOVE_NAT_FUCTIONALITY
    NATTraversalInfo natinfo;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    domainname fqdn;
    AuthRecord arv4;                     // registered IPv4 address record
    AuthRecord arv6;                     // registered IPv6 address record
    mDNSRecordCallback * StatusCallback; // callback to deliver success or error code to client layer
    const void * StatusContext;          // Client Context
} HostnameInfo;

typedef struct ExtraResourceRecord_struct ExtraResourceRecord;
struct ExtraResourceRecord_struct
{
    ExtraResourceRecord * next;
    mDNSu32 ClientID; // Opaque ID field to be used by client to map an AddRecord call to a set of Extra records
    AuthRecord r;
    // Note: Add any additional fields *before* the AuthRecord in this structure, not at the end.
    // In some cases clients can allocate larger chunks of memory and set r->rdata->MaxRDLength to indicate
    // that this extra memory is available, which would result in any fields after the AuthRecord getting smashed
};

// Note: Within an mDNSServiceCallback mDNS all API calls are legal except mDNS_Init(), mDNS_Exit(), mDNS_Execute()
typedef void mDNSServiceCallback(mDNS * const m, ServiceRecordSet * const sr, mStatus result);

// A ServiceRecordSet has no special meaning to the core code of the Multicast DNS protocol engine;
// it is just a convenience structure to group together the records that make up a standard service
// registration so that they can be allocted and deallocted together as a single memory object.
// It contains its own ServiceCallback+ServiceContext to report aggregate results up to the next layer of software above.
// It also contains:
//  * the basic PTR/SRV/TXT triplet used to represent any DNS-SD service
//  * the "_services" PTR record for service enumeration
//  * the optional list of SubType PTR records
//  * the optional list of additional records attached to the service set (e.g. iChat pictures)

struct ServiceRecordSet_struct
{
    // These internal state fields are used internally by mDNSCore; the client layer needn't be concerned with them.
    // No fields need to be set up by the client prior to calling mDNS_RegisterService();
    // all required data is passed as parameters to that function.
    mDNSServiceCallback * ServiceCallback;
    void * ServiceContext;
    mDNSBool Conflict; // Set if this record set was forcibly deregistered because of a conflict

    ExtraResourceRecord * Extras; // Optional list of extra AuthRecords attached to this service registration
    mDNSu32 NumSubTypes;
    AuthRecord * SubTypes;
    const mDNSu8 * AnonData;
    mDNSu32 flags;     // saved for subsequent calls to mDNS_RegisterService() if records
                       // need to be re-registered.
    AuthRecord RR_ADV; // e.g. _services._dns-sd._udp.local. PTR _printer._tcp.local.
    AuthRecord RR_PTR; // e.g. _printer._tcp.local.        PTR Name._printer._tcp.local.
    AuthRecord RR_SRV; // e.g. Name._printer._tcp.local.   SRV 0 0 port target
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    AuthRecord RR_TXT; // e.g. Name._printer._tcp.local.   TXT PrintQueueName
#endif                 // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // Don't add any fields after AuthRecord RR_TXT.
    // This is where the implicit extra space goes if we allocate a ServiceRecordSet containing an oversized RR_TXT record
};

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Question structures
#endif

// We record the last eight instances of each duplicate query
// This gives us v4/v6 on each of Ethernet, AirPort and Firewire, and two free slots "for future expansion"
// If the host has more active interfaces that this it is not fatal -- duplicate question suppression will degrade gracefully.
// Since we will still remember the last eight, the busiest interfaces will still get the effective duplicate question suppression.
#define DupSuppressInfoSize 8

typedef struct
{
    mDNSs32 Time;
    mDNSInterfaceID InterfaceID;
    mDNSs32 Type; // v4 or v6?
} DupSuppressInfo;

typedef enum
{
    LLQ_InitialRequest   = 1,
    LLQ_SecondaryRequest = 2,
    LLQ_Established      = 3,
    LLQ_Poll             = 4
} LLQ_State;

// LLQ constants
#define kLLQ_Vers 1
#define kLLQ_DefLease 7200 // 2 hours
#define kLLQ_MAX_TRIES 3   // retry an operation 3 times max
#define kLLQ_INIT_RESEND 2 // resend an un-ack'd packet after 2 seconds, then double for each additional
// LLQ Operation Codes
#define kLLQOp_Setup 1
#define kLLQOp_Refresh 2
#define kLLQOp_Event 3

// LLQ Errror Codes
enum
{
    LLQErr_NoError    = 0,
    LLQErr_ServFull   = 1,
    LLQErr_Static     = 2,
    LLQErr_FormErr    = 3,
    LLQErr_NoSuchLLQ  = 4,
    LLQErr_BadVers    = 5,
    LLQErr_UnknownErr = 6
};

enum
{
    NoAnswer_Normal    = 0,
    NoAnswer_Suspended = 1,
    NoAnswer_Fail      = 2
};

#define HMAC_LEN 64
#define HMAC_IPAD 0x36
#define HMAC_OPAD 0x5c
#define MD5_LEN 16

#ifndef REMOVE_TUNNELING
#define AutoTunnelUnregistered(X)                                                                                                  \
    ((X)->AutoTunnelHostRecord.resrec.RecordType == kDNSRecordTypeUnregistered &&                                                  \
     (X)->AutoTunnelTarget.resrec.RecordType == kDNSRecordTypeUnregistered &&                                                      \
     (X)->AutoTunnelDeviceInfo.resrec.RecordType == kDNSRecordTypeUnregistered &&                                                  \
     (X)->AutoTunnelService.resrec.RecordType == kDNSRecordTypeUnregistered &&                                                     \
     (X)->AutoTunnel6Record.resrec.RecordType == kDNSRecordTypeUnregistered)
#endif // #ifndef REMOVE_TUNNELING

// Internal data structure to maintain authentication information
typedef struct DomainAuthInfo
{
    struct DomainAuthInfo * next;
    mDNSs32 deltime; // If we're planning to delete this DomainAuthInfo, the time we want it deleted
#ifndef REMOVE_TUNNELING
    mDNSBool AutoTunnel;               // Whether this is AutoTunnel
    AuthRecord AutoTunnelHostRecord;   // User-visible hostname; used as SRV target for AutoTunnel services
    AuthRecord AutoTunnelTarget;       // Opaque hostname of tunnel endpoint; used as SRV target for AutoTunnelService record
    AuthRecord AutoTunnelDeviceInfo;   // Device info of tunnel endpoint
    AuthRecord AutoTunnelService;      // Service record (possibly NAT-Mapped) of IKE daemon implementing tunnel endpoint
    AuthRecord AutoTunnel6Record;      // AutoTunnel AAAA Record obtained from awacsd
    mDNSBool AutoTunnelServiceStarted; // Whether a service has been registered in this domain
    mDNSv6Addr AutoTunnelInnerAddress;
#endif // #ifndef REMOVE_TUNNELING
    domainname domain;
    domainname keyname;
    domainname hostname;
    mDNSIPPort port;
    char b64keydata[32];
    mDNSu8 keydata_ipad[HMAC_LEN]; // padded key for inner hash rounds
    mDNSu8 keydata_opad[HMAC_LEN]; // padded key for outer hash rounds
} DomainAuthInfo;

// Note: Within an mDNSQuestionCallback mDNS all API calls are legal except mDNS_Init(), mDNS_Exit(), mDNS_Execute()
// Note: Any value other than QC_rmv i.e., any non-zero value will result in kDNSServiceFlagsAdd to the application
// layer. These values are used within mDNSResponder and not sent across to the application. QC_addnocache is for
// delivering a response without adding to the cache. QC_forceresponse is superset of QC_addnocache where in
// addition to not entering in the cache, it also forces the negative response through.
typedef enum
{
    QC_rmv = 0,
    QC_add,
    QC_addnocache,
    QC_forceresponse,
    QC_dnssec,
    QC_nodnssec,
    QC_suppressed
} QC_result;
typedef void mDNSQuestionCallback(mDNS * const m, DNSQuestion * question, const ResourceRecord * const answer, QC_result AddRecord);
typedef void AsyncDispatchFunc(mDNS * const m, void * context);
typedef void DNSSECAuthInfoFreeCallback(mDNS * const m, void * context);
extern void mDNSPlatformDispatchAsync(mDNS * const m, void * context, AsyncDispatchFunc func);

#define NextQSendTime(Q) ((Q)->LastQTime + (Q)->ThisQInterval)
#define ActiveQuestion(Q) ((Q)->ThisQInterval > 0 && !(Q)->DuplicateOf)
#define TimeToSendThisQuestion(Q, time) (ActiveQuestion(Q) && (time) -NextQSendTime(Q) >= 0)

// q->ValidationStatus is either DNSSECValNotRequired or DNSSECValRequired and then moves onto DNSSECValInProgress.
// When Validation is done, we mark all "DNSSECValInProgress" questions "DNSSECValDone". If we are answering
// questions from /etc/hosts, then we go straight to DNSSECValDone from the initial state.
typedef enum
{
    DNSSECValNotRequired = 0,
    DNSSECValRequired,
    DNSSECValInProgress,
    DNSSECValDone
} DNSSECValState;

// ValidationRequired can be set to the following values:
//
// SECURE validation is set to determine whether something is secure or bogus
// INSECURE validation is set internally by dnssec code to indicate that it is currently proving something
// is insecure
#define DNSSEC_VALIDATION_NONE 0x00
#define DNSSEC_VALIDATION_SECURE 0x01
#define DNSSEC_VALIDATION_SECURE_OPTIONAL 0x02
#define DNSSEC_VALIDATION_INSECURE 0x03

// For both ValidationRequired and ValidatingResponse question, we validate DNSSEC responses.
// For ProxyQuestion with DNSSECOK, we just receive the DNSSEC records to pass them along without
// validation and if the CD bit is not set, we also validate.
#define DNSSECQuestion(q) ((q)->ValidationRequired || (q)->ValidatingResponse || ((q)->ProxyQuestion && (q)->ProxyDNSSECOK))

// ValidatingQuestion is used when we need to know whether we are validating the DNSSEC responses for a question
#define ValidatingQuestion(q) ((q)->ValidationRequired || (q)->ValidatingResponse)

#define DNSSECOptionalQuestion(q) ((q)->ValidationRequired == DNSSEC_VALIDATION_SECURE_OPTIONAL)

// Given the resource record and the question, should we follow the CNAME ?
#define FollowCNAME(q, rr, AddRecord)                                                                                              \
    (AddRecord && (q)->qtype != kDNSType_CNAME && (rr)->RecordType != kDNSRecordTypePacketNegative &&                              \
     (rr)->rrtype == kDNSType_CNAME)

// RFC 4122 defines it to be 16 bytes
#define UUID_SIZE 16

struct DNSQuestion_struct
{
    // Internal state fields. These are used internally by mDNSCore; the client layer needn't be concerned with them.
    DNSQuestion * next;
    mDNSu32 qnamehash;
    mDNSs32 DelayAnswering;             // Set if we want to defer answering this question until the cache settles
    mDNSs32 LastQTime;                  // Last scheduled transmission of this Q on *all* applicable interfaces
    mDNSs32 ThisQInterval;              // LastQTime + ThisQInterval is the next scheduled transmission of this Q
                                        // ThisQInterval > 0 for an active question;
                                        // ThisQInterval = 0 for a suspended question that's still in the list
                                        // ThisQInterval = -1 for a cancelled question (should not still be in list)
    mDNSs32 ExpectUnicastResp;          // Set when we send a query with the kDNSQClass_UnicastResponse bit set
    mDNSs32 LastAnswerPktNum;           // The sequence number of the last response packet containing an answer to this Q
    mDNSu32 RecentAnswerPkts;           // Number of answers since the last time we sent this query
    mDNSu32 CurrentAnswers;             // Number of records currently in the cache that answer this question
    mDNSu32 BrowseThreshold;            // If we have received at least this number of answers,
                                        // set the next question interval to MaxQuestionInterval
    mDNSu32 LargeAnswers;               // Number of answers with rdata > 1024 bytes
    mDNSu32 UniqueAnswers;              // Number of answers received with kDNSClass_UniqueRRSet bit set
    mDNSInterfaceID FlappingInterface1; // Set when an interface goes away, to flag if remove events are delivered for this Q
    mDNSInterfaceID FlappingInterface2; // Set when an interface goes away, to flag if remove events are delivered for this Q
    DomainAuthInfo * AuthInfo;          // Non-NULL if query is currently being done using Private DNS
    DNSQuestion * DuplicateOf;
    DNSQuestion * NextInDQList;
    AnonymousInfo * AnonInfo; // Anonymous Information
    DupSuppressInfo DupSuppress[DupSuppressInfoSize];
    mDNSInterfaceID SendQNow;         // The interface this query is being sent on right now
    mDNSBool SendOnAll;               // Set if we're sending this question on all active interfaces
    mDNSBool CachedAnswerNeedsUpdate; // See SendQueries().  Set if we're sending this question
                                      // because a cached answer needs to be refreshed.
    mDNSu32 RequestUnicast;           // Non-zero if we want to send query with kDNSQClass_UnicastResponse bit set
    mDNSs32 LastQTxTime;              // Last time this Q was sent on one (but not necessarily all) interfaces
    mDNSu32 CNAMEReferrals;           // Count of how many CNAME redirections we've done
    mDNSBool SuppressQuery;           // This query should be suppressed and not sent on the wire
    mDNSu8 LOAddressAnswers;          // Number of answers from the local only auth records that are
                                      // answering A, AAAA, CNAME, or PTR (/etc/hosts)
    mDNSu8 WakeOnResolveCount;        // Number of wakes that should be sent on resolve
    mDNSs32 StopTime;                 // Time this question should be stopped by giving them a negative answer

    // DNSSEC fields
    DNSSECValState ValidationState; // Current state of the Validation process
    DNSSECStatus ValidationStatus;  // Validation status for "ValidationRequired" questions (dnssec)
    mDNSu8 ValidatingResponse;      // Question trying to validate a response (dnssec) on behalf of
                                    // ValidationRequired question
    void * DNSSECAuthInfo;
    DNSSECAuthInfoFreeCallback * DAIFreeCallback;

    // Wide Area fields. These are used internally by the uDNS core (Unicast)
    UDPSocket * LocalSocket;

    // |-> DNS Configuration related fields used in uDNS (Subset of Wide Area/Unicast fields)
    DNSServer * qDNSServer;       // Caching server for this query (in the absence of an SRV saying otherwise)
    mDNSOpaque64 validDNSServers; // Valid DNSServers for this question
    mDNSu16 noServerResponse;     // At least one server did not respond.
    mDNSu16 triedAllServersOnce;  // Tried all DNS servers once
    mDNSu8 unansweredQueries;     // The number of unanswered queries to this server

    ZoneData * nta;    // Used for getting zone data for private or LLQ query
    mDNSAddr servAddr; // Address and port learned from _dns-llq, _dns-llq-tls or _dns-query-tls SRV query
    mDNSIPPort servPort;
    struct tcpInfo_t * tcp;
    mDNSIPPort tcpSrcPort; // Local Port TCP packet received on;need this as tcp struct is disposed
                           // by tcpCallback before calling into mDNSCoreReceive
    mDNSu8 NoAnswer;       // Set if we want to suppress answers until tunnel setup has completed
    mDNSu8 Restart;        // This question should be restarted soon

    // LLQ-specific fields. These fields are only meaningful when LongLived flag is set
    LLQ_State state;
    mDNSu32 ReqLease; // seconds (relative)
    mDNSs32 expire;   // ticks (absolute)
    mDNSs16 ntries;   // for UDP: the number of packets sent for this LLQ state
                      // for TCP: there is some ambiguity in the use of this variable, but in general, it is
                      //          the number of TCP/TLS connection attempts for this LLQ state, or
                      //          the number of packets sent for this TCP/TLS connection
    mDNSOpaque64 id;

    // DNS Proxy fields
    mDNSOpaque16 responseFlags; // Temporary place holder for the error we get back from the DNS server
                                // till we populate in the cache
    mDNSBool DisallowPID;       // Is the query allowed for the "PID" that we are sending on behalf of ?
    mDNSs32 ServiceID;          // Service identifier to match against the DNS server

    // Client API fields: The client must set up these fields *before* calling mDNS_StartQuery()
    mDNSInterfaceID InterfaceID; // Non-zero if you want to issue queries only on a single specific IP interface
    mDNSu32 flags;               // flags from original DNSService*() API request.
    mDNSAddr Target;             // Non-zero if you want to direct queries to a specific unicast target address
    mDNSIPPort TargetPort;       // Must be set if Target is set
    mDNSOpaque16 TargetQID;      // Must be set if Target is set
    domainname qname;
    mDNSu16 qtype;
    mDNSu16 qclass;
    mDNSBool LongLived;            // Set by client for calls to mDNS_StartQuery to indicate LLQs to unicast layer.
    mDNSBool ExpectUnique;         // Set by client if it's expecting unique RR(s) for this question, not shared RRs
    mDNSBool ForceMCast;           // Set by client to force mDNS query, even for apparently uDNS names
    mDNSBool ReturnIntermed;       // Set by client to request callbacks for intermediate CNAME/NXDOMAIN results
    mDNSBool SuppressUnusable;     // Set by client to suppress unusable queries to be sent on the wire
    mDNSu8 RetryWithSearchDomains; // Retry with search domains if there is no entry in the cache or AuthRecords
    mDNSu8 TimeoutQuestion;        // Timeout this question if there is no reply in configured time
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    mDNSu8 WakeOnResolve;             // Send wakeup on resolve
#endif                                //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    mDNSu8 UseBackgroundTrafficClass; // Use background traffic class for request
    mDNSs8 SearchListIndex;           // Index into SearchList; Used by the client layer but not touched by core
    mDNSs8 AppendSearchDomains;       // Search domains can be appended for this query
    mDNSs8 AppendLocalSearchDomains;  // Search domains ending in .local can be appended for this query
    mDNSu8 ValidationRequired;        // Requires DNSSEC validation.
    mDNSu8 ProxyQuestion;             // Proxy Question
    mDNSu8 ProxyDNSSECOK;             // Proxy Question with EDNS0 DNSSEC OK bit set
    mDNSs32 pid;                      // Process ID of the client that is requesting the question
    mDNSu8 uuid[UUID_SIZE];           // Unique ID of the client that is requesting the question (valid only if pid is zero)
    domainname * qnameOrig;           // Copy of the original question name if it is not fully qualified
    mDNSQuestionCallback * QuestionCallback;
    void * QuestionContext;
};

typedef struct
{
    // Client API fields: The client must set up name and InterfaceID *before* calling mDNS_StartResolveService()
    // When the callback is invoked, ip, port, TXTlen and TXTinfo will have been filled in with the results learned from the
    // network.
    domainname name;
    mDNSInterfaceID InterfaceID; // ID of the interface the response was received on
    mDNSAddr ip;                 // Remote (destination) IP address where this service can be accessed
    mDNSIPPort port;             // Port where this service can be accessed
    mDNSu16 TXTlen;
    mDNSu8 TXTinfo[2048]; // Additional demultiplexing information (e.g. LPR queue name)
} ServiceInfo;

// Note: Within an mDNSServiceInfoQueryCallback mDNS all API calls are legal except mDNS_Init(), mDNS_Exit(), mDNS_Execute()
typedef struct ServiceInfoQuery_struct ServiceInfoQuery;
typedef void mDNSServiceInfoQueryCallback(mDNS * const m, ServiceInfoQuery * query);
struct ServiceInfoQuery_struct
{
    // Internal state fields. These are used internally by mDNSCore; the client layer needn't be concerned with them.
    // No fields need to be set up by the client prior to calling mDNS_StartResolveService();
    // all required data is passed as parameters to that function.
    // The ServiceInfoQuery structure memory is working storage for mDNSCore to discover the requested information
    // and place it in the ServiceInfo structure. After the client has called mDNS_StopResolveService(), it may
    // dispose of the ServiceInfoQuery structure while retaining the results in the ServiceInfo structure.
    DNSQuestion qSRV;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    DNSQuestion qTXT;
#endif //#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    DNSQuestion qAv4;
    DNSQuestion qAv6;
    mDNSu8 GotSRV;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    mDNSu8 GotTXT;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    mDNSu8 GotADD;
    mDNSu32 Answers;
    ServiceInfo * info;
    mDNSServiceInfoQueryCallback * ServiceInfoQueryCallback;
    void * ServiceInfoQueryContext;
};

typedef enum
{
    ZoneServiceUpdate,
    ZoneServiceQuery,
    ZoneServiceLLQ
} ZoneService;

typedef void ZoneDataCallback(mDNS * const m, mStatus err, const ZoneData * result);

struct ZoneData_struct
{
    domainname ChildName;                // Name for which we're trying to find the responsible server
    ZoneService ZoneService;             // Which service we're seeking for this zone (update, query, or LLQ)
    domainname * CurrentSOA;             // Points to somewhere within ChildName
    domainname ZoneName;                 // Discovered result: Left-hand-side of SOA record
    mDNSu16 ZoneClass;                   // Discovered result: DNS Class from SOA record
    domainname Host;                     // Discovered result: Target host from SRV record
    mDNSIPPort Port;                     // Discovered result: Update port, query port, or LLQ port from SRV record
    mDNSAddr Addr;                       // Discovered result: Address of Target host from SRV record
    mDNSBool ZonePrivate;                // Discovered result: Does zone require encrypted queries?
    ZoneDataCallback * ZoneDataCallback; // Caller-specified function to be called upon completion
    void * ZoneDataContext;
    DNSQuestion question; // Storage for any active question
};

extern ZoneData * StartGetZoneData(mDNS * const m, const domainname * const name, const ZoneService target,
                                   ZoneDataCallback callback, void * callbackInfo);
extern void CancelGetZoneData(mDNS * const m, ZoneData * nta);
extern mDNSBool IsGetZoneDataQuestion(DNSQuestion * q);

typedef struct DNameListElem
{
    struct DNameListElem * next;
    mDNSu32 uid;
    domainname name;
} DNameListElem;

#if APPLE_OSX_mDNSResponder
// Different states that we go through locating the peer
#define TC_STATE_AAAA_PEER 0x000000001       /* Peer's BTMM IPv6 address */
#define TC_STATE_AAAA_PEER_RELAY 0x000000002 /* Peer's IPv6 Relay address */
#define TC_STATE_SRV_PEER 0x000000003        /* Peer's SRV Record corresponding to IPv4 address */
#define TC_STATE_ADDR_PEER 0x000000004       /* Peer's IPv4 address */

typedef struct ClientTunnel
{
    struct ClientTunnel * next;
    domainname dstname;
    mDNSBool MarkedForDeletion;
    mDNSv6Addr loc_inner;
    mDNSv4Addr loc_outer;
    mDNSv6Addr loc_outer6;
    mDNSv6Addr rmt_inner;
    mDNSv4Addr rmt_outer;
    mDNSv6Addr rmt_outer6;
    mDNSIPPort rmt_outer_port;
    mDNSu16 tc_state;
    DNSQuestion q;
} ClientTunnel;
#endif

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - NetworkInterfaceInfo_struct
#endif

typedef struct NetworkInterfaceInfo_struct NetworkInterfaceInfo;

// A NetworkInterfaceInfo_struct serves two purposes:
// 1. It holds the address, PTR and HINFO records to advertise a given IP address on a given physical interface
// 2. It tells mDNSCore which physical interfaces are available; each physical interface has its own unique InterfaceID.
//    Since there may be multiple IP addresses on a single physical interface,
//    there may be multiple NetworkInterfaceInfo_structs with the same InterfaceID.
//    In this case, to avoid sending the same packet n times, when there's more than one
//    struct with the same InterfaceID, mDNSCore picks one member of the set to be the
//    active representative of the set; all others have the 'InterfaceActive' flag unset.

struct NetworkInterfaceInfo_struct
{
    // Internal state fields. These are used internally by mDNSCore; the client layer needn't be concerned with them.
    NetworkInterfaceInfo * next;

    mDNSu8 InterfaceActive; // Set if interface is sending & receiving packets (see comment above)
    mDNSu8 IPv4Available;   // If InterfaceActive, set if v4 available on this InterfaceID
    mDNSu8 IPv6Available;   // If InterfaceActive, set if v6 available on this InterfaceID

#ifndef REMOVE_NETWAKE
    DNSQuestion NetWakeBrowse;
#ifdef ONLY_ONE_NetWakeResolve
    DNSQuestion NetWakeResolve;
#else
    DNSQuestion NetWakeResolve[3]; // For fault-tolerance, we try up to three Sleep Proxies
#endif
#endif // #ifndef REMOVE_NETWAKE
    mDNSAddr SPSAddr[3];
    mDNSIPPort SPSPort[3];
    mDNSs32 NextSPSAttempt; // -1 if we're not currently attempting to register with any Sleep Proxy
    mDNSs32 NextSPSAttemptTime;

    // Standard AuthRecords that every Responder host should have (one per active IP address)
    AuthRecord RR_A;   // 'A' or 'AAAA' (address) record for our ".local" name
    AuthRecord RR_PTR; // PTR (reverse lookup) record
#ifndef REMOVE_RR_HINFO
    AuthRecord RR_HINFO;
#endif

    // Client API fields: The client must set up these fields *before* calling mDNS_RegisterInterface()
    mDNSInterfaceID InterfaceID; // Identifies physical interface; MUST NOT be 0, -1, or -2
    mDNSAddr ip;                 // The IPv4 or IPv6 address to advertise
    mDNSAddr mask;
    mDNSEthAddr MAC;
    char ifname[64];     // Windows uses a GUID string for the interface name, which doesn't fit in 16 bytes
    mDNSu8 Advertise;    // False if you are only searching on this interface
    mDNSu8 McastTxRx;    // Send/Receive multicast on this { InterfaceID, address family } ?
    mDNSu8 NetWake;      // Set if Wake-On-Magic-Packet is enabled on this interface
    mDNSu8 Loopback;     // Set if this is the loopback interface
    mDNSu8 IgnoreIPv4LL; // Set if IPv4 Link-Local addresses have to be ignored.
    mDNSu8 SendGoodbyes; // Send goodbyes on this interface while sleeping
    mDNSBool DirectLink; // a direct link, indicating we can skip the probe for
                         // address records
};

#define SLE_DELETE 0x00000001
#define SLE_WAB_BROWSE_QUERY_STARTED 0x00000002
#define SLE_WAB_LBROWSE_QUERY_STARTED 0x00000004
#define SLE_WAB_REG_QUERY_STARTED 0x00000008

typedef struct SearchListElem
{
    struct SearchListElem * next;
    domainname domain;
    int flag;
    mDNSInterfaceID InterfaceID;
    DNSQuestion BrowseQ;
    DNSQuestion DefBrowseQ;
    DNSQuestion AutomaticBrowseQ;
    DNSQuestion RegisterQ;
    DNSQuestion DefRegisterQ;
    int numCfAnswers;
    ARListElem * AuthRecs;
} SearchListElem;

// For domain enumeration and automatic browsing
// This is the user's DNS search list.
// In each of these domains we search for our special pointer records (lb._dns-sd._udp.<domain>, etc.)
// to discover recommended domains for domain enumeration (browse, default browse, registration,
// default registration) and possibly one or more recommended automatic browsing domains.
extern SearchListElem * SearchList; // This really ought to be part of mDNS_struct -- SC

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Main mDNS object, used to hold all the mDNS state
#endif

typedef void mDNSCallback(mDNS * const m, mStatus result);

#ifndef CACHE_HASH_SLOTS
#define CACHE_HASH_SLOTS 499
#endif

enum
{
    SleepState_Awake        = 0,
    SleepState_Transferring = 1,
    SleepState_Sleeping     = 2
};

typedef enum
{
    kStatsActionIncrement,
    kStatsActionDecrement,
    kStatsActionClear,
    kStatsActionSet
} DNSSECStatsAction;

typedef enum
{
    kStatsTypeMemoryUsage,
    kStatsTypeLatency,
    kStatsTypeExtraPackets,
    kStatsTypeStatus,
    kStatsTypeProbe,
    kStatsTypeMsgSize
} DNSSECStatsType;

typedef struct
{
    mDNSu32 TotalMemUsed;
    mDNSu32 Latency0;       // 0 to 4 ms
    mDNSu32 Latency5;       // 5 to  9 ms
    mDNSu32 Latency10;      // 10 to 19 ms
    mDNSu32 Latency20;      // 20 to 49 ms
    mDNSu32 Latency50;      // 50 to 99 ms
    mDNSu32 Latency100;     // >= 100 ms
    mDNSu32 ExtraPackets0;  // 0 to 2 packets
    mDNSu32 ExtraPackets3;  // 3 to 6 packets
    mDNSu32 ExtraPackets7;  // 7 to 9 packets
    mDNSu32 ExtraPackets10; // >= 10 packets
    mDNSu32 SecureStatus;
    mDNSu32 InsecureStatus;
    mDNSu32 IndeterminateStatus;
    mDNSu32 BogusStatus;
    mDNSu32 NoResponseStatus;
    mDNSu32 NumProbesSent; // Number of probes sent
    mDNSu32 MsgSize0;      // DNSSEC message size <= 1024
    mDNSu32 MsgSize1;      // DNSSEC message size <= 2048
    mDNSu32 MsgSize2;      // DNSSEC message size > 2048
} DNSSECStatistics;

typedef struct
{
    mDNSu32 NameConflicts;             // Normal Name conflicts
    mDNSu32 KnownUniqueNameConflicts;  // Name Conflicts for KnownUnique Records
    mDNSu32 DupQuerySuppressions;      // Duplicate query suppressions
    mDNSu32 KnownAnswerSuppressions;   // Known Answer suppressions
    mDNSu32 KnownAnswerMultiplePkts;   // Known Answer in queries spannign multiple packets
    mDNSu32 PoofCacheDeletions;        // Number of times the cache was deleted due to POOF
    mDNSu32 UnicastBitInQueries;       // Queries with QU bit set
    mDNSu32 NormalQueries;             // Queries with QU bit not set
    mDNSu32 MatchingAnswersForQueries; // Queries for which we had a response
    mDNSu32 UnicastResponses;          // Unicast responses to queries
    mDNSu32 MulticastResponses;        // Multicast responses to queries
    mDNSu32 UnicastDemotedToMulticast; // Number of times unicast demoted to multicast
    mDNSu32 Sleeps;                    // Total sleeps
    mDNSu32 Wakes;                     // Total wakes
    mDNSu32 InterfaceUp;               // Total Interface UP events
    mDNSu32 InterfaceUpFlap;           // Total Interface UP events with flaps
    mDNSu32 InterfaceDown;             // Total Interface Down events
    mDNSu32 InterfaceDownFlap;         // Total Interface Down events with flaps
    mDNSu32 CacheRefreshQueries;       // Number of queries that we sent for refreshing cache
    mDNSu32 CacheRefreshed;            // Number of times the cache was refreshed due to a response
    mDNSu32 WakeOnResolves;            // Number of times we did a wake on resolve
} mDNSStatistics;
extern void LogMDNSStatistics(mDNS * const m);

struct mDNS_struct
{
    // Internal state fields. These hold the main internal state of mDNSCore;
    // the client layer needn't be concerned with them.
    // No fields need to be set up by the client prior to calling mDNS_Init();
    // all required data is passed as parameters to that function.

    mDNS_PlatformSupport * p; // Pointer to platform-specific data of indeterminite size
    mDNSBool CanReceiveUnicastOn5353;
    mDNSBool AdvertiseLocalAddresses;
    mDNSBool DivertMulticastAdvertisements; // from interfaces that do not advertise local addresses to local-only
    mStatus mDNSPlatformStatus;
    mDNSIPPort UnicastPort4;
    mDNSIPPort UnicastPort6;
    mDNSEthAddr PrimaryMAC; // Used as unique host ID
    mDNSCallback * MainCallback;
    void * MainContext;

    // For debugging: To catch and report locking failures
    mDNSu32 mDNS_busy;       // Incremented between mDNS_Lock/mDNS_Unlock section
    mDNSu32 mDNS_reentrancy; // Incremented when calling a client callback
    mDNSu8 lock_rrcache;     // For debugging: Set at times when these lists may not be modified
    mDNSu8 lock_Questions;
    mDNSu8 lock_Records;
#ifndef MaxMsg
#define MaxMsg 512
#endif
    char MsgBuffer[MaxMsg]; // Temp storage used while building error log messages

    // Task Scheduling variables
    mDNSs32 timenow_adjust;        // Correction applied if we ever discover time went backwards
    mDNSs32 timenow;               // The time that this particular activation of the mDNS code started
    mDNSs32 timenow_last;          // The time the last time we ran
    mDNSs32 NextScheduledEvent;    // Derived from values below
    mDNSs32 ShutdownTime;          // Set when we're shutting down; allows us to skip some unnecessary steps
    mDNSs32 SuppressSending;       // Don't send local-link mDNS packets during this time
    mDNSs32 NextCacheCheck;        // Next time to refresh cache record before it expires
    mDNSs32 NextScheduledQuery;    // Next time to send query in its exponential backoff sequence
    mDNSs32 NextScheduledProbe;    // Next time to probe for new authoritative record
    mDNSs32 NextScheduledResponse; // Next time to send authoritative record(s) in responses
#ifndef REMOVE_NAT_FUCTIONALITY
    mDNSs32 NextScheduledNATOp;    // Next time to send NAT-traversal packets
#endif                             //#ifndef REMOVE_NAT_FUCTIONALITY
    mDNSs32 NextScheduledSPS;      // Next time to purge expiring Sleep Proxy records
    mDNSs32 NextScheduledKA;       // Next time to send Keepalive packets (SPS)
    mDNSs32 RandomQueryDelay;      // For de-synchronization of query packets on the wire
    mDNSu32 RandomReconfirmDelay;  // For de-synchronization of reconfirmation queries on the wire
    mDNSs32 PktNum;                // Unique sequence number assigned to each received packet
    mDNSs32 MPktNum;               // Unique sequence number assigned to each received Multicast packet
    mDNSu8 LocalRemoveEvents;      // Set if we may need to deliver remove events for local-only questions and/or local-only records
    mDNSu8 SleepState;             // Set if we're sleeping
    mDNSu8 SleepSeqNum;            // "Epoch number" of our current period of wakefulness
    mDNSu8 SystemWakeOnLANEnabled; // Set if we want to register with a Sleep Proxy before going to sleep
    mDNSu8 SentSleepProxyRegistration; // Set if we registered (or tried to register) with a Sleep Proxy
    mDNSu8 SystemSleepOnlyIfWakeOnLAN; // Set if we may only sleep if we managed to register with a Sleep Proxy
    mDNSs32 AnnounceOwner;             // After waking from sleep, include OWNER option in packets until this time
    mDNSs32 DelaySleep;                // To inhibit re-sleeping too quickly right after wake
    mDNSs32 SleepLimit;                // Time window to allow deregistrations, etc.,
                                       // during which underying platform layer should inhibit system sleep
    mDNSs32 TimeSlept;                 // Time we went to sleep.

    mDNSs32 StatStartTime;        // Time we started gathering statistics during this interval.
    mDNSs32 NextStatLogTime;      // Next time to log statistics.
    mDNSs32 ActiveStatTime;       // Total time awake/gathering statistics for this log period.
    mDNSs32 UnicastPacketsSent;   // Number of unicast packets sent.
    mDNSs32 MulticastPacketsSent; // Number of multicast packets sent.
    mDNSs32 RemoteSubnet;         // Multicast packets received from outside our subnet.

    mDNSs32 NextScheduledSPRetry; // Time next sleep proxy registration action is required.
                                  // Only valid if SleepLimit is nonzero and DelaySleep is zero.

    mDNSs32 NextScheduledStopTime; // Next time to stop a question

    // These fields only required for mDNS Searcher...
    DNSQuestion * Questions;             // List of all registered questions, active and inactive
    DNSQuestion * NewQuestions;          // Fresh questions not yet answered from cache
    DNSQuestion * CurrentQuestion;       // Next question about to be examined in AnswerLocalQuestions()
    DNSQuestion * LocalOnlyQuestions;    // Questions with InterfaceID set to mDNSInterface_LocalOnly or mDNSInterface_P2P
    DNSQuestion * NewLocalOnlyQuestions; // Fresh local-only or P2P questions not yet answered
    DNSQuestion * RestartQuestion;       // Questions that are being restarted (stop followed by start)
    DNSQuestion * ValidationQuestion;    // Questions that are being validated (dnssec)
    mDNSu32 rrcache_size;                // Total number of available cache entries
    mDNSu32 rrcache_totalused;           // Number of cache entries currently occupied
    mDNSu32 rrcache_totalused_unicast;   // Number of cache entries currently occupied by unicast
    mDNSu32 rrcache_active;              // Number of cache entries currently occupied by records that answer active questions
    mDNSu32 rrcache_report;
    CacheEntity * rrcache_free;
    CacheGroup * rrcache_hash[CACHE_HASH_SLOTS];
    mDNSs32 rrcache_nextcheck[CACHE_HASH_SLOTS];

    AuthHash rrauth;

    // Fields below only required for mDNS Responder...
    domainlabel nicelabel;        // Rich text label encoded using canonically precomposed UTF-8
    domainlabel hostlabel;        // Conforms to RFC 1034 "letter-digit-hyphen" ARPANET host name rules
    domainname MulticastHostname; // Fully Qualified "dot-local" Host Name, e.g. "Foo.local."
    UTF8str255 HIHardware;
    UTF8str255 HISoftware;
    AuthRecord DeviceInfo;
    AuthRecord * ResourceRecords;
    AuthRecord * DuplicateRecords; // Records currently 'on hold' because they are duplicates of existing records
    AuthRecord * NewLocalRecords;  // Fresh AuthRecords (public) not yet delivered to our local-only questions
    AuthRecord * CurrentRecord;    // Next AuthRecord about to be examined
    mDNSBool NewLocalOnlyRecords;  // Fresh AuthRecords (local only) not yet delivered to our local questions
    NetworkInterfaceInfo * HostInterfaces;
    mDNSs32 ProbeFailTime;
    mDNSu32 NumFailedProbes;
    mDNSs32 SuppressProbes;
    Platform_t mDNS_plat;

    // Unicast-specific data
    mDNSs32 NextuDNSEvent; // uDNS next event
    mDNSs32 NextSRVUpdate; // Time to perform delayed update

    DNSServer * DNSServers;         // list of DNS servers
    McastResolver * McastResolvers; // list of Mcast Resolvers

    mDNSAddr Router;
    mDNSAddr AdvertisedV4; // IPv4 address pointed to by hostname
    mDNSAddr AdvertisedV6; // IPv6 address pointed to by hostname

    DomainAuthInfo * AuthInfoList; // list of domains requiring authentication for updates

    DNSQuestion ReverseMap; // Reverse-map query to find static hostname for service target
    DNSQuestion AutomaticBrowseDomainQ;
    domainname StaticHostname; // Current answer to reverse-map query
    domainname FQDN;
    HostnameInfo * Hostnames; // List of registered hostnames + hostname metadata
#ifndef REMOVE_NAT_FUCTIONALITY
    NATTraversalInfo AutoTunnelNAT; // Shared between all AutoTunnel DomainAuthInfo structs
#endif                              //#ifndef REMOVE_NAT_FUCTIONALITY
    mDNSv6Addr AutoTunnelRelayAddr;

    mDNSu32 WABBrowseQueriesCount;  // Number of WAB Browse domain enumeration queries (b, db) callers
    mDNSu32 WABLBrowseQueriesCount; // Number of legacy WAB Browse domain enumeration queries (lb) callers
    mDNSu32 WABRegQueriesCount;     // Number of WAB Registration domain enumeration queries (r, dr) callers
    mDNSu8 SearchDomainsHash[MD5_LEN];

#ifndef REMOVE_NAT_FUCTIONALITY
    // NAT-Traversal fields
    NATTraversalInfo LLQNAT; // Single shared NAT Traversal to receive inbound LLQ notifications
    NATTraversalInfo * NATTraversals;
    NATTraversalInfo * CurrentNATTraversal;
    mDNSs32 retryIntervalGetAddr; // delta between time sent and retry for NAT-PMP & UPnP/IGD external address request
    mDNSs32 retryGetAddr;         // absolute time when we retry for NAT-PMP & UPnP/IGD external address request
    mDNSv4Addr ExtAddress;        // the external address discovered via NAT-PMP or UPnP/IGD
    mDNSu32 PCPNonce[3];          // the nonce if using PCP

    UDPSocket * NATMcastRecvskt;   // For receiving PCP & NAT-PMP announcement multicasts from router on port 5350
    mDNSu32 LastNATupseconds;      // NAT engine uptime in seconds, from most recent NAT packet
    mDNSs32 LastNATReplyLocalTime; // Local time in ticks when most recent NAT packet was received
    mDNSu16 LastNATMapResultCode;  // Most recent error code for mappings

    tcpLNTInfo tcpAddrInfo;        // legacy NAT traversal TCP connection info for external address
    tcpLNTInfo tcpDeviceInfo;      // legacy NAT traversal TCP connection info for device info
    tcpLNTInfo * tcpInfoUnmapList; // list of pending unmap requests
#endif                             //#ifndef REMOVE_NAT_FUCTIONALITY
    mDNSInterfaceID UPnPInterfaceID;
    UDPSocket * SSDPSocket;           // For SSDP request/response
    mDNSBool SSDPWANPPPConnection;    // whether we should send the SSDP query for WANIPConnection or WANPPPConnection
    mDNSIPPort UPnPRouterPort;        // port we send discovery messages to
    mDNSIPPort UPnPSOAPPort;          // port we send SOAP messages to
    mDNSu8 * UPnPRouterURL;           // router's URL string
    mDNSBool UPnPWANPPPConnection;    // whether we're using WANIPConnection or WANPPPConnection
    mDNSu8 * UPnPSOAPURL;             // router's SOAP control URL string
    mDNSu8 * UPnPRouterAddressString; // holds both the router's address and port
    mDNSu8 * UPnPSOAPAddressString;   // holds both address and port for SOAP messages

    // Sleep Proxy client fields
    AuthRecord * SPSRRSet; // To help the client keep track of the records registered with the sleep proxy

    // Sleep Proxy Server fields
    mDNSu8 SPSType;          // 0 = off, 10-99 encodes desirability metric
    mDNSu8 SPSPortability;   // 10-99
    mDNSu8 SPSMarginalPower; // 10-99
    mDNSu8 SPSTotalPower;    // 10-99
    mDNSu8 SPSFeatureFlags;  // Features supported. Currently 1 = TCP KeepAlive supported.
    mDNSu8 SPSState;         // 0 = off, 1 = running, 2 = shutting down, 3 = suspended during sleep
    mDNSInterfaceID SPSProxyListChanged;
    UDPSocket * SPSSocket;
#ifndef SPC_DISABLED
    ServiceRecordSet SPSRecords;
#endif
    mDNSQuestionCallback * SPSBrowseCallback; // So the platform layer can do something useful with SPS browse results
    int ProxyRecords;                         // Total number of records we're holding as proxy
#define MAX_PROXY_RECORDS 10000               /* DOS protection: 400 machines at 25 records each */

#if APPLE_OSX_mDNSResponder
    ClientTunnel * TunnelClients;
    uuid_t asl_uuid; // uuid for ASL logging
    void * WCF;
#endif
    // DNS Proxy fields
    mDNSu32 dp_ipintf[MaxIp]; // input interface index list from the DNS Proxy Client
    mDNSu32 dp_opintf;        // output interface index from the DNS Proxy Client

    TrustAnchor * TrustAnchors;
    int notifyToken;
    int uds_listener_skt;       // Listening socket for incoming UDS clients
    mDNSBool mDNSOppCaching;    // Opportunistic Caching
    mDNSu32 AutoTargetServices; // # of services that have AutoTarget set
    DNSSECStatistics DNSSECStats;
    mDNSStatistics mDNSStats;

    // Fixed storage, to avoid creating large objects on the stack
    // The imsg is declared as a union with a pointer type to enforce CPU-appropriate alignment
    union
    {
        DNSMessage m;
        void * p;
    } imsg;               // Incoming message received from wire
    DNSMessage omsg;      // Outgoing message we're building
    LargeCacheRecord rec; // Resource Record extracted from received message
};

#define FORALL_CACHERECORDS(SLOT, CG, CR)                                                                                          \
    for ((SLOT) = 0; (SLOT) < CACHE_HASH_SLOTS; (SLOT)++)                                                                          \
        for ((CG) = m->rrcache_hash[(SLOT)]; (CG); (CG) = (CG)->next)                                                              \
            for ((CR) = (CG)->members; (CR); (CR) = (CR)->next)

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Useful Static Constants
#endif

extern const mDNSInterfaceID mDNSInterface_Any;       // Zero
extern const mDNSInterfaceID mDNSInterface_LocalOnly; // Special value
extern const mDNSInterfaceID mDNSInterface_Unicast;   // Special value
extern const mDNSInterfaceID mDNSInterfaceMark;       // Special value
extern const mDNSInterfaceID mDNSInterface_P2P;       // Special value
extern const mDNSInterfaceID uDNSInterfaceMark;       // Special value

extern const mDNSIPPort DiscardPort;
extern const mDNSIPPort SSHPort;
extern const mDNSIPPort UnicastDNSPort;
extern const mDNSIPPort SSDPPort;
extern const mDNSIPPort IPSECPort;
extern const mDNSIPPort NSIPCPort;
#ifndef REMOVE_NAT_FUCTIONALITY
extern const mDNSIPPort NATPMPAnnouncementPort;
extern const mDNSIPPort NATPMPPort;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
extern const mDNSIPPort DNSEXTPort;
extern const mDNSIPPort MulticastDNSPort;
extern const mDNSIPPort LoopbackIPCPort;
extern const mDNSIPPort PrivateDNSPort;

extern const OwnerOptData zeroOwner;

extern const mDNSIPPort zeroIPPort;
extern const mDNSv4Addr zerov4Addr;
extern const mDNSv6Addr zerov6Addr;
extern const mDNSEthAddr zeroEthAddr;
extern const mDNSv4Addr onesIPv4Addr;
extern const mDNSv6Addr onesIPv6Addr;
extern const mDNSEthAddr onesEthAddr;
extern const mDNSAddr zeroAddr;

extern const mDNSv4Addr AllDNSAdminGroup;
extern const mDNSv4Addr AllHosts_v4;
extern const mDNSv6Addr AllHosts_v6;
extern const mDNSv6Addr NDP_prefix;
extern const mDNSEthAddr AllHosts_v6_Eth;
extern const mDNSAddr AllDNSLinkGroup_v4;
extern const mDNSAddr AllDNSLinkGroup_v6;

extern const mDNSOpaque16 zeroID;
extern const mDNSOpaque16 onesID;
extern const mDNSOpaque16 QueryFlags;
extern const mDNSOpaque16 uQueryFlags;
extern const mDNSOpaque16 DNSSecQFlags;
extern const mDNSOpaque16 ResponseFlags;
extern const mDNSOpaque16 UpdateReqFlags;
extern const mDNSOpaque16 UpdateRespFlags;

extern const mDNSOpaque64 zeroOpaque64;

extern mDNSBool StrictUnicastOrdering;
extern mDNSu8 NumUnicastDNSServers;

#define localdomain                                                                                                                \
    (*(const domainname *) "\x5"                                                                                                   \
                           "local")
#define DeviceInfoName                                                                                                             \
    (*(const domainname *) "\xC"                                                                                                   \
                           "_device-info"                                                                                          \
                           "\x4"                                                                                                   \
                           "_tcp")
#define LocalDeviceInfoName                                                                                                        \
    (*(const domainname *) "\xC"                                                                                                   \
                           "_device-info"                                                                                          \
                           "\x4"                                                                                                   \
                           "_tcp"                                                                                                  \
                           "\x5"                                                                                                   \
                           "local")
#define SleepProxyServiceType                                                                                                      \
    (*(const domainname *) "\xC"                                                                                                   \
                           "_sleep-proxy"                                                                                          \
                           "\x4"                                                                                                   \
                           "_udp")

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Inline functions
#endif

#if (defined(_MSC_VER))
#define mDNSinline static __inline
#elif ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ >= 9)))
#define mDNSinline static inline
// for Keil
#else
#define mDNSinline static inline
#endif

// If we're not doing inline functions, then this header needs to have the extern declarations
#if !defined(mDNSinline)
extern mDNSs32 NonZeroTime(mDNSs32 t);
extern mDNSu16 mDNSVal16(mDNSOpaque16 x);
extern mDNSOpaque16 mDNSOpaque16fromIntVal(mDNSu16 v);
#endif

// If we're compiling the particular C file that instantiates our inlines, then we
// define "mDNSinline" (to empty string) so that we generate code in the following section
#if (!defined(mDNSinline) && mDNS_InstantiateInlines)
#define mDNSinline
#endif

#ifdef mDNSinline

mDNSinline mDNSs32 NonZeroTime(mDNSs32 t)
{
    if (t)
        return (t);
    else
        return (1);
}

mDNSinline mDNSu16 mDNSVal16(mDNSOpaque16 x)
{
    return ((mDNSu16) ((mDNSu16) x.b[0] << 8 | (mDNSu16) x.b[1]));
}

mDNSinline mDNSOpaque16 mDNSOpaque16fromIntVal(mDNSu16 v)
{
    mDNSOpaque16 x;
    x.b[0] = (mDNSu8) (v >> 8);
    x.b[1] = (mDNSu8) (v & 0xFF);
    return (x);
}

#endif

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Main Client Functions
#endif

// Every client should call mDNS_Init, passing in storage for the mDNS object and the mDNS_PlatformSupport object.
//
// Clients that are only advertising services should use mDNS_Init_NoCache and mDNS_Init_ZeroCacheSize.
// Clients that plan to perform queries (mDNS_StartQuery, mDNS_StartBrowse, mDNS_StartResolveService, etc.)
// need to provide storage for the resource record cache, or the query calls will return 'mStatus_NoCache'.
// The rrcachestorage parameter is the address of memory for the resource record cache, and
// the rrcachesize parameter is the number of entries in the CacheRecord array passed in.
// (i.e. the size of the cache memory needs to be sizeof(CacheRecord) * rrcachesize).
// OS X 10.3 Panther uses an initial cache size of 64 entries, and then mDNSCore sends an
// mStatus_GrowCache message if it needs more.
//
// Most clients should use mDNS_Init_AdvertiseLocalAddresses. This causes mDNSCore to automatically
// create the correct address records for all the hosts interfaces. If you plan to advertise
// services being offered by the local machine, this is almost always what you want.
// There are two cases where you might use mDNS_Init_DontAdvertiseLocalAddresses:
// 1. A client-only device, that browses for services but doesn't advertise any of its own.
// 2. A proxy-registration service, that advertises services being offered by other machines, and takes
//    the appropriate steps to manually create the correct address records for those other machines.
// In principle, a proxy-like registration service could manually create address records for its own machine too,
// but this would be pointless extra effort when using mDNS_Init_AdvertiseLocalAddresses does that for you.
//
// Note that a client-only device that wishes to prohibit multicast advertisements (e.g. from
// higher-layer API calls) must also set DivertMulticastAdvertisements in the mDNS structure and
// advertise local address(es) on a loopback interface.
//
// When mDNS has finished setting up the client's callback is called
// A client can also spin and poll the mDNSPlatformStatus field to see when it changes from mStatus_Waiting to mStatus_NoError
//
// Call mDNS_StartExit to tidy up before exiting
// Because exiting may be an asynchronous process (e.g. if unicast records need to be deregistered)
// client layer may choose to wait until mDNS_ExitNow() returns true before calling mDNS_FinalExit().
//
// Call mDNS_Register with a completed AuthRecord object to register a resource record
// If the resource record type is kDNSRecordTypeUnique (or kDNSknownunique) then if a conflicting resource record is discovered,
// the resource record's mDNSRecordCallback will be called with error code mStatus_NameConflict. The callback should deregister
// the record, and may then try registering the record again after picking a new name (e.g. by automatically appending a number).
// Following deregistration, the RecordCallback will be called with result mStatus_MemFree to signal that it is safe to deallocate
// the record's storage (memory must be freed asynchronously to allow for goodbye packets and dynamic update deregistration).
//
// Call mDNS_StartQuery to initiate a query. mDNS will proceed to issue Multicast DNS query packets, and any time a response
// is received containing a record which matches the question, the DNSQuestion's mDNSAnswerCallback function will be called
// Call mDNS_StopQuery when no more answers are required
//
// Care should be taken on multi-threaded or interrupt-driven environments.
// The main mDNS routines call mDNSPlatformLock() on entry and mDNSPlatformUnlock() on exit;
// each platform layer needs to implement these appropriately for its respective platform.
// For example, if the support code on a particular platform implements timer callbacks at interrupt time, then
// mDNSPlatformLock/Unlock need to disable interrupts or do similar concurrency control to ensure that the mDNS
// code is not entered by an interrupt-time timer callback while in the middle of processing a client call.

extern mStatus mDNS_Init(mDNS * const m, mDNS_PlatformSupport * const p, CacheEntity * rrcachestorage, mDNSu32 rrcachesize,
                         mDNSBool AdvertiseLocalAddresses, mDNSCallback * Callback, void * Context);
// See notes above on use of NoCache/ZeroCacheSize
#define mDNS_Init_NoCache mDNSNULL
#define mDNS_Init_ZeroCacheSize 0
// See notes above on use of Advertise/DontAdvertiseLocalAddresses
#define mDNS_Init_AdvertiseLocalAddresses mDNStrue
#define mDNS_Init_DontAdvertiseLocalAddresses mDNSfalse
#define mDNS_Init_NoInitCallback mDNSNULL
#define mDNS_Init_NoInitCallbackContext mDNSNULL

extern void mDNS_ConfigChanged(mDNS * const m);
extern void mDNS_GrowCache(mDNS * const m, CacheEntity * storage, mDNSu32 numrecords);
extern void mDNS_GrowAuth(mDNS * const m, AuthEntity * storage, mDNSu32 numrecords);
extern void mDNS_StartExit(mDNS * const m);
extern void mDNS_FinalExit(mDNS * const m);
#define mDNS_Close(m)                                                                                                              \
    do                                                                                                                             \
    {                                                                                                                              \
        mDNS_StartExit(m);                                                                                                         \
        mDNS_FinalExit(m);                                                                                                         \
    } while (0)
#define mDNS_ExitNow(m, now) ((now) - (m)->ShutdownTime >= 0 || (!(m)->ResourceRecords))

extern mDNSs32 mDNS_Execute(mDNS * const m);

extern mStatus mDNS_Register(mDNS * const m, AuthRecord * const rr);
extern mStatus mDNS_Update(mDNS * const m, AuthRecord * const rr, mDNSu32 newttl, const mDNSu16 newrdlength, RData * const newrdata,
                           mDNSRecordUpdateCallback * Callback);
extern mStatus mDNS_Deregister(mDNS * const m, AuthRecord * const rr);

extern mStatus mDNS_StartQuery(mDNS * const m, DNSQuestion * const question);
extern mStatus mDNS_StopQuery(mDNS * const m, DNSQuestion * const question);
extern mStatus mDNS_StopQueryWithRemoves(mDNS * const m, DNSQuestion * const question);
extern mStatus mDNS_Reconfirm(mDNS * const m, CacheRecord * const cacherr);
extern mStatus mDNS_Reconfirm_internal(mDNS * const m, CacheRecord * const rr, mDNSu32 interval);
extern mStatus mDNS_ReconfirmByValue(mDNS * const m, ResourceRecord * const rr);
extern void mDNS_PurgeCacheResourceRecord(mDNS * const m, CacheRecord * rr);
extern mDNSs32 mDNS_TimeNow(const mDNS * const m);

#ifndef REMOVE_NAT_FUCTIONALITY
extern mStatus mDNS_StartNATOperation(mDNS * const m, NATTraversalInfo * traversal);
extern mStatus mDNS_StopNATOperation(mDNS * const m, NATTraversalInfo * traversal);
extern mStatus mDNS_StopNATOperation_internal(mDNS * m, NATTraversalInfo * traversal);
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
extern DomainAuthInfo * GetAuthInfoForName(mDNS * m, const domainname * const name);

extern void mDNS_UpdateAllowSleep(mDNS * const m);

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Platform support functions that are accessible to the client layer too
#endif

extern mDNSs32 mDNSPlatformOneSecond;

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - General utility and helper functions
#endif

// mDNS_Dereg_normal is used for most calls to mDNS_Deregister_internal
// mDNS_Dereg_rapid is used to send one goodbye instead of three, when we want the memory available for reuse sooner
// mDNS_Dereg_conflict is used to indicate that this record is being forcibly deregistered because of a conflict
// mDNS_Dereg_repeat is used when cleaning up, for records that may have already been forcibly deregistered
typedef enum
{
    mDNS_Dereg_normal,
    mDNS_Dereg_rapid,
    mDNS_Dereg_conflict,
    mDNS_Dereg_repeat
} mDNS_Dereg_type;

// mDNS_RegisterService is a single call to register the set of resource records associated with a given named service.
//
// mDNS_StartResolveService is single call which is equivalent to multiple calls to mDNS_StartQuery,
// to find the IP address, port number, and demultiplexing information for a given named service.
// As with mDNS_StartQuery, it executes asynchronously, and calls the ServiceInfoQueryCallback when the answer is
// found. After the service is resolved, the client should call mDNS_StopResolveService to complete the transaction.
// The client can also call mDNS_StopResolveService at any time to abort the transaction.
//
// mDNS_AddRecordToService adds an additional record to a Service Record Set.  This record may be deregistered
// via mDNS_RemoveRecordFromService, or by deregistering the service.  mDNS_RemoveRecordFromService is passed a
// callback to free the memory associated with the extra RR when it is safe to do so.  The ExtraResourceRecord
// object can be found in the record's context pointer.

// mDNS_GetBrowseDomains is a special case of the mDNS_StartQuery call, where the resulting answers
// are a list of PTR records indicating (in the rdata) domains that are recommended for browsing.
// After getting the list of domains to browse, call mDNS_StopQuery to end the search.
// mDNS_GetDefaultBrowseDomain returns the name of the domain that should be highlighted by default.
//
// mDNS_GetRegistrationDomains and mDNS_GetDefaultRegistrationDomain are the equivalent calls to get the list
// of one or more domains that should be offered to the user as choices for where they may register their service,
// and the default domain in which to register in the case where the user has made no selection.

extern void mDNS_SetupResourceRecord(AuthRecord * rr, RData * RDataStorage, mDNSInterfaceID InterfaceID, mDNSu16 rrtype,
                                     mDNSu32 ttl, mDNSu8 RecordType, AuthRecType artype, mDNSRecordCallback Callback,
                                     void * Context);

// mDNS_RegisterService() flags parameter bit definitions.
// Note these are only defined to transfer the corresponding DNSServiceFlags settings into mDNSCore routines,
// since code in mDNSCore does not include the DNSServiceFlags definitions in dns_sd.h.
enum
{
    coreFlagIncludeP2P  = 0x1, // include P2P interfaces when using mDNSInterface_Any
    coreFlagIncludeAWDL = 0x2, // include AWDL interface when using mDNSInterface_Any
    coreFlagKnownUnique = 0x4, // client guarantees that SRV and TXT record names are unique
    coreFlagWakeOnly    = 0x8  // Service won't be registered with sleep proxy
};

extern mStatus mDNS_RegisterService(mDNS * const m, ServiceRecordSet * sr, const domainlabel * const name,
                                    const domainname * const type, const domainname * const domain, const domainname * const host,
                                    mDNSIPPort port, const mDNSu8 txtinfo[], mDNSu16 txtlen, AuthRecord * SubTypes,
                                    mDNSu32 NumSubTypes, mDNSInterfaceID InterfaceID, mDNSServiceCallback Callback, void * Context,
                                    mDNSu32 flags);
extern mStatus mDNS_AddRecordToService(mDNS * const m, ServiceRecordSet * sr, ExtraResourceRecord * extra, RData * rdata,
                                       mDNSu32 ttl, mDNSu32 flags);
extern mStatus mDNS_RemoveRecordFromService(mDNS * const m, ServiceRecordSet * sr, ExtraResourceRecord * extra,
                                            mDNSRecordCallback MemFreeCallback, void * Context);
extern mStatus mDNS_RenameAndReregisterService(mDNS * const m, ServiceRecordSet * const sr, const domainlabel * newname);
extern mStatus mDNS_DeregisterService_drt(mDNS * const m, ServiceRecordSet * sr, mDNS_Dereg_type drt);
#define mDNS_DeregisterService(M, S) mDNS_DeregisterService_drt((M), (S), mDNS_Dereg_normal)

extern mStatus mDNS_RegisterNoSuchService(mDNS * const m, AuthRecord * const rr, const domainlabel * const name,
                                          const domainname * const type, const domainname * const domain,
                                          const domainname * const host, const mDNSInterfaceID InterfaceID,
                                          mDNSRecordCallback Callback, void * Context, mDNSu32 flags);
#define mDNS_DeregisterNoSuchService mDNS_Deregister

extern void mDNS_SetupQuestion(DNSQuestion * const q, const mDNSInterfaceID InterfaceID, const domainname * const name,
                               const mDNSu16 qtype, mDNSQuestionCallback * const callback, void * const context);

extern mStatus mDNS_StartBrowse(mDNS * const m, DNSQuestion * const question, const domainname * const srv,
                                const domainname * const domain, const mDNSu8 * anondata, const mDNSInterfaceID InterfaceID,
                                mDNSu32 flags, mDNSBool ForceMCast, mDNSBool useBackgroundTrafficClass,
                                mDNSQuestionCallback * Callback, void * Context);
#define mDNS_StopBrowse mDNS_StopQuery

extern mStatus mDNS_StartResolveService(mDNS * const m, ServiceInfoQuery * query, ServiceInfo * info,
                                        mDNSServiceInfoQueryCallback * Callback, void * Context);
extern void mDNS_StopResolveService(mDNS * const m, ServiceInfoQuery * query);

typedef enum
{
    mDNS_DomainTypeBrowse              = 0,
    mDNS_DomainTypeBrowseDefault       = 1,
    mDNS_DomainTypeBrowseAutomatic     = 2,
    mDNS_DomainTypeRegistration        = 3,
    mDNS_DomainTypeRegistrationDefault = 4,

    mDNS_DomainTypeMax = 4
} mDNS_DomainType;

extern const char * const mDNS_DomainTypeNames[];

extern mStatus mDNS_GetDomains(mDNS * const m, DNSQuestion * const question, mDNS_DomainType DomainType, const domainname * dom,
                               const mDNSInterfaceID InterfaceID, mDNSQuestionCallback * Callback, void * Context);
#define mDNS_StopGetDomains mDNS_StopQuery
extern mStatus mDNS_AdvertiseDomains(mDNS * const m, AuthRecord * rr, mDNS_DomainType DomainType, const mDNSInterfaceID InterfaceID,
                                     char * domname);
#define mDNS_StopAdvertiseDomains mDNS_Deregister

extern mDNSOpaque16 mDNS_NewMessageID(mDNS * const m);
extern mDNSBool mDNS_AddressIsLocalSubnet(mDNS * const m, const mDNSInterfaceID InterfaceID, const mDNSAddr * addr,
                                          mDNSBool * myself);

extern DNSServer * GetServerForQuestion(mDNS * m, DNSQuestion * question);
extern mDNSu32 SetValidDNSServers(mDNS * m, DNSQuestion * question);

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - DNS name utility functions
#endif

// In order to expose the full capabilities of the DNS protocol (which allows any arbitrary eight-bit values
// in domain name labels, including unlikely characters like ascii nulls and even dots) all the mDNS APIs
// work with DNS's native length-prefixed strings. For convenience in C, the following utility functions
// are provided for converting between C's null-terminated strings and DNS's length-prefixed strings.

// Assignment
// A simple C structure assignment of a domainname can cause a protection fault by accessing unmapped memory,
// because that object is defined to be 256 bytes long, but not all domainname objects are truly the full size.
// This macro uses mDNSPlatformMemCopy() to make sure it only touches the actual bytes that are valid.
#define AssignDomainName(DST, SRC)                                                                                                 \
    do                                                                                                                             \
    {                                                                                                                              \
        mDNSu16 len__ = DomainNameLength((SRC));                                                                                   \
        if (len__ <= MAX_DOMAIN_NAME)                                                                                              \
            mDNSPlatformMemCopy((DST)->c, (SRC)->c, len__);                                                                        \
        else                                                                                                                       \
            (DST)->c[0] = 0;                                                                                                       \
    } while (0)

// Comparison functions
#define SameDomainLabelCS(A, B) ((A)[0] == (B)[0] && mDNSPlatformMemSame((A) + 1, (B) + 1, (A)[0]))
extern mDNSBool SameDomainLabel(const mDNSu8 * a, const mDNSu8 * b);
extern mDNSBool SameDomainName(const domainname * const d1, const domainname * const d2);
extern mDNSBool SameDomainNameCS(const domainname * const d1, const domainname * const d2);
typedef mDNSBool DomainNameComparisonFn(const domainname * const d1, const domainname * const d2);
extern mDNSBool
IsLocalDomain(const domainname * d); // returns true for domains that by default should be looked up using link-local multicast

#define StripFirstLabel(X) ((const domainname *) &(X)->c[(X)->c[0] ? 1 + (X)->c[0] : 0])

#define FirstLabel(X) ((const domainlabel *) (X))
#define SecondLabel(X) ((const domainlabel *) StripFirstLabel(X))
#define ThirdLabel(X) ((const domainlabel *) StripFirstLabel(StripFirstLabel(X)))

extern const mDNSu8 * LastLabel(const domainname * d);

// Get total length of domain name, in native DNS format, including terminal root label
//   (e.g. length of "com." is 5 (length byte, three data bytes, final zero)
extern mDNSu16 DomainNameLengthLimit(const domainname * const name, const mDNSu8 * limit);
#define DomainNameLength(name) DomainNameLengthLimit((name), (name)->c + MAX_DOMAIN_NAME)

// Append functions to append one or more labels to an existing native format domain name:
//   AppendLiteralLabelString adds a single label from a literal C string, with no escape character interpretation.
//   AppendDNSNameString      adds zero or more labels from a C string using conventional DNS dots-and-escaping interpretation
//   AppendDomainLabel        adds a single label from a native format domainlabel
//   AppendDomainName         adds zero or more labels from a native format domainname
extern mDNSu8 * AppendLiteralLabelString(domainname * const name, const char * cstr);
extern mDNSu8 * AppendDNSNameString(domainname * const name, const char * cstr);
extern mDNSu8 * AppendDomainLabel(domainname * const name, const domainlabel * const label);
extern mDNSu8 * AppendDomainName(domainname * const name, const domainname * const append);

// Convert from null-terminated string to native DNS format:
//   The DomainLabel form makes a single label from a literal C string, with no escape character interpretation.
//   The DomainName form makes native format domain name from a C string using conventional DNS interpretation:
//     dots separate labels, and within each label, '\.' represents a literal dot, '\\' represents a literal
//     backslash and backslash with three decimal digits (e.g. \000) represents an arbitrary byte value.
extern mDNSBool MakeDomainLabelFromLiteralString(domainlabel * const label, const char * cstr);
extern mDNSu8 * MakeDomainNameFromDNSNameString(domainname * const name, const char * cstr);

// Convert native format domainlabel or domainname back to C string format
// IMPORTANT:
// When using ConvertDomainLabelToCString, the target buffer must be MAX_ESCAPED_DOMAIN_LABEL (254) bytes long
// to guarantee there will be no buffer overrun. It is only safe to use a buffer shorter than this in rare cases
// where the label is known to be constrained somehow (for example, if the label is known to be either "_tcp" or "_udp").
// Similarly, when using ConvertDomainNameToCString, the target buffer must be MAX_ESCAPED_DOMAIN_NAME (1009) bytes long.
// See definitions of MAX_ESCAPED_DOMAIN_LABEL and MAX_ESCAPED_DOMAIN_NAME for more detailed explanation.
extern char * ConvertDomainLabelToCString_withescape(const domainlabel * const name, char * cstr, char esc);
#define ConvertDomainLabelToCString_unescaped(D, C) ConvertDomainLabelToCString_withescape((D), (C), 0)
#define ConvertDomainLabelToCString(D, C) ConvertDomainLabelToCString_withescape((D), (C), '\\')
extern char * ConvertDomainNameToCString_withescape(const domainname * const name, char * cstr, char esc);
#define ConvertDomainNameToCString_unescaped(D, C) ConvertDomainNameToCString_withescape((D), (C), 0)
#define ConvertDomainNameToCString(D, C) ConvertDomainNameToCString_withescape((D), (C), '\\')

extern void ConvertUTF8PstringToRFC1034HostLabel(const mDNSu8 UTF8Name[], domainlabel * const hostlabel);

extern mDNSu8 * ConstructServiceName(domainname * const fqdn, const domainlabel * name, const domainname * type,
                                     const domainname * const domain);
extern mDNSBool DeconstructServiceName(const domainname * const fqdn, domainlabel * const name, domainname * const type,
                                       domainname * const domain);

// Note: Some old functions have been replaced by more sensibly-named versions.
// You can uncomment the hash-defines below if you don't want to have to change your source code right away.
// When updating your code, note that (unlike the old versions) *all* the new routines take the target object
// as their first parameter.
//#define ConvertCStringToDomainName(SRC,DST)  MakeDomainNameFromDNSNameString((DST),(SRC))
//#define ConvertCStringToDomainLabel(SRC,DST) MakeDomainLabelFromLiteralString((DST),(SRC))
//#define AppendStringLabelToName(DST,SRC)     AppendLiteralLabelString((DST),(SRC))
//#define AppendStringNameToName(DST,SRC)      AppendDNSNameString((DST),(SRC))
//#define AppendDomainLabelToName(DST,SRC)     AppendDomainLabel((DST),(SRC))
//#define AppendDomainNameToName(DST,SRC)      AppendDomainName((DST),(SRC))

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Other utility functions and macros
#endif

// mDNS_vsnprintf/snprintf return the number of characters written, excluding the final terminating null.
// The output is always null-terminated: for example, if the output turns out to be exactly buflen long,
// then the output will be truncated by one character to allow space for the terminating null.
// Unlike standard C vsnprintf/snprintf, they return the number of characters *actually* written,
// not the number of characters that *would* have been printed were buflen unlimited.
extern mDNSu32 mDNS_vsnprintf(char * sbuffer, mDNSu32 buflen, const char * fmt, va_list arg);
extern mDNSu32 mDNS_snprintf(char * sbuffer, mDNSu32 buflen, const char * fmt, ...) IS_A_PRINTF_STYLE_FUNCTION(3, 4);
extern mDNSu32 NumCacheRecordsForInterfaceID(const mDNS * const m, mDNSInterfaceID id);
extern char * DNSTypeName(mDNSu16 rrtype);
extern char * GetRRDisplayString_rdb(const ResourceRecord * const rr, const RDataBody * const rd1, char * const buffer);
#define RRDisplayString(m, rr) GetRRDisplayString_rdb(rr, &(rr)->rdata->u, (m)->MsgBuffer)
#define ARDisplayString(m, rr) GetRRDisplayString_rdb(&(rr)->resrec, &(rr)->resrec.rdata->u, (m)->MsgBuffer)
#define CRDisplayString(m, rr) GetRRDisplayString_rdb(&(rr)->resrec, &(rr)->resrec.rdata->u, (m)->MsgBuffer)
extern mDNSBool mDNSSameAddress(const mDNSAddr * ip1, const mDNSAddr * ip2);
extern void IncrementLabelSuffix(domainlabel * name, mDNSBool RichText);
extern mDNSBool mDNSv4AddrIsRFC1918(const mDNSv4Addr * const addr); // returns true for RFC1918 private addresses
#define mDNSAddrIsRFC1918(X) ((X)->type == mDNSAddrType_IPv4 && mDNSv4AddrIsRFC1918(&(X)->ip.v4))

// For PCP
extern void mDNSAddrMapIPv4toIPv6(mDNSv4Addr * in, mDNSv6Addr * out);
extern mDNSBool mDNSAddrIPv4FromMappedIPv6(mDNSv6Addr * in, mDNSv4Addr * out);

#define mDNSSameIPPort(A, B) ((A).NotAnInteger == (B).NotAnInteger)
#define mDNSSameOpaque16(A, B) ((A).NotAnInteger == (B).NotAnInteger)
#define mDNSSameOpaque32(A, B) ((A).NotAnInteger == (B).NotAnInteger)
#define mDNSSameOpaque64(A, B) ((A)->l[0] == (B)->l[0] && (A)->l[1] == (B)->l[1])

#define mDNSSameIPv4Address(A, B) ((A).NotAnInteger == (B).NotAnInteger)
#define mDNSSameIPv6Address(A, B) ((A).l[0] == (B).l[0] && (A).l[1] == (B).l[1] && (A).l[2] == (B).l[2] && (A).l[3] == (B).l[3])
#define mDNSSameIPv6NetworkPart(A, B) ((A).l[0] == (B).l[0] && (A).l[1] == (B).l[1])
#define mDNSSameEthAddress(A, B) ((A)->w[0] == (B)->w[0] && (A)->w[1] == (B)->w[1] && (A)->w[2] == (B)->w[2])

#define mDNSIPPortIsZero(A) ((A).NotAnInteger == 0)
#define mDNSOpaque16IsZero(A) ((A).NotAnInteger == 0)
#define mDNSOpaque64IsZero(A) (((A)->l[0] | (A)->l[1]) == 0)
#define mDNSIPv4AddressIsZero(A) ((A).NotAnInteger == 0)
#define mDNSIPv6AddressIsZero(A) (((A).l[0] | (A).l[1] | (A).l[2] | (A).l[3]) == 0)
#define mDNSEthAddressIsZero(A) (((A).w[0] | (A).w[1] | (A).w[2]) == 0)

#define mDNSIPv4AddressIsOnes(A) ((A).NotAnInteger == 0xFFFFFFFF)
#define mDNSIPv6AddressIsOnes(A) (((A).l[0] & (A).l[1] & (A).l[2] & (A).l[3]) == 0xFFFFFFFF)

#define mDNSAddressIsAllDNSLinkGroup(X)                                                                                            \
    (((X)->type == mDNSAddrType_IPv4 && mDNSSameIPv4Address((X)->ip.v4, AllDNSLinkGroup_v4.ip.v4)) ||                              \
     ((X)->type == mDNSAddrType_IPv6 && mDNSSameIPv6Address((X)->ip.v6, AllDNSLinkGroup_v6.ip.v6)))

#define mDNSAddressIsZero(X)                                                                                                       \
    (((X)->type == mDNSAddrType_IPv4 && mDNSIPv4AddressIsZero((X)->ip.v4)) ||                                                      \
     ((X)->type == mDNSAddrType_IPv6 && mDNSIPv6AddressIsZero((X)->ip.v6)))

#define mDNSAddressIsValidNonZero(X)                                                                                               \
    (((X)->type == mDNSAddrType_IPv4 && !mDNSIPv4AddressIsZero((X)->ip.v4)) ||                                                     \
     ((X)->type == mDNSAddrType_IPv6 && !mDNSIPv6AddressIsZero((X)->ip.v6)))

#define mDNSAddressIsOnes(X)                                                                                                       \
    (((X)->type == mDNSAddrType_IPv4 && mDNSIPv4AddressIsOnes((X)->ip.v4)) ||                                                      \
     ((X)->type == mDNSAddrType_IPv6 && mDNSIPv6AddressIsOnes((X)->ip.v6)))

#define mDNSAddressIsValid(X)                                                                                                      \
    (((X)->type == mDNSAddrType_IPv4)       ? !(mDNSIPv4AddressIsZero((X)->ip.v4) || mDNSIPv4AddressIsOnes((X)->ip.v4))            \
         : ((X)->type == mDNSAddrType_IPv6) ? !(mDNSIPv6AddressIsZero((X)->ip.v6) || mDNSIPv6AddressIsOnes((X)->ip.v6))            \
                                            : mDNSfalse)

#define mDNSv4AddressIsLinkLocal(X) ((X)->b[0] == 169 && (X)->b[1] == 254)
#define mDNSv6AddressIsLinkLocal(X) ((X)->b[0] == 0xFE && ((X)->b[1] & 0xC0) == 0x80)

#define mDNSAddressIsLinkLocal(X)                                                                                                  \
    (((X)->type == mDNSAddrType_IPv4)       ? mDNSv4AddressIsLinkLocal(&(X)->ip.v4)                                                \
         : ((X)->type == mDNSAddrType_IPv6) ? mDNSv6AddressIsLinkLocal(&(X)->ip.v6)                                                \
                                            : mDNSfalse)

#define mDNSv4AddressIsLoopback(X) ((X)->b[0] == 127 && (X)->b[1] == 0 && (X)->b[2] == 0 && (X)->b[3] == 1)
#define mDNSv6AddressIsLoopback(X)                                                                                                 \
    ((((X)->l[0] | (X)->l[1] | (X)->l[2]) == 0) && ((X)->b[12] == 0 && (X)->b[13] == 0 && (X)->b[14] == 0 && (X)->b[15] == 1))

#define mDNSAddressIsLoopback(X)                                                                                                   \
    (((X)->type == mDNSAddrType_IPv4)       ? mDNSv4AddressIsLoopback(&(X)->ip.v4)                                                 \
         : ((X)->type == mDNSAddrType_IPv6) ? mDNSv6AddressIsLoopback(&(X)->ip.v6)                                                 \
                                            : mDNSfalse)

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Authentication Support
#endif

// Unicast DNS and Dynamic Update specific Client Calls
//
// mDNS_SetSecretForDomain tells the core to authenticate (via TSIG with an HMAC_MD5 hash of the shared secret)
// when dynamically updating a given zone (and its subdomains).  The key used in authentication must be in
// domain name format.  The shared secret must be a null-terminated base64 encoded string.  A minimum size of
// 16 bytes (128 bits) is recommended for an MD5 hash as per RFC 2485.
// Calling this routine multiple times for a zone replaces previously entered values.  Call with a NULL key
// to disable authentication for the zone.  A non-NULL autoTunnelPrefix means this is an AutoTunnel domain,
// and the value is prepended to the IPSec identifier (used for key lookup)

extern mStatus mDNS_SetSecretForDomain(mDNS * m, DomainAuthInfo * info, const domainname * domain, const domainname * keyname,
                                       const char * b64keydata, const domainname * hostname, mDNSIPPort * port,
                                       mDNSBool autoTunnel);
#ifndef REMOVE_NAT_FUCTIONALITY
extern void RecreateNATMappings(mDNS * const m, const mDNSu32 waitTicks);
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
// Hostname/Unicast Interface Configuration

// All hostnames advertised point to one IPv4 address and/or one IPv6 address, set via SetPrimaryInterfaceInfo.  Invoking this
// routine updates all existing hostnames to point to the new address.

// A hostname is added via AddDynDNSHostName, which points to the primary interface's v4 and/or v6 addresss

// The status callback is invoked to convey success or failure codes - the callback should not modify the AuthRecord or free memory.
// Added hostnames may be removed (deregistered) via mDNS_RemoveDynDNSHostName.

// Host domains added prior to specification of the primary interface address and computer name will be deferred until
// these values are initialized.

// DNS servers used to resolve unicast queries are specified by mDNS_AddDNSServer.
// For "split" DNS configurations, in which queries for different domains are sent to different servers (e.g. VPN and external),
// a domain may be associated with a DNS server.  For standard configurations, specify the root label (".") or NULL.

extern void mDNS_AddDynDNSHostName(mDNS * m, const domainname * fqdn, mDNSRecordCallback * StatusCallback,
                                   const void * StatusContext);
extern void mDNS_RemoveDynDNSHostName(mDNS * m, const domainname * fqdn);
extern void mDNS_SetPrimaryInterfaceInfo(mDNS * m, const mDNSAddr * v4addr, const mDNSAddr * v6addr, const mDNSAddr * router);
extern DNSServer * mDNS_AddDNSServer(mDNS * const m, const domainname * d, const mDNSInterfaceID interface, mDNSs32 serviceID,
                                     const mDNSAddr * addr, const mDNSIPPort port, mDNSu32 scoped, mDNSu32 timeout,
                                     mDNSBool cellIntf, mDNSu16 resGroupID, mDNSBool reqA, mDNSBool reqAAAA, mDNSBool reqDO);
extern void PenalizeDNSServer(mDNS * const m, DNSQuestion * q, mDNSOpaque16 responseFlags);
extern void mDNS_AddSearchDomain(const domainname * const domain, mDNSInterfaceID InterfaceID);

extern McastResolver * mDNS_AddMcastResolver(mDNS * const m, const domainname * d, const mDNSInterfaceID interface,
                                             mDNSu32 timeout);

// We use ((void *)0) here instead of mDNSNULL to avoid compile warnings on gcc 4.2
#define mDNS_AddSearchDomain_CString(X, I)                                                                                         \
    do                                                                                                                             \
    {                                                                                                                              \
        domainname d__;                                                                                                            \
        if (((X) != (void *) 0) && MakeDomainNameFromDNSNameString(&d__, (X)) && d__.c[0])                                         \
            mDNS_AddSearchDomain(&d__, I);                                                                                         \
    } while (0)

// Routines called by the core, exported by DNSDigest.c

// Convert an arbitrary base64 encoded key key into an HMAC key (stored in AuthInfo struct)
extern mDNSs32 DNSDigest_ConstructHMACKeyfromBase64(DomainAuthInfo * info, const char * b64key);

// sign a DNS message.  The message must be complete, with all values in network byte order.  end points to the end
// of the message, and is modified by this routine.  numAdditionals is a pointer to the number of additional
// records in HOST byte order, which is incremented upon successful completion of this routine.  The function returns
// the new end pointer on success, and NULL on failure.
extern void DNSDigest_SignMessage(DNSMessage * msg, mDNSu8 ** end, DomainAuthInfo * info, mDNSu16 tcode);

#define SwapDNSHeaderBytes(M)                                                                                                      \
    do                                                                                                                             \
    {                                                                                                                              \
        (M)->h.numQuestions   = (mDNSu16) ((mDNSu8 *) &(M)->h.numQuestions)[0] << 8 | ((mDNSu8 *) &(M)->h.numQuestions)[1];        \
        (M)->h.numAnswers     = (mDNSu16) ((mDNSu8 *) &(M)->h.numAnswers)[0] << 8 | ((mDNSu8 *) &(M)->h.numAnswers)[1];            \
        (M)->h.numAuthorities = (mDNSu16) ((mDNSu8 *) &(M)->h.numAuthorities)[0] << 8 | ((mDNSu8 *) &(M)->h.numAuthorities)[1];    \
        (M)->h.numAdditionals = (mDNSu16) ((mDNSu8 *) &(M)->h.numAdditionals)[0] << 8 | ((mDNSu8 *) &(M)->h.numAdditionals)[1];    \
    } while (0)

#define DNSDigest_SignMessageHostByteOrder(M, E, INFO)                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        SwapDNSHeaderBytes(M);                                                                                                     \
        DNSDigest_SignMessage((M), (E), (INFO), 0);                                                                                \
        SwapDNSHeaderBytes(M);                                                                                                     \
    } while (0)

// verify a DNS message.  The message must be complete, with all values in network byte order.  end points to the
// end of the record.  tsig is a pointer to the resource record that contains the TSIG OPT record.  info is
// the matching key to use for verifying the message.  This function expects that the additionals member
// of the DNS message header has already had one subtracted from it.
extern mDNSBool DNSDigest_VerifyMessage(DNSMessage * msg, mDNSu8 * end, LargeCacheRecord * tsig, DomainAuthInfo * info,
                                        mDNSu16 * rcode, mDNSu16 * tcode);

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - PlatformSupport interface
#endif

// This section defines the interface to the Platform Support layer.
// Normal client code should not use any of types defined here, or directly call any of the functions defined here.
// The definitions are placed here because sometimes clients do use these calls indirectly, via other supported client operations.
// For example, AssignDomainName is a macro defined using mDNSPlatformMemCopy()

// Every platform support module must provide the following functions.
// mDNSPlatformInit() typically opens a communication endpoint, and starts listening for mDNS packets.
// When Setup is complete, the platform support layer calls mDNSCoreInitComplete().
// mDNSPlatformSendUDP() sends one UDP packet
// When a packet is received, the PlatformSupport code calls mDNSCoreReceive()
// mDNSPlatformClose() tidies up on exit
//
// Note: mDNSPlatformMemAllocate/mDNSPlatformMemFree are only required for handling oversized resource records and unicast DNS.
// If your target platform has a well-defined specialized application, and you know that all the records it uses
// are InlineCacheRDSize or less, then you can just make a simple mDNSPlatformMemAllocate() stub that always returns
// NULL. InlineCacheRDSize is a compile-time constant, which is set by default to 68. If you need to handle records
// a little larger than this and you don't want to have to implement run-time allocation and freeing, then you
// can raise the value of this constant to a suitable value (at the expense of increased memory usage).
//
// USE CAUTION WHEN CALLING mDNSPlatformRawTime: The m->timenow_adjust correction factor needs to be added
// Generally speaking:
// Code that's protected by the main mDNS lock should just use the m->timenow value
// Code outside the main mDNS lock should use mDNS_TimeNow(m) to get properly adjusted time
// In certain cases there may be reasons why it's necessary to get the time without taking the lock first
// (e.g. inside the routines that are doing the locking and unlocking, where a call to get the lock would result in a
// recursive loop); in these cases use mDNS_TimeNow_NoLock(m) to get mDNSPlatformRawTime with the proper correction factor added.
//
// mDNSPlatformUTC returns the time, in seconds, since Jan 1st 1970 UTC and is required for generating TSIG records

extern mStatus mDNSPlatformInit(mDNS * const m);
extern void mDNSPlatformClose(mDNS * const m);
extern mStatus mDNSPlatformSendUDP(const mDNS * const m, const void * const msg, const mDNSu8 * const end,
                                   mDNSInterfaceID InterfaceID, UDPSocket * src, const mDNSAddr * dst, mDNSIPPort dstport,
                                   mDNSBool useBackgroundTrafficClass);

extern mDNSBool mDNSPlatformPeekUDP(mDNS * const m, UDPSocket * src);
extern void mDNSPlatformLock(const mDNS * const m);
extern void mDNSPlatformUnlock(const mDNS * const m);

extern void mDNSPlatformStrCopy(void * dst, const void * src);
extern mDNSu32 mDNSPlatformStrLen(const void * src);
extern void mDNSPlatformMemCopy(void * dst, const void * src, mDNSu32 len);
extern mDNSBool mDNSPlatformMemSame(const void * dst, const void * src, mDNSu32 len);
extern int mDNSPlatformMemCmp(const void * dst, const void * src, mDNSu32 len);
extern void mDNSPlatformMemZero(void * dst, mDNSu32 len);
extern void mDNSPlatformQsort(void * base, int nel, int width, int (*compar)(const void *, const void *));
#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING
#define mDNSPlatformMemAllocate(X) mallocL(#X, X)
#else
extern void * mDNSPlatformMemAllocate(mDNSu32 len);
#endif
extern void mDNSPlatformMemFree(void * mem);

// If the platform doesn't have a strong PRNG, we define a naive multiply-and-add based on a seed
// from the platform layer.  Long-term, we should embed an arc4 implementation, but the strength
// will still depend on the randomness of the seed.
#if !defined(_PLATFORM_HAS_STRONG_PRNG_) && (_BUILDING_XCODE_PROJECT_ || defined(_WIN32))
#define _PLATFORM_HAS_STRONG_PRNG_ 1
#endif
#if _PLATFORM_HAS_STRONG_PRNG_
extern mDNSu32 mDNSPlatformRandomNumber(void);
#else
extern mDNSu32 mDNSPlatformRandomSeed(void);
#endif // _PLATFORM_HAS_STRONG_PRNG_

extern mStatus mDNSPlatformTimeInit(void);
extern mDNSs32 mDNSPlatformRawTime(void);
extern mDNSs32 mDNSPlatformUTC(void);
#define mDNS_TimeNow_NoLock(m) (mDNSPlatformRawTime() + (m)->timenow_adjust)

#ifndef REMOVE_LOG_FUNCTIONS
#if MDNS_DEBUGMSGS
extern void mDNSPlatformWriteDebugMsg(const char * msg);
#endif
extern void mDNSPlatformWriteLogMsg(const char * ident, const char * msg, mDNSLogLevel_t loglevel);
#endif //#ifndef REMOVE_LOG_FUNCTIONS

#if APPLE_OSX_mDNSResponder
// Utility function for ASL logging
mDNSexport void mDNSASLLog(uuid_t * uuid, const char * subdomain, const char * result, const char * signature, const char * fmt,
                           ...);

// Log unicast and multicast traffic statistics once a day. Also used for DNSSEC statistics.
#define kDefaultNextStatsticsLogTime (24 * 60 * 60)

extern void mDNSLogStatistics(mDNS * const m);

#endif // APPLE_OSX_mDNSResponder

// Platform support modules should provide the following functions to map between opaque interface IDs
// and interface indexes in order to support the DNS-SD API. If your target platform does not support
// multiple interfaces and/or does not support the DNS-SD API, these functions can be empty.
extern mDNSInterfaceID mDNSPlatformInterfaceIDfromInterfaceIndex(mDNS * const m, mDNSu32 ifindex);
extern mDNSu32 mDNSPlatformInterfaceIndexfromInterfaceID(mDNS * const m, mDNSInterfaceID id, mDNSBool suppressNetworkChange);

// Every platform support module must provide the following functions if it is to support unicast DNS
// and Dynamic Update.
// All TCP socket operations implemented by the platform layer MUST NOT BLOCK.
// mDNSPlatformTCPConnect initiates a TCP connection with a peer, adding the socket descriptor to the
// main event loop.  The return value indicates whether the connection succeeded, failed, or is pending
// (i.e. the call would block.)  On return, the descriptor parameter is set to point to the connected socket.
// The TCPConnectionCallback is subsequently invoked when the connection
// completes (in which case the ConnectionEstablished parameter is true), or data is available for
// reading on the socket (indicated by the ConnectionEstablished parameter being false.)  If the connection
// asynchronously fails, the TCPConnectionCallback should be invoked as usual, with the error being
// returned in subsequent calls to PlatformReadTCP or PlatformWriteTCP.  (This allows for platforms
// with limited asynchronous error detection capabilities.)  PlatformReadTCP and PlatformWriteTCP must
// return the number of bytes read/written, 0 if the call would block, and -1 if an error.  PlatformReadTCP
// should set the closed argument if the socket has been closed.
// PlatformTCPCloseConnection must close the connection to the peer and remove the descriptor from the
// event loop.  CloseConnectin may be called at any time, including in a ConnectionCallback.

typedef enum
{
    kTCPSocketFlags_Zero   = 0,
    kTCPSocketFlags_UseTLS = (1 << 0)
} TCPSocketFlags;

typedef void (*TCPConnectionCallback)(TCPSocket * sock, void * context, mDNSBool ConnectionEstablished, mStatus err);
extern TCPSocket * mDNSPlatformTCPSocket(mDNS * const m, TCPSocketFlags flags, mDNSIPPort * port,
                                         mDNSBool useBackgroundTrafficClass); // creates a TCP socket
extern TCPSocket * mDNSPlatformTCPAccept(TCPSocketFlags flags, int sd);
extern int mDNSPlatformTCPGetFD(TCPSocket * sock);
extern mStatus mDNSPlatformTCPConnect(TCPSocket * sock, const mDNSAddr * dst, mDNSOpaque16 dstport, domainname * hostname,
                                      mDNSInterfaceID InterfaceID, TCPConnectionCallback callback, void * context);
extern void mDNSPlatformTCPCloseConnection(TCPSocket * sock);
extern long mDNSPlatformReadTCP(TCPSocket * sock, void * buf, unsigned long buflen, mDNSBool * closed);
extern long mDNSPlatformWriteTCP(TCPSocket * sock, const char * msg, unsigned long len);
extern UDPSocket * mDNSPlatformUDPSocket(mDNS * const m, const mDNSIPPort requestedport);
extern mDNSu16 mDNSPlatformGetUDPPort(UDPSocket * sock);
extern void mDNSPlatformUDPClose(UDPSocket * sock);
extern void mDNSPlatformReceiveBPF_fd(mDNS * const m, int fd);
extern void mDNSPlatformUpdateProxyList(mDNS * const m, const mDNSInterfaceID InterfaceID);
extern void mDNSPlatformSendRawPacket(const void * const msg, const mDNSu8 * const end, mDNSInterfaceID InterfaceID);
extern void mDNSPlatformSetLocalAddressCacheEntry(mDNS * const m, const mDNSAddr * const tpa, const mDNSEthAddr * const tha,
                                                  mDNSInterfaceID InterfaceID);
extern void mDNSPlatformSourceAddrForDest(mDNSAddr * const src, const mDNSAddr * const dst);
extern void mDNSPlatformSendKeepalive(mDNSAddr * sadd, mDNSAddr * dadd, mDNSIPPort * lport, mDNSIPPort * rport, mDNSu32 seq,
                                      mDNSu32 ack, mDNSu16 win);
extern mStatus mDNSPlatformRetrieveTCPInfo(mDNS * const m, mDNSAddr * laddr, mDNSIPPort * lport, mDNSAddr * raddr,
                                           mDNSIPPort * rport, mDNSTCPInfo * mti);
extern mStatus mDNSPlatformGetRemoteMacAddr(mDNS * const m, mDNSAddr * raddr);
extern mStatus mDNSPlatformStoreSPSMACAddr(mDNSAddr * spsaddr, char * ifname);
extern mStatus mDNSPlatformClearSPSMACAddr(void);

// mDNSPlatformTLSSetupCerts/mDNSPlatformTLSTearDownCerts used by dnsextd
extern mStatus mDNSPlatformTLSSetupCerts(void);
extern void mDNSPlatformTLSTearDownCerts(void);

// Platforms that support unicast browsing and dynamic update registration for clients who do not specify a domain
// in browse/registration calls must implement these routines to get the "default" browse/registration list.

extern mDNSBool mDNSPlatformSetDNSConfig(mDNS * const m, mDNSBool setservers, mDNSBool setsearch, domainname * const fqdn,
                                         DNameListElem ** RegDomains, DNameListElem ** BrowseDomains, mDNSBool ackConfig);
extern mStatus mDNSPlatformGetPrimaryInterface(mDNS * const m, mDNSAddr * v4, mDNSAddr * v6, mDNSAddr * router);
extern void mDNSPlatformDynDNSHostNameStatusChanged(const domainname * const dname, const mStatus status);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
extern void mDNSPlatformSetAllowSleep(mDNS * const m, mDNSBool allowSleep, const char * reason);
extern void mDNSPlatformSendWakeupPacket(mDNS * const m, mDNSInterfaceID InterfaceID, char * EthAddr, char * IPAddr, int iteration);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

extern mDNSBool mDNSPlatformInterfaceIsD2D(mDNSInterfaceID InterfaceID);
extern mDNSBool mDNSPlatformInterfaceIsAWDL(const NetworkInterfaceInfo * intf);
extern mDNSBool mDNSPlatformValidRecordForQuestion(const ResourceRecord * const rr, const DNSQuestion * const q);
extern mDNSBool mDNSPlatformValidRecordForInterface(AuthRecord * rr, const NetworkInterfaceInfo * intf);
extern mDNSBool mDNSPlatformValidQuestionForInterface(DNSQuestion * q, const NetworkInterfaceInfo * intf);

extern void mDNSPlatformFormatTime(unsigned long t, mDNSu8 * buf, int bufsize);
#ifndef REMOVE_NAT_FUCTIONALITY
#ifdef _LEGACY_NAT_TRAVERSAL_
// Support for legacy NAT traversal protocols, implemented by the platform layer and callable by the core.
extern void LNT_SendDiscoveryMsg(mDNS * m);
extern void LNT_ConfigureRouterInfo(mDNS * m, const mDNSInterfaceID InterfaceID, const mDNSu8 * const data, const mDNSu16 len);
extern mStatus LNT_GetExternalAddress(mDNS * m);
extern mStatus LNT_MapPort(mDNS * m, NATTraversalInfo * const n);
extern mStatus LNT_UnmapPort(mDNS * m, NATTraversalInfo * const n);
extern void LNT_ClearState(mDNS * const m);
#endif // _LEGACY_NAT_TRAVERSAL_
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
// The core mDNS code provides these functions, for the platform support code to call at appropriate times
//
// mDNS_SetFQDN() is called once on startup (typically from mDNSPlatformInit())
// and then again on each subsequent change of the host name.
//
// mDNS_RegisterInterface() is used by the platform support layer to inform mDNSCore of what
// physical and/or logical interfaces are available for sending and receiving packets.
// Typically it is called on startup for each available interface, but register/deregister may be
// called again later, on multiple occasions, to inform the core of interface configuration changes.
// If set->Advertise is set non-zero, then mDNS_RegisterInterface() also registers the standard
// resource records that should be associated with every publicised IP address/interface:
// -- Name-to-address records (A/AAAA)
// -- Address-to-name records (PTR)
// -- Host information (HINFO)
// IMPORTANT: The specified mDNSInterfaceID MUST NOT be 0, -1, or -2; these values have special meaning
// mDNS_RegisterInterface does not result in the registration of global hostnames via dynamic update -
// see mDNS_SetPrimaryInterfaceInfo, mDNS_AddDynDNSHostName, etc. for this purpose.
// Note that the set may be deallocated immediately after it is deregistered via mDNS_DeegisterInterface.
//
// mDNS_RegisterDNS() is used by the platform support layer to provide the core with the addresses of
// available domain name servers for unicast queries/updates.  RegisterDNS() should be called once for
// each name server, typically at startup, or when a new name server becomes available.  DeregiterDNS()
// must be called whenever a registered name server becomes unavailable.  DeregisterDNSList deregisters
// all registered servers.  mDNS_DNSRegistered() returns true if one or more servers are registered in the core.
//
// mDNSCoreInitComplete() is called when the platform support layer is finished.
// Typically this is at the end of mDNSPlatformInit(), but may be later
// (on platforms like OT that allow asynchronous initialization of the networking stack).
//
// mDNSCoreReceive() is called when a UDP packet is received
//
// mDNSCoreMachineSleep() is called when the machine sleeps or wakes
// (This refers to heavyweight laptop-style sleep/wake that disables network access,
// not lightweight second-by-second CPU power management modes.)

extern void mDNS_SetFQDN(mDNS * const m);
extern void mDNS_ActivateNetWake_internal(mDNS * const m, NetworkInterfaceInfo * set);
extern void mDNS_DeactivateNetWake_internal(mDNS * const m, NetworkInterfaceInfo * set);
extern mStatus mDNS_RegisterInterface(mDNS * const m, NetworkInterfaceInfo * set, mDNSBool flapping);
extern void mDNS_DeregisterInterface(mDNS * const m, NetworkInterfaceInfo * set, mDNSBool flapping);
extern void mDNSCoreInitComplete(mDNS * const m, mStatus result);
extern void mDNSCoreReceive(mDNS * const m, void * const msg, const mDNSu8 * const end, const mDNSAddr * const srcaddr,
                            const mDNSIPPort srcport, const mDNSAddr * dstaddr, const mDNSIPPort dstport,
                            const mDNSInterfaceID InterfaceID);
extern void mDNSCoreRestartQueries(mDNS * const m);
extern void mDNSCoreRestartQuestion(mDNS * const m, DNSQuestion * q);
extern void mDNSCoreRestartRegistration(mDNS * const m, AuthRecord * rr, int announceCount);
typedef void (*FlushCache)(mDNS * const m);
typedef void (*CallbackBeforeStartQuery)(mDNS * const m, void * context);
extern void mDNSCoreRestartAddressQueries(mDNS * const m, mDNSBool SearchDomainsChanged, FlushCache flushCacheRecords,
                                          CallbackBeforeStartQuery beforeQueryStart, void * context);
extern mDNSBool mDNSCoreHaveAdvertisedMulticastServices(mDNS * const m);
#ifndef REMOVE_MACHINE_SLEEP
extern void mDNSCoreMachineSleep(mDNS * const m, mDNSBool wake);
extern mDNSBool mDNSCoreReadyForSleep(mDNS * m, mDNSs32 now);
extern mDNSs32 mDNSCoreIntervalToNextWake(mDNS * const m, mDNSs32 now);
#endif //#ifndef REMOVE_MACHINE_SLEEP

extern void mDNSCoreReceiveRawPacket(mDNS * const m, const mDNSu8 * const p, const mDNSu8 * const end,
                                     const mDNSInterfaceID InterfaceID);

extern mDNSBool mDNSAddrIsDNSMulticast(const mDNSAddr * ip);

extern CacheRecord * CreateNewCacheEntry(mDNS * const m, const mDNSu32 slot, CacheGroup * cg, mDNSs32 delay, mDNSBool Add,
                                         const mDNSAddr * sourceAddress);
extern CacheGroup * CacheGroupForName(const mDNS * const m, const mDNSu32 slot, const mDNSu32 namehash,
                                      const domainname * const name);
extern void ReleaseCacheRecord(mDNS * const m, CacheRecord * r);
extern void ScheduleNextCacheCheckTime(mDNS * const m, const mDNSu32 slot, const mDNSs32 event);
extern void SetNextCacheCheckTimeForRecord(mDNS * const m, CacheRecord * const rr);
extern void GrantCacheExtensions(mDNS * const m, DNSQuestion * q, mDNSu32 lease);
extern void MakeNegativeCacheRecord(mDNS * const m, CacheRecord * const cr, const domainname * const name, const mDNSu32 namehash,
                                    const mDNSu16 rrtype, const mDNSu16 rrclass, mDNSu32 ttl_seconds, mDNSInterfaceID InterfaceID,
                                    DNSServer * dnsserver);
extern void CompleteDeregistration(mDNS * const m, AuthRecord * rr);
extern void AnswerCurrentQuestionWithResourceRecord(mDNS * const m, CacheRecord * const rr, const QC_result AddRecord);
extern void AnswerQuestionByFollowingCNAME(mDNS * const m, DNSQuestion * q, ResourceRecord * rr);
extern char * InterfaceNameForID(mDNS * const m, const mDNSInterfaceID InterfaceID);
extern void DNSServerChangeForQuestion(mDNS * const m, DNSQuestion * q, DNSServer * newServer);
extern void ActivateUnicastRegistration(mDNS * const m, AuthRecord * const rr);
extern void CheckSuppressUnusableQuestions(mDNS * const m);
extern void RetrySearchDomainQuestions(mDNS * const m);
extern mDNSBool DomainEnumQuery(const domainname * qname);
extern mStatus UpdateKeepaliveRData(mDNS * const m, AuthRecord * rr, NetworkInterfaceInfo * const intf, mDNSBool updateMac,
                                    char * ethAddr);
extern void UpdateKeepaliveRMACAsync(mDNS * const m, void * context);
extern void UpdateRMACCallback(mDNS * const m, void * context);

// Used only in logging to restrict the number of /etc/hosts entries printed
extern void FreeEtcHosts(mDNS * const m, AuthRecord * const rr, mStatus result);
// exported for using the hash for /etc/hosts AuthRecords
extern AuthGroup * AuthGroupForName(AuthHash * r, const mDNSu32 slot, const mDNSu32 namehash, const domainname * const name);
extern AuthGroup * AuthGroupForRecord(AuthHash * r, const mDNSu32 slot, const ResourceRecord * const rr);
extern AuthGroup * InsertAuthRecord(mDNS * const m, AuthHash * r, AuthRecord * rr);
extern AuthGroup * RemoveAuthRecord(mDNS * const m, AuthHash * r, AuthRecord * rr);
extern mDNSBool mDNS_CheckForCacheRecord(mDNS * const m, DNSQuestion * q, mDNSu16 qtype);

// For now this AutoTunnel stuff is specific to Mac OS X.
// In the future, if there's demand, we may see if we can abstract it out cleanly into the platform layer
#if APPLE_OSX_mDNSResponder
extern void AutoTunnelCallback(mDNS * const m, DNSQuestion * question, const ResourceRecord * const answer, QC_result AddRecord);
extern void AddNewClientTunnel(mDNS * const m, DNSQuestion * const q);
extern void StartServerTunnel(mDNS * const m, DomainAuthInfo * const info);
extern void UpdateAutoTunnelDomainStatuses(const mDNS * const m);
extern void RemoveAutoTunnel6Record(mDNS * const m);
extern mDNSBool RecordReadyForSleep(mDNS * const m, AuthRecord * rr);
// For now this LocalSleepProxy stuff is specific to Mac OS X.
// In the future, if there's demand, we may see if we can abstract it out cleanly into the platform layer
extern mStatus ActivateLocalProxy(mDNS * const m, NetworkInterfaceInfo * const intf);
extern void mDNSPlatformUpdateDNSStatus(mDNS * const m, DNSQuestion * q);
extern void mDNSPlatformTriggerDNSRetry(mDNS * const m, DNSQuestion * v4q, DNSQuestion * v6q);
extern void mDNSPlatformLogToFile(int log_level, const char * buffer);
extern mDNSBool SupportsInNICProxy(NetworkInterfaceInfo * const intf);
#endif

typedef void ProxyCallback(mDNS * const m, void * socket, void * const msg, const mDNSu8 * const end,
                           const mDNSAddr * const srcaddr, const mDNSIPPort srcport, const mDNSAddr * dstaddr,
                           const mDNSIPPort dstport, const mDNSInterfaceID InterfaceID, void * context);
extern void mDNSPlatformInitDNSProxySkts(mDNS * const m, ProxyCallback * UDPCallback, ProxyCallback * TCPCallback);
extern void mDNSPlatformCloseDNSProxySkts(mDNS * const m);
extern void mDNSPlatformDisposeProxyContext(void * context);
extern mDNSu8 * DNSProxySetAttributes(DNSQuestion * q, DNSMessageHeader * h, DNSMessage * msg, mDNSu8 * start, mDNSu8 * limit);

// Sleep Assertions are specific to Mac OS X
#if APPLE_OSX_mDNSResponder
extern void mDNSPlatformSleepAssertion(mDNS * const m, double timeout);
#endif

extern mDNSBool mDNSPlatformAllowPID(mDNS * const m, DNSQuestion * q);
extern mDNSs32 mDNSPlatformGetServiceID(mDNS * const m, DNSQuestion * q);
extern void mDNSPlatformSetDelegatePID(UDPSocket * src, const mDNSAddr * dst, DNSQuestion * q);
extern mDNSs32 mDNSPlatformGetPID(void);

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Sleep Proxy
#endif

// Sleep Proxy Server Property Encoding
//
// Sleep Proxy Servers are advertised using a structured service name, consisting of four
// metrics followed by a human-readable name. The metrics assist clients in deciding which
// Sleep Proxy Server(s) to use when multiple are available on the network. Each metric
// is a two-digit decimal number in the range 10-99. Lower metrics are generally better.
//
//   AA-BB-CC-DD.FF Name
//
// Metrics:
//
// AA = Intent
// BB = Portability
// CC = Marginal Power
// DD = Total Power
// FF = Features Supported (Currently TCP Keepalive only)
//
//
// ** Intent Metric **
//
// 20 = Dedicated Sleep Proxy Server -- a device, permanently powered on,
//      installed for the express purpose of providing Sleep Proxy Service.
//
// 30 = Primary Network Infrastructure Hardware -- a router, DHCP server, NAT gateway,
//      or similar permanently installed device which is permanently powered on.
//      This is hardware designed for the express purpose of being network
//      infrastructure, and for most home users is typically a single point
//      of failure for the local network -- e.g. most home users only have
//      a single NAT gateway / DHCP server. Even though in principle the
//      hardware might technically be capable of running different software,
//      a typical user is unlikely to do that. e.g. AirPort base station.
//
// 40 = Primary Network Infrastructure Software -- a general-purpose computer
//      (e.g. Mac, Windows, Linux, etc.) which is currently running DHCP server
//      or NAT gateway software, but the user could choose to turn that off
//      fairly easily. e.g. iMac running Internet Sharing
//
// 50 = Secondary Network Infrastructure Hardware -- like primary infrastructure
//      hardware, except not a single point of failure for the entire local network.
//      For example, an AirPort base station in bridge mode. This may have clients
//      associated with it, and if it goes away those clients will be inconvenienced,
//      but unlike the NAT gateway / DHCP server, the entire local network is not
//      dependent on it.
//
// 60 = Secondary Network Infrastructure Software -- like 50, but in a general-
//      purpose CPU.
//
// 70 = Incidentally Available Hardware -- a device which has no power switch
//      and is generally left powered on all the time. Even though it is not a
//      part of what we conventionally consider network infrastructure (router,
//      DHCP, NAT, DNS, etc.), and the rest of the network can operate fine
//      without it, since it's available and unlikely to be turned off, it is a
//      reasonable candidate for providing Sleep Proxy Service e.g. Apple TV,
//      or an AirPort base station in client mode, associated with an existing
//      wireless network (e.g. AirPort Express connected to a music system, or
//      being used to share a USB printer).
//
// 80 = Incidentally Available Software -- a general-purpose computer which
//      happens at this time to be set to "never sleep", and as such could be
//      useful as a Sleep Proxy Server, but has not been intentionally provided
//      for this purpose. Of all the Intent Metric categories this is the
//      one most likely to be shut down or put to sleep without warning.
//      However, if nothing else is availalable, it may be better than nothing.
//      e.g. Office computer in the workplace which has been set to "never sleep"
//
//
// ** Portability Metric **
//
// Inversely related to mass of device, on the basis that, all other things
// being equal, heavier devices are less likely to be moved than lighter devices.
// E.g. A MacBook running Internet Sharing is probably more likely to be
// put to sleep and taken away than a Mac Pro running Internet Sharing.
// The Portability Metric is a logarithmic decibel scale, computed by taking the
// (approximate) mass of the device in milligrammes, taking the base 10 logarithm
// of that, multiplying by 10, and subtracting the result from 100:
//
//   Portability Metric = 100 - (log10(mg) * 10)
//
// The Portability Metric is not necessarily computed literally from the actual
// mass of the device; the intent is just that lower numbers indicate more
// permanent devices, and higher numbers indicate devices more likely to be
// removed from the network, e.g., in order of increasing portability:
//
// Mac Pro < iMac < Laptop < iPhone
//
// Example values:
//
// 10 = 1 metric tonne
// 40 = 1kg
// 70 = 1g
// 90 = 10mg
//
//
// ** Marginal Power and Total Power Metrics **
//
// The Marginal Power Metric is the power difference between sleeping and staying awake
// to be a Sleep Proxy Server.
//
// The Total Power Metric is the total power consumption when being Sleep Proxy Server.
//
// The Power Metrics use a logarithmic decibel scale, computed as ten times the
// base 10 logarithm of the (approximate) power in microwatts:
//
//   Power Metric = log10(uW) * 10
//
// Higher values indicate higher power consumption. Example values:
//
// 10 =  10 uW
// 20 = 100 uW
// 30 =   1 mW
// 60 =   1 W
// 90 =   1 kW
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
typedef enum
{
    mDNSSleepProxyMetric_Dedicated          = 20,
    mDNSSleepProxyMetric_PrimaryHardware    = 30,
    mDNSSleepProxyMetric_PrimarySoftware    = 40,
    mDNSSleepProxyMetric_SecondaryHardware  = 50,
    mDNSSleepProxyMetric_SecondarySoftware  = 60,
    mDNSSleepProxyMetric_IncidentalHardware = 70,
    mDNSSleepProxyMetric_IncidentalSoftware = 80
} mDNSSleepProxyMetric;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
typedef enum
{
    mDNS_NoWake        = 0, // System does not support Wake on LAN
    mDNS_WakeOnAC      = 1, // System supports Wake on LAN when connected to AC power only
    mDNS_WakeOnBattery = 2  // System supports Wake on LAN on battery
} mDNSWakeForNetworkAccess;

#ifndef REMOVE_MACHINE_SLEEP
extern void mDNSCoreBeSleepProxyServer_internal(mDNS * const m, mDNSu8 sps, mDNSu8 port, mDNSu8 marginalpower, mDNSu8 totpower,
                                                mDNSu8 features);
#define mDNSCoreBeSleepProxyServer(M, S, P, MP, TP, F)                                                                             \
    do                                                                                                                             \
    {                                                                                                                              \
        mDNS_Lock(m);                                                                                                              \
        mDNSCoreBeSleepProxyServer_internal((M), (S), (P), (MP), (TP), (F));                                                       \
        mDNS_Unlock(m);                                                                                                            \
    } while (0)
#endif // #ifndef REMOVE_MACHINE_SLEEP

extern void FindSPSInCache(mDNS * const m, const DNSQuestion * const q, const CacheRecord * sps[3]);
#define PrototypeSPSName(X)                                                                                                        \
    ((X)[0] >= 11 && (X)[3] == '-' && (X)[4] == '9' && (X)[5] == '9' && (X)[6] == '-' && (X)[7] == '9' && (X)[8] == '9' &&         \
     (X)[9] == '-' && (X)[10] == '9' && (X)[11] == '9')
#define ValidSPSName(X) ((X)[0] >= 5 && mDNSIsDigit((X)[1]) && mDNSIsDigit((X)[2]) && mDNSIsDigit((X)[4]) && mDNSIsDigit((X)[5]))
#define SPSMetric(X)                                                                                                               \
    (!ValidSPSName(X) || PrototypeSPSName(X) ? 1000000                                                                             \
                                             : ((X)[1] - '0') * 100000 + ((X)[2] - '0') * 10000 + ((X)[4] - '0') * 1000 +          \
             ((X)[5] - '0') * 100 + ((X)[7] - '0') * 10 + ((X)[8] - '0'))
#define LocalSPSMetric(X) ((X)->SPSType * 10000 + (X)->SPSPortability * 100 + (X)->SPSMarginalPower)
#define SPSFeatures(X) ((X)[0] >= 13 && (X)[12] == '.' ? ((X)[13] - '0') : 0)

#define MD5_DIGEST_LENGTH 16 /* digest length in bytes */
#define MD5_BLOCK_BYTES 64   /* block size in bytes */
#define MD5_BLOCK_LONG (MD5_BLOCK_BYTES / sizeof(mDNSu32))

typedef struct MD5state_st
{
    mDNSu32 A, B, C, D;
    mDNSu32 Nl, Nh;
    mDNSu32 data[MD5_BLOCK_LONG];
    int num;
} MD5_CTX;

extern int MD5_Init(MD5_CTX * c);
extern int MD5_Update(MD5_CTX * c, const void * data, unsigned long len);
extern int MD5_Final(unsigned char * md, MD5_CTX * c);

// ***************************************************************************
#if 0
#pragma mark -
#pragma mark - Compile-Time assertion checks
#endif

// Some C compiler cleverness. We can make the compiler check certain things for
// us, and report compile-time errors if anything is wrong. The usual way to do
// this would be to use a run-time "if" statement, but then you don't find out
// what's wrong until you run the software. This way, if the assertion condition
// is false, the array size is negative, and the complier complains immediately.

struct CompileTimeAssertionChecks_mDNS
{
    // Check that the compiler generated our on-the-wire packet format structure definitions
    // properly packed, without adding padding bytes to align fields on 32-bit or 64-bit boundaries.
    char assert0[(sizeof(rdataSRV) == 262) ? 1 : -1];
    // char assert0[(sizeof(rdataSRV)         == 70                          ) ? 1 : -1];
    char assert1[(sizeof(DNSMessageHeader) == 12) ? 1 : -1];
    char assert2[(sizeof(DNSMessage) == 12 + AbsoluteMaxDNSMessageData) ? 1 : -1];
    char assert3[(sizeof(mDNSs8) == 1) ? 1 : -1];
    char assert4[(sizeof(mDNSu8) == 1) ? 1 : -1];
    char assert5[(sizeof(mDNSs16) == 2) ? 1 : -1];
    char assert6[(sizeof(mDNSu16) == 2) ? 1 : -1];
    char assert7[(sizeof(mDNSs32) == 4) ? 1 : -1];
    char assert8[(sizeof(mDNSu32) == 4) ? 1 : -1];
    char assert9[(sizeof(mDNSOpaque16) == 2) ? 1 : -1];
    char assertA[(sizeof(mDNSOpaque32) == 4) ? 1 : -1];
    char assertB[(sizeof(mDNSOpaque128) == 16) ? 1 : -1];
    char assertC[(sizeof(CacheRecord) == sizeof(CacheGroup)) ? 1 : -1];
    char assertD[(sizeof(int) >= 4) ? 1 : -1];
    char assertE[(StandardAuthRDSize >= 256) ? 1 : -1];
    // char assertE[(StandardAuthRDSize       >=  64                         ) ? 1 : -1];
    char assertF[(14 == 14) ? 1 : -1];
    char assertG[(28 == 28) ? 1 : -1];
    char assertH[(sizeof(IPv4Header) == 20) ? 1 : -1];
    char assertI[(sizeof(IPv6Header) == 40) ? 1 : -1];
    char assertJ[(sizeof(IPv6NDP) == 24) ? 1 : -1];
    char assertK[(sizeof(UDPHeader) == 8) ? 1 : -1];
    char assertL[(sizeof(IKEHeader) == 28) ? 1 : -1];
    char assertM[(sizeof(TCPHeader) == 20) ? 1 : -1];

    // Check our structures are reasonable sizes. Including overly-large buffers, or embedding
    // other overly-large structures instead of having a pointer to them, can inadvertently
    // cause structure sizes (and therefore memory usage) to balloon unreasonably.
    char sizecheck_RDataBody[(sizeof(RDataBody) == 264) ? 1 : -1];
    // char sizecheck_RDataBody           [(sizeof(RDataBody)            ==   72) ? 1 : -1];
    char sizecheck_ResourceRecord[(sizeof(ResourceRecord) <= 72) ? 1 : -1];
    char sizecheck_AuthRecord[(sizeof(AuthRecord) <= 1208) ? 1 : -1];
    char sizecheck_CacheRecord[(sizeof(CacheRecord) <= 232) ? 1 : -1];
    char sizecheck_CacheGroup[(sizeof(CacheGroup) <= 232) ? 1 : -1];
    char sizecheck_DNSQuestion[(sizeof(DNSQuestion) <= 832) ? 1 : -1];

// Checks commented out when sizeof(DNSQuestion) change cascaded into having to change yet another
// set of hardcoded size values because these structures contain one or more DNSQuestion
// instances.
//     char sizecheck_ZoneData            [(sizeof(ZoneData)             <=  1648) ? 1 : -1];
#ifndef REMOVE_NAT_FUCTIONALITY
    char sizecheck_NATTraversalInfo[(sizeof(NATTraversalInfo) <= 200) ? 1 : -1];
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    char sizecheck_HostnameInfo[(sizeof(HostnameInfo) <= 3050) ? 1 : -1];
    char sizecheck_DNSServer[(sizeof(DNSServer) <= 340) ? 1 : -1];
    //    char sizecheck_NetworkInterfaceInfo[(sizeof(NetworkInterfaceInfo) <=  6988) ? 1 : -1];
    char sizecheck_ServiceRecordSet[(sizeof(ServiceRecordSet) <= 5540) ? 1 : -1];
    char sizecheck_DomainAuthInfo[(sizeof(DomainAuthInfo) <= 7888) ? 1 : -1];
//    char sizecheck_ServiceInfoQuery    [(sizeof(ServiceInfoQuery)     <=  3302) ? 1 : -1];
#if APPLE_OSX_mDNSResponder
//    char sizecheck_ClientTunnel        [(sizeof(ClientTunnel)         <=  1160) ? 1 : -1];
#endif
};

// Routine to initialize device-info TXT record contents
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
mDNSu32 initializeDeviceInfoTXT(mDNS * m, mDNSu8 * ptr);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

#if APPLE_OSX_mDNSResponder
extern void D2D_start_advertising_interface(NetworkInterfaceInfo * interface);
extern void D2D_stop_advertising_interface(NetworkInterfaceInfo * interface);
#endif

// ***************************************************************************

#ifdef __cplusplus
}
#endif

#endif
