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

#ifndef __DNSCOMMON_H_
#define __DNSCOMMON_H_

#include "mDNSEmbeddedAPI.h"

#ifdef  __cplusplus
extern "C" {
#endif

//*************************************************************************************************************
// Macros

// Note: The C preprocessor stringify operator ('#') makes a string from its argument, without macro expansion
// e.g. If "version" is #define'd to be "4", then STRINGIFY_AWE(version) will return the string "version", not "4"
// To expand "version" to its value before making the string, use STRINGIFY(version) instead
#define STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s) # s
#define STRINGIFY(s) STRINGIFY_ARGUMENT_WITHOUT_EXPANSION(s)

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - DNS Protocol Constants
#endif

typedef enum
{
    kDNSFlag0_QR_Mask     = 0x80,       // Query or response?
    kDNSFlag0_QR_Query    = 0x00,
    kDNSFlag0_QR_Response = 0x80,

    kDNSFlag0_OP_Mask     = 0x78,       // Operation type
    kDNSFlag0_OP_StdQuery = 0x00,
    kDNSFlag0_OP_Iquery   = 0x08,
    kDNSFlag0_OP_Status   = 0x10,
    kDNSFlag0_OP_Unused3  = 0x18,
    kDNSFlag0_OP_Notify   = 0x20,
    kDNSFlag0_OP_Update   = 0x28,

    kDNSFlag0_QROP_Mask   = kDNSFlag0_QR_Mask | kDNSFlag0_OP_Mask,

    kDNSFlag0_AA          = 0x04,       // Authoritative Answer?
    kDNSFlag0_TC          = 0x02,       // Truncated?
    kDNSFlag0_RD          = 0x01,       // Recursion Desired?
    kDNSFlag1_RA          = 0x80,       // Recursion Available?

    kDNSFlag1_Zero        = 0x40,       // Reserved; must be zero
    kDNSFlag1_AD          = 0x20,       // Authentic Data [RFC 2535]
    kDNSFlag1_CD          = 0x10,       // Checking Disabled [RFC 2535]

    kDNSFlag1_RC_Mask     = 0x0F,       // Response code
    kDNSFlag1_RC_NoErr    = 0x00,
    kDNSFlag1_RC_FormErr  = 0x01,
    kDNSFlag1_RC_ServFail = 0x02,
    kDNSFlag1_RC_NXDomain = 0x03,
    kDNSFlag1_RC_NotImpl  = 0x04,
    kDNSFlag1_RC_Refused  = 0x05,
    kDNSFlag1_RC_YXDomain = 0x06,
    kDNSFlag1_RC_YXRRSet  = 0x07,
    kDNSFlag1_RC_NXRRSet  = 0x08,
    kDNSFlag1_RC_NotAuth  = 0x09,
    kDNSFlag1_RC_NotZone  = 0x0A
} DNS_Flags;

typedef enum
{
    TSIG_ErrBadSig  = 16,
    TSIG_ErrBadKey  = 17,
    TSIG_ErrBadTime = 18
} TSIG_ErrorCode;

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - General Utility Functions
#endif

extern NetworkInterfaceInfo *GetFirstActiveInterface(NetworkInterfaceInfo *intf);
extern mDNSInterfaceID GetNextActiveInterfaceID(const NetworkInterfaceInfo *intf);

extern mDNSu32 mDNSRandom(mDNSu32 max);     // Returns pseudo-random result from zero to max inclusive

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Domain Name Utility Functions
#endif

#define mDNSSubTypeLabel   "\x04_sub"

#define mDNSIsDigit(X)     ((X) >= '0' && (X) <= '9')
#define mDNSIsUpperCase(X) ((X) >= 'A' && (X) <= 'Z')
#define mDNSIsLowerCase(X) ((X) >= 'a' && (X) <= 'z')
#define mDNSIsLetter(X)    (mDNSIsUpperCase(X) || mDNSIsLowerCase(X))

#define mDNSValidHostChar(X, notfirst, notlast) (mDNSIsLetter(X) || mDNSIsDigit(X) || ((notfirst) && (notlast) && (X) == '-') )

extern mDNSu16 CompressedDomainNameLength(const domainname *const name, const domainname *parent);
extern int CountLabels(const domainname *d);
extern const domainname *SkipLeadingLabels(const domainname *d, int skip);

extern mDNSu32 TruncateUTF8ToLength(mDNSu8 *string, mDNSu32 length, mDNSu32 max);
extern mDNSBool LabelContainsSuffix(const domainlabel *const name, const mDNSBool RichText);
extern mDNSu32 RemoveLabelSuffix(domainlabel *name, mDNSBool RichText);
extern void AppendLabelSuffix(domainlabel *const name, mDNSu32 val, const mDNSBool RichText);
#define ValidateDomainName(N) (DomainNameLength(N) <= MAX_DOMAIN_NAME)

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Resource Record Utility Functions
#endif

// IdenticalResourceRecord returns true if two resources records have
// the same name, type, class, and identical rdata (InterfaceID and TTL may differ)

// IdenticalSameNameRecord is the same, except it skips the expensive SameDomainName() check,
// which is at its most expensive and least useful in cases where we know in advance that the names match

// Note: The dominant use of IdenticalResourceRecord is from ProcessQuery(), handling known-answer lists. In this case
// it's common to have a whole bunch or records with exactly the same name (e.g. "_http._tcp.local") but different RDATA.
// The SameDomainName() check is expensive when the names match, and in this case *all* the names match, so we
// used to waste a lot of CPU time verifying that the names match, only then to find that the RDATA is different.
// We observed mDNSResponder spending 30% of its total CPU time on this single task alone.
// By swapping the checks so that we check the RDATA first, we can quickly detect when it's different
// (99% of the time) and then bail out before we waste time on the expensive SameDomainName() check.

#define IdenticalResourceRecord(r1,r2) ( \
        (r1)->rrtype    == (r2)->rrtype      && \
        (r1)->rrclass   == (r2)->rrclass     && \
        (r1)->namehash  == (r2)->namehash    && \
        (r1)->rdlength  == (r2)->rdlength    && \
        (r1)->rdatahash == (r2)->rdatahash   && \
        SameRDataBody((r1), &(r2)->rdata->u, SameDomainName) && \
        SameDomainName((r1)->name, (r2)->name))

#define IdenticalSameNameRecord(r1,r2) ( \
        (r1)->rrtype    == (r2)->rrtype      && \
        (r1)->rrclass   == (r2)->rrclass     && \
        (r1)->rdlength  == (r2)->rdlength    && \
        (r1)->rdatahash == (r2)->rdatahash   && \
        SameRDataBody((r1), &(r2)->rdata->u, SameDomainName))

// A given RRType answers a QuestionType if RRType is CNAME, or types match, or QuestionType is ANY,
// or the RRType is NSEC and positively asserts the nonexistence of the type being requested
#define RRTypeAnswersQuestionType(R,Q) ((R)->rrtype == kDNSType_CNAME || (R)->rrtype == (Q) || (Q) == kDNSQType_ANY || RRAssertsNonexistence((R),(Q)))
// Unicast NSEC records have the NSEC bit set whereas the multicast NSEC ones don't
#define UNICAST_NSEC(rr) ((rr)->rrtype == kDNSType_NSEC && RRAssertsExistence((rr), kDNSType_NSEC))

extern mDNSu32 RDataHashValue(const ResourceRecord *const rr);
extern mDNSBool SameRDataBody(const ResourceRecord *const r1, const RDataBody *const r2, DomainNameComparisonFn *samename);
extern mDNSBool SameNameRecordAnswersQuestion(const ResourceRecord *const rr, const DNSQuestion *const q);
extern mDNSBool ResourceRecordAnswersQuestion(const ResourceRecord *const rr, const DNSQuestion *const q);
extern mDNSBool AnyTypeRecordAnswersQuestion (const ResourceRecord *const rr, const DNSQuestion *const q);
extern mDNSBool ResourceRecordAnswersUnicastResponse(const ResourceRecord *const rr, const DNSQuestion *const q);
extern mDNSBool LocalOnlyRecordAnswersQuestion(AuthRecord *const rr, const DNSQuestion *const q);
extern mDNSu16 GetRDLength(const ResourceRecord *const rr, mDNSBool estimate);
extern mDNSBool ValidateRData(const mDNSu16 rrtype, const mDNSu16 rdlength, const RData *const rd);
extern mStatus DNSNameToLowerCase(domainname *d, domainname *result);

#define GetRRDomainNameTarget(RR) (                                                                          \
        ((RR)->rrtype == kDNSType_NS || (RR)->rrtype == kDNSType_CNAME || (RR)->rrtype == kDNSType_PTR || (RR)->rrtype == kDNSType_DNAME) ? &(RR)->rdata->u.name        : \
        ((RR)->rrtype == kDNSType_MX || (RR)->rrtype == kDNSType_AFSDB || (RR)->rrtype == kDNSType_RT  || (RR)->rrtype == kDNSType_KX   ) ? &(RR)->rdata->u.mx.exchange : \
        ((RR)->rrtype == kDNSType_SRV                                  ) ? &(RR)->rdata->u.srv.target : mDNSNULL )

#define LocalRecordReady(X) ((X)->resrec.RecordType != kDNSRecordTypeUnique)

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNS Message Creation Functions
#endif

extern void InitializeDNSMessage(DNSMessageHeader *h, mDNSOpaque16 id, mDNSOpaque16 flags);
extern const mDNSu8 *FindCompressionPointer(const mDNSu8 *const base, const mDNSu8 *const end, const mDNSu8 *const domname);
extern mDNSu8 *putDomainNameAsLabels(const DNSMessage *const msg, mDNSu8 *ptr, const mDNSu8 *const limit, const domainname *const name);
extern mDNSu8 *putRData(const DNSMessage *const msg, mDNSu8 *ptr, const mDNSu8 *const limit, const ResourceRecord *const rr);

// If we have a single large record to put in the packet, then we allow the packet to be up to 9K bytes,
// but in the normal case we try to keep the packets below 1500 to avoid IP fragmentation on standard Ethernet

#define AllowedRRSpace(msg) (((msg)->h.numAnswers || (msg)->h.numAuthorities || (msg)->h.numAdditionals) ? NormalMaxDNSMessageData : AbsoluteMaxDNSMessageData)

extern mDNSu8 *PutResourceRecordTTLWithLimit(DNSMessage *const msg, mDNSu8 *ptr, mDNSu16 *count, ResourceRecord *rr, mDNSu32 ttl, const mDNSu8 *limit);

#define PutResourceRecordTTL(msg, ptr, count, rr, ttl) \
    PutResourceRecordTTLWithLimit((msg), (ptr), (count), (rr), (ttl), (msg)->data + AllowedRRSpace(msg))

#define PutResourceRecordTTLJumbo(msg, ptr, count, rr, ttl) \
    PutResourceRecordTTLWithLimit((msg), (ptr), (count), (rr), (ttl), (msg)->data + AbsoluteMaxDNSMessageData)

#define PutResourceRecord(MSG, P, C, RR) PutResourceRecordTTL((MSG), (P), (C), (RR), (RR)->rroriginalttl)

// The PutRR_OS variants assume a local variable 'm', put build the packet at m->omsg,
// and assume local variables 'OwnerRecordSpace' & 'TraceRecordSpace' indicating how many bytes (if any) to reserve to add an OWNER/TRACER option at the end
#ifndef REMOVE_MDNS_TRACING  
	#define PutRR_OS_TTL(ptr, count, rr, ttl) \
		PutResourceRecordTTLWithLimit(&m->omsg, (ptr), (count), (rr), (ttl), m->omsg.data + AllowedRRSpace(&m->omsg) - OwnerRecordSpace - TraceRecordSpace)
#else
	#define PutRR_OS_TTL(ptr, count, rr, ttl) \
		PutResourceRecordTTLWithLimit(&m->omsg, (ptr), (count), (rr), (ttl), m->omsg.data + AllowedRRSpace(&m->omsg) - OwnerRecordSpace)
#endif //#ifndef REMOVE_MDNS_TRACING  
#define PutRR_OS(P, C, RR) PutRR_OS_TTL((P), (C), (RR), (RR)->rroriginalttl)

extern mDNSu8 *putQuestion(DNSMessage *const msg, mDNSu8 *ptr, const mDNSu8 *const limit, const domainname *const name, mDNSu16 rrtype, mDNSu16 rrclass);
extern mDNSu8 *putZone(DNSMessage *const msg, mDNSu8 *ptr, mDNSu8 *limit, const domainname *zone, mDNSOpaque16 zoneClass);
extern mDNSu8 *putPrereqNameNotInUse(const domainname *const name, DNSMessage *const msg, mDNSu8 *const ptr, mDNSu8 *const end);
extern mDNSu8 *putDeletionRecord(DNSMessage *msg, mDNSu8 *ptr, ResourceRecord *rr);
extern mDNSu8 *putDeletionRecordWithLimit(DNSMessage *msg, mDNSu8 *ptr, ResourceRecord *rr, mDNSu8 *limit);
extern mDNSu8 *putDeleteRRSetWithLimit(DNSMessage *msg, mDNSu8 *ptr, const domainname *name, mDNSu16 rrtype, mDNSu8 *limit);
extern mDNSu8 *putDeleteAllRRSets(DNSMessage *msg, mDNSu8 *ptr, const domainname *name);
extern mDNSu8 *putUpdateLease(DNSMessage *msg, mDNSu8 *end, mDNSu32 lease);
extern mDNSu8 *putUpdateLeaseWithLimit(DNSMessage *msg, mDNSu8 *ptr, mDNSu32 lease, mDNSu8 *limit);

#ifndef REMOVE_TUNNELING
extern mDNSu8 *putHINFO(const mDNS *const m, DNSMessage *const msg, mDNSu8 *ptr, DomainAuthInfo *authInfo, mDNSu8 *limit);
#endif // #ifndef REMOVE_TUNNELING
extern mDNSu8 *putDNSSECOption(DNSMessage *msg, mDNSu8 *end, mDNSu8 *limit);
extern int baseEncode(char *buffer, int blen, const mDNSu8 *data, int len, int encAlg);
extern void NSEC3Parse(const ResourceRecord *const rr, mDNSu8 **salt, int *hashLength, mDNSu8 **nxtName, int *bitmaplen, mDNSu8 **bitmap);

extern const mDNSu8 *NSEC3HashName(const domainname *name, rdataNSEC3 *nsec3, const mDNSu8 *AnonData, int AnonDataLen,
    const mDNSu8 hash[NSEC3_MAX_HASH_LEN], int *dlen);

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - DNS Message Parsing Functions
#endif

#define AuthHashSlot(X) (DomainNameHashValue(X) % AUTH_HASH_SLOTS)
#define HashSlot(X) (DomainNameHashValue(X) % CACHE_HASH_SLOTS)
extern mDNSu32 DomainNameHashValue(const domainname *const name);
extern void SetNewRData(ResourceRecord *const rr, RData *NewRData, mDNSu16 rdlength);
extern const mDNSu8 *skipDomainName(const DNSMessage *const msg, const mDNSu8 *ptr, const mDNSu8 *const end);
extern const mDNSu8 *getDomainName(const DNSMessage *const msg, const mDNSu8 *ptr, const mDNSu8 *const end,
                                   domainname *const name);
extern const mDNSu8 *skipResourceRecord(const DNSMessage *msg, const mDNSu8 *ptr, const mDNSu8 *end);
extern const mDNSu8 *GetLargeResourceRecord(mDNS *const m, const DNSMessage * const msg, const mDNSu8 *ptr,
                                            const mDNSu8 * end, const mDNSInterfaceID InterfaceID, mDNSu8 RecordType, LargeCacheRecord *const largecr);
extern mDNSBool SetRData(const DNSMessage *const msg, const mDNSu8 *ptr, const mDNSu8 *end,
    LargeCacheRecord *const largecr, mDNSu16 rdlength);
extern const mDNSu8 *skipQuestion(const DNSMessage *msg, const mDNSu8 *ptr, const mDNSu8 *end);
extern const mDNSu8 *getQuestion(const DNSMessage *msg, const mDNSu8 *ptr, const mDNSu8 *end, const mDNSInterfaceID InterfaceID,
                                 DNSQuestion *question);
extern const mDNSu8 *LocateAnswers(const DNSMessage *const msg, const mDNSu8 *const end);
extern const mDNSu8 *LocateAuthorities(const DNSMessage *const msg, const mDNSu8 *const end);
extern const mDNSu8 *LocateAdditionals(const DNSMessage *const msg, const mDNSu8 *const end);
extern const mDNSu8 *LocateOptRR(const DNSMessage *const msg, const mDNSu8 *const end, int minsize);
extern const rdataOPT *GetLLQOptData(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end);
extern mDNSu32 GetPktLease(mDNS *m, DNSMessage *msg, const mDNSu8 *end);
extern void DumpPacket(mDNS *const m, mStatus status, mDNSBool sent, char *transport,
                       const mDNSAddr *srcaddr, mDNSIPPort srcport,
                       const mDNSAddr *dstaddr, mDNSIPPort dstport, const DNSMessage *const msg, const mDNSu8 *const end);
extern mDNSBool RRAssertsNonexistence(const ResourceRecord *const rr, mDNSu16 type);
extern mDNSBool RRAssertsExistence(const ResourceRecord *const rr, mDNSu16 type);
extern mDNSBool BitmapTypeCheck(mDNSu8 *bmap, int bitmaplen, mDNSu16 type);

extern mDNSu16 swap16(mDNSu16 x);
extern mDNSu32 swap32(mDNSu32 x);

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Packet Sending Functions
#endif

extern mStatus mDNSSendDNSMessage(mDNS *const m, DNSMessage *const msg, mDNSu8 *end,
                                  mDNSInterfaceID InterfaceID, UDPSocket *src, const mDNSAddr *dst, 
                                  mDNSIPPort dstport, TCPSocket *sock, DomainAuthInfo *authInfo,
                                  mDNSBool useBackgroundTrafficClass);

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - RR List Management & Task Management
#endif

extern void ShowTaskSchedulingError(mDNS *const m);
extern void mDNS_Lock_(mDNS *const m, const char * const functionname);
extern void mDNS_Unlock_(mDNS *const m, const char * const functionname);

#if defined(_WIN32)
 #define __func__ __FUNCTION__
#endif

#define mDNS_Lock(X) mDNS_Lock_((X), __func__)

#define mDNS_Unlock(X) mDNS_Unlock_((X), __func__)

#define mDNS_CheckLock(X) { if ((X)->mDNS_busy != (X)->mDNS_reentrancy+1) \
                            LogMsg("%s: Lock not held! mDNS_busy (%ld) mDNS_reentrancy (%ld)", __func__, (X)->mDNS_busy, (X)->mDNS_reentrancy); }

#define mDNS_DropLockBeforeCallback() do { m->mDNS_reentrancy++; \
                                           if (m->mDNS_busy != m->mDNS_reentrancy) LogMsg("%s: Locking Failure! mDNS_busy (%ld) != mDNS_reentrancy (%ld)", __func__, m->mDNS_busy, m->mDNS_reentrancy);                                                                                                                                                                  \
} while (0)

#define mDNS_ReclaimLockAfterCallback() do { \
        if (m->mDNS_busy != m->mDNS_reentrancy) LogMsg("%s: Unlocking Failure! mDNS_busy (%ld) != mDNS_reentrancy (%ld)", __func__, m->mDNS_busy, m->mDNS_reentrancy);                                                                                                                                                                    \
        m->mDNS_reentrancy--; } while (0)

#ifdef  __cplusplus
}
#endif

#endif // __DNSCOMMON_H_
