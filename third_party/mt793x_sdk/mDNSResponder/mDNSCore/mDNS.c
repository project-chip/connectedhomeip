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
 *
 * This code is completely 100% portable C. It does not depend on any external header files
 * from outside the mDNS project -- all the types it expects to find are defined right here.
 *
 * The previous point is very important: This file does not depend on any external
 * header files. It should compile on *any* platform that has a C compiler, without
 * making *any* assumptions about availability of so-called "standard" C functions,
 * routines, or types (which may or may not be present on any given platform).
 */

#include <stdio.h>
#include <stdlib.h>
#include "DNSCommon.h"                  // Defines general DNS untility routines
#include "uDNS.h"                       // Defines entry points into unicast-specific routines
#include "nsec.h"
#include "dnssec.h"
#include "anonymous.h"
//#define MTK_ENHANCE_BONJOUR 1

// memory reduction
#define UNICAST_DISABLED
#define SPC_DISABLED

// Disable certain benign warnings with Microsoft compilers
#if (defined(_MSC_VER))
// Disable "conditional expression is constant" warning for debug macros.
// Otherwise, this generates warnings for the perfectly natural construct "while(1)"
// If someone knows a variant way of writing "while(1)" that doesn't generate warning messages, please let us know
    #pragma warning(disable:4127)

// Disable "assignment within conditional expression".
// Other compilers understand the convention that if you place the assignment expression within an extra pair
// of parentheses, this signals to the compiler that you really intended an assignment and no warning is necessary.
// The Microsoft compiler doesn't understand this convention, so in the absense of any other way to signal
// to the compiler that the assignment is intentional, we have to just turn this warning off completely.
    #pragma warning(disable:4706)
#endif

#include "dns_sd.h" // for kDNSServiceFlags* definitions

#if APPLE_OSX_mDNSResponder

#include <WebFilterDNS/WebFilterDNS.h>

#if !NO_WCF
WCFConnection *WCFConnectionNew(void) __attribute__((weak_import));
void WCFConnectionDealloc(WCFConnection* c) __attribute__((weak_import));

// Do we really need to define a macro for "if"?
#define CHECK_WCF_FUNCTION(X) if (X)
#endif // ! NO_WCF

#else

#define NO_WCF 1
#endif // APPLE_OSX_mDNSResponder

// Forward declarations
mDNSlocal void BeginSleepProcessing(mDNS *const m);
mDNSlocal void RetrySPSRegistrations(mDNS *const m);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void SendWakeup(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSEthAddr *EthAddr, mDNSOpaque48 *password);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal mDNSBool CacheRecordRmvEventsForQuestion(mDNS *const m, DNSQuestion *q);
mDNSlocal mDNSBool LocalRecordRmvEventsForQuestion(mDNS *const m, DNSQuestion *q);
mDNSlocal void mDNS_PurgeForQuestion(mDNS *const m, DNSQuestion *q);
mDNSlocal void CheckForDNSSECRecords(mDNS *const m, DNSQuestion *q);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void mDNS_SendKeepalives(mDNS *const m);
mDNSlocal void mDNS_ExtractKeepaliveInfo(AuthRecord *ar, mDNSu32 *timeout, mDNSAddr *laddr, mDNSAddr *raddr, mDNSEthAddr *eth,
                                         mDNSu32 *seq, mDNSu32 *ack, mDNSIPPort *lport, mDNSIPPort *rport, mDNSu16 *win);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void AdvertiseAllInterfaceRecords(mDNS *const m);
mDNSlocal void DeadvertiseAllInterfaceRecords(mDNS *const m);
mDNSlocal void FreeNSECRecords(mDNS *const m, CacheRecord *NSECRecords);
mDNSlocal void mDNSParseNSEC3Records(mDNS *const m, const DNSMessage *const response, const mDNSu8 *end,
                                        const mDNSInterfaceID InterfaceID, CacheRecord **NSEC3Records);


// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Program Constants
#endif

// To Turn OFF mDNS_Tracer set MDNS_TRACER to 0 or undef it
#define MDNS_TRACER 1

#define NO_HINFO 1

// Any records bigger than this are considered 'large' records
//#define SmallRecordLimit 1024
#define SmallRecordLimit 256

//#define kMaxUpdateCredits 10
#define kMaxUpdateCredits 3
#define kUpdateCreditRefreshInterval (mDNSPlatformOneSecond * 6)

// define special NR_AnswerTo values
#define NR_AnswerMulticast  (mDNSu8*)~0
#define NR_AnswerUnicast    (mDNSu8*)~1

// Defined to set the kDNSQClass_UnicastResponse bit in the first four query packets.
// else, it's just set it the first query.
#define mDNS_REQUEST_UNICAST_RESPONSE 0

// The code (see SendQueries() and BuildQuestion()) needs to have the
// RequestUnicast value set to a value one greater than the number of times you want the query
// sent with the "request unicast response" (QU) bit set.
#define SET_QU_IN_FIRST_QUERY   2
#define SET_QU_IN_FIRST_FOUR_QUERIES   5


mDNSexport const char *const mDNS_DomainTypeNames[] =
{
    "b._dns-sd._udp.",      // Browse
    "db._dns-sd._udp.",     // Default Browse
    "lb._dns-sd._udp.",     // Automatic Browse
    "r._dns-sd._udp.",      // Registration
    "dr._dns-sd._udp."      // Default Registration
};

#ifdef UNICAST_DISABLED
#define uDNS_IsActiveQuery(q, u) mDNSfalse
#endif

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - General Utility Functions
#endif

// Returns true if this is a  unique, authoritative LocalOnly record that answers questions of type 
// A, AAAA , CNAME, or PTR.  The caller should answer the question with this record and not send out 
// the question on the wire if LocalOnlyRecordAnswersQuestion() also returns true.
// Main use is to handle /etc/hosts records and the LocalOnly PTR records created for localhost.
#define UniqueLocalOnlyRecord(rr) ((rr)->ARType == AuthRecordLocalOnly && \
                                        (rr)->resrec.RecordType & kDNSRecordTypeUniqueMask && \
                                        ((rr)->resrec.rrtype == kDNSType_A || (rr)->resrec.rrtype == kDNSType_AAAA || \
                                         (rr)->resrec.rrtype == kDNSType_CNAME || \
                                         (rr)->resrec.rrtype == kDNSType_PTR))

mDNSlocal void SetNextQueryStopTime(mDNS *const m, const DNSQuestion *const q)
{
    mDNS_CheckLock(m);

#if ForceAlerts
    if (m->mDNS_busy != m->mDNS_reentrancy+1) *(long*)0 = 0;
#endif

    if (m->NextScheduledStopTime - q->StopTime > 0)
        m->NextScheduledStopTime = q->StopTime;
}

mDNSexport void SetNextQueryTime(mDNS *const m, const DNSQuestion *const q)
{
    mDNS_CheckLock(m);

#if ForceAlerts
    if (m->mDNS_busy != m->mDNS_reentrancy+1) *(long*)0 = 0;
#endif

    if (ActiveQuestion(q))
    {
        // Depending on whether this is a multicast or unicast question we want to set either:
        // m->NextScheduledQuery = NextQSendTime(q) or
        // m->NextuDNSEvent      = NextQSendTime(q)
        mDNSs32 *const timer = mDNSOpaque16IsZero(q->TargetQID) ? &m->NextScheduledQuery : &m->NextuDNSEvent;
        if (*timer - NextQSendTime(q) > 0)
            *timer = NextQSendTime(q);
    }
}

mDNSlocal void ReleaseAuthEntity(AuthHash *r, AuthEntity *e)
{
#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING >= 1
    unsigned int i;
    for (i=0; i<sizeof(*e); i++) ((char*)e)[i] = 0xFF;
#endif
    e->next = r->rrauth_free;
    r->rrauth_free = e;
    r->rrauth_totalused--;
}

mDNSlocal void ReleaseAuthGroup(AuthHash *r, AuthGroup **cp)
{
    AuthEntity *e = (AuthEntity *)(*cp);
    LogMsg("ReleaseAuthGroup:  Releasing AuthGroup %##s", (*cp)->name->c);
    if ((*cp)->rrauth_tail != &(*cp)->members)
        LogMsg("ERROR: (*cp)->members == mDNSNULL but (*cp)->rrauth_tail != &(*cp)->members)");
    if ((*cp)->name != (domainname*)((*cp)->namestorage)) mDNSPlatformMemFree((*cp)->name);
    (*cp)->name = mDNSNULL;
    *cp = (*cp)->next;          // Cut record from list
    ReleaseAuthEntity(r, e);
}

mDNSlocal AuthEntity *GetAuthEntity(AuthHash *r, const AuthGroup *const PreserveAG)
{
    AuthEntity *e = mDNSNULL;

    if (r->rrauth_lock) { LogMsg("GetFreeCacheRR ERROR! Cache already locked!"); return(mDNSNULL); }
    r->rrauth_lock = 1;

    if (!r->rrauth_free)
    {
        // We allocate just one AuthEntity at a time because we need to be able
        // free them all individually which normally happens when we parse /etc/hosts into
        // AuthHash where we add the "new" entries and discard (free) the already added
        // entries. If we allocate as chunks, we can't free them individually.
        AuthEntity *storage = mDNSPlatformMemAllocate(sizeof(AuthEntity));
        storage->next = mDNSNULL;
        r->rrauth_free = storage;
    }

    // If we still have no free records, recycle all the records we can.
    // Enumerating the entire auth is moderately expensive, so when we do it, we reclaim all the records we can in one pass.
    if (!r->rrauth_free)
    {
        mDNSu32 oldtotalused = r->rrauth_totalused;
        mDNSu32 slot;
        for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
        {
            AuthGroup **cp = &r->rrauth_hash[slot];
            while (*cp)
            {
                if ((*cp)->members || (*cp)==PreserveAG) cp=&(*cp)->next;
                else ReleaseAuthGroup(r, cp);
            }
        }
        LogInfo("GetAuthEntity: Recycled %d records to reduce auth cache from %d to %d",
                oldtotalused - r->rrauth_totalused, oldtotalused, r->rrauth_totalused);
    }

    if (r->rrauth_free) // If there are records in the free list, take one
    {
        e = r->rrauth_free;
        r->rrauth_free = e->next;
        if (++r->rrauth_totalused >= r->rrauth_report)
        {
            LogInfo("RR Auth now using %ld objects", r->rrauth_totalused);
            if      (r->rrauth_report <  100) r->rrauth_report += 10;
            else if (r->rrauth_report < 1000) r->rrauth_report += 100;
            else r->rrauth_report += 1000;
        }
        mDNSPlatformMemZero(e, sizeof(*e));
    }

    r->rrauth_lock = 0;

    return(e);
}

mDNSexport AuthGroup *AuthGroupForName(AuthHash *r, const mDNSu32 slot, const mDNSu32 namehash, const domainname *const name)
{
    AuthGroup *ag;
    for (ag = r->rrauth_hash[slot]; ag; ag=ag->next)
        if (ag->namehash == namehash && SameDomainName(ag->name, name))
            break;
    return(ag);
}

mDNSexport AuthGroup *AuthGroupForRecord(AuthHash *r, const mDNSu32 slot, const ResourceRecord *const rr)
{
    return(AuthGroupForName(r, slot, rr->namehash, rr->name));
}

mDNSlocal AuthGroup *GetAuthGroup(AuthHash *r, const mDNSu32 slot, const ResourceRecord *const rr)
{
    mDNSu16 namelen = DomainNameLength(rr->name);
    AuthGroup *ag = (AuthGroup*)GetAuthEntity(r, mDNSNULL);
    if (!ag) { LogMsg("GetAuthGroup: Failed to allocate memory for %##s", rr->name->c); return(mDNSNULL); }
    ag->next         = r->rrauth_hash[slot];
    ag->namehash     = rr->namehash;
    ag->members      = mDNSNULL;
    ag->rrauth_tail  = &ag->members;
    ag->NewLocalOnlyRecords = mDNSNULL;
    if (namelen > sizeof(ag->namestorage))
        ag->name = mDNSPlatformMemAllocate(namelen);
    else
        ag->name = (domainname*)ag->namestorage;
    if (!ag->name)
    {
        LogMsg("GetAuthGroup: Failed to allocate name storage for %##s", rr->name->c);
        ReleaseAuthEntity(r, (AuthEntity*)ag);
        return(mDNSNULL);
    }
    AssignDomainName(ag->name, rr->name);

    if (AuthGroupForRecord(r, slot, rr)) LogMsg("GetAuthGroup: Already have AuthGroup for %##s", rr->name->c);
    r->rrauth_hash[slot] = ag;
    if (AuthGroupForRecord(r, slot, rr) != ag) LogMsg("GetAuthGroup: Not finding AuthGroup for %##s", rr->name->c);

    return(ag);
}

// Returns the AuthGroup in which the AuthRecord was inserted
mDNSexport AuthGroup *InsertAuthRecord(mDNS *const m, AuthHash *r, AuthRecord *rr)
{
    AuthGroup *ag;
    const mDNSu32 slot = AuthHashSlot(rr->resrec.name);
    ag = AuthGroupForRecord(r, slot, &rr->resrec);
    if (!ag) ag = GetAuthGroup(r, slot, &rr->resrec);   // If we don't have a AuthGroup for this name, make one now
    if (ag)
    {
        LogInfo("InsertAuthRecord: inserting auth record %s from table", ARDisplayString(m, rr));
        *(ag->rrauth_tail) = rr;                // Append this record to tail of cache slot list
        ag->rrauth_tail = &(rr->next);          // Advance tail pointer
    }
    return ag;
}

mDNSexport AuthGroup *RemoveAuthRecord(mDNS *const m, AuthHash *r, AuthRecord *rr)
{
    AuthGroup *a;
    AuthGroup **ag = &a;
    AuthRecord **rp;
    const mDNSu32 slot = AuthHashSlot(rr->resrec.name);

    a = AuthGroupForRecord(r, slot, &rr->resrec);
    if (!a) { LogMsg("RemoveAuthRecord: ERROR!! AuthGroup not found for %s", ARDisplayString(m, rr)); return mDNSNULL; }
    rp = &(*ag)->members;
    while (*rp)
    {
        if (*rp != rr)
            rp=&(*rp)->next;
        else
        {
            // We don't break here, so that we can set the tail below without tracking "prev" pointers

            LogInfo("RemoveAuthRecord: removing auth record %s from table", ARDisplayString(m, rr));
            *rp = (*rp)->next;          // Cut record from list
        }
    }
    // TBD: If there are no more members, release authgroup ?
    (*ag)->rrauth_tail = rp;
    return a;
}

mDNSexport CacheGroup *CacheGroupForName(const mDNS *const m, const mDNSu32 slot, const mDNSu32 namehash, const domainname *const name)
{
    CacheGroup *cg;
    for (cg = m->rrcache_hash[slot]; cg; cg=cg->next)
        if (cg->namehash == namehash && SameDomainName(cg->name, name))
            break;
    return(cg);
}

mDNSlocal CacheGroup *CacheGroupForRecord(const mDNS *const m, const mDNSu32 slot, const ResourceRecord *const rr)
{
    return(CacheGroupForName(m, slot, rr->namehash, rr->name));
}

mDNSexport mDNSBool mDNS_AddressIsLocalSubnet(mDNS *const m, const mDNSInterfaceID InterfaceID, const mDNSAddr *addr, mDNSBool *myself)
{
    NetworkInterfaceInfo *intf;

    if (addr->type == mDNSAddrType_IPv4)
    {
        // Normally we resist touching the NotAnInteger fields, but here we're doing tricky bitwise masking so we make an exception
        if (mDNSv4AddressIsLinkLocal(&addr->ip.v4)) return(mDNStrue);
        for (intf = m->HostInterfaces; intf; intf = intf->next)
            if (intf->ip.type == addr->type && intf->InterfaceID == InterfaceID && intf->McastTxRx)
                if (((intf->ip.ip.v4.NotAnInteger ^ addr->ip.v4.NotAnInteger) & intf->mask.ip.v4.NotAnInteger) == 0)
                {
                    if (myself)
                    {
                        if (mDNSSameIPv4Address(intf->ip.ip.v4, addr->ip.v4))
                            *myself = mDNStrue;
                        else
                            *myself = mDNSfalse;
                        if (*myself)
                            debugf("mDNS_AddressIsLocalSubnet: IPv4 %#a returning true", addr);
                        else
                            debugf("mDNS_AddressIsLocalSubnet: IPv4 %#a returning false", addr);
                    }
                    return(mDNStrue);
                }
    }

    if (addr->type == mDNSAddrType_IPv6)
    {
        for (intf = m->HostInterfaces; intf; intf = intf->next)
            if (intf->ip.type == addr->type && intf->InterfaceID == InterfaceID && intf->McastTxRx)
                if ((((intf->ip.ip.v6.l[0] ^ addr->ip.v6.l[0]) & intf->mask.ip.v6.l[0]) == 0) &&
                    (((intf->ip.ip.v6.l[1] ^ addr->ip.v6.l[1]) & intf->mask.ip.v6.l[1]) == 0) &&
                    (((intf->ip.ip.v6.l[2] ^ addr->ip.v6.l[2]) & intf->mask.ip.v6.l[2]) == 0) &&
                    (((intf->ip.ip.v6.l[3] ^ addr->ip.v6.l[3]) & intf->mask.ip.v6.l[3]) == 0))
                    {
                        if (myself)
                        {
                            if (mDNSSameIPv6Address(intf->ip.ip.v6, addr->ip.v6))
                                *myself = mDNStrue;
                            else
                                *myself = mDNSfalse;
                            if (*myself)
                                debugf("mDNS_AddressIsLocalSubnet: IPv6 %#a returning true", addr);
                            else
                                debugf("mDNS_AddressIsLocalSubnet: IPv6 %#a returning false", addr);
                        }
                        return(mDNStrue);
                    }
    }

    return(mDNSfalse);
}

mDNSlocal NetworkInterfaceInfo *FirstInterfaceForID(mDNS *const m, const mDNSInterfaceID InterfaceID)
{
    NetworkInterfaceInfo *intf = m->HostInterfaces;
    while (intf && intf->InterfaceID != InterfaceID) intf = intf->next;
    return(intf);
}

mDNSlocal NetworkInterfaceInfo *FirstIPv4LLInterfaceForID(mDNS *const m, const mDNSInterfaceID InterfaceID)
{
    NetworkInterfaceInfo *intf;

    if (!InterfaceID)
        return mDNSNULL;

    // Note: We don't check for InterfaceActive, as the active interface could be IPv6 and 
    // we still want to find the first IPv4 Link-Local interface
    for (intf = m->HostInterfaces; intf; intf = intf->next)
    {
        if (intf->InterfaceID == InterfaceID &&
            intf->ip.type == mDNSAddrType_IPv4 && mDNSv4AddressIsLinkLocal(&intf->ip.ip.v4))
        {
            debugf("FirstIPv4LLInterfaceForID: found LL interface with address %.4a", &intf->ip.ip.v4);
            return intf;
        }
    }
    return (mDNSNULL);
}

mDNSexport char *InterfaceNameForID(mDNS *const m, const mDNSInterfaceID InterfaceID)
{
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, InterfaceID);
    return(intf ? intf->ifname : mDNSNULL);
}

// Caller should hold the lock
mDNSlocal void GenerateNegativeResponse(mDNS *const m, QC_result qc)
{
    DNSQuestion *q;
    if (!m->CurrentQuestion) { LogMsg("GenerateNegativeResponse: ERROR!! CurrentQuestion not set"); return; }
    q = m->CurrentQuestion;
    LogInfo("GenerateNegativeResponse: Generating negative response for question %##s (%s)", q->qname.c, DNSTypeName(q->qtype));

    MakeNegativeCacheRecord(m, &m->rec.r, &q->qname, q->qnamehash, q->qtype, q->qclass, 60, mDNSInterface_Any, mDNSNULL);
    // We need to force the response through in the following cases
    //
    //  a) SuppressUnusable questions that are suppressed
    //  b) Append search domains and retry the question
    //
    // The question may not have set Intermediates in which case we don't deliver negative responses. So, to force
    // through we use "QC_forceresponse".
    AnswerCurrentQuestionWithResourceRecord(m, &m->rec.r, qc);
    if (m->CurrentQuestion == q) { q->ThisQInterval = 0; }              // Deactivate this question
    // Don't touch the question after this
    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
}

mDNSexport void AnswerQuestionByFollowingCNAME(mDNS *const m, DNSQuestion *q, ResourceRecord *rr)
{
    const mDNSBool selfref = SameDomainName(&q->qname, &rr->rdata->u.name);
    if (q->CNAMEReferrals >= 10 || selfref)
        LogMsg("AnswerQuestionByFollowingCNAME: %p %##s (%s) NOT following CNAME referral %d%s for %s",
               q, q->qname.c, DNSTypeName(q->qtype), q->CNAMEReferrals, selfref ? " (Self-Referential)" : "", RRDisplayString(m, rr));
    else
    {
        const mDNSu32 c = q->CNAMEReferrals + 1;        // Stash a copy of the new q->CNAMEReferrals value
        UDPSocket *sock = q->LocalSocket;
        mDNSOpaque16 id = q->TargetQID;

        // if there is a message waiting at the socket, we want to process that instead
        // of throwing it away. If we have a CNAME response that answers
        // both A and AAAA question and while answering it we don't want to throw
        // away the response where the actual addresses are present.
        if (mDNSPlatformPeekUDP(m, q->LocalSocket))
        {
            LogInfo("AnswerQuestionByFollowingCNAME: Preserving UDP socket for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
            q->LocalSocket = mDNSNULL;
        }
        else
        {
            sock = mDNSNULL;
        }

        // The SameDomainName check above is to ignore bogus CNAME records that point right back at
        // themselves. Without that check we can get into a case where we have two duplicate questions,
        // A and B, and when we stop question A, UpdateQuestionDuplicates copies the value of CNAMEReferrals
        // from A to B, and then A is re-appended to the end of the list as a duplicate of B (because
        // the target name is still the same), and then when we stop question B, UpdateQuestionDuplicates
        // copies the B's value of CNAMEReferrals back to A, and we end up not incrementing CNAMEReferrals
        // for either of them. This is not a problem for CNAME loops of two or more records because in
        // those cases the newly re-appended question A has a different target name and therefore cannot be
        // a duplicate of any other question ('B') which was itself a duplicate of the previous question A.

        // Right now we just stop and re-use the existing query. If we really wanted to be 100% perfect,
        // and track CNAMEs coming and going, we should really create a subordinate query here,
        // which we would subsequently cancel and retract if the CNAME referral record were removed.
        // In reality this is such a corner case we'll ignore it until someone actually needs it.

        LogInfo("AnswerQuestionByFollowingCNAME: %p %##s (%s) following CNAME referral %d for %s",
                q, q->qname.c, DNSTypeName(q->qtype), q->CNAMEReferrals, RRDisplayString(m, rr));

        mDNS_StopQuery_internal(m, q);                              // Stop old query
        AssignDomainName(&q->qname, &rr->rdata->u.name);            // Update qname
        q->qnamehash = DomainNameHashValue(&q->qname);              // and namehash
        // If a unicast query results in a CNAME that points to a .local, we need to re-try
        // this as unicast. Setting the mDNSInterface_Unicast tells mDNS_StartQuery_internal
        // to try this as unicast query even though it is a .local name
        if (!mDNSOpaque16IsZero(q->TargetQID) && IsLocalDomain(&q->qname))
        {
            LogInfo("AnswerQuestionByFollowingCNAME: Resolving a .local CNAME %p %##s (%s) Record %s",
                    q, q->qname.c, DNSTypeName(q->qtype), RRDisplayString(m, rr));
            q->InterfaceID = mDNSInterface_Unicast;
        }
        mDNS_StartQuery_internal(m, q);                             // start new query
        // Record how many times we've done this. We need to do this *after* mDNS_StartQuery_internal,
        // because mDNS_StartQuery_internal re-initializes CNAMEReferrals to zero
        q->CNAMEReferrals = c;
        if (sock)
        {
            // We have a message waiting and that should answer this question.
            if (q->LocalSocket)
                mDNSPlatformUDPClose(q->LocalSocket);
            q->LocalSocket = sock;
            q->TargetQID = id;
        }
    }
}

// For a single given DNSQuestion pointed to by CurrentQuestion, deliver an add/remove result for the single given AuthRecord
// Note: All the callers should use the m->CurrentQuestion to see if the question is still valid or not
mDNSlocal void AnswerLocalQuestionWithLocalAuthRecord(mDNS *const m, AuthRecord *rr, QC_result AddRecord)
{
    DNSQuestion *q = m->CurrentQuestion;
    mDNSBool followcname;

    if (!q)
    {
        LogMsg("AnswerLocalQuestionWithLocalAuthRecord: ERROR!! CurrentQuestion NULL while answering with %s", ARDisplayString(m, rr));
        return;
    }

    followcname = FollowCNAME(q, &rr->resrec, AddRecord);

    // We should not be delivering results for record types Unregistered, Deregistering, and (unverified) Unique
    if (!(rr->resrec.RecordType & kDNSRecordTypeActiveMask))
    {
        LogMsg("AnswerLocalQuestionWithLocalAuthRecord: *NOT* delivering %s event for local record type %X %s",
               AddRecord ? "Add" : "Rmv", rr->resrec.RecordType, ARDisplayString(m, rr));
        return;
    }

    // Indicate that we've given at least one positive answer for this record, so we should be prepared to send a goodbye for it
    if (AddRecord) rr->AnsweredLocalQ = mDNStrue;
    mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
    if (q->QuestionCallback && !q->NoAnswer)
    {
        q->CurrentAnswers += AddRecord ? 1 : -1;
        if (UniqueLocalOnlyRecord(rr))
        {
            if (!followcname || q->ReturnIntermed)
            {
                // Don't send this packet on the wire as we answered from /etc/hosts
                q->ThisQInterval = 0;
                q->LOAddressAnswers += AddRecord ? 1 : -1;
                q->QuestionCallback(m, q, &rr->resrec, AddRecord);
            }
            mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
            // The callback above could have caused the question to stop. Detect that
            // using m->CurrentQuestion
            if (followcname && m->CurrentQuestion == q)
                AnswerQuestionByFollowingCNAME(m, q, &rr->resrec);
            return;
        }
        else
        {
            q->QuestionCallback(m, q, &rr->resrec, AddRecord);
        }
    }
    mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
}

mDNSlocal void AnswerInterfaceAnyQuestionsWithLocalAuthRecord(mDNS *const m, AuthRecord *rr, QC_result AddRecord)
{
    if (m->CurrentQuestion)
        LogMsg("AnswerInterfaceAnyQuestionsWithLocalAuthRecord: ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    while (m->CurrentQuestion && m->CurrentQuestion != m->NewQuestions)
    {
        mDNSBool answered;
        DNSQuestion *q = m->CurrentQuestion;
        if (RRAny(rr))
            answered = ResourceRecordAnswersQuestion(&rr->resrec, q);
        else
            answered = LocalOnlyRecordAnswersQuestion(rr, q);
        if (answered)
            AnswerLocalQuestionWithLocalAuthRecord(m, rr, AddRecord);       // MUST NOT dereference q again
        if (m->CurrentQuestion == q)    // If m->CurrentQuestion was not auto-advanced, do it ourselves now
            m->CurrentQuestion = q->next;
    }
    m->CurrentQuestion = mDNSNULL;
}

// When a new local AuthRecord is created or deleted, AnswerAllLocalQuestionsWithLocalAuthRecord()
// delivers the appropriate add/remove events to listening questions:
// 1. It runs though all our LocalOnlyQuestions delivering answers as appropriate,
//    stopping if it reaches a NewLocalOnlyQuestion -- brand-new questions are handled by AnswerNewLocalOnlyQuestion().
// 2. If the AuthRecord is marked mDNSInterface_LocalOnly or mDNSInterface_P2P, then it also runs though
//    our main question list, delivering answers to mDNSInterface_Any questions as appropriate,
//    stopping if it reaches a NewQuestion -- brand-new questions are handled by AnswerNewQuestion().
//
// AnswerAllLocalQuestionsWithLocalAuthRecord is used by the m->NewLocalRecords loop in mDNS_Execute(),
// and by mDNS_Deregister_internal()

mDNSlocal void AnswerAllLocalQuestionsWithLocalAuthRecord(mDNS *const m, AuthRecord *rr, QC_result AddRecord)
{
    if (m->CurrentQuestion)
        LogMsg("AnswerAllLocalQuestionsWithLocalAuthRecord ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));

    m->CurrentQuestion = m->LocalOnlyQuestions;
    while (m->CurrentQuestion && m->CurrentQuestion != m->NewLocalOnlyQuestions)
    {
        mDNSBool answered;
        DNSQuestion *q = m->CurrentQuestion;
        // We are called with both LocalOnly/P2P record or a regular AuthRecord
        if (RRAny(rr))
            answered = ResourceRecordAnswersQuestion(&rr->resrec, q);
        else
            answered = LocalOnlyRecordAnswersQuestion(rr, q);
        if (answered)
            AnswerLocalQuestionWithLocalAuthRecord(m, rr, AddRecord);           // MUST NOT dereference q again
        if (m->CurrentQuestion == q)    // If m->CurrentQuestion was not auto-advanced, do it ourselves now
            m->CurrentQuestion = q->next;
    }

    m->CurrentQuestion = mDNSNULL;

    // If this AuthRecord is marked LocalOnly or P2P, then we want to deliver it to all local 'mDNSInterface_Any' questions
    if (rr->ARType == AuthRecordLocalOnly || rr->ARType == AuthRecordP2P)
        AnswerInterfaceAnyQuestionsWithLocalAuthRecord(m, rr, AddRecord);

}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Resource Record Utility Functions
#endif

#define RRTypeIsAddressType(T) ((T) == kDNSType_A || (T) == kDNSType_AAAA)

#define ResourceRecordIsValidAnswer(RR) ( ((RR)->resrec.RecordType & kDNSRecordTypeActiveMask)  && \
                                          ((RR)->Additional1 == mDNSNULL || ((RR)->Additional1->resrec.RecordType & kDNSRecordTypeActiveMask)) && \
                                          ((RR)->Additional2 == mDNSNULL || ((RR)->Additional2->resrec.RecordType & kDNSRecordTypeActiveMask)) && \
                                          ((RR)->DependentOn == mDNSNULL || ((RR)->DependentOn->resrec.RecordType & kDNSRecordTypeActiveMask))  )

#define ResourceRecordIsValidInterfaceAnswer(RR, INTID) \
    (ResourceRecordIsValidAnswer(RR) && \
     ((RR)->resrec.InterfaceID == mDNSInterface_Any || (RR)->resrec.InterfaceID == (INTID)))
#if defined (MTK_ENHANCE_BONJOUR)
#define DefaultProbeCountForTypeUnique ((mDNSu8)8)
#else
#define DefaultProbeCountForTypeUnique ((mDNSu8)3)
#endif
#define DefaultProbeCountForRecordType(X)      ((X) == kDNSRecordTypeUnique ? DefaultProbeCountForTypeUnique : (mDNSu8)0)

// See RFC 6762: "8.3 Announcing"
// "The Multicast DNS responder MUST send at least two unsolicited responses, one second apart."
// Send 4, which is really 8 since we send on both IPv4 and IPv6.
#if defined (MTK_ENHANCE_BONJOUR)
#define InitialAnnounceCount ((mDNSu8)12)
#else
#define InitialAnnounceCount ((mDNSu8)4)
#endif
// For goodbye packets we set the count to 3, and for wakeups we set it to 18
// (which will be up to 15 wakeup attempts over the course of 30 seconds,
// and then if the machine fails to wake, 3 goodbye packets).
#define GoodbyeCount ((mDNSu8)3)
#define WakeupCount ((mDNSu8)18)
#define MAX_PROBE_RESTARTS ((mDNSu8)20)

// Number of wakeups we send if WakeOnResolve is set in the question
#define InitialWakeOnResolveCount ((mDNSu8)3)

// Note that the announce intervals use exponential backoff, doubling each time. The probe intervals do not.
// This means that because the announce interval is doubled after sending the first packet, the first
// observed on-the-wire inter-packet interval between announcements is actually one second.
// The half-second value here may be thought of as a conceptual (non-existent) half-second delay *before* the first packet is sent.
#if defined (MTK_ENHANCE_BONJOUR)
#define DefaultProbeIntervalForTypeUnique (mDNSPlatformOneSecond/2)
#define DefaultAnnounceIntervalForTypeShared (mDNSPlatformOneSecond/1)
#define DefaultAnnounceIntervalForTypeUnique (mDNSPlatformOneSecond/1)
#else
#define DefaultProbeIntervalForTypeUnique (mDNSPlatformOneSecond/4)
#define DefaultAnnounceIntervalForTypeShared (mDNSPlatformOneSecond/2)
#define DefaultAnnounceIntervalForTypeUnique (mDNSPlatformOneSecond/2)
#endif
#define DefaultAPIntervalForRecordType(X)  ((X) &kDNSRecordTypeActiveSharedMask ? DefaultAnnounceIntervalForTypeShared : \
                                            (X) &kDNSRecordTypeUnique           ? DefaultProbeIntervalForTypeUnique    : \
                                            (X) &kDNSRecordTypeActiveUniqueMask ? DefaultAnnounceIntervalForTypeUnique : 0)

#define TimeToAnnounceThisRecord(RR,time) ((RR)->AnnounceCount && (time) - ((RR)->LastAPTime + (RR)->ThisAPInterval) >= 0)
#define TimeToSendThisRecord(RR,time) ((TimeToAnnounceThisRecord(RR,time) || (RR)->ImmedAnswer) && ResourceRecordIsValidAnswer(RR))
#define TicksTTL(RR) ((mDNSs32)(RR)->resrec.rroriginalttl * mDNSPlatformOneSecond)
#define RRExpireTime(RR) ((RR)->TimeRcvd + TicksTTL(RR))

// Adjustment factor to avoid race condition (used for unicast cache entries) :
// Suppose real record has TTL of 3600, and our local caching server has held it for 3500 seconds, so it returns an aged TTL of 100.
// If we do our normal refresh at 80% of the TTL, our local caching server will return 20 seconds, so we'll do another
// 80% refresh after 16 seconds, and then the server will return 4 seconds, and so on, in the fashion of Zeno's paradox.
// To avoid this, we extend the record's effective TTL to give it a little extra grace period.
// We adjust the 100 second TTL to 127. This means that when we do our 80% query at 102 seconds,
// the cached copy at our local caching server will already have expired, so the server will be forced
// to fetch a fresh copy from the authoritative server, and then return a fresh record with the full TTL of 3600 seconds.

#define RRAdjustTTL(ttl) ((ttl) + ((ttl)/4) + 2)
#define RRUnadjustedTTL(ttl) ((((ttl) - 2) * 4) / 5)

#define MaxUnansweredQueries 4

// SameResourceRecordSignature returns true if two resources records have the same name, type, and class, and may be sent
// (or were received) on the same interface (i.e. if *both* records specify an interface, then it has to match).
// TTL and rdata may differ.
// This is used for cache flush management:
// When sending a unique record, all other records matching "SameResourceRecordSignature" must also be sent
// When receiving a unique record, all old cache records matching "SameResourceRecordSignature" are flushed

// SameResourceRecordNameClassInterface is functionally the same as SameResourceRecordSignature, except rrtype does not have to match

#define SameResourceRecordSignature(A,B) (A)->resrec.rrtype == (B)->resrec.rrtype && SameResourceRecordNameClassInterface((A),(B))

mDNSlocal mDNSBool SameResourceRecordNameClassInterface(const AuthRecord *const r1, const AuthRecord *const r2)
{
    if (!r1) { LogMsg("SameResourceRecordSignature ERROR: r1 is NULL"); return(mDNSfalse); }
    if (!r2) { LogMsg("SameResourceRecordSignature ERROR: r2 is NULL"); return(mDNSfalse); }
    if (r1->resrec.InterfaceID &&
        r2->resrec.InterfaceID &&
        r1->resrec.InterfaceID != r2->resrec.InterfaceID) return(mDNSfalse);
    return (mDNSBool)(
               r1->resrec.rrclass  == r2->resrec.rrclass &&
               r1->resrec.namehash == r2->resrec.namehash &&
               SameDomainName(r1->resrec.name, r2->resrec.name));
}

// PacketRRMatchesSignature behaves as SameResourceRecordSignature, except that types may differ if our
// authoratative record is unique (as opposed to shared). For unique records, we are supposed to have
// complete ownership of *all* types for this name, so *any* record type with the same name is a conflict.
// In addition, when probing we send our questions with the wildcard type kDNSQType_ANY,
// so a response of any type should match, even if it is not actually the type the client plans to use.

// For now, to make it easier to avoid false conflicts, we treat SPS Proxy records like shared records,
// and require the rrtypes to match for the rdata to be considered potentially conflicting
mDNSlocal mDNSBool PacketRRMatchesSignature(const CacheRecord *const pktrr, const AuthRecord *const authrr)
{
    if (!pktrr)  { LogMsg("PacketRRMatchesSignature ERROR: pktrr is NULL"); return(mDNSfalse); }
    if (!authrr) { LogMsg("PacketRRMatchesSignature ERROR: authrr is NULL"); return(mDNSfalse); }
    if (pktrr->resrec.InterfaceID &&
        authrr->resrec.InterfaceID &&
        pktrr->resrec.InterfaceID != authrr->resrec.InterfaceID) return(mDNSfalse);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (!(authrr->resrec.RecordType & kDNSRecordTypeUniqueMask) || authrr->WakeUp.HMAC.l[0])
#else
    if (!(authrr->resrec.RecordType & kDNSRecordTypeUniqueMask))
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (pktrr->resrec.rrtype != authrr->resrec.rrtype) return(mDNSfalse);
    return (mDNSBool)(
               pktrr->resrec.rrclass == authrr->resrec.rrclass &&
               pktrr->resrec.namehash == authrr->resrec.namehash &&
               SameDomainName(pktrr->resrec.name, authrr->resrec.name));
}

// CacheRecord *ka is the CacheRecord from the known answer list in the query.
// This is the information that the requester believes to be correct.
// AuthRecord *rr is the answer we are proposing to give, if not suppressed.
// This is the information that we believe to be correct.
// We've already determined that we plan to give this answer on this interface
// (either the record is non-specific, or it is specific to this interface)
// so now we just need to check the name, type, class, rdata and TTL.
mDNSlocal mDNSBool ShouldSuppressKnownAnswer(const CacheRecord *const ka, const AuthRecord *const rr)
{
    // If RR signature is different, or data is different, then don't suppress our answer
    if (!IdenticalResourceRecord(&ka->resrec, &rr->resrec)) return(mDNSfalse);

    // If the requester's indicated TTL is less than half the real TTL,
    // we need to give our answer before the requester's copy expires.
    // If the requester's indicated TTL is at least half the real TTL,
    // then we can suppress our answer this time.
    // If the requester's indicated TTL is greater than the TTL we believe,
    // then that's okay, and we don't need to do anything about it.
    // (If two responders on the network are offering the same information,
    // that's okay, and if they are offering the information with different TTLs,
    // the one offering the lower TTL should defer to the one offering the higher TTL.)
    return (mDNSBool)(ka->resrec.rroriginalttl >= rr->resrec.rroriginalttl / 2);
}

mDNSlocal void SetNextAnnounceProbeTime(mDNS *const m, const AuthRecord *const rr)
{
    if (rr->resrec.RecordType == kDNSRecordTypeUnique)
    {
        if ((rr->LastAPTime + rr->ThisAPInterval) - m->timenow > mDNSPlatformOneSecond * 10)
        {
            LogMsg("SetNextAnnounceProbeTime: ProbeCount %d Next in %d %s", rr->ProbeCount, (rr->LastAPTime + rr->ThisAPInterval) - m->timenow, ARDisplayString(m, rr));
            LogMsg("SetNextAnnounceProbeTime: m->SuppressProbes %d m->timenow %d diff %d", m->SuppressProbes, m->timenow, m->SuppressProbes - m->timenow);
        }
        if (m->NextScheduledProbe - (rr->LastAPTime + rr->ThisAPInterval) >= 0)
            m->NextScheduledProbe = (rr->LastAPTime + rr->ThisAPInterval);
        // Some defensive code:
        // If (rr->LastAPTime + rr->ThisAPInterval) happens to be far in the past, we don't want to allow
        // NextScheduledProbe to be set excessively in the past, because that can cause bad things to happen.
        // See: <rdar://problem/7795434> mDNS: Sometimes advertising stops working and record interval is set to zero
        if (m->NextScheduledProbe - m->timenow < 0)
            m->NextScheduledProbe = m->timenow;
    }
    else if (rr->AnnounceCount && (ResourceRecordIsValidAnswer(rr) || rr->resrec.RecordType == kDNSRecordTypeDeregistering))
    {
        if (m->NextScheduledResponse - (rr->LastAPTime + rr->ThisAPInterval) >= 0)
            m->NextScheduledResponse = (rr->LastAPTime + rr->ThisAPInterval);
    }
}

mDNSlocal void InitializeLastAPTime(mDNS *const m, AuthRecord *const rr)
{
    // For reverse-mapping Sleep Proxy PTR records, probe interval is one second
    rr->ThisAPInterval = rr->AddressProxy.type ? mDNSPlatformOneSecond : DefaultAPIntervalForRecordType(rr->resrec.RecordType);

    // * If this is a record type that's going to probe, then we use the m->SuppressProbes time.
    // * Otherwise, if it's not going to probe, but m->SuppressProbes is set because we have other
    //   records that are going to probe, then we delay its first announcement so that it will
    //   go out synchronized with the first announcement for the other records that *are* probing.
    //   This is a minor performance tweak that helps keep groups of related records synchronized together.
    //   The addition of "interval / 2" is to make sure that, in the event that any of the probes are
    //   delayed by a few milliseconds, this announcement does not inadvertently go out *before* the probing is complete.
    //   When the probing is complete and those records begin to announce, these records will also be picked up and accelerated,
    //   because they will meet the criterion of being at least half-way to their scheduled announcement time.
    // * If it's not going to probe and m->SuppressProbes is not already set then we should announce immediately.

    if (rr->ProbeCount)
    {
        // If we have no probe suppression time set, or it is in the past, set it now
        if (m->SuppressProbes == 0 || m->SuppressProbes - m->timenow < 0)
        {
            // To allow us to aggregate probes when a group of services are registered together,
            // the first probe is delayed 1/4 second. This means the common-case behaviour is:
            // 1/4 second wait; probe
            // 1/4 second wait; probe
            // 1/4 second wait; probe
            // 1/4 second wait; announce (i.e. service is normally announced exactly one second after being registered)
            m->SuppressProbes = NonZeroTime(m->timenow + DefaultProbeIntervalForTypeUnique/2 + mDNSRandom(DefaultProbeIntervalForTypeUnique/2));

            // If we already have a *probe* scheduled to go out sooner, then use that time to get better aggregation
            if (m->SuppressProbes - m->NextScheduledProbe >= 0)
                m->SuppressProbes = NonZeroTime(m->NextScheduledProbe);
            if (m->SuppressProbes - m->timenow < 0)     // Make sure we don't set m->SuppressProbes excessively in the past
                m->SuppressProbes = m->timenow;

            // If we already have a *query* scheduled to go out sooner, then use that time to get better aggregation
            if (m->SuppressProbes - m->NextScheduledQuery >= 0)
                m->SuppressProbes = NonZeroTime(m->NextScheduledQuery);
            if (m->SuppressProbes - m->timenow < 0)     // Make sure we don't set m->SuppressProbes excessively in the past
                m->SuppressProbes = m->timenow;

            // except... don't expect to be able to send before the m->SuppressSending timer fires
            if (m->SuppressSending && m->SuppressProbes - m->SuppressSending < 0)
                m->SuppressProbes = NonZeroTime(m->SuppressSending);

            if (m->SuppressProbes - m->timenow > mDNSPlatformOneSecond * 8)
            {
                LogMsg("InitializeLastAPTime ERROR m->SuppressProbes %d m->NextScheduledProbe %d m->NextScheduledQuery %d m->SuppressSending %d %d",
                       m->SuppressProbes     - m->timenow,
                       m->NextScheduledProbe - m->timenow,
                       m->NextScheduledQuery - m->timenow,
                       m->SuppressSending,
                       m->SuppressSending    - m->timenow);
                m->SuppressProbes = NonZeroTime(m->timenow + DefaultProbeIntervalForTypeUnique/2 + mDNSRandom(DefaultProbeIntervalForTypeUnique/2));
            }
        }
        rr->LastAPTime = m->SuppressProbes - rr->ThisAPInterval;
    }
    else if (m->SuppressProbes && m->SuppressProbes - m->timenow >= 0)
        rr->LastAPTime = m->SuppressProbes - rr->ThisAPInterval + DefaultProbeIntervalForTypeUnique * DefaultProbeCountForTypeUnique + rr->ThisAPInterval / 2;
    else
        rr->LastAPTime = m->timenow - rr->ThisAPInterval;

    // For reverse-mapping Sleep Proxy PTR records we don't want to start probing instantly -- we
    // wait one second to give the client a chance to go to sleep, and then start our ARP/NDP probing.
    // After three probes one second apart with no answer, we conclude the client is now sleeping
    // and we can begin broadcasting our announcements to take over ownership of that IP address.
    // If we don't wait for the client to go to sleep, then when the client sees our ARP Announcements there's a risk
    // (depending on the OS and networking stack it's using) that it might interpret it as a conflict and change its IP address.
    if (rr->AddressProxy.type) 
        rr->LastAPTime = m->timenow;

    // Set LastMCTime to now, to inhibit multicast responses
    // (no need to send additional multicast responses when we're announcing anyway)
    rr->LastMCTime      = m->timenow;
    rr->LastMCInterface = mDNSInterfaceMark;

    SetNextAnnounceProbeTime(m, rr);
}

mDNSlocal const domainname *SetUnicastTargetToHostName(mDNS *const m, AuthRecord *rr)
{
    const domainname *target;
#ifndef REMOVE_NAT_FUCTIONALITY
    if (rr->AutoTarget)
    {
        // For autotunnel services pointing at our IPv6 ULA we don't need or want a NAT mapping, but for all other
        // advertised services referencing our uDNS hostname, we want NAT mappings automatically created as appropriate,
        // with the port number in our advertised SRV record automatically tracking the external mapped port.
        DomainAuthInfo *AuthInfo = GetAuthInfoForName_internal(m, rr->resrec.name);
        if (!AuthInfo || !AuthInfo->AutoTunnel) rr->AutoTarget = Target_AutoHostAndNATMAP;
    }
#endif //#ifndef REMOVE_NAT_FUCTIONALITY

    target = GetServiceTarget(m, rr);
    if (!target || target->c[0] == 0)
    {
        // defer registration until we've got a target
        LogInfo("SetUnicastTargetToHostName No target for %s", ARDisplayString(m, rr));
        rr->state = regState_NoTarget;
        return mDNSNULL;
    }
    else
    {
        LogInfo("SetUnicastTargetToHostName target %##s for resource record %s", target->c, ARDisplayString(m,rr));
        return target;
    }
}

// Right now this only applies to mDNS (.local) services where the target host is always m->MulticastHostname
// Eventually we should unify this with GetServiceTarget() in uDNS.c
mDNSlocal void SetTargetToHostName(mDNS *const m, AuthRecord *const rr)
{
    domainname *const target = GetRRDomainNameTarget(&rr->resrec);
    const domainname *newname = &m->MulticastHostname;

    if (!target) LogInfo("SetTargetToHostName: Don't know how to set the target of rrtype %s", DNSTypeName(rr->resrec.rrtype));

    if (!(rr->ForceMCast || rr->ARType == AuthRecordLocalOnly || rr->ARType == AuthRecordP2P || IsLocalDomain(&rr->namestorage)))
    {
        const domainname *const n = SetUnicastTargetToHostName(m, rr);
        if (n) newname = n;
        else { target->c[0] = 0; SetNewRData(&rr->resrec, mDNSNULL, 0); return; }
    }

    if (target && SameDomainName(target, newname))
        debugf("SetTargetToHostName: Target of %##s is already %##s", rr->resrec.name->c, target->c);

    if (target && !SameDomainName(target, newname))
    {
        AssignDomainName(target, newname);
        SetNewRData(&rr->resrec, mDNSNULL, 0);      // Update rdlength, rdestimate, rdatahash

        // If we're in the middle of probing this record, we need to start again,
        // because changing its rdata may change the outcome of the tie-breaker.
        // (If the record type is kDNSRecordTypeUnique (unconfirmed unique) then DefaultProbeCountForRecordType is non-zero.)
        rr->ProbeCount     = DefaultProbeCountForRecordType(rr->resrec.RecordType);

        // If we've announced this record, we really should send a goodbye packet for the old rdata before
        // changing to the new rdata. However, in practice, we only do SetTargetToHostName for unique records,
        // so when we announce them we'll set the kDNSClass_UniqueRRSet and clear any stale data that way.
        if (rr->RequireGoodbye && rr->resrec.RecordType == kDNSRecordTypeShared)
            debugf("Have announced shared record %##s (%s) at least once: should have sent a goodbye packet before updating",
                   rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));

        rr->AnnounceCount  = InitialAnnounceCount;
        rr->RequireGoodbye = mDNSfalse;
        rr->ProbeRestartCount = 0;
        InitializeLastAPTime(m, rr);
    }
}

mDNSlocal void AcknowledgeRecord(mDNS *const m, AuthRecord *const rr)
{
    if (rr->RecordCallback)
    {
        // CAUTION: MUST NOT do anything more with rr after calling rr->Callback(), because the client's callback function
        // is allowed to do anything, including starting/stopping queries, registering/deregistering records, etc.
        rr->Acknowledged = mDNStrue;
        mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
        rr->RecordCallback(m, rr, mStatus_NoError);
        mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
    }
}

mDNSexport void ActivateUnicastRegistration(mDNS *const m, AuthRecord *const rr)
{
    // Make sure that we don't activate the SRV record and associated service records, if it is in
    // NoTarget state. First time when a service is being instantiated, SRV record may be in NoTarget state.
    // We should not activate any of the other reords (PTR, TXT) that are part of the service. When
    // the target becomes available, the records will be reregistered.
    if (rr->resrec.rrtype != kDNSType_SRV)
    {
        AuthRecord *srvRR = mDNSNULL;
        if (rr->resrec.rrtype == kDNSType_PTR)
            srvRR = rr->Additional1;
        else if (rr->resrec.rrtype == kDNSType_TXT)
            srvRR = rr->DependentOn;
        if (srvRR)
        {
            if (srvRR->resrec.rrtype != kDNSType_SRV)
            {
                LogMsg("ActivateUnicastRegistration: ERROR!! Resource record %s wrong, expecting SRV type", ARDisplayString(m, srvRR));
            }
            else
            {
                LogInfo("ActivateUnicastRegistration: Found Service Record %s in state %d for %##s (%s)",
                        ARDisplayString(m, srvRR), srvRR->state, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
                rr->state = srvRR->state;
            }
        }
    }

    if (rr->state == regState_NoTarget)
    {
        LogInfo("ActivateUnicastRegistration record %s in regState_NoTarget, not activating", ARDisplayString(m, rr));
        return;
    }
    // When we wake up from sleep, we call ActivateUnicastRegistration. It is possible that just before we went to sleep,
    // the service/record was being deregistered. In that case, we should not try to register again. For the cases where
    // the records are deregistered due to e.g., no target for the SRV record, we would have returned from above if it
    // was already in NoTarget state. If it was in the process of deregistration but did not complete fully before we went
    // to sleep, then it is okay to start in Pending state as we will go back to NoTarget state if we don't have a target.
    if (rr->resrec.RecordType == kDNSRecordTypeDeregistering)
    {
        LogInfo("ActivateUnicastRegistration: Resource record %s, current state %d, moving to DeregPending", ARDisplayString(m, rr), rr->state);
        rr->state = regState_DeregPending;
    }
    else
    {
        LogInfo("ActivateUnicastRegistration: Resource record %s, current state %d, moving to Pending", ARDisplayString(m, rr), rr->state);
        rr->state = regState_Pending;
    }
    rr->ProbeCount     = 0;
    rr->ProbeRestartCount = 0;
    rr->AnnounceCount  = 0;
    rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
    rr->LastAPTime     = m->timenow - rr->ThisAPInterval;
    rr->expire         = 0; // Forget about all the leases, start fresh
    rr->uselease       = mDNStrue;
    rr->updateid       = zeroID;
    rr->SRVChanged     = mDNSfalse;
    rr->updateError    = mStatus_NoError;
    // RestartRecordGetZoneData calls this function whenever a new interface gets registered with core.
    // The records might already be registered with the server and hence could have NAT state.
#ifndef REMOVE_NAT_FUCTIONALITY
    if (rr->NATinfo.clientContext)
    {
        mDNS_StopNATOperation_internal(m, &rr->NATinfo);
        rr->NATinfo.clientContext = mDNSNULL;
    }
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    if (rr->nta) { CancelGetZoneData(m, rr->nta); rr->nta = mDNSNULL; }
    if (rr->tcp) { DisposeTCPConn(rr->tcp);       rr->tcp = mDNSNULL; }
    if (m->NextuDNSEvent - (rr->LastAPTime + rr->ThisAPInterval) >= 0)
        m->NextuDNSEvent = (rr->LastAPTime + rr->ThisAPInterval);
}

// Two records qualify to be local duplicates if:
// (a) the RecordTypes are the same, or
// (b) one is Unique and the other Verified
// (c) either is in the process of deregistering
#define RecordLDT(A,B) ((A)->resrec.RecordType == (B)->resrec.RecordType || \
                        ((A)->resrec.RecordType | (B)->resrec.RecordType) == (kDNSRecordTypeUnique | kDNSRecordTypeVerified) || \
                        ((A)->resrec.RecordType == kDNSRecordTypeDeregistering || (B)->resrec.RecordType == kDNSRecordTypeDeregistering))

#define RecordIsLocalDuplicate(A,B) \
    ((A)->resrec.InterfaceID == (B)->resrec.InterfaceID && RecordLDT((A),(B)) && IdenticalResourceRecord(& (A)->resrec, & (B)->resrec))

mDNSlocal AuthRecord *CheckAuthIdenticalRecord(AuthHash *r, AuthRecord *rr)
{
    AuthGroup *a;
    AuthGroup **ag = &a;
    AuthRecord **rp;
    const mDNSu32 slot = AuthHashSlot(rr->resrec.name);

    a = AuthGroupForRecord(r, slot, &rr->resrec);
    if (!a) return mDNSNULL;
    rp = &(*ag)->members;
    while (*rp)
    {
        if (!RecordIsLocalDuplicate(*rp, rr))
            rp=&(*rp)->next;
        else
        {
            if ((*rp)->resrec.RecordType == kDNSRecordTypeDeregistering)
            {
                (*rp)->AnnounceCount = 0;
                rp=&(*rp)->next;
            }
            else return *rp;
        }
    }
    return (mDNSNULL);
}

mDNSlocal mDNSBool CheckAuthRecordConflict(AuthHash *r, AuthRecord *rr)
{
    AuthGroup *a;
    AuthGroup **ag = &a;
    AuthRecord **rp;
    const mDNSu32 slot = AuthHashSlot(rr->resrec.name);

    a = AuthGroupForRecord(r, slot, &rr->resrec);
    if (!a) return mDNSfalse;
    rp = &(*ag)->members;
    while (*rp)
    {
        const AuthRecord *s1 = rr->RRSet ? rr->RRSet : rr;
        const AuthRecord *s2 = (*rp)->RRSet ? (*rp)->RRSet : *rp;
        if (s1 != s2 && SameResourceRecordSignature((*rp), rr) && !IdenticalSameNameRecord(&(*rp)->resrec, &rr->resrec))
            return mDNStrue;
        else
            rp=&(*rp)->next;
    }
    return (mDNSfalse);
}

// checks to see if "rr" is already present
mDNSlocal AuthRecord *CheckAuthSameRecord(AuthHash *r, AuthRecord *rr)
{
    AuthGroup *a;
    AuthGroup **ag = &a;
    AuthRecord **rp;
    const mDNSu32 slot = AuthHashSlot(rr->resrec.name);

    a = AuthGroupForRecord(r, slot, &rr->resrec);
    if (!a) return mDNSNULL;
    rp = &(*ag)->members;
    while (*rp)
    {
        if (*rp != rr)
            rp=&(*rp)->next;
        else
        {
            return *rp;
        }
    }
    return (mDNSNULL);
}


mDNSlocal void DecrementAutoTargetServices(mDNS *const m, AuthRecord *const rr)
{
    if (!AuthRecord_uDNS(rr) && rr->resrec.rrtype == kDNSType_SRV && rr->AutoTarget == Target_AutoHost)
    {
        m->AutoTargetServices--;
        LogInfo("DecrementAutoTargetServices: AutoService Record %s, AutoTargetServices %d", ARDisplayString(m, rr), m->AutoTargetServices);
        if (!m->AutoTargetServices)
            DeadvertiseAllInterfaceRecords(m);
    }
}

mDNSlocal void IncrementAutoTargetServices(mDNS *const m, AuthRecord *const rr)
{
    if (!AuthRecord_uDNS(rr) && rr->resrec.rrtype == kDNSType_SRV && rr->AutoTarget == Target_AutoHost)
    {
        int count = m->AutoTargetServices;

        // Bump up before calling AdvertiseAllInterfaceRecords. AdvertiseInterface
        // returns without doing anything if the count is zero.
        m->AutoTargetServices++;
        LogInfo("IncrementAutoTargetServices: AutoService Record %s, AutoTargetServices %d", ARDisplayString(m, rr), m->AutoTargetServices);
        if (!count)
            AdvertiseAllInterfaceRecords(m);
    }
}
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void getKeepaliveRaddr(mDNS *const m, AuthRecord *rr, mDNSAddr *raddr)
{
    mDNSAddr     laddr;
    mDNSEthAddr  eth;
    mDNSIPPort   lport, rport;
    mDNSu32      timeout, seq, ack;
    mDNSu16      win;

    if (mDNS_KeepaliveRecord(&rr->resrec))
    {
        mDNS_ExtractKeepaliveInfo(rr, &timeout, &laddr, raddr, &eth, &seq, &ack, &lport, &rport, &win);
        if (!timeout || mDNSAddressIsZero(&laddr) || mDNSAddressIsZero(raddr) || mDNSIPPortIsZero(lport) || mDNSIPPortIsZero(rport))
        {
            LogMsg("getKeepaliveRaddr: not a valid record %s for keepalive %#a:%d %#a:%d", ARDisplayString(m, rr), &laddr, lport.NotAnInteger, raddr, rport.NotAnInteger);
            return;
        }
    }
}
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
// Exported so uDNS.c can call this
mDNSexport mStatus mDNS_Register_internal(mDNS *const m, AuthRecord *const rr)
{
    domainname *target = GetRRDomainNameTarget(&rr->resrec);
    AuthRecord *r;
    AuthRecord **p = &m->ResourceRecords;
    AuthRecord **d = &m->DuplicateRecords;

    if ((mDNSs32)rr->resrec.rroriginalttl <= 0)
    { LogMsg("mDNS_Register_internal: TTL %X should be 1 - 0x7FFFFFFF %s", rr->resrec.rroriginalttl, ARDisplayString(m, rr)); return(mStatus_BadParamErr); }

    if (!rr->resrec.RecordType)
    { LogMsg("mDNS_Register_internal: RecordType must be non-zero %s", ARDisplayString(m, rr)); return(mStatus_BadParamErr); }

    if (m->ShutdownTime)
    { LogMsg("mDNS_Register_internal: Shutting down, can't register %s", ARDisplayString(m, rr)); return(mStatus_ServiceNotRunning); }

    if (m->DivertMulticastAdvertisements && !AuthRecord_uDNS(rr))
    {
        mDNSInterfaceID previousID = rr->resrec.InterfaceID;
        if (rr->resrec.InterfaceID == mDNSInterface_Any || rr->resrec.InterfaceID == mDNSInterface_P2P)
        {
            rr->resrec.InterfaceID = mDNSInterface_LocalOnly;
            rr->ARType = AuthRecordLocalOnly;
        }
        if (rr->resrec.InterfaceID != mDNSInterface_LocalOnly)
        {
            NetworkInterfaceInfo *intf = FirstInterfaceForID(m, rr->resrec.InterfaceID);
            if (intf && !intf->Advertise) { rr->resrec.InterfaceID = mDNSInterface_LocalOnly; rr->ARType = AuthRecordLocalOnly; }
        }
        if (rr->resrec.InterfaceID != previousID)
            LogInfo("mDNS_Register_internal: Diverting record to local-only %s", ARDisplayString(m, rr));
    }

    if (RRLocalOnly(rr))
    {
        if (CheckAuthSameRecord(&m->rrauth, rr))
        {
            LogMsg("mDNS_Register_internal: ERROR!! Tried to register LocalOnly AuthRecord %p %##s (%s) that's already in the list",
                   rr, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
            return(mStatus_AlreadyRegistered);
        }
    }
    else
    {
        while (*p && *p != rr) p=&(*p)->next;
        if (*p)
        {
            LogMsg("mDNS_Register_internal: ERROR!! Tried to register AuthRecord %p %##s (%s) that's already in the list",
                   rr, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
            return(mStatus_AlreadyRegistered);
        }
    }

    while (*d && *d != rr) d=&(*d)->next;
    if (*d)
    {
        LogMsg("mDNS_Register_internal: ERROR!! Tried to register AuthRecord %p %##s (%s) that's already in the Duplicate list",
               rr, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
        return(mStatus_AlreadyRegistered);
    }

    if (rr->DependentOn)
    {
        if (rr->resrec.RecordType == kDNSRecordTypeUnique)
            rr->resrec.RecordType =  kDNSRecordTypeVerified;
        else
        {
            LogMsg("mDNS_Register_internal: ERROR! %##s (%s): rr->DependentOn && RecordType != kDNSRecordTypeUnique",
                   rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
            return(mStatus_Invalid);
        }
        if (!(rr->DependentOn->resrec.RecordType & (kDNSRecordTypeUnique | kDNSRecordTypeVerified | kDNSRecordTypeKnownUnique)))
        {
            LogMsg("mDNS_Register_internal: ERROR! %##s (%s): rr->DependentOn->RecordType bad type %X",
                   rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), rr->DependentOn->resrec.RecordType);
            return(mStatus_Invalid);
        }
    }

    // If this resource record is referencing a specific interface, make sure it exists.
    // Skip checks for LocalOnly and P2P as they are not valid InterfaceIDs. Also, for scoped
    // entries in /etc/hosts skip that check as that interface may not be valid at this time.
    if (rr->resrec.InterfaceID && rr->ARType != AuthRecordLocalOnly && rr->ARType != AuthRecordP2P)
    {
        NetworkInterfaceInfo *intf = FirstInterfaceForID(m, rr->resrec.InterfaceID);
        if (!intf)
        {
            debugf("mDNS_Register_internal: Bogus InterfaceID %p in resource record", rr->resrec.InterfaceID);
            return(mStatus_BadReferenceErr);
        }
    }

    rr->next = mDNSNULL;

    // Field Group 1: The actual information pertaining to this resource record
    // Set up by client prior to call

    // Field Group 2: Persistent metadata for Authoritative Records
//	rr->Additional1       = set to mDNSNULL  in mDNS_SetupResourceRecord; may be overridden by client
//	rr->Additional2       = set to mDNSNULL  in mDNS_SetupResourceRecord; may be overridden by client
//	rr->DependentOn       = set to mDNSNULL  in mDNS_SetupResourceRecord; may be overridden by client
//	rr->RRSet             = set to mDNSNULL  in mDNS_SetupResourceRecord; may be overridden by client
//	rr->Callback          = already set      in mDNS_SetupResourceRecord
//	rr->Context           = already set      in mDNS_SetupResourceRecord
//	rr->RecordType        = already set      in mDNS_SetupResourceRecord
//	rr->HostTarget        = set to mDNSfalse in mDNS_SetupResourceRecord; may be overridden by client
//	rr->AllowRemoteQuery  = set to mDNSfalse in mDNS_SetupResourceRecord; may be overridden by client
    // Make sure target is not uninitialized data, or we may crash writing debugging log messages
    if (rr->AutoTarget && target) target->c[0] = 0;

    // Field Group 3: Transient state for Authoritative Records
    rr->Acknowledged      = mDNSfalse;
    rr->ProbeCount        = DefaultProbeCountForRecordType(rr->resrec.RecordType);
    rr->ProbeRestartCount = 0;
    rr->AnnounceCount     = InitialAnnounceCount;
    rr->RequireGoodbye    = mDNSfalse;
    rr->AnsweredLocalQ    = mDNSfalse;
    rr->IncludeInProbe    = mDNSfalse;
    rr->ImmedUnicast      = mDNSfalse;
    rr->SendNSECNow       = mDNSNULL;
    rr->ImmedAnswer       = mDNSNULL;
    rr->ImmedAdditional   = mDNSNULL;
    rr->SendRNow          = mDNSNULL;
    rr->v4Requester       = zerov4Addr;
    rr->v6Requester       = zerov6Addr;
    rr->NextResponse      = mDNSNULL;
    rr->NR_AnswerTo       = mDNSNULL;
    rr->NR_AdditionalTo   = mDNSNULL;
    if (!rr->AutoTarget) InitializeLastAPTime(m, rr);
//	rr->LastAPTime        = Set for us in InitializeLastAPTime()
//	rr->LastMCTime        = Set for us in InitializeLastAPTime()
//	rr->LastMCInterface   = Set for us in InitializeLastAPTime()
    rr->NewRData          = mDNSNULL;
    rr->newrdlength       = 0;
    rr->UpdateCallback    = mDNSNULL;
    rr->UpdateCredits     = kMaxUpdateCredits;
    rr->NextUpdateCredit  = 0;
    rr->UpdateBlocked     = 0;

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    // For records we're holding as proxy (except reverse-mapping PTR records) two announcements is sufficient
    if (rr->WakeUp.HMAC.l[0] && !rr->AddressProxy.type) rr->AnnounceCount = 2;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

    // Field Group 4: Transient uDNS state for Authoritative Records
    rr->state             = regState_Zero;
    rr->uselease          = 0;
    rr->expire            = 0;
    rr->Private           = 0;
    rr->updateid          = zeroID;
    rr->updateIntID       = zeroOpaque64;
    rr->zone              = rr->resrec.name;
    rr->nta               = mDNSNULL;
    rr->tcp               = mDNSNULL;
    rr->OrigRData         = 0;
    rr->OrigRDLen         = 0;
    rr->InFlightRData     = 0;
    rr->InFlightRDLen     = 0;
    rr->QueuedRData       = 0;
    rr->QueuedRDLen       = 0;
    //mDNSPlatformMemZero(&rr->NATinfo, sizeof(rr->NATinfo));
    // We should be recording the actual internal port for this service record here. Once we initiate our NAT mapping
    // request we'll subsequently overwrite srv.port with the allocated external NAT port -- potentially multiple
    // times with different values if the external NAT port changes during the lifetime of the service registration.
    //if (rr->resrec.rrtype == kDNSType_SRV) rr->NATinfo.IntPort = rr->resrec.rdata->u.srv.port;

//	rr->resrec.interface         = already set in mDNS_SetupResourceRecord
//	rr->resrec.name->c           = MUST be set by client
//	rr->resrec.rrtype            = already set in mDNS_SetupResourceRecord
//	rr->resrec.rrclass           = already set in mDNS_SetupResourceRecord
//	rr->resrec.rroriginalttl     = already set in mDNS_SetupResourceRecord
//	rr->resrec.rdata             = MUST be set by client, unless record type is CNAME or PTR and rr->HostTarget is set

    // BIND named (name daemon) doesn't allow TXT records with zero-length rdata. This is strictly speaking correct,
    // since RFC 1035 specifies a TXT record as "One or more <character-string>s", not "Zero or more <character-string>s".
    // Since some legacy apps try to create zero-length TXT records, we'll silently correct it here.
    if (rr->resrec.rrtype == kDNSType_TXT && rr->resrec.rdlength == 0) { rr->resrec.rdlength = 1; rr->resrec.rdata->u.txt.c[0] = 0; }

    if (rr->AutoTarget)
    {
        SetTargetToHostName(m, rr); // Also sets rdlength and rdestimate for us, and calls InitializeLastAPTime();
#ifndef UNICAST_DISABLED
        // If we have no target record yet, SetTargetToHostName will set rr->state == regState_NoTarget
        // In this case we leave the record half-formed in the list, and later we'll remove it from the list and re-add it properly.
        if (rr->state == regState_NoTarget)
        {
            // Initialize the target so that we don't crash while logging etc.
            domainname *tar = GetRRDomainNameTarget(&rr->resrec);
            if (tar) tar->c[0] = 0;
            LogInfo("mDNS_Register_internal: record %s in NoTarget state", ARDisplayString(m, rr));
        }
#endif
    }
    else
    {
        rr->resrec.rdlength   = GetRDLength(&rr->resrec, mDNSfalse);
        rr->resrec.rdestimate = GetRDLength(&rr->resrec, mDNStrue);
    }

    if (!ValidateDomainName(rr->resrec.name))
    { LogMsg("Attempt to register record with invalid name: %s", ARDisplayString(m, rr)); return(mStatus_Invalid); }

    // Don't do this until *after* we've set rr->resrec.rdlength
    if (!ValidateRData(rr->resrec.rrtype, rr->resrec.rdlength, rr->resrec.rdata))
    { LogMsg("Attempt to register record with invalid rdata: %s", ARDisplayString(m, rr)); return(mStatus_Invalid); }

    rr->resrec.namehash   = DomainNameHashValue(rr->resrec.name);
    rr->resrec.rdatahash  = target ? DomainNameHashValue(target) : RDataHashValue(&rr->resrec);

    if (RRLocalOnly(rr))
    {
        // If this is supposed to be unique, make sure we don't have any name conflicts.
        // If we found a conflict, we may still want to insert the record in the list but mark it appropriately
        // (kDNSRecordTypeDeregistering) so that we deliver RMV events to the application. But this causes more
        // complications and not clear whether there are any benefits. See rdar:9304275 for details.
        // Hence, just bail out.
        if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
        {
            if (CheckAuthRecordConflict(&m->rrauth, rr))
            {
                LogInfo("mDNS_Register_internal: Name conflict %s (%p), InterfaceID %p", ARDisplayString(m, rr), rr, rr->resrec.InterfaceID);
                return mStatus_NameConflict;
            }
        }
    }

    // For uDNS records, we don't support duplicate checks at this time.
#ifndef UNICAST_DISABLED
    if (AuthRecord_uDNS(rr))
    {
        if (!m->NewLocalRecords) m->NewLocalRecords = rr;
        // When we called SetTargetToHostName, it may have caused mDNS_Register_internal to be re-entered, appending new
        // records to the list, so we now need to update p to advance to the new end to the list before appending our new record.
        // Note that for AutoTunnel this should never happen, but this check makes the code future-proof.
        while (*p) p=&(*p)->next;
        *p = rr;
        if (rr->resrec.RecordType == kDNSRecordTypeUnique) rr->resrec.RecordType = kDNSRecordTypeVerified;
        rr->ProbeCount    = 0;
        rr->ProbeRestartCount = 0;
        rr->AnnounceCount = 0;
        if (rr->state != regState_NoTarget) ActivateUnicastRegistration(m, rr);
        return(mStatus_NoError);            // <--- Note: For unicast records, code currently bails out at this point
    }
#endif

    // Now that we've finished building our new record, make sure it's not identical to one we already have
    if (RRLocalOnly(rr))
    {
        rr->ProbeCount    = 0;
        rr->ProbeRestartCount = 0;
        rr->AnnounceCount = 0;
        r = CheckAuthIdenticalRecord(&m->rrauth, rr);
    }
    else
    {
        for (r = m->ResourceRecords; r; r=r->next)
            if (RecordIsLocalDuplicate(r, rr))
            {
                if (r->resrec.RecordType == kDNSRecordTypeDeregistering) r->AnnounceCount = 0;
                else break;
            }
    }

    if (r)
    {
        debugf("mDNS_Register_internal:Adding to duplicate list %s", ARDisplayString(m,rr));
        *d = rr;
        // If the previous copy of this record is already verified unique,
        // then indicate that we should move this record promptly to kDNSRecordTypeUnique state.
        // Setting ProbeCount to zero will cause SendQueries() to advance this record to
        // kDNSRecordTypeVerified state and call the client callback at the next appropriate time.
        if (rr->resrec.RecordType == kDNSRecordTypeUnique && r->resrec.RecordType == kDNSRecordTypeVerified)
            rr->ProbeCount = 0;
    }
    else
    {
        debugf("mDNS_Register_internal: Adding to active record list %s", ARDisplayString(m,rr));
        if (RRLocalOnly(rr))
        {
            AuthGroup *ag;
            ag = InsertAuthRecord(m, &m->rrauth, rr);
            if (ag && !ag->NewLocalOnlyRecords) {
                m->NewLocalOnlyRecords = mDNStrue;
                ag->NewLocalOnlyRecords = rr;
            }
            // No probing for LocalOnly records, Acknowledge them right away
            if (rr->resrec.RecordType == kDNSRecordTypeUnique) rr->resrec.RecordType = kDNSRecordTypeVerified;
            AcknowledgeRecord(m, rr);
            return(mStatus_NoError);
        }
        else
        {
            if (!m->NewLocalRecords) m->NewLocalRecords = rr;
            *p = rr;
        }
    }

    // If this is a keepalive record, fetch the MAC address of the remote host.
    // This is used by the in-NIC proxy to send the keepalive packets.
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (mDNS_KeepaliveRecord(&rr->resrec))
    {
        // Set the record type to known unique to prevent probing keep alive records.
        // Also make sure we do not announce the keepalive records.
       rr->resrec.RecordType = kDNSRecordTypeKnownUnique;
       rr->AnnounceCount     = 0;
       mDNSAddr   raddr;
       getKeepaliveRaddr(m, rr, &raddr);
       // This is an asynchronous call. Once the remote MAC address is available, helper will schedule an
       // asynchronous task to update the resource record
       mDNSPlatformGetRemoteMacAddr(m, &raddr);
    }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (!AuthRecord_uDNS(rr))   // This check is superfluous, given that for unicast records we (currently) bail out above
    {
        // We have inserted the record in the list. See if we have to advertise the A/AAAA,HINFO,PTR records.
        IncrementAutoTargetServices(m, rr);
        // For records that are not going to probe, acknowledge them right away
        if (rr->resrec.RecordType != kDNSRecordTypeUnique && rr->resrec.RecordType != kDNSRecordTypeDeregistering)
            AcknowledgeRecord(m, rr);

        // Adding a record may affect whether or not we should sleep
        mDNS_UpdateAllowSleep(m);
    }
    mdnslogInfo("mDNS_Register_internal Done:%s type=%d ", rr->namestorage.c, rr->resrec.rrtype);
    return(mStatus_NoError);
}

mDNSlocal void RecordProbeFailure(mDNS *const m, const AuthRecord *const rr)
{
    m->ProbeFailTime = m->timenow;
    m->NumFailedProbes++;
    // If we've had fifteen or more probe failures, rate-limit to one every five seconds.
    // If a bunch of hosts have all been configured with the same name, then they'll all
    // conflict and run through the same series of names: name-2, name-3, name-4, etc.,
    // up to name-10. After that they'll start adding random increments in the range 1-100,
    // so they're more likely to branch out in the available namespace and settle on a set of
    // unique names quickly. If after five more tries the host is still conflicting, then we
    // may have a serious problem, so we start rate-limiting so we don't melt down the network.
    if (m->NumFailedProbes >= 15)
    {
        m->SuppressProbes = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 5);
        LogMsg("Excessive name conflicts (%lu) for %##s (%s); rate limiting in effect",
               m->NumFailedProbes, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
    }
}

mDNSlocal void CompleteRDataUpdate(mDNS *const m, AuthRecord *const rr)
{
    RData *OldRData = rr->resrec.rdata;
    mDNSu16 OldRDLen = rr->resrec.rdlength;
    SetNewRData(&rr->resrec, rr->NewRData, rr->newrdlength);    // Update our rdata
    rr->NewRData = mDNSNULL;                                    // Clear the NewRData pointer ...
    if (rr->UpdateCallback)
        rr->UpdateCallback(m, rr, OldRData, OldRDLen);          // ... and let the client know
}

// Note: mDNS_Deregister_internal can call a user callback, which may change the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
// Exported so uDNS.c can call this
mDNSexport mStatus mDNS_Deregister_internal(mDNS *const m, AuthRecord *const rr, mDNS_Dereg_type drt)
{
    AuthRecord *r2;
    mDNSu8 RecordType = rr->resrec.RecordType;
    AuthRecord **p = &m->ResourceRecords;   // Find this record in our list of active records
    mDNSBool dupList = mDNSfalse;

    if (RRLocalOnly(rr))
    {
        AuthGroup *a;
        AuthGroup **ag = &a;
        AuthRecord **rp;
        const mDNSu32 slot = AuthHashSlot(rr->resrec.name);

        a = AuthGroupForRecord(&m->rrauth, slot, &rr->resrec);
        if (!a) return mDNSfalse;
        rp = &(*ag)->members;
        while (*rp && *rp != rr) rp=&(*rp)->next;
        p = rp;
    }
    else
    {
        while (*p && *p != rr) p=&(*p)->next;
    }

    if (*p)
    {
        // We found our record on the main list. See if there are any duplicates that need special handling.
        if (drt == mDNS_Dereg_conflict)     // If this was a conflict, see that all duplicates get the same treatment
        {
            // Scan for duplicates of rr, and mark them for deregistration at the end of this routine, after we've finished
            // deregistering rr. We need to do this scan *before* we give the client the chance to free and reuse the rr memory.
            for (r2 = m->DuplicateRecords; r2; r2=r2->next) if (RecordIsLocalDuplicate(r2, rr)) r2->ProbeCount = 0xFF;
        }
        else
        {
            // Before we delete the record (and potentially send a goodbye packet)
            // first see if we have a record on the duplicate list ready to take over from it.
            AuthRecord **d = &m->DuplicateRecords;
            while (*d && !RecordIsLocalDuplicate(*d, rr)) d=&(*d)->next;
            if (*d)
            {
                AuthRecord *dup = *d;
                debugf("mDNS_Register_internal: Duplicate record %p taking over from %p %##s (%s)",
                       dup, rr, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
                *d        = dup->next;      // Cut replacement record from DuplicateRecords list
                if (RRLocalOnly(rr))
                {
                    dup->next = mDNSNULL;
                    if (!InsertAuthRecord(m, &m->rrauth, dup)) LogMsg("mDNS_Deregister_internal: ERROR!! cannot insert %s", ARDisplayString(m, dup));
                }
                else
                {
                    dup->next = rr->next;       // And then...
                    rr->next  = dup;            // ... splice it in right after the record we're about to delete
                }
                dup->resrec.RecordType        = rr->resrec.RecordType;
                dup->ProbeCount      = rr->ProbeCount;
                dup->ProbeRestartCount = rr->ProbeRestartCount;
                dup->AnnounceCount   = rr->AnnounceCount;
                dup->RequireGoodbye  = rr->RequireGoodbye;
                dup->AnsweredLocalQ  = rr->AnsweredLocalQ;
                dup->ImmedAnswer     = rr->ImmedAnswer;
                dup->ImmedUnicast    = rr->ImmedUnicast;
                dup->ImmedAdditional = rr->ImmedAdditional;
                dup->v4Requester     = rr->v4Requester;
                dup->v6Requester     = rr->v6Requester;
                dup->ThisAPInterval  = rr->ThisAPInterval;
                dup->LastAPTime      = rr->LastAPTime;
                dup->LastMCTime      = rr->LastMCTime;
                dup->LastMCInterface = rr->LastMCInterface;
                dup->Private         = rr->Private;
                dup->state           = rr->state;
                rr->RequireGoodbye = mDNSfalse;
                rr->AnsweredLocalQ = mDNSfalse;
            }
        }
    }
    else
    {
        // We didn't find our record on the main list; try the DuplicateRecords list instead.
        p = &m->DuplicateRecords;
        while (*p && *p != rr) p=&(*p)->next;
        // If we found our record on the duplicate list, then make sure we don't send a goodbye for it
        if (*p)
        {
            // Duplicate records are not used for sending wakeups or goodbyes. Hence, deregister them
            // immediately. When there is a conflict, we deregister all the conflicting duplicate records
            // also that have been marked above in this function. In that case, we come here and if we don't
            // deregister (unilink from the DuplicateRecords list), we will be recursing infinitely. Hence,
            // clear the HMAC which will cause it to deregister. See <rdar://problem/10380988> for
            // details.
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            rr->WakeUp.HMAC    = zeroEthAddr;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            rr->RequireGoodbye = mDNSfalse;
            rr->resrec.RecordType = kDNSRecordTypeDeregistering;
            dupList = mDNStrue;
        }
        if (*p) debugf("mDNS_Deregister_internal: Deleting DuplicateRecord %p %##s (%s)",
                       rr, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
    }

    if (!*p)
    {
        // No need to log an error message if we already know this is a potentially repeated deregistration
        if (drt != mDNS_Dereg_repeat)
            LogMsg("mDNS_Deregister_internal: Record %p not found in list %s", rr, ARDisplayString(m,rr));
        return(mStatus_BadReferenceErr);
    }

    // If this is a shared record and we've announced it at least once,
    // we need to retract that announcement before we delete the record

    // If this is a record (including mDNSInterface_LocalOnly records) for which we've given local-only answers then
    // it's tempting to just do "AnswerAllLocalQuestionsWithLocalAuthRecord(m, rr, mDNSfalse)" here, but that would not not be safe.
    // The AnswerAllLocalQuestionsWithLocalAuthRecord routine walks the question list invoking client callbacks, using the "m->CurrentQuestion"
    // mechanism to cope with the client callback modifying the question list while that's happening.
    // However, mDNS_Deregister could have been called from a client callback (e.g. from the domain enumeration callback FoundDomain)
    // which means that the "m->CurrentQuestion" mechanism is already in use to protect that list, so we can't use it twice.
    // More generally, if we invoke callbacks from within a client callback, then those callbacks could deregister other
    // records, thereby invoking yet more callbacks, without limit.
    // The solution is to defer delivering the "Remove" events until mDNS_Execute time, just like we do for sending
    // actual goodbye packets.

#ifndef UNICAST_DISABLED
    if (AuthRecord_uDNS(rr))
    {
        if (rr->RequireGoodbye)
        {
            if (rr->tcp) { DisposeTCPConn(rr->tcp); rr->tcp = mDNSNULL; }
            rr->resrec.RecordType    = kDNSRecordTypeDeregistering;
            m->LocalRemoveEvents     = mDNStrue;
            uDNS_DeregisterRecord(m, rr);
            // At this point unconditionally we bail out
            // Either uDNS_DeregisterRecord will have completed synchronously, and called CompleteDeregistration,
            // which calls us back here with RequireGoodbye set to false, or it will have initiated the deregistration
            // process and will complete asynchronously. Either way we don't need to do anything more here.
            return(mStatus_NoError);
        }
        // Sometimes the records don't complete proper deregistration i.e., don't wait for a response
        // from the server. In that case, if the records have been part of a group update, clear the
        // state here. Some recors e.g., AutoTunnel gets reused without ever being completely initialized
        rr->updateid = zeroID;

        // We defer cleaning up NAT state only after sending goodbyes. This is important because
        // RecordRegistrationGotZoneData guards against creating NAT state if clientContext is non-NULL.
        // This happens today when we turn on/off interface where we get multiple network transitions
        // and RestartRecordGetZoneData triggers re-registration of the resource records even though
        // they may be in Registered state which causes NAT information to be setup multiple times. Defering
        // the cleanup here keeps clientContext non-NULL and hence prevents that. Note that cleaning up
        // NAT state here takes care of the case where we did not send goodbyes at all.
        if (rr->NATinfo.clientContext)
        {
            mDNS_StopNATOperation_internal(m, &rr->NATinfo);
            rr->NATinfo.clientContext = mDNSNULL;
        }
        if (rr->nta) { CancelGetZoneData(m, rr->nta); rr->nta = mDNSNULL; }
        if (rr->tcp) { DisposeTCPConn(rr->tcp);       rr->tcp = mDNSNULL; }
    }
#endif // UNICAST_DISABLED

    if      (RecordType == kDNSRecordTypeUnregistered)
        LogMsg("mDNS_Deregister_internal: %s already marked kDNSRecordTypeUnregistered", ARDisplayString(m, rr));
    else if (RecordType == kDNSRecordTypeDeregistering)
    {
        LogMsg("mDNS_Deregister_internal: %s already marked kDNSRecordTypeDeregistering", ARDisplayString(m, rr));
        return(mStatus_BadReferenceErr);
    }

    // <rdar://problem/7457925> Local-only questions don't get remove events for unique records
    // We may want to consider changing this code so that we generate local-only question "rmv"
    // events (and maybe goodbye packets too) for unique records as well as for shared records
    // Note: If we change the logic for this "if" statement, need to ensure that the code in
    // CompleteDeregistration() sets the appropriate state variables to gaurantee that "else"
    // clause will execute here and the record will be cut from the list.
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (rr->WakeUp.HMAC.l[0] ||
        (RecordType == kDNSRecordTypeShared && (rr->RequireGoodbye || rr->AnsweredLocalQ)))
#else
    if (RecordType == kDNSRecordTypeShared && (rr->RequireGoodbye || rr->AnsweredLocalQ))
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    {
        verbosedebugf("mDNS_Deregister_internal: Starting deregistration for %s", ARDisplayString(m, rr));
        rr->resrec.RecordType    = kDNSRecordTypeDeregistering;
        rr->resrec.rroriginalttl = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        rr->AnnounceCount        = rr->WakeUp.HMAC.l[0] ? WakeupCount : (drt == mDNS_Dereg_rapid) ? 1 : GoodbyeCount;
#else
        rr->AnnounceCount        = (drt == mDNS_Dereg_rapid) ? 1 : GoodbyeCount;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        rr->ThisAPInterval       = mDNSPlatformOneSecond * 2;
        rr->LastAPTime           = m->timenow - rr->ThisAPInterval;
        m->LocalRemoveEvents     = mDNStrue;
        if (m->NextScheduledResponse - (m->timenow + mDNSPlatformOneSecond/10) >= 0)
            m->NextScheduledResponse = (m->timenow + mDNSPlatformOneSecond/10);
    }
    else
    {
        if (!dupList && RRLocalOnly(rr))
        {
            AuthGroup *ag = RemoveAuthRecord(m, &m->rrauth, rr);
            if (ag->NewLocalOnlyRecords == rr) ag->NewLocalOnlyRecords = rr->next;
        }
        else
        {
            *p = rr->next;                  // Cut this record from the list
            if (m->NewLocalRecords == rr) m->NewLocalRecords = rr->next;
            DecrementAutoTargetServices(m, rr);
        }
        // If someone is about to look at this, bump the pointer forward
        if (m->CurrentRecord   == rr) m->CurrentRecord   = rr->next;
        rr->next = mDNSNULL;

        // Should we generate local remove events here?
        // i.e. something like:
        // if (rr->AnsweredLocalQ) { AnswerAllLocalQuestionsWithLocalAuthRecord(m, rr, mDNSfalse); rr->AnsweredLocalQ = mDNSfalse; }

        verbosedebugf("mDNS_Deregister_internal: Deleting record for %s", ARDisplayString(m, rr));
        rr->resrec.RecordType = kDNSRecordTypeUnregistered;

        if ((drt == mDNS_Dereg_conflict || drt == mDNS_Dereg_repeat) && RecordType == kDNSRecordTypeShared)
            debugf("mDNS_Deregister_internal: Cannot have a conflict on a shared record! %##s (%s)",
                   rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));

        // If we have an update queued up which never executed, give the client a chance to free that memory
        if (rr->NewRData) CompleteRDataUpdate(m, rr);   // Update our rdata, clear the NewRData pointer, and return memory to the client


        // CAUTION: MUST NOT do anything more with rr after calling rr->Callback(), because the client's callback function
        // is allowed to do anything, including starting/stopping queries, registering/deregistering records, etc.
        // In this case the likely client action to the mStatus_MemFree message is to free the memory,
        // so any attempt to touch rr after this is likely to lead to a crash.
        if (drt != mDNS_Dereg_conflict)
        {
            mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
            LogInfo("mDNS_Deregister_internal: mStatus_MemFree for %s", ARDisplayString(m, rr));
            if (rr->RecordCallback)
                rr->RecordCallback(m, rr, mStatus_MemFree);         // MUST NOT touch rr after this
            mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
        }
        else
        {
            RecordProbeFailure(m, rr);
            mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
            if (rr->RecordCallback)
                rr->RecordCallback(m, rr, mStatus_NameConflict);    // MUST NOT touch rr after this
            mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
            // Now that we've finished deregistering rr, check our DuplicateRecords list for any that we marked previously.
            // Note that with all the client callbacks going on, by the time we get here all the
            // records we marked may have been explicitly deregistered by the client anyway.
            r2 = m->DuplicateRecords;
            while (r2)
            {
                if (r2->ProbeCount != 0xFF)
                {
                    r2 = r2->next;
                }
                else
                {
                    mDNS_Deregister_internal(m, r2, mDNS_Dereg_conflict);
                    // As this is a duplicate record, it will be unlinked from the list
                    // immediately
                    r2 = m->DuplicateRecords;
                }
            }
        }
    }
    mDNS_UpdateAllowSleep(m);
    return(mStatus_NoError);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Packet Sending Functions
#endif

mDNSlocal void AddRecordToResponseList(AuthRecord ***nrpp, AuthRecord *rr, AuthRecord *add)
{
    if (rr->NextResponse == mDNSNULL && *nrpp != &rr->NextResponse)
    {
        **nrpp = rr;
        // NR_AdditionalTo must point to a record with NR_AnswerTo set (and not NR_AdditionalTo)
        // If 'add' does not meet this requirement, then follow its NR_AdditionalTo pointer to a record that does
        // The referenced record will definitely be acceptable (by recursive application of this rule)
        if (add && add->NR_AdditionalTo) add = add->NR_AdditionalTo;
        rr->NR_AdditionalTo = add;
        *nrpp = &rr->NextResponse;
    }
    debugf("AddRecordToResponseList: %##s (%s) already in list", rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
}

mDNSlocal void AddAdditionalsToResponseList(mDNS *const m,
                                            AuthRecord *ResponseRecords,
                                            AuthRecord ***nrpp,
                                            const mDNSInterfaceID InterfaceID)
{
    AuthRecord  *rr, *rr2;
    
    debugf("AddAdditionalsToResponseList");
    
    for (rr=ResponseRecords; rr; rr=rr->NextResponse)           // For each record we plan to put
    {
        // (Note: This is an "if", not a "while". If we add a record, we'll find it again
        // later in the "for" loop, and we will follow further "additional" links then.)
        if (rr->Additional1 && ResourceRecordIsValidInterfaceAnswer(rr->Additional1, InterfaceID))
            AddRecordToResponseList(nrpp, rr->Additional1, rr);

        if (rr->Additional2 && ResourceRecordIsValidInterfaceAnswer(rr->Additional2, InterfaceID))
            AddRecordToResponseList(nrpp, rr->Additional2, rr);

        // For SRV records, automatically add the Address record(s) for the target host
        if (rr->resrec.rrtype == kDNSType_SRV)
        {
            debugf("AddAdditionalsToResponseList: rr->resrec.rrtype == kDNSType_SRV, add the Address record");
            
            for (rr2=m->ResourceRecords; rr2; rr2=rr2->next) {                  // Scan list of resource records
            
                domainname *target = GetRRDomainNameTarget(&rr->resrec);

                if (RRTypeIsAddressType(rr2->resrec.rrtype)) {

                    if (ResourceRecordIsValidInterfaceAnswer(rr2, InterfaceID)) {
                        debugf("rrtype %s", rr2->resrec.rrtype == kDNSType_A ? "ipv4" : "ipv6");

                        debugf("rr");
                        debugf("   resrec.InterfaceID %d",  rr2->resrec.InterfaceID);
                        debugf("   InterfaceID        %d",  InterfaceID);
                        debugf("   resrec.rdatahash   %d",  rr->resrec.rdatahash);
                        static char CString[MAX_ESCAPED_DOMAIN_NAME];
                        if (target) {
                        ConvertDomainNameToCString(target, CString);
                        debugf("   domainname         %s",  CString);
                        }

                        debugf("rr2");
                        ConvertDomainNameToCString(rr2->resrec.name, CString);
                        debugf("   resrec.name        %s",  CString);
                        debugf("   resrec.namehash    %d",  rr2->resrec.namehash);
                    }
                }

                if (RRTypeIsAddressType(rr2->resrec.rrtype) &&                  // For all address records (A/AAAA) ...
                    ResourceRecordIsValidInterfaceAnswer(rr2, InterfaceID) &&   // ... which are valid for answer ...
                    rr->resrec.rdatahash == rr2->resrec.namehash &&             // ... whose name is the name of the SRV target
                    SameDomainName(&rr->resrec.rdata->u.srv.target, rr2->resrec.name))
				{
                    debugf("call AddRecordToResponseList %d", __LINE__);
                    AddRecordToResponseList(nrpp, rr2, rr);
                }
            }
        }
        else if (RRTypeIsAddressType(rr->resrec.rrtype))    // For A or AAAA, put counterpart as additional
        {
            debugf("RRTypeIsAddressType(rr->resrec.rrtype) %d", RRTypeIsAddressType(rr->resrec.rrtype));
            for (rr2=m->ResourceRecords; rr2; rr2=rr2->next)                    // Scan list of resource records
                if (RRTypeIsAddressType(rr2->resrec.rrtype) &&                  // For all address records (A/AAAA) ...
                    ResourceRecordIsValidInterfaceAnswer(rr2, InterfaceID) &&   // ... which are valid for answer ...
                    rr->resrec.namehash == rr2->resrec.namehash &&              // ... and have the same name
                    SameDomainName(rr->resrec.name, rr2->resrec.name))
                {
                    debugf("call AddRecordToResponseList %d", __LINE__);
                    AddRecordToResponseList(nrpp, rr2, rr);
                }
        }
        else if (rr->resrec.rrtype == kDNSType_PTR)         // For service PTR, see if we want to add DeviceInfo record
        {
            debugf("rr->resrec.rrtype == kDNSType_PTR %d", rr->resrec.rrtype == kDNSType_PTR);
            if (ResourceRecordIsValidInterfaceAnswer(&m->DeviceInfo, InterfaceID) &&
                SameDomainLabel(rr->resrec.rdata->u.name.c, m->DeviceInfo.resrec.name->c))
                AddRecordToResponseList(nrpp, &m->DeviceInfo, rr);
        }
    }
}

mDNSlocal int AnonInfoSpace(AnonymousInfo *info)
{
    ResourceRecord *rr = info->nsec3RR;

    // 2 bytes for compressed name + type (2) class (2) TTL (4) rdlength (2) rdata (n)
    return (2 + 10 + rr->rdlength);
}

mDNSlocal void SendDelayedUnicastResponse(mDNS *const m, const mDNSAddr *const dest, const mDNSInterfaceID InterfaceID)
{
    AuthRecord *rr;
    AuthRecord  *ResponseRecords = mDNSNULL;
    AuthRecord **nrp             = &ResponseRecords;
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, InterfaceID);
    int AnoninfoSpace = 0;

    // Make a list of all our records that need to be unicast to this destination
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        // If we find we can no longer unicast this answer, clear ImmedUnicast
        if (rr->ImmedAnswer == mDNSInterfaceMark               ||
            mDNSSameIPv4Address(rr->v4Requester, onesIPv4Addr) ||
            mDNSSameIPv6Address(rr->v6Requester, onesIPv6Addr)  )
            rr->ImmedUnicast = mDNSfalse;

        if (rr->ImmedUnicast && rr->ImmedAnswer == InterfaceID)
        {
            if ((dest->type == mDNSAddrType_IPv4 && mDNSSameIPv4Address(rr->v4Requester, dest->ip.v4)) ||
                (dest->type == mDNSAddrType_IPv6 && mDNSSameIPv6Address(rr->v6Requester, dest->ip.v6)))
            {
                rr->ImmedAnswer  = mDNSNULL;                // Clear the state fields
                rr->ImmedUnicast = mDNSfalse;
                rr->v4Requester  = zerov4Addr;
                rr->v6Requester  = zerov6Addr;

                // Only sent records registered for P2P over P2P interfaces
                if (intf && !mDNSPlatformValidRecordForInterface(rr, intf))
                {
                    LogInfo("SendDelayedUnicastResponse: Not sending %s, on %s", ARDisplayString(m, rr), InterfaceNameForID(m, InterfaceID));
                    continue;
                }

                if (rr->NextResponse == mDNSNULL && nrp != &rr->NextResponse)   // rr->NR_AnswerTo
                {
                    rr->NR_AnswerTo = NR_AnswerMulticast;
                    *nrp = rr;
                    nrp = &rr->NextResponse;
                }
            }
        }
    }

    debugf("%s %d call AddAdditionalsToResponseList", __func__, __LINE__);
    AddAdditionalsToResponseList(m, ResponseRecords, &nrp, InterfaceID);

    while (ResponseRecords)
    {
        mDNSu8 *responseptr = m->omsg.data;
        mDNSu8 *newptr;
        InitializeDNSMessage(&m->omsg.h, zeroID, ResponseFlags);

        // Put answers in the packet
        while (ResponseRecords && ResponseRecords->NR_AnswerTo)
        {
            rr = ResponseRecords;
            if (rr->resrec.AnonInfo)
            {
                AnoninfoSpace += AnonInfoSpace(rr->resrec.AnonInfo);
                rr->resrec.AnonInfo->SendNow = mDNSInterfaceMark;
            }
            if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
                rr->resrec.rrclass |= kDNSClass_UniqueRRSet;        // Temporarily set the cache flush bit so PutResourceRecord will set it

            // Retract the limit by AnoninfoSpace which we need to put the AnoInfo option.
            newptr = PutResourceRecordTTLWithLimit(&m->omsg, responseptr, &m->omsg.h.numAnswers, &rr->resrec, rr->resrec.rroriginalttl,
                 m->omsg.data + (AllowedRRSpace(&m->omsg) - AnoninfoSpace));

            rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;           // Make sure to clear cache flush bit back to normal state
            if (!newptr && m->omsg.h.numAnswers)
            {
                break; // If packet full, send it now
            }
            if (newptr) responseptr = newptr;
            ResponseRecords = rr->NextResponse;
            rr->NextResponse    = mDNSNULL;
            rr->NR_AnswerTo     = mDNSNULL;
            rr->NR_AdditionalTo = mDNSNULL;
            rr->RequireGoodbye  = mDNStrue;
        }

        // We have reserved the space for AnonInfo option. PutResourceRecord uses the
        // standard limit (AllowedRRSpace) and we should have space now.
        for (rr = m->ResourceRecords; rr; rr=rr->next)
        {
            if (rr->resrec.AnonInfo && rr->resrec.AnonInfo->SendNow == mDNSInterfaceMark)
            {
                ResourceRecord *nsec3RR = rr->resrec.AnonInfo->nsec3RR;

                newptr = PutResourceRecord(&m->omsg, responseptr, &m->omsg.h.numAuthorities, nsec3RR);
                if (newptr)
                {
                    responseptr = newptr;
                    debugf("SendDelayedUnicastResponse: Added NSEC3 Record %s on %p", RRDisplayString(m, nsec3RR), intf->InterfaceID);
                }
                else
                {
                    // We allocated space and we should not fail. Don't break, we need to clear the SendNow flag.
                    LogMsg("SendDelayedUnicastResponse: ERROR!! Cannot Add NSEC3 Record %s on %p", RRDisplayString(m, nsec3RR), intf->InterfaceID);
                }
                rr->resrec.AnonInfo->SendNow = mDNSNULL;
            }
        }

        // Add additionals, if there's space
        while (ResponseRecords && !ResponseRecords->NR_AnswerTo)
        {
            rr = ResponseRecords;
            if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
                rr->resrec.rrclass |= kDNSClass_UniqueRRSet;        // Temporarily set the cache flush bit so PutResourceRecord will set it
            newptr = PutResourceRecord(&m->omsg, responseptr, &m->omsg.h.numAdditionals, &rr->resrec);
            rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;           // Make sure to clear cache flush bit back to normal state

            if (newptr) responseptr = newptr;
            if (newptr && m->omsg.h.numAnswers) rr->RequireGoodbye = mDNStrue;
            else if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask) rr->ImmedAnswer = mDNSInterfaceMark;
            ResponseRecords = rr->NextResponse;
            rr->NextResponse    = mDNSNULL;
            rr->NR_AnswerTo     = mDNSNULL;
            rr->NR_AdditionalTo = mDNSNULL;
        }

        if (m->omsg.h.numAnswers)
            mDNSSendDNSMessage(m, &m->omsg, responseptr, InterfaceID, mDNSNULL, dest, MulticastDNSPort, mDNSNULL, mDNSNULL, mDNSfalse);
    }
}

// CompleteDeregistration guarantees that on exit the record will have been cut from the m->ResourceRecords list
// and the client's mStatus_MemFree callback will have been invoked
mDNSexport void CompleteDeregistration(mDNS *const m, AuthRecord *rr)
{
    LogInfo("CompleteDeregistration: called for Resource record %s", ARDisplayString(m, rr));
    // Clearing rr->RequireGoodbye signals mDNS_Deregister_internal() that
    // it should go ahead and immediately dispose of this registration
    rr->resrec.RecordType = kDNSRecordTypeShared;
    rr->RequireGoodbye    = mDNSfalse;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    rr->WakeUp.HMAC       = zeroEthAddr;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (rr->AnsweredLocalQ) { AnswerAllLocalQuestionsWithLocalAuthRecord(m, rr, mDNSfalse); rr->AnsweredLocalQ = mDNSfalse; }
    mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);     // Don't touch rr after this
}

// DiscardDeregistrations is used on shutdown and sleep to discard (forcibly and immediately)
// any deregistering records that remain in the m->ResourceRecords list.
// DiscardDeregistrations calls mDNS_Deregister_internal which can call a user callback,
// which may change the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSlocal void DiscardDeregistrations(mDNS *const m)
{
    if (m->CurrentRecord)
        LogMsg("DiscardDeregistrations ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = m->ResourceRecords;

    while (m->CurrentRecord)
    {
        AuthRecord *rr = m->CurrentRecord;
        if (!AuthRecord_uDNS(rr) && rr->resrec.RecordType == kDNSRecordTypeDeregistering)
            CompleteDeregistration(m, rr);      // Don't touch rr after this
        else
            m->CurrentRecord = rr->next;
    }
}

mDNSlocal mStatus GetLabelDecimalValue(const mDNSu8 *const src, mDNSu8 *dst)
{
    int i, val = 0;
    if (src[0] < 1 || src[0] > 3) return(mStatus_Invalid);
    for (i=1; i<=src[0]; i++)
    {
        if (src[i] < '0' || src[i] > '9') return(mStatus_Invalid);
        val = val * 10 + src[i] - '0';
    }
    if (val > 255) return(mStatus_Invalid);
    *dst = (mDNSu8)val;
    return(mStatus_NoError);
}

mDNSlocal mStatus GetIPv4FromName(mDNSAddr *const a, const domainname *const name)
{
    int skip = CountLabels(name) - 6;
    if (skip < 0) { LogMsg("GetIPFromName: Need six labels in IPv4 reverse mapping name %##s", name); return mStatus_Invalid; }
    if (GetLabelDecimalValue(SkipLeadingLabels(name, skip+3)->c, &a->ip.v4.b[0]) ||
        GetLabelDecimalValue(SkipLeadingLabels(name, skip+2)->c, &a->ip.v4.b[1]) ||
        GetLabelDecimalValue(SkipLeadingLabels(name, skip+1)->c, &a->ip.v4.b[2]) ||
        GetLabelDecimalValue(SkipLeadingLabels(name, skip+0)->c, &a->ip.v4.b[3])) return mStatus_Invalid;
    a->type = mDNSAddrType_IPv4;
    return(mStatus_NoError);
}

#define HexVal(X) ( ((X) >= '0' && (X) <= '9') ? ((X) - '0'     ) :   \
                    ((X) >= 'A' && (X) <= 'F') ? ((X) - 'A' + 10) :   \
                    ((X) >= 'a' && (X) <= 'f') ? ((X) - 'a' + 10) : -1)

mDNSlocal mStatus GetIPv6FromName(mDNSAddr *const a, const domainname *const name)
{
    int i, h, l;
    const domainname *n;

    int skip = CountLabels(name) - 34;
    if (skip < 0) { LogMsg("GetIPFromName: Need 34 labels in IPv6 reverse mapping name %##s", name); return mStatus_Invalid; }

    n = SkipLeadingLabels(name, skip);
    for (i=0; i<16; i++)
    {
        if (n->c[0] != 1) return mStatus_Invalid;
        l = HexVal(n->c[1]);
        n = (const domainname *)(n->c + 2);

        if (n->c[0] != 1) return mStatus_Invalid;
        h = HexVal(n->c[1]);
        n = (const domainname *)(n->c + 2);

        if (l<0 || h<0) return mStatus_Invalid;
        a->ip.v6.b[15-i] = (mDNSu8)((h << 4) | l);
    }

    a->type = mDNSAddrType_IPv6;
    return(mStatus_NoError);
}

mDNSlocal mDNSs32 ReverseMapDomainType(const domainname *const name)
{
    int skip = CountLabels(name) - 2;
    if (skip >= 0)
    {
        const domainname *suffix = SkipLeadingLabels(name, skip);
        if (SameDomainName(suffix, (const domainname*)"\x7" "in-addr" "\x4" "arpa")) return mDNSAddrType_IPv4;
        if (SameDomainName(suffix, (const domainname*)"\x3" "ip6"     "\x4" "arpa")) return mDNSAddrType_IPv6;
    }
    return(mDNSAddrType_None);
}

mDNSlocal void SendARP(mDNS *const m, const mDNSu8 op, const AuthRecord *const rr,
                       const mDNSv4Addr *const spa, const mDNSEthAddr *const tha, const mDNSv4Addr *const tpa, const mDNSEthAddr *const dst)
{
    int i;
    mDNSu8 *ptr = m->omsg.data;
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, rr->resrec.InterfaceID);
    if (!intf) { LogMsg("SendARP: No interface with InterfaceID %p found %s", rr->resrec.InterfaceID, ARDisplayString(m,rr)); return; }

    // 0x00 Destination address
    for (i=0; i<6; i++) *ptr++ = dst->b[i];

    // 0x06 Source address (Note: Since we don't currently set the BIOCSHDRCMPLT option, BPF will fill in the real interface address for us)
    for (i=0; i<6; i++) *ptr++ = intf->MAC.b[0];

    // 0x0C ARP Ethertype (0x0806)
    *ptr++ = 0x08; *ptr++ = 0x06;

    // 0x0E ARP header
    *ptr++ = 0x00; *ptr++ = 0x01;   // Hardware address space; Ethernet = 1
    *ptr++ = 0x08; *ptr++ = 0x00;   // Protocol address space; IP = 0x0800
    *ptr++ = 6;                     // Hardware address length
    *ptr++ = 4;                     // Protocol address length
    *ptr++ = 0x00; *ptr++ = op;     // opcode; Request = 1, Response = 2

    // 0x16 Sender hardware address (our MAC address)
    for (i=0; i<6; i++) *ptr++ = intf->MAC.b[i];

    // 0x1C Sender protocol address
    for (i=0; i<4; i++) *ptr++ = spa->b[i];

    // 0x20 Target hardware address
    for (i=0; i<6; i++) *ptr++ = tha->b[i];

    // 0x26 Target protocol address
    for (i=0; i<4; i++) *ptr++ = tpa->b[i];

    // 0x2A Total ARP Packet length 42 bytes
    mDNSPlatformSendRawPacket(m->omsg.data, ptr, rr->resrec.InterfaceID);
}

mDNSlocal mDNSu16 CheckSum(const void *const data, mDNSs32 length, mDNSu32 sum)
{
    const mDNSu16 *ptr = data;
    while (length > 0) { length -= 2; sum += *ptr++; }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum = (sum & 0xFFFF) + (sum >> 16);
    return(sum != 0xFFFF ? sum : 0);
}

mDNSlocal mDNSu16 IPv6CheckSum(const mDNSv6Addr *const src, const mDNSv6Addr *const dst, const mDNSu8 protocol, const void *const data, const mDNSu32 length)
{
    IPv6PseudoHeader ph;
    ph.src = *src;
    ph.dst = *dst;
    ph.len.b[0] = length >> 24;
    ph.len.b[1] = length >> 16;
    ph.len.b[2] = length >> 8;
    ph.len.b[3] = length;
    ph.pro.b[0] = 0;
    ph.pro.b[1] = 0;
    ph.pro.b[2] = 0;
    ph.pro.b[3] = protocol;
    return CheckSum(&ph, sizeof(ph), CheckSum(data, length, 0));
}

mDNSlocal void SendNDP(mDNS *const m, const mDNSu8 op, const mDNSu8 flags, const AuthRecord *const rr,
                       const mDNSv6Addr *const spa, const mDNSEthAddr *const tha, const mDNSv6Addr *const tpa, const mDNSEthAddr *const dst)
{
    int i;
    mDNSOpaque16 checksum;
    mDNSu8 *ptr = m->omsg.data;
    // Some recipient hosts seem to ignore Neighbor Solicitations if the IPv6-layer destination address is not the
    // appropriate IPv6 solicited node multicast address, so we use that IPv6-layer destination address, even though
    // at the Ethernet-layer we unicast the packet to the intended target, to avoid wasting network bandwidth.
    const mDNSv6Addr mc = { { 0xFF,0x02,0x00,0x00, 0,0,0,0, 0,0,0,1, 0xFF,tpa->b[0xD],tpa->b[0xE],tpa->b[0xF] } };
    const mDNSv6Addr *const v6dst = (op == NDP_Sol) ? &mc : tpa;
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, rr->resrec.InterfaceID);
    if (!intf) { LogMsg("SendNDP: No interface with InterfaceID %p found %s", rr->resrec.InterfaceID, ARDisplayString(m,rr)); return; }

    // 0x00 Destination address
    for (i=0; i<6; i++) *ptr++ = dst->b[i];
    // Right now we only send Neighbor Solicitations to verify whether the host we're proxying for has gone to sleep yet.
    // Since we know who we're looking for, we send it via Ethernet-layer unicast, rather than bothering every host on the
    // link with a pointless link-layer multicast.
    // Should we want to send traditional Neighbor Solicitations in the future, where we really don't know in advance what
    // Ethernet-layer address we're looking for, we'll need to send to the appropriate Ethernet-layer multicast address:
    // *ptr++ = 0x33;
    // *ptr++ = 0x33;
    // *ptr++ = 0xFF;
    // *ptr++ = tpa->b[0xD];
    // *ptr++ = tpa->b[0xE];
    // *ptr++ = tpa->b[0xF];

    // 0x06 Source address (Note: Since we don't currently set the BIOCSHDRCMPLT option, BPF will fill in the real interface address for us)
    for (i=0; i<6; i++) *ptr++ = (tha ? *tha : intf->MAC).b[i];

    // 0x0C IPv6 Ethertype (0x86DD)
    *ptr++ = 0x86; *ptr++ = 0xDD;

    // 0x0E IPv6 header
    *ptr++ = 0x60; *ptr++ = 0x00; *ptr++ = 0x00; *ptr++ = 0x00;     // Version, Traffic Class, Flow Label
    *ptr++ = 0x00; *ptr++ = 0x20;                                   // Length
    *ptr++ = 0x3A;                                                  // Protocol == ICMPv6
    *ptr++ = 0xFF;                                                  // Hop Limit

    // 0x16 Sender IPv6 address
    for (i=0; i<16; i++) *ptr++ = spa->b[i];

    // 0x26 Destination IPv6 address
    for (i=0; i<16; i++) *ptr++ = v6dst->b[i];

    // 0x36 NDP header
    *ptr++ = op;                    // 0x87 == Neighbor Solicitation, 0x88 == Neighbor Advertisement
    *ptr++ = 0x00;                  // Code
    *ptr++ = 0x00; *ptr++ = 0x00;   // Checksum placeholder (0x38, 0x39)
    *ptr++ = flags;
    *ptr++ = 0x00; *ptr++ = 0x00; *ptr++ = 0x00;

    if (op == NDP_Sol)  // Neighbor Solicitation. The NDP "target" is the address we seek.
    {
        // 0x3E NDP target.
        for (i=0; i<16; i++) *ptr++ = tpa->b[i];
        // 0x4E Source Link-layer Address
        // <http://www.ietf.org/rfc/rfc2461.txt>
        // MUST NOT be included when the source IP address is the unspecified address.
        // Otherwise, on link layers that have addresses this option MUST be included
        // in multicast solicitations and SHOULD be included in unicast solicitations.
        if (!mDNSIPv6AddressIsZero(*spa))
        {
            *ptr++ = NDP_SrcLL; // Option Type 1 == Source Link-layer Address
            *ptr++ = 0x01;      // Option length 1 (in units of 8 octets)
            for (i=0; i<6; i++) *ptr++ = (tha ? *tha : intf->MAC).b[i];
        }
    }
    else            // Neighbor Advertisement. The NDP "target" is the address we're giving information about.
    {
        // 0x3E NDP target.
        for (i=0; i<16; i++) *ptr++ = spa->b[i];
        // 0x4E Target Link-layer Address
        *ptr++ = NDP_TgtLL; // Option Type 2 == Target Link-layer Address
        *ptr++ = 0x01;      // Option length 1 (in units of 8 octets)
        for (i=0; i<6; i++) *ptr++ = (tha ? *tha : intf->MAC).b[i];
    }

    // 0x4E or 0x56 Total NDP Packet length 78 or 86 bytes
    m->omsg.data[0x13] = ptr - &m->omsg.data[0x36];     // Compute actual length
    checksum.NotAnInteger = ~IPv6CheckSum(spa, v6dst, 0x3A, &m->omsg.data[0x36], m->omsg.data[0x13]);
    m->omsg.data[0x38] = checksum.b[0];
    m->omsg.data[0x39] = checksum.b[1];

    mDNSPlatformSendRawPacket(m->omsg.data, ptr, rr->resrec.InterfaceID);
}
#ifndef REMOVE_MDNS_TRACING  
mDNSlocal void SetupTracerOpt(const mDNS *const m, rdataOPT *const Trace)
{
    mDNSu32 DNS_VERS = _DNS_SD_H;
    Trace->u.tracer.platf    = m->mDNS_plat;
    Trace->u.tracer.mDNSv    = DNS_VERS;

    Trace->opt              = kDNSOpt_Trace;
    Trace->optlen           = DNSOpt_TraceData_Space - 4;
}
#endif //#ifndef REMOVE_MDNS_TRACING  
mDNSlocal void SetupOwnerOpt(const mDNS *const m, const NetworkInterfaceInfo *const intf, rdataOPT *const owner)
{
    owner->u.owner.vers     = 0;
    owner->u.owner.seq      = m->SleepSeqNum;
    owner->u.owner.HMAC     = m->PrimaryMAC;
    owner->u.owner.IMAC     = intf->MAC;
    owner->u.owner.password = zeroEthAddr;

    // Don't try to compute the optlen until *after* we've set up the data fields
    // Right now the DNSOpt_Owner_Space macro does not depend on the owner->u.owner being set up correctly, but in the future it might
    owner->opt              = kDNSOpt_Owner;
    owner->optlen           = DNSOpt_Owner_Space(&m->PrimaryMAC, &intf->MAC) - 4;
}

mDNSlocal void GrantUpdateCredit(AuthRecord *rr)
{
    if (++rr->UpdateCredits >= kMaxUpdateCredits) rr->NextUpdateCredit = 0;
    else rr->NextUpdateCredit = NonZeroTime(rr->NextUpdateCredit + kUpdateCreditRefreshInterval);
}

mDNSlocal mDNSBool ShouldSendGoodbyesBeforeSleep(mDNS *const m, const NetworkInterfaceInfo *intf, AuthRecord *rr)
{
    // If there are no sleep proxies, we set the state to SleepState_Sleeping explicitly
    // and hence there is no need to check for Transfering state. But if we have sleep
    // proxies and partially sending goodbyes for some records, we will be in Transfering
    // state and hence need to make sure that we send goodbyes in that case too. Checking whether
    // we are not awake handles both cases.
    if ((rr->AuthFlags & AuthFlagsWakeOnly) && (m->SleepState != SleepState_Awake))
    {
        debugf("ShouldSendGoodbyesBeforeSleep: marking for goodbye", ARDisplayString(m, rr));
        return mDNStrue;
    }

    if (m->SleepState != SleepState_Sleeping)
        return mDNSfalse;

    // If we are going to sleep and in SleepState_Sleeping, SendGoodbyes on the interface tell you
    // whether you can send goodbyes or not.
    if (!intf->SendGoodbyes)
    {
        debugf("ShouldSendGoodbyesBeforeSleep: not sending goodbye %s, int %p", ARDisplayString(m, rr), intf->InterfaceID);
        return mDNSfalse;
    }
    else
    {
        debugf("ShouldSendGoodbyesBeforeSleep: sending goodbye %s, int %p", ARDisplayString(m, rr), intf->InterfaceID);
        return mDNStrue;
    }
}

// Note about acceleration of announcements to facilitate automatic coalescing of
// multiple independent threads of announcements into a single synchronized thread:
// The announcements in the packet may be at different stages of maturity;
// One-second interval, two-second interval, four-second interval, and so on.
// After we've put in all the announcements that are due, we then consider
// whether there are other nearly-due announcements that are worth accelerating.
// To be eligible for acceleration, a record MUST NOT be older (further along
// its timeline) than the most mature record we've already put in the packet.
// In other words, younger records can have their timelines accelerated to catch up
// with their elder bretheren; this narrows the age gap and helps them eventually get in sync.
// Older records cannot have their timelines accelerated; this would just widen
// the gap between them and their younger bretheren and get them even more out of sync.

// Note: SendResponses calls mDNS_Deregister_internal which can call a user callback, which may change
// the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSlocal void SendResponses(mDNS *const m)
{
    int pktcount = 0;
    AuthRecord *rr, *r2;
    mDNSs32 maxExistingAnnounceInterval = 0;
    const NetworkInterfaceInfo *intf = GetFirstActiveInterface(m->HostInterfaces);

    m->NextScheduledResponse = m->timenow + 0x78000000;

    if (m->SleepState == SleepState_Transferring) RetrySPSRegistrations(m);

    for (rr = m->ResourceRecords; rr; rr=rr->next)
        if (rr->ImmedUnicast)
        {
            mDNSAddr v4 = { mDNSAddrType_IPv4, {{{0}}} };
            mDNSAddr v6 = { mDNSAddrType_IPv6, {{{0}}} };
            v4.ip.v4 = rr->v4Requester;
            v6.ip.v6 = rr->v6Requester;
            if (!mDNSIPv4AddressIsZero(rr->v4Requester)) SendDelayedUnicastResponse(m, &v4, rr->ImmedAnswer);
            if (!mDNSIPv6AddressIsZero(rr->v6Requester)) SendDelayedUnicastResponse(m, &v6, rr->ImmedAnswer);
            if (rr->ImmedUnicast)
            {
                LogMsg("SendResponses: ERROR: rr->ImmedUnicast still set: %s", ARDisplayString(m, rr));
                rr->ImmedUnicast = mDNSfalse;
            }
        }

    // ***
    // *** 1. Setup: Set the SendRNow and ImmedAnswer fields to indicate which interface(s) the records need to be sent on
    // ***

    // Run through our list of records, and decide which ones we're going to announce on all interfaces
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        while (rr->NextUpdateCredit && m->timenow - rr->NextUpdateCredit >= 0) GrantUpdateCredit(rr);
        if (TimeToAnnounceThisRecord(rr, m->timenow))
        {
            if (rr->resrec.RecordType == kDNSRecordTypeDeregistering)
            {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                if (!rr->WakeUp.HMAC.l[0])
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                {
                    if (rr->AnnounceCount) rr->ImmedAnswer = mDNSInterfaceMark;     // Send goodbye packet on all interfaces
                }
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                else
                {
                    LogSPS("SendResponses: Sending wakeup %2d for %.6a %s", rr->AnnounceCount-3, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
                    SendWakeup(m, rr->resrec.InterfaceID, &rr->WakeUp.IMAC, &rr->WakeUp.password);
                    for (r2 = rr; r2; r2=r2->next)
                        if (r2->AnnounceCount && r2->resrec.InterfaceID == rr->resrec.InterfaceID && mDNSSameEthAddress(&r2->WakeUp.IMAC, &rr->WakeUp.IMAC) &&
                                !mDNSSameEthAddress(&zeroEthAddr, &r2->WakeUp.HMAC))
                        {
                            // For now we only want to send a single Unsolicited Neighbor Advertisement restoring the address to the original
                            // owner, because these packets can cause some IPv6 stacks to falsely conclude that there's an address conflict.
                            if (r2->AddressProxy.type == mDNSAddrType_IPv6 && r2->AnnounceCount == WakeupCount)
                            {
                                LogSPS("NDP Announcement %2d Releasing traffic for H-MAC %.6a I-MAC %.6a %s",
                                       r2->AnnounceCount-3, &r2->WakeUp.HMAC, &r2->WakeUp.IMAC, ARDisplayString(m,r2));
                                SendNDP(m, NDP_Adv, NDP_Override, r2, &r2->AddressProxy.ip.v6, &r2->WakeUp.IMAC, &AllHosts_v6, &AllHosts_v6_Eth);
                            }
                            r2->LastAPTime = m->timenow;
                            // After 15 wakeups without success (maybe host has left the network) send three goodbyes instead
                            if (--r2->AnnounceCount <= GoodbyeCount) r2->WakeUp.HMAC = zeroEthAddr;
                        }
                }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            }
            else if (ResourceRecordIsValidAnswer(rr))
            {
                if (rr->AddressProxy.type)
                {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    if (!mDNSSameEthAddress(&zeroEthAddr, &rr->WakeUp.HMAC))
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    {
                        rr->AnnounceCount--;
                        rr->ThisAPInterval *= 2;
                        rr->LastAPTime = m->timenow;
                        if (rr->AddressProxy.type == mDNSAddrType_IPv4)
                        {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                            LogSPS("ARP Announcement %2d Capturing traffic for H-MAC %.6a I-MAC %.6a %s",
                                    rr->AnnounceCount, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m,rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                            SendARP(m, 1, rr, &rr->AddressProxy.ip.v4, &zeroEthAddr, &rr->AddressProxy.ip.v4, &onesEthAddr);
                        }
                        else if (rr->AddressProxy.type == mDNSAddrType_IPv6)
                        {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                            LogSPS("NDP Announcement %2d Capturing traffic for H-MAC %.6a I-MAC %.6a %s",
                                    rr->AnnounceCount, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m,rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                            SendNDP(m, NDP_Adv, NDP_Override, rr, &rr->AddressProxy.ip.v6, mDNSNULL, &AllHosts_v6, &AllHosts_v6_Eth);
                        }
                    }
                }
                else
                {
                    rr->ImmedAnswer = mDNSInterfaceMark;        // Send on all interfaces
                    if (maxExistingAnnounceInterval < rr->ThisAPInterval)
                        maxExistingAnnounceInterval = rr->ThisAPInterval;
                    if (rr->UpdateBlocked) rr->UpdateBlocked = 0;
                }
            }
        }
    }

    // Any interface-specific records we're going to send are marked as being sent on all appropriate interfaces (which is just one)
    // Eligible records that are more than half-way to their announcement time are accelerated
    for (rr = m->ResourceRecords; rr; rr=rr->next)
        if ((rr->resrec.InterfaceID && rr->ImmedAnswer) ||
            (rr->ThisAPInterval <= maxExistingAnnounceInterval &&
             TimeToAnnounceThisRecord(rr, m->timenow + rr->ThisAPInterval/2) &&
             !rr->AddressProxy.type &&                  // Don't include ARP Annoucements when considering which records to accelerate
             ResourceRecordIsValidAnswer(rr)))
            rr->ImmedAnswer = mDNSInterfaceMark;        // Send on all interfaces

    // When sending SRV records (particularly when announcing a new service) automatically add related Address record(s) as additionals
    // Note: Currently all address records are interface-specific, so it's safe to set ImmedAdditional to their InterfaceID,
    // which will be non-null. If by some chance there is an address record that's not interface-specific (should never happen)
    // then all that means is that it won't get sent -- which would not be the end of the world.
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (rr->ImmedAnswer && rr->resrec.rrtype == kDNSType_SRV)
            for (r2=m->ResourceRecords; r2; r2=r2->next)                // Scan list of resource records
                if (RRTypeIsAddressType(r2->resrec.rrtype) &&           // For all address records (A/AAAA) ...
                    ResourceRecordIsValidAnswer(r2) &&                  // ... which are valid for answer ...
                    rr->LastMCTime - r2->LastMCTime >= 0 &&             // ... which we have not sent recently ...
                    rr->resrec.rdatahash == r2->resrec.namehash &&      // ... whose name is the name of the SRV target
                    SameDomainName(&rr->resrec.rdata->u.srv.target, r2->resrec.name) &&
                    (rr->ImmedAnswer == mDNSInterfaceMark || rr->ImmedAnswer == r2->resrec.InterfaceID))
                    r2->ImmedAdditional = r2->resrec.InterfaceID;       // ... then mark this address record for sending too
        // We also make sure we send the DeviceInfo TXT record too, if necessary
        // We check for RecordType == kDNSRecordTypeShared because we don't want to tag the
        // DeviceInfo TXT record onto a goodbye packet (RecordType == kDNSRecordTypeDeregistering).
        if (rr->ImmedAnswer && rr->resrec.RecordType == kDNSRecordTypeShared && rr->resrec.rrtype == kDNSType_PTR)
            if (ResourceRecordIsValidAnswer(&m->DeviceInfo) && SameDomainLabel(rr->resrec.rdata->u.name.c, m->DeviceInfo.resrec.name->c))
            {
                if (!m->DeviceInfo.ImmedAnswer) m->DeviceInfo.ImmedAnswer = rr->ImmedAnswer;
                else m->DeviceInfo.ImmedAnswer = mDNSInterfaceMark;
            }
    }

    // If there's a record which is supposed to be unique that we're going to send, then make sure that we give
    // the whole RRSet as an atomic unit. That means that if we have any other records with the same name/type/class
    // then we need to mark them for sending too. Otherwise, if we set the kDNSClass_UniqueRRSet bit on a
    // record, then other RRSet members that have not been sent recently will get flushed out of client caches.
    // -- If a record is marked to be sent on a certain interface, make sure the whole set is marked to be sent on that interface
    // -- If any record is marked to be sent on all interfaces, make sure the whole set is marked to be sent on all interfaces
    for (rr = m->ResourceRecords; rr; rr=rr->next)
        if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
        {
            if (rr->ImmedAnswer)            // If we're sending this as answer, see that its whole RRSet is similarly marked
            {
                for (r2 = m->ResourceRecords; r2; r2=r2->next)
                    if (ResourceRecordIsValidAnswer(r2))
                        if (r2->ImmedAnswer != mDNSInterfaceMark &&
                            r2->ImmedAnswer != rr->ImmedAnswer && SameResourceRecordSignature(r2, rr))
                            r2->ImmedAnswer = !r2->ImmedAnswer ? rr->ImmedAnswer : mDNSInterfaceMark;
            }
            else if (rr->ImmedAdditional)   // If we're sending this as additional, see that its whole RRSet is similarly marked
            {
                for (r2 = m->ResourceRecords; r2; r2=r2->next)
                    if (ResourceRecordIsValidAnswer(r2))
                        if (r2->ImmedAdditional != rr->ImmedAdditional && SameResourceRecordSignature(r2, rr))
                            r2->ImmedAdditional = rr->ImmedAdditional;
            }
        }

    // Now set SendRNow state appropriately
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (rr->ImmedAnswer == mDNSInterfaceMark)       // Sending this record on all appropriate interfaces
        {
            rr->SendRNow = !intf ? mDNSNULL : (rr->resrec.InterfaceID) ? rr->resrec.InterfaceID : intf->InterfaceID;
            rr->ImmedAdditional = mDNSNULL;             // No need to send as additional if sending as answer
            rr->LastMCTime      = m->timenow;
            rr->LastMCInterface = rr->ImmedAnswer;
            // If we're announcing this record, and it's at least half-way to its ordained time, then consider this announcement done
            if (TimeToAnnounceThisRecord(rr, m->timenow + rr->ThisAPInterval/2))
            {
                rr->AnnounceCount--;
                if (rr->resrec.RecordType != kDNSRecordTypeDeregistering)
                    rr->ThisAPInterval *= 2;
                rr->LastAPTime = m->timenow;
                debugf("Announcing %##s (%s) %d", rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), rr->AnnounceCount);
            }
        }
        else if (rr->ImmedAnswer)                       // Else, just respond to a single query on single interface:
        {
            rr->SendRNow        = rr->ImmedAnswer;      // Just respond on that interface
            rr->ImmedAdditional = mDNSNULL;             // No need to send as additional too
            rr->LastMCTime      = m->timenow;
            rr->LastMCInterface = rr->ImmedAnswer;
        }
        SetNextAnnounceProbeTime(m, rr);
        //if (rr->SendRNow) LogMsg("%-15.4a %s", &rr->v4Requester, ARDisplayString(m, rr));
    }

    // ***
    // *** 2. Loop through interface list, sending records as appropriate
    // ***

    while (intf)
    {
        int OwnerRecordSpace = (m->AnnounceOwner && intf->MAC.l[0]) ? DNSOpt_Header_Space + DNSOpt_Owner_Space(&m->PrimaryMAC, &intf->MAC) : 0;
#ifndef REMOVE_MDNS_TRACING  
        int TraceRecordSpace = (mDNS_McastTracingEnabled && MDNS_TRACER) ? DNSOpt_Header_Space + DNSOpt_TraceData_Space : 0;
#endif //#ifndef REMOVE_MDNS_TRACING  
        int numDereg    = 0;
        int numAnnounce = 0;
        int numAnswer   = 0;
        int AnoninfoSpace = 0;
        mDNSu8 *responseptr = m->omsg.data;
        // mdnslogInfo("Responseptr m->omsg.data=%s", m->omsg.data);
        mDNSu8 *newptr;
        InitializeDNSMessage(&m->omsg.h, zeroID, ResponseFlags);

        // First Pass. Look for:
        // 1. Deregistering records that need to send their goodbye packet
        // 2. Updated records that need to retract their old data
        // 3. Answers and announcements we need to send
        for (rr = m->ResourceRecords; rr; rr=rr->next)
        {

            // Skip this interface if the record InterfaceID is *Any and the record is not
            // appropriate for the interface type.
            if ((rr->SendRNow == intf->InterfaceID) &&
                ((rr->resrec.InterfaceID == mDNSInterface_Any) && !mDNSPlatformValidRecordForInterface(rr, intf)))
            {
                LogInfo("SendResponses: Not sending %s, on %s", ARDisplayString(m, rr), InterfaceNameForID(m, rr->SendRNow));
                rr->SendRNow = GetNextActiveInterfaceID(intf);
            }
            else if (rr->SendRNow == intf->InterfaceID)
            {
                RData  *OldRData    = rr->resrec.rdata;
                mDNSu16 oldrdlength = rr->resrec.rdlength;
                mDNSu8 active = (mDNSu8)
                                (rr->resrec.RecordType != kDNSRecordTypeDeregistering && !ShouldSendGoodbyesBeforeSleep(m, intf, rr));
                newptr = mDNSNULL;
                if (rr->NewRData && active)
                {
                    // See if we should send a courtesy "goodbye" for the old data before we replace it.
                    if (ResourceRecordIsValidAnswer(rr) && rr->resrec.RecordType == kDNSRecordTypeShared && rr->RequireGoodbye)
                    {
                        newptr = PutRR_OS_TTL(responseptr, &m->omsg.h.numAnswers, &rr->resrec, 0);
                        if (newptr) { responseptr = newptr; numDereg++; rr->RequireGoodbye = mDNSfalse; }
                        else continue; // If this packet is already too full to hold the goodbye for this record, skip it for now and we'll retry later
                    }
                    SetNewRData(&rr->resrec, rr->NewRData, rr->newrdlength);
                }

                if (rr->resrec.AnonInfo)
                {
                    int tmp = AnonInfoSpace(rr->resrec.AnonInfo);

                    AnoninfoSpace += tmp;
                    // Adjust OwnerRecordSpace/TraceRecordSpace which is used by PutRR_OS_TTL below so that
                    // we have space to put in the NSEC3 record in the authority section.
                    OwnerRecordSpace += tmp;
#ifndef REMOVE_MDNS_TRACING   
                    TraceRecordSpace += tmp;
#endif //#ifndef REMOVE_MDNS_TRACING  
                }

                if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
                    rr->resrec.rrclass |= kDNSClass_UniqueRRSet;        // Temporarily set the cache flush bit so PutResourceRecord will set it
                newptr = PutRR_OS_TTL(responseptr, &m->omsg.h.numAnswers, &rr->resrec, active ? rr->resrec.rroriginalttl : 0);
                rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;           // Make sure to clear cache flush bit back to normal state
                if (newptr)
                {
                    responseptr = newptr;
                    rr->RequireGoodbye = active;
                    if (rr->resrec.RecordType == kDNSRecordTypeDeregistering) numDereg++;
                    else if (rr->LastAPTime == m->timenow) numAnnounce++;else numAnswer++;
                }

                if (rr->NewRData && active)
                    SetNewRData(&rr->resrec, OldRData, oldrdlength);

                // The first time through (pktcount==0), if this record is verified unique
                // (i.e. typically A, AAAA, SRV, TXT and reverse-mapping PTR), set the flag to add an NSEC too.
                if (!pktcount && active && (rr->resrec.RecordType & kDNSRecordTypeActiveUniqueMask) && !rr->SendNSECNow)
                    rr->SendNSECNow = mDNSInterfaceMark;

                if (newptr)     // If succeeded in sending, advance to next interface
                {
                    if (rr->resrec.AnonInfo)
                    {
#ifndef REMOVE_MDNS_TRACING  
                        debugf("SendResponses: Marking %s, OwnerRecordSpace %d, TraceRecordSpace %d, limit %p", ARDisplayString(m, rr), OwnerRecordSpace,
                                TraceRecordSpace, m->omsg.data + AllowedRRSpace(&m->omsg) - OwnerRecordSpace - TraceRecordSpace);
#endif //#ifndef REMOVE_MDNS_TRACING  
                        rr->resrec.AnonInfo->SendNow = intf->InterfaceID;
                    }

                    // If sending on all interfaces, go to next interface; else we're finished now
                    if (rr->ImmedAnswer == mDNSInterfaceMark && rr->resrec.InterfaceID == mDNSInterface_Any)
                        rr->SendRNow = GetNextActiveInterfaceID(intf);
                    else
                        rr->SendRNow = mDNSNULL;
                }
            }
        }

        // Get the reserved space back
        OwnerRecordSpace -= AnoninfoSpace;
#ifndef REMOVE_MDNS_TRACING  
        TraceRecordSpace -= AnoninfoSpace; 
#endif //#ifndef REMOVE_MDNS_TRACING  
        newptr = responseptr;
        for (rr = m->ResourceRecords; rr; rr=rr->next)
        {
            if (rr->resrec.AnonInfo && rr->resrec.AnonInfo->SendNow == intf->InterfaceID)
            {
                ResourceRecord *nsec3RR = rr->resrec.AnonInfo->nsec3RR;

                newptr = PutRR_OS_TTL(newptr, &m->omsg.h.numAuthorities, nsec3RR,  nsec3RR->rroriginalttl);
                if (newptr)
                {
                    responseptr = newptr;
#ifndef REMOVE_MDNS_TRACING  
                   debugf("SendResponses: Added NSEC3 %s, OwnerRecordSpace %d, TraceRecordSpace %d, limit %p", ARDisplayString(m, rr), OwnerRecordSpace,
                            TraceRecordSpace, m->omsg.data + AllowedRRSpace(&m->omsg) - OwnerRecordSpace - TraceRecordSpace);
                }
                else
                {
                    LogMsg("SendResponses: Cannot add NSEC3 %s, OwnerRecordSpace %d, TraceRecordSpace %d, limit %p", ARDisplayString(m, rr), OwnerRecordSpace,
                            TraceRecordSpace, m->omsg.data + AllowedRRSpace(&m->omsg) - OwnerRecordSpace - TraceRecordSpace);
#endif //#ifndef REMOVE_MDNS_TRACING  
                }
                rr->resrec.AnonInfo->SendNow = mDNSNULL;
            }
        }
        // Second Pass. Add additional records, if there's space.
        newptr = responseptr;
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->ImmedAdditional == intf->InterfaceID)
                if (ResourceRecordIsValidAnswer(rr))
                {
                    // If we have at least one answer already in the packet, then plan to add additionals too
                    mDNSBool SendAdditional = (m->omsg.h.numAnswers > 0);

                    // If we're not planning to send any additionals, but this record is a unique one, then
                    // make sure we haven't already sent any other members of its RRSet -- if we have, then they
                    // will have had the cache flush bit set, so now we need to finish the job and send the rest.
                    if (!SendAdditional && (rr->resrec.RecordType & kDNSRecordTypeUniqueMask))
                    {
                        const AuthRecord *a;
                        for (a = m->ResourceRecords; a; a=a->next)
                            if (a->LastMCTime      == m->timenow &&
                                a->LastMCInterface == intf->InterfaceID &&
                                SameResourceRecordSignature(a, rr)) { SendAdditional = mDNStrue; break; }
                    }
                    if (!SendAdditional)                    // If we don't want to send this after all,
                        rr->ImmedAdditional = mDNSNULL;     // then cancel its ImmedAdditional field
                    else if (newptr)                        // Else, try to add it if we can
                    {
                        // The first time through (pktcount==0), if this record is verified unique
                        // (i.e. typically A, AAAA, SRV, TXT and reverse-mapping PTR), set the flag to add an NSEC too.
                        if (!pktcount && (rr->resrec.RecordType & kDNSRecordTypeActiveUniqueMask) && !rr->SendNSECNow)
                            rr->SendNSECNow = mDNSInterfaceMark;

                        if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
                            rr->resrec.rrclass |= kDNSClass_UniqueRRSet;    // Temporarily set the cache flush bit so PutResourceRecord will set it
                        newptr = PutRR_OS(newptr, &m->omsg.h.numAdditionals, &rr->resrec);
                        rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;       // Make sure to clear cache flush bit back to normal state
                        if (newptr)
                        {
                            responseptr = newptr;
                            rr->ImmedAdditional = mDNSNULL;
                            rr->RequireGoodbye = mDNStrue;
                            // If we successfully put this additional record in the packet, we record LastMCTime & LastMCInterface.
                            // This matters particularly in the case where we have more than one IPv6 (or IPv4) address, because otherwise,
                            // when we see our own multicast with the cache flush bit set, if we haven't set LastMCTime, then we'll get
                            // all concerned and re-announce our record again to make sure it doesn't get flushed from peer caches.
                            rr->LastMCTime      = m->timenow;
                            rr->LastMCInterface = intf->InterfaceID;
                        }
                    }
                }

        // Third Pass. Add NSEC records, if there's space.
        // When we're generating an NSEC record in response to a specify query for that type
        // (recognized by rr->SendNSECNow == intf->InterfaceID) we should really put the NSEC in the Answer Section,
        // not Additional Section, but for now it's easier to handle both cases in this Additional Section loop here.
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->SendNSECNow == mDNSInterfaceMark || rr->SendNSECNow == intf->InterfaceID)
            {
                AuthRecord nsec;
                mDNSu8 *ptr;
                int len;
                mDNS_SetupResourceRecord(&nsec, mDNSNULL, mDNSInterface_Any, kDNSType_NSEC, rr->resrec.rroriginalttl, kDNSRecordTypeUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
                nsec.resrec.rrclass |= kDNSClass_UniqueRRSet;
                AssignDomainName(&nsec.namestorage, rr->resrec.name);
                ptr = nsec.rdatastorage.u.data;
                len = DomainNameLength(rr->resrec.name);
                // We have a nxt name followed by window number, window length and a window bitmap
                nsec.resrec.rdlength = len + 2 + NSEC_MCAST_WINDOW_SIZE;
                if (nsec.resrec.rdlength <= StandardAuthRDSize)
                {
                    mDNSPlatformMemZero(ptr, nsec.resrec.rdlength);
                    AssignDomainName((domainname *)ptr, rr->resrec.name);
                    ptr += len;
                    *ptr++ = 0; // window number
                    *ptr++ = NSEC_MCAST_WINDOW_SIZE; // window length
                    for (r2 = m->ResourceRecords; r2; r2=r2->next)
                        if (ResourceRecordIsValidAnswer(r2) && SameResourceRecordNameClassInterface(r2, rr))
                        {
                            if (r2->resrec.rrtype >= kDNSQType_ANY) { LogMsg("SendResponses: Can't create NSEC for record %s", ARDisplayString(m, r2)); break; }
                            else ptr[r2->resrec.rrtype >> 3] |= 128 >> (r2->resrec.rrtype & 7);
                        }
                    newptr = responseptr;
                    if (!r2)    // If we successfully built our NSEC record, add it to the packet now
                    {
                        newptr = PutRR_OS(responseptr, &m->omsg.h.numAdditionals, &nsec.resrec);
                        if (newptr) responseptr = newptr;
                    }
                }
                else LogMsg("SendResponses: not enough space (%d)  in authrecord for nsec", nsec.resrec.rdlength);

                // If we successfully put the NSEC record, clear the SendNSECNow flag
                // If we consider this NSEC optional, then we unconditionally clear the SendNSECNow flag, even if we fail to put this additional record
                if (newptr || rr->SendNSECNow == mDNSInterfaceMark)
                {
                    rr->SendNSECNow = mDNSNULL;
                    // Run through remainder of list clearing SendNSECNow flag for all other records which would generate the same NSEC
                    for (r2 = rr->next; r2; r2=r2->next)
                        if (SameResourceRecordNameClassInterface(r2, rr))
                            if (r2->SendNSECNow == mDNSInterfaceMark || r2->SendNSECNow == intf->InterfaceID)
                                r2->SendNSECNow = mDNSNULL;
                }
            }

        if (m->omsg.h.numAnswers || m->omsg.h.numAdditionals)
        {
            // If we have data to send, add OWNER/TRACER/OWNER+TRACER option if necessary, then send packet
#ifndef REMOVE_MDNS_TRACING  
            if (OwnerRecordSpace || TraceRecordSpace)
#else
            if (OwnerRecordSpace)
#endif //#ifndef REMOVE_MDNS_TRACING  
            {
                AuthRecord opt;
                mDNS_SetupResourceRecord(&opt, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
                opt.resrec.rrclass    = NormalMaxDNSMessageData;
                opt.resrec.rdlength   = sizeof(rdataOPT);
                opt.resrec.rdestimate = sizeof(rdataOPT);
#ifndef REMOVE_MDNS_TRACING  
                if (OwnerRecordSpace && TraceRecordSpace)
                {
                    opt.resrec.rdlength   += sizeof(rdataOPT); // Two options in this OPT record
                    opt.resrec.rdestimate += sizeof(rdataOPT);
                    SetupOwnerOpt(m, intf, &opt.resrec.rdata->u.opt[0]);
                    SetupTracerOpt(m, &opt.resrec.rdata->u.opt[1]);
                }
                else if (OwnerRecordSpace)
                {
#endif //#ifndef REMOVE_MDNS_TRACING  
                    SetupOwnerOpt(m, intf, &opt.resrec.rdata->u.opt[0]);
#ifndef REMOVE_MDNS_TRACING  
                }
                else if (TraceRecordSpace)
                {
                    SetupTracerOpt(m, &opt.resrec.rdata->u.opt[0]);
                }
#endif //#ifndef REMOVE_MDNS_TRACING  
                newptr = PutResourceRecord(&m->omsg, responseptr, &m->omsg.h.numAdditionals, &opt.resrec);
                if (newptr) 
                { 
                    responseptr = newptr; 
#ifndef REMOVE_MDNS_TRACING  
                    LogInfo("SendResponses put %s %s: %s %s", OwnerRecordSpace ? "OWNER" : "", TraceRecordSpace ? "TRACER" : "", intf->ifname, ARDisplayString(m, &opt));
                }
                else if (m->omsg.h.numAnswers + m->omsg.h.numAuthorities + m->omsg.h.numAdditionals == 1)
                {
                    LogInfo("SendResponses: No space in packet for %s %s OPT record (%d/%d/%d/%d) %s", OwnerRecordSpace ? "OWNER" : "", TraceRecordSpace ? "TRACER" : "",
                            m->omsg.h.numQuestions, m->omsg.h.numAnswers, m->omsg.h.numAuthorities, m->omsg.h.numAdditionals, ARDisplayString(m, &opt));
                }
                else
                {
                    LogMsg("SendResponses: How did we fail to have space for %s %s OPT record (%d/%d/%d/%d) %s", OwnerRecordSpace ? "OWNER" : "", TraceRecordSpace ? "TRACER" : "",
                           m->omsg.h.numQuestions, m->omsg.h.numAnswers, m->omsg.h.numAuthorities, m->omsg.h.numAdditionals, ARDisplayString(m, &opt));
#endif //#ifndef REMOVE_MDNS_TRACING  
                }
            }

            mdnslogInfo("SendResponses: Sending %d Deregistration%s, %d Announcement%s, %d Answer%s, %d Additional%s on %p",
                   numDereg,                 numDereg                 == 1 ? "" : "s",
                   numAnnounce,              numAnnounce              == 1 ? "" : "s",
                   numAnswer,                numAnswer                == 1 ? "" : "s",
                   m->omsg.h.numAdditionals, m->omsg.h.numAdditionals == 1 ? "" : "s", intf->InterfaceID);

            if (intf->IPv4Available)
            {
                // mdnslogInfo("IPv4 mDNSSendDNSMessage=%s", m->omsg.data);
                mDNSSendDNSMessage(m, &m->omsg, responseptr, intf->InterfaceID, mDNSNULL, &AllDNSLinkGroup_v4, MulticastDNSPort, mDNSNULL, mDNSNULL, mDNSfalse);
            }
            if (intf->IPv6Available) 
            {
                // mdnslogInfo("IPv6 mDNSSendDNSMessage=%s", m->omsg.data);
                mDNSSendDNSMessage(m, &m->omsg, responseptr, intf->InterfaceID, mDNSNULL, &AllDNSLinkGroup_v6, MulticastDNSPort, mDNSNULL, mDNSNULL, mDNSfalse);
            }
            if (!m->SuppressSending) m->SuppressSending = NonZeroTime(m->timenow + (mDNSPlatformOneSecond+9)/10);
            if (++pktcount >= 1000) { LogMsg("SendResponses exceeded loop limit %d: giving up", pktcount); break; }
            // There might be more things to send on this interface, so go around one more time and try again.
        }
        else    // Nothing more to send on this interface; go to next
        {
            const NetworkInterfaceInfo *next = GetFirstActiveInterface(intf->next);
            #if MDNS_DEBUGMSGS && 0
            const char *const msg = next ? "SendResponses: Nothing more on %p; moving to %p" : "SendResponses: Nothing more on %p";
            debugf(msg, intf, next);
            #endif
            intf = next;
            pktcount = 0;       // When we move to a new interface, reset packet count back to zero -- NSEC generation logic uses it
        }
    }

    // ***
    // *** 3. Cleanup: Now that everything is sent, call client callback functions, and reset state variables
    // ***

    if (m->CurrentRecord)
        LogMsg("SendResponses ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = m->ResourceRecords;
    while (m->CurrentRecord)
    {
        rr = m->CurrentRecord;
        m->CurrentRecord = rr->next;

        if (rr->SendRNow)
        {
            if (rr->ARType != AuthRecordLocalOnly && rr->ARType != AuthRecordP2P)
                LogMsg("SendResponses: No active interface %p to send: %p %02X %s", rr->SendRNow, rr->resrec.InterfaceID, rr->resrec.RecordType, ARDisplayString(m, rr));
            rr->SendRNow = mDNSNULL;
        }

        if (rr->ImmedAnswer || rr->resrec.RecordType == kDNSRecordTypeDeregistering)
        {
            if (rr->NewRData) CompleteRDataUpdate(m, rr);   // Update our rdata, clear the NewRData pointer, and return memory to the client

            if (rr->resrec.RecordType == kDNSRecordTypeDeregistering && rr->AnnounceCount == 0)
            {
                // For Unicast, when we get the response from the server, we will call CompleteDeregistration
                if (!AuthRecord_uDNS(rr)) CompleteDeregistration(m, rr);        // Don't touch rr after this
            }
            else
            {
                rr->ImmedAnswer  = mDNSNULL;
                rr->ImmedUnicast = mDNSfalse;
                rr->v4Requester  = zerov4Addr;
                rr->v6Requester  = zerov6Addr;
            }
        }
    }
    verbosedebugf("SendResponses: Next in %ld ticks", m->NextScheduledResponse - m->timenow);
}

// Calling CheckCacheExpiration() is an expensive operation because it has to look at the entire cache,
// so we want to be lazy about how frequently we do it.
// 1. If a cache record is currently referenced by *no* active questions,
//    then we don't mind expiring it up to a minute late (who will know?)
// 2. Else, if a cache record is due for some of its final expiration queries,
//    we'll allow them to be late by up to 2% of the TTL
// 3. Else, if a cache record has completed all its final expiration queries without success,
//    and is expiring, and had an original TTL more than ten seconds, we'll allow it to be one second late
// 4. Else, it is expiring and had an original TTL of ten seconds or less (includes explicit goodbye packets),
//    so allow at most 1/10 second lateness
// 5. For records with rroriginalttl set to zero, that means we really want to delete them immediately
//    (we have a new record with DelayDelivery set, waiting for the old record to go away before we can notify clients).
#define CacheCheckGracePeriod(RR) (                                                   \
        ((RR)->CRActiveQuestion == mDNSNULL            ) ? (60 * mDNSPlatformOneSecond) : \
        ((RR)->UnansweredQueries < MaxUnansweredQueries) ? (TicksTTL(rr)/50)            : \
        ((RR)->resrec.rroriginalttl > 10               ) ? (mDNSPlatformOneSecond)      : \
        ((RR)->resrec.rroriginalttl > 0                ) ? (mDNSPlatformOneSecond/10)   : 0)

#define NextCacheCheckEvent(RR) ((RR)->NextRequiredQuery + CacheCheckGracePeriod(RR))

mDNSexport void ScheduleNextCacheCheckTime(mDNS *const m, const mDNSu32 slot, const mDNSs32 event)
{
    if (m->rrcache_nextcheck[slot] - event > 0)
        m->rrcache_nextcheck[slot] = event;
    if (m->NextCacheCheck          - event > 0)
        m->NextCacheCheck          = event;
}

// Note: MUST call SetNextCacheCheckTimeForRecord any time we change:
// rr->TimeRcvd
// rr->resrec.rroriginalttl
// rr->UnansweredQueries
// rr->CRActiveQuestion
mDNSexport void SetNextCacheCheckTimeForRecord(mDNS *const m, CacheRecord *const rr)
{
    rr->NextRequiredQuery = RRExpireTime(rr);

    // If we have an active question, then see if we want to schedule a refresher query for this record.
    // Usually we expect to do four queries, at 80-82%, 85-87%, 90-92% and then 95-97% of the TTL.
    if (rr->CRActiveQuestion && rr->UnansweredQueries < MaxUnansweredQueries)
    {
        rr->NextRequiredQuery -= TicksTTL(rr)/20 * (MaxUnansweredQueries - rr->UnansweredQueries);
        rr->NextRequiredQuery += mDNSRandom((mDNSu32)TicksTTL(rr)/50);
        verbosedebugf("SetNextCacheCheckTimeForRecord: NextRequiredQuery in %ld sec CacheCheckGracePeriod %d ticks for %s",
                      (rr->NextRequiredQuery - m->timenow) / mDNSPlatformOneSecond, CacheCheckGracePeriod(rr), CRDisplayString(m,rr));
    }
    ScheduleNextCacheCheckTime(m, HashSlot(rr->resrec.name), NextCacheCheckEvent(rr));
}

#define kMinimumReconfirmTime                     ((mDNSu32)mDNSPlatformOneSecond *  5)
#define kDefaultReconfirmTimeForWake              ((mDNSu32)mDNSPlatformOneSecond *  5)
#define kDefaultReconfirmTimeForNoAnswer          ((mDNSu32)mDNSPlatformOneSecond *  5)
#define kDefaultReconfirmTimeForFlappingInterface ((mDNSu32)mDNSPlatformOneSecond *  5)

mDNSexport mStatus mDNS_Reconfirm_internal(mDNS *const m, CacheRecord *const rr, mDNSu32 interval)
{
    if (interval < kMinimumReconfirmTime)
        interval = kMinimumReconfirmTime;
    if (interval > 0x10000000)  // Make sure interval doesn't overflow when we multiply by four below
        interval = 0x10000000;

    // If the expected expiration time for this record is more than interval+33%, then accelerate its expiration
    if (RRExpireTime(rr) - m->timenow > (mDNSs32)((interval * 4) / 3))
    {
        // Add a 33% random amount to the interval, to avoid synchronization between multiple hosts
        // For all the reconfirmations in a given batch, we want to use the same random value
        // so that the reconfirmation questions can be grouped into a single query packet
        if (!m->RandomReconfirmDelay) m->RandomReconfirmDelay = 1 + mDNSRandom(0x3FFFFFFF);
        interval += m->RandomReconfirmDelay % ((interval/3) + 1);
        rr->TimeRcvd          = m->timenow - (mDNSs32)interval * 3;
        rr->resrec.rroriginalttl     = (interval * 4 + mDNSPlatformOneSecond - 1) / mDNSPlatformOneSecond;
        SetNextCacheCheckTimeForRecord(m, rr);
    }
    debugf("mDNS_Reconfirm_internal:%6ld ticks to go for %s %p",
           RRExpireTime(rr) - m->timenow, CRDisplayString(m, rr), rr->CRActiveQuestion);
    return(mStatus_NoError);
}

// BuildQuestion puts a question into a DNS Query packet and if successful, updates the value of queryptr.
// It also appends to the list of known answer records that need to be included,
// and updates the forcast for the size of the known answer section.
mDNSlocal mDNSBool BuildQuestion(mDNS *const m, DNSMessage *query, mDNSu8 **queryptr, DNSQuestion *q,
                                 CacheRecord ***kalistptrptr, mDNSu32 *answerforecast)
{
    mDNSBool ucast = (q->LargeAnswers || q->RequestUnicast) && m->CanReceiveUnicastOn5353;
    mDNSu16 ucbit = (mDNSu16)(ucast ? kDNSQClass_UnicastResponse : 0);
    const mDNSu8 *const limit = query->data + NormalMaxDNSMessageData;
    mDNSu8 anoninfo_space = q->AnonInfo ? AnonInfoSpace(q->AnonInfo) : 0;
    mDNSu8 *newptr = putQuestion(query, *queryptr, limit - *answerforecast - anoninfo_space, &q->qname, q->qtype, (mDNSu16)(q->qclass | ucbit));
    if (!newptr)
    {
        debugf("BuildQuestion: No more space in this packet for question %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        return(mDNSfalse);
    }
    else
    {
        mDNSu32 forecast = *answerforecast + anoninfo_space;
        const mDNSu32 slot = HashSlot(&q->qname);
        const CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
        CacheRecord *rr;
        CacheRecord **ka = *kalistptrptr;   // Make a working copy of the pointer we're going to update

        for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)             // If we have a resource record in our cache,
            if (rr->resrec.InterfaceID == q->SendQNow &&                    // received on this interface
                !(rr->resrec.RecordType & kDNSRecordTypeUniqueMask) &&      // which is a shared (i.e. not unique) record type
                rr->NextInKAList == mDNSNULL && ka != &rr->NextInKAList &&  // which is not already in the known answer list
                rr->resrec.rdlength <= SmallRecordLimit &&                  // which is small enough to sensibly fit in the packet
                SameNameRecordAnswersQuestion(&rr->resrec, q) &&            // which answers our question
                rr->TimeRcvd + TicksTTL(rr)/2 - m->timenow >                // and its half-way-to-expiry time is at least 1 second away
                mDNSPlatformOneSecond)                                      // (also ensures we never include goodbye records with TTL=1)
            {
                // We don't want to include unique records in the Known Answer section. The Known Answer section
                // is intended to suppress floods of shared-record replies from many other devices on the network.
                // That concept really does not apply to unique records, and indeed if we do send a query for
                // which we have a unique record already in our cache, then including that unique record as a
                // Known Answer, so as to suppress the only answer we were expecting to get, makes little sense.

                *ka = rr;   // Link this record into our known answer chain
                ka = &rr->NextInKAList;
                // We forecast: compressed name (2) type (2) class (2) TTL (4) rdlength (2) rdata (n)
                forecast += 12 + rr->resrec.rdestimate;
                // If we're trying to put more than one question in this packet, and it doesn't fit
                // then undo that last question and try again next time
                if (query->h.numQuestions > 1 && newptr + forecast >= limit)
                {
                    query->h.numQuestions--;
                    debugf("BuildQuestion: Retracting question %##s (%s) new forecast total %d, total questions %d",
                           q->qname.c, DNSTypeName(q->qtype), newptr + forecast - query->data, query->h.numQuestions);
                    ka = *kalistptrptr;     // Go back to where we started and retract these answer records
                    while (*ka) { CacheRecord *c = *ka; *ka = mDNSNULL; ka = &c->NextInKAList; }
                    return(mDNSfalse);      // Return false, so we'll try again in the next packet
                }
            }

        // Success! Update our state pointers, increment UnansweredQueries as appropriate, and return
        *queryptr        = newptr;              // Update the packet pointer
        *answerforecast  = forecast;            // Update the forecast
        *kalistptrptr    = ka;                  // Update the known answer list pointer
        if (ucast) q->ExpectUnicastResp = NonZeroTime(m->timenow);

        for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)             // For every resource record in our cache,
            if (rr->resrec.InterfaceID == q->SendQNow &&                    // received on this interface
                rr->NextInKAList == mDNSNULL && ka != &rr->NextInKAList &&  // which is not in the known answer list
                SameNameRecordAnswersQuestion(&rr->resrec, q))              // which answers our question
            {
                rr->UnansweredQueries++;                                    // indicate that we're expecting a response
                rr->LastUnansweredTime = m->timenow;
                SetNextCacheCheckTimeForRecord(m, rr);
            }

        return(mDNStrue);
    }
}

// When we have a query looking for a specified name, but there appear to be no answers with
// that name, ReconfirmAntecedents() is called with depth=0 to start the reconfirmation process
// for any records in our cache that reference the given name (e.g. PTR and SRV records).
// For any such cache record we find, we also recursively call ReconfirmAntecedents() for *its* name.
// We increment depth each time we recurse, to guard against possible infinite loops, with a limit of 5.
// A typical reconfirmation scenario might go like this:
// Depth 0: Name "myhost.local" has no address records
// Depth 1: SRV "My Service._example._tcp.local." refers to "myhost.local"; may be stale
// Depth 2: PTR "_example._tcp.local." refers to "My Service"; may be stale
// Depth 3: PTR "_services._dns-sd._udp.local." refers to "_example._tcp.local."; may be stale
// Currently depths 4 and 5 are not expected to occur; if we did get to depth 5 we'd reconfim any records we
// found referring to the given name, but not recursively descend any further reconfirm *their* antecedents.
mDNSlocal void ReconfirmAntecedents(mDNS *const m, const domainname *const name, const mDNSu32 namehash, const int depth)
{
    mDNSu32 slot;
    CacheGroup *cg;
    CacheRecord *cr;
    debugf("ReconfirmAntecedents (depth=%d) for %##s", depth, name->c);
    FORALL_CACHERECORDS(slot, cg, cr)
    {
        domainname *crtarget = GetRRDomainNameTarget(&cr->resrec);
        if (crtarget && cr->resrec.rdatahash == namehash && SameDomainName(crtarget, name))
        {
            LogInfo("ReconfirmAntecedents: Reconfirming (depth=%d) %s", depth, CRDisplayString(m, cr));
            mDNS_Reconfirm_internal(m, cr, kDefaultReconfirmTimeForNoAnswer);
            if (depth < 5)
                ReconfirmAntecedents(m, cr->resrec.name, cr->resrec.namehash, depth+1);
        }
    }
}

// If we get no answer for a AAAA query, then before doing an automatic implicit ReconfirmAntecedents
// we check if we have an address record for the same name. If we do have an IPv4 address for a given
// name but not an IPv6 address, that's okay (it just means the device doesn't do IPv6) so the failure
// to get a AAAA response is not grounds to doubt the PTR/SRV chain that lead us to that name.
mDNSlocal const CacheRecord *CacheHasAddressTypeForName(mDNS *const m, const domainname *const name, const mDNSu32 namehash)
{
    CacheGroup *const cg = CacheGroupForName(m, HashSlot(name), namehash, name);
    const CacheRecord *cr = cg ? cg->members : mDNSNULL;
    while (cr && !RRTypeIsAddressType(cr->resrec.rrtype)) cr=cr->next;
    return(cr);
}


mDNSlocal const CacheRecord *FindSPSInCache1(mDNS *const m, const DNSQuestion *const q, const CacheRecord *const c0, const CacheRecord *const c1)
{
#ifndef SPC_DISABLED
    CacheGroup *const cg = CacheGroupForName(m, HashSlot(&q->qname), q->qnamehash, &q->qname);
    const CacheRecord *cr, *bestcr = mDNSNULL;
    mDNSu32 bestmetric = 1000000;
    for (cr = cg ? cg->members : mDNSNULL; cr; cr=cr->next)
        if (cr->resrec.rrtype == kDNSType_PTR && cr->resrec.rdlength >= 6)                      // If record is PTR type, with long enough name,
            if (cr != c0 && cr != c1)                                                           // that's not one we've seen before,
                if (SameNameRecordAnswersQuestion(&cr->resrec, q))                              // and answers our browse query,
                    if (!IdenticalSameNameRecord(&cr->resrec, &m->SPSRecords.RR_PTR.resrec))    // and is not our own advertised service...
                    {
                        mDNSu32 metric = SPSMetric(cr->resrec.rdata->u.name.c);
                        if (bestmetric > metric) { bestmetric = metric; bestcr = cr; }
                    }
    return(bestcr);
#else // SPC_DISABLED
	(void) m;
	(void) q;
	(void) c0;
	(void) c1;
	(void) c1;
    return mDNSNULL;
#endif // SPC_DISABLED
}

mDNSlocal void CheckAndSwapSPS(const CacheRecord *sps1, const CacheRecord *sps2)
{
    const CacheRecord *swap_sps;
    mDNSu32 metric1, metric2;

    if (!sps1 || !sps2) return;
    metric1 = SPSMetric(sps1->resrec.rdata->u.name.c);
    metric2 = SPSMetric(sps2->resrec.rdata->u.name.c);
    if (!SPSFeatures(sps1->resrec.rdata->u.name.c) && SPSFeatures(sps2->resrec.rdata->u.name.c) && (metric2 >= metric1))
    {
        swap_sps = sps1;
        sps1     = sps2;
        sps2     = swap_sps;
    }
}

mDNSlocal void ReorderSPSByFeature(const CacheRecord *sps[3])
{
    CheckAndSwapSPS(sps[0], sps[1]);
    CheckAndSwapSPS(sps[0], sps[2]);
    CheckAndSwapSPS(sps[1], sps[2]);
}


// Finds the three best Sleep Proxies we currently have in our cache
mDNSexport void FindSPSInCache(mDNS *const m, const DNSQuestion *const q, const CacheRecord *sps[3])
{
    sps[0] =                      FindSPSInCache1(m, q, mDNSNULL, mDNSNULL);
    sps[1] = !sps[0] ? mDNSNULL : FindSPSInCache1(m, q, sps[0],   mDNSNULL);
    sps[2] = !sps[1] ? mDNSNULL : FindSPSInCache1(m, q, sps[0],   sps[1]);

    // SPS is already sorted by metric. We want to move the entries to the beginning of the array
    // only if they have equally good metric and support features.
    ReorderSPSByFeature(sps);
}

// Only DupSuppressInfos newer than the specified 'time' are allowed to remain active
mDNSlocal void ExpireDupSuppressInfo(DupSuppressInfo ds[DupSuppressInfoSize], mDNSs32 time)
{
    int i;
    for (i=0; i<DupSuppressInfoSize; i++) if (ds[i].Time - time < 0) ds[i].InterfaceID = mDNSNULL;
}

mDNSlocal void ExpireDupSuppressInfoOnInterface(DupSuppressInfo ds[DupSuppressInfoSize], mDNSs32 time, mDNSInterfaceID InterfaceID)
{
    int i;
    for (i=0; i<DupSuppressInfoSize; i++) if (ds[i].InterfaceID == InterfaceID && ds[i].Time - time < 0) ds[i].InterfaceID = mDNSNULL;
}

mDNSlocal mDNSBool SuppressOnThisInterface(const DupSuppressInfo ds[DupSuppressInfoSize], const NetworkInterfaceInfo * const intf)
{
    int i;
    mDNSBool v4 = !intf->IPv4Available;     // If this interface doesn't do v4, we don't need to find a v4 duplicate of this query
    mDNSBool v6 = !intf->IPv6Available;     // If this interface doesn't do v6, we don't need to find a v6 duplicate of this query
    for (i=0; i<DupSuppressInfoSize; i++)
        if (ds[i].InterfaceID == intf->InterfaceID)
        {
            if      (ds[i].Type == mDNSAddrType_IPv4) v4 = mDNStrue;
            else if (ds[i].Type == mDNSAddrType_IPv6) v6 = mDNStrue;
            if (v4 && v6) return(mDNStrue);
        }
    return(mDNSfalse);
}

mDNSlocal int RecordDupSuppressInfo(DupSuppressInfo ds[DupSuppressInfoSize], mDNSs32 Time, mDNSInterfaceID InterfaceID, mDNSs32 Type)
{
    int i, j;

    // See if we have this one in our list somewhere already
    for (i=0; i<DupSuppressInfoSize; i++) if (ds[i].InterfaceID == InterfaceID && ds[i].Type == Type) break;

    // If not, find a slot we can re-use
    if (i >= DupSuppressInfoSize)
    {
        i = 0;
        for (j=1; j<DupSuppressInfoSize && ds[i].InterfaceID; j++)
            if (!ds[j].InterfaceID || ds[j].Time - ds[i].Time < 0)
                i = j;
    }

    // Record the info about this query we saw
    ds[i].Time        = Time;
    ds[i].InterfaceID = InterfaceID;
    ds[i].Type        = Type;

    return(i);
}
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void mDNSSendWakeOnResolve(mDNS *const m, DNSQuestion *q)
{
    int len, i, cnt;
    mDNSInterfaceID InterfaceID = q->InterfaceID;
    domainname *d = &q->qname;

    // We can't send magic packets without knowing which interface to send it on.
    if (InterfaceID == mDNSInterface_Any || InterfaceID == mDNSInterface_LocalOnly || InterfaceID == mDNSInterface_P2P)
    {
        LogMsg("mDNSSendWakeOnResolve: ERROR!! Invalid InterfaceID %p for question %##s", InterfaceID, q->qname.c);
        return;
    }

    // Split MAC@IPAddress and pass them separately
    len = d->c[0];
    i = 1;
    cnt = 0;
    for (i = 1; i < len; i++)
    {
        if (d->c[i] == '@')
        {
            char EthAddr[18];   // ethernet adddress : 12 bytes + 5 ":" + 1 NULL byte
            char IPAddr[47];    // Max IP address len: 46 bytes (IPv6) + 1 NULL byte
            if (cnt != 5)
            {
                LogMsg("mDNSSendWakeOnResolve: ERROR!! Malformed Ethernet address %##s, cnt %d", q->qname.c, cnt);
                return;
            }
            if ((i - 1) > (int) (sizeof(EthAddr) - 1))
            {
                LogMsg("mDNSSendWakeOnResolve: ERROR!! Malformed Ethernet address %##s, length %d", q->qname.c, i - 1);
                return;
            }
            if ((len - i) > (int)(sizeof(IPAddr) - 1))
            {
                LogMsg("mDNSSendWakeOnResolve: ERROR!! Malformed IP address %##s, length %d", q->qname.c, len - i);
                return;
            }
            mDNSPlatformMemCopy(EthAddr, &d->c[1], i - 1);
            EthAddr[i - 1] = 0;
            mDNSPlatformMemCopy(IPAddr, &d->c[i + 1], len - i);
            IPAddr[len - i] = 0;
            m->mDNSStats.WakeOnResolves++;
            mDNSPlatformSendWakeupPacket(m, InterfaceID, EthAddr, IPAddr, InitialWakeOnResolveCount - q->WakeOnResolveCount);
            return;
        }
        else if (d->c[i] == ':')
            cnt++;
    }
    LogMsg("mDNSSendWakeOnResolve: ERROR!! Malformed WakeOnResolve name %##s", q->qname.c);
}
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

mDNSlocal mDNSBool AccelerateThisQuery(mDNS *const m, DNSQuestion *q)
{
    // If more than 90% of the way to the query time, we should unconditionally accelerate it
    if (TimeToSendThisQuestion(q, m->timenow + q->ThisQInterval/10))
        return(mDNStrue);

    // If half-way to next scheduled query time, only accelerate if it will add less than 512 bytes to the packet
    if (TimeToSendThisQuestion(q, m->timenow + q->ThisQInterval/2))
    {
        // We forecast: qname (n) type (2) class (2)
        mDNSu32 forecast = (mDNSu32)DomainNameLength(&q->qname) + 4;
        const mDNSu32 slot = HashSlot(&q->qname);
        const CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
        const CacheRecord *rr;
        for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)             // If we have a resource record in our cache,
            if (rr->resrec.rdlength <= SmallRecordLimit &&                  // which is small enough to sensibly fit in the packet
                SameNameRecordAnswersQuestion(&rr->resrec, q) &&            // which answers our question
                rr->TimeRcvd + TicksTTL(rr)/2 - m->timenow >= 0 &&          // and it is less than half-way to expiry
                rr->NextRequiredQuery - (m->timenow + q->ThisQInterval) > 0) // and we'll ask at least once again before NextRequiredQuery
            {
                // We forecast: compressed name (2) type (2) class (2) TTL (4) rdlength (2) rdata (n)
                forecast += 12 + rr->resrec.rdestimate;
                if (forecast >= 512) return(mDNSfalse); // If this would add 512 bytes or more to the packet, don't accelerate
            }
        return(mDNStrue);
    }

    return(mDNSfalse);
}

// How Standard Queries are generated:
// 1. The Question Section contains the question
// 2. The Additional Section contains answers we already know, to suppress duplicate responses

// How Probe Queries are generated:
// 1. The Question Section contains queries for the name we intend to use, with QType=ANY because
// if some other host is already using *any* records with this name, we want to know about it.
// 2. The Authority Section contains the proposed values we intend to use for one or more
// of our records with that name (analogous to the Update section of DNS Update packets)
// because if some other host is probing at the same time, we each want to know what the other is
// planning, in order to apply the tie-breaking rule to see who gets to use the name and who doesn't.

mDNSlocal void SendQueries(mDNS *const m)
{
    mDNSu32 slot;
    CacheGroup *cg;
    CacheRecord *cr;
    AuthRecord *ar;
    int pktcount = 0;
    DNSQuestion *q;
    // For explanation of maxExistingQuestionInterval logic, see comments for maxExistingAnnounceInterval
    mDNSs32 maxExistingQuestionInterval = 0;
    const NetworkInterfaceInfo *intf = GetFirstActiveInterface(m->HostInterfaces);
    CacheRecord *KnownAnswerList = mDNSNULL;

    // 1. If time for a query, work out what we need to do

    // We're expecting to send a query anyway, so see if any expiring cache records are close enough
    // to their NextRequiredQuery to be worth batching them together with this one
    FORALL_CACHERECORDS(slot, cg, cr)
    {
        if (cr->CRActiveQuestion && cr->UnansweredQueries < MaxUnansweredQueries)
        {
            if (m->timenow + TicksTTL(cr)/50 - cr->NextRequiredQuery >= 0)
            {
                debugf("Sending %d%% cache expiration query for %s", 80 + 5 * cr->UnansweredQueries, CRDisplayString(m, cr));
                q = cr->CRActiveQuestion;
                ExpireDupSuppressInfoOnInterface(q->DupSuppress, m->timenow - TicksTTL(cr)/20, cr->resrec.InterfaceID);
                // For uDNS queries (TargetQID non-zero) we adjust LastQTime,
                // and bump UnansweredQueries so that we don't spin trying to send the same cache expiration query repeatedly
                if (q->Target.type)
                {
                    q->SendQNow = mDNSInterfaceMark; // If targeted query, mark it
                }
                else if (!mDNSOpaque16IsZero(q->TargetQID))
                {
                    q->LastQTime = m->timenow - q->ThisQInterval;
                    cr->UnansweredQueries++;
                    m->mDNSStats.CacheRefreshQueries++;
                }
                else if (q->SendQNow == mDNSNULL)
                {
                    q->SendQNow = cr->resrec.InterfaceID;
                }
                else if (q->SendQNow != cr->resrec.InterfaceID)
                {
                    q->SendQNow = mDNSInterfaceMark;
                }

                // Indicate that this question was marked for sending
                // to update an existing cached answer record.
                // The browse throttling logic below uses this to determine
                // if the query should be sent.
                if (mDNSOpaque16IsZero(q->TargetQID))
                    q->CachedAnswerNeedsUpdate = mDNStrue;
            }
        }
    }

    // Scan our list of questions to see which:
    //     *WideArea*  queries need to be sent
    //     *unicast*   queries need to be sent
    //     *multicast* queries we're definitely going to send
    if (m->CurrentQuestion)
        LogMsg("SendQueries ERROR m->CurrentQuestion already set: %##s (%s)", m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    while (m->CurrentQuestion && m->CurrentQuestion != m->NewQuestions)
    {
        q = m->CurrentQuestion;
        if (q->Target.type && (q->SendQNow || TimeToSendThisQuestion(q, m->timenow)))
        {
            mDNSu8       *qptr        = m->omsg.data;
            const mDNSu8 *const limit = m->omsg.data + sizeof(m->omsg.data);

            // If we fail to get a new on-demand socket (should only happen cases of the most extreme resource exhaustion), we'll try again next time
            if (!q->LocalSocket) q->LocalSocket = mDNSPlatformUDPSocket(m, zeroIPPort);
            if (q->LocalSocket)
            {
                InitializeDNSMessage(&m->omsg.h, q->TargetQID, QueryFlags);
                qptr = putQuestion(&m->omsg, qptr, limit, &q->qname, q->qtype, q->qclass);
                mDNSSendDNSMessage(m, &m->omsg, qptr, mDNSInterface_Any, q->LocalSocket, &q->Target, q->TargetPort, mDNSNULL, mDNSNULL, q->UseBackgroundTrafficClass);
                q->ThisQInterval    *= QuestionIntervalStep;
            }
            if (q->ThisQInterval > MaxQuestionInterval)
                q->ThisQInterval = MaxQuestionInterval;
            q->LastQTime         = m->timenow;
            q->LastQTxTime       = m->timenow;
            q->RecentAnswerPkts  = 0;
            q->SendQNow          = mDNSNULL;
            q->ExpectUnicastResp = NonZeroTime(m->timenow);
        }
        else if (mDNSOpaque16IsZero(q->TargetQID) && !q->Target.type && TimeToSendThisQuestion(q, m->timenow))
        {
            //LogInfo("Time to send %##s (%s) %d", q->qname.c, DNSTypeName(q->qtype), m->timenow - NextQSendTime(q));
            q->SendQNow = mDNSInterfaceMark;        // Mark this question for sending on all interfaces
            if (maxExistingQuestionInterval < q->ThisQInterval)
                maxExistingQuestionInterval = q->ThisQInterval;
        }
        // If m->CurrentQuestion wasn't modified out from under us, advance it now
        // We can't do this at the start of the loop because uDNS_CheckCurrentQuestion() depends on having
        // m->CurrentQuestion point to the right question
        if (q == m->CurrentQuestion) m->CurrentQuestion = m->CurrentQuestion->next;
    }
    while (m->CurrentQuestion)
    {
        LogInfo("SendQueries question loop 1: Skipping NewQuestion %##s (%s)", m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
        m->CurrentQuestion = m->CurrentQuestion->next;
    }
    m->CurrentQuestion = mDNSNULL;

    // Scan our list of questions
    // (a) to see if there are any more that are worth accelerating, and
    // (b) to update the state variables for *all* the questions we're going to send
    // Note: Don't set NextScheduledQuery until here, because uDNS_CheckCurrentQuestion in the loop above can add new questions to the list,
    // which causes NextScheduledQuery to get (incorrectly) set to m->timenow. Setting it here is the right place, because the very
    // next thing we do is scan the list and call SetNextQueryTime() for every question we find, so we know we end up with the right value.
    m->NextScheduledQuery = m->timenow + 0x78000000;
    for (q = m->Questions; q && q != m->NewQuestions; q=q->next)
    {
        if (mDNSOpaque16IsZero(q->TargetQID)
            && (q->SendQNow || (!q->Target.type && ActiveQuestion(q) && q->ThisQInterval <= maxExistingQuestionInterval && AccelerateThisQuery(m,q))))
        {
            // If at least halfway to next query time, advance to next interval
            // If less than halfway to next query time, then
            // treat this as logically a repeat of the last transmission, without advancing the interval
            if (m->timenow - (q->LastQTime + (q->ThisQInterval/2)) >= 0)
            {
                // If we have reached the answer threshold for this question, 
                // don't send it again until MaxQuestionInterval unless:
                //  one of its cached answers needs to be refreshed,
                //  or it's the initial query for a kDNSServiceFlagsThresholdFinder mode browse.
                if (q->BrowseThreshold 
                    && (q->CurrentAnswers >= q->BrowseThreshold)
                    && (q->CachedAnswerNeedsUpdate == mDNSfalse)
                    && !((q->flags & kDNSServiceFlagsThresholdFinder) && (q->ThisQInterval == InitialQuestionInterval)))
                {
                    q->SendQNow = mDNSNULL;
                    q->ThisQInterval = MaxQuestionInterval;
                    q->LastQTime = m->timenow;
                    q->RequestUnicast = 0;
                    LogInfo("SendQueries: (%s) %##s reached threshold of %d answers",
                         DNSTypeName(q->qtype), q->qname.c, q->BrowseThreshold);
                }
                else
                {
                    // Mark this question for sending on all interfaces
                    q->SendQNow = mDNSInterfaceMark;
                    q->ThisQInterval *= QuestionIntervalStep;
                }

                debugf("SendQueries: %##s (%s) next interval %d seconds RequestUnicast = %d",
                       q->qname.c, DNSTypeName(q->qtype), q->ThisQInterval / InitialQuestionInterval, q->RequestUnicast);

                if (q->ThisQInterval >= QuestionIntervalThreshold)
                {
                    q->ThisQInterval = MaxQuestionInterval;
                }
                else if (q->CurrentAnswers == 0 && q->ThisQInterval == InitialQuestionInterval * QuestionIntervalStep3 && !q->RequestUnicast &&
                         !(RRTypeIsAddressType(q->qtype) && CacheHasAddressTypeForName(m, &q->qname, q->qnamehash)))
                {
                    // Generally don't need to log this.
                    // It's not especially noteworthy if a query finds no results -- this usually happens for domain
                    // enumeration queries in the LL subdomain (e.g. "db._dns-sd._udp.0.0.254.169.in-addr.arpa")
                    // and when there simply happen to be no instances of the service the client is looking
                    // for (e.g. iTunes is set to look for RAOP devices, and the current network has none).
                    debugf("SendQueries: Zero current answers for %##s (%s); will reconfirm antecedents",
                           q->qname.c, DNSTypeName(q->qtype));
                    // Sending third query, and no answers yet; time to begin doubting the source
                    ReconfirmAntecedents(m, &q->qname, q->qnamehash, 0);
                }
            }

            // Mark for sending. (If no active interfaces, then don't even try.)
            q->SendOnAll = (q->SendQNow == mDNSInterfaceMark);
            if (q->SendOnAll)
            {
                q->SendQNow  = !intf ? mDNSNULL : (q->InterfaceID) ? q->InterfaceID : intf->InterfaceID;
                q->LastQTime = m->timenow;
            }

            // If we recorded a duplicate suppression for this question less than half an interval ago,
            // then we consider it recent enough that we don't need to do an identical query ourselves.
            ExpireDupSuppressInfo(q->DupSuppress, m->timenow - q->ThisQInterval/2);

            q->LastQTxTime      = m->timenow;
            q->RecentAnswerPkts = 0;
            if (q->RequestUnicast) q->RequestUnicast--;
        }
        // For all questions (not just the ones we're sending) check what the next scheduled event will be
        // We don't need to consider NewQuestions here because for those we'll set m->NextScheduledQuery in AnswerNewQuestion
        SetNextQueryTime(m,q);
    }

    // 2. Scan our authoritative RR list to see what probes we might need to send

    m->NextScheduledProbe = m->timenow + 0x78000000;

    if (m->CurrentRecord)
        LogMsg("SendQueries ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = m->ResourceRecords;
    while (m->CurrentRecord)
    {
        ar = m->CurrentRecord;
        m->CurrentRecord = ar->next;
        if (!AuthRecord_uDNS(ar) && ar->resrec.RecordType == kDNSRecordTypeUnique)  // For all records that are still probing...
        {
            // 1. If it's not reached its probe time, just make sure we update m->NextScheduledProbe correctly
            if (m->timenow - (ar->LastAPTime + ar->ThisAPInterval) < 0)
            {
                SetNextAnnounceProbeTime(m, ar);
            }
            // 2. else, if it has reached its probe time, mark it for sending and then update m->NextScheduledProbe correctly
            else if (ar->ProbeCount)
            {
                if (ar->AddressProxy.type == mDNSAddrType_IPv4)
                {
                    LogSPS("SendQueries ARP Probe %d %s %s", ar->ProbeCount, InterfaceNameForID(m, ar->resrec.InterfaceID), ARDisplayString(m,ar));
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    SendARP(m, 1, ar, &zerov4Addr, &zeroEthAddr, &ar->AddressProxy.ip.v4, &ar->WakeUp.IMAC);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                }
                else if (ar->AddressProxy.type == mDNSAddrType_IPv6)
                {
                    LogSPS("SendQueries NDP Probe %d %s %s", ar->ProbeCount, InterfaceNameForID(m, ar->resrec.InterfaceID), ARDisplayString(m,ar));
                    // IPv6 source = zero
                    // No target hardware address
                    // IPv6 target address is address we're probing
                    // Ethernet destination address is Ethernet interface address of the Sleep Proxy client we're probing
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    SendNDP(m, NDP_Sol, 0, ar, &zerov6Addr, mDNSNULL, &ar->AddressProxy.ip.v6, &ar->WakeUp.IMAC);
                }
                // Mark for sending. (If no active interfaces, then don't even try.)
                ar->SendRNow   = (!intf || ar->WakeUp.HMAC.l[0]) ? mDNSNULL : ar->resrec.InterfaceID ? ar->resrec.InterfaceID : intf->InterfaceID;
#else
                }
                ar->SendRNow   = (!intf) ? mDNSNULL : ar->resrec.InterfaceID ? ar->resrec.InterfaceID : intf->InterfaceID;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                ar->LastAPTime = m->timenow;
                // When we have a late conflict that resets a record to probing state we use a special marker value greater
                // than DefaultProbeCountForTypeUnique. Here we detect that state and reset ar->ProbeCount back to the right value.
                if (ar->ProbeCount > DefaultProbeCountForTypeUnique)
                    ar->ProbeCount = DefaultProbeCountForTypeUnique;
                ar->ProbeCount--;
                SetNextAnnounceProbeTime(m, ar);
                if (ar->ProbeCount == 0)
                {
                    // If this is the last probe for this record, then see if we have any matching records
                    // on our duplicate list which should similarly have their ProbeCount cleared to zero...
                    AuthRecord *r2;
                    for (r2 = m->DuplicateRecords; r2; r2=r2->next)
                        if (r2->resrec.RecordType == kDNSRecordTypeUnique && RecordIsLocalDuplicate(r2, ar))
                            r2->ProbeCount = 0;
                    // ... then acknowledge this record to the client.
                    // We do this optimistically, just as we're about to send the third probe.
                    // This helps clients that both advertise and browse, and want to filter themselves
                    // from the browse results list, because it helps ensure that the registration
                    // confirmation will be delivered 1/4 second *before* the browse "add" event.
                    // A potential downside is that we could deliver a registration confirmation and then find out
                    // moments later that there's a name conflict, but applications have to be prepared to handle
                    // late conflicts anyway (e.g. on connection of network cable, etc.), so this is nothing new.
                    if (!ar->Acknowledged) AcknowledgeRecord(m, ar);
                }
            }
            // else, if it has now finished probing, move it to state Verified,
            // and update m->NextScheduledResponse so it will be announced
            else
            {
                if (!ar->Acknowledged) AcknowledgeRecord(m, ar);    // Defensive, just in case it got missed somehow
                ar->resrec.RecordType     = kDNSRecordTypeVerified;
                ar->ThisAPInterval = DefaultAnnounceIntervalForTypeUnique;
                ar->LastAPTime     = m->timenow - DefaultAnnounceIntervalForTypeUnique;
                SetNextAnnounceProbeTime(m, ar);
            }
        }
    }
    m->CurrentRecord = m->DuplicateRecords;
    while (m->CurrentRecord)
    {
        ar = m->CurrentRecord;
        m->CurrentRecord = ar->next;
        if (ar->resrec.RecordType == kDNSRecordTypeUnique && ar->ProbeCount == 0 && !ar->Acknowledged)
            AcknowledgeRecord(m, ar);
    }

    // 3. Now we know which queries and probes we're sending,
    // go through our interface list sending the appropriate queries on each interface
    while (intf)
    {
        int OwnerRecordSpace = (m->AnnounceOwner && intf->MAC.l[0]) ? DNSOpt_Header_Space + DNSOpt_Owner_Space(&m->PrimaryMAC, &intf->MAC) : 0;
#ifndef REMOVE_MDNS_TRACING
        int TraceRecordSpace = (mDNS_McastTracingEnabled && MDNS_TRACER) ? DNSOpt_Header_Space + DNSOpt_TraceData_Space : 0;
#endif //#ifndef REMOVE_MDNS_TRACING  
        mDNSu8 *queryptr = m->omsg.data;
        mDNSBool useBackgroundTrafficClass = mDNSfalse;    // set if we should use background traffic class

        InitializeDNSMessage(&m->omsg.h, zeroID, QueryFlags);
        if (KnownAnswerList) verbosedebugf("SendQueries:   KnownAnswerList set... Will continue from previous packet");
        if (!KnownAnswerList)
        {
            // Start a new known-answer list
            CacheRecord **kalistptr = &KnownAnswerList;
#ifndef REMOVE_MDNS_TRACING  
            mDNSu32 answerforecast = OwnerRecordSpace + TraceRecordSpace;  // Start by assuming we'll need at least enough space to put the Owner+Tracer Option
#else
            mDNSu32 answerforecast = OwnerRecordSpace;  // Start by assuming we'll need at least enough space to put the Owner Option
#endif //#ifndef REMOVE_MDNS_TRACING  
            // Put query questions in this packet
            for (q = m->Questions; q && q != m->NewQuestions; q=q->next)
            {
                if (mDNSOpaque16IsZero(q->TargetQID) && (q->SendQNow == intf->InterfaceID))
                {
                    mDNSBool Suppress = mDNSfalse;
                    debugf("SendQueries: %s question for %##s (%s) at %d forecast total %d",
                           SuppressOnThisInterface(q->DupSuppress, intf) ? "Suppressing" : "Putting    ",
                           q->qname.c, DNSTypeName(q->qtype), queryptr - m->omsg.data, queryptr + answerforecast - m->omsg.data);

                    // If interface is P2P type, verify that query should be sent over it.
                    if (!mDNSPlatformValidQuestionForInterface(q, intf))
                    {
                        LogInfo("SendQueries: Not sending (%s) %##s on %s", DNSTypeName(q->qtype), q->qname.c, InterfaceNameForID(m, intf->InterfaceID));
                        q->SendQNow = (q->InterfaceID || !q->SendOnAll) ? mDNSNULL : GetNextActiveInterfaceID(intf);
                    }
                    // If we're suppressing this question, or we successfully put it, update its SendQNow state
                    else if ((Suppress = SuppressOnThisInterface(q->DupSuppress, intf)) ||
                        BuildQuestion(m, &m->omsg, &queryptr, q, &kalistptr, &answerforecast))
                    {
                        // We successfully added the question to the packet. Make sure that
                        // we also send the NSEC3 record if required. BuildQuestion accounted for
                        // the space.
                        //
                        // Note: We don't suppress anonymous questions and hence Suppress should always
                        // be zero.

                        if (Suppress)
                            m->mDNSStats.DupQuerySuppressions++;

                        if (!Suppress && q->AnonInfo)
                        {
                            debugf("SendQueries: marking for question %##s, Suppress %d", q->qname.c, Suppress);
                            q->AnonInfo->SendNow = intf->InterfaceID;
                        }
                        q->SendQNow = (q->InterfaceID || !q->SendOnAll) ? mDNSNULL : GetNextActiveInterfaceID(intf);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                        if (q->WakeOnResolveCount)
                        {
                            mDNSSendWakeOnResolve(m, q);
                            q->WakeOnResolveCount--;
                        }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                        // use brackground traffic class if any included question requires it
                        if (q->UseBackgroundTrafficClass)
                        {
                            useBackgroundTrafficClass = mDNStrue;
                        }
                    }
                }
            }

            // Put probe questions in this packet
            for (ar = m->ResourceRecords; ar; ar=ar->next)
                if (ar->SendRNow == intf->InterfaceID)
                {
                    mDNSBool ucast = (ar->ProbeCount >= DefaultProbeCountForTypeUnique-1) && m->CanReceiveUnicastOn5353;
                    mDNSu16 ucbit = (mDNSu16)(ucast ? kDNSQClass_UnicastResponse : 0);
                    const mDNSu8 *const limit = m->omsg.data + (m->omsg.h.numQuestions ? NormalMaxDNSMessageData : AbsoluteMaxDNSMessageData);
                    // We forecast: compressed name (2) type (2) class (2) TTL (4) rdlength (2) rdata (n)
                    mDNSu32 forecast = answerforecast + 12 + ar->resrec.rdestimate;
                    mDNSu8 *newptr = putQuestion(&m->omsg, queryptr, limit - forecast, ar->resrec.name, kDNSQType_ANY, (mDNSu16)(ar->resrec.rrclass | ucbit));
                    if (newptr)
                    {
                        queryptr       = newptr;
                        answerforecast = forecast;
                        ar->SendRNow = (ar->resrec.InterfaceID) ? mDNSNULL : GetNextActiveInterfaceID(intf);
                        ar->IncludeInProbe = mDNStrue;
                        verbosedebugf("SendQueries:   Put Question %##s (%s) probecount %d",
                                      ar->resrec.name->c, DNSTypeName(ar->resrec.rrtype), ar->ProbeCount);
                    }
                }
        }

        // Put our known answer list (either new one from this question or questions, or remainder of old one from last time)
        while (KnownAnswerList)
        {
            CacheRecord *ka = KnownAnswerList;
            mDNSu32 SecsSinceRcvd = ((mDNSu32)(m->timenow - ka->TimeRcvd)) / mDNSPlatformOneSecond;
#ifndef REMOVE_MDNS_TRACING  
            mDNSu8 *newptr = PutResourceRecordTTLWithLimit(&m->omsg, queryptr, &m->omsg.h.numAnswers, &ka->resrec, ka->resrec.rroriginalttl - SecsSinceRcvd,
                                                           m->omsg.data + NormalMaxDNSMessageData - OwnerRecordSpace - TraceRecordSpace);
#else
            mDNSu8 *newptr = PutResourceRecordTTLWithLimit(&m->omsg, queryptr, &m->omsg.h.numAnswers, &ka->resrec, ka->resrec.rroriginalttl - SecsSinceRcvd,
                                                           m->omsg.data + NormalMaxDNSMessageData - OwnerRecordSpace);
#endif //#ifndef REMOVE_MDNS_TRACING  
            if (newptr)
            {
                verbosedebugf("SendQueries:   Put %##s (%s) at %d - %d",
                              ka->resrec.name->c, DNSTypeName(ka->resrec.rrtype), queryptr - m->omsg.data, newptr - m->omsg.data);
                queryptr = newptr;
                KnownAnswerList = ka->NextInKAList;
                ka->NextInKAList = mDNSNULL;
            }
            else
            {
                // If we ran out of space and we have more than one question in the packet, that's an error --
                // we shouldn't have put more than one question if there was a risk of us running out of space.
                if (m->omsg.h.numQuestions > 1)
                    LogMsg("SendQueries:   Put %d answers; No more space for known answers", m->omsg.h.numAnswers);
                m->omsg.h.flags.b[0] |= kDNSFlag0_TC;
                break;
            }
        }

        for (ar = m->ResourceRecords; ar; ar=ar->next)
        {
            if (ar->IncludeInProbe)
            {
                mDNSu8 *newptr = PutResourceRecord(&m->omsg, queryptr, &m->omsg.h.numAuthorities, &ar->resrec);
                ar->IncludeInProbe = mDNSfalse;
                if (newptr) queryptr = newptr;
                else LogMsg("SendQueries:   How did we fail to have space for the Update record %s", ARDisplayString(m,ar));
            }
        }

        for (q = m->Questions; q; q = q->next)
        {
            if (q->AnonInfo && q->AnonInfo->SendNow == intf->InterfaceID)
            {
                mDNSu8 *newptr = PutResourceRecord(&m->omsg, queryptr, &m->omsg.h.numAuthorities, q->AnonInfo->nsec3RR);
                if (newptr)
                {
                    debugf("SendQueries: Added NSEC3 record %s on InterfaceID %p", RRDisplayString(m, q->AnonInfo->nsec3RR), intf->InterfaceID);
                    queryptr = newptr;
                }
                else
                {
                    LogMsg("SendQueries: ERROR!! Cannot add NSEC3 record %s on InterfaceID %p", RRDisplayString(m, q->AnonInfo->nsec3RR), intf->InterfaceID);
                }
                q->AnonInfo->SendNow = mDNSNULL;
            }
        }

        if (queryptr > m->omsg.data)
        {
            // If we have data to send, add OWNER/TRACER/OWNER+TRACER option if necessary, then send packet
#ifndef REMOVE_MDNS_TRACING  
            if (OwnerRecordSpace || TraceRecordSpace)
#else
            if (OwnerRecordSpace)
#endif //#ifndef REMOVE_MDNS_TRACING  
            {
                AuthRecord opt;
                mDNS_SetupResourceRecord(&opt, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
                opt.resrec.rrclass    = NormalMaxDNSMessageData;
                opt.resrec.rdlength   = sizeof(rdataOPT); 
                opt.resrec.rdestimate = sizeof(rdataOPT);
#ifndef REMOVE_MDNS_TRACING  
                if (OwnerRecordSpace && TraceRecordSpace)
                {
                    opt.resrec.rdlength   += sizeof(rdataOPT);  // Two options in this OPT record
                    opt.resrec.rdestimate += sizeof(rdataOPT);
                    SetupOwnerOpt(m, intf, &opt.resrec.rdata->u.opt[0]);
                    SetupTracerOpt(m, &opt.resrec.rdata->u.opt[1]);
                }
                else if (OwnerRecordSpace)
                {
#endif //#ifndef REMOVE_MDNS_TRACING  
                    SetupOwnerOpt(m, intf, &opt.resrec.rdata->u.opt[0]);
#ifndef REMOVE_MDNS_TRACING  
                }
                else if (TraceRecordSpace)
                {
                    SetupTracerOpt(m, &opt.resrec.rdata->u.opt[0]);
                }
                LogInfo("SendQueries putting %s %s: %s %s", OwnerRecordSpace ? "OWNER" : "", TraceRecordSpace ? "TRACER" : "", intf->ifname, ARDisplayString(m, &opt));
#endif //#ifndef REMOVE_MDNS_TRACING  
                queryptr = PutResourceRecordTTLWithLimit(&m->omsg, queryptr, &m->omsg.h.numAdditionals,
                                                         &opt.resrec, opt.resrec.rroriginalttl, m->omsg.data + AbsoluteMaxDNSMessageData);
#ifndef REMOVE_MDNS_TRACING  
                if (!queryptr)
                { 
                    LogMsg("SendQueries: How did we fail to have space for %s %s OPT record (%d/%d/%d/%d) %s", OwnerRecordSpace ? "OWNER" : "", TraceRecordSpace ? "TRACER" : "",
                           m->omsg.h.numQuestions, m->omsg.h.numAnswers, m->omsg.h.numAuthorities, m->omsg.h.numAdditionals, ARDisplayString(m, &opt));
                }
                if (queryptr > m->omsg.data + NormalMaxDNSMessageData)
                {
                    if (m->omsg.h.numQuestions != 1 || m->omsg.h.numAnswers != 0 || m->omsg.h.numAuthorities != 1 || m->omsg.h.numAdditionals != 1)
                        LogMsg("SendQueries: Why did we generate oversized packet with %s %s OPT record %p %p %p (%d/%d/%d/%d) %s", OwnerRecordSpace ? "OWNER" : "", 
                                TraceRecordSpace ? "TRACER" : "", m->omsg.data, m->omsg.data + NormalMaxDNSMessageData, queryptr, m->omsg.h.numQuestions, m->omsg.h.numAnswers, 
                                m->omsg.h.numAuthorities, m->omsg.h.numAdditionals, ARDisplayString(m, &opt));
                }
#endif //#ifndef REMOVE_MDNS_TRACING  
            }

            if ((m->omsg.h.flags.b[0] & kDNSFlag0_TC) && m->omsg.h.numQuestions > 1)
                LogMsg("SendQueries: Should not have more than one question (%d) in a truncated packet", m->omsg.h.numQuestions);
            debugf("SendQueries:   Sending %d Question%s %d Answer%s %d Update%s on %p",
                   m->omsg.h.numQuestions,   m->omsg.h.numQuestions   == 1 ? "" : "s",
                   m->omsg.h.numAnswers,     m->omsg.h.numAnswers     == 1 ? "" : "s",
                   m->omsg.h.numAuthorities, m->omsg.h.numAuthorities == 1 ? "" : "s", intf->InterfaceID);
            if (intf->IPv4Available) mDNSSendDNSMessage(m, &m->omsg, queryptr, intf->InterfaceID, mDNSNULL, &AllDNSLinkGroup_v4, MulticastDNSPort, mDNSNULL, mDNSNULL, useBackgroundTrafficClass);
            if (intf->IPv6Available) mDNSSendDNSMessage(m, &m->omsg, queryptr, intf->InterfaceID, mDNSNULL, &AllDNSLinkGroup_v6, MulticastDNSPort, mDNSNULL, mDNSNULL, useBackgroundTrafficClass);
            if (!m->SuppressSending) m->SuppressSending = NonZeroTime(m->timenow + (mDNSPlatformOneSecond+9)/10);
            if (++pktcount >= 1000)
            { LogMsg("SendQueries exceeded loop limit %d: giving up", pktcount); break; }
            // There might be more records left in the known answer list, or more questions to send
            // on this interface, so go around one more time and try again.
        }
        else    // Nothing more to send on this interface; go to next
        {
            const NetworkInterfaceInfo *next = GetFirstActiveInterface(intf->next);
/*            #if MDNS_DEBUGMSGS && 0
            const char *const msg = next ? "SendQueries:   Nothing more on %p; moving to %p" : "SendQueries:   Nothing more on %p";
            debugf(msg, intf, next);
            #endif
 */
	        intf = next;
        }
    }

    // 4. Final housekeeping

    // 4a. Debugging check: Make sure we announced all our records
    for (ar = m->ResourceRecords; ar; ar=ar->next)
        if (ar->SendRNow)
        {
            if (ar->ARType != AuthRecordLocalOnly && ar->ARType != AuthRecordP2P)
                LogMsg("SendQueries: No active interface %p to send probe: %p %s", ar->SendRNow, ar->resrec.InterfaceID, ARDisplayString(m, ar));
            ar->SendRNow = mDNSNULL;
        }

    // 4b. When we have lingering cache records that we're keeping around for a few seconds in the hope
    // that their interface which went away might come back again, the logic will want to send queries
    // for those records, but we can't because their interface isn't here any more, so to keep the
    // state machine ticking over we just pretend we did so.
    // If the interface does not come back in time, the cache record will expire naturally
    FORALL_CACHERECORDS(slot, cg, cr)
    {
        if (cr->CRActiveQuestion && cr->UnansweredQueries < MaxUnansweredQueries)
        {
            if (m->timenow + TicksTTL(cr)/50 - cr->NextRequiredQuery >= 0)
            {
                cr->UnansweredQueries++;
                cr->CRActiveQuestion->SendQNow = mDNSNULL;
                SetNextCacheCheckTimeForRecord(m, cr);
            }
        }
    }

    // 4c. Debugging check: Make sure we sent all our planned questions
    // Do this AFTER the lingering cache records check above, because that will prevent spurious warnings for questions
    // we legitimately couldn't send because the interface is no longer available
    for (q = m->Questions; q; q=q->next)
    {
        if (q->SendQNow)
        {
            DNSQuestion *x;
            for (x = m->NewQuestions; x; x=x->next) if (x == q) break;  // Check if this question is a NewQuestion
            LogMsg("SendQueries: No active interface %p to send %s question: %p %##s (%s)", q->SendQNow, x ? "new" : "old", q->InterfaceID, q->qname.c, DNSTypeName(q->qtype));
            q->SendQNow = mDNSNULL;
        }
        q->CachedAnswerNeedsUpdate = mDNSfalse;
    }
}
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void SendWakeup(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSEthAddr *EthAddr, mDNSOpaque48 *password)
{
    int i, j;
    mDNSu8 *ptr = m->omsg.data;
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, InterfaceID);
    if (!intf) { LogMsg("SendARP: No interface with InterfaceID %p found", InterfaceID); return; }

    // 0x00 Destination address
    for (i=0; i<6; i++) *ptr++ = EthAddr->b[i];

    // 0x06 Source address (Note: Since we don't currently set the BIOCSHDRCMPLT option, BPF will fill in the real interface address for us)
    for (i=0; i<6; i++) *ptr++ = intf->MAC.b[0];

    // 0x0C Ethertype (0x0842)
    *ptr++ = 0x08;
    *ptr++ = 0x42;

    // 0x0E Wakeup sync sequence
    for (i=0; i<6; i++) *ptr++ = 0xFF;

    // 0x14 Wakeup data
    for (j=0; j<16; j++) for (i=0; i<6; i++) *ptr++ = EthAddr->b[i];

    // 0x74 Password
    for (i=0; i<6; i++) *ptr++ = password->b[i];

    mDNSPlatformSendRawPacket(m->omsg.data, ptr, InterfaceID);

    // For Ethernet switches that don't flood-foward packets with unknown unicast destination MAC addresses,
    // broadcast is the only reliable way to get a wakeup packet to the intended target machine.
    // For 802.11 WPA networks, where a sleeping target machine may have missed a broadcast/multicast
    // key rotation, unicast is the only way to get a wakeup packet to the intended target machine.
    // So, we send one of each, unicast first, then broadcast second.
    for (i=0; i<6; i++) m->omsg.data[i] = 0xFF;
    mDNSPlatformSendRawPacket(m->omsg.data, ptr, InterfaceID);
}
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - RR List Management & Task Management
#endif

// Whenever a question is answered, reset its state so that we don't query
// the network repeatedly. This happens first time when we answer the question and
// and later when we refresh the cache.
mDNSlocal void ResetQuestionState(mDNS *const m, DNSQuestion *q)
{
    q->LastQTime        = m->timenow;
    q->LastQTxTime      = m->timenow;
    q->RecentAnswerPkts = 0;
    q->ThisQInterval    = MaxQuestionInterval;
    q->RequestUnicast   = 0;
    // Reset unansweredQueries so that we don't penalize this server later when we
    // start sending queries when the cache expires.
    q->unansweredQueries = 0;
    debugf("ResetQuestionState: Set MaxQuestionInterval for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
}

// Note: AnswerCurrentQuestionWithResourceRecord can call a user callback, which may change the record list and/or question list.
// Any code walking either list must use the m->CurrentQuestion (and possibly m->CurrentRecord) mechanism to protect against this.
// In fact, to enforce this, the routine will *only* answer the question currently pointed to by m->CurrentQuestion,
// which will be auto-advanced (possibly to NULL) if the client callback cancels the question.
mDNSexport void AnswerCurrentQuestionWithResourceRecord(mDNS *const m, CacheRecord *const rr, const QC_result AddRecord)
{
    DNSQuestion *const q = m->CurrentQuestion;
    mDNSBool followcname = FollowCNAME(q, &rr->resrec, AddRecord);

    verbosedebugf("AnswerCurrentQuestionWithResourceRecord:%4lu %s TTL %d %s",
                  q->CurrentAnswers, AddRecord ? "Add" : "Rmv", rr->resrec.rroriginalttl, CRDisplayString(m, rr));

    // When the response for the question was validated, the entire rrset was validated. If we deliver
    // a RMV for a single record in the rrset, we invalidate the response. If we deliver another add
    // in the future, we will do the revalidation again.
    //
    // Also, if we deliver an ADD for a negative cache record and it has no NSEC/NSEC3, the ValidationStatus needs
    // to be reset. This happens normally when we deliver a "secure" negative response followed by an insecure
    // negative response which can happen e.g., when disconnecting from network that leads to a negative response
    // due to no DNS servers. As we don't deliver RMVs for negative responses that were delivered before, we need
    // to do it on the next ADD of a negative cache record. This ADD could be the result of a timeout, no DNS servers
    // etc. in which case we need to reset the state to make sure we don't deliver them as secure. If this is
    // a real negative response, we would reset the state here and validate the results at the end of this function.
    // or the real response again if we purge the cache. 
    if (q->ValidationRequired && ((AddRecord == QC_rmv) ||
        (rr->resrec.RecordType == kDNSRecordTypePacketNegative && (AddRecord == QC_add))))
    {
        q->ValidationStatus = 0;
        q->ValidationState = DNSSECValRequired;
    }

    // Normally we don't send out the unicast query if we have answered using our local only auth records e.g., /etc/hosts.
    // But if the query for "A" record has a local answer but query for "AAAA" record has no local answer, we might
    // send the AAAA query out which will come back with CNAME and will also answer the "A" query. To prevent that,
    // we check to see if that query already has a unique local answer.
    if (q->LOAddressAnswers)
    {
        LogInfo("AnswerCurrentQuestionWithResourceRecord: Question %p %##s (%s) not answering with record %s due to "
                "LOAddressAnswers %d", q, q->qname.c, DNSTypeName(q->qtype), ARDisplayString(m, rr),
                q->LOAddressAnswers);
        return;
    }

    if (QuerySuppressed(q))
    {
        // If the query is suppressed, then we don't want to answer from the cache. But if this query is
        // supposed to time out, we still want to callback the clients. We do this only for TimeoutQuestions
        // that are timing out, which we know are answered with negative cache record when timing out.
        if (!q->TimeoutQuestion || rr->resrec.RecordType != kDNSRecordTypePacketNegative || (m->timenow - q->StopTime < 0))
            return;
    }

    // Note: Use caution here. In the case of records with rr->DelayDelivery set, AnswerCurrentQuestionWithResourceRecord(... mDNStrue)
    // may be called twice, once when the record is received, and again when it's time to notify local clients.
    // If any counters or similar are added here, care must be taken to ensure that they are not double-incremented by this.

    rr->LastUsed = m->timenow;
    if (AddRecord == QC_add && !q->DuplicateOf && rr->CRActiveQuestion != q)
    {
        if (!rr->CRActiveQuestion) m->rrcache_active++; // If not previously active, increment rrcache_active count
        debugf("AnswerCurrentQuestionWithResourceRecord: Updating CRActiveQuestion from %p to %p for cache record %s, CurrentAnswer %d",
               rr->CRActiveQuestion, q, CRDisplayString(m,rr), q->CurrentAnswers);
        rr->CRActiveQuestion = q;                       // We know q is non-null
        SetNextCacheCheckTimeForRecord(m, rr);
    }

    // If this is:
    // (a) a no-cache add, where we've already done at least one 'QM' query, or
    // (b) a normal add, where we have at least one unique-type answer,
    // then there's no need to keep polling the network.
    // (If we have an answer in the cache, then we'll automatically ask again in time to stop it expiring.)
    // We do this for mDNS questions and uDNS one-shot questions, but not for
    // uDNS LongLived questions, because that would mess up our LLQ lease renewal timing.
    if ((AddRecord == QC_addnocache && !q->RequestUnicast) ||
        (AddRecord == QC_add && (q->ExpectUnique || (rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask))))
        if (ActiveQuestion(q) && (mDNSOpaque16IsZero(q->TargetQID) || !q->LongLived))
        {
            ResetQuestionState(m, q);
        }

    if (rr->DelayDelivery) return;      // We'll come back later when CacheRecordDeferredAdd() calls us

    // Only deliver negative answers if client has explicitly requested them except when we are forcing a negative response
    // for the purpose of retrying search domains/timeout OR the question is suppressed
    if (rr->resrec.RecordType == kDNSRecordTypePacketNegative || (q->qtype != kDNSType_NSEC && RRAssertsNonexistence(&rr->resrec, q->qtype)))
        if (!AddRecord || (AddRecord != QC_suppressed && AddRecord != QC_forceresponse && !q->ReturnIntermed)) return;

    // For CNAME results to non-CNAME questions, only inform the client if they explicitly requested that
    if (q->QuestionCallback && !q->NoAnswer && (!followcname || q->ReturnIntermed))
    {
        mDNS_DropLockBeforeCallback();      // Allow client (and us) to legally make mDNS API calls
        if (q->qtype != kDNSType_NSEC && RRAssertsNonexistence(&rr->resrec, q->qtype))
        {
            CacheRecord neg;
            MakeNegativeCacheRecord(m, &neg, &q->qname, q->qnamehash, q->qtype, q->qclass, 1, rr->resrec.InterfaceID, q->qDNSServer);
            q->QuestionCallback(m, q, &neg.resrec, AddRecord);
        }
        else
            q->QuestionCallback(m, q, &rr->resrec, AddRecord);
        mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
    }
    // If this is an "Add" operation and this question needs validation, validate the response.
    // In the case of negative responses, extra care should be taken. Negative cache records are
    // used for many purposes. For example,
    //
    // 1) Suppressing questions (SuppressUnusable)
    // 2) Timeout questions
    // 3) The name does not exist
    // 4) No DNS servers are available and we need a quick response for the application
    //
    // (1) and (2) are handled by "QC_add" check as AddRecord would be "QC_forceresponse" or "QC_suppressed"
    // in that case. For (3), it is possible that we don't get nsecs back but we still need to call
    // VerifySignature so that we can deliver the appropriate DNSSEC result. There is no point in verifying
    // signature for (4) and hence the explicit check for q->qDNSServer.
    //
    if (m->CurrentQuestion == q && (AddRecord == QC_add) && !q->ValidatingResponse && q->ValidationRequired &&
        q->ValidationState == DNSSECValRequired && q->qDNSServer)
    {
        q->ValidationState = DNSSECValInProgress;
        // Treat it as callback call as that's what dnssec code expects
        mDNS_DropLockBeforeCallback();      // Allow client (and us) to legally make mDNS API calls
        VerifySignature(m, mDNSNULL, q);
        mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
        return;
    }

    // Note: Proceed with caution here because client callback function is allowed to do anything,
    // including starting/stopping queries, registering/deregistering records, etc.
    //
    // If we get a CNAME back while we are validating the response (i.e., CNAME for DS, DNSKEY, RRSIG),
    // don't follow them. If it is a ValidationRequired question, wait for the CNAME to be validated
    // first before following it
    if (!ValidatingQuestion(q) && followcname && m->CurrentQuestion == q)
        AnswerQuestionByFollowingCNAME(m, q, &rr->resrec);
}

mDNSlocal void CacheRecordDeferredAdd(mDNS *const m, CacheRecord *rr)
{
    rr->DelayDelivery = 0;
    if (m->CurrentQuestion)
        LogMsg("CacheRecordDeferredAdd ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    while (m->CurrentQuestion && m->CurrentQuestion != m->NewQuestions)
    {
        DNSQuestion *q = m->CurrentQuestion;
        if (ResourceRecordAnswersQuestion(&rr->resrec, q))
            AnswerCurrentQuestionWithResourceRecord(m, rr, QC_add);
        if (m->CurrentQuestion == q)    // If m->CurrentQuestion was not auto-advanced, do it ourselves now
            m->CurrentQuestion = q->next;
    }
    m->CurrentQuestion = mDNSNULL;
}

mDNSlocal mDNSs32 CheckForSoonToExpireRecords(mDNS *const m, const domainname *const name, const mDNSu32 namehash, const mDNSu32 slot, mDNSBool *purge)
{
    const mDNSs32 threshhold = m->timenow + mDNSPlatformOneSecond;  // See if there are any records expiring within one second
    const mDNSs32 start      = m->timenow - 0x10000000;
    mDNSs32 delay = start;
    CacheGroup *cg = CacheGroupForName(m, slot, namehash, name);
    const CacheRecord *rr;

    if (purge)
        *purge = mDNSfalse;
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
    {
        // If there are records that will expire soon, there are cases that need delayed
        // delivery of events:
        //
        // 1) A new cache entry is about to be added as a replacement. The caller needs to
        //    deliver a RMV (for the current old entry) followed by ADD (for the new entry).
        //    It needs to schedule the timer for the next cache expiry (ScheduleNextCacheCheckTime),
        //    so that the cache entry can be purged (purging causes the RMV followed by ADD)
        //   
        // 2) A new question is about to be answered and the caller needs to know whether it's
        //    scheduling should be delayed so that the question is not answered with this record.
        //    Instead of delivering an ADD (old entry) followed by RMV (old entry) and another ADD
        //    (new entry), a single ADD can be delivered by delaying the scheduling of the question
        //    immediately.
        //
        // When the unicast cache record is created, it's TTL has been extended beyond its value
        // given in the resource record (See RRAdjustTTL). If it is in the "extended" time, the
        // cache is already expired and we set "purge" to indicate that. When "purge" is set, the
        // return value of the function should be ignored by the callers.
        //
        // Note: For case (1), "purge" argument is NULL and hence the following checks are skipped.
        // It is okay to skip in that case because the cache records have been set to expire almost
        // immediately and the extended time does not apply.
        //
        // Also, if there is already an active question we don't try to optimize as purging the cache
        // would end up delivering RMV for the active question and hence we avoid that.

        if (purge && !rr->resrec.InterfaceID && !rr->CRActiveQuestion && rr->resrec.rroriginalttl)
        {
            mDNSu32 uTTL = RRUnadjustedTTL(rr->resrec.rroriginalttl);
            if (m->timenow - (rr->TimeRcvd + ((mDNSs32)uTTL * mDNSPlatformOneSecond)) >= 0)
            {
                LogInfo("CheckForSoonToExpireRecords: %s: rroriginalttl %u, unadjustedTTL %u, currentTTL %u",
                    CRDisplayString(m, rr), rr->resrec.rroriginalttl, uTTL, (m->timenow - rr->TimeRcvd)/mDNSPlatformOneSecond);
                *purge = mDNStrue;
                continue;
            }
        }
        if (threshhold - RRExpireTime(rr) >= 0)     // If we have records about to expire within a second
        {
            if (delay - RRExpireTime(rr) < 0)       // then delay until after they've been deleted
                delay = RRExpireTime(rr);
        }
    }
    if (delay - start > 0)
        return(NonZeroTime(delay));
    else
        return(0);
}

// CacheRecordAdd is only called from CreateNewCacheEntry, *never* directly as a result of a client API call.
// If new questions are created as a result of invoking client callbacks, they will be added to
// the end of the question list, and m->NewQuestions will be set to indicate the first new question.
// rr is a new CacheRecord just received into our cache
// (kDNSRecordTypePacketAns/PacketAnsUnique/PacketAdd/PacketAddUnique).
// Note: CacheRecordAdd calls AnswerCurrentQuestionWithResourceRecord which can call a user callback,
// which may change the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSlocal void CacheRecordAdd(mDNS *const m, CacheRecord *rr)
{
    DNSQuestion *q;

    // We stop when we get to NewQuestions -- if we increment their CurrentAnswers/LargeAnswers/UniqueAnswers
    // counters here we'll end up double-incrementing them when we do it again in AnswerNewQuestion().
    for (q = m->Questions; q && q != m->NewQuestions; q=q->next)
    {
        if (ResourceRecordAnswersQuestion(&rr->resrec, q))
        {
            // If this question is one that's actively sending queries, and it's received ten answers within one
            // second of sending the last query packet, then that indicates some radical network topology change,
            // so reset its exponential backoff back to the start. We must be at least at the eight-second interval
            // to do this. If we're at the four-second interval, or less, there's not much benefit accelerating
            // because we will anyway send another query within a few seconds. The first reset query is sent out
            // randomized over the next four seconds to reduce possible synchronization between machines.
            if (q->LastAnswerPktNum != m->PktNum)
            {
                q->LastAnswerPktNum = m->PktNum;
                if (mDNSOpaque16IsZero(q->TargetQID) && ActiveQuestion(q) && ++q->RecentAnswerPkts >= 10 &&
                    q->ThisQInterval > InitialQuestionInterval * QuestionIntervalStep3 && m->timenow - q->LastQTxTime < mDNSPlatformOneSecond)
                {
                    LogMsg("CacheRecordAdd: %##s (%s) got immediate answer burst (%d); restarting exponential backoff sequence (%d)",
                           q->qname.c, DNSTypeName(q->qtype), q->RecentAnswerPkts, q->ThisQInterval);
                    q->LastQTime      = m->timenow - InitialQuestionInterval + (mDNSs32)mDNSRandom((mDNSu32)mDNSPlatformOneSecond*4);
                    q->ThisQInterval  = InitialQuestionInterval;
                    SetNextQueryTime(m,q);
                }
            }
            verbosedebugf("CacheRecordAdd %p %##s (%s) %lu %#a:%d question %p", rr, rr->resrec.name->c,
                          DNSTypeName(rr->resrec.rrtype), rr->resrec.rroriginalttl, rr->resrec.rDNSServer ?
                          &rr->resrec.rDNSServer->addr : mDNSNULL, mDNSVal16(rr->resrec.rDNSServer ?
                                                                             rr->resrec.rDNSServer->port : zeroIPPort), q);
            q->CurrentAnswers++;

            q->unansweredQueries = 0;
            if (rr->resrec.rdlength > SmallRecordLimit) q->LargeAnswers++;
            if (rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask) q->UniqueAnswers++;
            if (q->CurrentAnswers > 4000)
            {
                static int msgcount = 0;
                if (msgcount++ < 10)
                    LogMsg("CacheRecordAdd: %##s (%s) has %d answers; shedding records to resist DOS attack",
                           q->qname.c, DNSTypeName(q->qtype), q->CurrentAnswers);
                rr->resrec.rroriginalttl = 0;
                rr->UnansweredQueries = MaxUnansweredQueries;
            }
        }
    }

    if (!rr->DelayDelivery)
    {
        if (m->CurrentQuestion)
            LogMsg("CacheRecordAdd ERROR m->CurrentQuestion already set: %##s (%s)", m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
        m->CurrentQuestion = m->Questions;
        while (m->CurrentQuestion && m->CurrentQuestion != m->NewQuestions)
        {
            q = m->CurrentQuestion;
            if (ResourceRecordAnswersQuestion(&rr->resrec, q))
                AnswerCurrentQuestionWithResourceRecord(m, rr, QC_add);
            if (m->CurrentQuestion == q)    // If m->CurrentQuestion was not auto-advanced, do it ourselves now
                m->CurrentQuestion = q->next;
        }
        m->CurrentQuestion = mDNSNULL;
    }

    SetNextCacheCheckTimeForRecord(m, rr);
}

// NoCacheAnswer is only called from mDNSCoreReceiveResponse, *never* directly as a result of a client API call.
// If new questions are created as a result of invoking client callbacks, they will be added to
// the end of the question list, and m->NewQuestions will be set to indicate the first new question.
// rr is a new CacheRecord just received from the wire (kDNSRecordTypePacketAns/AnsUnique/Add/AddUnique)
// but we don't have any place to cache it. We'll deliver question 'add' events now, but we won't have any
// way to deliver 'remove' events in future, nor will we be able to include this in known-answer lists,
// so we immediately bump ThisQInterval up to MaxQuestionInterval to avoid pounding the network.
// Note: NoCacheAnswer calls AnswerCurrentQuestionWithResourceRecord which can call a user callback,
// which may change the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSlocal void NoCacheAnswer(mDNS *const m, CacheRecord *rr)
{
    LogMsg("No cache space: Delivering non-cached result for %##s", m->rec.r.resrec.name->c);
    if (m->CurrentQuestion)
        LogMsg("NoCacheAnswer ERROR m->CurrentQuestion already set: %##s (%s)", m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    // We do this for *all* questions, not stopping when we get to m->NewQuestions,
    // since we're not caching the record and we'll get no opportunity to do this later
    while (m->CurrentQuestion)
    {
        DNSQuestion *q = m->CurrentQuestion;
        if (ResourceRecordAnswersQuestion(&rr->resrec, q))
            AnswerCurrentQuestionWithResourceRecord(m, rr, QC_addnocache);  // QC_addnocache means "don't expect remove events for this"
        if (m->CurrentQuestion == q)    // If m->CurrentQuestion was not auto-advanced, do it ourselves now
            m->CurrentQuestion = q->next;
    }
    m->CurrentQuestion = mDNSNULL;
}

// CacheRecordRmv is only called from CheckCacheExpiration, which is called from mDNS_Execute.
// Note that CacheRecordRmv is *only* called for records that are referenced by at least one active question.
// If new questions are created as a result of invoking client callbacks, they will be added to
// the end of the question list, and m->NewQuestions will be set to indicate the first new question.
// rr is an existing cache CacheRecord that just expired and is being deleted
// (kDNSRecordTypePacketAns/PacketAnsUnique/PacketAdd/PacketAddUnique).
// Note: CacheRecordRmv calls AnswerCurrentQuestionWithResourceRecord which can call a user callback,
// which may change the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSlocal void CacheRecordRmv(mDNS *const m, CacheRecord *rr)
{
    if (m->CurrentQuestion)
        LogMsg("CacheRecordRmv ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;

    // We stop when we get to NewQuestions -- for new questions their CurrentAnswers/LargeAnswers/UniqueAnswers counters
    // will all still be zero because we haven't yet gone through the cache counting how many answers we have for them.
    while (m->CurrentQuestion && m->CurrentQuestion != m->NewQuestions)
    {
        DNSQuestion *q = m->CurrentQuestion;
        // When a question enters suppressed state, we generate RMV events and generate a negative
        // response. A cache may be present that answers this question e.g., cache entry generated
        // before the question became suppressed. We need to skip the suppressed questions here as
        // the RMV event has already been generated.
        if (!QuerySuppressed(q) && ResourceRecordAnswersQuestion(&rr->resrec, q))
        {
            verbosedebugf("CacheRecordRmv %p %s", rr, CRDisplayString(m, rr));
            q->FlappingInterface1 = mDNSNULL;
            q->FlappingInterface2 = mDNSNULL;

            if (q->CurrentAnswers == 0)
                LogMsg("CacheRecordRmv ERROR!!: How can CurrentAnswers already be zero for %p %##s (%s) DNSServer %#a:%d",
                       q, q->qname.c, DNSTypeName(q->qtype), q->qDNSServer ? &q->qDNSServer->addr : mDNSNULL,
                       mDNSVal16(q->qDNSServer ? q->qDNSServer->port : zeroIPPort));
            else
            {
                q->CurrentAnswers--;
                if (rr->resrec.rdlength > SmallRecordLimit) q->LargeAnswers--;
                if (rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask) q->UniqueAnswers--;
            }

            // If we have dropped below the answer threshold for this mDNS question,
            // restart the queries at InitialQuestionInterval.
            if (mDNSOpaque16IsZero(q->TargetQID) && (q->BrowseThreshold > 0) && (q->CurrentAnswers < q->BrowseThreshold))
            {
                q->ThisQInterval = InitialQuestionInterval;
                q->LastQTime     = m->timenow - q->ThisQInterval;
                SetNextQueryTime(m,q);
                LogInfo("CacheRecordRmv: (%s) %##s dropped below threshold of %d answers",
                    DNSTypeName(q->qtype), q->qname.c, q->BrowseThreshold);
            }
            if (rr->resrec.rdata->MaxRDLength) // Never generate "remove" events for negative results
            {
                if (q->CurrentAnswers == 0)
                {
                    LogInfo("CacheRecordRmv: Last answer for %##s (%s) expired from cache; will reconfirm antecedents",
                            q->qname.c, DNSTypeName(q->qtype));
                    ReconfirmAntecedents(m, &q->qname, q->qnamehash, 0);
                }
                AnswerCurrentQuestionWithResourceRecord(m, rr, QC_rmv);
            }
        }
        if (m->CurrentQuestion == q)    // If m->CurrentQuestion was not auto-advanced, do it ourselves now
            m->CurrentQuestion = q->next;
    }
    m->CurrentQuestion = mDNSNULL;
}

mDNSlocal void ReleaseCacheEntity(mDNS *const m, CacheEntity *e)
{
#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING >= 1
    unsigned int i;
    for (i=0; i<sizeof(*e); i++) ((char*)e)[i] = 0xFF;
#endif
    e->next = m->rrcache_free;
    m->rrcache_free = e;
    m->rrcache_totalused--;
}

mDNSlocal void ReleaseCacheGroup(mDNS *const m, CacheGroup **cp)
{
    CacheEntity *e = (CacheEntity *)(*cp);
    //LogMsg("ReleaseCacheGroup:  Releasing CacheGroup for %p, %##s", (*cp)->name->c, (*cp)->name->c);
    if ((*cp)->rrcache_tail != &(*cp)->members)
        LogMsg("ERROR: (*cp)->members == mDNSNULL but (*cp)->rrcache_tail != &(*cp)->members)");
    //if ((*cp)->name != (domainname*)((*cp)->namestorage))
    //	LogMsg("ReleaseCacheGroup: %##s, %p %p", (*cp)->name->c, (*cp)->name, (domainname*)((*cp)->namestorage));
    if ((*cp)->name != (domainname*)((*cp)->namestorage)) mDNSPlatformMemFree((*cp)->name);
    (*cp)->name = mDNSNULL;
    *cp = (*cp)->next;          // Cut record from list
    ReleaseCacheEntity(m, e);
}

mDNSlocal void ReleaseAdditionalCacheRecords(mDNS *const m, CacheRecord **rp)
{
    while (*rp)
    {
        CacheRecord *rr = *rp;
        *rp = (*rp)->next;          // Cut record from list
        if (rr->resrec.rdata && rr->resrec.rdata != (RData*)&rr->smallrdatastorage)
        {
            mDNSPlatformMemFree(rr->resrec.rdata);
            rr->resrec.rdata = mDNSNULL;
        }
        // NSEC or SOA records that are not added to the CacheGroup do not share the name
        // of the CacheGroup.
        if (rr->resrec.name)
        {
            debugf("ReleaseAdditionalCacheRecords: freeing cached record %##s (%s)", rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
            mDNSPlatformMemFree((void *)rr->resrec.name);
            rr->resrec.name = mDNSNULL;
        }
        // Don't count the NSEC3 records used by anonymous browse/reg
        if (!rr->resrec.InterfaceID)
        {
            m->rrcache_totalused_unicast -= rr->resrec.rdlength;
            if (DNSSECRecordType(rr->resrec.rrtype))
                BumpDNSSECStats(m, kStatsActionDecrement, kStatsTypeMemoryUsage, rr->resrec.rdlength);
        }
        ReleaseCacheEntity(m, (CacheEntity *)rr);
    }
}

mDNSexport void ReleaseCacheRecord(mDNS *const m, CacheRecord *r)
{
    CacheGroup *cg;
    const mDNSu32 slot = HashSlot(r->resrec.name);

    //LogMsg("ReleaseCacheRecord: Releasing %s", CRDisplayString(m, r));
    if (r->resrec.rdata && r->resrec.rdata != (RData*)&r->smallrdatastorage) mDNSPlatformMemFree(r->resrec.rdata);
    r->resrec.rdata = mDNSNULL;

    cg = CacheGroupForRecord(m, slot, &r->resrec);
    
    if (!cg)
    {
        // It is okay to have this printed for NSEC/NSEC3s
        LogInfo("ReleaseCacheRecord: ERROR!! cg NULL for %##s (%s)", r->resrec.name->c, DNSTypeName(r->resrec.rrtype));
    }
    // When NSEC records are not added to the cache, it is usually cached at the "nsec" list
    // of the CacheRecord. But sometimes they may be freed without adding to the "nsec" list
    // (which is handled below) and in that case it should be freed here.
    if (r->resrec.name && cg && r->resrec.name != cg->name)
    {
        debugf("ReleaseCacheRecord: freeing %##s (%s)", r->resrec.name->c, DNSTypeName(r->resrec.rrtype));
        mDNSPlatformMemFree((void *)r->resrec.name);
    }
    r->resrec.name = mDNSNULL;

    if (r->resrec.AnonInfo)
    {
        debugf("ReleaseCacheRecord: freeing AnonInfo for %##s (%s)", r->resrec.name->c, DNSTypeName(r->resrec.rrtype));
        FreeAnonInfo((void *)r->resrec.AnonInfo);
    }
    r->resrec.AnonInfo = mDNSNULL;

    if (!r->resrec.InterfaceID)
    {
        m->rrcache_totalused_unicast -= r->resrec.rdlength;
        if (DNSSECRecordType(r->resrec.rrtype))
            BumpDNSSECStats(m, kStatsActionDecrement, kStatsTypeMemoryUsage, r->resrec.rdlength);
    }

    ReleaseAdditionalCacheRecords(m, &r->nsec);
    ReleaseAdditionalCacheRecords(m, &r->soa);

    ReleaseCacheEntity(m, (CacheEntity *)r);
}

// Note: We want to be careful that we deliver all the CacheRecordRmv calls before delivering
// CacheRecordDeferredAdd calls. The in-order nature of the cache lists ensures that all
// callbacks for old records are delivered before callbacks for newer records.
mDNSlocal void CheckCacheExpiration(mDNS *const m, const mDNSu32 slot, CacheGroup *const cg)
{
    CacheRecord **rp = &cg->members;

    if (m->lock_rrcache) { LogMsg("CheckCacheExpiration ERROR! Cache already locked!"); return; }
    m->lock_rrcache = 1;

    while (*rp)
    {
        CacheRecord *const rr = *rp;
        mDNSs32 event = RRExpireTime(rr);
        if (m->timenow - event >= 0)    // If expired, delete it
        {
            *rp = rr->next;             // Cut it from the list
            
            verbosedebugf("CheckCacheExpiration: Deleting%7d %7d %p %s",
                          m->timenow - rr->TimeRcvd, rr->resrec.rroriginalttl, rr->CRActiveQuestion, CRDisplayString(m, rr));
            if (rr->CRActiveQuestion)   // If this record has one or more active questions, tell them it's going away
            {
                DNSQuestion *q = rr->CRActiveQuestion;
                // When a cache record is about to expire, we expect to do four queries at 80-82%, 85-87%, 90-92% and
                // then 95-97% of the TTL. If the DNS server does not respond, then we will remove the cache entry
                // before we pick a new DNS server. As the question interval is set to MaxQuestionInterval, we may
                // not send out a query anytime soon. Hence, we need to reset the question interval. If this is
                // a normal deferred ADD case, then AnswerCurrentQuestionWithResourceRecord will reset it to
                // MaxQuestionInterval. If we have inactive questions referring to negative cache entries,
                // don't ressurect them as they will deliver duplicate "No such Record" ADD events
                if (!mDNSOpaque16IsZero(q->TargetQID) && !q->LongLived && ActiveQuestion(q))
                {
                    q->ThisQInterval = InitialQuestionInterval;
                    q->LastQTime     = m->timenow - q->ThisQInterval;
                    SetNextQueryTime(m, q);
                }
                CacheRecordRmv(m, rr);
                m->rrcache_active--;
            }
            ReleaseCacheRecord(m, rr);
        }
        else                            // else, not expired; see if we need to query
        {
            // If waiting to delay delivery, do nothing until then
            if (rr->DelayDelivery && rr->DelayDelivery - m->timenow > 0)
                event = rr->DelayDelivery;
            else
            {
                if (rr->DelayDelivery) CacheRecordDeferredAdd(m, rr);
                if (rr->CRActiveQuestion && rr->UnansweredQueries < MaxUnansweredQueries)
                {
                    if (m->timenow - rr->NextRequiredQuery < 0)     // If not yet time for next query
                        event = NextCacheCheckEvent(rr);            // then just record when we want the next query
                    else                                            // else trigger our question to go out now
                    {
                        // Set NextScheduledQuery to timenow so that SendQueries() will run.
                        // SendQueries() will see that we have records close to expiration, and send FEQs for them.
                        m->NextScheduledQuery = m->timenow;
                        // After sending the query we'll increment UnansweredQueries and call SetNextCacheCheckTimeForRecord(),
                        // which will correctly update m->NextCacheCheck for us.
                        event = m->timenow + 0x3FFFFFFF;
                    }
                }
            }
            verbosedebugf("CheckCacheExpiration:%6d %5d %s",
                          (event - m->timenow) / mDNSPlatformOneSecond, CacheCheckGracePeriod(rr), CRDisplayString(m, rr));
            if (m->rrcache_nextcheck[slot] - event > 0)
                m->rrcache_nextcheck[slot] = event;
            rp = &rr->next;
        }
    }
    if (cg->rrcache_tail != rp) verbosedebugf("CheckCacheExpiration: Updating CacheGroup tail from %p to %p", cg->rrcache_tail, rp);
    cg->rrcache_tail = rp;
    m->lock_rrcache = 0;
}

// "LORecord" includes both LocalOnly and P2P record. This function assumes m->CurrentQuestion is pointing to "q".
//
// If "CheckOnly" is set to "true", the question won't be answered but just check to see if there is an answer and
// returns true if there is an answer.
//
// If "CheckOnly" is set to "false", the question will be answered if there is a LocalOnly/P2P record and 
// returns true to indicate the same.
mDNSlocal mDNSBool AnswerQuestionWithLORecord(mDNS *const m, DNSQuestion *q, mDNSBool checkOnly)
{
    mDNSu32 slot;
    AuthRecord *lr;
    AuthGroup *ag;

    if (m->CurrentRecord)
        LogMsg("AnswerQuestionWithLORecord ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));

    slot = AuthHashSlot(&q->qname);
    ag = AuthGroupForName(&m->rrauth, slot, q->qnamehash, &q->qname);
    if (ag)
    {
        m->CurrentRecord = ag->members;
        while (m->CurrentRecord && m->CurrentRecord != ag->NewLocalOnlyRecords)
        {
            AuthRecord *rr = m->CurrentRecord;
            m->CurrentRecord = rr->next;
            //
            // If the question is mDNSInterface_LocalOnly, all records local to the machine should be used
            // to answer the query. This is handled in AnswerNewLocalOnlyQuestion.
            //
            // We handle mDNSInterface_Any and scoped questions here. See LocalOnlyRecordAnswersQuestion for more
            // details on how we handle this case. For P2P we just handle "Interface_Any" questions. For LocalOnly
            // we handle both mDNSInterface_Any and scoped questions.

            if (rr->ARType == AuthRecordLocalOnly || (rr->ARType == AuthRecordP2P && q->InterfaceID == mDNSInterface_Any))
                if (LocalOnlyRecordAnswersQuestion(rr, q))
                {
                    if (checkOnly)
                    {
                        LogInfo("AnswerQuestionWithLORecord: question %##s (%s) answered by %s", q->qname.c, DNSTypeName(q->qtype),
                            ARDisplayString(m, rr));
                        m->CurrentRecord = mDNSNULL;
                        return mDNStrue;
                    }
                    AnswerLocalQuestionWithLocalAuthRecord(m, rr, mDNStrue);
                    if (m->CurrentQuestion != q)
                        break;     // If callback deleted q, then we're finished here
                }
        }
    }
    m->CurrentRecord = mDNSNULL;

    if (m->CurrentQuestion != q)
    {
        LogInfo("AnswerQuestionWithLORecord: Question deleted while while answering LocalOnly record answers");
        return mDNStrue;
    }

    if (q->LOAddressAnswers)
    {
        LogInfo("AnswerQuestionWithLORecord: Question %p %##s (%s) answered using local auth records LOAddressAnswers %d",
                q, q->qname.c, DNSTypeName(q->qtype), q->LOAddressAnswers);
        return mDNStrue;
    }

    // Before we go check the cache and ship this query on the wire, we have to be sure that there are
    // no local records that could possibly answer this question. As we did not check the NewLocalRecords, we
    // need to just peek at them to see whether it will answer this question. If it would answer, pretend
    // that we answered. AnswerAllLocalQuestionsWithLocalAuthRecord will answer shortly. This happens normally
    // when we add new /etc/hosts entries and restart the question. It is a new question and also a new record.
    if (ag)
    {
        lr = ag->NewLocalOnlyRecords;
        while (lr)
        {
            if (UniqueLocalOnlyRecord(lr) && LocalOnlyRecordAnswersQuestion(lr, q))
            {
                LogInfo("AnswerQuestionWithLORecord: Question %p %##s (%s) will be answered using new local auth records "
                        " LOAddressAnswers %d", q, q->qname.c, DNSTypeName(q->qtype), q->LOAddressAnswers);
                return mDNStrue;
            }
            lr = lr->next;
        }
    }
    return mDNSfalse;
}

// Today, we suppress questions (not send them on the wire) for several reasons e.g.,
// AAAA query is suppressed because no IPv6 capability or PID is not allowed to make
// DNS requests. We need to temporarily suspend the suppress status so that we can
// deliver a negative response (AnswerCurrentQuestionWithResourceRecord does not answer
// suppressed questions) and reset it back. In the future, if there are other
// reasons for suppressing the query, this function should be updated.
mDNSlocal void AnswerSuppressedQuestion(mDNS *const m, DNSQuestion *q)
{
    mDNSBool SuppressQuery = q->SuppressQuery;
    mDNSBool DisallowPID   = q->DisallowPID;

    // make sure that QuerySuppressed() returns false
    q->SuppressQuery = mDNSfalse;
    q->DisallowPID   = mDNSfalse;

    GenerateNegativeResponse(m, QC_suppressed);

    q->SuppressQuery = SuppressQuery;
    q->DisallowPID   = DisallowPID;
}

mDNSlocal void AnswerNewQuestion(mDNS *const m)
{
    mDNSBool ShouldQueryImmediately = mDNStrue;
    DNSQuestion *const q = m->NewQuestions;     // Grab the question we're going to answer
    mDNSu32 slot = HashSlot(&q->qname);
    CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
    mDNSBool AnsweredFromCache = mDNSfalse;

    verbosedebugf("AnswerNewQuestion: Answering %##s (%s)", q->qname.c, DNSTypeName(q->qtype));

    if (cg) CheckCacheExpiration(m, slot, cg);
    if (m->NewQuestions != q) { LogInfo("AnswerNewQuestion: Question deleted while doing CheckCacheExpiration"); goto exit; }
    m->NewQuestions = q->next;
    // Advance NewQuestions to the next *after* calling CheckCacheExpiration, because if we advance it first
    // then CheckCacheExpiration may give this question add/remove callbacks, and it's not yet ready for that.
    //
    // Also, CheckCacheExpiration() calls CacheRecordDeferredAdd() and CacheRecordRmv(), which invoke
    // client callbacks, which may delete their own or any other question. Our mechanism for detecting
    // whether our current m->NewQuestions question got deleted by one of these callbacks is to store the
    // value of m->NewQuestions in 'q' before calling CheckCacheExpiration(), and then verify afterwards
    // that they're still the same. If m->NewQuestions has changed (because mDNS_StopQuery_internal
    // advanced it), that means the question was deleted, so we no longer need to worry about answering
    // it (and indeed 'q' is now a dangling pointer, so dereferencing it at all would be bad, and the
    // values we computed for slot and cg are now stale and relate to a question that no longer exists).
    //
    // We can't use the usual m->CurrentQuestion mechanism for this because  CacheRecordDeferredAdd() and
    // CacheRecordRmv() both use that themselves when walking the list of (non-new) questions generating callbacks.
    // Fortunately mDNS_StopQuery_internal auto-advances both m->CurrentQuestion *AND* m->NewQuestions when
    // deleting a question, so luckily we have an easy alternative way of detecting if our question got deleted.

    if (m->lock_rrcache) LogMsg("AnswerNewQuestion ERROR! Cache already locked!");
    // This should be safe, because calling the client's question callback may cause the
    // question list to be modified, but should not ever cause the rrcache list to be modified.
    // If the client's question callback deletes the question, then m->CurrentQuestion will
    // be advanced, and we'll exit out of the loop
    m->lock_rrcache = 1;
    if (m->CurrentQuestion)
        LogMsg("AnswerNewQuestion ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = q;     // Indicate which question we're answering, so we'll know if it gets deleted

    if (q->NoAnswer == NoAnswer_Fail)
    {
        LogMsg("AnswerNewQuestion: NoAnswer_Fail %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        MakeNegativeCacheRecord(m, &m->rec.r, &q->qname, q->qnamehash, q->qtype, q->qclass, 60, mDNSInterface_Any, q->qDNSServer);
        q->NoAnswer = NoAnswer_Normal;      // Temporarily turn off answer suppression
        AnswerCurrentQuestionWithResourceRecord(m, &m->rec.r, QC_addnocache);
        // Don't touch the question if it has been stopped already
        if (m->CurrentQuestion == q) q->NoAnswer = NoAnswer_Fail;       // Restore NoAnswer state
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }

    if (m->CurrentQuestion != q)
    {
        LogInfo("AnswerNewQuestion: Question deleted while generating NoAnswer_Fail response");
        goto exit;
    }

    // See if we want to tell it about LocalOnly/P2P records. If we answered them using LocalOnly
    // or P2P record, then we are done.
    if (AnswerQuestionWithLORecord(m, q, mDNSfalse))
        goto exit;

    // If we are not supposed to answer this question, generate a negative response.
    // Temporarily suspend the SuppressQuery so that AnswerCurrentQuestionWithResourceRecord can answer the question
    //
    // If it is a question trying to validate some response, it already checked the cache for a response. If it still
    // reissues a question it means it could not find the RRSIGs. So, we need to bypass the cache check and send
    // the question out.
    if (QuerySuppressed(q))
    {
        AnswerSuppressedQuestion(m, q);
    }
    else if (!q->ValidatingResponse)
    {
        CacheRecord *rr;
        for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
            if (SameNameRecordAnswersQuestion(&rr->resrec, q))
            {
                // SecsSinceRcvd is whole number of elapsed seconds, rounded down
                mDNSu32 SecsSinceRcvd = ((mDNSu32)(m->timenow - rr->TimeRcvd)) / mDNSPlatformOneSecond;
                if (rr->resrec.rroriginalttl <= SecsSinceRcvd)
                {
                    LogMsg("AnswerNewQuestion: How is rr->resrec.rroriginalttl %lu <= SecsSinceRcvd %lu for %s %d %d",
                           rr->resrec.rroriginalttl, SecsSinceRcvd, CRDisplayString(m, rr), m->timenow, rr->TimeRcvd);
                    continue;   // Go to next one in loop
                }

                // If this record set is marked unique, then that means we can reasonably assume we have the whole set
                // -- we don't need to rush out on the network and query immediately to see if there are more answers out there
                if ((rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask) || (q->ExpectUnique))
                    ShouldQueryImmediately = mDNSfalse;
                q->CurrentAnswers++;
                if (rr->resrec.rdlength > SmallRecordLimit) q->LargeAnswers++;
                if (rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask) q->UniqueAnswers++;
                AnsweredFromCache = mDNStrue;
                AnswerCurrentQuestionWithResourceRecord(m, rr, QC_add);
                if (m->CurrentQuestion != q) break;     // If callback deleted q, then we're finished here
            }
            else if (RRTypeIsAddressType(rr->resrec.rrtype) && RRTypeIsAddressType(q->qtype))
                ShouldQueryImmediately = mDNSfalse;
    }
    // We don't use LogInfo for this "Question deleted" message because it happens so routinely that
    // it's not remotely remarkable, and therefore unlikely to be of much help tracking down bugs.
    if (m->CurrentQuestion != q) { debugf("AnswerNewQuestion: Question deleted while giving cache answers"); goto exit; }

    // Neither a local record nor a cache entry could answer this question. If this question need to be retried
    // with search domains, generate a negative response which will now retry after appending search domains.
    // If the query was suppressed above, we already generated a negative response. When it gets unsuppressed,
    // we will retry with search domains.
    if (!QuerySuppressed(q) && !AnsweredFromCache && q->RetryWithSearchDomains)
    {
        LogInfo("AnswerNewQuestion: Generating response for retrying with search domains %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        GenerateNegativeResponse(m, QC_forceresponse);
    }

    if (m->CurrentQuestion != q) { debugf("AnswerNewQuestion: Question deleted while giving negative answer"); goto exit; }

    // Note: When a query gets suppressed or retried with search domains, we de-activate the question.
    // Hence we don't execute the following block of code for those cases.
    if (ShouldQueryImmediately && ActiveQuestion(q))
    {
        debugf("AnswerNewQuestion: ShouldQueryImmediately %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        q->ThisQInterval  = InitialQuestionInterval;
        q->LastQTime      = m->timenow - q->ThisQInterval;
        if (mDNSOpaque16IsZero(q->TargetQID))       // For mDNS, spread packets to avoid a burst of simultaneous queries
        {
            // Compute random delay in the range 1-6 seconds, then divide by 50 to get 20-120ms
            if (!m->RandomQueryDelay)
                m->RandomQueryDelay = (mDNSPlatformOneSecond + mDNSRandom(mDNSPlatformOneSecond*5) - 1) / 50 + 1;
            q->LastQTime += m->RandomQueryDelay;
        }
    }

    // IN ALL CASES make sure that m->NextScheduledQuery is set appropriately.
    // In cases where m->NewQuestions->DelayAnswering is set, we may have delayed generating our
    // answers for this question until *after* its scheduled transmission time, in which case
    // m->NextScheduledQuery may now be set to 'never', and in that case -- even though we're *not* doing
    // ShouldQueryImmediately -- we still need to make sure we set m->NextScheduledQuery correctly.
    SetNextQueryTime(m,q);

exit:
    m->CurrentQuestion = mDNSNULL;
    m->lock_rrcache = 0;
}

// When a NewLocalOnlyQuestion is created, AnswerNewLocalOnlyQuestion runs though our ResourceRecords delivering any
// appropriate answers, stopping if it reaches a NewLocalOnlyRecord -- these will be handled by AnswerAllLocalQuestionsWithLocalAuthRecord
mDNSlocal void AnswerNewLocalOnlyQuestion(mDNS *const m)
{
    mDNSu32 slot;
    AuthGroup *ag;
    DNSQuestion *q = m->NewLocalOnlyQuestions;      // Grab the question we're going to answer
    m->NewLocalOnlyQuestions = q->next;             // Advance NewLocalOnlyQuestions to the next (if any)

    debugf("AnswerNewLocalOnlyQuestion: Answering %##s (%s)", q->qname.c, DNSTypeName(q->qtype));

    if (m->CurrentQuestion)
        LogMsg("AnswerNewLocalOnlyQuestion ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = q;     // Indicate which question we're answering, so we'll know if it gets deleted

    if (m->CurrentRecord)
        LogMsg("AnswerNewLocalOnlyQuestion ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));

    // 1. First walk the LocalOnly records answering the LocalOnly question
    // 2. As LocalOnly questions should also be answered by any other Auth records local to the machine,
    //    walk the ResourceRecords list delivering the answers
    slot = AuthHashSlot(&q->qname);
    ag = AuthGroupForName(&m->rrauth, slot, q->qnamehash, &q->qname);
    if (ag)
    {
        m->CurrentRecord = ag->members;
        while (m->CurrentRecord && m->CurrentRecord != ag->NewLocalOnlyRecords)
        {
            AuthRecord *rr = m->CurrentRecord;
            m->CurrentRecord = rr->next;
            if (LocalOnlyRecordAnswersQuestion(rr, q))
            {
                AnswerLocalQuestionWithLocalAuthRecord(m, rr, mDNStrue);
                if (m->CurrentQuestion != q) break;     // If callback deleted q, then we're finished here
            }
        }
    }

    if (m->CurrentQuestion == q)
    {
        m->CurrentRecord = m->ResourceRecords;

        while (m->CurrentRecord && m->CurrentRecord != m->NewLocalRecords)
        {
            AuthRecord *rr = m->CurrentRecord;
            m->CurrentRecord = rr->next;
            if (ResourceRecordAnswersQuestion(&rr->resrec, q))
            {
                AnswerLocalQuestionWithLocalAuthRecord(m, rr, mDNStrue);
                if (m->CurrentQuestion != q) break;     // If callback deleted q, then we're finished here
            }
        }
    }

    m->CurrentQuestion = mDNSNULL;
    m->CurrentRecord   = mDNSNULL;
}

mDNSlocal CacheEntity *GetCacheEntity(mDNS *const m, const CacheGroup *const PreserveCG)
{
    CacheEntity *e = mDNSNULL;

    if (m->lock_rrcache) { LogMsg("GetFreeCacheRR ERROR! Cache already locked!"); return(mDNSNULL); }
    m->lock_rrcache = 1;

    // If we have no free records, ask the client layer to give us some more memory
    if (!m->rrcache_free && m->MainCallback)
    {
        if (m->rrcache_totalused != m->rrcache_size)
            LogMsg("GetFreeCacheRR: count mismatch: m->rrcache_totalused %lu != m->rrcache_size %lu",
                   m->rrcache_totalused, m->rrcache_size);

        // We don't want to be vulnerable to a malicious attacker flooding us with an infinite
        // number of bogus records so that we keep growing our cache until the machine runs out of memory.
        // To guard against this, if our cache grows above 512kB (approx 3168 records at 164 bytes each),
        // and we're actively using less than 1/32 of that cache, then we purge all the unused records
        // and recycle them, instead of allocating more memory.
        if (m->rrcache_size > 5000 && m->rrcache_size / 32 > m->rrcache_active)
            LogInfo("Possible denial-of-service attack in progress: m->rrcache_size %lu; m->rrcache_active %lu",
                    m->rrcache_size, m->rrcache_active);
        else
        {
            mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
            m->MainCallback(m, mStatus_GrowCache);
            mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
        }
    }

    // If we still have no free records, recycle all the records we can.
    // Enumerating the entire cache is moderately expensive, so when we do it, we reclaim all the records we can in one pass.
    if (!m->rrcache_free)
    {
#ifndef REMOVE_UNUSED_VARS
        mDNSu32 oldtotalused = m->rrcache_totalused;
#endif
        mDNSu32 slot;
        for (slot = 0; slot < CACHE_HASH_SLOTS; slot++)
        {
            CacheGroup **cp = &m->rrcache_hash[slot];
            while (*cp)
            {
                CacheRecord **rp = &(*cp)->members;
                while (*rp)
                {
                    // Records that answer still-active questions are not candidates for recycling
                    // Records that are currently linked into the CacheFlushRecords list may not be recycled, or we'll crash
                    if ((*rp)->CRActiveQuestion || (*rp)->NextInCFList)
                        rp=&(*rp)->next;
                    else
                    {
                        CacheRecord *rr = *rp;
                        *rp = (*rp)->next;          // Cut record from list
                        ReleaseCacheRecord(m, rr);
                    }
                }
                if ((*cp)->rrcache_tail != rp)
                    verbosedebugf("GetFreeCacheRR: Updating rrcache_tail[%lu] from %p to %p", slot, (*cp)->rrcache_tail, rp);
                (*cp)->rrcache_tail = rp;
                if ((*cp)->members || (*cp)==PreserveCG) cp=&(*cp)->next;
                else ReleaseCacheGroup(m, cp);
            }
        }
#ifndef REMOVE_UNUSED_VARS
        LogInfo("GetCacheEntity recycled %d records to reduce cache from %d to %d",
                oldtotalused - m->rrcache_totalused, oldtotalused, m->rrcache_totalused);
#endif //#ifndef REMOVE_UNUSED_VARS
    }

    if (m->rrcache_free)    // If there are records in the free list, take one
    {
        e = m->rrcache_free;
        m->rrcache_free = e->next;
        if (++m->rrcache_totalused >= m->rrcache_report)
        {
            LogInfo("RR Cache now using %ld objects", m->rrcache_totalused);
            if      (m->rrcache_report <  100) m->rrcache_report += 10;
            else if (m->rrcache_report < 1000) m->rrcache_report += 100;
            else m->rrcache_report += 1000;
        }
        mDNSPlatformMemZero(e, sizeof(*e));
    }

    m->lock_rrcache = 0;

    return(e);
}

mDNSlocal CacheRecord *GetCacheRecord(mDNS *const m, CacheGroup *cg, mDNSu16 RDLength)
{
    CacheRecord *r = (CacheRecord *)GetCacheEntity(m, cg);
    if (r)
    {
        r->resrec.rdata = (RData*)&r->smallrdatastorage;    // By default, assume we're usually going to be using local storage
        if (RDLength > InlineCacheRDSize)           // If RDLength is too big, allocate extra storage
        {
            r->resrec.rdata = (RData*)mDNSPlatformMemAllocate(sizeofRDataHeader + RDLength);
            if (r->resrec.rdata) r->resrec.rdata->MaxRDLength = r->resrec.rdlength = RDLength;
            else { ReleaseCacheEntity(m, (CacheEntity*)r); r = mDNSNULL; }
        }
    }
    return(r);
}

mDNSlocal CacheGroup *GetCacheGroup(mDNS *const m, const mDNSu32 slot, const ResourceRecord *const rr)
{
    mDNSu16 namelen = DomainNameLength(rr->name);
    CacheGroup *cg = (CacheGroup*)GetCacheEntity(m, mDNSNULL);
    if (!cg) { LogMsg("GetCacheGroup: Failed to allocate memory for %##s", rr->name->c); return(mDNSNULL); }
    cg->next         = m->rrcache_hash[slot];
    cg->namehash     = rr->namehash;
    cg->members      = mDNSNULL;
    cg->rrcache_tail = &cg->members;
    if (namelen > sizeof(cg->namestorage)) 
        cg->name = mDNSPlatformMemAllocate(namelen);
    else
        cg->name = (domainname*)cg->namestorage;
    if (!cg->name)
    {
        LogMsg("GetCacheGroup: Failed to allocate name storage for %##s", rr->name->c);
        ReleaseCacheEntity(m, (CacheEntity*)cg);
        return(mDNSNULL);
    }
    AssignDomainName(cg->name, rr->name);

    if (CacheGroupForRecord(m, slot, rr)) LogMsg("GetCacheGroup: Already have CacheGroup for %##s", rr->name->c);
    m->rrcache_hash[slot] = cg;
    if (CacheGroupForRecord(m, slot, rr) != cg) LogMsg("GetCacheGroup: Not finding CacheGroup for %##s", rr->name->c);

    return(cg);
}

mDNSexport void mDNS_PurgeCacheResourceRecord(mDNS *const m, CacheRecord *rr)
{
    mDNS_CheckLock(m);

    // Make sure we mark this record as thoroughly expired -- we don't ever want to give
    // a positive answer using an expired record (e.g. from an interface that has gone away).
    // We don't want to clear CRActiveQuestion here, because that would leave the record subject to
    // summary deletion without giving the proper callback to any questions that are monitoring it.
    // By setting UnansweredQueries to MaxUnansweredQueries we ensure it won't trigger any further expiration queries.
    rr->TimeRcvd          = m->timenow - mDNSPlatformOneSecond * 60;
    rr->UnansweredQueries = MaxUnansweredQueries;
    rr->resrec.rroriginalttl     = 0;
    SetNextCacheCheckTimeForRecord(m, rr);
}

mDNSexport mDNSs32 mDNS_TimeNow(const mDNS *const m)
{
    mDNSs32 time;
    mDNSPlatformLock(m);
    if (m->mDNS_busy)
    {
        LogMsg("mDNS_TimeNow called while holding mDNS lock. This is incorrect. Code protected by lock should just use m->timenow.");
        if (!m->timenow) LogMsg("mDNS_TimeNow: m->mDNS_busy is %ld but m->timenow not set", m->mDNS_busy);
    }

    if (m->timenow) time = m->timenow;
    else time = mDNS_TimeNow_NoLock(m);
    mDNSPlatformUnlock(m);
    return(time);
}

// To avoid pointless CPU thrash, we use SetSPSProxyListChanged(X) to record the last interface that
// had its Sleep Proxy client list change, and defer to actual BPF reconfiguration to mDNS_Execute().
// (GetNextScheduledEvent() returns "now" when m->SPSProxyListChanged is set)
#define SetSPSProxyListChanged(X) do { \
        if (m->SPSProxyListChanged && m->SPSProxyListChanged != (X)) mDNSPlatformUpdateProxyList(m, m->SPSProxyListChanged);                                                                                                                          \
        m->SPSProxyListChanged = (X); } while(0)

// Called from mDNS_Execute() to expire stale proxy records
mDNSlocal void CheckProxyRecords(mDNS *const m, AuthRecord *list)
{
    m->CurrentRecord = list;
    while (m->CurrentRecord)
    {
        AuthRecord *rr = m->CurrentRecord;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (rr->resrec.RecordType != kDNSRecordTypeDeregistering && rr->WakeUp.HMAC.l[0])
#else
        if (rr->resrec.RecordType != kDNSRecordTypeDeregistering)
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        {
            // If m->SPSSocket is NULL that means we're not acting as a sleep proxy any more,
            // so we need to cease proxying for *all* records we may have, expired or not.
            if (m->SPSSocket && m->timenow - rr->TimeExpire < 0)    // If proxy record not expired yet, update m->NextScheduledSPS
            {
                if (m->NextScheduledSPS - rr->TimeExpire > 0)
                    m->NextScheduledSPS = rr->TimeExpire;
            }
            else                                                    // else proxy record expired, so remove it
            {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                LogSPS("CheckProxyRecords: Removing %d H-MAC %.6a I-MAC %.6a %d %s",
                       m->ProxyRecords, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, rr->WakeUp.seq, ARDisplayString(m, rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                SetSPSProxyListChanged(rr->resrec.InterfaceID);
                mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
                // Don't touch rr after this -- memory may have been free'd
            }
        }
        // Mustn't advance m->CurrentRecord until *after* mDNS_Deregister_internal, because
        // new records could have been added to the end of the list as a result of that call.
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}

mDNSlocal void CheckRmvEventsForLocalRecords(mDNS *const m)
{
    while (m->CurrentRecord)
    {
        AuthRecord *rr = m->CurrentRecord;
        if (rr->AnsweredLocalQ && rr->resrec.RecordType == kDNSRecordTypeDeregistering)
        {
            debugf("CheckRmvEventsForLocalRecords: Generating local RMV events for %s", ARDisplayString(m, rr));
            rr->resrec.RecordType = kDNSRecordTypeShared;
            AnswerAllLocalQuestionsWithLocalAuthRecord(m, rr, mDNSfalse);
            if (m->CurrentRecord == rr) // If rr still exists in list, restore its state now
            {
                rr->resrec.RecordType = kDNSRecordTypeDeregistering;
                rr->AnsweredLocalQ = mDNSfalse;
                // SendResponses normally calls CompleteDeregistration after sending goodbyes.
                // For LocalOnly records, we don't do that and hence we need to do that here.
                if (RRLocalOnly(rr)) CompleteDeregistration(m, rr);
            }
        }
        if (m->CurrentRecord == rr)     // If m->CurrentRecord was not auto-advanced, do it ourselves now
            m->CurrentRecord = rr->next;
    }
}

mDNSlocal void TimeoutQuestions(mDNS *const m)
{
    m->NextScheduledStopTime = m->timenow + 0x3FFFFFFF;
    if (m->CurrentQuestion)
        LogMsg("TimeoutQuestions ERROR m->CurrentQuestion already set: %##s (%s)", m->CurrentQuestion->qname.c,
               DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    while (m->CurrentQuestion)
    {
        DNSQuestion *const q = m->CurrentQuestion;
        if (q->StopTime)
        {
            if (!q->TimeoutQuestion)
                LogMsg("TimeoutQuestions: ERROR!! TimeoutQuestion not set, but StopTime set for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));

            if (m->timenow - q->StopTime >= 0)
            {
                LogInfo("TimeoutQuestions: question %p %##s timed out, time %d", q, q->qname.c, m->timenow - q->StopTime);
                GenerateNegativeResponse(m, QC_forceresponse);
                if (m->CurrentQuestion == q) q->StopTime = 0;
            }
            else
            {
                if (m->NextScheduledStopTime - q->StopTime > 0)
                    m->NextScheduledStopTime = q->StopTime;
            }
        }
        // If m->CurrentQuestion wasn't modified out from under us, advance it now
        // We can't do this at the start of the loop because GenerateNegativeResponse
        // depends on having m->CurrentQuestion point to the right question
        if (m->CurrentQuestion == q)
            m->CurrentQuestion = q->next;
    }
    m->CurrentQuestion = mDNSNULL;
}
#ifndef REMOVE_MACHINE_SLEEP
mDNSlocal void mDNSCoreFreeProxyRR(mDNS *const m)
{
    AuthRecord *rrPtr = m->SPSRRSet, *rrNext = mDNSNULL;
    LogSPS("%s : Freeing stored sleep proxy A/AAAA records", __func__);
    while (rrPtr)
    {
        rrNext = rrPtr->next;
        mDNSPlatformMemFree(rrPtr);
        rrPtr  = rrNext;
    }
    m->SPSRRSet = mDNSNULL;
}
#endif //#ifndef REMOVE_MACHINE_SLEEP

mDNSexport mDNSs32 mDNS_Execute(mDNS *const m)
{
    mDNS_Lock(m);   // Must grab lock before trying to read m->timenow

#if APPLE_OSX_mDNSResponder
    mDNSLogStatistics(m);
#endif // APPLE_OSX_mDNSResponder

    if (m->timenow - m->NextScheduledEvent >= 0)
    {
        int i;
        AuthRecord *head, *tail;
        mDNSu32 slot;
        AuthGroup *ag;

        verbosedebugf("mDNS_Execute");

        if (m->CurrentQuestion)
            LogMsg("mDNS_Execute: ERROR m->CurrentQuestion already set: %##s (%s)",
                   m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));

        if (m->CurrentRecord)
            LogMsg("mDNS_Execute: ERROR m->CurrentRecord already set: %s", ARDisplayString(m, m->CurrentRecord));

        // 1. If we're past the probe suppression time, we can clear it
        if (m->SuppressProbes && m->timenow - m->SuppressProbes >= 0) m->SuppressProbes = 0;

        // 2. If it's been more than ten seconds since the last probe failure, we can clear the counter
        if (m->NumFailedProbes && m->timenow - m->ProbeFailTime >= mDNSPlatformOneSecond * 10) m->NumFailedProbes = 0;

        // 3. Purge our cache of stale old records
        if (m->rrcache_size && m->timenow - m->NextCacheCheck >= 0)
        {
            mDNSu32 numchecked = 0;
            m->NextCacheCheck = m->timenow + 0x3FFFFFFF;
            for (slot = 0; slot < CACHE_HASH_SLOTS; slot++)
            {
                if (m->timenow - m->rrcache_nextcheck[slot] >= 0)
                {
                    CacheGroup **cp = &m->rrcache_hash[slot];
                    m->rrcache_nextcheck[slot] = m->timenow + 0x3FFFFFFF;
                    while (*cp)
                    {
                        debugf("m->NextCacheCheck %4d Slot %3d %##s", numchecked, slot, *cp ? (*cp)->name : (domainname*)"\x04NULL");
                        numchecked++;
                        CheckCacheExpiration(m, slot, *cp);
                        if ((*cp)->members) cp=&(*cp)->next;
                        else ReleaseCacheGroup(m, cp);
                    }
                }
                // Even if we didn't need to actually check this slot yet, still need to
                // factor its nextcheck time into our overall NextCacheCheck value
                if (m->NextCacheCheck - m->rrcache_nextcheck[slot] > 0)
                    m->NextCacheCheck = m->rrcache_nextcheck[slot];
            }
            debugf("m->NextCacheCheck %4d checked, next in %d", numchecked, m->NextCacheCheck - m->timenow);
        }

        if (m->timenow - m->NextScheduledSPS >= 0)
        {
            m->NextScheduledSPS = m->timenow + 0x3FFFFFFF;
            CheckProxyRecords(m, m->DuplicateRecords);  // Clear m->DuplicateRecords first, then m->ResourceRecords
            CheckProxyRecords(m, m->ResourceRecords);
        }

        SetSPSProxyListChanged(mDNSNULL);       // Perform any deferred BPF reconfiguration now

        // Check to see if we need to send any keepalives. Do this after we called CheckProxyRecords above
        // as records could have expired during that check
        if (m->timenow - m->NextScheduledKA >= 0)
        {
            m->NextScheduledKA = m->timenow + 0x3FFFFFFF;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            mDNS_SendKeepalives(m);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        }

        // Clear AnnounceOwner if necessary. (Do this *before* SendQueries() and SendResponses().)
        if (m->AnnounceOwner && m->timenow - m->AnnounceOwner >= 0)
        {
            m->AnnounceOwner = 0;
        }

        if (m->DelaySleep && m->timenow - m->DelaySleep >= 0)
        {
            m->DelaySleep = 0;
            if (m->SleepState == SleepState_Transferring)
            {
                LogSPS("Re-sleep delay passed; now checking for Sleep Proxy Servers");
                BeginSleepProcessing(m);
            }
        }

        // 4. See if we can answer any of our new local questions from the cache
        for (i=0; m->NewQuestions && i<1000; i++)
        {
            if (m->NewQuestions->DelayAnswering && m->timenow - m->NewQuestions->DelayAnswering < 0) break;
            AnswerNewQuestion(m);
        }
        if (i >= 1000) LogMsg("mDNS_Execute: AnswerNewQuestion exceeded loop limit");

        // Make sure we deliver *all* local RMV events, and clear the corresponding rr->AnsweredLocalQ flags, *before*
        // we begin generating *any* new ADD events in the m->NewLocalOnlyQuestions and m->NewLocalRecords loops below.
        for (i=0; i<1000 && m->LocalRemoveEvents; i++)
        {
            m->LocalRemoveEvents = mDNSfalse;
            m->CurrentRecord = m->ResourceRecords;
            CheckRmvEventsForLocalRecords(m);
            // Walk the LocalOnly records and deliver the RMV events
            for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
                for (ag = m->rrauth.rrauth_hash[slot]; ag; ag = ag->next)
                {
                    m->CurrentRecord = ag->members;
                    if (m->CurrentRecord) CheckRmvEventsForLocalRecords(m);
                }
        }

        if (i >= 1000) LogMsg("mDNS_Execute: m->LocalRemoveEvents exceeded loop limit");

        for (i=0; m->NewLocalOnlyQuestions && i<1000; i++) AnswerNewLocalOnlyQuestion(m);
        if (i >= 1000) LogMsg("mDNS_Execute: AnswerNewLocalOnlyQuestion exceeded loop limit");

        head = tail = mDNSNULL;
        for (i=0; i<1000 && m->NewLocalRecords && m->NewLocalRecords != head; i++)
        {
            AuthRecord *rr = m->NewLocalRecords;
            m->NewLocalRecords = m->NewLocalRecords->next;
            if (LocalRecordReady(rr))
            {
                debugf("mDNS_Execute: Delivering Add event with LocalAuthRecord %s", ARDisplayString(m, rr));
                AnswerAllLocalQuestionsWithLocalAuthRecord(m, rr, mDNStrue);
            }
            else if (!rr->next)
            {
                // If we have just one record that is not ready, we don't have to unlink and
                // reinsert. As the NewLocalRecords will be NULL for this case, the loop will
                // terminate and set the NewLocalRecords to rr.
                debugf("mDNS_Execute: Just one LocalAuthRecord %s, breaking out of the loop early", ARDisplayString(m, rr));
                if (head != mDNSNULL || m->NewLocalRecords != mDNSNULL)
                    LogMsg("mDNS_Execute: ERROR!!: head %p, NewLocalRecords %p", head, m->NewLocalRecords);

                head = rr;
            }
            else
            {
                AuthRecord **p = &m->ResourceRecords;   // Find this record in our list of active records
                debugf("mDNS_Execute: Skipping LocalAuthRecord %s", ARDisplayString(m, rr));
                // if this is the first record we are skipping, move to the end of the list.
                // if we have already skipped records before, append it at the end.
                while (*p && *p != rr) p=&(*p)->next;
                if (*p) *p = rr->next;                  // Cut this record from the list
                else { LogMsg("mDNS_Execute: ERROR!! Cannot find record %s in ResourceRecords list", ARDisplayString(m, rr)); break; }
                if (!head)
                {
                    while (*p) p=&(*p)->next;
                    *p = rr;
                    head = tail = rr;
                }
                else
                {
                    tail->next = rr;
                    tail = rr;
                }
                rr->next = mDNSNULL;
            }
        }
        m->NewLocalRecords = head;
        debugf("mDNS_Execute: Setting NewLocalRecords to %s", (head ? ARDisplayString(m, head) : "NULL"));

        if (i >= 1000) LogMsg("mDNS_Execute: m->NewLocalRecords exceeded loop limit");

        // Check to see if we have any new LocalOnly/P2P records to examine for delivering
        // to our local questions
        if (m->NewLocalOnlyRecords)
        {
            m->NewLocalOnlyRecords = mDNSfalse;
            for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
                for (ag = m->rrauth.rrauth_hash[slot]; ag; ag = ag->next)
                {
                    for (i=0; i<100 && ag->NewLocalOnlyRecords; i++)
                    {
                        AuthRecord *rr = ag->NewLocalOnlyRecords;
                        ag->NewLocalOnlyRecords = ag->NewLocalOnlyRecords->next;
                        // LocalOnly records should always be ready as they never probe
                        if (LocalRecordReady(rr))
                        {
                            debugf("mDNS_Execute: Delivering Add event with LocalAuthRecord %s", ARDisplayString(m, rr));
                            AnswerAllLocalQuestionsWithLocalAuthRecord(m, rr, mDNStrue);
                        }
                        else LogMsg("mDNS_Execute: LocalOnlyRecord %s not ready", ARDisplayString(m, rr));
                    }
                    // We limit about 100 per AuthGroup that can be serviced at a time
                    if (i >= 100) LogMsg("mDNS_Execute: ag->NewLocalOnlyRecords exceeded loop limit");
                }
        }

        // 5. See what packets we need to send
        if (m->mDNSPlatformStatus != mStatus_NoError || (m->SleepState == SleepState_Sleeping))
            DiscardDeregistrations(m);
        if (m->mDNSPlatformStatus == mStatus_NoError && (m->SuppressSending == 0 || m->timenow - m->SuppressSending >= 0))
        {
            // If the platform code is ready, and we're not suppressing packet generation right now
            // then send our responses, probes, and questions.
            // We check the cache first, because there might be records close to expiring that trigger questions to refresh them.
            // We send queries next, because there might be final-stage probes that complete their probing here, causing
            // them to advance to announcing state, and we want those to be included in any announcements we send out.
            // Finally, we send responses, including the previously mentioned records that just completed probing.
            m->SuppressSending = 0;

            // 6. Send Query packets. This may cause some probing records to advance to announcing state
            if (m->timenow - m->NextScheduledQuery >= 0 || m->timenow - m->NextScheduledProbe >= 0) SendQueries(m);
            if (m->timenow - m->NextScheduledQuery >= 0)
            {
                DNSQuestion *q;
                LogMsg("mDNS_Execute: SendQueries didn't send all its queries (%d - %d = %d) will try again in one second",
                       m->timenow, m->NextScheduledQuery, m->timenow - m->NextScheduledQuery);
                m->NextScheduledQuery = m->timenow + mDNSPlatformOneSecond;
                for (q = m->Questions; q && q != m->NewQuestions; q=q->next)
                    if (ActiveQuestion(q) && m->timenow - NextQSendTime(q) >= 0)
                        LogMsg("mDNS_Execute: SendQueries didn't send %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
            }
            if (m->timenow - m->NextScheduledProbe >= 0)
            {
                LogMsg("mDNS_Execute: SendQueries didn't send all its probes (%d - %d = %d) will try again in one second",
                       m->timenow, m->NextScheduledProbe, m->timenow - m->NextScheduledProbe);
                m->NextScheduledProbe = m->timenow + mDNSPlatformOneSecond;
            }

            // 7. Send Response packets, including probing records just advanced to announcing state
            if (m->timenow - m->NextScheduledResponse >= 0) 
            {
                // mdnslogInfo("SendResponses");
                SendResponses(m);
            }
            else
            {
                mdnslogInfo("No SendResponses");
            }

            if (m->timenow - m->NextScheduledResponse >= 0)
            {
                LogMsg("mDNS_Execute: SendResponses didn't send all its responses; will try again in one second");
                m->NextScheduledResponse = m->timenow + mDNSPlatformOneSecond;
            }
        }

        // Clear RandomDelay values, ready to pick a new different value next time
        m->RandomQueryDelay     = 0;
        m->RandomReconfirmDelay = 0;

        if (m->NextScheduledStopTime && m->timenow - m->NextScheduledStopTime >= 0) TimeoutQuestions(m);
#ifndef UNICAST_DISABLED
        if (m->NextSRVUpdate && m->timenow - m->NextSRVUpdate >= 0) UpdateAllSRVRecords(m);
        if (m->timenow - m->NextScheduledNATOp >= 0) CheckNATMappings(m);
        if (m->timenow - m->NextuDNSEvent >= 0) uDNS_Tasks(m);
#endif
    }

    // Note about multi-threaded systems:
    // On a multi-threaded system, some other thread could run right after the mDNS_Unlock(),
    // performing mDNS API operations that change our next scheduled event time.
    //
    // On multi-threaded systems (like the current Windows implementation) that have a single main thread
    // calling mDNS_Execute() (and other threads allowed to call mDNS API routines) it is the responsibility
    // of the mDNSPlatformUnlock() routine to signal some kind of stateful condition variable that will
    // signal whatever blocking primitive the main thread is using, so that it will wake up and execute one
    // more iteration of its loop, and immediately call mDNS_Execute() again. The signal has to be stateful
    // in the sense that if the main thread has not yet entered its blocking primitive, then as soon as it
    // does, the state of the signal will be noticed, causing the blocking primitive to return immediately
    // without blocking. This avoids the race condition between the signal from the other thread arriving
    // just *before* or just *after* the main thread enters the blocking primitive.
    //
    // On multi-threaded systems (like the current Mac OS 9 implementation) that are entirely timer-driven,
    // with no main mDNS_Execute() thread, it is the responsibility of the mDNSPlatformUnlock() routine to
    // set the timer according to the m->NextScheduledEvent value, and then when the timer fires, the timer
    // callback function should call mDNS_Execute() (and ignore the return value, which may already be stale
    // by the time it gets to the timer callback function).

    mDNS_Unlock(m);     // Calling mDNS_Unlock is what gives m->NextScheduledEvent its new value
    return(m->NextScheduledEvent);
}

#ifndef UNICAST_DISABLED
mDNSlocal void SuspendLLQs(mDNS *m)
{
    DNSQuestion *q;
    for (q = m->Questions; q; q = q->next)
        if (ActiveQuestion(q) && !mDNSOpaque16IsZero(q->TargetQID) && q->LongLived && q->state == LLQ_Established)
        { q->ReqLease = 0; sendLLQRefresh(m, q); }
}
#endif // UNICAST_DISABLED

mDNSlocal mDNSBool QuestionHasLocalAnswers(mDNS *const m, DNSQuestion *q)
{
    AuthRecord *rr;
    mDNSu32 slot;
    AuthGroup *ag;

    slot = AuthHashSlot(&q->qname);
    ag = AuthGroupForName(&m->rrauth, slot, q->qnamehash, &q->qname);
    if (ag)
    {
        for (rr = ag->members; rr; rr=rr->next)
            // Filter the /etc/hosts records - LocalOnly, Unique, A/AAAA/CNAME
            if (UniqueLocalOnlyRecord(rr) && LocalOnlyRecordAnswersQuestion(rr, q))
            {
                LogInfo("QuestionHasLocalAnswers: Question %p %##s (%s) has local answer %s", q, q->qname.c, DNSTypeName(q->qtype), ARDisplayString(m, rr));
                return mDNStrue;
            }
    }
    return mDNSfalse;
}

// ActivateUnicastQuery() is called from three places:
// 1. When a new question is created
// 2. On wake from sleep
// 3. When the DNS configuration changes
// In case 1 we don't want to mess with our established ThisQInterval and LastQTime (ScheduleImmediately is false)
// In cases 2 and 3 we do want to cause the question to be resent immediately (ScheduleImmediately is true)
mDNSlocal void ActivateUnicastQuery(mDNS *const m, DNSQuestion *const question, mDNSBool ScheduleImmediately)
{
    // For now this AutoTunnel stuff is specific to Mac OS X.
    // In the future, if there's demand, we may see if we can abstract it out cleanly into the platform layer
#if APPLE_OSX_mDNSResponder
    // Even though BTMM client tunnels are only useful for AAAA queries, we need to treat v4 and v6 queries equally.
    // Otherwise we can get the situation where the A query completes really fast (with an NXDOMAIN result) and the
    // caller then gives up waiting for the AAAA result while we're still in the process of setting up the tunnel.
    // To level the playing field, we block both A and AAAA queries while tunnel setup is in progress, and then
    // returns results for both at the same time. If we are looking for the _autotunnel6 record, then skip this logic
    // as this would trigger looking up _autotunnel6._autotunnel6 and end up failing the original query.

    if (RRTypeIsAddressType(question->qtype) && PrivateQuery(question) &&
        !SameDomainLabel(question->qname.c, (const mDNSu8 *)"\x0c_autotunnel6")&& question->QuestionCallback != AutoTunnelCallback)
    {
        question->NoAnswer = NoAnswer_Suspended;
        AddNewClientTunnel(m, question);
        return;
    }
#endif // APPLE_OSX_mDNSResponder

    if (!question->DuplicateOf)
    {
        debugf("ActivateUnicastQuery: %##s %s%s%s",
               question->qname.c, DNSTypeName(question->qtype), PrivateQuery(question) ? " (Private)" : "", ScheduleImmediately ? " ScheduleImmediately" : "");
        question->CNAMEReferrals = 0;
        if (question->nta) { CancelGetZoneData(m, question->nta); question->nta = mDNSNULL; }
        if (question->LongLived)
        {
            question->state = LLQ_InitialRequest;
            question->id = zeroOpaque64;
            question->servPort = zeroIPPort;
            if (question->tcp) { DisposeTCPConn(question->tcp); question->tcp = mDNSNULL; }
        }
        // If the question has local answers, then we don't want answers from outside
        if (ScheduleImmediately && !QuestionHasLocalAnswers(m, question))
        {
            question->ThisQInterval = InitialQuestionInterval;
            question->LastQTime     = m->timenow - question->ThisQInterval;
            SetNextQueryTime(m, question);
        }
    }
}

// Caller should hold the lock
mDNSexport void mDNSCoreRestartAddressQueries(mDNS *const m, mDNSBool SearchDomainsChanged, FlushCache flushCacheRecords,
                                              CallbackBeforeStartQuery BeforeStartCallback, void *context)
{
    DNSQuestion *q;
    DNSQuestion *restart = mDNSNULL;

    mDNS_CheckLock(m);

    // 1. Flush the cache records
    if (flushCacheRecords) flushCacheRecords(m);

    // 2. Even though we may have purged the cache records above, before it can generate RMV event
    // we are going to stop the question. Hence we need to deliver the RMV event before we
    // stop the question.
    //
    // CurrentQuestion is used by RmvEventsForQuestion below. While delivering RMV events, the
    // application callback can potentially stop the current question (detected by CurrentQuestion) or
    // *any* other question which could be the next one that we may process here. RestartQuestion
    // points to the "next" question which will be automatically advanced in mDNS_StopQuery_internal
    // if the "next" question is stopped while the CurrentQuestion is stopped

    if (m->RestartQuestion)
        LogMsg("mDNSCoreRestartAddressQueries: ERROR!! m->RestartQuestion already set: %##s (%s)",
               m->RestartQuestion->qname.c, DNSTypeName(m->RestartQuestion->qtype));

    m->RestartQuestion = m->Questions;
    while (m->RestartQuestion)
    {
        q = m->RestartQuestion;
        m->RestartQuestion = q->next;
        // GetZoneData questions are referenced by other questions (original query that started the GetZoneData
        // question)  through their "nta" pointer. Normally when the original query stops, it stops the
        // GetZoneData question and also frees the memory (See CancelGetZoneData). If we stop the GetZoneData
        // question followed by the original query that refers to this GetZoneData question, we will end up
        // freeing the GetZoneData question and then start the "freed" question at the end.

        if (IsGetZoneDataQuestion(q))
        {
            DNSQuestion *refq = q->next;
            LogInfo("mDNSCoreRestartAddressQueries: Skipping GetZoneDataQuestion %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
            // debug stuff, we just try to find the referencing question and don't do much with it
            while (refq)
            {
                if (q == &refq->nta->question)
                {
                    LogInfo("mDNSCoreRestartAddressQueries: Question %p %##s (%s) referring to GetZoneDataQuestion %p, not stopping", refq, refq->qname.c, DNSTypeName(refq->qtype), q);
                }
                refq = refq->next;
            }
            continue;
        }

        // This function is called when /etc/hosts changes and that could affect A, AAAA and CNAME queries
        if (q->qtype != kDNSType_A && q->qtype != kDNSType_AAAA && q->qtype != kDNSType_CNAME) continue;

        // If the search domains did not change, then we restart all the queries. Otherwise, only
        // for queries for which we "might" have appended search domains ("might" because we may
        // find results before we apply search domains even though AppendSearchDomains is set to 1)
        if (!SearchDomainsChanged || q->AppendSearchDomains)
        {
            // NOTE: CacheRecordRmvEventsForQuestion will not generate RMV events for queries that have non-zero
            // LOAddressAnswers. Hence it is important that we call CacheRecordRmvEventsForQuestion before
            // LocalRecordRmvEventsForQuestion (which decrements LOAddressAnswers). Let us say that
            // /etc/hosts has an A Record for web.apple.com. Any queries for web.apple.com will be answered locally.
            // But this can't prevent a CNAME/AAAA query to not to be sent on the wire. When it is sent on the wire,
            // it could create cache entries. When we are restarting queries, we can't deliver the cache RMV events
            // for the original query using these cache entries as ADDs were never delivered using these cache
            // entries and hence this order is needed.

            // If the query is suppressed, the RMV events won't be delivered
            if (!CacheRecordRmvEventsForQuestion(m, q)) { LogInfo("mDNSCoreRestartAddressQueries: Question deleted while delivering Cache Record RMV events"); continue; }

            // SuppressQuery status does not affect questions that are answered using local records
            if (!LocalRecordRmvEventsForQuestion(m, q)) { LogInfo("mDNSCoreRestartAddressQueries: Question deleted while delivering Local Record RMV events"); continue; }

            LogInfo("mDNSCoreRestartAddressQueries: Stop question %p %##s (%s), AppendSearchDomains %d, qnameOrig %p", q,
                    q->qname.c, DNSTypeName(q->qtype), q->AppendSearchDomains, q->qnameOrig);
            mDNS_StopQuery_internal(m, q);
            // Reset state so that it looks like it was in the beginning i.e it should look at /etc/hosts, cache
            // and then search domains should be appended. At the beginning, qnameOrig was NULL.
            if (q->qnameOrig)
            {
                LogInfo("mDNSCoreRestartAddressQueries: qnameOrig %##s", q->qnameOrig);
                AssignDomainName(&q->qname, q->qnameOrig);
                mDNSPlatformMemFree(q->qnameOrig);
                q->qnameOrig = mDNSNULL;
                q->RetryWithSearchDomains = ApplySearchDomainsFirst(q) ? 1 : 0;
            }
            q->SearchListIndex = 0;
            q->next = restart;
            restart = q;
        }
    }

    // 3. Callback before we start the query
    if (BeforeStartCallback) BeforeStartCallback(m, context);

    // 4. Restart all the stopped queries
    while (restart)
    {
        q = restart;
        restart = restart->next;
        q->next = mDNSNULL;
        LogInfo("mDNSCoreRestartAddressQueries: Start question %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
        mDNS_StartQuery_internal(m, q);
    }
}

mDNSexport void mDNSCoreRestartQueries(mDNS *const m)
{
    DNSQuestion *q;

#ifndef UNICAST_DISABLED
    // Retrigger all our uDNS questions
    if (m->CurrentQuestion)
        LogMsg("mDNSCoreRestartQueries: ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    while (m->CurrentQuestion)
    {
        q = m->CurrentQuestion;
        m->CurrentQuestion = m->CurrentQuestion->next;
        if (!mDNSOpaque16IsZero(q->TargetQID) && ActiveQuestion(q)) ActivateUnicastQuery(m, q, mDNStrue);
    }
#endif

    // Retrigger all our mDNS questions
    for (q = m->Questions; q; q=q->next)                // Scan our list of questions
            mDNSCoreRestartQuestion(m, q);
}

// restart question if it's multicast and currently active
mDNSexport void mDNSCoreRestartQuestion(mDNS *const m, DNSQuestion *q)
{
    if (mDNSOpaque16IsZero(q->TargetQID) && ActiveQuestion(q))
    {
        q->ThisQInterval    = InitialQuestionInterval;  // MUST be > zero for an active question
#if mDNS_REQUEST_UNICAST_RESPONSE
        q->RequestUnicast = SET_QU_IN_FIRST_FOUR_QUERIES;
#else   // mDNS_REQUEST_UNICAST_RESPONSE
        q->RequestUnicast   = SET_QU_IN_FIRST_QUERY;
#endif  // mDNS_REQUEST_UNICAST_RESPONSE
        q->LastQTime        = m->timenow - q->ThisQInterval;
        q->RecentAnswerPkts = 0;
        ExpireDupSuppressInfo(q->DupSuppress, m->timenow);
        m->NextScheduledQuery = m->timenow;
    }
}

// restart the probe/announce cycle for multicast record
mDNSexport void mDNSCoreRestartRegistration(mDNS *const m, AuthRecord  *rr, int announceCount)
{
    if (!AuthRecord_uDNS(rr))
    {
        if (rr->resrec.RecordType == kDNSRecordTypeVerified && !rr->DependentOn) rr->resrec.RecordType = kDNSRecordTypeUnique;
        rr->ProbeCount     = DefaultProbeCountForRecordType(rr->resrec.RecordType);

        // announceCount < 0 indicates default announce count should be used
        if (announceCount < 0)
            announceCount = InitialAnnounceCount;
        if (rr->AnnounceCount < announceCount) 
            rr->AnnounceCount  = announceCount;

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (mDNS_KeepaliveRecord(&rr->resrec))
            rr->AnnounceCount  = 0; // Do not announce keepalive records
        else
            rr->AnnounceCount  = InitialAnnounceCount;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        rr->SendNSECNow    = mDNSNULL;
        InitializeLastAPTime(m, rr);
    }
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Power Management (Sleep/Wake)
#endif

mDNSexport void mDNS_UpdateAllowSleep(mDNS *const m)
{
#ifndef IDLESLEEPCONTROL_DISABLED
    mDNSBool allowSleep = mDNStrue;
    char reason[128];

    reason[0] = 0;

    if (m->SystemSleepOnlyIfWakeOnLAN)
    {
        // Don't sleep if we are a proxy for any services
        if (m->ProxyRecords)
        {
            allowSleep = mDNSfalse;
            mDNS_snprintf(reason, sizeof(reason), "sleep proxy for %d records", m->ProxyRecords);
            LogInfo("mDNS_UpdateAllowSleep: Sleep disabled because we are proxying %d records", m->ProxyRecords);
        }

        if (allowSleep && mDNSCoreHaveAdvertisedMulticastServices(m))
        {
            // Scan the list of active interfaces
            NetworkInterfaceInfo *intf;
            for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
            {
                if (intf->McastTxRx && !intf->Loopback && !mDNSPlatformInterfaceIsD2D(intf->InterfaceID))
                {
                    // Disallow sleep if this interface doesn't support NetWake
#ifndef REMOVE_NETWAKE
                    if (!intf->NetWake)
#endif // ifndef REMOVE_NETWAKE
                    {
                        allowSleep = mDNSfalse;
                        mDNS_snprintf(reason, sizeof(reason), "%s does not support NetWake", intf->ifname);
                        LogInfo("mDNS_UpdateAllowSleep: Sleep disabled because %s does not support NetWake", intf->ifname);
                        break;
                    }

                    // Disallow sleep if there is no sleep proxy server
                    const CacheRecord *cr = FindSPSInCache1(m, &intf->NetWakeBrowse, mDNSNULL, mDNSNULL);
                    if ( cr == mDNSNULL)
                    {
                        allowSleep = mDNSfalse;
                        mDNS_snprintf(reason, sizeof(reason), "No sleep proxy server on %s", intf->ifname);
                        LogInfo("mDNS_UpdateAllowSleep: Sleep disabled because %s has no sleep proxy server", intf->ifname);
                        break;
                    }
                    else if (m->SPSType != 0)
                    {
                        mDNSu32 mymetric = LocalSPSMetric(m);
                        mDNSu32 metric   = SPSMetric(cr->resrec.rdata->u.name.c);
                        if (metric >= mymetric)
                        {
                            allowSleep = mDNSfalse;
                            mDNS_snprintf(reason, sizeof(reason), "No sleep proxy server with better metric on %s", intf->ifname);
                            LogInfo("mDNS_UpdateAllowSleep: Sleep disabled because %s has no sleep proxy server with a better metric", intf->ifname);
                            break;
                        }
                    }
                }
            }
        }
    }

    // Call the platform code to enable/disable sleep
    mDNSPlatformSetAllowSleep(m, allowSleep, reason);
#else
    (void) m;
#endif /* !defined(IDLESLEEPCONTROL_DISABLED) */
}

mDNSlocal mDNSBool mDNSUpdateOkToSend(mDNS *const m, AuthRecord *rr, NetworkInterfaceInfo *const intf, mDNSu32 scopeid)
{
    // If it is not a uDNS record, check to see if the updateid is zero. "updateid" is cleared when we have
    // sent the resource record on all the interfaces. If the update id is not zero, check to see if it is time
    // to send.
    if (AuthRecord_uDNS(rr) || (rr->AuthFlags & AuthFlagsWakeOnly) || mDNSOpaque16IsZero(rr->updateid) || 
        m->timenow - (rr->LastAPTime + rr->ThisAPInterval) < 0)
    {
        return mDNSfalse;
    }

    // If we have a pending registration for "scopeid", it is ok to send the update on that interface.
    // If the scopeid is too big to check for validity, we don't check against updateIntID. When
    // we successfully update on all the interfaces (with whatever set in "rr->updateIntID"), we clear
    // updateid and we should have returned from above.
    //
    // Note: scopeid is the same as intf->InterfaceID. It is passed in so that we don't have to call the
    // platform function to extract the value from "intf" everytime.

    if ((scopeid >= (sizeof(rr->updateIntID) * mDNSNBBY) || bit_get_opaque64(rr->updateIntID, scopeid)) &&
        (!rr->resrec.InterfaceID || rr->resrec.InterfaceID == intf->InterfaceID))
        return mDNStrue;

    return mDNSfalse;
}

mDNSexport void UpdateRMACCallback(mDNS *const m, void *context)
{
    IPAddressMACMapping *addrmap = (IPAddressMACMapping *)context ;
    m->CurrentRecord = m->ResourceRecords;

    if (!addrmap)
    {
        LogMsg("UpdateRMACCallback: Address mapping is NULL");
        return;
    }

    while (m->CurrentRecord)
    {
        AuthRecord *rr = m->CurrentRecord;
        // If this is a keepalive record and the remote IP address matches, update the RData
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (mDNS_KeepaliveRecord(&rr->resrec))
        {
            mDNSAddr raddr;
            getKeepaliveRaddr(m, rr, &raddr);
            if (mDNSSameAddress(&raddr, &addrmap->ipaddr))
            {
                UpdateKeepaliveRData(m, rr, mDNSNULL, mDNStrue, (char *)(addrmap->ethaddr));
            }
        }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        m->CurrentRecord = rr->next;
    }

    if (addrmap)
    {
        mDNSPlatformMemFree(addrmap);
    }
}

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSexport mStatus UpdateKeepaliveRData(mDNS *const m, AuthRecord *rr, NetworkInterfaceInfo *const intf, mDNSBool updateMac, char *ethAddr)
{
    mDNSu16 newrdlength;
    mDNSAddr laddr, raddr;
    mDNSEthAddr eth;
    mDNSIPPort lport, rport;
    mDNSu32 timeout, seq, ack;
    mDNSu16 win;
    UTF8str255 txt;
    int rdsize;
    RData *newrd;
    mDNSTCPInfo mti;
    mStatus ret;

    // Note: If we fail to update the  DNS NULL  record with additional information in this function, it will be registered
    // with the SPS like any other record. SPS will not send keepalives if it does not have additional information.
    mDNS_ExtractKeepaliveInfo(rr, &timeout, &laddr, &raddr, &eth, &seq, &ack, &lport, &rport, &win);
    if (!timeout || mDNSAddressIsZero(&laddr) || mDNSAddressIsZero(&raddr) || mDNSIPPortIsZero(lport) ||
        mDNSIPPortIsZero(rport))
    {
        LogMsg("UpdateKeepaliveRData: not a valid record %s for keepalive %#a:%d %#a:%d", ARDisplayString(m, rr), &laddr, lport.NotAnInteger, &raddr, rport.NotAnInteger);
        return mStatus_UnknownErr;
    }

    if (updateMac)
    {
        if (laddr.type == mDNSAddrType_IPv4)
            newrdlength = mDNS_snprintf((char *)&txt.c[1], sizeof(txt.c) - 1, "t=%d i=%d c=%d h=%#a d=%#a l=%u r=%u m=%s", timeout, kKeepaliveRetryInterval, kKeepaliveRetryCount, &laddr, &raddr, mDNSVal16(lport), mDNSVal16(rport), ethAddr);
        else
            newrdlength = mDNS_snprintf((char *)&txt.c[1], sizeof(txt.c) - 1, "t=%d i=%d c=%d H=%#a D=%#a l=%u r=%u m=%s", timeout, kKeepaliveRetryInterval, kKeepaliveRetryCount, &laddr, &raddr,  mDNSVal16(lport), mDNSVal16(rport), ethAddr);

    }
    else
    {
        // If this keepalive packet would be sent on a different interface than the current one that we are processing
        // now, then we don't update the DNS NULL record. But we do not prevent it from registering with the SPS. When SPS sees
        // this DNS NULL record, it does not send any keepalives as it does not have all the information
        mDNSPlatformMemZero(&mti, sizeof (mDNSTCPInfo));
        ret = mDNSPlatformRetrieveTCPInfo(m, &laddr, &lport, &raddr, &rport, &mti);
        if (ret != mStatus_NoError)
        {
            LogMsg("mDNSPlatformRetrieveTCPInfo: mDNSPlatformRetrieveTCPInfo failed %d", ret);
            return ret;
        }
        if ((intf != mDNSNULL) && (mti.IntfId != intf->InterfaceID))
        {
            LogInfo("mDNSPlatformRetrieveTCPInfo: InterfaceID  mismatch mti.IntfId = %p InterfaceID = %p",  mti.IntfId, intf->InterfaceID);
            return mStatus_BadParamErr;
        }

        if (laddr.type == mDNSAddrType_IPv4)
            newrdlength = mDNS_snprintf((char *)&txt.c[1], sizeof(txt.c) - 1, "t=%d i=%d c=%d h=%#a d=%#a l=%u r=%u m=%.6a s=%u a=%u w=%u", timeout, kKeepaliveRetryInterval, kKeepaliveRetryCount, &laddr, &raddr, mDNSVal16(lport), mDNSVal16(rport), &eth, mti.seq, mti.ack, mti.window);
        else
            newrdlength = mDNS_snprintf((char *)&txt.c[1], sizeof(txt.c) - 1, "t=%d i=%d c=%d H=%#a D=%#a l=%u r=%u m=%.6a s=%u a=%u w=%u", timeout, kKeepaliveRetryInterval, kKeepaliveRetryCount, &laddr, &raddr, mDNSVal16(lport), mDNSVal16(rport), &eth, mti.seq, mti.ack, mti.window);
    }

    // Did we insert a null byte at the end ?
    if (newrdlength == (sizeof(txt.c) - 1))
    {
        LogMsg("UpdateKeepaliveRData: could not allocate memory %s", ARDisplayString(m, rr));
        return mStatus_NoMemoryErr;
    }

    // Include the length for the null byte at the end
    txt.c[0] = newrdlength + 1;
    // Account for the first length byte and the null byte at the end
    newrdlength += 2;

    rdsize = newrdlength > sizeof(RDataBody) ? newrdlength : sizeof(RDataBody);
    newrd = mDNSPlatformMemAllocate(sizeof(RData) - sizeof(RDataBody) + rdsize);
    if (!newrd) { LogMsg("UpdateKeepaliveRData: ptr NULL"); return mStatus_NoMemoryErr; }

    newrd->MaxRDLength = (mDNSu16) rdsize;
    mDNSPlatformMemCopy(&newrd->u, txt.c, newrdlength);

    //  If we are updating the record for the first time, rdata points to rdatastorage as the rdata memory
    //  was allocated as part of the AuthRecord itself. We allocate memory when we update the AuthRecord.
    //  If the resource record has data that we allocated in a previous pass (to update MAC address),
    //  free that memory here before copying in the new data.
    if ( rr->resrec.rdata != &rr->rdatastorage)
    {
        mDNSPlatformMemFree(rr->resrec.rdata);
        LogSPS("UpdateKeepaliveRData: Freed allocated memory for keep alive packet: %s ", ARDisplayString(m, rr));
    }
    SetNewRData(&rr->resrec, newrd, newrdlength);    // Update our rdata

    LogSPS("UpdateKeepaliveRData: successfully updated the record %s", ARDisplayString(m, rr));
    return mStatus_NoError;
}
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

mDNSlocal void SendSPSRegistrationForOwner(mDNS *const m, NetworkInterfaceInfo *const intf, const mDNSOpaque16 id, const OwnerOptData *const owner)
{
    const int optspace = DNSOpt_Header_Space + DNSOpt_LeaseData_Space + DNSOpt_Owner_Space(&m->PrimaryMAC, &intf->MAC);
    const int sps = intf->NextSPSAttempt / 3;
    AuthRecord *rr;
    mDNSOpaque16 msgid;
    mDNSu32 scopeid;

    scopeid = mDNSPlatformInterfaceIndexfromInterfaceID(m, intf->InterfaceID, mDNStrue);
    if (!intf->SPSAddr[sps].type)
    {
        intf->NextSPSAttemptTime = m->timenow + mDNSPlatformOneSecond;
        if (m->NextScheduledSPRetry - intf->NextSPSAttemptTime > 0)
            m->NextScheduledSPRetry = intf->NextSPSAttemptTime;
#ifndef REMOVE_NETWAKE
#ifdef ONLY_ONE_NetWakeResolve
        LogSPS("SendSPSRegistration: %s SPS %d (%d) %##s not yet resolved", intf->ifname, intf->NextSPSAttempt, sps, intf->NetWakeResolve.qname.c);
#else       
        LogSPS("SendSPSRegistration: %s SPS %d (%d) %##s not yet resolved", intf->ifname, intf->NextSPSAttempt, sps, intf->NetWakeResolve[sps].qname.c);
#endif     
#endif // #ifndef REMOVE_NETWAKE
        goto exit;
    }

    // Mark our mDNS records (not unicast records) for transfer to SPS
    if (mDNSOpaque16IsZero(id))
    {
        // We may have to register this record over multiple interfaces and we don't want to
        // overwrite the id. We send the registration over interface X with id "IDX" and before
        // we get a response, we overwrite with id "IDY" for interface Y and we won't accept responses
        // for "IDX". Hence, we want to use the same ID across all interfaces.
        //
        // In the case of sleep proxy server transfering its records when it goes to sleep, the owner
        // option check below will set the same ID across the records from the same owner. Records
        // with different owner option gets different ID.
        msgid = mDNS_NewMessageID(m);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        for (rr = m->ResourceRecords; rr; rr=rr->next)
        {
            if (!(rr->AuthFlags & AuthFlagsWakeOnly) && rr->resrec.RecordType > kDNSRecordTypeDeregistering)
            {
                if (rr->resrec.InterfaceID == intf->InterfaceID || (!rr->resrec.InterfaceID && (rr->ForceMCast || IsLocalDomain(rr->resrec.name))))
                {
                    if (mDNSPlatformMemSame(owner, &rr->WakeUp, sizeof(*owner)))
                    {
                        rr->SendRNow = mDNSInterfaceMark;   // mark it now
                        // When we are registering on the first interface, rr->updateid is zero in which case
                        // initialize with the new ID. For subsequent interfaces, we want to use the same ID.
                        // At the end, all the updates sent across all the interfaces with the same ID.
                        if (mDNSOpaque16IsZero(rr->updateid))
                            rr->updateid = msgid;
                        else
                            msgid = rr->updateid;
                    }
                }
            }
        }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    }
    else
        msgid = id;

    while (1)
    {
        mDNSu8 *p = m->omsg.data;
        // To comply with RFC 2782, PutResourceRecord suppresses name compression for SRV records in unicast updates.
        // For now we follow that same logic for SPS registrations too.
        // If we decide to compress SRV records in SPS registrations in the future, we can achieve that by creating our
        // initial DNSMessage with h.flags set to zero, and then update it to UpdateReqFlags right before sending the packet.
        InitializeDNSMessage(&m->omsg.h, msgid, UpdateReqFlags);

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->SendRNow || mDNSUpdateOkToSend(m, rr, intf, scopeid))
            {
                if (mDNSPlatformMemSame(owner, &rr->WakeUp, sizeof(*owner)))
                {
                    mDNSu8 *newptr;
                    const mDNSu8 *const limit = m->omsg.data + (m->omsg.h.mDNS_numUpdates ? NormalMaxDNSMessageData : AbsoluteMaxDNSMessageData) - optspace;

                    // If we can't update the keepalive record, don't send it
                    if (mDNS_KeepaliveRecord(&rr->resrec) && (UpdateKeepaliveRData(m, rr, intf, mDNSfalse, mDNSNULL) != mStatus_NoError))
                    {
                        if (scopeid < (sizeof(rr->updateIntID) * mDNSNBBY))
                        {
                            bit_clr_opaque64(rr->updateIntID, scopeid);
                        }
                        rr->SendRNow = mDNSNULL;
                        continue;
                    }
                    if (rr->resrec.RecordType & kDNSRecordTypeUniqueMask)
                        rr->resrec.rrclass |= kDNSClass_UniqueRRSet;    // Temporarily set the 'unique' bit so PutResourceRecord will set it
                    newptr = PutResourceRecordTTLWithLimit(&m->omsg, p, &m->omsg.h.mDNS_numUpdates, &rr->resrec, rr->resrec.rroriginalttl, limit);
                    rr->resrec.rrclass &= ~kDNSClass_UniqueRRSet;       // Make sure to clear 'unique' bit back to normal state
                    if (!newptr)
                        LogSPS("SendSPSRegistration put %s FAILED %d/%d %s", intf->ifname, p - m->omsg.data, limit - m->omsg.data, ARDisplayString(m, rr));
                    else
                    {
                        LogSPS("SendSPSRegistration put %s 0x%x 0x%x (updateid %d)  %s", intf->ifname, rr->updateIntID.l[1], rr->updateIntID.l[0], mDNSVal16(m->omsg.h.id), ARDisplayString(m, rr));
                        rr->SendRNow       = mDNSNULL;
                        rr->ThisAPInterval = mDNSPlatformOneSecond;
                        rr->LastAPTime     = m->timenow;
                        // should be initialized above
                        if (mDNSOpaque16IsZero(rr->updateid)) LogMsg("SendSPSRegistration: ERROR!! rr %s updateid is zero", ARDisplayString(m, rr));
                        if (m->NextScheduledResponse - (rr->LastAPTime + rr->ThisAPInterval) >= 0)
                            m->NextScheduledResponse = (rr->LastAPTime + rr->ThisAPInterval);
                        p = newptr;
                    }
                }
            }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

        if (!m->omsg.h.mDNS_numUpdates) break;
        else
        {
            AuthRecord opt;
            mDNS_SetupResourceRecord(&opt, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
            opt.resrec.rrclass    = NormalMaxDNSMessageData;
            opt.resrec.rdlength   = sizeof(rdataOPT) * 2;   // Two options in this OPT record
            opt.resrec.rdestimate = sizeof(rdataOPT) * 2;
            opt.resrec.rdata->u.opt[0].opt           = kDNSOpt_Lease;
            opt.resrec.rdata->u.opt[0].optlen        = DNSOpt_LeaseData_Space - 4;
            opt.resrec.rdata->u.opt[0].u.updatelease = DEFAULT_UPDATE_LEASE;
            if (!owner->HMAC.l[0])                                          // If no owner data,
                SetupOwnerOpt(m, intf, &opt.resrec.rdata->u.opt[1]);        // use our own interface information
            else                                                            // otherwise, use the owner data we were given
            {
                opt.resrec.rdata->u.opt[1].u.owner = *owner;
                opt.resrec.rdata->u.opt[1].opt     = kDNSOpt_Owner;
                opt.resrec.rdata->u.opt[1].optlen  = DNSOpt_Owner_Space(&owner->HMAC, &owner->IMAC) - 4;
            }
            LogSPS("SendSPSRegistration put %s %s", intf->ifname, ARDisplayString(m, &opt));
            p = PutResourceRecordTTLWithLimit(&m->omsg, p, &m->omsg.h.numAdditionals, &opt.resrec, opt.resrec.rroriginalttl, m->omsg.data + AbsoluteMaxDNSMessageData);
            if (!p)
                LogMsg("SendSPSRegistration: Failed to put OPT record (%d updates) %s", m->omsg.h.mDNS_numUpdates, ARDisplayString(m, &opt));
            else
            {
                mStatus err;

                LogSPS("SendSPSRegistration: Sending Update %s %d (%d) id %5d with %d records %d bytes to %#a:%d", intf->ifname, intf->NextSPSAttempt, sps,
                       mDNSVal16(m->omsg.h.id), m->omsg.h.mDNS_numUpdates, p - m->omsg.data, &intf->SPSAddr[sps], mDNSVal16(intf->SPSPort[sps]));
                // if (intf->NextSPSAttempt < 5) m->omsg.h.flags = zeroID;	// For simulating packet loss
                err = mDNSSendDNSMessage(m, &m->omsg, p, intf->InterfaceID, mDNSNULL, &intf->SPSAddr[sps], intf->SPSPort[sps], mDNSNULL, mDNSNULL, mDNSfalse);
                if (err) LogSPS("SendSPSRegistration: mDNSSendDNSMessage err %d", err);
#ifndef REMOVE_NETWAKE
#ifdef ONLY_ONE_NetWakeResolve
                if (err && intf->SPSAddr[sps].type == mDNSAddrType_IPv4 && intf->NetWakeResolve.ThisQInterval == -1)
                {
                    LogSPS("SendSPSRegistration %d %##s failed to send to IPv4 address; will try IPv6 instead", sps, intf->NetWakeResolve.qname.c);
                    intf->NetWakeResolve.qtype = kDNSType_AAAA;
                    mDNS_StartQuery_internal(m, &intf->NetWakeResolve);
                    return;
                }
#else
	       if (err && intf->SPSAddr[sps].type == mDNSAddrType_IPv4 && intf->NetWakeResolve[sps].ThisQInterval == -1)
                {
                    LogSPS("SendSPSRegistration %d %##s failed to send to IPv4 address; will try IPv6 instead", sps, intf->NetWakeResolve[sps].qname.c);
                    intf->NetWakeResolve[sps].qtype = kDNSType_AAAA;
                    mDNS_StartQuery_internal(m, &intf->NetWakeResolve[sps]);
                    return;
                }
#endif
#endif // #ifndef REMOVE_NETWAKE
            }
        }
    }

    intf->NextSPSAttemptTime = m->timenow + mDNSPlatformOneSecond * 10;     // If successful, update NextSPSAttemptTime

exit:
    if (mDNSOpaque16IsZero(id) && intf->NextSPSAttempt < 8) intf->NextSPSAttempt++;
}

mDNSlocal mDNSBool RecordIsFirstOccurrenceOfOwner(mDNS *const m, const AuthRecord *const rr)
{
    AuthRecord *ar;
    for (ar = m->ResourceRecords; ar && ar != rr; ar=ar->next)
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (mDNSPlatformMemSame(&rr->WakeUp, &ar->WakeUp, sizeof(rr->WakeUp))) return mDNSfalse;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    return mDNStrue;
}
#ifndef REMOVE_MACHINE_SLEEP
mDNSlocal void mDNSCoreStoreProxyRR(mDNS *const m, const mDNSInterfaceID InterfaceID, AuthRecord *const rr)
{
    AuthRecord *newRR = mDNSPlatformMemAllocate(sizeof(AuthRecord));

    if (newRR == mDNSNULL)
    {
        LogSPS("%s : could not allocate memory for new resource record", __func__);
        return;
    }

    mDNSPlatformMemZero(newRR, sizeof(AuthRecord));
    mDNS_SetupResourceRecord(newRR, mDNSNULL, InterfaceID, rr->resrec.rrtype,
                             rr->resrec.rroriginalttl, rr->resrec.RecordType,
                             rr->ARType, mDNSNULL, mDNSNULL);

    AssignDomainName(&newRR->namestorage, &rr->namestorage);
    newRR->resrec.rdlength = DomainNameLength(rr->resrec.name);
    newRR->resrec.namehash = DomainNameHashValue(newRR->resrec.name);
    newRR->resrec.rrclass  = rr->resrec.rrclass;

    if (rr->resrec.rrtype == kDNSType_A)
    {
        newRR->resrec.rdata->u.ipv4 =  rr->resrec.rdata->u.ipv4;
    }
    else if (rr->resrec.rrtype == kDNSType_AAAA)
    {
        newRR->resrec.rdata->u.ipv6 = rr->resrec.rdata->u.ipv6;
    }
    SetNewRData(&newRR->resrec, mDNSNULL, 0);

    // Insert the new node at the head of the list.
    newRR->next        = m->SPSRRSet;
    m->SPSRRSet        = newRR;
    LogSPS("%s : Storing proxy record : %s ", __func__, ARDisplayString(m, rr));
}

// Some records are interface specific and some are not. The ones that are supposed to be registered
// on multiple interfaces need to be initialized with all the valid interfaces on which it will be sent.
// updateIntID bit field tells us on which interfaces we need to register this record. When we get an
// ack from the sleep proxy server, we clear the interface bit. This way, we know when a record completes
// registration on all the interfaces
mDNSlocal void SPSInitRecordsBeforeUpdate(mDNS *const m, mDNSOpaque64 updateIntID, mDNSBool *WakeOnlyService)
{
    AuthRecord *ar;
    LogSPS("SPSInitRecordsBeforeUpdate: UpdateIntID 0x%x 0x%x", updateIntID.l[1], updateIntID.l[0]);
    
    *WakeOnlyService = mDNSfalse;

    // Before we store the A and AAAA records that we are going to register with the sleep proxy,
    // make sure that the old sleep proxy records are removed.
    mDNSCoreFreeProxyRR(m);

    // For records that are registered only on a specific interface, mark only that bit as it will
    // never be registered on any other interface. For others, it should be sent on all interfaces.
    for (ar = m->ResourceRecords; ar; ar=ar->next)
    {
        ar->updateIntID = zeroOpaque64;
        ar->updateid    = zeroID;
        if (AuthRecord_uDNS(ar))
        {
            continue;
        }
        if (ar->AuthFlags & AuthFlagsWakeOnly)
        {
            if (ar->resrec.RecordType == kDNSRecordTypeShared && ar->RequireGoodbye)
            {
                ar->ImmedAnswer = mDNSInterfaceMark;
                *WakeOnlyService = mDNStrue;
                continue;
            }
        }
        if (!ar->resrec.InterfaceID)
        {
            LogSPS("Setting scopeid (ALL) 0x%x 0x%x for %s", updateIntID.l[1], updateIntID.l[0], ARDisplayString(m, ar));
            ar->updateIntID = updateIntID;
        }
        else
        {
            // Filter records that belong to interfaces that we won't register the records on. UpdateIntID captures
            // exactly this.
            mDNSu32 scopeid = mDNSPlatformInterfaceIndexfromInterfaceID(m, ar->resrec.InterfaceID, mDNStrue);
            if ((scopeid < (sizeof(updateIntID) * mDNSNBBY)) && bit_get_opaque64(updateIntID, scopeid))
            {
                bit_set_opaque64(ar->updateIntID, scopeid);
                LogSPS("SPSInitRecordsBeforeUpdate: Setting scopeid(%d) 0x%x 0x%x for %s", scopeid, ar->updateIntID.l[1],
                    ar->updateIntID.l[0], ARDisplayString(m, ar));
            }
            else
            {
                LogSPS("SPSInitRecordsBeforeUpdate: scopeid %d beyond range or not valid for SPS registration", scopeid);
            }
        }
        // Store the A and AAAA records that we registered with the sleep proxy.
        // We will use this to prevent spurious name conflicts that may occur when we wake up
        if (ar->resrec.rrtype == kDNSType_A || ar->resrec.rrtype == kDNSType_AAAA)
        {
            mDNSCoreStoreProxyRR(m, ar->resrec.InterfaceID, ar);
        }
    }
}
#endif //#ifndef REMOVE_MACHINE_SLEEP

mDNSlocal void SendSPSRegistration(mDNS *const m, NetworkInterfaceInfo *const intf, const mDNSOpaque16 id)
{
    AuthRecord *ar;
    OwnerOptData owner = zeroOwner;

    SendSPSRegistrationForOwner(m, intf, id, &owner);

    for (ar = m->ResourceRecords; ar; ar=ar->next)
    {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (!mDNSPlatformMemSame(&owner, &ar->WakeUp, sizeof(owner)) && RecordIsFirstOccurrenceOfOwner(m, ar))
        {
            owner = ar->WakeUp;
#else
        if (RecordIsFirstOccurrenceOfOwner(m, ar))
        {
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            SendSPSRegistrationForOwner(m, intf, id, &owner);
        }
    }
}

// RetrySPSRegistrations is called from SendResponses, with the lock held
mDNSlocal void RetrySPSRegistrations(mDNS *const m)
{
    AuthRecord *rr;
    NetworkInterfaceInfo *intf;

    // First make sure none of our interfaces' NextSPSAttemptTimes are inadvertently set to m->timenow + mDNSPlatformOneSecond * 10
    for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
        if (intf->NextSPSAttempt && intf->NextSPSAttemptTime == m->timenow + mDNSPlatformOneSecond * 10)
            intf->NextSPSAttemptTime++;

    // Retry any record registrations that are due
    for (rr = m->ResourceRecords; rr; rr=rr->next)
        if (!AuthRecord_uDNS(rr) && !mDNSOpaque16IsZero(rr->updateid) && m->timenow - (rr->LastAPTime + rr->ThisAPInterval) >= 0)
        {
            for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
            {
                // If we still have registrations pending on this interface, send it now
                mDNSu32 scopeid = mDNSPlatformInterfaceIndexfromInterfaceID(m, intf->InterfaceID, mDNStrue);
                if ((scopeid >= (sizeof(rr->updateIntID) * mDNSNBBY) || bit_get_opaque64(rr->updateIntID, scopeid)) &&
                    (!rr->resrec.InterfaceID || rr->resrec.InterfaceID == intf->InterfaceID))
                {
                    LogSPS("RetrySPSRegistrations: 0x%x 0x%x (updateid %d) %s", rr->updateIntID.l[1], rr->updateIntID.l[0], mDNSVal16(rr->updateid), ARDisplayString(m, rr));
                    SendSPSRegistration(m, intf, rr->updateid);
                }
            }
        }

    // For interfaces where we did an SPS registration attempt, increment intf->NextSPSAttempt
    for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
        if (intf->NextSPSAttempt && intf->NextSPSAttemptTime == m->timenow + mDNSPlatformOneSecond * 10 && intf->NextSPSAttempt < 8)
            intf->NextSPSAttempt++;
}
#ifndef REMOVE_NETWAKE
mDNSlocal void NetWakeResolve(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{

    NetworkInterfaceInfo *intf = (NetworkInterfaceInfo *)question->QuestionContext;
#ifdef ONLY_ONE_NetWakeResolve
       int sps = 0;
#else
    int sps = (int)(question - intf->NetWakeResolve);
    (void)m;            // Unused
    LogSPS("NetWakeResolve: SPS: %d Add: %d %s", sps, AddRecord, RRDisplayString(m, answer));
#endif
    if (!AddRecord) return;                                             // Don't care about REMOVE events
    if (answer->rrtype != question->qtype) return;                      // Don't care about CNAMEs

    // if (answer->rrtype == kDNSType_AAAA && sps == 0) return;	// To test failing to resolve sleep proxy's address

    if (answer->rrtype == kDNSType_SRV)
    {
        // 1. Got the SRV record; now look up the target host's IP address
        mDNS_StopQuery(m, question);
        intf->SPSPort[sps] = answer->rdata->u.srv.port;
        AssignDomainName(&question->qname, &answer->rdata->u.srv.target);
        question->qtype = kDNSType_A;
        mDNS_StartQuery(m, question);
    }
    else if (answer->rrtype == kDNSType_A && answer->rdlength == sizeof(mDNSv4Addr))
    {
        // 2. Got an IPv4 address for the target host; record address and initiate an SPS registration if appropriate
        mDNS_StopQuery(m, question);
        question->ThisQInterval = -1;
        intf->SPSAddr[sps].type = mDNSAddrType_IPv4;
        intf->SPSAddr[sps].ip.v4 = answer->rdata->u.ipv4;
        mDNS_Lock(m);
        if (sps == intf->NextSPSAttempt/3) SendSPSRegistration(m, intf, zeroID);    // If we're ready for this result, use it now
        mDNS_Unlock(m);
    }
    else if (answer->rrtype == kDNSType_A && answer->rdlength == 0)
    {
        // 3. Got negative response -- target host apparently has IPv6 disabled -- so try looking up the target host's IPv4 address(es) instead
        mDNS_StopQuery(m, question);
        LogSPS("NetWakeResolve: SPS %d %##s has no IPv4 address, will try IPv6 instead", sps, question->qname.c);
        question->qtype = kDNSType_AAAA;
        mDNS_StartQuery(m, question);
    }
    else if (answer->rrtype == kDNSType_AAAA && answer->rdlength == sizeof(mDNSv6Addr) && mDNSv6AddressIsLinkLocal(&answer->rdata->u.ipv6))
    {
        // 4. Got the target host's IPv6 link-local address; record address and initiate an SPS registration if appropriate
        mDNS_StopQuery(m, question);
        question->ThisQInterval = -1;
        intf->SPSAddr[sps].type = mDNSAddrType_IPv6;
        intf->SPSAddr[sps].ip.v6 = answer->rdata->u.ipv6;
        mDNS_Lock(m);
        if (sps == intf->NextSPSAttempt/3) SendSPSRegistration(m, intf, zeroID);    // If we're ready for this result, use it now
        mDNS_Unlock(m);
    }

}
#endif // #ifndef REMOVE_NETWAKE
mDNSexport mDNSBool mDNSCoreHaveAdvertisedMulticastServices(mDNS *const m)
{
    AuthRecord *rr;
    for (rr = m->ResourceRecords; rr; rr=rr->next)
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (mDNS_KeepaliveRecord(&rr->resrec) || (rr->resrec.rrtype == kDNSType_SRV && !AuthRecord_uDNS(rr) && !mDNSSameIPPort(rr->resrec.rdata->u.srv.port, DiscardPort)))
            return mDNStrue;
#else
        if (rr->resrec.rrtype == kDNSType_SRV && !AuthRecord_uDNS(rr) && !mDNSSameIPPort(rr->resrec.rdata->u.srv.port, DiscardPort))
            return mDNStrue;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    return mDNSfalse;
}

#ifdef APPLE_OSX_mDNSResponder
// This function is used only in the case of local NIC proxy. For external
// sleep proxy server, we do this in SPSInitRecordsBeforeUpdate when we
// walk the resource records.
mDNSlocal void SendGoodbyesForWakeOnlyService(mDNS *const m, mDNSBool *WakeOnlyService)
{
    AuthRecord *rr;

    *WakeOnlyService = mDNSfalse;

    // Mark all the records we need to deregister and send them
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if ((rr->AuthFlags & AuthFlagsWakeOnly) && 
            rr->resrec.RecordType == kDNSRecordTypeShared && rr->RequireGoodbye)
        {
            rr->ImmedAnswer = mDNSInterfaceMark;
            *WakeOnlyService = mDNStrue;
        }
    }
}
#endif // APPLE_OSx_mDNSResponder
#ifndef REMOVE_MACHINE_SLEEP
mDNSlocal void SendSleepGoodbyes(mDNS *const m, mDNSBool AllInterfaces, mDNSBool unicast)
{

    AuthRecord *rr;
    m->SleepState = SleepState_Sleeping;

    // If AllInterfaces is not set, the caller has already marked it appropriately
    // on which interfaces this should be sent.
    if (AllInterfaces)
    {
        NetworkInterfaceInfo *intf;
        for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
        {
            intf->SendGoodbyes = 1;
        }
    }
    if (unicast)
    {
#ifndef UNICAST_DISABLED
        SleepRecordRegistrations(m);    // If we have no SPS, need to deregister our uDNS records
#endif /* UNICAST_DISABLED */
    }

    // Mark all the records we need to deregister and send them
    for (rr = m->ResourceRecords; rr; rr=rr->next)
        if (rr->resrec.RecordType == kDNSRecordTypeShared && rr->RequireGoodbye)
            rr->ImmedAnswer = mDNSInterfaceMark;
    SendResponses(m);

}
#endif // #ifndef REMOVE_MACHINE_SLEEP
/*
 * This function attempts to detect if multiple interfaces are on the same subnet.
 * It makes this determination based only on the IPv4 Addresses and subnet masks.
 * IPv6 link local addresses that are configured by default on all interfaces make
 * it hard to make this determination
 *
 * The 'real' fix for this would be to send out multicast packets over one interface
 * and conclude that multiple interfaces are on the same subnet only if these packets
 * are seen on other interfaces on the same system
 */
#ifndef REMOVE_MACHINE_SLEEP
mDNSlocal mDNSBool skipSameSubnetRegistration(mDNS *const m, mDNSInterfaceID *regID, mDNSu32 count, mDNSInterfaceID intfid)
{
    NetworkInterfaceInfo *intf;
    NetworkInterfaceInfo *newIntf;
    mDNSu32 i;

    for (newIntf = FirstInterfaceForID(m, intfid); newIntf; newIntf = newIntf->next)
    {
        if ((newIntf->InterfaceID != intfid) ||
            (newIntf->ip.type     != mDNSAddrType_IPv4))
        {
            continue;
        }
        for ( i = 0; i < count; i++)
        {
            for (intf = FirstInterfaceForID(m, regID[i]); intf; intf = intf->next)
            {
                if ((intf->InterfaceID != regID[i]) ||
                    (intf->ip.type     != mDNSAddrType_IPv4))
                {
                    continue;
                }
                if ((intf->ip.ip.v4.NotAnInteger & intf->mask.ip.v4.NotAnInteger) == (newIntf->ip.ip.v4.NotAnInteger & newIntf->mask.ip.v4.NotAnInteger))
                {
                    LogSPS("%s : Already registered for the same subnet (IPv4) for interface %s", __func__, intf->ifname);
                    return (mDNStrue);
                }
            }
        }
    }
    return (mDNSfalse);
}


mDNSlocal void DoKeepaliveCallbacks(mDNS *m)
{
    // Loop through the keepalive records and callback with an error
    m->CurrentRecord = m->ResourceRecords;
    while (m->CurrentRecord)
    {
        AuthRecord *const rr = m->CurrentRecord;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if ((mDNS_KeepaliveRecord(&rr->resrec)) && (rr->resrec.RecordType != kDNSRecordTypeDeregistering))
#else
        if (rr->resrec.RecordType != kDNSRecordTypeDeregistering)
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        {
            LogSPS("DoKeepaliveCallbacks: Invoking the callback for %s", ARDisplayString(m, rr));
            if (rr->RecordCallback)
                rr->RecordCallback(m, rr, mStatus_BadStateErr);
        }
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}
#endif // #ifndef REMOVE_MACHINE_SLEEP
// BeginSleepProcessing is called, with the lock held, from either mDNS_Execute or mDNSCoreMachineSleep
mDNSlocal void BeginSleepProcessing(mDNS *const m)
{
#ifndef REMOVE_NETWAKE
    mDNSBool SendGoodbyes = mDNStrue;
    mDNSBool WakeOnlyService  = mDNSfalse;
    mDNSBool invokeKACallback = mDNStrue;
    const CacheRecord *sps[3] = { mDNSNULL };
    mDNSOpaque64 updateIntID = zeroOpaque64;
    mDNSInterfaceID registeredIntfIDS[128];
    mDNSu32 registeredCount = 0;
    int skippedRegistrations = 0;

    m->NextScheduledSPRetry = m->timenow;

    if      (!m->SystemWakeOnLANEnabled) LogSPS("BeginSleepProcessing: m->SystemWakeOnLANEnabled is false");
    else if (!mDNSCoreHaveAdvertisedMulticastServices(m)) LogSPS("BeginSleepProcessing: No advertised services");
    else    // If we have at least one advertised service
    {
        NetworkInterfaceInfo *intf;

        // Clear out the SCDynamic entry that stores the external SPS information
        mDNSPlatformClearSPSMACAddr();

        for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
        {
            // Intialize it to false. These values make sense only when SleepState is set to Sleeping.
            intf->SendGoodbyes = 0;

            // If it is not multicast capable, we could not have possibly discovered sleep proxy
            // servers.
            if (!intf->McastTxRx || mDNSPlatformInterfaceIsD2D(intf->InterfaceID))
            {
                LogSPS("BeginSleepProcessing: %-6s Ignoring for registrations", intf->ifname);
                continue;
            }

            // If we are not capable of WOMP, then don't register with sleep proxy.
            //
            // Note: If we are not NetWake capable, we don't browse for the sleep proxy server.
            // We might find sleep proxy servers in the cache and start a resolve on them.
            // But then if the interface goes away, we won't stop these questions because
            // mDNS_DeactivateNetWake_internal assumes that a browse has been started for it
            // to stop both the browse and resolve questions.
            if (!intf->NetWake)
            {
                LogSPS("BeginSleepProcessing: %-6s not capable of magic packet wakeup", intf->ifname);
                intf->SendGoodbyes = 1;
                skippedRegistrations++;
                continue;
            }

            // Check if we have already registered with a sleep proxy for this subnet
            if (skipSameSubnetRegistration(m, registeredIntfIDS, registeredCount, intf->InterfaceID))
            {
                LogSPS("%s : Skipping sleep proxy registration on %s", __func__, intf->ifname);
                continue;
            }

#if APPLE_OSX_mDNSResponder
            else if (SupportsInNICProxy(intf))
            {
                if (ActivateLocalProxy(m, intf) == mStatus_NoError)
                {
                    SendGoodbyesForWakeOnlyService(m, &WakeOnlyService);
                    SendGoodbyes = mDNSfalse;
                    invokeKACallback  = mDNSfalse;
                    LogSPS("BeginSleepProcessing: %-6s using local proxy", intf->ifname);
                    // This will leave m->SleepState set to SleepState_Transferring,
                    // which is okay because with no outstanding resolves, or updates in flight,
                    // mDNSCoreReadyForSleep() will conclude correctly that all the updates have already completed

                    registeredIntfIDS[registeredCount] = intf->InterfaceID;
                    registeredCount++;
                }
            }
#endif // APPLE_OSX_mDNSResponder
            else
            {
#if APPLE_OSX_mDNSResponder
                // If on battery, do not attempt to offload to external sleep proxies
                if (m->SystemWakeOnLANEnabled == mDNS_WakeOnBattery)
                {
                    LogSPS("BegingSleepProcessing: Not connected to AC power - Not registering with an external sleep proxy.");
                    return;
                }
#endif // APPLE_OSX_mDNSResponder
                FindSPSInCache(m, &intf->NetWakeBrowse, sps);
                if (!sps[0]) LogSPS("BeginSleepProcessing: %-6s %#a No Sleep Proxy Server found (Next Browse Q in %d, interval %d)",
                                    intf->ifname, &intf->ip, NextQSendTime(&intf->NetWakeBrowse) - m->timenow, intf->NetWakeBrowse.ThisQInterval);
                else
                {
                    int i;
                    mDNSu32 scopeid;
                    SendGoodbyes = mDNSfalse;
                    intf->NextSPSAttempt = 0;
                    intf->NextSPSAttemptTime = m->timenow + mDNSPlatformOneSecond;

                    scopeid = mDNSPlatformInterfaceIndexfromInterfaceID(m, intf->InterfaceID, mDNStrue);
                    // Now we know for sure that we have to wait for registration to complete on this interface.
                    if (scopeid < (sizeof(updateIntID) * mDNSNBBY))
                        bit_set_opaque64(updateIntID, scopeid);
#ifdef ONLY_ONE_NetWakeResolve
                    // Don't need to set m->NextScheduledSPRetry here because we already set "m->NextScheduledSPRetry = m->timenow" above
                    for (i=0; i<3; i++)
                    {
#if ForceAlerts
                        if (intf->SPSAddr[i].type)
                        { LogMsg("BeginSleepProcessing: %s %d intf->SPSAddr[i].type %d", intf->ifname, i, intf->SPSAddr[i].type); *(long*)0 = 0; }
                        if (intf->NetWakeResolve.ThisQInterval >= 0)
                        { LogMsg("BeginSleepProcessing: %s %d intf->NetWakeResolve.ThisQInterval %d", intf->ifname, i, intf->NetWakeResolve.ThisQInterval); *(long*)0 = 0; }
#endif
                        intf->SPSAddr[i].type = mDNSAddrType_None;
                        if (intf->NetWakeResolve.ThisQInterval >= 0) mDNS_StopQuery(m, &intf->NetWakeResolve);
                        intf->NetWakeResolve.ThisQInterval = -1;
                        if (sps[i])
                        {
                            LogSPS("BeginSleepProcessing: %-6s Found Sleep Proxy Server %d TTL %d %s", intf->ifname, i, sps[i]->resrec.rroriginalttl, CRDisplayString(m, sps[i]));
                            mDNS_SetupQuestion(&intf->NetWakeResolve, intf->InterfaceID, &sps[i]->resrec.rdata->u.name, kDNSType_SRV, NetWakeResolve, intf);
                            intf->NetWakeResolve.ReturnIntermed = mDNStrue;
                            mDNS_StartQuery_internal(m, &intf->NetWakeResolve);

                            // If we are registering with a Sleep Proxy for a new subnet, add it to our list
                            registeredIntfIDS[registeredCount] = intf->InterfaceID;
                            registeredCount++;
                        }
                    }

#else
                    // Don't need to set m->NextScheduledSPRetry here because we already set "m->NextScheduledSPRetry = m->timenow" above
                    for (i=0; i<3; i++)
                    {
#if ForceAlerts
                        if (intf->SPSAddr[i].type)
                        { LogMsg("BeginSleepProcessing: %s %d intf->SPSAddr[i].type %d", intf->ifname, i, intf->SPSAddr[i].type); *(long*)0 = 0; }
                        if (intf->NetWakeResolve[i].ThisQInterval >= 0)
                        { LogMsg("BeginSleepProcessing: %s %d intf->NetWakeResolve[i].ThisQInterval %d", intf->ifname, i, intf->NetWakeResolve[i].ThisQInterval); *(long*)0 = 0; }
#endif
                        intf->SPSAddr[i].type = mDNSAddrType_None;
                        if (intf->NetWakeResolve[i].ThisQInterval >= 0) mDNS_StopQuery(m, &intf->NetWakeResolve[i]);
                        intf->NetWakeResolve[i].ThisQInterval = -1;
                        if (sps[i])
                        {
                            LogSPS("BeginSleepProcessing: %-6s Found Sleep Proxy Server %d TTL %d %s", intf->ifname, i, sps[i]->resrec.rroriginalttl, CRDisplayString(m, sps[i]));
                            mDNS_SetupQuestion(&intf->NetWakeResolve[i], intf->InterfaceID, &sps[i]->resrec.rdata->u.name, kDNSType_SRV, NetWakeResolve, intf);
                            intf->NetWakeResolve[i].ReturnIntermed = mDNStrue;
                            mDNS_StartQuery_internal(m, &intf->NetWakeResolve[i]);

                            // If we are registering with a Sleep Proxy for a new subnet, add it to our list
                            registeredIntfIDS[registeredCount] = intf->InterfaceID;
                            registeredCount++;
                        }
                    }
#endif
                }
            }
        }
    }

    // If we have at least one interface on which we are registering with an external sleep proxy,
    // initialize all the records appropriately.
    if (!mDNSOpaque64IsZero(&updateIntID))
        SPSInitRecordsBeforeUpdate(m, updateIntID, &WakeOnlyService);

    // Call the applicaitons that registered a keepalive record to inform them that we failed to offload
    // the records to a sleep proxy.
    if (invokeKACallback)
    {
        LogSPS("BeginSleepProcessing: Did not register with an in-NIC proxy - invoking the callbacks for KA records");
        DoKeepaliveCallbacks(m);
    }

    // SendSleepGoodbyes last two arguments control whether we send goodbyes on all
    // interfaces and also deregister unicast registrations.
    //
    // - If there are no sleep proxy servers, then send goodbyes on all interfaces
    //   for both multicast and unicast.
    // 
    // - If we skipped registrations on some interfaces, then we have already marked
    //   them appropriately above. We don't need to send goodbyes for unicast as
    //   we have registered with at least one sleep proxy.
    //
    // - If we are not planning to send any goodbyes, then check for WakeOnlyServices.
    //
    // Note: If we are planning to send goodbyes, we mark the record with mDNSInterfaceAny
    // and call SendResponses which inturn calls ShouldSendGoodbyesBeforeSleep which looks
    // at WakeOnlyServices first.
    if (SendGoodbyes)
    {
        LogSPS("BeginSleepProcessing: Not registering with Sleep Proxy Server");
        SendSleepGoodbyes(m, mDNStrue, mDNStrue);
    }
    else if (skippedRegistrations)
    {
        LogSPS("BeginSleepProcessing: Not registering with Sleep Proxy Server on all interfaces");
        SendSleepGoodbyes(m, mDNSfalse, mDNSfalse);
    }
    else if (WakeOnlyService)
    {
        // If we saw WakeOnly service above, send the goodbyes now.
        LogSPS("BeginSleepProcessing: Sending goodbyes for WakeOnlyServices");
        SendResponses(m);
    }
#endif // #ifndef REMOVE_NETWAKE
}

#ifndef REMOVE_MACHINE_SLEEP
// Call mDNSCoreMachineSleep(m, mDNStrue) when the machine is about to go to sleep.
// Call mDNSCoreMachineSleep(m, mDNSfalse) when the machine is has just woken up.
// Normally, the platform support layer below mDNSCore should call this, not the client layer above.
mDNSexport void mDNSCoreMachineSleep(mDNS *const m, mDNSBool sleep)
{
    AuthRecord *rr;

    LogSPS("%s (old state %d) at %ld", sleep ? "Sleeping" : "Waking", m->SleepState, m->timenow);

    if (sleep && !m->SleepState)        // Going to sleep
    {
        mDNS_Lock(m);
        // If we're going to sleep, need to stop advertising that we're a Sleep Proxy Server
        if (m->SPSSocket)
        {
            mDNSu8 oldstate = m->SPSState;
            mDNS_DropLockBeforeCallback();      // mDNS_DeregisterService expects to be called without the lock held, so we emulate that here
            m->SPSState = 2;
#ifndef SPC_DISABLED
            if (oldstate == 1) mDNS_DeregisterService(m, &m->SPSRecords);
#else
			(void)oldstate;
#endif
            mDNS_ReclaimLockAfterCallback();
        }

        m->SleepState = SleepState_Transferring;
        if (m->SystemWakeOnLANEnabled && m->DelaySleep)
        {
            // If we just woke up moments ago, allow ten seconds for networking to stabilize before going back to sleep
            LogSPS("mDNSCoreMachineSleep: Re-sleeping immediately after waking; will delay for %d ticks", m->DelaySleep - m->timenow);
            m->SleepLimit = NonZeroTime(m->DelaySleep + mDNSPlatformOneSecond * 10);
        }
        else
        {
            m->DelaySleep = 0;
            m->SleepLimit = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 10);
            m->mDNSStats.Sleeps++;
            BeginSleepProcessing(m);
        }

#ifndef UNICAST_DISABLED
        SuspendLLQs(m);
#endif
#if APPLE_OSX_mDNSResponder
        RemoveAutoTunnel6Record(m);
#endif
        LogSPS("mDNSCoreMachineSleep: m->SleepState %d (%s) seq %d", m->SleepState,
               m->SleepState == SleepState_Transferring ? "Transferring" :
               m->SleepState == SleepState_Sleeping     ? "Sleeping"     : "?", m->SleepSeqNum);
        mDNS_Unlock(m);
    }
    else if (!sleep)        // Waking up
    {
        mDNSu32 slot;
        CacheGroup *cg;
        CacheRecord *cr;
        NetworkInterfaceInfo *intf;
        mDNSs32 currtime, diff;

        mDNS_Lock(m);
        // Reset SleepLimit back to 0 now that we're awake again.
        m->SleepLimit = 0;

        // If we were previously sleeping, but now we're not, increment m->SleepSeqNum to indicate that we're entering a new period of wakefulness
        if (m->SleepState != SleepState_Awake)
        {
            m->SleepState = SleepState_Awake;
            m->SleepSeqNum++;
            // If the machine wakes and then immediately tries to sleep again (e.g. a maintenance wake)
            // then we enforce a minimum delay of 16 seconds before we begin sleep processing.
            // This is to allow time for the Ethernet link to come up, DHCP to get an address, mDNS to issue queries, etc.,
            // before we make our determination of whether there's a Sleep Proxy out there we should register with.
            m->DelaySleep = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 16);
        }

        if (m->SPSState == 3)
        {
            m->SPSState = 0;
            mDNSCoreBeSleepProxyServer_internal(m, m->SPSType, m->SPSPortability, m->SPSMarginalPower, m->SPSTotalPower, m->SPSFeatureFlags);
        }
        m->mDNSStats.Wakes++;

        // ... and the same for NextSPSAttempt
        for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next)) intf->NextSPSAttempt = -1;

        // Restart unicast and multicast queries
        mDNSCoreRestartQueries(m);

        // and reactivtate service registrations
        m->NextSRVUpdate = NonZeroTime(m->timenow + mDNSPlatformOneSecond);
        LogInfo("mDNSCoreMachineSleep waking: NextSRVUpdate in %d %d", m->NextSRVUpdate - m->timenow, m->timenow);

        // 2. Re-validate our cache records
        currtime = mDNSPlatformUTC();

#if APPLE_OSX_mDNSResponder
        // start time of this statistics gathering interval
        m->StatStartTime = currtime;
#endif // APPLE_OSX_mDNSResponder

        diff = currtime - m->TimeSlept;
        FORALL_CACHERECORDS(slot, cg, cr)
        {
            // Temporary fix: For unicast cache records, look at how much time we slept.
            // Adjust the RecvTime by the amount of time we slept so that we age the
            // cache record appropriately. If it is expired already, purge. If there
            // is a network change that happens after the wakeup, we might purge the
            // cache anyways and this helps only in the case where there are no network
            // changes across sleep/wakeup transition.
            //
            // Note: If there is a network/DNS server change that already happened and
            // these cache entries are already refreshed and we are getting a delayed
            // wake up notification, we might adjust the TimeRcvd based on the time slept
            // now which can cause the cache to purge pre-maturely. As this is not a very
            // common case, this should happen rarely.
            if (!cr->resrec.InterfaceID)
            {
                if (diff > 0)
                {
                    mDNSu32 uTTL = RRUnadjustedTTL(cr->resrec.rroriginalttl);
                    const mDNSs32 remain = uTTL - (m->timenow - cr->TimeRcvd) / mDNSPlatformOneSecond;

                    // -if we have slept longer than the remaining TTL, purge and start fresh. 
                    // -if we have been sleeping for a long time, we could reduce TimeRcvd below by
                    //  a sufficiently big value which could cause the value to go into the future
                    //  because of the signed comparison of time. For this to happen, we should have been
                    //  sleeping really long (~24 days). For now, we want to be conservative and flush even
                    //  if we have slept for more than two days.

                    if (diff >= remain || diff > (2 * 24 * 3600))
                    {
                        LogInfo("mDNSCoreMachineSleep: %s: Purging cache entry SleptTime %d, Remaining TTL %d",
                            CRDisplayString(m, cr), diff, remain);
                        mDNS_PurgeCacheResourceRecord(m, cr);
                        continue;
                    }
                    cr->TimeRcvd -= (diff * mDNSPlatformOneSecond);
                    if (m->timenow - (cr->TimeRcvd + ((mDNSs32)uTTL * mDNSPlatformOneSecond)) >= 0)
                    {
                        LogInfo("mDNSCoreMachineSleep: %s: Purging after adjusting the remaining TTL %d by %d seconds",
                            CRDisplayString(m, cr), remain, diff);
                        mDNS_PurgeCacheResourceRecord(m, cr);
                    }
                    else
                    {
                        LogInfo("mDNSCoreMachineSleep: %s: Adjusted the remain ttl %u by %d seconds", CRDisplayString(m, cr), remain, diff);
                    }
                }
            }
            else
            {
                mDNS_Reconfirm_internal(m, cr, kDefaultReconfirmTimeForWake);
            }
        }

        // 3. Retrigger probing and announcing for all our authoritative records
        for (rr = m->ResourceRecords; rr; rr=rr->next)
        {
            if (AuthRecord_uDNS(rr))
            {
                ActivateUnicastRegistration(m, rr);
            }
            else
            {
                mDNSCoreRestartRegistration(m, rr, -1);
            }
        }

        // 4. Refresh NAT mappings
        // We don't want to have to assume that all hardware can necessarily keep accurate
        // track of passage of time while asleep, so on wake we refresh our NAT mappings.
        // We typically wake up with no interfaces active, so there's no need to rush to try to find our external address.
        // But if we do get a network configuration change, mDNSMacOSXNetworkChanged will call uDNS_SetupDNSConfig, which
        // will call mDNS_SetPrimaryInterfaceInfo, which will call RecreateNATMappings to refresh them, potentially sooner
        // than five seconds from now.
        LogInfo("mDNSCoreMachineSleep: recreating NAT mappings in 5 seconds");
#ifndef REMOVE_NAT_FUCTIONALITY
        RecreateNATMappings(m, mDNSPlatformOneSecond * 5);
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
        mDNS_Unlock(m);
    }
}
#endif // #ifndef REMOVE_MACHINE_SLEEP
mDNSexport mDNSBool mDNSCoreReadyForSleep(mDNS *m, mDNSs32 now)
{
#ifndef REMOVE_NETWAKE
    DNSQuestion *q;
    AuthRecord *rr;
    NetworkInterfaceInfo *intf;

    mDNS_Lock(m);

    if (m->DelaySleep) goto notready;

    // If we've not hit the sleep limit time, and it's not time for our next retry, we can skip these checks
    if (m->SleepLimit - now > 0 && m->NextScheduledSPRetry - now > 0) goto notready;

    m->NextScheduledSPRetry = now + 0x40000000UL;

    // See if we might need to retransmit any lost Sleep Proxy Registrations
    for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
        if (intf->NextSPSAttempt >= 0)
        {
            if (now - intf->NextSPSAttemptTime >= 0)
            {
                LogSPS("mDNSCoreReadyForSleep: retrying for %s SPS %d try %d",
                       intf->ifname, intf->NextSPSAttempt/3, intf->NextSPSAttempt);
                SendSPSRegistration(m, intf, zeroID);
                // Don't need to "goto notready" here, because if we do still have record registrations
                // that have not been acknowledged yet, we'll catch that in the record list scan below.
            }
            else
            if (m->NextScheduledSPRetry - intf->NextSPSAttemptTime > 0)
                m->NextScheduledSPRetry = intf->NextSPSAttemptTime;
        }

    // Scan list of interfaces, and see if we're still waiting for any sleep proxy resolves to complete
    for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
    {
        int sps = (intf->NextSPSAttempt == 0) ? 0 : (intf->NextSPSAttempt-1)/3;
#ifdef ONLY_ONE_NetWakeResolve
         if (intf->NetWakeResolve.ThisQInterval >= 0)
        {
            LogSPS("mDNSCoreReadyForSleep: waiting for SPS Resolve %s %##s (%s)",
                   intf->ifname, intf->NetWakeResolve.qname.c, DNSTypeName(intf->NetWakeResolve.qtype));
            goto spsnotready;
        }
#else
        if (intf->NetWakeResolve[sps].ThisQInterval >= 0)
        {
            LogSPS("mDNSCoreReadyForSleep: waiting for SPS Resolve %s %##s (%s)",
                   intf->ifname, intf->NetWakeResolve[sps].qname.c, DNSTypeName(intf->NetWakeResolve[sps].qtype));
            goto spsnotready;
        }
#endif
    }

    // Scan list of registered records
    for (rr = m->ResourceRecords; rr; rr = rr->next)
        if (!AuthRecord_uDNS(rr))
            if (!mDNSOpaque64IsZero(&rr->updateIntID))
            { LogSPS("mDNSCoreReadyForSleep: waiting for SPS updateIntID 0x%x 0x%x (updateid %d) %s", rr->updateIntID.l[1], rr->updateIntID.l[0], mDNSVal16(rr->updateid), ARDisplayString(m,rr)); goto spsnotready; }

    // Scan list of private LLQs, and make sure they've all completed their handshake with the server
    for (q = m->Questions; q; q = q->next)
        if (!mDNSOpaque16IsZero(q->TargetQID) && q->LongLived && q->ReqLease == 0 && q->tcp)
        {
            LogSPS("mDNSCoreReadyForSleep: waiting for LLQ %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
            goto notready;
        }

    // Scan list of registered records
    for (rr = m->ResourceRecords; rr; rr = rr->next)
        if (AuthRecord_uDNS(rr))
        {
            if (rr->state == regState_Refresh && rr->tcp)
            { LogSPS("mDNSCoreReadyForSleep: waiting for Record updateIntID 0x%x 0x%x (updateid %d) %s", rr->updateIntID.l[1], rr->updateIntID.l[0], mDNSVal16(rr->updateid), ARDisplayString(m,rr)); goto notready; }
            #if APPLE_OSX_mDNSResponder
            if (!RecordReadyForSleep(m, rr)) { LogSPS("mDNSCoreReadyForSleep: waiting for %s", ARDisplayString(m, rr)); goto notready; }
            #endif
        }

    mDNS_Unlock(m);
    return mDNStrue;

spsnotready:

    // If we failed to complete sleep proxy registration within ten seconds, we give up on that
    // and allow up to ten seconds more to complete wide-area deregistration instead
    if (now - m->SleepLimit >= 0)
    {
        LogMsg("Failed to register with SPS, now sending goodbyes");

        for (intf = GetFirstActiveInterface(m->HostInterfaces); intf; intf = GetFirstActiveInterface(intf->next))
            if (intf->NetWakeBrowse.ThisQInterval >= 0)
            {
#ifdef ONLY_ONE_NetWakeResolve
                LogSPS("ReadyForSleep mDNS_DeactivateNetWake %s %##s (%s)",
                       intf->ifname, intf->NetWakeResolve.qname.c, DNSTypeName(intf->NetWakeResolve.qtype));
#else
                LogSPS("ReadyForSleep mDNS_DeactivateNetWake %s %##s (%s)",
                       intf->ifname, intf->NetWakeResolve[0].qname.c, DNSTypeName(intf->NetWakeResolve[0].qtype));
#endif
                mDNS_DeactivateNetWake_internal(m, intf);
            }

        for (rr = m->ResourceRecords; rr; rr = rr->next)
            if (!AuthRecord_uDNS(rr))
                if (!mDNSOpaque64IsZero(&rr->updateIntID))
                {
                    LogSPS("ReadyForSleep clearing updateIntID 0x%x 0x%x (updateid %d) for %s", rr->updateIntID.l[1], rr->updateIntID.l[0], mDNSVal16(rr->updateid), ARDisplayString(m, rr));
                    rr->updateIntID = zeroOpaque64;
                }

        // We'd really like to allow up to ten seconds more here,
        // but if we don't respond to the sleep notification within 30 seconds
        // we'll be put back to sleep forcibly without the chance to schedule the next maintenance wake.
        // Right now we wait 16 sec after wake for all the interfaces to come up, then we wait up to 10 seconds
        // more for SPS resolves and record registrations to complete, which puts us at 26 seconds.
        // If we allow just one more second to send our goodbyes, that puts us at 27 seconds.
        m->SleepLimit = now + mDNSPlatformOneSecond * 1;

        SendSleepGoodbyes(m, mDNStrue, mDNStrue);
    }

notready:
    mDNS_Unlock(m);
#endif //#ifndef REMOVE_NETWAKE
    return mDNSfalse;
}

mDNSexport mDNSs32 mDNSCoreIntervalToNextWake(mDNS *const m, mDNSs32 now)
{
    AuthRecord *ar;

    // Even when we have no wake-on-LAN-capable interfaces, or we failed to find a sleep proxy, or we have other
    // failure scenarios, we still want to wake up in at most 120 minutes, to see if the network environment has changed.
    // E.g. we might wake up and find no wireless network because the base station got rebooted just at that moment,
    // and if that happens we don't want to just give up and go back to sleep and never try again.
    mDNSs32 e = now + (120 * 60 * mDNSPlatformOneSecond);       // Sleep for at most 120 minutes
#ifndef REMOVE_NAT_FUCTIONALITY
    NATTraversalInfo *nat;
    for (nat = m->NATTraversals; nat; nat=nat->next)
        if (nat->Protocol && nat->ExpiryTime && nat->ExpiryTime - now > mDNSPlatformOneSecond*4)
        {
            mDNSs32 t = nat->ExpiryTime - (nat->ExpiryTime - now) / 10;     // Wake up when 90% of the way to the expiry time
            if (e - t > 0) e = t;
            LogSPS("ComputeWakeTime: %p %s Int %5d Ext %5d Err %d Retry %5d Interval %5d Expire %5d Wake %5d",
                   nat, nat->Protocol == NATOp_MapTCP ? "TCP" : "UDP",
                   mDNSVal16(nat->IntPort), mDNSVal16(nat->ExternalPort), nat->Result,
                   nat->retryPortMap ? (nat->retryPortMap - now) / mDNSPlatformOneSecond : 0,
                   nat->retryInterval / mDNSPlatformOneSecond,
                   nat->ExpiryTime ? (nat->ExpiryTime - now) / mDNSPlatformOneSecond : 0,
                   (t - now) / mDNSPlatformOneSecond);
        }
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    // This loop checks both the time we need to renew wide-area registrations,
    // and the time we need to renew Sleep Proxy registrations
    for (ar = m->ResourceRecords; ar; ar = ar->next)
        if (ar->expire && ar->expire - now > mDNSPlatformOneSecond*4)
        {
            mDNSs32 t = ar->expire - (ar->expire - now) / 10;       // Wake up when 90% of the way to the expiry time
            if (e - t > 0) e = t;
            LogSPS("ComputeWakeTime: %p Int %7d Next %7d Expire %7d Wake %7d %s",
                   ar, ar->ThisAPInterval / mDNSPlatformOneSecond,
                   (ar->LastAPTime + ar->ThisAPInterval - now) / mDNSPlatformOneSecond,
                   ar->expire ? (ar->expire - now) / mDNSPlatformOneSecond : 0,
                   (t - now) / mDNSPlatformOneSecond, ARDisplayString(m, ar));
        }

    return(e - now);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Packet Reception Functions
#endif

#define MustSendRecord(RR) ((RR)->NR_AnswerTo || (RR)->NR_AdditionalTo)

mDNSlocal mDNSu8 *GenerateUnicastResponse(const DNSMessage *const query, const mDNSu8 *const end,
                                          const mDNSInterfaceID InterfaceID, mDNSBool LegacyQuery, DNSMessage *const response, AuthRecord *ResponseRecords)
{
    mDNSu8          *responseptr     = response->data;
    const mDNSu8    *const limit     = response->data + sizeof(response->data);
    const mDNSu8    *ptr             = query->data;
    AuthRecord  *rr;
    mDNSu32 maxttl = 0x70000000;
    int i;

    // Initialize the response fields so we can answer the questions
    InitializeDNSMessage(&response->h, query->h.id, ResponseFlags);

    // ***
    // *** 1. Write out the list of questions we are actually going to answer with this packet
    // ***
    if (LegacyQuery)
    {
        maxttl = kStaticCacheTTL;
        for (i=0; i<query->h.numQuestions; i++)                     // For each question...
        {
            DNSQuestion q;
            ptr = getQuestion(query, ptr, end, InterfaceID, &q);    // get the question...
            if (!ptr) return(mDNSNULL);

            for (rr=ResponseRecords; rr; rr=rr->NextResponse)       // and search our list of proposed answers
            {
                if (rr->NR_AnswerTo == ptr)                         // If we're going to generate a record answering this question
                {                                                   // then put the question in the question section
                    responseptr = putQuestion(response, responseptr, limit, &q.qname, q.qtype, q.qclass);
                    if (!responseptr) { debugf("GenerateUnicastResponse: Ran out of space for questions!"); return(mDNSNULL); }
                    break;      // break out of the ResponseRecords loop, and go on to the next question
                }
            }
        }

        if (response->h.numQuestions == 0) { LogMsg("GenerateUnicastResponse: ERROR! Why no questions?"); return(mDNSNULL); }
    }

    // ***
    // *** 2. Write Answers
    // ***
    for (rr=ResponseRecords; rr; rr=rr->NextResponse)
        if (rr->NR_AnswerTo)
        {
            mDNSu8 *p = PutResourceRecordTTL(response, responseptr, &response->h.numAnswers, &rr->resrec,
                                             maxttl < rr->resrec.rroriginalttl ? maxttl : rr->resrec.rroriginalttl);
            if (p) responseptr = p;
            else { debugf("GenerateUnicastResponse: Ran out of space for answers!"); response->h.flags.b[0] |= kDNSFlag0_TC; }
        }

    // ***
    // *** 3. Write Additionals
    // ***
    for (rr=ResponseRecords; rr; rr=rr->NextResponse)
        if (rr->NR_AdditionalTo && !rr->NR_AnswerTo)
        {
            mDNSu8 *p = PutResourceRecordTTL(response, responseptr, &response->h.numAdditionals, &rr->resrec,
                                             maxttl < rr->resrec.rroriginalttl ? maxttl : rr->resrec.rroriginalttl);
            if (p) responseptr = p;
            else debugf("GenerateUnicastResponse: No more space for additionals");
        }

    return(responseptr);
}

// AuthRecord *our is our Resource Record
// CacheRecord *pkt is the Resource Record from the response packet we've witnessed on the network
// Returns 0 if there is no conflict
// Returns +1 if there was a conflict and we won
// Returns -1 if there was a conflict and we lost and have to rename
mDNSlocal int CompareRData(const AuthRecord *const our, const CacheRecord *const pkt)
{
    mDNSu8 ourdata[256], *ourptr = ourdata, *ourend;
    mDNSu8 pktdata[256], *pktptr = pktdata, *pktend;
    if (!our) { LogMsg("CompareRData ERROR: our is NULL"); return(+1); }
    if (!pkt) { LogMsg("CompareRData ERROR: pkt is NULL"); return(+1); }

    ourend = putRData(mDNSNULL, ourdata, ourdata + sizeof(ourdata), &our->resrec);
    pktend = putRData(mDNSNULL, pktdata, pktdata + sizeof(pktdata), &pkt->resrec);
    while (ourptr < ourend && pktptr < pktend && *ourptr == *pktptr) { ourptr++; pktptr++; }
    if (ourptr >= ourend && pktptr >= pktend) return(0);            // If data identical, not a conflict

    if (ourptr >= ourend) return(-1);                               // Our data ran out first; We lost
    if (pktptr >= pktend) return(+1);                               // Packet data ran out first; We won
    if (*pktptr > *ourptr) return(-1);                              // Our data is numerically lower; We lost
    if (*pktptr < *ourptr) return(+1);                              // Packet data is numerically lower; We won

    LogMsg("CompareRData ERROR: Invalid state");
    return(-1);
}

// See if we have an authoritative record that's identical to this packet record,
// whose canonical DependentOn record is the specified master record.
// The DependentOn pointer is typically used for the TXT record of service registrations
// It indicates that there is no inherent conflict detection for the TXT record
// -- it depends on the SRV record to resolve name conflicts
// If we find any identical ResourceRecords in our authoritative list, then follow their DependentOn
// pointer chain (if any) to make sure we reach the canonical DependentOn record
// If the record has no DependentOn, then just return that record's pointer
// Returns NULL if we don't have any local RRs that are identical to the one from the packet
mDNSlocal mDNSBool MatchDependentOn(const mDNS *const m, const CacheRecord *const pktrr, const AuthRecord *const master)
{
    const AuthRecord *r1;
    for (r1 = m->ResourceRecords; r1; r1=r1->next)
    {
        if (IdenticalResourceRecord(&r1->resrec, &pktrr->resrec))
        {
            const AuthRecord *r2 = r1;
            while (r2->DependentOn) r2 = r2->DependentOn;
            if (r2 == master) return(mDNStrue);
        }
    }
    for (r1 = m->DuplicateRecords; r1; r1=r1->next)
    {
        if (IdenticalResourceRecord(&r1->resrec, &pktrr->resrec))
        {
            const AuthRecord *r2 = r1;
            while (r2->DependentOn) r2 = r2->DependentOn;
            if (r2 == master) return(mDNStrue);
        }
    }
    return(mDNSfalse);
}

// Find the canonical RRSet pointer for this RR received in a packet.
// If we find any identical AuthRecord in our authoritative list, then follow its RRSet
// pointers (if any) to make sure we return the canonical member of this name/type/class
// Returns NULL if we don't have any local RRs that are identical to the one from the packet
mDNSlocal const AuthRecord *FindRRSet(const mDNS *const m, const CacheRecord *const pktrr)
{
    const AuthRecord *rr;
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (IdenticalResourceRecord(&rr->resrec, &pktrr->resrec))
        {
            while (rr->RRSet && rr != rr->RRSet) rr = rr->RRSet;
            return(rr);
        }
    }
    return(mDNSNULL);
}

// PacketRRConflict is called when we've received an RR (pktrr) which has the same name
// as one of our records (our) but different rdata.
// 1. If our record is not a type that's supposed to be unique, we don't care.
// 2a. If our record is marked as dependent on some other record for conflict detection, ignore this one.
// 2b. If the packet rr exactly matches one of our other RRs, and *that* record's DependentOn pointer
//     points to our record, ignore this conflict (e.g. the packet record matches one of our
//     TXT records, and that record is marked as dependent on 'our', its SRV record).
// 3. If we have some *other* RR that exactly matches the one from the packet, and that record and our record
//    are members of the same RRSet, then this is not a conflict.
mDNSlocal mDNSBool PacketRRConflict(const mDNS *const m, const AuthRecord *const our, const CacheRecord *const pktrr)
{
    // If not supposed to be unique, not a conflict
    if (!(our->resrec.RecordType & kDNSRecordTypeUniqueMask)) return(mDNSfalse);

    // If a dependent record, not a conflict
    if (our->DependentOn || MatchDependentOn(m, pktrr, our)) return(mDNSfalse);
    else
    {
        // If the pktrr matches a member of ourset, not a conflict
        const AuthRecord *ourset = our->RRSet ? our->RRSet : our;
        const AuthRecord *pktset = FindRRSet(m, pktrr);
        if (pktset == ourset) return(mDNSfalse);

        // For records we're proxying, where we don't know the full
        // relationship between the records, having any matching record
        // in our AuthRecords list is sufficient evidence of non-conflict
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (our->WakeUp.HMAC.l[0] && pktset) return(mDNSfalse);
#else
        if (pktset) return(mDNSfalse);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    }

    // Okay, this is a conflict
    return(mDNStrue);
}

// Note: ResolveSimultaneousProbe calls mDNS_Deregister_internal which can call a user callback, which may change
// the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSlocal void ResolveSimultaneousProbe(mDNS *const m, const DNSMessage *const query, const mDNSu8 *const end,
                                        DNSQuestion *q, AuthRecord *our)
{
    int i;
    const mDNSu8 *ptr = LocateAuthorities(query, end);
    mDNSBool FoundUpdate = mDNSfalse;

    for (i = 0; i < query->h.numAuthorities; i++)
    {
        ptr = GetLargeResourceRecord(m, query, ptr, end, q->InterfaceID, kDNSRecordTypePacketAuth, &m->rec);
        if (!ptr) break;
        if (m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && ResourceRecordAnswersQuestion(&m->rec.r.resrec, q))
        {
            FoundUpdate = mDNStrue;
            if (PacketRRConflict(m, our, &m->rec.r))
            {
                int result          = (int)our->resrec.rrclass - (int)m->rec.r.resrec.rrclass;
                if (!result) result = (int)our->resrec.rrtype  - (int)m->rec.r.resrec.rrtype;
                if (!result) result = CompareRData(our, &m->rec.r);
                if (result)
                {
                    const char *const msg = (result < 0) ? "lost:" : (result > 0) ? "won: " : "tie: ";
                    LogMsg("ResolveSimultaneousProbe: %p Pkt Record:        %08lX %s", q->InterfaceID, m->rec.r.resrec.rdatahash, CRDisplayString(m, &m->rec.r));
                    LogMsg("ResolveSimultaneousProbe: %p Our Record %d %s %08lX %s", our->resrec.InterfaceID, our->ProbeCount, msg, our->resrec.rdatahash, ARDisplayString(m, our));
                }
                // If we lost the tie-break for simultaneous probes, we don't immediately give up, because we might be seeing stale packets on the network.
                // Instead we pause for one second, to give the other host (if real) a chance to establish its name, and then try probing again.
                // If there really is another live host out there with the same name, it will answer our probes and we'll then rename.
                if (result < 0)
                {
                    m->SuppressProbes   = NonZeroTime(m->timenow + mDNSPlatformOneSecond);
                    our->ProbeCount     = DefaultProbeCountForTypeUnique;
                    our->AnnounceCount  = InitialAnnounceCount;
                    InitializeLastAPTime(m, our);
                    goto exit;
                }
            }
#if 0
            else
            {
                LogMsg("ResolveSimultaneousProbe: %p Pkt Record:        %08lX %s", q->InterfaceID, m->rec.r.resrec.rdatahash, CRDisplayString(m, &m->rec.r));
                LogMsg("ResolveSimultaneousProbe: %p Our Record %d ign:  %08lX %s", our->resrec.InterfaceID, our->ProbeCount, our->resrec.rdatahash, ARDisplayString(m, our));
            }
#endif
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }
    if (!FoundUpdate)
        LogInfo("ResolveSimultaneousProbe: %##s (%s): No Update Record found", our->resrec.name->c, DNSTypeName(our->resrec.rrtype));
exit:
    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
}

mDNSlocal CacheRecord *FindIdenticalRecordInCache(const mDNS *const m, const ResourceRecord *const pktrr)
{
    mDNSu32 slot = HashSlot(pktrr->name);
    CacheGroup *cg = CacheGroupForRecord(m, slot, pktrr);
    CacheRecord *rr;
    mDNSBool match;
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
    {
        if (!pktrr->InterfaceID)
        {
            mDNSu16 id1 = (pktrr->rDNSServer ? pktrr->rDNSServer->resGroupID : 0);
            mDNSu16 id2 = (rr->resrec.rDNSServer ? rr->resrec.rDNSServer->resGroupID : 0);
            match = (id1 == id2);
        }
        else match = (pktrr->InterfaceID == rr->resrec.InterfaceID);

        if (match && IdenticalSameNameRecord(pktrr, &rr->resrec)) break;
    }
    return(rr);
}
mDNSlocal void DeregisterProxyRecord(mDNS *const m, AuthRecord *const rr)
{
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    rr->WakeUp.HMAC    = zeroEthAddr; // Clear HMAC so that mDNS_Deregister_internal doesn't waste packets trying to wake this host
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    rr->RequireGoodbye = mDNSfalse;   // and we don't want to send goodbye for it
    mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
    SetSPSProxyListChanged(m->rec.r.resrec.InterfaceID);
}

mDNSlocal void ClearKeepaliveProxyRecords(mDNS *const m, const OwnerOptData *const owner, AuthRecord *const thelist, const mDNSInterfaceID InterfaceID)
{
    if (m->CurrentRecord)
        LogMsg("ClearIdenticalProxyRecords ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = thelist;

    // Normally, the RDATA of the keepalive record will be different each time and hence we always
    // clean up the keepalive record.
    while (m->CurrentRecord)
    {
        AuthRecord *const rr = m->CurrentRecord;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (InterfaceID == rr->resrec.InterfaceID && mDNSSameEthAddress(&owner->HMAC, &rr->WakeUp.HMAC))
#else
        if (InterfaceID == rr->resrec.InterfaceID)
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            if (mDNS_KeepaliveRecord(&m->rec.r.resrec))
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                LogSPS("ClearKeepaliveProxyRecords: Removing %3d H-MAC %.6a I-MAC %.6a %d %d %s",
                       m->ProxyRecords, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, rr->WakeUp.seq, owner->seq, ARDisplayString(m, rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                DeregisterProxyRecord(m, rr);
            }
        }
        // Mustn't advance m->CurrentRecord until *after* mDNS_Deregister_internal, because
        // new records could have been added to the end of the list as a result of that call.
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}

// Called from mDNSCoreReceiveUpdate when we get a sleep proxy registration request,
// to check our lists and discard any stale duplicates of this record we already have
mDNSlocal void ClearIdenticalProxyRecords(mDNS *const m, const OwnerOptData *const owner, AuthRecord *const thelist)
{
    if (m->CurrentRecord)
        LogMsg("ClearIdenticalProxyRecords ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = thelist;
    while (m->CurrentRecord)
    {
        AuthRecord *const rr = m->CurrentRecord;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (m->rec.r.resrec.InterfaceID == rr->resrec.InterfaceID && mDNSSameEthAddress(&owner->HMAC, &rr->WakeUp.HMAC))
#else
        if (m->rec.r.resrec.InterfaceID == rr->resrec.InterfaceID)
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            if (IdenticalResourceRecord(&rr->resrec, &m->rec.r.resrec))
            {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                LogSPS("ClearIdenticalProxyRecords: Removing %3d H-MAC %.6a I-MAC %.6a %d %d %s",
                       m->ProxyRecords, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, rr->WakeUp.seq, owner->seq, ARDisplayString(m, rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                DeregisterProxyRecord(m, rr);
            }
        // Mustn't advance m->CurrentRecord until *after* mDNS_Deregister_internal, because
        // new records could have been added to the end of the list as a result of that call.
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}

// Called from ProcessQuery when we get an mDNS packet with an owner record in it
mDNSlocal void ClearProxyRecords(mDNS *const m, const OwnerOptData *const owner, AuthRecord *const thelist)
{
    if (m->CurrentRecord)
        LogMsg("ClearProxyRecords ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = thelist;
    while (m->CurrentRecord)
    {
        AuthRecord *const rr = m->CurrentRecord;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (m->rec.r.resrec.InterfaceID == rr->resrec.InterfaceID && mDNSSameEthAddress(&owner->HMAC, &rr->WakeUp.HMAC))
            if (owner->seq != rr->WakeUp.seq || m->timenow - rr->TimeRcvd > mDNSPlatformOneSecond * 60)
#else
        if (m->rec.r.resrec.InterfaceID == rr->resrec.InterfaceID)
            if (m->timenow - rr->TimeRcvd > mDNSPlatformOneSecond * 60)
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            {
                if (rr->AddressProxy.type == mDNSAddrType_IPv6)
                {
                    // We don't do this here because we know that the host is waking up at this point, so we don't send
                    // Unsolicited Neighbor Advertisements -- even Neighbor Advertisements agreeing with what the host should be
                    // saying itself -- because it can cause some IPv6 stacks to falsely conclude that there's an address conflict.
                    #if MDNS_USE_Unsolicited_Neighbor_Advertisements
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    LogSPS("NDP Announcement -- Releasing traffic for H-MAC %.6a I-MAC %.6a %s",
                           &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m,rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    SendNDP(m, NDP_Adv, NDP_Override, rr, &rr->AddressProxy.ip.v6, &rr->WakeUp.IMAC, &AllHosts_v6, &AllHosts_v6_Eth);
                    #endif
                }
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                LogSPS("ClearProxyRecords: Removing %3d AC %2d %02X H-MAC %.6a I-MAC %.6a %d %d %s",
                       m->ProxyRecords, rr->AnnounceCount, rr->resrec.RecordType,
                       &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, rr->WakeUp.seq, owner->seq, ARDisplayString(m, rr));
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                if (rr->resrec.RecordType == kDNSRecordTypeDeregistering) rr->resrec.RecordType = kDNSRecordTypeShared;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                rr->WakeUp.HMAC = zeroEthAddr;  // Clear HMAC so that mDNS_Deregister_internal doesn't waste packets trying to wake this host
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                rr->RequireGoodbye = mDNSfalse; // and we don't want to send goodbye for it, since real host is now back and functional
                mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
                SetSPSProxyListChanged(m->rec.r.resrec.InterfaceID);
            }
        // Mustn't advance m->CurrentRecord until *after* mDNS_Deregister_internal, because
        // new records could have been added to the end of the list as a result of that call.
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}

// ProcessQuery examines a received query to see if we have any answers to give
mDNSlocal mDNSu8 *ProcessQuery(mDNS *const m, const DNSMessage *const query, const mDNSu8 *const end,
                               const mDNSAddr *srcaddr, const mDNSInterfaceID InterfaceID, mDNSBool LegacyQuery, mDNSBool QueryWasMulticast,
                               mDNSBool QueryWasLocalUnicast, DNSMessage *const response)
{
    mDNSBool FromLocalSubnet    = srcaddr && mDNS_AddressIsLocalSubnet(m, InterfaceID, srcaddr, mDNSNULL);
    AuthRecord   *ResponseRecords    = mDNSNULL;
    AuthRecord  **nrp                = &ResponseRecords;

#if POOF_ENABLED
    CacheRecord  *ExpectedAnswers    = mDNSNULL;            // Records in our cache we expect to see updated
    CacheRecord **eap                = &ExpectedAnswers;
#endif // POOF_ENABLED

    DNSQuestion  *DupQuestions       = mDNSNULL;            // Our questions that are identical to questions in this packet
    DNSQuestion **dqp                = &DupQuestions;
    mDNSs32 delayresponse      = 0;
    mDNSBool SendLegacyResponse = mDNSfalse;
    const mDNSu8 *ptr;
    mDNSu8       *responseptr        = mDNSNULL;
    AuthRecord   *rr;
    int i;
    CacheRecord *McastNSEC3Records   = mDNSNULL;

    // ***
    // *** 1. Look in Additional Section for an OPT record
    // ***
    ptr = LocateOptRR(query, end, DNSOpt_OwnerData_ID_Space);
    if (ptr)
    {
        ptr = GetLargeResourceRecord(m, query, ptr, end, InterfaceID, kDNSRecordTypePacketAdd, &m->rec);
        if (ptr && m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && m->rec.r.resrec.rrtype == kDNSType_OPT)
        {
            const rdataOPT *opt;
            const rdataOPT *const e = (const rdataOPT *)&m->rec.r.resrec.rdata->u.data[m->rec.r.resrec.rdlength];
            // Find owner sub-option(s). We verify that the MAC is non-zero, otherwise we could inadvertently
            // delete all our own AuthRecords (which are identified by having zero MAC tags on them).
            for (opt = &m->rec.r.resrec.rdata->u.opt[0]; opt < e; opt++)
                if (opt->opt == kDNSOpt_Owner && opt->u.owner.vers == 0 && opt->u.owner.HMAC.l[0])
                {
                    ClearProxyRecords(m, &opt->u.owner, m->DuplicateRecords);
                    ClearProxyRecords(m, &opt->u.owner, m->ResourceRecords);
                }
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }

    // 
    // Look in Authority Section for NSEC3 record
    // 

    mDNSParseNSEC3Records(m, query, end, InterfaceID, &McastNSEC3Records);

    debugf("%s %d", __func__, __LINE__);

    // ***
    // *** 2. Parse Question Section and mark potential answers
    // ***
    ptr = query->data;
    for (i=0; i<query->h.numQuestions; i++)                     // For each question...
    {
        mDNSBool QuestionNeedsMulticastResponse;
        int NumAnswersForThisQuestion = 0;
        AuthRecord *NSECAnswer = mDNSNULL;
        DNSQuestion pktq, *q;
        ptr = getQuestion(query, ptr, end, InterfaceID, &pktq); // get the question...
        if (!ptr) goto exit;

        pktq.AnonInfo = mDNSNULL;
        if (McastNSEC3Records)
            InitializeAnonInfoForQuestion(m, &McastNSEC3Records, &pktq);
        // The only queries that *need* a multicast response are:
        // * Queries sent via multicast
        // * from port 5353
        // * that don't have the kDNSQClass_UnicastResponse bit set
        // These queries need multicast responses because other clients will:
        // * suppress their own identical questions when they see these questions, and
        // * expire their cache records if they don't see the expected responses
        // For other queries, we may still choose to send the occasional multicast response anyway,
        // to keep our neighbours caches warm, and for ongoing conflict detection.
        QuestionNeedsMulticastResponse = QueryWasMulticast && !LegacyQuery && !(pktq.qclass & kDNSQClass_UnicastResponse);

        if (pktq.qclass & kDNSQClass_UnicastResponse)
            m->mDNSStats.UnicastBitInQueries++;
        else
            m->mDNSStats.NormalQueries++;

        // Clear the UnicastResponse flag -- don't want to confuse the rest of the code that follows later
        pktq.qclass &= ~kDNSQClass_UnicastResponse;

        // Note: We use the m->CurrentRecord mechanism here because calling ResolveSimultaneousProbe
        // can result in user callbacks which may change the record list and/or question list.
        // Also note: we just mark potential answer records here, without trying to build the
        // "ResponseRecords" list, because we don't want to risk user callbacks deleting records
        // from that list while we're in the middle of trying to build it.
        if (m->CurrentRecord)
            LogMsg("ProcessQuery ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
        m->CurrentRecord = m->ResourceRecords;
        while (m->CurrentRecord)
        {
            rr = m->CurrentRecord;
            m->CurrentRecord = rr->next;
            if (AnyTypeRecordAnswersQuestion(&rr->resrec, &pktq) && (QueryWasMulticast || QueryWasLocalUnicast || rr->AllowRemoteQuery))
            {
                m->mDNSStats.MatchingAnswersForQueries++;
                if (RRTypeAnswersQuestionType(&rr->resrec, pktq.qtype))
                {
                    if (rr->resrec.RecordType == kDNSRecordTypeUnique)
                        ResolveSimultaneousProbe(m, query, end, &pktq, rr);
                    else if (ResourceRecordIsValidAnswer(rr))
                    {
                        NumAnswersForThisQuestion++;
                        // As we have verified this question to be part of the same subset,
                        // set the anonymous data which is needed below when walk the cache
                        // records to see what answers we should be expecting. The cache records
                        // may cache only the nsec3RR and not the anonymous data itself. 
                        if (pktq.AnonInfo && rr->resrec.AnonInfo)
                            SetAnonData(&pktq, &rr->resrec, mDNStrue);

                        // Note: We should check here if this is a probe-type query, and if so, generate an immediate
                        // unicast answer back to the source, because timeliness in answering probes is important.

                        // Notes:
                        // NR_AnswerTo pointing into query packet means "answer via immediate legacy unicast" (may *also* choose to multicast)
                        // NR_AnswerTo == NR_AnswerUnicast   means "answer via delayed unicast" (to modern querier; may promote to multicast instead)
                        // NR_AnswerTo == NR_AnswerMulticast means "definitely answer via multicast" (can't downgrade to unicast later)
                        // If we're not multicasting this record because the kDNSQClass_UnicastResponse bit was set,
                        // but the multicast querier is not on a matching subnet (e.g. because of overlaid subnets on one link)
                        // then we'll multicast it anyway (if we unicast, the receiver will ignore it because it has an apparently non-local source)
                        if (QuestionNeedsMulticastResponse || (!FromLocalSubnet && QueryWasMulticast && !LegacyQuery))
                        {
                            // We only mark this question for sending if it is at least one second since the last time we multicast it
                            // on this interface. If it is more than a second, or LastMCInterface is different, then we may multicast it.
                            // This is to guard against the case where someone blasts us with queries as fast as they can.
                            if (m->timenow - (rr->LastMCTime + mDNSPlatformOneSecond) >= 0 ||
                                (rr->LastMCInterface != mDNSInterfaceMark && rr->LastMCInterface != InterfaceID))
                                rr->NR_AnswerTo = NR_AnswerMulticast;
                        }
                        else if (!rr->NR_AnswerTo) rr->NR_AnswerTo = LegacyQuery ? ptr : NR_AnswerUnicast;
                    }
                }
                else if ((rr->resrec.RecordType & kDNSRecordTypeActiveUniqueMask) && ResourceRecordIsValidAnswer(rr))
                {
                    // If we don't have any answers for this question, but we do own another record with the same name,
                    // then we'll want to mark it to generate an NSEC record on this interface
                    if (!NSECAnswer) NSECAnswer = rr;
                }
            }
        }

        if (NumAnswersForThisQuestion == 0 && NSECAnswer)
        {
            NumAnswersForThisQuestion++;
            NSECAnswer->SendNSECNow = InterfaceID;
            m->NextScheduledResponse = m->timenow;
        }

        // If we couldn't answer this question, someone else might be able to,
        // so use random delay on response to reduce collisions
        if (NumAnswersForThisQuestion == 0) delayresponse = mDNSPlatformOneSecond;  // Divided by 50 = 20ms

        if (query->h.flags.b[0] & kDNSFlag0_TC)
            m->mDNSStats.KnownAnswerMultiplePkts++;
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
        if (QuestionNeedsMulticastResponse)
#else
        // We only do the following accelerated cache expiration and duplicate question suppression processing
        // for non-truncated multicast queries with multicast responses.
        // For any query generating a unicast response we don't do this because we can't assume we will see the response.
        // For truncated queries we don't do this because a response we're expecting might be suppressed by a subsequent
        // known-answer packet, and when there's packet loss we can't safely assume we'll receive *all* known-answer packets.
        if (QuestionNeedsMulticastResponse && !(query->h.flags.b[0] & kDNSFlag0_TC))
#endif
        {
#if POOF_ENABLED
            const mDNSu32 slot = HashSlot(&pktq.qname);
            CacheGroup *cg = CacheGroupForName(m, slot, pktq.qnamehash, &pktq.qname);
            CacheRecord *cr;

            // Make a list indicating which of our own cache records we expect to see updated as a result of this query
            // Note: Records larger than 1K are not habitually multicast, so don't expect those to be updated
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
            if (!(query->h.flags.b[0] & kDNSFlag0_TC))
#endif // ENABLE_MULTI_PACKET_QUERY_SNOOPING
            for (cr = cg ? cg->members : mDNSNULL; cr; cr=cr->next)
                if (SameNameRecordAnswersQuestion(&cr->resrec, &pktq) && cr->resrec.rdlength <= SmallRecordLimit)
                    if (!cr->NextInKAList && eap != &cr->NextInKAList)
                    {
                        *eap = cr;
                        eap = &cr->NextInKAList;
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
                        if (cr->MPUnansweredQ == 0 || m->timenow - cr->MPLastUnansweredQT >= mDNSPlatformOneSecond)
                        {
                            // Although MPUnansweredQ is only really used for multi-packet query processing,
                            // we increment it for both single-packet and multi-packet queries, so that it stays in sync
                            // with the MPUnansweredKA value, which by necessity is incremented for both query types.
                            cr->MPUnansweredQ++;
                            cr->MPLastUnansweredQT = m->timenow;
                            cr->MPExpectingKA = mDNStrue;
                        }
#endif // ENABLE_MULTI_PACKET_QUERY_SNOOPING
                    }
#endif // POOF_ENABLED

            // Check if this question is the same as any of mine.
            // We only do this for non-truncated queries. Right now it would be too complicated to try
            // to keep track of duplicate suppression state between multiple packets, especially when we
            // can't guarantee to receive all of the Known Answer packets that go with a particular query.
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
            if (!(query->h.flags.b[0] & kDNSFlag0_TC))
#endif
            // For anonymous question, the duplicate suppressesion should happen if the
            // question belongs in the same group. As the group is expected to be
            // small, we don't do the optimization for now.
            if (!pktq.AnonInfo)
            {
                for (q = m->Questions; q; q=q->next)
                    if (!q->Target.type && ActiveQuestion(q) && m->timenow - q->LastQTxTime > mDNSPlatformOneSecond / 4)
                        if (!q->InterfaceID || q->InterfaceID == InterfaceID)
                            if (q->NextInDQList == mDNSNULL && dqp != &q->NextInDQList)
                                if (q->qtype == pktq.qtype &&
                                    q->qclass == pktq.qclass &&
                                    q->qnamehash == pktq.qnamehash && SameDomainName(&q->qname, &pktq.qname))
                                { *dqp = q; dqp = &q->NextInDQList; }
            }
        }
        if (pktq.AnonInfo)
        {
            FreeAnonInfo(pktq.AnonInfo);
        }
    }

    // ***
    // *** 3. Now we can safely build the list of marked answers
    // ***
    for (rr = m->ResourceRecords; rr; rr=rr->next)              // Now build our list of potential answers
        if (rr->NR_AnswerTo) {                                  // If we marked the record...
            debugf("%s call AddRecordToResponseList", __func__);
            AddRecordToResponseList(&nrp, rr, mDNSNULL);        // ... add it to the list
        }

    // ***
    // *** 4. Add additional records
    // ***
    debugf("%s %d call AddAdditionalsToResponseList", __func__, __LINE__);
    AddAdditionalsToResponseList(m, ResponseRecords, &nrp, InterfaceID);

    // ***
    // *** 5. Parse Answer Section and cancel any records disallowed by Known-Answer list
    // ***
    for (i=0; i<query->h.numAnswers; i++)                       // For each record in the query's answer section...
    {
        // Get the record...
        CacheRecord *ourcacherr;
        ptr = GetLargeResourceRecord(m, query, ptr, end, InterfaceID, kDNSRecordTypePacketAns, &m->rec);
        if (!ptr) goto exit;
        if (m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative)
        {
            // See if this Known-Answer suppresses any of our currently planned answers
            for (rr=ResponseRecords; rr; rr=rr->NextResponse)
            {
                if (MustSendRecord(rr) && ShouldSuppressKnownAnswer(&m->rec.r, rr))
                {
                    m->mDNSStats.KnownAnswerSuppressions++;
                    rr->NR_AnswerTo = mDNSNULL;
                    rr->NR_AdditionalTo = mDNSNULL;
                }
            }

            // See if this Known-Answer suppresses any previously scheduled answers (for multi-packet KA suppression)
            for (rr=m->ResourceRecords; rr; rr=rr->next)
            {
                // If we're planning to send this answer on this interface, and only on this interface, then allow KA suppression
                if (rr->ImmedAnswer == InterfaceID && ShouldSuppressKnownAnswer(&m->rec.r, rr))
                {
                    if (srcaddr->type == mDNSAddrType_IPv4)
                    {
                        if (mDNSSameIPv4Address(rr->v4Requester, srcaddr->ip.v4)) rr->v4Requester = zerov4Addr;
                    }
                    else if (srcaddr->type == mDNSAddrType_IPv6)
                    {
                        if (mDNSSameIPv6Address(rr->v6Requester, srcaddr->ip.v6)) rr->v6Requester = zerov6Addr;
                    }
                    if (mDNSIPv4AddressIsZero(rr->v4Requester) && mDNSIPv6AddressIsZero(rr->v6Requester))
                    {
                        m->mDNSStats.KnownAnswerSuppressions++;
                        rr->ImmedAnswer  = mDNSNULL;
                        rr->ImmedUnicast = mDNSfalse;
    #if MDNS_LOG_ANSWER_SUPPRESSION_TIMES
                        LogMsg("Suppressed after%4d: %s", m->timenow - rr->ImmedAnswerMarkTime, ARDisplayString(m, rr));
    #endif
                    }
                }
            }

            ourcacherr = FindIdenticalRecordInCache(m, &m->rec.r.resrec);

    #if ENABLE_MULTI_PACKET_QUERY_SNOOPING
            // See if this Known-Answer suppresses any answers we were expecting for our cache records. We do this always,
            // even if the TC bit is not set (the TC bit will *not* be set in the *last* packet of a multi-packet KA list).
            if (ourcacherr && ourcacherr->MPExpectingKA && m->timenow - ourcacherr->MPLastUnansweredQT < mDNSPlatformOneSecond)
            {
                ourcacherr->MPUnansweredKA++;
                ourcacherr->MPExpectingKA = mDNSfalse;
            }
    #endif

#if POOF_ENABLED
            // Having built our ExpectedAnswers list from the questions in this packet, we then remove
            // any records that are suppressed by the Known Answer list in this packet.
            eap = &ExpectedAnswers;
            while (*eap)
            {
                CacheRecord *cr = *eap;
                if (cr->resrec.InterfaceID == InterfaceID && IdenticalResourceRecord(&m->rec.r.resrec, &cr->resrec))
                { *eap = cr->NextInKAList; cr->NextInKAList = mDNSNULL; }
                else eap = &cr->NextInKAList;
            }
#endif // POOF_ENABLED

            // See if this Known-Answer is a surprise to us. If so, we shouldn't suppress our own query.
            if (!ourcacherr)
            {
                dqp = &DupQuestions;
                while (*dqp)
                {
                    DNSQuestion *q = *dqp;
                    if (ResourceRecordAnswersQuestion(&m->rec.r.resrec, q))
                    { *dqp = q->NextInDQList; q->NextInDQList = mDNSNULL; }
                    else dqp = &q->NextInDQList;
                }
            }
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }

    // ***
    // *** 6. Cancel any additionals that were added because of now-deleted records
    // ***
    for (rr=ResponseRecords; rr; rr=rr->NextResponse)
        if (rr->NR_AdditionalTo && !MustSendRecord(rr->NR_AdditionalTo))
        { rr->NR_AnswerTo = mDNSNULL; rr->NR_AdditionalTo = mDNSNULL; }

    // ***
    // *** 7. Mark the send flags on the records we plan to send
    // ***
    for (rr=ResponseRecords; rr; rr=rr->NextResponse)
    {
        if (rr->NR_AnswerTo)
        {
            mDNSBool SendMulticastResponse = mDNSfalse;     // Send modern multicast response
            mDNSBool SendUnicastResponse   = mDNSfalse;     // Send modern unicast response (not legacy unicast response)

#if !TARGET_OS_EMBEDDED
    // always honor kDNSQClass_UnicastResponse in embedded environment to increase reliability
    // in high multicast packet loss environments.

            // If it's been one TTL/4 since we multicast this, then send a multicast response 
            // for conflict detection, etc.
            if (m->timenow - (rr->LastMCTime + TicksTTL(rr)/4) >= 0)
            {
                SendMulticastResponse = mDNStrue;
                // If this record was marked for modern (delayed) unicast response, then mark it as promoted to
                // multicast response instead (don't want to end up ALSO setting SendUnicastResponse in the check below).
                // If this record was marked for legacy unicast response, then we mustn't change the NR_AnswerTo value.
                if (rr->NR_AnswerTo == NR_AnswerUnicast)
                {
                    m->mDNSStats.UnicastDemotedToMulticast++;
                    rr->NR_AnswerTo = NR_AnswerMulticast;
                }
            }
#endif // !TARGET_OS_EMBEDDED

            // If the client insists on a multicast response, then we'd better send one
            if      (rr->NR_AnswerTo == NR_AnswerMulticast)
            {
                m->mDNSStats.MulticastResponses++;
                SendMulticastResponse = mDNStrue;
            }
            else if (rr->NR_AnswerTo == NR_AnswerUnicast)
            {
                m->mDNSStats.UnicastResponses++;
                SendUnicastResponse   = mDNStrue;
            }
            else if (rr->NR_AnswerTo)
            {
                SendLegacyResponse    = mDNStrue;
            }
            

            if (SendMulticastResponse || SendUnicastResponse)
            {
#if MDNS_LOG_ANSWER_SUPPRESSION_TIMES
                rr->ImmedAnswerMarkTime = m->timenow;
#endif
                m->NextScheduledResponse = m->timenow;
                // If we're already planning to send this on another interface, just send it on all interfaces
                if (rr->ImmedAnswer && rr->ImmedAnswer != InterfaceID)
                    rr->ImmedAnswer = mDNSInterfaceMark;
                else
                {
                    rr->ImmedAnswer = InterfaceID;          // Record interface to send it on
                    if (SendUnicastResponse) rr->ImmedUnicast = mDNStrue;
                    if (srcaddr->type == mDNSAddrType_IPv4)
                    {
                        if      (mDNSIPv4AddressIsZero(rr->v4Requester)) rr->v4Requester = srcaddr->ip.v4;
                        else if (!mDNSSameIPv4Address(rr->v4Requester, srcaddr->ip.v4)) rr->v4Requester = onesIPv4Addr;
                    }
                    else if (srcaddr->type == mDNSAddrType_IPv6)
                    {
                        if      (mDNSIPv6AddressIsZero(rr->v6Requester)) rr->v6Requester = srcaddr->ip.v6;
                        else if (!mDNSSameIPv6Address(rr->v6Requester, srcaddr->ip.v6)) rr->v6Requester = onesIPv6Addr;
                    }
                }
            }
            // If TC flag is set, it means we should expect that additional known answers may be coming in another packet,
            // so we allow roughly half a second before deciding to reply (we've observed inter-packet delays of 100-200ms on 802.11)
            // else, if record is a shared one, spread responses over 100ms to avoid implosion of simultaneous responses
            // else, for a simple unique record reply, we can reply immediately; no need for delay
            if      (query->h.flags.b[0] & kDNSFlag0_TC) delayresponse = mDNSPlatformOneSecond * 20;            // Divided by 50 = 400ms
            else if (rr->resrec.RecordType == kDNSRecordTypeShared) delayresponse = mDNSPlatformOneSecond;      // Divided by 50 = 20ms
        }
        else if (rr->NR_AdditionalTo && rr->NR_AdditionalTo->NR_AnswerTo == NR_AnswerMulticast)
        {
            // Since additional records are an optimization anyway, we only ever send them on one interface at a time
            // If two clients on different interfaces do queries that invoke the same optional additional answer,
            // then the earlier client is out of luck
            rr->ImmedAdditional = InterfaceID;
            // No need to set m->NextScheduledResponse here
            // We'll send these additional records when we send them, or not, as the case may be
        }
    }

    // ***
    // *** 8. If we think other machines are likely to answer these questions, set our packet suppression timer
    // ***
    if (delayresponse && (!m->SuppressSending || (m->SuppressSending - m->timenow) < (delayresponse + 49) / 50))
    {
#if MDNS_LOG_ANSWER_SUPPRESSION_TIMES
        mDNSs32 oldss = m->SuppressSending;
        if (oldss && delayresponse)
            LogMsg("Current SuppressSending delay%5ld; require%5ld", m->SuppressSending - m->timenow, (delayresponse + 49) / 50);
#endif
        // Pick a random delay:
        // We start with the base delay chosen above (typically either 1 second or 20 seconds),
        // and add a random value in the range 0-5 seconds (making 1-6 seconds or 20-25 seconds).
        // This is an integer value, with resolution determined by the platform clock rate.
        // We then divide that by 50 to get the delay value in ticks. We defer the division until last
        // to get better results on platforms with coarse clock granularity (e.g. ten ticks per second).
        // The +49 before dividing is to ensure we round up, not down, to ensure that even
        // on platforms where the native clock rate is less than fifty ticks per second,
        // we still guarantee that the final calculated delay is at least one platform tick.
        // We want to make sure we don't ever allow the delay to be zero ticks,
        // because if that happens we'll fail the Bonjour Conformance Test.
        // Our final computed delay is 20-120ms for normal delayed replies,
        // or 400-500ms in the case of multi-packet known-answer lists.
        m->SuppressSending = m->timenow + (delayresponse + (mDNSs32)mDNSRandom((mDNSu32)mDNSPlatformOneSecond*5) + 49) / 50;
        if (m->SuppressSending == 0) m->SuppressSending = 1;
#if MDNS_LOG_ANSWER_SUPPRESSION_TIMES
        if (oldss && delayresponse)
            LogMsg("Set     SuppressSending to   %5ld", m->SuppressSending - m->timenow);
#endif
    }

    // ***
    // *** 9. If query is from a legacy client, or from a new client requesting a unicast reply, then generate a unicast response too
    // ***
    if (SendLegacyResponse)
        responseptr = GenerateUnicastResponse(query, end, InterfaceID, LegacyQuery, response, ResponseRecords);

exit:
    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it

    // ***
    // *** 10. Finally, clear our link chains ready for use next time
    // ***
    while (ResponseRecords)
    {
        rr = ResponseRecords;
        ResponseRecords = rr->NextResponse;
        rr->NextResponse    = mDNSNULL;
        rr->NR_AnswerTo     = mDNSNULL;
        rr->NR_AdditionalTo = mDNSNULL;
    }

#if POOF_ENABLED
    while (ExpectedAnswers)
    {
        CacheRecord *cr = ExpectedAnswers;
        ExpectedAnswers = cr->NextInKAList;
        cr->NextInKAList = mDNSNULL;

        // For non-truncated queries, we can definitively say that we should expect
        // to be seeing a response for any records still left in the ExpectedAnswers list
        if (!(query->h.flags.b[0] & kDNSFlag0_TC))
            if (cr->UnansweredQueries == 0 || m->timenow - cr->LastUnansweredTime >= mDNSPlatformOneSecond)
            {
                cr->UnansweredQueries++;
                cr->LastUnansweredTime = m->timenow;
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
                if (cr->UnansweredQueries > 1)
                    debugf("ProcessQuery: (!TC) UAQ %lu MPQ %lu MPKA %lu %s",
                           cr->UnansweredQueries, cr->MPUnansweredQ, cr->MPUnansweredKA, CRDisplayString(m, cr));
#endif // ENABLE_MULTI_PACKET_QUERY_SNOOPING
                SetNextCacheCheckTimeForRecord(m, cr);
            }

        // If we've seen multiple unanswered queries for this record,
        // then mark it to expire in five seconds if we don't get a response by then.
        if (cr->UnansweredQueries >= MaxUnansweredQueries)
        {
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
            // Only show debugging message if this record was not about to expire anyway
            if (RRExpireTime(cr) - m->timenow > 4 * mDNSPlatformOneSecond)
                debugf("ProcessQuery: (Max) UAQ %lu MPQ %lu MPKA %lu mDNS_Reconfirm() for %s",
                       cr->UnansweredQueries, cr->MPUnansweredQ, cr->MPUnansweredKA, CRDisplayString(m, cr));
#endif // ENABLE_MULTI_PACKET_QUERY_SNOOPING
            m->mDNSStats.PoofCacheDeletions++;
            mDNS_Reconfirm_internal(m, cr, kDefaultReconfirmTimeForNoAnswer);
        }
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
        // Make a guess, based on the multi-packet query / known answer counts, whether we think we
        // should have seen an answer for this. (We multiply MPQ by 4 and MPKA by 5, to allow for
        // possible packet loss of up to 20% of the additional KA packets.)
        else if (cr->MPUnansweredQ * 4 > cr->MPUnansweredKA * 5 + 8)
        {
            // We want to do this conservatively.
            // If there are so many machines on the network that they have to use multi-packet known-answer lists,
            // then we don't want them to all hit the network simultaneously with their final expiration queries.
            // By setting the record to expire in four minutes, we achieve two things:
            // (a) the 90-95% final expiration queries will be less bunched together
            // (b) we allow some time for us to witness enough other failed queries that we don't have to do our own
            mDNSu32 remain = (mDNSu32)(RRExpireTime(cr) - m->timenow) / 4;
            if (remain > 240 * (mDNSu32)mDNSPlatformOneSecond)
                remain = 240 * (mDNSu32)mDNSPlatformOneSecond;

            // Only show debugging message if this record was not about to expire anyway
            if (RRExpireTime(cr) - m->timenow > 4 * mDNSPlatformOneSecond)
                debugf("ProcessQuery: (MPQ) UAQ %lu MPQ %lu MPKA %lu mDNS_Reconfirm() for %s",
                       cr->UnansweredQueries, cr->MPUnansweredQ, cr->MPUnansweredKA, CRDisplayString(m, cr));

            if (remain <= 60 * (mDNSu32)mDNSPlatformOneSecond)
                cr->UnansweredQueries++;    // Treat this as equivalent to one definite unanswered query
            cr->MPUnansweredQ  = 0;         // Clear MPQ/MPKA statistics
            cr->MPUnansweredKA = 0;
            cr->MPExpectingKA  = mDNSfalse;

            if (remain < kDefaultReconfirmTimeForNoAnswer)
                remain = kDefaultReconfirmTimeForNoAnswer;
            mDNS_Reconfirm_internal(m, cr, remain);
        }
#endif // ENABLE_MULTI_PACKET_QUERY_SNOOPING
    }
#endif // POOF_ENABLED

    while (DupQuestions)
    {
        DNSQuestion *q = DupQuestions;
        DupQuestions = q->NextInDQList;
        q->NextInDQList = mDNSNULL;
        i = RecordDupSuppressInfo(q->DupSuppress, m->timenow, InterfaceID, srcaddr->type);
        debugf("ProcessQuery: Recorded DSI for %##s (%s) on %p/%s %d", q->qname.c, DNSTypeName(q->qtype), InterfaceID,
               srcaddr->type == mDNSAddrType_IPv4 ? "v4" : "v6", i);
    }

    if (McastNSEC3Records)
    {
        debugf("ProcessQuery: McastNSEC3Records not used");
        FreeNSECRecords(m, McastNSEC3Records);
    }

    return(responseptr);
}

mDNSlocal void mDNSCoreReceiveQuery(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end,
                                    const mDNSAddr *srcaddr, const mDNSIPPort srcport, const mDNSAddr *dstaddr, mDNSIPPort dstport,
                                    const mDNSInterfaceID InterfaceID)
{
    mDNSu8    *responseend = mDNSNULL;
    mDNSBool QueryWasLocalUnicast = srcaddr && dstaddr &&
                                    !mDNSAddrIsDNSMulticast(dstaddr) && mDNS_AddressIsLocalSubnet(m, InterfaceID, srcaddr, mDNSNULL);

    if (!InterfaceID && dstaddr && mDNSAddrIsDNSMulticast(dstaddr))
    {
        LogMsg("Ignoring Query from %#-15a:%-5d to %#-15a:%-5d on %p with "
               "%2d Question%s %2d Answer%s %2d Authorit%s %2d Additional%s %d bytes (Multicast, but no InterfaceID)",
               srcaddr, mDNSVal16(srcport), dstaddr, mDNSVal16(dstport), InterfaceID,
               msg->h.numQuestions,   msg->h.numQuestions   == 1 ? ", "   : "s,",
               msg->h.numAnswers,     msg->h.numAnswers     == 1 ? ", "   : "s,",
               msg->h.numAuthorities, msg->h.numAuthorities == 1 ? "y,  " : "ies,",
               msg->h.numAdditionals, msg->h.numAdditionals == 1 ? " "    : "s", end - msg->data);
        return;
    }

    mdnslogInfo("Received Query from %#-15a:%-5d to %#-15a:%-5d on %p with "
                  "%2d Question%s %2d Answer%s %2d Authorit%s %2d Additional%s %d bytes",
                  srcaddr, mDNSVal16(srcport), dstaddr, mDNSVal16(dstport), InterfaceID,
                  msg->h.numQuestions,   msg->h.numQuestions   == 1 ? ", "   : "s,",
                  msg->h.numAnswers,     msg->h.numAnswers     == 1 ? ", "   : "s,",
                  msg->h.numAuthorities, msg->h.numAuthorities == 1 ? "y,  " : "ies,",
                  msg->h.numAdditionals, msg->h.numAdditionals == 1 ? " "    : "s", end - msg->data);

    responseend = ProcessQuery(m, msg, end, srcaddr, InterfaceID,
                               !mDNSSameIPPort(srcport, MulticastDNSPort), mDNSAddrIsDNSMulticast(dstaddr), QueryWasLocalUnicast, &m->omsg);

    if (responseend)    // If responseend is non-null, that means we built a unicast response packet
    {
        mdnslogInfo("Unicast Response: %d Question%s, %d Answer%s, %d Additional%s to %#-15a:%d on %p/%ld",
               m->omsg.h.numQuestions,   m->omsg.h.numQuestions   == 1 ? "" : "s",
               m->omsg.h.numAnswers,     m->omsg.h.numAnswers     == 1 ? "" : "s",
               m->omsg.h.numAdditionals, m->omsg.h.numAdditionals == 1 ? "" : "s",
               srcaddr, mDNSVal16(srcport), InterfaceID, srcaddr->type);
        mDNSSendDNSMessage(m, &m->omsg, responseend, InterfaceID, mDNSNULL, srcaddr, srcport, mDNSNULL, mDNSNULL, mDNSfalse);
    }
}

#if 0
mDNSlocal mDNSBool TrustedSource(const mDNS *const m, const mDNSAddr *const srcaddr)
{
    DNSServer *s;
    (void)m; // Unused
    (void)srcaddr; // Unused
    for (s = m->DNSServers; s; s = s->next)
        if (mDNSSameAddress(srcaddr, &s->addr)) return(mDNStrue);
    return(mDNSfalse);
}
#endif

struct UDPSocket_struct
{
    mDNSIPPort port; // MUST BE FIRST FIELD -- mDNSCoreReceive expects every UDPSocket_struct to begin with mDNSIPPort port
};

mDNSlocal DNSQuestion *ExpectingUnicastResponseForQuestion(const mDNS *const m, const mDNSIPPort port, const mDNSOpaque16 id, const DNSQuestion *const question, mDNSBool tcp)
{
    DNSQuestion *q;
    for (q = m->Questions; q; q=q->next)
    {
        if (!tcp && !q->LocalSocket) continue;
        if (mDNSSameIPPort(tcp ? q->tcpSrcPort : q->LocalSocket->port, port)     &&
            mDNSSameOpaque16(q->TargetQID,         id)       &&
            q->qtype                  == question->qtype     &&
            q->qclass                 == question->qclass    &&
            q->qnamehash              == question->qnamehash &&
            SameDomainName(&q->qname, &question->qname))
            return(q);
    }
    return(mDNSNULL);
}

// This function is called when we receive a unicast response. This could be the case of a unicast response from the
// DNS server or a response to the QU query. Hence, the cache record's InterfaceId can be both NULL or non-NULL (QU case)
mDNSlocal DNSQuestion *ExpectingUnicastResponseForRecord(mDNS *const m,
                                                         const mDNSAddr *const srcaddr, const mDNSBool SrcLocal, const mDNSIPPort port, const mDNSOpaque16 id, const CacheRecord *const rr, mDNSBool tcp)
{
    DNSQuestion *q;
    (void)id;
    (void)srcaddr;

    for (q = m->Questions; q; q=q->next)
    {
        if (!q->DuplicateOf && ResourceRecordAnswersUnicastResponse(&rr->resrec, q))
        {
            if (!mDNSOpaque16IsZero(q->TargetQID))
            {
                debugf("ExpectingUnicastResponseForRecord msg->h.id %d q->TargetQID %d for %s", mDNSVal16(id), mDNSVal16(q->TargetQID), CRDisplayString(m, rr));

                if (mDNSSameOpaque16(q->TargetQID, id))
                {
                    mDNSIPPort srcp;
                    if (!tcp)
                    {
                        srcp = q->LocalSocket ? q->LocalSocket->port : zeroIPPort;
                    }
                    else
                    {
                        srcp = q->tcpSrcPort;
                    }
                    if (mDNSSameIPPort(srcp, port)) return(q);

                    //	if (mDNSSameAddress(srcaddr, &q->Target))                   return(mDNStrue);
                    //	if (q->LongLived && mDNSSameAddress(srcaddr, &q->servAddr)) return(mDNStrue); Shouldn't need this now that we have LLQType checking
                    //	if (TrustedSource(m, srcaddr))                              return(mDNStrue);
                    LogInfo("WARNING: Ignoring suspect uDNS response for %##s (%s) [q->Target %#a:%d] from %#a:%d %s",
                            q->qname.c, DNSTypeName(q->qtype), &q->Target, mDNSVal16(srcp), srcaddr, mDNSVal16(port), CRDisplayString(m, rr));
                    return(mDNSNULL);
                }
            }
            else
            {
                if (SrcLocal && q->ExpectUnicastResp && (mDNSu32)(m->timenow - q->ExpectUnicastResp) < (mDNSu32)(mDNSPlatformOneSecond*2))
                    return(q);
            }
        }
    }
    return(mDNSNULL);
}

// Return a pointer to the primary service name, skipping subtype name if present.
mDNSlocal const domainname *getPrimaryServiceName(const domainname *domainName)
{
    const domainname *primaryName = domainName;
    const domainname *subName = SkipLeadingLabels(domainName, 1);
    
    if (SameDomainLabel(subName->c, (const mDNSu8 *)mDNSSubTypeLabel))
    {
        // skip "<sub type name>._sub" portion of name
        primaryName = SkipLeadingLabels(domainName, 2);
        debugf("getPrimaryServiceName: returning %##s for _sub type", primaryName);
    }

    return primaryName;
}

// This function is not called if the packet is from us, which implies that we accept all multicast packets coming from us.
mDNSlocal mDNSBool ExpectingMulticastResponseForRecord(mDNS *const m, CacheRecord *rr, const mDNSAddr *srcaddr, mDNSBool recordAccepted,
    CacheRecord **McastNSEC3Records)
{
    DNSQuestion *q;

    // Accept A and AAAA if we accepted something before in the same packet as most likely related to the
    // service records that we may have accepted.
    if (recordAccepted && (rr->resrec.rrtype == kDNSType_A || rr->resrec.rrtype == kDNSType_AAAA))
    {
        LogInfo("ExpectingMulticastResponseForRecord:A:AAAA: accepting %s, from %#a due to same packet %d", CRDisplayString(m, rr), srcaddr, m->PktNum);
        return mDNStrue;
    }
    for (q = m->Questions; q; q=q->next)
    {
        if (!q->DuplicateOf && mDNSOpaque16IsZero(q->TargetQID))
        {
            mDNSBool ret;
            // 1. If a resource record answers question, cache it. This also will cache NSECs if it asserts
            //    non-existence of q->qtype. If we have any matching NSEC3 Records for the question, send
            //    it along with the resource record. Do it only for questions that are expecting to
            //    discover only its peers (q->AnonInfo not NULL)
            if (q->AnonInfo && McastNSEC3Records && !rr->resrec.AnonInfo)
            {
                InitializeAnonInfoForCR(m, McastNSEC3Records, rr);
            }
            ret = ResourceRecordAnswersQuestion(&rr->resrec, q);
            if (ret)
            {
                // The record and the question belong to the same subset. Set the
                // anonymous data in the cache record.
                if (q->AnonInfo && rr->resrec.AnonInfo)
                {
                    SetAnonData(q, &rr->resrec, mDNSfalse);
                }
                LogInfo("ExpectingMulticastResponseForRecord: Name and Type match, accepting %s, from %#a", CRDisplayString(m, rr), srcaddr);
                if (rr->resrec.rrtype == kDNSType_NSEC)
                    LogInfo("ExpectingMulticastResponseForRecord: record %s, question %##s (%s)", CRDisplayString(m, rr), q->qname.c, DNSTypeName(q->qtype));
                return mDNStrue;
            }
            if (rr->resrec.rrtype == kDNSType_SRV || rr->resrec.rrtype == kDNSType_TXT)
            {
                // Point to the service type in the record name
                const domainname *name = SkipLeadingLabels(rr->resrec.name, 1);

                // If question is for a sub type, just compare against the primary service type
                const domainname *primaryName = getPrimaryServiceName(&q->qname);

                // 2. If the SRV or TXT record matches the service name, then cache it. If the TXT or SRV record is
                //    before the PTR record in the packet, PTR record may not be in the cache yet and hence the logic
                //    in (3) below will fail to cache it.
                if (q->qtype == kDNSType_PTR && name && SameDomainName(primaryName, name))
                {
                    LogInfo("ExpectingMulticastResponseForRecord: Accepting %s due to PTR match, question %##s from %#a, pktnum %d",
                        CRDisplayString(m, rr), q->qname.c, srcaddr, m->PktNum);
                    return mDNStrue;
                }

                if (name)
                {
                    const mDNSu32 slot = HashSlot(name);
                    const mDNSu32 namehash = DomainNameHashValue(name);
                    CacheGroup *cg = CacheGroupForName(m, slot, namehash, name);
                    CacheRecord *cr;
     
                    // 3. Same as in (2), but look in the cache in case we don't have the PTR question.

                    for (cr = cg ? cg->members : mDNSNULL; cr; cr=cr->next)
                    {
                        if (cr->resrec.rrtype == kDNSType_PTR)
                        {
                            primaryName = getPrimaryServiceName(cr->resrec.name);

                            if (SameDomainName(primaryName, name))
                            {
                                LogInfo("ExpectingMulticastResponseForRecord: accepting %s, from %#a, pktnum %d",
                                    CRDisplayString(m, rr), srcaddr, m->PktNum);
                                return mDNStrue;
                            }
                        }
                    }
                }
            }
        }
    }
    debugf("ExpectingMulticastResponseForRecord: discarding %s, from %#a, pktnum %d", CRDisplayString(m, rr), srcaddr, m->PktNum);
    return(mDNSfalse);
}

// Certain data types need more space for in-memory storage than their in-packet rdlength would imply
// Currently this applies only to rdata types containing more than one domainname,
// or types where the domainname is not the last item in the structure.
mDNSlocal mDNSu16 GetRDLengthMem(const ResourceRecord *const rr)
{
    switch (rr->rrtype)
    {
    case kDNSType_SOA: return sizeof(rdataSOA);
    case kDNSType_RP:  return sizeof(rdataRP);
    case kDNSType_PX:  return sizeof(rdataPX);
    default:           return rr->rdlength;
    }
}

mDNSexport CacheRecord *CreateNewCacheEntry(mDNS *const m, const mDNSu32 slot, CacheGroup *cg, mDNSs32 delay, mDNSBool Add, const mDNSAddr *sourceAddress)
{
    CacheRecord *rr = mDNSNULL;
    mDNSu16 RDLength = GetRDLengthMem(&m->rec.r.resrec);

    if (!m->rec.r.resrec.InterfaceID) debugf("CreateNewCacheEntry %s", CRDisplayString(m, &m->rec.r));

    //if (RDLength > InlineCacheRDSize)
    //	LogInfo("Rdata len %4d > InlineCacheRDSize %d %s", RDLength, InlineCacheRDSize, CRDisplayString(m, &m->rec.r));

    if (!cg) cg = GetCacheGroup(m, slot, &m->rec.r.resrec); // If we don't have a CacheGroup for this name, make one now
    if (cg) rr = GetCacheRecord(m, cg, RDLength);   // Make a cache record, being careful not to recycle cg
    if (!rr) NoCacheAnswer(m, &m->rec.r);
    else
    {
        RData *saveptr = rr->resrec.rdata;      // Save the rr->resrec.rdata pointer
        *rr = m->rec.r;                         // Block copy the CacheRecord object
        rr->resrec.rdata  = saveptr;            // Restore rr->resrec.rdata after the structure assignment
        rr->resrec.name   = cg->name;           // And set rr->resrec.name to point into our CacheGroup header

        // We need to add the anonymous info before we call CacheRecordAdd so that
        // if it finds a matching question with this record, it bumps up the counters like
        // CurrentAnswers etc. Otherwise, when a cache entry gets removed, CacheRecordRmv
        // will complain.
        if (m->rec.r.resrec.AnonInfo)
        {
            rr->resrec.AnonInfo = m->rec.r.resrec.AnonInfo;
            m->rec.r.resrec.AnonInfo = mDNSNULL;
        }
        rr->DelayDelivery = delay;

        // If this is an oversized record with external storage allocated, copy rdata to external storage
        if      (rr->resrec.rdata == (RData*)&rr->smallrdatastorage && RDLength > InlineCacheRDSize)
            LogMsg("rr->resrec.rdata == &rr->rdatastorage but length > InlineCacheRDSize %##s", m->rec.r.resrec.name->c);
        else if (rr->resrec.rdata != (RData*)&rr->smallrdatastorage && RDLength <= InlineCacheRDSize)
            LogMsg("rr->resrec.rdata != &rr->rdatastorage but length <= InlineCacheRDSize %##s", m->rec.r.resrec.name->c);
        if (RDLength > InlineCacheRDSize)
            mDNSPlatformMemCopy(rr->resrec.rdata, m->rec.r.resrec.rdata, sizeofRDataHeader + RDLength);

        rr->next = mDNSNULL;                    // Clear 'next' pointer
        rr->nsec = mDNSNULL;
        rr->soa  = mDNSNULL;

        if (sourceAddress)
            rr->sourceAddress = *sourceAddress;

        if (!rr->resrec.InterfaceID)
        {
            m->rrcache_totalused_unicast += rr->resrec.rdlength;
            if (DNSSECRecordType(rr->resrec.rrtype))
                BumpDNSSECStats(m, kStatsActionIncrement, kStatsTypeMemoryUsage, rr->resrec.rdlength);
        }

        if (Add)
        {
            *(cg->rrcache_tail) = rr;               // Append this record to tail of cache slot list
            cg->rrcache_tail = &(rr->next);         // Advance tail pointer
            CacheRecordAdd(m, rr);  // CacheRecordAdd calls SetNextCacheCheckTimeForRecord(m, rr); for us
        }
        else
        {
            // Can't use the "cg->name" if we are not adding to the cache as the
            // CacheGroup may be released anytime if it is empty
            domainname *name = mDNSPlatformMemAllocate(DomainNameLength(cg->name));
            if (name)
            {
                AssignDomainName(name, cg->name);
                rr->resrec.name   = name;
            }
            else
            {
                ReleaseCacheRecord(m, rr);
                NoCacheAnswer(m, &m->rec.r);
                rr = mDNSNULL;
            }
        }
    }
    return(rr);
}

mDNSlocal void RefreshCacheRecord(mDNS *const m, CacheRecord *rr, mDNSu32 ttl)
{
    rr->TimeRcvd             = m->timenow;
    rr->resrec.rroriginalttl = ttl;
    rr->UnansweredQueries = 0;
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
    rr->MPUnansweredQ     = 0;
    rr->MPUnansweredKA    = 0;
    rr->MPExpectingKA     = mDNSfalse;
#endif
    SetNextCacheCheckTimeForRecord(m, rr);
}

mDNSexport void GrantCacheExtensions(mDNS *const m, DNSQuestion *q, mDNSu32 lease)
{
    CacheRecord *rr;
    const mDNSu32 slot = HashSlot(&q->qname);
    CacheGroup *cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
        if (rr->CRActiveQuestion == q)
        {
            //LogInfo("GrantCacheExtensions: new lease %d / %s", lease, CRDisplayString(m, rr));
            RefreshCacheRecord(m, rr, lease);
        }
}

mDNSlocal mDNSu32 GetEffectiveTTL(const uDNS_LLQType LLQType, mDNSu32 ttl)      // TTL in seconds
{
    if      (LLQType == uDNS_LLQ_Entire) ttl = kLLQ_DefLease;
    else if (LLQType == uDNS_LLQ_Events)
    {
        // If the TTL is -1 for uDNS LLQ event packet, that means "remove"
        if (ttl == 0xFFFFFFFF) ttl = 0;
        else ttl = kLLQ_DefLease;
    }
    else    // else not LLQ (standard uDNS response)
    {
        // The TTL is already capped to a maximum value in GetLargeResourceRecord, but just to be extra safe we
        // also do this check here to make sure we can't get overflow below when we add a quarter to the TTL
        if (ttl > 0x60000000UL / mDNSPlatformOneSecond) ttl = 0x60000000UL / mDNSPlatformOneSecond;

        ttl = RRAdjustTTL(ttl);

        // For mDNS, TTL zero means "delete this record"
        // For uDNS, TTL zero means: this data is true at this moment, but don't cache it.
        // For the sake of network efficiency, we impose a minimum effective TTL of 15 seconds.
        // This means that we'll do our 80, 85, 90, 95% queries at 12.00, 12.75, 13.50, 14.25 seconds
        // respectively, and then if we get no response, delete the record from the cache at 15 seconds.
        // This gives the server up to three seconds to respond between when we send our 80% query at 12 seconds
        // and when we delete the record at 15 seconds. Allowing cache lifetimes less than 15 seconds would
        // (with the current code) result in the server having even less than three seconds to respond
        // before we deleted the record and reported a "remove" event to any active questions.
        // Furthermore, with the current code, if we were to allow a TTL of less than 2 seconds
        // then things really break (e.g. we end up making a negative cache entry).
        // In the future we may want to revisit this and consider properly supporting non-cached (TTL=0) uDNS answers.
        if (ttl < 15) ttl = 15;
    }

    return ttl;
}

// When the response does not match the question directly, we still want to cache them sometimes. The current response is
// in m->rec.
mDNSlocal mDNSBool IsResponseAcceptable(mDNS *const m, const CacheRecord *crlist, DNSQuestion *q, mDNSBool *nseclist)
{
    CacheRecord *const newcr = &m->rec.r;
    ResourceRecord *rr = &newcr->resrec;
    const CacheRecord *cr;

    *nseclist = mDNSfalse;
    for (cr = crlist; cr != (CacheRecord*)1; cr = cr->NextInCFList)
    {
        domainname *target = GetRRDomainNameTarget(&cr->resrec);
        // When we issue a query for A record, the response might contain both a CNAME and A records. Only the CNAME would
        // match the question and we already created a cache entry in the previous pass of this loop. Now when we process
        // the A record, it does not match the question because the record name here is the CNAME. Hence we try to
        // match with the previous records to make it an AcceptableResponse. We have to be careful about setting the
        // DNSServer value that we got in the previous pass. This can happen for other record types like SRV also.

        if (target && cr->resrec.rdatahash == rr->namehash && SameDomainName(target, rr->name))
        {
            LogInfo("IsResponseAcceptable: Found a matching entry for %##s in the CacheFlushRecords %s", rr->name->c, CRDisplayString(m, cr));
            return (mDNStrue);
        }
    }

    // Either the question requires validation or we are validating a response with DNSSEC in which case
    // we need to accept the RRSIGs also so that we can validate the response. It is also possible that
    // we receive NSECs for our query which does not match the qname and we need to cache in that case
    // too. nseclist is set if they have to be cached as part of the negative cache record.
    if (q && DNSSECQuestion(q))
    {
        mDNSBool same = SameDomainName(&q->qname, rr->name);
        if (same && (q->qtype == rr->rrtype || rr->rrtype == kDNSType_CNAME))
        {
            LogInfo("IsResponseAcceptable: Accepting, same name and qtype %s, CR %s", DNSTypeName(q->qtype),
                    CRDisplayString(m, newcr));
            return mDNStrue;
        }
        // We cache RRSIGS if it covers the question type or NSEC.  If it covers a NSEC,
        // "nseclist" is set
        if (rr->rrtype == kDNSType_RRSIG)
        {
            RDataBody2 *const rdb = (RDataBody2 *)newcr->smallrdatastorage.data;
            rdataRRSig *rrsig = &rdb->rrsig;
            mDNSu16 typeCovered = swap16(rrsig->typeCovered);

            // Note the ordering. If we are looking up the NSEC record, then the RRSIG's typeCovered
            // would match the qtype and they are cached normally as they are not used to prove the
            // non-existence of any name. In that case, it is like any other normal dnssec validation
            // and hence nseclist should not be set.

            if (same && ((typeCovered == q->qtype) || (typeCovered == kDNSType_CNAME)))
            {
                LogInfo("IsResponseAcceptable: Accepting RRSIG %s matches question type %s", CRDisplayString(m, newcr),
                        DNSTypeName(q->qtype));
                return mDNStrue;
            }
            else if (typeCovered == kDNSType_NSEC || typeCovered == kDNSType_NSEC3)
            {
                LogInfo("IsResponseAcceptable: Accepting RRSIG %s matches %s type (nseclist = 1)", CRDisplayString(m, newcr), DNSTypeName(typeCovered));
                *nseclist = mDNStrue;
                return mDNStrue;
            }
            else if (typeCovered == kDNSType_SOA)
            {
                LogInfo("IsResponseAcceptable: Accepting RRSIG %s matches SOA type (nseclist = 1)", CRDisplayString(m, newcr));
                *nseclist = mDNStrue;
                return mDNStrue;
            }
            else return mDNSfalse;
        }
        if (rr->rrtype == kDNSType_NSEC)
        {
            if (!UNICAST_NSEC(rr))
            {
                LogMsg("IsResponseAcceptable: ERROR!! Not a unicast NSEC %s", CRDisplayString(m, newcr));
                return mDNSfalse;
            }
            LogInfo("IsResponseAcceptable: Accepting NSEC %s (nseclist = 1)", CRDisplayString(m, newcr));
            *nseclist = mDNStrue;
            return mDNStrue;
        }
        if (rr->rrtype == kDNSType_SOA)
        {
            LogInfo("IsResponseAcceptable: Accepting SOA %s (nseclist = 1)", CRDisplayString(m, newcr));
            *nseclist = mDNStrue;
            return mDNStrue;
        }
        else if (rr->rrtype == kDNSType_NSEC3)
        {
            LogInfo("IsResponseAcceptable: Accepting NSEC3 %s (nseclist = 1)", CRDisplayString(m, newcr));
            *nseclist = mDNStrue;
            return mDNStrue;
        }
    }
    return mDNSfalse;
}

mDNSlocal void FreeNSECRecords(mDNS *const m, CacheRecord *NSECRecords)
{
    CacheRecord *rp, *next;

    for (rp = NSECRecords; rp; rp = next)
    {
        next = rp->next;
        ReleaseCacheRecord(m, rp);
    }
}

// If we received zero DNSSEC records even when the DO/EDNS0 bit was set, we need to provide this
// information to ValidatingResponse question to indicate the DNSSEC status to the application
mDNSlocal void mDNSCoreReceiveNoDNSSECAnswers(mDNS *const m, const DNSMessage *const response, const mDNSu8 *end, const mDNSAddr *dstaddr,
    mDNSIPPort dstport, const mDNSInterfaceID InterfaceID)
{
    int i;
    const mDNSu8 *ptr = response->data;

    for (i = 0; i < response->h.numQuestions && ptr && ptr < end; i++)
    {
        DNSQuestion pktq;
        DNSQuestion *qptr = mDNSNULL;
        ptr = getQuestion(response, ptr, end, InterfaceID, &pktq);
        if (ptr && (qptr = ExpectingUnicastResponseForQuestion(m, dstport, response->h.id, &pktq, !dstaddr)) &&
            qptr->ValidatingResponse)
        {
            DNSQuestion *next, *q;

            if (qptr->DuplicateOf)
                LogMsg("mDNSCoreReceiveNoDNSSECAnswers: ERROR!! qptr %##s (%s) Duplicate question matching response", qptr->qname.c, DNSTypeName(qptr->qtype));

            // Be careful to call the callback for duplicate questions first and then the original
            // question. If we called the callback on the original question, it could stop and 
            // a duplicate question would become the original question.
            mDNS_DropLockBeforeCallback();      // Allow client (and us) to legally make mDNS API calls
            for (q = qptr->next ; q && q != m->NewQuestions; q = next)
            {
                next = q->next;
                if (q->DuplicateOf == qptr)
                {
                    if (q->ValidatingResponse)
                        LogInfo("mDNSCoreReceiveNoDNSSECAnswers: qptr %##s (%s) Duplicate question found", q->qname.c, DNSTypeName(q->qtype));
                    else
                        LogMsg("mDNSCoreReceiveNoDNSSECAnswers: ERROR!! qptr %##s (%s) Duplicate question not ValidatingResponse", q->qname.c, DNSTypeName(q->qtype));
                    if (q->QuestionCallback)
                        q->QuestionCallback(m, q, mDNSNULL, QC_nodnssec);
                }
            }
            if (qptr->QuestionCallback)
                qptr->QuestionCallback(m, qptr, mDNSNULL, QC_nodnssec);
            mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
        }
    }
}

mDNSlocal void mDNSCoreReceiveNoUnicastAnswers(mDNS *const m, const DNSMessage *const response, const mDNSu8 *end, const mDNSAddr *dstaddr,
                                               mDNSIPPort dstport, const mDNSInterfaceID InterfaceID, uDNS_LLQType LLQType, mDNSu8 rcode, CacheRecord *NSECRecords)
{
    int i;
    const mDNSu8 *ptr   = response->data;
    CacheRecord *SOARecord = mDNSNULL;

    for (i = 0; i < response->h.numQuestions && ptr && ptr < end; i++)
    {
        DNSQuestion q;
        DNSQuestion *qptr = mDNSNULL;
        ptr = getQuestion(response, ptr, end, InterfaceID, &q);
        if (ptr && (qptr = ExpectingUnicastResponseForQuestion(m, dstport, response->h.id, &q, !dstaddr)))
        {
            CacheRecord *rr, *neg = mDNSNULL;
            mDNSu32 slot = HashSlot(&q.qname);
            CacheGroup *cg = CacheGroupForName(m, slot, q.qnamehash, &q.qname);
            for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
                if (SameNameRecordAnswersQuestion(&rr->resrec, qptr))
                {
                    // 1. If we got a fresh answer to this query, then don't need to generate a negative entry
                    if (RRExpireTime(rr) - m->timenow > 0) break;
                    // 2. If we already had a negative entry, keep track of it so we can resurrect it instead of creating a new one
                    if (rr->resrec.RecordType == kDNSRecordTypePacketNegative) neg = rr;
                }
            // When we're doing parallel unicast and multicast queries for dot-local names (for supporting Microsoft
            // Active Directory sites) we don't want to waste memory making negative cache entries for all the unicast answers.
            // Otherwise we just fill up our cache with negative entries for just about every single multicast name we ever look up
            // (since the Microsoft Active Directory server is going to assert that pretty much every single multicast name doesn't exist).
            // This is not only a waste of memory, but there's also the problem of those negative entries confusing us later -- e.g. we
            // suppress sending our mDNS query packet because we think we already have a valid (negative) answer to that query in our cache.
            // The one exception is that we *DO* want to make a negative cache entry for "local. SOA", for the (common) case where we're
            // *not* on a Microsoft Active Directory network, and there is no authoritative server for "local". Note that this is not
            // in conflict with the mDNS spec, because that spec says, "Multicast DNS Zones have no SOA record," so it's okay to cache
            // negative answers for "local. SOA" from a uDNS server, because the mDNS spec already says that such records do not exist :-)
            //
            // By suppressing negative responses, it might take longer to timeout a .local question as it might be expecting a
            // response e.g., we deliver a positive "A" response and suppress negative "AAAA" response and the upper layer may
            // be waiting longer to get the AAAA response before returning the "A" response to the application. To handle this
            // case without creating the negative cache entries, we generate a negative response and let the layer above us
            // do the appropriate thing. This negative response is also needed for appending new search domains.
            if (!InterfaceID && q.qtype != kDNSType_SOA && IsLocalDomain(&q.qname))
            {
                if (!rr)
                {
                    LogInfo("mDNSCoreReceiveNoUnicastAnswers: Generate negative response for %##s (%s)", q.qname.c, DNSTypeName(q.qtype));
                    m->CurrentQuestion = qptr;
                    // We are not creating a cache record in this case, we need to pass back
                    // the error we got so that the proxy code can return the right one to
                    // the application
                    if (qptr->ProxyQuestion)
                        qptr->responseFlags = response->h.flags;
                    GenerateNegativeResponse(m, QC_forceresponse);
                    m->CurrentQuestion = mDNSNULL;
                }
                else
                {
                    LogInfo("mDNSCoreReceiveNoUnicastAnswers: Skipping check and not creating a negative cache entry for %##s (%s)", q.qname.c, DNSTypeName(q.qtype));
                }
            }
            else
            {
                if (!rr)
                {
                    // We start off assuming a negative caching TTL of 60 seconds
                    // but then look to see if we can find an SOA authority record to tell us a better value we should be using
                    mDNSu32 negttl = 60;
                    int repeat = 0;
                    const domainname *name = &q.qname;
                    mDNSu32 hash = q.qnamehash;

                    // Special case for our special Microsoft Active Directory "local SOA" check.
                    // Some cheap home gateways don't include an SOA record in the authority section when
                    // they send negative responses, so we don't know how long to cache the negative result.
                    // Because we don't want to keep hitting the root name servers with our query to find
                    // if we're on a network using Microsoft Active Directory using "local" as a private
                    // internal top-level domain, we make sure to cache the negative result for at least one day.
                    if (q.qtype == kDNSType_SOA && SameDomainName(&q.qname, &localdomain)) negttl = 60 * 60 * 24;

                    // If we're going to make (or update) a negative entry, then look for the appropriate TTL from the SOA record
                    if (response->h.numAuthorities && (ptr = LocateAuthorities(response, end)) != mDNSNULL)
                    {
                        ptr = GetLargeResourceRecord(m, response, ptr, end, InterfaceID, kDNSRecordTypePacketAuth, &m->rec);
                        if (ptr && m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && m->rec.r.resrec.rrtype == kDNSType_SOA)
                        {
                            const mDNSu32 s = HashSlot(m->rec.r.resrec.name);
                            CacheGroup *cgSOA = CacheGroupForRecord(m, s, &m->rec.r.resrec);
                            const rdataSOA *const soa = (const rdataSOA *)m->rec.r.resrec.rdata->u.data;
                            mDNSu32 ttl_s = soa->min;
                            // We use the lesser of the SOA.MIN field and the SOA record's TTL, *except*
                            // for the SOA record for ".", where the record is reported as non-cacheable
                            // (TTL zero) for some reason, so in this case we just take the SOA record's TTL as-is
                            if (ttl_s > m->rec.r.resrec.rroriginalttl && m->rec.r.resrec.name->c[0])
                                ttl_s = m->rec.r.resrec.rroriginalttl;
                            if (negttl < ttl_s) negttl = ttl_s;

                            // Create the SOA record as we may have to return this to the questions
                            // that we are acting as a proxy for currently or in the future.
                            SOARecord = CreateNewCacheEntry(m, s, cgSOA, 1, mDNSfalse, mDNSNULL);

                            // Special check for SOA queries: If we queried for a.b.c.d.com, and got no answer,
                            // with an Authority Section SOA record for d.com, then this is a hint that the authority
                            // is d.com, and consequently SOA records b.c.d.com and c.d.com don't exist either.
                            // To do this we set the repeat count so the while loop below will make a series of negative cache entries for us
                            //
                            // For ProxyQuestions, we don't do this as we need to create additional SOA records to cache them
                            // along with the negative cache record. For simplicity, we don't create the additional records.
                            if (!qptr->ProxyQuestion && q.qtype == kDNSType_SOA)
                            {
                                int qcount = CountLabels(&q.qname);
                                int scount = CountLabels(m->rec.r.resrec.name);
                                if (qcount - 1 > scount)
                                    if (SameDomainName(SkipLeadingLabels(&q.qname, qcount - scount), m->rec.r.resrec.name))
                                        repeat = qcount - 1 - scount;
                            }
                        }
                        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
                    }

                    // If we already had a negative entry in the cache, then we double our existing negative TTL. This is to avoid
                    // the case where the record doesn't exist (e.g. particularly for things like our lb._dns-sd._udp.<domain> query),
                    // and the server returns no SOA record (or an SOA record with a small MIN TTL) so we assume a TTL
                    // of 60 seconds, and we end up polling the server every minute for a record that doesn't exist.
                    // With this fix in place, when this happens, we double the effective TTL each time (up to one hour),
                    // so that we back off our polling rate and don't keep hitting the server continually.
                    if (neg)
                    {
                        if (negttl < neg->resrec.rroriginalttl * 2)
                            negttl = neg->resrec.rroriginalttl * 2;
                        if (negttl > 3600)
                            negttl = 3600;
                    }

                    negttl = GetEffectiveTTL(LLQType, negttl);  // Add 25% grace period if necessary

                    // If we already had a negative cache entry just update it, else make one or more new negative cache entries.
                    if (neg)
                    {
                        LogInfo("mDNSCoreReceiveNoUnicastAnswers: Renewing negative TTL from %d to %d %s", neg->resrec.rroriginalttl, negttl, CRDisplayString(m, neg));
                        RefreshCacheRecord(m, neg, negttl);
                        // When we created the cache for the first time and answered the question, the question's
                        // interval was set to MaxQuestionInterval. If the cache is about to expire and we are resending
                        // the queries, the interval should still be at MaxQuestionInterval. If the query is being
                        // restarted (setting it to InitialQuestionInterval) for other reasons e.g., wakeup,
                        // we should reset its question interval here to MaxQuestionInterval.
                        ResetQuestionState(m, qptr);
                        if (DNSSECQuestion(qptr))
                            neg->CRDNSSECQuestion = 1;
                        // Update the NSEC records again.
                        // TBD: Need to purge and revalidate if the cached NSECS and the new set are not same.
                        if (NSECRecords)
                        {
                            if (!AddNSECSForCacheRecord(m, NSECRecords, neg, rcode))
                            {
                                // We might just have an SOA record for zones that are not signed and hence don't log
                                // this as an error
                                LogInfo("mDNSCoreReceiveNoUnicastAnswers: AddNSECSForCacheRecord failed to add NSEC for negcr %s during refresh", CRDisplayString(m, neg));
                                FreeNSECRecords(m, NSECRecords);
                                neg->CRDNSSECQuestion = 0;
                            }
                            NSECRecords = mDNSNULL;
                        }
                        if (SOARecord)
                        {
                            if (neg->soa)
                                ReleaseCacheRecord(m, neg->soa);
                            neg->soa = SOARecord;
                            SOARecord = mDNSNULL;
                        }
                    }
                    else while (1)
                        {
                            CacheRecord *negcr;
                            debugf("mDNSCoreReceiveNoUnicastAnswers making negative cache entry TTL %d for %##s (%s)", negttl, name->c, DNSTypeName(q.qtype));
                            MakeNegativeCacheRecord(m, &m->rec.r, name, hash, q.qtype, q.qclass, negttl, mDNSInterface_Any, qptr->qDNSServer);
                            m->rec.r.responseFlags = response->h.flags;
                            // We create SOA records above which might create new cache groups. Earlier
                            // in the function we looked up the cache group for the name and it could have
                            // been NULL. If we pass NULL cg to new cache entries that we create below,
                            // it will create additional cache groups for the same name. To avoid that,
                            // look up the cache group again to re-initialize cg again.
                            cg = CacheGroupForName(m, slot, hash, name);
                            if (NSECRecords && DNSSECQuestion(qptr))
                            {
                                // Create the cache entry with delay and then add the NSEC records
                                // to it and add it immediately.
                                negcr = CreateNewCacheEntry(m, slot, cg, 1, mDNStrue, mDNSNULL);
                                if (negcr)
                                {
                                    negcr->CRDNSSECQuestion = 0;
                                    if (!AddNSECSForCacheRecord(m, NSECRecords, negcr, rcode))
                                    {
                                        LogInfo("mDNSCoreReceiveNoUnicastAnswers: AddNSECSForCacheRecord failed to add NSEC for negcr %s",
                                            CRDisplayString(m, negcr));
                                        FreeNSECRecords(m, NSECRecords);
                                    }
                                    else
                                    {
                                        negcr->CRDNSSECQuestion = 1;
                                        LogInfo("mDNSCoreReceiveNoUnicastAnswers: AddNSECSForCacheRecord added neg NSEC for %s", CRDisplayString(m, negcr));
                                    }
                                    NSECRecords = mDNSNULL;
                                    negcr->DelayDelivery = 0;
                                    CacheRecordDeferredAdd(m, negcr);
                                }
                                m->rec.r.resrec.RecordType = 0; // Clear RecordType to show we're not still using it
                                break;
                            }
                            else
                            {
                                // Need to add with a delay so that we can tag the SOA record
                                negcr = CreateNewCacheEntry(m, slot, cg, 1, mDNStrue, mDNSNULL);
                                if (negcr)
                                {
                                    negcr->CRDNSSECQuestion = 0;
                                    if (DNSSECQuestion(qptr))
                                        negcr->CRDNSSECQuestion = 1;
                                    negcr->DelayDelivery = 0;

                                    if (SOARecord)
                                    {
                                        if (negcr->soa)
                                            ReleaseCacheRecord(m, negcr->soa);
                                        negcr->soa = SOARecord;
                                        SOARecord = mDNSNULL;
                                    }
                                    CacheRecordDeferredAdd(m, negcr);
                                }
                            }
                            m->rec.r.responseFlags = zeroID;
                            m->rec.r.resrec.RecordType = 0; // Clear RecordType to show we're not still using it
                            if (!repeat) break;
                            repeat--;
                            name = (const domainname *)(name->c + 1 + name->c[0]);
                            hash = DomainNameHashValue(name);
                            slot = HashSlot(name);
                            cg   = CacheGroupForName(m, slot, hash, name);
                        }
                }
            }
        }
    }
    if (NSECRecords) { LogInfo("mDNSCoreReceiveNoUnicastAnswers: NSECRecords not used"); FreeNSECRecords(m, NSECRecords); }
    if (SOARecord)   { LogInfo("mDNSCoreReceiveNoUnicastAnswers: SOARecord not used"); ReleaseCacheRecord(m, SOARecord); }
}

mDNSlocal void mDNSCorePrintStoredProxyRecords(mDNS *const m)
{
    AuthRecord *rrPtr = mDNSNULL;
    LogSPS("Stored Proxy records :");
    for (rrPtr = m->SPSRRSet; rrPtr; rrPtr = rrPtr->next)
    {
        LogSPS("%s", ARDisplayString(m, rrPtr));
    }
}

mDNSlocal mDNSBool mDNSCoreRegisteredProxyRecord(mDNS *const m, AuthRecord *rr)
{
    AuthRecord *rrPtr = mDNSNULL;

    for (rrPtr = m->SPSRRSet; rrPtr; rrPtr = rrPtr->next)
    {
        if (IdenticalResourceRecord(&rrPtr->resrec, &rr->resrec))
        {
            LogSPS("mDNSCoreRegisteredProxyRecord: Ignoring packet registered with sleep proxy : %s ", ARDisplayString(m, rr));
            return mDNStrue;
        }
    }
    mDNSCorePrintStoredProxyRecords(m);
    return mDNSfalse;
}

mDNSlocal CacheRecord* mDNSCoreReceiveCacheCheck(mDNS *const m, const DNSMessage *const response, uDNS_LLQType LLQType,
    const mDNSu32 slot, CacheGroup *cg, DNSQuestion *unicastQuestion, CacheRecord ***cfp, CacheRecord **NSECCachePtr,
    mDNSInterfaceID InterfaceID)
{
    CacheRecord *rr;
    CacheRecord **cflocal = *cfp;

    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
    {
        mDNSBool match;
        // Resource record received via unicast, the resGroupID should match ?
        if (!InterfaceID)
        {
            mDNSu16 id1 = (rr->resrec.rDNSServer ? rr->resrec.rDNSServer->resGroupID : 0);
            mDNSu16 id2 = (m->rec.r.resrec.rDNSServer ? m->rec.r.resrec.rDNSServer->resGroupID : 0);
            match = (id1 == id2);
        }
        else
            match = (rr->resrec.InterfaceID == InterfaceID);
        // If we found this exact resource record, refresh its TTL
        if (match && IdenticalSameNameRecord(&m->rec.r.resrec, &rr->resrec))
        {
            if (m->rec.r.resrec.rdlength > InlineCacheRDSize)
                verbosedebugf("mDNSCoreReceiveCacheCheck: Found record size %5d interface %p already in cache: %s",
                              m->rec.r.resrec.rdlength, InterfaceID, CRDisplayString(m, &m->rec.r));

            if (m->rec.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask)
            {
                // If this packet record has the kDNSClass_UniqueRRSet flag set, then add it to our cache flushing list
                if (rr->NextInCFList == mDNSNULL && *cfp != &rr->NextInCFList && LLQType != uDNS_LLQ_Events)
                {
                    *cflocal = rr;
                    cflocal = &rr->NextInCFList;
                    *cflocal = (CacheRecord*)1;
                    *cfp = &rr->NextInCFList;
                }

                // If this packet record is marked unique, and our previous cached copy was not, then fix it
                if (!(rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask))
                {
                    DNSQuestion *q;
                    for (q = m->Questions; q; q=q->next)
                    {
                        if (ResourceRecordAnswersQuestion(&rr->resrec, q))
                            q->UniqueAnswers++;
                    }
                    rr->resrec.RecordType = m->rec.r.resrec.RecordType;
                }
            }

            if (!SameRDataBody(&m->rec.r.resrec, &rr->resrec.rdata->u, SameDomainNameCS))
            {
                // If the rdata of the packet record differs in name capitalization from the record in our cache
                // then mDNSPlatformMemSame will detect this. In this case, throw the old record away, so that clients get
                // a 'remove' event for the record with the old capitalization, and then an 'add' event for the new one.
                // <rdar://problem/4015377> mDNS -F returns the same domain multiple times with different casing
                rr->resrec.rroriginalttl = 0;
                rr->TimeRcvd = m->timenow;
                rr->UnansweredQueries = MaxUnansweredQueries;
                SetNextCacheCheckTimeForRecord(m, rr);
                LogInfo("mDNSCoreReceiveCacheCheck: Discarding due to domainname case change old: %s", CRDisplayString(m, rr));
                LogInfo("mDNSCoreReceiveCacheCheck: Discarding due to domainname case change new: %s", CRDisplayString(m, &m->rec.r));
                LogInfo("mDNSCoreReceiveCacheCheck: Discarding due to domainname case change in %d slot %3d in %d %d",
                        NextCacheCheckEvent(rr) - m->timenow, slot, m->rrcache_nextcheck[slot] - m->timenow, m->NextCacheCheck - m->timenow);
                // DO NOT break out here -- we want to continue as if we never found it
            }
            else if (!IdenticalAnonInfo(m->rec.r.resrec.AnonInfo, rr->resrec.AnonInfo))
            {
                // If the NSEC3 record changed, a few possibilities
                //
                // 1) the peer reinitialized e.g., after network change and still part of the
                //    same set.
                // 2) the peer went to a different set but we did not see the goodbyes. If we just
                //    update the nsec3 record, it would be incorrect. Flush the cache so that we
                //    can deliver a RMV followed by ADD.
                // 3) if the peer is ourselves and we see the goodbye when moving to a different set
                //    and so we flush the cache and create a new cache record with the new set information.
                //    Now we move back to the original set. In this case, we can't just update the
                //    NSEC3 record alone. We need to flush so that we can deliver an RMV followed by ADD
                //    when we create the new cache entry.
                //
                // Note: For case (1), we could avoid flushing the cache but we can't tell the difference
                // from the other cases.
                rr->resrec.rroriginalttl = 0;
                rr->TimeRcvd = m->timenow;
                rr->UnansweredQueries = MaxUnansweredQueries;
                SetNextCacheCheckTimeForRecord(m, rr);
                LogInfo("mDNSCoreReceiveCacheCheck: AnonInfo changed for %s", CRDisplayString(m, rr));
                // DO NOT break out here -- we want to continue as if we never found it. When we return
                // from this function, we will create a new cache entry with the new NSEC3 record
            }
            else if (m->rec.r.resrec.rroriginalttl > 0)
            {
                DNSQuestion *q;

                m->mDNSStats.CacheRefreshed++;

                if (rr->resrec.rroriginalttl == 0) debugf("uDNS rescuing %s", CRDisplayString(m, rr));
                RefreshCacheRecord(m, rr, m->rec.r.resrec.rroriginalttl);
                rr->responseFlags = response->h.flags;

                // If we may have NSEC records returned with the answer (which we don't know yet as it
                // has not been processed), we need to cache them along with the first cache
                // record in the list that answers the question so that it can be used for validation
                // later. The "type" check below is to make sure that we cache on the cache record
                // that would answer the question. It is possible that we might cache additional things
                // e.g., MX question might cache A records also, and we want to cache the NSEC on
                // the record that answers the question.
                if (response->h.numAnswers && unicastQuestion && unicastQuestion->qtype == rr->resrec.rrtype
                    && !(*NSECCachePtr))
                {
                    LogInfo("mDNSCoreReceiveCacheCheck: rescuing RR %s", CRDisplayString(m, rr));
                    *NSECCachePtr = rr;
                }
                // We have to reset the question interval to MaxQuestionInterval so that we don't keep
                // polling the network once we get a valid response back. For the first time when a new
                // cache entry is created, AnswerCurrentQuestionWithResourceRecord does that.
                // Subsequently, if we reissue questions from within the mDNSResponder e.g., DNS server
                // configuration changed, without flushing the cache, we reset the question interval here.
                // Currently, we do this for for both multicast and unicast questions as long as the record
                // type is unique. For unicast, resource record is always unique and for multicast it is
                // true for records like A etc. but not for PTR.
                if (rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask)
                {
                    for (q = m->Questions; q; q=q->next)
                    {
                        if (!q->DuplicateOf && !q->LongLived &&
                            ActiveQuestion(q) && ResourceRecordAnswersQuestion(&rr->resrec, q))
                        {
                            ResetQuestionState(m, q);
                            debugf("mDNSCoreReceiveCacheCheck: Set MaxQuestionInterval for %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
                            break;      // Why break here? Aren't there other questions we might want to look at?-- SC July 2010
                        }
                    }
                }
                break;
            }
            else
            {
            
                // If the packet TTL is zero, that means we're deleting this record.
                // To give other hosts on the network a chance to protest, we push the deletion
                // out one second into the future. Also, we set UnansweredQueries to MaxUnansweredQueries.
                // Otherwise, we'll do final queries for this record at 80% and 90% of its apparent
                // lifetime (800ms and 900ms from now) which is a pointless waste of network bandwidth.
                // If record's current expiry time is more than a second from now, we set it to expire in one second.
                // If the record is already going to expire in less than one second anyway, we leave it alone --
                // we don't want to let the goodbye packet *extend* the record's lifetime in our cache.
                debugf("DE for %s", CRDisplayString(m, rr));
                if (RRExpireTime(rr) - m->timenow > mDNSPlatformOneSecond)
                {
                    rr->resrec.rroriginalttl = 1;
                    rr->TimeRcvd = m->timenow;
                    rr->UnansweredQueries = MaxUnansweredQueries;
                    SetNextCacheCheckTimeForRecord(m, rr);
                }
                break;
            }
        }
    }
    return rr;
}

mDNSlocal void mDNSParseNSEC3Records(mDNS *const m, const DNSMessage *const response, const mDNSu8 *end,
    const mDNSInterfaceID InterfaceID, CacheRecord **NSEC3Records)
{
    const mDNSu8 *ptr   = response->data;
    CacheRecord *rr;
    int i;

    if (!response->h.numAuthorities)
        return;
    ptr = LocateAuthorities(response, end);
    if (!ptr)
    {
        LogInfo("mDNSParseNSEC3Records: ERROR can't locate authorities");
        return;
    }
    for (i = 0; i < response->h.numAuthorities && ptr && ptr < end; i++)
    {
        mDNSu32 slot;
        CacheGroup *cg;

        ptr = GetLargeResourceRecord(m, response, ptr, end, InterfaceID, kDNSRecordTypePacketAuth, &m->rec);
        if (!ptr || m->rec.r.resrec.RecordType == kDNSRecordTypePacketNegative || m->rec.r.resrec.rrtype != kDNSType_NSEC3)
        {
            debugf("mDNSParseNSEC3Records: ptr %p, Record %s, ignoring", ptr, CRDisplayString(m, &m->rec.r));
            m->rec.r.resrec.RecordType = 0;
            continue;
        }
        slot = HashSlot(m->rec.r.resrec.name);
        cg = CacheGroupForRecord(m, slot, &m->rec.r.resrec);
        // Create the cache entry but don't add it to the cache it. We need
        // to cache this along with the main cache record.
        rr = CreateNewCacheEntry(m, slot, cg, 0, mDNSfalse, mDNSNULL);
        if (rr)
        {
            debugf("mDNSParseNSEC3Records: %s", CRDisplayString(m, rr));
            *NSEC3Records = rr;
            NSEC3Records = &rr->next;
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }
}

mDNSlocal void mDNSCoreResetRecord(mDNS *const m)
{
    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    if (m->rec.r.resrec.AnonInfo)
    {
        FreeAnonInfo(m->rec.r.resrec.AnonInfo); 
        m->rec.r.resrec.AnonInfo = mDNSNULL;
    }
}

#define DEVICE_INFO_RECORD_LABELS 4

// Determine if the record is an instance of _device-info._tcp.local.
mDNSlocal mDNSBool IsDeviceInfoRecord(const domainname *d)
{
    const domainname *afterInstance;

    if (CountLabels(d) != DEVICE_INFO_RECORD_LABELS)
        return mDNSfalse;

    // skip the instance name
    afterInstance = SkipLeadingLabels(d, 1);
    if (SameDomainName(afterInstance, &LocalDeviceInfoName))
        return mDNStrue;

    return mDNSfalse;
}

// Note: mDNSCoreReceiveResponse calls mDNS_Deregister_internal which can call a user callback, which may change
// the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
// InterfaceID non-NULL tells us the interface this multicast response was received on
// InterfaceID NULL tells us this was a unicast response
// dstaddr NULL tells us we received this over an outgoing TCP connection we made
mDNSlocal void mDNSCoreReceiveResponse(mDNS *const m,
                                       const DNSMessage *const response, const mDNSu8 *end,
                                       const mDNSAddr *srcaddr, const mDNSIPPort srcport, const mDNSAddr *dstaddr, mDNSIPPort dstport,
                                       const mDNSInterfaceID InterfaceID)
{
    int i;
    mDNSBool myself;
    mDNSBool ResponseMCast    = dstaddr && mDNSAddrIsDNSMulticast(dstaddr);
    mDNSBool ResponseSrcLocal = !srcaddr || mDNS_AddressIsLocalSubnet(m, InterfaceID, srcaddr, &myself);
    DNSQuestion *llqMatch = mDNSNULL;
    DNSQuestion *unicastQuestion = mDNSNULL;
    uDNS_LLQType LLQType      = uDNS_recvLLQResponse(m, response, end, srcaddr, srcport, &llqMatch);

    // "(CacheRecord*)1" is a special (non-zero) end-of-list marker
    // We use this non-zero marker so that records in our CacheFlushRecords list will always have NextInCFList
    // set non-zero, and that tells GetCacheEntity() that they're not, at this moment, eligible for recycling.
    CacheRecord *CacheFlushRecords = (CacheRecord*)1;
    CacheRecord **cfp = &CacheFlushRecords;
    CacheRecord *NSECRecords = mDNSNULL;
    CacheRecord *NSECCachePtr = mDNSNULL;
    CacheRecord **nsecp = &NSECRecords;
    CacheRecord *McastNSEC3Records = mDNSNULL;
    mDNSBool nseclist;
    mDNSu8 rcode = '\0';
    mDNSBool rrsigsCreated = mDNSfalse;
    mDNSBool DNSSECQuestion = mDNSfalse;
    mDNSBool recordAccepted = mDNSfalse;
    NetworkInterfaceInfo *llintf = FirstIPv4LLInterfaceForID(m, InterfaceID);

    // All records in a DNS response packet are treated as equally valid statements of truth. If we want
    // to guard against spoof responses, then the only credible protection against that is cryptographic
    // security, e.g. DNSSEC., not worring about which section in the spoof packet contained the record
    int firstauthority  =                   response->h.numAnswers;
    int firstadditional = firstauthority  + response->h.numAuthorities;
    int totalrecords    = firstadditional + response->h.numAdditionals;
    const mDNSu8 *ptr   = response->data;
    DNSServer *uDNSServer = mDNSNULL;

    debugf("Received Response from %#-15a addressed to %#-15a on %p with "
           "%2d Question%s %2d Answer%s %2d Authorit%s %2d Additional%s %d bytes LLQType %d",
           srcaddr, dstaddr, InterfaceID,
           response->h.numQuestions,   response->h.numQuestions   == 1 ? ", "   : "s,",
           response->h.numAnswers,     response->h.numAnswers     == 1 ? ", "   : "s,",
           response->h.numAuthorities, response->h.numAuthorities == 1 ? "y,  " : "ies,",
           response->h.numAdditionals, response->h.numAdditionals == 1 ? " "    : "s", end - response->data, LLQType);

    // According to RFC 2181 <http://www.ietf.org/rfc/rfc2181.txt>
    //    When a DNS client receives a reply with TC
    //    set, it should ignore that response, and query again, using a
    //    mechanism, such as a TCP connection, that will permit larger replies.
    // It feels wrong to be throwing away data after the network went to all the trouble of delivering it to us, but
    // delivering some records of the RRSet first and then the remainder a couple of milliseconds later was causing
    // failures in our Microsoft Active Directory client, which expects to get the entire set of answers at once.
    // <rdar://problem/6690034> Can't bind to Active Directory
    // In addition, if the client immediately canceled its query after getting the initial partial response, then we'll
    // abort our TCP connection, and not complete the operation, and end up with an incomplete RRSet in our cache.
    // Next time there's a query for this RRSet we'll see answers in our cache, and assume we have the whole RRSet already,
    // and not even do the TCP query.
    // Accordingly, if we get a uDNS reply with kDNSFlag0_TC set, we bail out and wait for the TCP response containing the entire RRSet.
    if (!InterfaceID && (response->h.flags.b[0] & kDNSFlag0_TC)) return;

    if (LLQType == uDNS_LLQ_Ignore) return;

    // 1. We ignore questions (if any) in mDNS response packets
    // 2. If this is an LLQ response, we handle it much the same
    // 3. If we get a uDNS UDP response with the TC (truncated) bit set, then we can't treat this
    //    answer as being the authoritative complete RRSet, and respond by deleting all other
    //    matching cache records that don't appear in this packet.
    // Otherwise, this is a authoritative uDNS answer, so arrange for any stale records to be purged
    if (ResponseMCast || LLQType == uDNS_LLQ_Events || (response->h.flags.b[0] & kDNSFlag0_TC))
        ptr = LocateAnswers(response, end);
    // Otherwise, for one-shot queries, any answers in our cache that are not also contained
    // in this response packet are immediately deemed to be invalid.
    else
    {
        mDNSBool failure, returnEarly;
        rcode = (mDNSu8)(response->h.flags.b[1] & kDNSFlag1_RC_Mask);
        failure = !(rcode == kDNSFlag1_RC_NoErr || rcode == kDNSFlag1_RC_NXDomain || rcode == kDNSFlag1_RC_NotAuth);
        returnEarly = mDNSfalse;
        // We could possibly combine this with the similar loop at the end of this function --
        // instead of tagging cache records here and then rescuing them if we find them in the answer section,
        // we could instead use the "m->PktNum" mechanism to tag each cache record with the packet number in
        // which it was received (or refreshed), and then at the end if we find any cache records which
        // answer questions in this packet's question section, but which aren't tagged with this packet's
        // packet number, then we deduce they are old and delete them
        for (i = 0; i < response->h.numQuestions && ptr && ptr < end; i++)
        {
            DNSQuestion q, *qptr = mDNSNULL;
            ptr = getQuestion(response, ptr, end, InterfaceID, &q);
            if (ptr && (qptr = ExpectingUnicastResponseForQuestion(m, dstport, response->h.id, &q, !dstaddr)))
            {
                if (!failure)
                {
                    CacheRecord *rr;
                    // Remember the unicast question that we found, which we use to make caching
                    // decisions later on in this function
                    const mDNSu32 slot = HashSlot(&q.qname);
                    CacheGroup *cg = CacheGroupForName(m, slot, q.qnamehash, &q.qname);
                    if (!mDNSOpaque16IsZero(response->h.id))
                    {
                        unicastQuestion = qptr;
                        if (qptr->qDNSServer && DNSSECQuestion(qptr))
                        {
                            LogInfo("mDNSCoreReceiveResponse: Setting aware for %##s (%s) on %#a", qptr->qname.c,
                                DNSTypeName(qptr->qtype), &qptr->qDNSServer->addr);
                            qptr->qDNSServer->DNSSECAware = mDNStrue;
                            qptr->qDNSServer->req_DO = mDNStrue;
                        }
                        if (qptr->ValidatingResponse)
                            DNSSECQuestion = mDNStrue;
                    }
                    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
                        if (SameNameRecordAnswersQuestion(&rr->resrec, qptr))
                        {
                            debugf("uDNS marking %p %##s (%s) %p %s", q.InterfaceID, q.qname.c, DNSTypeName(q.qtype),
                                   rr->resrec.InterfaceID, CRDisplayString(m, rr));
                            // Don't want to disturb rroriginalttl here, because code below might need it for the exponential backoff doubling algorithm
                            rr->TimeRcvd          = m->timenow - TicksTTL(rr) - 1;
                            rr->UnansweredQueries = MaxUnansweredQueries;
                            rr->CRDNSSECQuestion = 0;
                            if (unicastQuestion && DNSSECQuestion(unicastQuestion))
                            {
                                LogInfo("mDNSCoreReceiveResponse: CRDNSSECQuestion set for record %s, question %##s (%s)", CRDisplayString(m, rr),
                                    unicastQuestion->qname.c, DNSTypeName(unicastQuestion->qtype));
                                rr->CRDNSSECQuestion = 1;
                            }
                        }
                }
                else
                {
                    if (qptr)
                    {
                        // If we recv any error from the DNSServer for a DNSSEC Query and if we know that the server
                        // is not DNSSEC aware, stop doing DNSSEC for that DNSServer. Note that by setting the
                        // req_DO to false here, the next retransmission for this question will turn off validation
                        // and hence retransmit without the EDNS0/DOK option.
                        if (DNSSECOptionalQuestion(qptr) && qptr->qDNSServer && !qptr->qDNSServer->DNSSECAware)
                        {
                            LogInfo("mDNSCoreReceiveResponse: Server %p responded with code %d to DNSSEC Query %##s (%s), clear DO flag", 
                                    qptr->qDNSServer, rcode, q.qname.c, DNSTypeName(q.qtype));
                            qptr->qDNSServer->req_DO = mDNSfalse;    
                        }
                        // For Unicast DNS Queries, penalize the DNSServer
                        else
                        {
                            LogInfo("mDNSCoreReceiveResponse: Server %p responded with code %d to query %##s (%s)", 
                                    qptr->qDNSServer, rcode, q.qname.c, DNSTypeName(q.qtype));
                            PenalizeDNSServer(m, qptr, response->h.flags);
                        }
                    }
                    returnEarly = mDNStrue;
                }
            }
        }
        if (returnEarly)
        {
            LogInfo("Ignoring %2d Answer%s %2d Authorit%s %2d Additional%s",
                    response->h.numAnswers,     response->h.numAnswers     == 1 ? ", " : "s,",
                    response->h.numAuthorities, response->h.numAuthorities == 1 ? "y,  " : "ies,",
                    response->h.numAdditionals, response->h.numAdditionals == 1 ? "" : "s");
            // not goto exit because we won't have any CacheFlushRecords and we do not want to
            // generate negative cache entries (we want to query the next server)
            return;
        }
        if (unicastQuestion && DNSSECQuestion(unicastQuestion))
        {
            BumpDNSSECStats(m, kStatsActionSet, kStatsTypeMsgSize, (end - response->data));
        }
    }

    // Parse the NSEC3 records from the Authority section before we process
    // the Answer section so that we can cache them along with the proper
    // cache records we create.
    if (mDNSOpaque16IsZero(response->h.id))
        mDNSParseNSEC3Records(m, response, end, InterfaceID, &McastNSEC3Records);

    for (i = 0; i < totalrecords && ptr && ptr < end; i++)
    {
        // All responses sent via LL multicast are acceptable for caching
        // All responses received over our outbound TCP connections are acceptable for caching
        mDNSBool AcceptableResponse = ResponseMCast || !dstaddr || LLQType;
        // (Note that just because we are willing to cache something, that doesn't necessarily make it a trustworthy answer
        // to any specific question -- any code reading records from the cache needs to make that determination for itself.)

        const mDNSu8 RecordType =
            (i < firstauthority ) ? (mDNSu8)kDNSRecordTypePacketAns  :
            (i < firstadditional) ? (mDNSu8)kDNSRecordTypePacketAuth : (mDNSu8)kDNSRecordTypePacketAdd;
        ptr = GetLargeResourceRecord(m, response, ptr, end, InterfaceID, RecordType, &m->rec);
        if (!ptr) goto exit;        // Break out of the loop and clean up our CacheFlushRecords list before exiting

        if (m->rec.r.resrec.RecordType == kDNSRecordTypePacketNegative)
        {
            mDNSCoreResetRecord(m);
            continue;
        }

        // We have already parsed the NSEC3 records and cached them approrpriately for
        // multicast responses.
        if (mDNSOpaque16IsZero(response->h.id) && m->rec.r.resrec.rrtype == kDNSType_NSEC3)
        {
            mDNSCoreResetRecord(m);
            continue;
        }
        // Don't want to cache OPT or TSIG pseudo-RRs
        if (m->rec.r.resrec.rrtype == kDNSType_TSIG)
        {
            mDNSCoreResetRecord(m);
            continue;
        }
        if (m->rec.r.resrec.rrtype == kDNSType_OPT)
        {
            const rdataOPT *opt;
            const rdataOPT *const e = (const rdataOPT *)&m->rec.r.resrec.rdata->u.data[m->rec.r.resrec.rdlength];
            // Find owner sub-option(s). We verify that the MAC is non-zero, otherwise we could inadvertently
            // delete all our own AuthRecords (which are identified by having zero MAC tags on them).
            for (opt = &m->rec.r.resrec.rdata->u.opt[0]; opt < e; opt++)
                if (opt->opt == kDNSOpt_Owner && opt->u.owner.vers == 0 && opt->u.owner.HMAC.l[0])
                {
                    ClearProxyRecords(m, &opt->u.owner, m->DuplicateRecords);
                    ClearProxyRecords(m, &opt->u.owner, m->ResourceRecords);
                }
            mDNSCoreResetRecord(m);
            continue;
        }
        // if a CNAME record points to itself, then don't add it to the cache
        if ((m->rec.r.resrec.rrtype == kDNSType_CNAME) && SameDomainName(m->rec.r.resrec.name, &m->rec.r.resrec.rdata->u.name))
        {
            LogInfo("mDNSCoreReceiveResponse: CNAME loop domain name %##s", m->rec.r.resrec.name->c);
            mDNSCoreResetRecord(m);
            continue;
        }

        // When we receive uDNS LLQ responses, we assume a long cache lifetime --
        // In the case of active LLQs, we'll get remove events when the records actually do go away
        // In the case of polling LLQs, we assume the record remains valid until the next poll
        if (!mDNSOpaque16IsZero(response->h.id))
            m->rec.r.resrec.rroriginalttl = GetEffectiveTTL(LLQType, m->rec.r.resrec.rroriginalttl);

        // If response was not sent via LL multicast,
        // then see if it answers a recent query of ours, which would also make it acceptable for caching.
        if (!ResponseMCast)
        {
            if (LLQType)
            {
                // For Long Lived queries that are both sent over UDP and Private TCP, LLQType is set.
                // Even though it is AcceptableResponse, we need a matching DNSServer pointer for the
                // queries to get ADD/RMV events. To lookup the question, we can't use
                // ExpectingUnicastResponseForRecord as the port numbers don't match. uDNS_recvLLQRespose
                // has already matched the question using the 64 bit Id in the packet and we use that here.

                if (llqMatch != mDNSNULL) m->rec.r.resrec.rDNSServer = uDNSServer = llqMatch->qDNSServer;

                // If this is a DNSSEC question that is also LongLived, don't accept records from the
                // Additional/Authority section blindly. We need to go through IsAcceptableResponse below
                // so that NSEC/NSEC3 record are cached in the nseclist if we accept them. This can happen
                // for both negative responses and wildcard expanded positive responses as both of come
                // back with NSEC/NSEC3s.
                if (unicastQuestion && DNSSECQuestion(unicastQuestion))
                    AcceptableResponse = mDNSfalse;
            }
            else if (!AcceptableResponse || !dstaddr)
            {
                // For responses that come over TCP (Responses that can't fit within UDP) or TLS (Private queries
                // that are not long lived e.g., AAAA lookup in a Private domain), it is indicated by !dstaddr.
                // Even though it is AcceptableResponse, we still need a DNSServer pointer for the resource records that
                // we create.

                if (!mDNSOpaque16IsZero(response->h.id))
                {
                    DNSQuestion *q = ExpectingUnicastResponseForRecord(m, srcaddr, ResponseSrcLocal, dstport, response->h.id, &m->rec.r, !dstaddr);

                    // Initialize the DNS server on the resource record which will now filter what questions we answer with
                    // this record.
                    //
                    // We could potentially lookup the DNS server based on the source address, but that may not work always
                    // and that's why ExpectingUnicastResponseForRecord does not try to verify whether the response came
                    // from the DNS server that queried. We follow the same logic here. If we can find a matching quetion based
                    // on the "id" and "source port", then this response answers the question and assume the response
                    // came from the same DNS server that we sent the query to.

                    if (q != mDNSNULL)
                    {
                        AcceptableResponse = mDNStrue;
                        if (!InterfaceID)
                        {
                            debugf("mDNSCoreReceiveResponse: InterfaceID %p %##s (%s)", q->InterfaceID, q->qname.c, DNSTypeName(q->qtype));
                            m->rec.r.resrec.rDNSServer = uDNSServer = q->qDNSServer;
                        }
                        else
                            LogInfo("mDNSCoreReceiveResponse: InterfaceID %p %##s (%s)", q->InterfaceID, q->qname.c, DNSTypeName(q->qtype));
                    }
                    else
                    {
                        // If we can't find a matching question, we need to see whether we have seen records earlier that matched
                        // the question. The code below does that. So, make this record unacceptable for now
                        if (!InterfaceID)
                        {
                            debugf("mDNSCoreReceiveResponse: Can't find question for record name %##s", m->rec.r.resrec.name->c);
                            AcceptableResponse = mDNSfalse;
                        }
                    }
                }
                else if (ExpectingMulticastResponseForRecord(m, &m->rec.r, srcaddr, recordAccepted, &McastNSEC3Records))
                {
                    recordAccepted = mDNStrue;
                    AcceptableResponse = mDNStrue;
                    LogInfo("mDNSCoreReceiveResponse: Accepting record in response to QU question %s, InterfaceID %p", CRDisplayString(m, &m->rec.r),
                        InterfaceID);
                }
                else if (IsDeviceInfoRecord(m->rec.r.resrec.name))
                {
                    recordAccepted = mDNStrue;
                    AcceptableResponse = mDNStrue;
                    LogInfo("mDNSCoreReceiveResponse: Accepting _device-info record %s, InterfaceID %p",
                         CRDisplayString(m, &m->rec.r), InterfaceID);
                }
            }
        }
        else if (llintf && llintf->IgnoreIPv4LL && m->rec.r.resrec.rrtype == kDNSType_A)
        {
            CacheRecord *const rr = &m->rec.r;
            RDataBody2 *const rdb = (RDataBody2 *)rr->smallrdatastorage.data;

            // If we are supposed to ignore link-local addresses on this interface, drop
            // all "A" records that have link-local address in them.
            if (mDNSv4AddressIsLinkLocal(&rdb->ipv4))
            {
                LogInfo("mDNSResponder: Dropping LinkLocal packet %s", CRDisplayString(m, &m->rec.r));
                mDNSCoreResetRecord(m);
                continue;
            }
        }

        // 1. Check that this packet resource record does not conflict with any of ours
        if (mDNSOpaque16IsZero(response->h.id) && m->rec.r.resrec.rrtype != kDNSType_NSEC)
        {
            if (m->CurrentRecord)
                LogMsg("mDNSCoreReceiveResponse ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
            m->CurrentRecord = m->ResourceRecords;
            while (m->CurrentRecord)
            {
                AuthRecord *rr = m->CurrentRecord;
                m->CurrentRecord = rr->next;
                // We accept all multicast responses, and unicast responses resulting from queries we issued
                // For other unicast responses, this code accepts them only for responses with an
                // (apparently) local source address that pertain to a record of our own that's in probing state
                if (!AcceptableResponse && !(ResponseSrcLocal && rr->resrec.RecordType == kDNSRecordTypeUnique)) continue;

                if (PacketRRMatchesSignature(&m->rec.r, rr))        // If interface, name, type (if shared record) and class match...
                {
                    // ... check to see if type and rdata are identical
                    if (IdenticalSameNameRecord(&m->rec.r.resrec, &rr->resrec))
                    {
                        // If the RR in the packet is identical to ours, just check they're not trying to lower the TTL on us
                        if (m->rec.r.resrec.rroriginalttl >= rr->resrec.rroriginalttl/2 || m->SleepState)
                        {
                            // If we were planning to send on this -- and only this -- interface, then we don't need to any more
                            if      (rr->ImmedAnswer == InterfaceID) { rr->ImmedAnswer = mDNSNULL; rr->ImmedUnicast = mDNSfalse; }
                        }
                        else
                        {
                            if      (rr->ImmedAnswer == mDNSNULL)    { rr->ImmedAnswer = InterfaceID;       m->NextScheduledResponse = m->timenow; }
                            else if (rr->ImmedAnswer != InterfaceID) { rr->ImmedAnswer = mDNSInterfaceMark; m->NextScheduledResponse = m->timenow; }
                        }
                    }
                    // else, the packet RR has different type or different rdata -- check to see if this is a conflict
                    else if (m->rec.r.resrec.rroriginalttl > 0 && PacketRRConflict(m, rr, &m->rec.r))
                    {
                        LogInfo("mDNSCoreReceiveResponse: Pkt Record: %08lX %s", m->rec.r.resrec.rdatahash, CRDisplayString(m, &m->rec.r));
                        LogInfo("mDNSCoreReceiveResponse: Our Record: %08lX %s", rr->resrec.rdatahash, ARDisplayString(m, rr));

                        // If this record is marked DependentOn another record for conflict detection purposes,
                        // then *that* record has to be bumped back to probing state to resolve the conflict
                        if (rr->DependentOn)
                        {
                            while (rr->DependentOn) rr = rr->DependentOn;
                            LogInfo("mDNSCoreReceiveResponse: Dep Record: %08lX %s", rr->resrec.rdatahash, ARDisplayString(m, rr));
                        }

                        // If we've just whacked this record's ProbeCount, don't need to do it again
                        if (rr->ProbeCount > DefaultProbeCountForTypeUnique)
                            LogInfo("mDNSCoreReceiveResponse: Already reset to Probing: %s", ARDisplayString(m, rr));
                        else if (rr->ProbeCount == DefaultProbeCountForTypeUnique)
                            LogMsg("mDNSCoreReceiveResponse: Ignoring response received before we even began probing: %s", ARDisplayString(m, rr));
                        else
                        {
                            LogMsg("mDNSCoreReceiveResponse: Received from %#a:%d %s", srcaddr, mDNSVal16(srcport), CRDisplayString(m, &m->rec.r));
                            // If we'd previously verified this record, put it back to probing state and try again
                            if (rr->resrec.RecordType == kDNSRecordTypeVerified)
                            {
                                LogMsg("mDNSCoreReceiveResponse: Resetting to Probing: %s", ARDisplayString(m, rr));
                                rr->resrec.RecordType     = kDNSRecordTypeUnique;
                                // We set ProbeCount to one more than the usual value so we know we've already touched this record.
                                // This is because our single probe for "example-name.local" could yield a response with (say) two A records and
                                // three AAAA records in it, and we don't want to call RecordProbeFailure() five times and count that as five conflicts.
                                // This special value is recognised and reset to DefaultProbeCountForTypeUnique in SendQueries().
                                rr->ProbeCount     = DefaultProbeCountForTypeUnique + 1;
                                rr->AnnounceCount  = InitialAnnounceCount;
                                InitializeLastAPTime(m, rr);
                                RecordProbeFailure(m, rr);  // Repeated late conflicts also cause us to back off to the slower probing rate
                            }
                            // If we're probing for this record, we just failed
                            else if (rr->resrec.RecordType == kDNSRecordTypeUnique)
                            {
                                // Before we call deregister, check if this is a packet we registered with the sleep proxy.
                                if (!mDNSCoreRegisteredProxyRecord(m, rr))
                                {
                                    LogMsg("mDNSCoreReceiveResponse: ProbeCount %d; will deregister %s", rr->ProbeCount, ARDisplayString(m, rr));
                                    
                                    m->mDNSStats.NameConflicts++;
                                    mDNS_Deregister_internal(m, rr, mDNS_Dereg_conflict);
                                }
                            }
                            // We assumed this record must be unique, but we were wrong. (e.g. There are two mDNSResponders on the
                            // same machine giving different answers for the reverse mapping record, or there are two machines on the
                            // network using the same IP address.) This is simply a misconfiguration, and there's nothing we can do
                            // to fix it -- e.g. it's not our job to be trying to change the machine's IP address. We just discard our
                            // record to avoid continued conflicts (as we do for a conflict on our Unique records) and get on with life.
                            else if (rr->resrec.RecordType == kDNSRecordTypeKnownUnique)
                            {
                                LogMsg("mDNSCoreReceiveResponse: Unexpected conflict discarding %s", ARDisplayString(m, rr));
                                m->mDNSStats.KnownUniqueNameConflicts++;
                                mDNS_Deregister_internal(m, rr, mDNS_Dereg_conflict);
                            }
                            else
                                LogMsg("mDNSCoreReceiveResponse: Unexpected record type %X %s", rr->resrec.RecordType, ARDisplayString(m, rr));
                        }
                    }
                    // Else, matching signature, different type or rdata, but not a considered a conflict.
                    // If the packet record has the cache-flush bit set, then we check to see if we
                    // have any record(s) of the same type that we should re-assert to rescue them
                    // (see note about "multi-homing and bridged networks" at the end of this function).
                    else if (m->rec.r.resrec.rrtype == rr->resrec.rrtype)
                        if ((m->rec.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask) && m->timenow - rr->LastMCTime > mDNSPlatformOneSecond/2)
                        { rr->ImmedAnswer = mDNSInterfaceMark; m->NextScheduledResponse = m->timenow; }
                }
            }
        }

        nseclist = mDNSfalse;
        if (!AcceptableResponse)
        {
            AcceptableResponse = IsResponseAcceptable(m, CacheFlushRecords, unicastQuestion, &nseclist);
            if (AcceptableResponse) m->rec.r.resrec.rDNSServer = uDNSServer;
        }

        // 2. See if we want to add this packet resource record to our cache
        // We only try to cache answers if we have a cache to put them in
        // Also, we ignore any apparent attempts at cache poisoning unicast to us that do not answer any outstanding active query
        if (!AcceptableResponse) LogInfo("mDNSCoreReceiveResponse ignoring %s", CRDisplayString(m, &m->rec.r));
        if (m->rrcache_size && AcceptableResponse)
        {
            const mDNSu32 slot = HashSlot(m->rec.r.resrec.name);
            CacheGroup *cg = CacheGroupForRecord(m, slot, &m->rec.r.resrec);
            CacheRecord *rr = mDNSNULL;

            if (McastNSEC3Records)
                InitializeAnonInfoForCR(m, &McastNSEC3Records, &m->rec.r);

            // 2a. Check if this packet resource record is already in our cache.
            //
            // If this record should go in the nseclist, don't look in the cache for updating it.
            // They are supposed to be cached under the "nsec" field of the cache record for
            // validation. Just create the cache record. 
            if (!nseclist)
            {
                rr = mDNSCoreReceiveCacheCheck(m, response, LLQType, slot, cg, unicastQuestion, &cfp, &NSECCachePtr, InterfaceID);
            }

            // If mDNSOppCaching is set (which affects only multicast), enable opportunistic caching in which case we cache
            // everything that was received over multicast. Otherwise, we are selective about the caching.
            //
            // Cache everything that is from ourselves (that's how we answer any questions looking for them). Otherwise call
            // ExpectingMulticastResponseForRecord which decides whether to cache this record or not.
            //
            if (!m->mDNSOppCaching && !rr && !myself && mDNSOpaque16IsZero(response->h.id))
            {
                if (!ExpectingMulticastResponseForRecord(m, &m->rec.r, srcaddr, recordAccepted, &McastNSEC3Records))
                {
                    //LogMsg("mDNSCoreReceiveResponse: discarding %s", CRDisplayString(m, &m->rec.r));
                    mDNSCoreResetRecord(m);
                    continue;
                }
                else
                {
                    recordAccepted = mDNStrue;
                }
            }


            // If packet resource record not in our cache, add it now
            // (unless it is just a deletion of a record we never had, in which case we don't care)
            if (!rr && m->rec.r.resrec.rroriginalttl > 0)
            {
                const mDNSBool AddToCFList = (m->rec.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask) && (LLQType != uDNS_LLQ_Events);
                mDNSs32 delay;

                if (AddToCFList)
                    delay = NonZeroTime(m->timenow + mDNSPlatformOneSecond);
                else
                    delay = CheckForSoonToExpireRecords(m, m->rec.r.resrec.name, m->rec.r.resrec.namehash, slot, mDNSNULL);

                // If unique, assume we may have to delay delivery of this 'add' event.
                // Below, where we walk the CacheFlushRecords list, we either call CacheRecordDeferredAdd()
                // to immediately to generate answer callbacks, or we call ScheduleNextCacheCheckTime()
                // to schedule an mDNS_Execute task at the appropriate time.
                rr = CreateNewCacheEntry(m, slot, cg, delay, !nseclist, srcaddr);
                if (rr)
                {
                    rr->responseFlags = response->h.flags;
                    // If we are not creating signatures, then we need to inform DNSSEC so that
                    // it does not wait forever. Don't do this if we got NSEC records
                    // as it indicates that this name does not exist.
                    if (rr->resrec.rrtype == kDNSType_RRSIG && !nseclist)
                    {
                        rrsigsCreated = mDNStrue;
                    }
                    // Remember whether we created a cache record in response to a DNSSEC question.
                    // This helps DNSSEC code not to reissue the question to fetch the DNSSEC records.
                    rr->CRDNSSECQuestion = 0;
                    if (unicastQuestion && DNSSECQuestion(unicastQuestion))
                    {
                        LogInfo("mDNSCoreReceiveResponse: CRDNSSECQuestion set for new record %s, question %##s (%s)", CRDisplayString(m, rr),
                            unicastQuestion->qname.c, DNSTypeName(unicastQuestion->qtype));
                        rr->CRDNSSECQuestion = 1;
                    }
                    // NSEC/NSEC3 records and its signatures are cached with the negative cache entry
                    // which we should be creating below. It is also needed in the wildcard
                    // expanded answer case and in that case it is cached along with the answer.
                    if (nseclist)
                    {
                        rr->TimeRcvd = m->timenow;
                        *nsecp = rr;
                        nsecp = &rr->next;
                    }
                    else if (AddToCFList)
                    {
                        *cfp = rr;
                        cfp = &rr->NextInCFList;
                        *cfp = (CacheRecord*)1;
                    }
                    else if (rr->DelayDelivery)
                    {
                        ScheduleNextCacheCheckTime(m, slot, rr->DelayDelivery);
                    }
                }
            }
            else
            {
                if (rr && rr->resrec.AnonInfo && m->rec.r.resrec.AnonInfo)
                {
                    CopyAnonInfoForCR(m, rr, &m->rec.r);
                }
            }
        }
        mDNSCoreResetRecord(m);
    }

exit:
    mDNSCoreResetRecord(m);

    // If we've just received one or more records with their cache flush bits set,
    // then scan that cache slot to see if there are any old stale records we need to flush
    while (CacheFlushRecords != (CacheRecord*)1)
    {
        CacheRecord *r1 = CacheFlushRecords, *r2;
        const mDNSu32 slot = HashSlot(r1->resrec.name);
        const CacheGroup *cg = CacheGroupForRecord(m, slot, &r1->resrec);
        CacheFlushRecords = CacheFlushRecords->NextInCFList;
        r1->NextInCFList = mDNSNULL;

        // Look for records in the cache with the same signature as this new one with the cache flush
        // bit set, and either (a) if they're fresh, just make sure the whole RRSet has the same TTL
        // (as required by DNS semantics) or (b) if they're old, mark them for deletion in one second.
        // We make these TTL adjustments *only* for records that still have *more* than one second
        // remaining to live. Otherwise, a record that we tagged for deletion half a second ago
        // (and now has half a second remaining) could inadvertently get its life extended, by either
        // (a) if we got an explicit goodbye packet half a second ago, the record would be considered
        // "fresh" and would be incorrectly resurrected back to the same TTL as the rest of the RRSet,
        // or (b) otherwise, the record would not be fully resurrected, but would be reset to expire
        // in one second, thereby inadvertently delaying its actual expiration, instead of hastening it.
        // If this were to happen repeatedly, the record's expiration could be deferred indefinitely.
        // To avoid this, we need to ensure that the cache flushing operation will only act to
        // *decrease* a record's remaining lifetime, never *increase* it.
        for (r2 = cg ? cg->members : mDNSNULL; r2; r2=r2->next)
        {
            mDNSu16 id1;
            mDNSu16 id2;
            if (!r1->resrec.InterfaceID)
            {
                id1 = (r1->resrec.rDNSServer ? r1->resrec.rDNSServer->resGroupID : 0);
                id2 = (r2->resrec.rDNSServer ? r2->resrec.rDNSServer->resGroupID : 0);
            }
            else
            {
                id1 = id2 = 0;
            }
            // When we receive new RRSIGs e.g., for DNSKEY record, we should not flush the old
            // RRSIGS e.g., for TXT record. To do so, we need to look at the typeCovered field of
            // the new RRSIG that we received. Process only if the typeCovered matches.
            if ((r1->resrec.rrtype == r2->resrec.rrtype) && (r1->resrec.rrtype == kDNSType_RRSIG))
            {
                rdataRRSig *rrsig1 = (rdataRRSig *)(((RDataBody2 *)(r1->resrec.rdata->u.data))->data);
                rdataRRSig *rrsig2 = (rdataRRSig *)(((RDataBody2 *)(r2->resrec.rdata->u.data))->data);
                if (swap16(rrsig1->typeCovered) != swap16(rrsig2->typeCovered))
                {
                    debugf("mDNSCoreReceiveResponse: Received RRSIG typeCovered %s, found %s, not processing",
                        DNSTypeName(swap16(rrsig1->typeCovered)), DNSTypeName(swap16(rrsig2->typeCovered)));
                    continue;
                }
            }
             
            // For Unicast (null InterfaceID) the resolver IDs should also match
            if ((r1->resrec.InterfaceID == r2->resrec.InterfaceID) &&
                (r1->resrec.InterfaceID || (id1 == id2)) &&
                r1->resrec.rrtype      == r2->resrec.rrtype &&
                r1->resrec.rrclass     == r2->resrec.rrclass)
            {
                // If record is recent, just ensure the whole RRSet has the same TTL (as required by DNS semantics)
                // else, if record is old, mark it to be flushed
                if (m->timenow - r2->TimeRcvd < mDNSPlatformOneSecond && RRExpireTime(r2) - m->timenow > mDNSPlatformOneSecond)
                {
                    // If we find mismatched TTLs in an RRSet, correct them.
                    // We only do this for records with a TTL of 2 or higher. It's possible to have a
                    // goodbye announcement with the cache flush bit set (or a case-change on record rdata,
                    // which we treat as a goodbye followed by an addition) and in that case it would be
                    // inappropriate to synchronize all the other records to a TTL of 0 (or 1).
                    // We suppress the message for the specific case of correcting from 240 to 60 for type TXT,
                    // because certain early Bonjour devices are known to have this specific mismatch, and
                    // there's no point filling syslog with messages about something we already know about.
                    // We also don't log this for uDNS responses, since a caching name server is obliged
                    // to give us an aged TTL to correct for how long it has held the record,
                    // so our received TTLs are expected to vary in that case
                    if (r2->resrec.rroriginalttl != r1->resrec.rroriginalttl && r1->resrec.rroriginalttl > 1)
                    {
                        if (!(r2->resrec.rroriginalttl == 240 && r1->resrec.rroriginalttl == 60 && r2->resrec.rrtype == kDNSType_TXT) &&
                            mDNSOpaque16IsZero(response->h.id))
                            LogInfo("Correcting TTL from %4d to %4d for %s",
                                    r2->resrec.rroriginalttl, r1->resrec.rroriginalttl, CRDisplayString(m, r2));
                        r2->resrec.rroriginalttl = r1->resrec.rroriginalttl;
                    }
                    r2->TimeRcvd = m->timenow;
                }
                else                // else, if record is old, mark it to be flushed
                {
                    verbosedebugf("Cache flush new %p age %d expire in %d %s", r1, m->timenow - r1->TimeRcvd, RRExpireTime(r1) - m->timenow, CRDisplayString(m, r1));
                    verbosedebugf("Cache flush old %p age %d expire in %d %s", r2, m->timenow - r2->TimeRcvd, RRExpireTime(r2) - m->timenow, CRDisplayString(m, r2));
                    // We set stale records to expire in one second.
                    // This gives the owner a chance to rescue it if necessary.
                    // This is important in the case of multi-homing and bridged networks:
                    //   Suppose host X is on Ethernet. X then connects to an AirPort base station, which happens to be
                    //   bridged onto the same Ethernet. When X announces its AirPort IP address with the cache-flush bit
                    //   set, the AirPort packet will be bridged onto the Ethernet, and all other hosts on the Ethernet
                    //   will promptly delete their cached copies of the (still valid) Ethernet IP address record.
                    //   By delaying the deletion by one second, we give X a change to notice that this bridging has
                    //   happened, and re-announce its Ethernet IP address to rescue it from deletion from all our caches.

                    // We set UnansweredQueries to MaxUnansweredQueries to avoid expensive and unnecessary
                    // final expiration queries for this record.

                    // If a record is deleted twice, first with an explicit DE record, then a second time by virtue of the cache
                    // flush bit on the new record replacing it, then we allow the record to be deleted immediately, without the usual
                    // one-second grace period. This improves responsiveness for mDNS_Update(), as used for things like iChat status updates.
                    // <rdar://problem/5636422> Updating TXT records is too slow
                    // We check for "rroriginalttl == 1" because we want to include records tagged by the "packet TTL is zero" check above,
                    // which sets rroriginalttl to 1, but not records tagged by the rdata case-change check, which sets rroriginalttl to 0.
                    if (r2->TimeRcvd == m->timenow && r2->resrec.rroriginalttl == 1 && r2->UnansweredQueries == MaxUnansweredQueries)
                    {
                        LogInfo("Cache flush for DE record %s", CRDisplayString(m, r2));
                        r2->resrec.rroriginalttl = 0;
                    }
                    else if (RRExpireTime(r2) - m->timenow > mDNSPlatformOneSecond)
                    {
                        // We only set a record to expire in one second if it currently has *more* than a second to live
                        // If it's already due to expire in a second or less, we just leave it alone
                        r2->resrec.rroriginalttl = 1;
                        r2->UnansweredQueries = MaxUnansweredQueries;
                        r2->TimeRcvd = m->timenow - 1;
                        // We use (m->timenow - 1) instead of m->timenow, because we use that to identify records
                        // that we marked for deletion via an explicit DE record
                    }
                }
                SetNextCacheCheckTimeForRecord(m, r2);
            }
        }

        if (r1->DelayDelivery)  // If we were planning to delay delivery of this record, see if we still need to
        {
            // If we had a unicast question for this response with at least one positive answer and we
            // have NSECRecords, it is most likely a wildcard expanded answer. Cache the NSEC and its
            // signatures along with the cache record which will be used for validation later. If
            // we rescued a few records earlier in this function, then NSECCachePtr would be set. In that
            // use that instead.
            if (response->h.numAnswers && unicastQuestion && NSECRecords)
            {
                if (!NSECCachePtr)
                {
                    LogInfo("mDNSCoreReceiveResponse: Updating NSECCachePtr to %s", CRDisplayString(m, r1));
                    NSECCachePtr = r1;
                }
                // Note: We need to do this before we call CacheRecordDeferredAdd as this
                // might start the verification process which needs these NSEC records
                if (!AddNSECSForCacheRecord(m, NSECRecords, NSECCachePtr, rcode))
                { 
                    LogInfo("mDNSCoreReceiveResponse: AddNSECSForCacheRecord failed to add NSEC for %s", CRDisplayString(m, NSECCachePtr));
                    FreeNSECRecords(m, NSECRecords);
                }
                NSECRecords = mDNSNULL;
                NSECCachePtr = mDNSNULL;
            }
            r1->DelayDelivery = CheckForSoonToExpireRecords(m, r1->resrec.name, r1->resrec.namehash, slot, mDNSNULL);
            // If no longer delaying, deliver answer now, else schedule delivery for the appropriate time
            if (!r1->DelayDelivery) CacheRecordDeferredAdd(m, r1);
            else ScheduleNextCacheCheckTime(m, slot, r1->DelayDelivery);
        }
    }

    // If we have not consumed the NSEC records yet e.g., just refreshing the cache,
    // update them now for future validations.
    if (NSECRecords && NSECCachePtr)
    {
        LogInfo("mDNSCoreReceieveResponse: Updating NSEC records in %s", CRDisplayString(m, NSECCachePtr));
        if (!AddNSECSForCacheRecord(m, NSECRecords, NSECCachePtr, rcode))
        { 
            LogInfo("mDNSCoreReceiveResponse: AddNSECSForCacheRecord failed to add NSEC for %s", CRDisplayString(m, NSECCachePtr));
            FreeNSECRecords(m, NSECRecords);
        }
        NSECRecords = mDNSNULL;
        NSECCachePtr = mDNSNULL;
    }

    // If there is at least one answer and we did not create RRSIGs and there was a
    // ValidatingResponse question waiting for this response, give a hint that no RRSIGs
    // were created. We don't need to give a hint:
    //
    // - if we have no answers, the mDNSCoreReceiveNoUnicastAnswers below should
    //   generate a negative response
    //
    // - if we have NSECRecords, it means we might have a potential proof for
    //   non-existence of name that we are looking for
    //
    if (response->h.numAnswers && !rrsigsCreated && DNSSECQuestion && !NSECRecords)
        mDNSCoreReceiveNoDNSSECAnswers(m, response, end, dstaddr, dstport, InterfaceID);

    // See if we need to generate negative cache entries for unanswered unicast questions
    mDNSCoreReceiveNoUnicastAnswers(m, response, end, dstaddr, dstport, InterfaceID, LLQType, rcode, NSECRecords);

    if (McastNSEC3Records)
    {
        debugf("mDNSCoreReceiveResponse: McastNSEC3Records not used");
        FreeNSECRecords(m, McastNSEC3Records);
    }
}

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
// ScheduleWakeup causes all proxy records with WakeUp.HMAC matching mDNSEthAddr 'e' to be deregistered, causing
// multiple wakeup magic packets to be sent if appropriate, and all records to be ultimately freed after a few seconds.
// ScheduleWakeup is called on mDNS record conflicts, ARP conflicts, NDP conflicts, or reception of trigger traffic
// that warrants waking the sleeping host.
// ScheduleWakeup must be called with the lock held (ScheduleWakeupForList uses mDNS_Deregister_internal)

mDNSlocal void ScheduleWakeupForList(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSEthAddr *e, AuthRecord *const thelist)
{
    // We need to use the m->CurrentRecord mechanism here when dealing with DuplicateRecords list as
    // mDNS_Deregister_internal deregisters duplicate records immediately as they are not used
    // to send wakeups or goodbyes. See the comment in that function for more details. To keep it
    // simple, we use the same mechanism for both lists.
    if (!e->l[0])
    {
        LogMsg("ScheduleWakeupForList ERROR: Target HMAC is zero");
        return;
    }
    m->CurrentRecord = thelist;
    while (m->CurrentRecord)
    {
        AuthRecord *const rr = m->CurrentRecord;
        if (rr->resrec.InterfaceID == InterfaceID && rr->resrec.RecordType != kDNSRecordTypeDeregistering && mDNSSameEthAddress(&rr->WakeUp.HMAC, e))
        {
            LogInfo("ScheduleWakeupForList: Scheduling wakeup packets for %s", ARDisplayString(m, rr));
            mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
        }
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}

mDNSlocal void ScheduleWakeup(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSEthAddr *e)
{
    if (!e->l[0]) { LogMsg("ScheduleWakeup ERROR: Target HMAC is zero"); return; }
    ScheduleWakeupForList(m, InterfaceID, e, m->DuplicateRecords);
    ScheduleWakeupForList(m, InterfaceID, e, m->ResourceRecords);
}
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

mDNSlocal void SPSRecordCallback(mDNS *const m, AuthRecord *const ar, mStatus result)
{
    if (result && result != mStatus_MemFree)
        LogInfo("SPS Callback %d %s", result, ARDisplayString(m, ar));
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (result == mStatus_NameConflict)
    {
        mDNS_Lock(m);
        LogMsg("%-7s Conflicting mDNS -- waking %.6a %s", InterfaceNameForID(m, ar->resrec.InterfaceID), &ar->WakeUp.HMAC, ARDisplayString(m, ar));
        if (ar->WakeUp.HMAC.l[0])
        {
            SendWakeup(m, ar->resrec.InterfaceID, &ar->WakeUp.IMAC, &ar->WakeUp.password);  // Send one wakeup magic packet
            ScheduleWakeup(m, ar->resrec.InterfaceID, &ar->WakeUp.HMAC);                    // Schedule all other records with the same owner to be woken
        }
        mDNS_Unlock(m);
    }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (result == mStatus_NameConflict || result == mStatus_MemFree)
    {
        m->ProxyRecords--;
        mDNSPlatformMemFree(ar);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
        mDNS_UpdateAllowSleep(m);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    }
}

mDNSlocal mDNSu8 *GetValueForMACAddr(mDNSu8 *ptr, mDNSu8 *limit, mDNSEthAddr *eth)
{
    int     i;
    mDNSs8  hval   = 0;
    int     colons = 0;
    mDNSu8  val    = 0;

    for (i = 0; ptr < limit && *ptr != ' ' && i < 17; i++, ptr++)
    {
        hval = HexVal(*ptr);
        if (hval != -1)
        {
            val <<= 4;
            val |= hval;
        }
        else if (*ptr == ':')
        {
            if (colons >=5 || val > 255)
            {
                LogMsg("GetValueForMACAddr: Address malformed colons %d val %d", colons, val);
                return mDNSNULL;
            }
            eth->b[colons] = val;
            colons++;
            val = 0;
        }
    }
    if (colons != 5)
    {
        LogMsg("GetValueForMACAddr: Address malformed colons %d", colons);
        return mDNSNULL;
    }
    eth->b[colons] = val;
    return ptr;
}

mDNSlocal mDNSu8 *GetValueForIPv6Addr(mDNSu8 *ptr, mDNSu8 *limit, mDNSv6Addr *v6)
{
    int hval;
    int value;
    int numBytes;
    int digitsProcessed;
    int zeroFillStart;
    int numColons;
    mDNSu8 v6addr[16];

    // RFC 3513: Section 2.2 specifies IPv6 presentation format. The following parsing
    // handles both (1) and (2) and does not handle embedded IPv4 addresses.
    //
    // First forms a address in "v6addr", then expands to fill the zeroes in and returns
    // the result in "v6"

    numColons = numBytes = value = digitsProcessed = zeroFillStart = 0;
    while (ptr < limit && *ptr != ' ')
    {
        hval = HexVal(*ptr);
        if (hval != -1)
        {
            value <<= 4;
            value |= hval;
            digitsProcessed = 1;
        }
        else if (*ptr == ':')
        {
            if (!digitsProcessed)
            {
                // If we have already seen a "::", we should not see one more. Handle the special
                // case of "::"
                if (numColons)
                {
                    // if we never filled any bytes and the next character is space (we have reached the end)
                    // we are done
                    if (!numBytes && (ptr + 1) < limit && *(ptr + 1) == ' ')
                    {
                        mDNSPlatformMemZero(v6->b, 16);
                        return ptr + 1;
                    }
                    LogMsg("GetValueForIPv6Addr: zeroFillStart non-zero %d", zeroFillStart);
                    return mDNSNULL;
                }

                // We processed "::". We need to fill zeroes later. For now, mark the
                // point where we will start filling zeroes from.
                zeroFillStart = numBytes;
                numColons++;
            }
            else if ((ptr + 1) < limit && *(ptr + 1) == ' ')
            {
                // We have a trailing ":" i.e., no more characters after ":"
                LogMsg("GetValueForIPv6Addr: Trailing colon");
                return mDNSNULL;
            }
            else
            {
                // For a fully expanded IPv6 address, we fill the 14th and 15th byte outside of this while
                // loop below as there is no ":" at the end. Hence, the last two bytes that can possibly
                // filled here is 12 and 13.
                if (numBytes > 13) { LogMsg("GetValueForIPv6Addr:1: numBytes is %d", numBytes); return mDNSNULL; }

                v6addr[numBytes++] = (mDNSu8) ((value >> 8) & 0xFF);
                v6addr[numBytes++] = (mDNSu8) (value & 0xFF);
                digitsProcessed = value = 0;

                // Make sure that we did not fill the 13th and 14th byte above
                if (numBytes > 14) { LogMsg("GetValueForIPv6Addr:2: numBytes is %d", numBytes); return mDNSNULL; }
            }
        }
        ptr++;
    }

    // We should be processing the last set of bytes following the last ":" here
    if (!digitsProcessed)
    {
        LogMsg("GetValueForIPv6Addr: no trailing bytes after colon, numBytes is %d", numBytes);
        return mDNSNULL;
    }

    if (numBytes > 14) { LogMsg("GetValueForIPv6Addr:3: numBytes is %d", numBytes); return mDNSNULL; }
    v6addr[numBytes++] = (mDNSu8) ((value >> 8) & 0xFF);
    v6addr[numBytes++] = (mDNSu8) (value & 0xFF);

    if (zeroFillStart)
    {
        int i, j, n;
        for (i = 0; i < zeroFillStart; i++)
            v6->b[i] = v6addr[i];
        for (j = i, n = 0; n < 16 - numBytes; j++, n++)
            v6->b[j] = 0;
        for (; j < 16; i++, j++)
            v6->b[j] = v6addr[i];
    }
    else if (numBytes == 16)
        mDNSPlatformMemCopy(v6->b, v6addr, 16);
    else
    {
        LogMsg("GetValueForIPv6addr: Not enough bytes for IPv6 address, numBytes is %d", numBytes);
        return mDNSNULL;
    }
    return ptr;
}

mDNSlocal mDNSu8 *GetValueForIPv4Addr(mDNSu8 *ptr, mDNSu8 *limit, mDNSv4Addr *v4)
{
    mDNSu32 val;
    int dots = 0;
    val = 0;

    for ( ; ptr < limit && *ptr != ' '; ptr++)
    {
        if (*ptr >= '0' &&  *ptr <= '9')
            val = val * 10 + *ptr - '0';
        else if (*ptr == '.')
        {
            if (val > 255 || dots >= 3)
            {
                LogMsg("GetValueForIPv4Addr: something wrong ptr(%p) %c, limit %p, dots %d", ptr, *ptr, limit, dots);
                return mDNSNULL;
            }
            v4->b[dots++] = val;
            val = 0;
        }
        else
        {
            // We have a zero at the end and if we reached that, then we are done.
            if (*ptr == 0 && ptr == limit - 1 && dots == 3)
            {
                v4->b[dots] = val;
                return ptr + 1;
            }
            else { LogMsg("GetValueForIPv4Addr: something wrong ptr(%p) %c, limit %p, dots %d", ptr, *ptr, limit, dots); return mDNSNULL; }
        }
    }
    if (dots != 3) { LogMsg("GetValueForIPv4Addr: Address malformed dots %d", dots); return mDNSNULL; }
    v4->b[dots] = val;
    return ptr;
}

mDNSlocal mDNSu8 *GetValueForKeepalive(mDNSu8 *ptr, mDNSu8 *limit, mDNSu32 *value)
{
    mDNSu32 val;

    val = 0;
    for ( ; ptr < limit && *ptr != ' '; ptr++)
    {
        if (*ptr < '0' || *ptr > '9')
        {
            // We have a zero at the end and if we reached that, then we are done.
            if (*ptr == 0 && ptr == limit - 1)
            {
                *value = val;
                return ptr + 1;
            }
            else { LogMsg("GetValueForKeepalive: *ptr %d, ptr %p, limit %p, ptr +1 %d", *ptr, ptr, limit, *(ptr + 1)); return mDNSNULL; }
        }
        val = val * 10 + *ptr - '0';
    }
    *value = val;
    return ptr;
}

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
mDNSlocal void mDNS_ExtractKeepaliveInfo(AuthRecord *ar, mDNSu32 *timeout, mDNSAddr *laddr, mDNSAddr *raddr, mDNSEthAddr *eth, mDNSu32 *seq,
                                         mDNSu32 *ack, mDNSIPPort *lport, mDNSIPPort *rport, mDNSu16 *win)
{
    if (ar->resrec.rrtype != kDNSType_NULL)
        return;

    if (mDNS_KeepaliveRecord(&ar->resrec))
    {
        int len = ar->resrec.rdlength;
        mDNSu8 *ptr = &ar->resrec.rdata->u.txt.c[1];
        mDNSu8 *limit = ptr + len - 1; // Exclude the first byte that is the length
        mDNSu32 value = 0;

        while (ptr < limit)
        {
            mDNSu8 param = *ptr;
            ptr += 2;   // Skip the letter and the "="
            if (param == 'h')
            {
                laddr->type = mDNSAddrType_IPv4;
                ptr = GetValueForIPv4Addr(ptr, limit, &laddr->ip.v4);
            }
            else if (param == 'd')
            {
                raddr->type = mDNSAddrType_IPv4;
                ptr = GetValueForIPv4Addr(ptr, limit, &raddr->ip.v4);
            }
            if (param == 'H')
            {
                laddr->type = mDNSAddrType_IPv6;
                ptr = GetValueForIPv6Addr(ptr, limit, &laddr->ip.v6);
            }
            else if (param == 'D')
            {
                raddr->type = mDNSAddrType_IPv6;
                ptr = GetValueForIPv6Addr(ptr, limit, &raddr->ip.v6);
            }
            else if (param == 'm')
            {
                ptr = GetValueForMACAddr(ptr, limit, eth);
            }
            else
            {
                ptr = GetValueForKeepalive(ptr, limit, &value);
            }
            if (!ptr) { LogMsg("mDNS_ExtractKeepaliveInfo: Cannot parse\n"); return; }

            // Extract everything in network order so that it is easy for sending a keepalive and also
            // for matching incoming TCP packets
            switch (param)
            {
            case 't':
                *timeout = value;
                //if (*timeout < 120) *timeout = 120;
                break;
            case 'h':
            case 'H':
            case 'd':
            case 'D':
            case 'm':
            case 'i':
            case 'c':
                break;
            case 'l':
                lport->NotAnInteger = swap16((mDNSu16)value);
                break;
            case 'r':
                rport->NotAnInteger = swap16((mDNSu16)value);
                break;
            case 's':
                *seq = swap32(value);
                break;
            case 'a':
                *ack = swap32(value);
                break;
            case 'w':
                *win = swap16((mDNSu16)value);
                break;
            default:
                LogMsg("mDNS_ExtractKeepaliveInfo: unknown value %c\n", param);
                ptr = limit;
                break;
            }
            ptr++; // skip the space
        }
    }
}

// Matches the proxied auth records to the incoming TCP packet and returns the match and its sequence and ack in "rseq" and "rack" so that
// the clients need not retrieve this information from the auth record again.
mDNSlocal AuthRecord* mDNS_MatchKeepaliveInfo(mDNS *const m, const mDNSAddr* pladdr, const mDNSAddr* praddr, const mDNSIPPort plport,
                                              const mDNSIPPort prport, mDNSu32 *rseq, mDNSu32 *rack)
{
    AuthRecord *ar;
    mDNSAddr laddr, raddr;
    mDNSEthAddr eth;
    mDNSIPPort lport, rport;
    mDNSu32 timeout, seq, ack;
    mDNSu16 win;

    for (ar = m->ResourceRecords; ar; ar=ar->next)
    {
        timeout = seq = ack = 0;
        win = 0;
        laddr = raddr = zeroAddr;
        lport = rport = zeroIPPort;

        if (!ar->WakeUp.HMAC.l[0]) continue;

        mDNS_ExtractKeepaliveInfo(ar, &timeout, &laddr, &raddr, &eth, &seq, &ack, &lport, &rport, &win);

        // Did we parse correctly ?
        if (!timeout || mDNSAddressIsZero(&laddr) || mDNSAddressIsZero(&raddr) || !seq || !ack || mDNSIPPortIsZero(lport) || mDNSIPPortIsZero(rport) || !win)
        {
            debugf("mDNS_MatchKeepaliveInfo: not a valid record %s for keepalive", ARDisplayString(m, ar));
            continue;
        }

        debugf("mDNS_MatchKeepaliveInfo: laddr %#a pladdr %#a, raddr %#a praddr %#a, lport %d plport %d, rport %d prport %d",
               &laddr, pladdr, &raddr, praddr, mDNSVal16(lport), mDNSVal16(plport), mDNSVal16(rport), mDNSVal16(prport));

        // Does it match the incoming TCP packet ?
        if (mDNSSameAddress(&laddr, pladdr) && mDNSSameAddress(&raddr, praddr) && mDNSSameIPPort(lport, plport) && mDNSSameIPPort(rport, prport))
        {
            // returning in network order
            *rseq = seq;
            *rack = ack;
            return ar;
        }
    }
    return mDNSNULL;
}

mDNSlocal void mDNS_SendKeepalives(mDNS *const m)
{
    AuthRecord *ar;

    for (ar = m->ResourceRecords; ar; ar=ar->next)
    {
        mDNSu32 timeout, seq, ack;
        mDNSu16 win;
        mDNSAddr laddr, raddr;
        mDNSEthAddr eth;
        mDNSIPPort lport, rport;

        timeout = seq = ack = 0;
        win = 0;

        laddr = raddr = zeroAddr;
        lport = rport = zeroIPPort;

        if (!ar->WakeUp.HMAC.l[0]) continue;

        mDNS_ExtractKeepaliveInfo(ar, &timeout, &laddr, &raddr, &eth, &seq, &ack, &lport, &rport, &win);

        if (!timeout || mDNSAddressIsZero(&laddr) || mDNSAddressIsZero(&raddr) || !seq || !ack || mDNSIPPortIsZero(lport) || mDNSIPPortIsZero(rport) || !win)
        {
            debugf("mDNS_SendKeepalives: not a valid record %s for keepalive", ARDisplayString(m, ar));
            continue;
        }
        LogMsg("mDNS_SendKeepalives: laddr %#a raddr %#a lport %d rport %d", &laddr, &raddr, mDNSVal16(lport), mDNSVal16(rport));

        // When we receive a proxy update, we set KATimeExpire to zero so that we always send a keepalive
        // immediately (to detect any potential problems). After that we always set it to a non-zero value.
        if (!ar->KATimeExpire || (m->timenow - ar->KATimeExpire >= 0))
        {
            mDNSPlatformSendKeepalive(&laddr, &raddr, &lport, &rport, seq, ack, win);
            ar->KATimeExpire = NonZeroTime(m->timenow + timeout * mDNSPlatformOneSecond);
        }
        if (m->NextScheduledKA - ar->KATimeExpire > 0)
            m->NextScheduledKA = ar->KATimeExpire;
    }
}

mDNSlocal void mDNS_SendKeepaliveACK(mDNS *const m, AuthRecord *ar)
{
    if (ar != mDNSNULL)
    {
        LogInfo("mDNS_SendKeepalivesACK: AuthRecord is NULL");
        return;
    }
    mDNSu32     timeout, seq, ack;
    mDNSu16     win;
    mDNSAddr    laddr, raddr;
    mDNSEthAddr eth;
    mDNSIPPort  lport, rport;

    timeout = seq = ack = 0;
    win = 0;

    laddr = raddr = zeroAddr;
    lport = rport = zeroIPPort;

    mDNS_ExtractKeepaliveInfo(ar, &timeout, &laddr, &raddr, &eth, &seq, &ack, &lport, &rport, &win);

    if (!timeout || mDNSAddressIsZero(&laddr) || mDNSAddressIsZero(&raddr) || !seq || !ack || mDNSIPPortIsZero(lport) || mDNSIPPortIsZero(rport) || !win)
    {
        LogInfo("mDNS_SendKeepaliveACK: not a valid record %s for keepalive", ARDisplayString(m, ar));
        return;
    }
    LogMsg("mDNS_SendKeepaliveACK: laddr %#a raddr %#a lport %d rport %d", &laddr, &raddr, mDNSVal16(lport), mDNSVal16(rport));
    mDNSPlatformSendKeepalive(&laddr, &raddr, &lport, &rport, seq, ack, win);
}
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

mDNSlocal void mDNSCoreReceiveUpdate(mDNS *const m,
                                     const DNSMessage *const msg, const mDNSu8 *end,
                                     const mDNSAddr *srcaddr, const mDNSIPPort srcport, const mDNSAddr *dstaddr, mDNSIPPort dstport,
                                     const mDNSInterfaceID InterfaceID)
{
    int i;
    AuthRecord opt;
    mDNSu8 *p = m->omsg.data;
    OwnerOptData owner = zeroOwner;     // Need to zero this, so we'll know if this Update packet was missing its Owner option
    mDNSu32 updatelease = 0;
    const mDNSu8 *ptr;

    mdnslogInfo("Received Update from %#-15a:%-5d to %#-15a:%-5d on %p with "
           "%2d Question%s %2d Answer%s %2d Authorit%s %2d Additional%s %d bytes",
           srcaddr, mDNSVal16(srcport), dstaddr, mDNSVal16(dstport), InterfaceID,
           msg->h.numQuestions,   msg->h.numQuestions   == 1 ? ", "   : "s,",
           msg->h.numAnswers,     msg->h.numAnswers     == 1 ? ", "   : "s,",
           msg->h.numAuthorities, msg->h.numAuthorities == 1 ? "y,  " : "ies,",
           msg->h.numAdditionals, msg->h.numAdditionals == 1 ? " "    : "s", end - msg->data);

    if (!InterfaceID || !m->SPSSocket || !mDNSSameIPPort(dstport, m->SPSSocket->port)) return;

    if (mDNS_PacketLoggingEnabled)
        DumpPacket(m, mStatus_NoError, mDNSfalse, "UDP", srcaddr, srcport, dstaddr, dstport, msg, end);

    ptr = LocateOptRR(msg, end, DNSOpt_LeaseData_Space + DNSOpt_OwnerData_ID_Space);
    if (ptr)
    {
        ptr = GetLargeResourceRecord(m, msg, ptr, end, 0, kDNSRecordTypePacketAdd, &m->rec);
        if (ptr && m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && m->rec.r.resrec.rrtype == kDNSType_OPT)
        {
            const rdataOPT *o;
            const rdataOPT *const e = (const rdataOPT *)&m->rec.r.resrec.rdata->u.data[m->rec.r.resrec.rdlength];
            for (o = &m->rec.r.resrec.rdata->u.opt[0]; o < e; o++)
            {
                if      (o->opt == kDNSOpt_Lease) updatelease = o->u.updatelease;
                else if (o->opt == kDNSOpt_Owner && o->u.owner.vers == 0) owner       = o->u.owner;
            }
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }

    InitializeDNSMessage(&m->omsg.h, msg->h.id, UpdateRespFlags);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (!updatelease || !owner.HMAC.l[0])
#else
    if (!updatelease)
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    {
        static int msgs = 0;
        if (msgs < 100)
        {
            msgs++;
            LogMsg("Refusing sleep proxy registration from %#a:%d:%s%s", srcaddr, mDNSVal16(srcport),
                   !updatelease ? " No lease" : "", !owner.HMAC.l[0] ? " No owner" : "");
        }
        m->omsg.h.flags.b[1] |= kDNSFlag1_RC_FormErr;
    }
    else if (m->ProxyRecords + msg->h.mDNS_numUpdates > MAX_PROXY_RECORDS)
    {
        static int msgs = 0;
        if (msgs < 100)
        {
            msgs++;
            LogMsg("Refusing sleep proxy registration from %#a:%d: Too many records %d + %d = %d > %d", srcaddr, mDNSVal16(srcport),
                   m->ProxyRecords, msg->h.mDNS_numUpdates, m->ProxyRecords + msg->h.mDNS_numUpdates, MAX_PROXY_RECORDS);
        }
        m->omsg.h.flags.b[1] |= kDNSFlag1_RC_Refused;
    }
    else
    {
        LogSPS("Received Update for H-MAC %.6a I-MAC %.6a Password %.6a seq %d", &owner.HMAC, &owner.IMAC, &owner.password, owner.seq);

        if (updatelease > 24 * 60 * 60)
            updatelease = 24 * 60 * 60;

        if (updatelease > 0x40000000UL / mDNSPlatformOneSecond)
            updatelease = 0x40000000UL / mDNSPlatformOneSecond;

        ptr = LocateAuthorities(msg, end);

        // Clear any stale TCP keepalive records that may exist
        ClearKeepaliveProxyRecords(m, &owner, m->DuplicateRecords, InterfaceID);
        ClearKeepaliveProxyRecords(m, &owner, m->ResourceRecords, InterfaceID);

        for (i = 0; i < msg->h.mDNS_numUpdates && ptr && ptr < end; i++)
        {
            ptr = GetLargeResourceRecord(m, msg, ptr, end, InterfaceID, kDNSRecordTypePacketAuth, &m->rec);
            if (ptr && m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative)
            {
                mDNSu16 RDLengthMem = GetRDLengthMem(&m->rec.r.resrec);
                AuthRecord *ar = mDNSPlatformMemAllocate(sizeof(AuthRecord) - sizeof(RDataBody) + RDLengthMem);
                if (!ar)
                {
                    m->omsg.h.flags.b[1] |= kDNSFlag1_RC_Refused;
                    break;
                }
                else
                {
                    mDNSu8 RecordType = m->rec.r.resrec.RecordType & kDNSRecordTypePacketUniqueMask ? kDNSRecordTypeUnique : kDNSRecordTypeShared;
                    m->rec.r.resrec.rrclass &= ~kDNSClass_UniqueRRSet;
                    // All stale keepalive records have been flushed prior to this loop.
#ifndef REMOVE_WAKEUP_FUNCTIONALITY 
                   if (!mDNS_KeepaliveRecord(&m->rec.r.resrec))
                    {
                        ClearIdenticalProxyRecords(m, &owner, m->DuplicateRecords); // Make sure we don't have any old stale duplicates of this record
                        ClearIdenticalProxyRecords(m, &owner, m->ResourceRecords);
                    }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    mDNS_SetupResourceRecord(ar, mDNSNULL, InterfaceID, m->rec.r.resrec.rrtype, m->rec.r.resrec.rroriginalttl, RecordType, AuthRecordAny, SPSRecordCallback, ar);
                    AssignDomainName(&ar->namestorage, m->rec.r.resrec.name);
                    ar->resrec.rdlength = GetRDLength(&m->rec.r.resrec, mDNSfalse);
                    ar->resrec.rdata->MaxRDLength = RDLengthMem;
                    mDNSPlatformMemCopy(ar->resrec.rdata->u.data, m->rec.r.resrec.rdata->u.data, RDLengthMem);
                    ar->ForceMCast = mDNStrue;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    ar->WakeUp     = owner;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    if (m->rec.r.resrec.rrtype == kDNSType_PTR)
                    {
                        mDNSs32 t = ReverseMapDomainType(m->rec.r.resrec.name);
                        if      (t == mDNSAddrType_IPv4) GetIPv4FromName(&ar->AddressProxy, m->rec.r.resrec.name);
                        else if (t == mDNSAddrType_IPv6) GetIPv6FromName(&ar->AddressProxy, m->rec.r.resrec.name);
                        debugf("mDNSCoreReceiveUpdate: PTR %d %d %#a %s", t, ar->AddressProxy.type, &ar->AddressProxy, ARDisplayString(m, ar));
                        if (ar->AddressProxy.type) SetSPSProxyListChanged(InterfaceID);
                    }
                    ar->TimeRcvd   = m->timenow;
                    ar->TimeExpire = m->timenow + updatelease * mDNSPlatformOneSecond;
                    if (m->NextScheduledSPS - ar->TimeExpire > 0)
                        m->NextScheduledSPS = ar->TimeExpire;
                    ar->KATimeExpire = 0;
                    mDNS_Register_internal(m, ar);

                    m->ProxyRecords++;
                    mDNS_UpdateAllowSleep(m);
                    LogSPS("SPS Registered %4d %X %s", m->ProxyRecords, RecordType, ARDisplayString(m,ar));
                }
            }
            m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
        }

        if (m->omsg.h.flags.b[1] & kDNSFlag1_RC_Mask)
        {
            LogMsg("Refusing sleep proxy registration from %#a:%d: Out of memory", srcaddr, mDNSVal16(srcport));
            ClearProxyRecords(m, &owner, m->DuplicateRecords);
            ClearProxyRecords(m, &owner, m->ResourceRecords);
        }
        else
        {
            mDNS_SetupResourceRecord(&opt, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
            opt.resrec.rrclass    = NormalMaxDNSMessageData;
            opt.resrec.rdlength   = sizeof(rdataOPT);   // One option in this OPT record
            opt.resrec.rdestimate = sizeof(rdataOPT);
            opt.resrec.rdata->u.opt[0].opt           = kDNSOpt_Lease;
            opt.resrec.rdata->u.opt[0].u.updatelease = updatelease;
            p = PutResourceRecordTTLWithLimit(&m->omsg, p, &m->omsg.h.numAdditionals, &opt.resrec, opt.resrec.rroriginalttl, m->omsg.data + AbsoluteMaxDNSMessageData);
        }
    }

    if (p) mDNSSendDNSMessage(m, &m->omsg, p, InterfaceID, m->SPSSocket, srcaddr, srcport, mDNSNULL, mDNSNULL, mDNSfalse);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    mDNS_SendKeepalives(m);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
}

mDNSlocal void mDNSCoreReceiveUpdateR(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *end, const mDNSAddr *srcaddr, const mDNSInterfaceID InterfaceID)
{
    if (InterfaceID)
    {
        mDNSu32 updatelease = 60 * 60;      // If SPS fails to indicate lease time, assume one hour
        const mDNSu8 *ptr = LocateOptRR(msg, end, DNSOpt_LeaseData_Space);
        if (ptr)
        {
            ptr = GetLargeResourceRecord(m, msg, ptr, end, 0, kDNSRecordTypePacketAdd, &m->rec);
            if (ptr && m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && m->rec.r.resrec.rrtype == kDNSType_OPT)
            {
                const rdataOPT *o;
                const rdataOPT *const e = (const rdataOPT *)&m->rec.r.resrec.rdata->u.data[m->rec.r.resrec.rdlength];
                for (o = &m->rec.r.resrec.rdata->u.opt[0]; o < e; o++)
                    if (o->opt == kDNSOpt_Lease)
                    {
                        updatelease = o->u.updatelease;
                        LogSPS("Sleep Proxy granted lease time %4d seconds, updateid %d, InterfaceID %p", updatelease, mDNSVal16(msg->h.id), InterfaceID);
                    }
            }
            m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
        }

        if (m->CurrentRecord)
            LogMsg("mDNSCoreReceiveUpdateR ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
        m->CurrentRecord = m->ResourceRecords;
        while (m->CurrentRecord)
        {
            AuthRecord *const rr = m->CurrentRecord;
            if (rr->resrec.InterfaceID == InterfaceID || (!rr->resrec.InterfaceID && (rr->ForceMCast || IsLocalDomain(rr->resrec.name))))
                if (mDNSSameOpaque16(rr->updateid, msg->h.id))
                {
                    // We successfully completed this record's registration on this "InterfaceID". Clear that bit.
                    // Clear the updateid when we are done sending on all interfaces.
                    mDNSu32 scopeid = mDNSPlatformInterfaceIndexfromInterfaceID(m, InterfaceID, mDNStrue);
                    if (scopeid < (sizeof(rr->updateIntID) * mDNSNBBY))
                        bit_clr_opaque64(rr->updateIntID, scopeid);
                    if (mDNSOpaque64IsZero(&rr->updateIntID))
                        rr->updateid = zeroID;
                    rr->expire   = NonZeroTime(m->timenow + updatelease * mDNSPlatformOneSecond);
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    LogSPS("Sleep Proxy %s record %5d 0x%x 0x%x (%d) %s", rr->WakeUp.HMAC.l[0] ? "transferred" : "registered", updatelease, rr->updateIntID.l[1], rr->updateIntID.l[0], mDNSVal16(rr->updateid), ARDisplayString(m,rr));
                    if (rr->WakeUp.HMAC.l[0])
                    {
                        rr->WakeUp.HMAC = zeroEthAddr;  // Clear HMAC so that mDNS_Deregister_internal doesn't waste packets trying to wake this host
                        rr->RequireGoodbye = mDNSfalse; // and we don't want to send goodbye for it
                        mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
                    }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                }
            // Mustn't advance m->CurrentRecord until *after* mDNS_Deregister_internal, because
            // new records could have been added to the end of the list as a result of that call.
            if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
                m->CurrentRecord = rr->next;
        }

        // Update the dynamic store with the IP Address and MAC address of the sleep proxy
        char *ifname = InterfaceNameForID(m, InterfaceID);
        mDNSAddr spsaddr;
        mDNSPlatformMemCopy(&spsaddr, srcaddr, sizeof (mDNSAddr));
        mDNSPlatformStoreSPSMACAddr(&spsaddr, ifname);
    }
    // If we were waiting to go to sleep, then this SPS registration or wide-area record deletion
    // may have been the thing we were waiting for, so schedule another check to see if we can sleep now.
    if (m->SleepLimit) m->NextScheduledSPRetry = m->timenow;
}

mDNSexport void MakeNegativeCacheRecord(mDNS *const m, CacheRecord *const cr,
                                        const domainname *const name, const mDNSu32 namehash, const mDNSu16 rrtype, const mDNSu16 rrclass, mDNSu32 ttl_seconds, mDNSInterfaceID InterfaceID, DNSServer *dnsserver)
{
    if (cr == &m->rec.r && m->rec.r.resrec.RecordType)
    {
        LogMsg("MakeNegativeCacheRecord: m->rec appears to be already in use for %s", CRDisplayString(m, &m->rec.r));
#if ForceAlerts
        *(long*)0 = 0;
#endif
    }

    // Create empty resource record
    cr->resrec.RecordType    = kDNSRecordTypePacketNegative;
    cr->resrec.InterfaceID   = InterfaceID;
    cr->resrec.rDNSServer    = dnsserver;
    cr->resrec.name          = name;    // Will be updated to point to cg->name when we call CreateNewCacheEntry
    cr->resrec.rrtype        = rrtype;
    cr->resrec.rrclass       = rrclass;
    cr->resrec.rroriginalttl = ttl_seconds;
    cr->resrec.rdlength      = 0;
    cr->resrec.rdestimate    = 0;
    cr->resrec.namehash      = namehash;
    cr->resrec.rdatahash     = 0;
    cr->resrec.rdata = (RData*)&cr->smallrdatastorage;
    cr->resrec.rdata->MaxRDLength = 0;

    cr->NextInKAList       = mDNSNULL;
    cr->TimeRcvd           = m->timenow;
    cr->DelayDelivery      = 0;
    cr->NextRequiredQuery  = m->timenow;
    cr->LastUsed           = m->timenow;
    cr->CRActiveQuestion   = mDNSNULL;
    cr->UnansweredQueries  = 0;
    cr->LastUnansweredTime = 0;
#if ENABLE_MULTI_PACKET_QUERY_SNOOPING
    cr->MPUnansweredQ      = 0;
    cr->MPLastUnansweredQT = 0;
    cr->MPUnansweredKA     = 0;
    cr->MPExpectingKA      = mDNSfalse;
#endif
    cr->NextInCFList       = mDNSNULL;
    cr->nsec               = mDNSNULL;
    cr->soa                = mDNSNULL;
    cr->CRDNSSECQuestion   = 0;
    // Initialize to the basic one and the caller can set it to more
    // specific based on the response if any
    cr->responseFlags      = ResponseFlags;
}

mDNSexport void mDNSCoreReceive(mDNS *const m, void *const pkt, const mDNSu8 *const end,
                                const mDNSAddr *const srcaddr, const mDNSIPPort srcport, const mDNSAddr *dstaddr, const mDNSIPPort dstport,
                                const mDNSInterfaceID InterfaceID)
{
    mDNSInterfaceID ifid = InterfaceID;
    DNSMessage  *msg  = (DNSMessage *)pkt;
    const mDNSu8 StdQ = kDNSFlag0_QR_Query    | kDNSFlag0_OP_StdQuery;
    const mDNSu8 StdR = kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery;
    const mDNSu8 UpdQ = kDNSFlag0_QR_Query    | kDNSFlag0_OP_Update;
    const mDNSu8 UpdR = kDNSFlag0_QR_Response | kDNSFlag0_OP_Update;
    mDNSu8 QR_OP;
    mDNSu8 *ptr = mDNSNULL;
    mDNSBool TLS = (dstaddr == (mDNSAddr *)1);  // For debug logs: dstaddr = 0 means TCP; dstaddr = 1 means TLS
    if (TLS) dstaddr = mDNSNULL;

#ifndef UNICAST_DISABLED
    if (mDNSSameAddress(srcaddr, &m->Router))
    {
#ifdef _LEGACY_NAT_TRAVERSAL_
        if (mDNSSameIPPort(srcport, SSDPPort) || (m->SSDPSocket && mDNSSameIPPort(dstport, m->SSDPSocket->port)))
        {
            mDNS_Lock(m);
            LNT_ConfigureRouterInfo(m, InterfaceID, pkt, (mDNSu16)(end - (mDNSu8 *)pkt));
            mDNS_Unlock(m);
            return;
        }
#endif
        if (mDNSSameIPPort(srcport, NATPMPPort))
        {
            mDNS_Lock(m);
            uDNS_ReceiveNATPacket(m, InterfaceID, pkt, (mDNSu16)(end - (mDNSu8 *)pkt));
            mDNS_Unlock(m);
            return;
        }
    }
#ifdef _LEGACY_NAT_TRAVERSAL_
    else if (m->SSDPSocket && mDNSSameIPPort(dstport, m->SSDPSocket->port)) { debugf("Ignoring SSDP response from %#a:%d", srcaddr, mDNSVal16(srcport)); return; }
#endif

#endif
    if ((unsigned)(end - (mDNSu8 *)pkt) < sizeof(DNSMessageHeader))
    {
        LogMsg("DNS Message from %#a:%d to %#a:%d length %d too short", srcaddr, mDNSVal16(srcport), dstaddr, mDNSVal16(dstport), end - (mDNSu8 *)pkt);
        return;
    }
    QR_OP = (mDNSu8)(msg->h.flags.b[0] & kDNSFlag0_QROP_Mask);
    // Read the integer parts which are in IETF byte-order (MSB first, LSB second)
    ptr = (mDNSu8 *)&msg->h.numQuestions;
    msg->h.numQuestions   = (mDNSu16)((mDNSu16)ptr[0] << 8 | ptr[1]);
    msg->h.numAnswers     = (mDNSu16)((mDNSu16)ptr[2] << 8 | ptr[3]);
    msg->h.numAuthorities = (mDNSu16)((mDNSu16)ptr[4] << 8 | ptr[5]);
    msg->h.numAdditionals = (mDNSu16)((mDNSu16)ptr[6] << 8 | ptr[7]);

    if (!m) { LogMsg("mDNSCoreReceive ERROR m is NULL"); return; }

    // We use zero addresses and all-ones addresses at various places in the code to indicate special values like "no address"
    // If we accept and try to process a packet with zero or all-ones source address, that could really mess things up
    if (srcaddr && !mDNSAddressIsValid(srcaddr)) { debugf("mDNSCoreReceive ignoring packet from %#a", srcaddr); return; }

    mDNS_Lock(m);
    m->PktNum++;
    if (mDNSOpaque16IsZero(msg->h.id))
    {
        m->MPktNum++;
#if APPLE_OSX_mDNSResponder
        // Track the number of multicast packets received from a source outside our subnet.
        // Check the destination address to avoid accounting for spurious packets that
        // comes in with message id zero.
        if (!mDNS_AddressIsLocalSubnet(m, InterfaceID, srcaddr, mDNSNULL) &&
            mDNSAddressIsAllDNSLinkGroup(dstaddr))
        {
            m->RemoteSubnet++;
        }
#endif // #if APPLE_OSX_mDNSResponder
    }

#ifndef UNICAST_DISABLED
    if (!dstaddr || (!mDNSAddressIsAllDNSLinkGroup(dstaddr) && (QR_OP == StdR || QR_OP == UpdR)))
        if (!mDNSOpaque16IsZero(msg->h.id)) // uDNS_ReceiveMsg only needs to get real uDNS responses, not "QU" mDNS responses
        {
            ifid = mDNSInterface_Any;
            if (mDNS_PacketLoggingEnabled)
                DumpPacket(m, mStatus_NoError, mDNSfalse, TLS ? "TLS" : !dstaddr ? "TCP" : "UDP", srcaddr, srcport, dstaddr, dstport, msg, end);
            uDNS_ReceiveMsg(m, msg, end, srcaddr, srcport);
            // Note: mDNSCore also needs to get access to received unicast responses
        }
#endif
    if (QR_OP == StdQ) {
        mdnslogInfo("mDNSCoreReceive mDNSCoreReceiveQuery");
        mDNSCoreReceiveQuery(m, msg, end, srcaddr, srcport, dstaddr, dstport, ifid);
    } else if (QR_OP == StdR) {
        mdnslogInfo("mDNSCoreReceive mDNSCoreReceiveResponse");
        mDNSCoreReceiveResponse(m, msg, end, srcaddr, srcport, dstaddr, dstport, ifid);
    } else if (QR_OP == UpdQ) {
        mdnslogInfo("mDNSCoreReceive mDNSCoreReceiveUpdate");
        mDNSCoreReceiveUpdate(m, msg, end, srcaddr, srcport, dstaddr, dstport, InterfaceID);
    } else if (QR_OP == UpdR) {
        mdnslogInfo("mDNSCoreReceive mDNSCoreReceiveUpdateR");
        mDNSCoreReceiveUpdateR(m, msg, end, srcaddr, InterfaceID);
    }
    else
    {
        mdnslogInfo("mDNSCoreReceive Unknown DNS packet type");
        LogMsg("Unknown DNS packet type %02X%02X from %#-15a:%-5d to %#-15a:%-5d length %d on %p (ignored)",
               msg->h.flags.b[0], msg->h.flags.b[1], srcaddr, mDNSVal16(srcport), dstaddr, mDNSVal16(dstport), end - (mDNSu8 *)pkt, InterfaceID);
#ifndef REMOVE_mDNS_LoggingEnabled
        if (mDNS_LoggingEnabled)
        {
            int i = 0;
            while (i<end - (mDNSu8 *)pkt)
            {
                char buffer[128];
                char *p = buffer + mDNS_snprintf(buffer, sizeof(buffer), "%04X", i);
                do if (i<end - (mDNSu8 *)pkt) p += mDNS_snprintf(p, sizeof(buffer), " %02X", ((mDNSu8 *)pkt)[i]);while (++i & 15);
                LogInfo("%s", buffer);
            }
        }
#endif // #ifndef REMOVE_mDNS_LoggingEnabled
    }
    // Packet reception often causes a change to the task list:
    // 1. Inbound queries can cause us to need to send responses
    // 2. Conflicing response packets received from other hosts can cause us to need to send defensive responses
    // 3. Other hosts announcing deletion of shared records can cause us to need to re-assert those records
    // 4. Response packets that answer questions may cause our client to issue new questions
    mDNS_Unlock(m);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Searcher Functions
#endif

// Targets are considered the same if both queries are untargeted, or
// if both are targeted to the same address+port
// (If Target address is zero, TargetPort is undefined)
#define SameQTarget(A,B) (((A)->Target.type == mDNSAddrType_None && (B)->Target.type == mDNSAddrType_None) || \
                          (mDNSSameAddress(& (A)->Target, & (B)->Target) && mDNSSameIPPort((A)->TargetPort, (B)->TargetPort)))

// Note: We explicitly disallow making a public query be a duplicate of a private one. This is to avoid the
// circular deadlock where a client does a query for something like "dns-sd -Q _dns-query-tls._tcp.company.com SRV"
// and we have a key for company.com, so we try to locate the private query server for company.com, which necessarily entails
// doing a standard DNS query for the _dns-query-tls._tcp SRV record for company.com. If we make the latter (public) query
// a duplicate of the former (private) query, then it will block forever waiting for an answer that will never come.
//
// We keep SuppressUnusable questions separate so that we can return a quick response to them and not get blocked behind
// the queries that are not marked SuppressUnusable. But if the query is not suppressed, they are treated the same as
// non-SuppressUnusable questions. This should be fine as the goal of SuppressUnusable is to return quickly only if it
// is suppressed. If it is not suppressed, we do try all the DNS servers for valid answers like any other question.
// The main reason for this design is that cache entries point to a *single* question and that question is responsible
// for keeping the cache fresh as long as it is active. Having multiple active question for a single cache entry
// breaks this design principle.
//

// If IsLLQ(Q) is true, it means the question is both:
// (a) long-lived and
// (b) being performed by a unicast DNS long-lived query (either full LLQ, or polling)
// for multicast questions, we don't want to treat LongLived as anything special
#define IsLLQ(Q) ((Q)->LongLived && !mDNSOpaque16IsZero((Q)->TargetQID))

mDNSlocal DNSQuestion *FindDuplicateQuestion(const mDNS *const m, const DNSQuestion *const question)
{
    DNSQuestion *q;
    // Note: A question can only be marked as a duplicate of one that occurs *earlier* in the list.
    // This prevents circular references, where two questions are each marked as a duplicate of the other.
    // Accordingly, we break out of the loop when we get to 'question', because there's no point searching
    // further in the list.
    for (q = m->Questions; q && q != question; q=q->next)       // Scan our list for another question
        if (q->InterfaceID == question->InterfaceID &&          // with the same InterfaceID,
            SameQTarget(q, question)                &&          // and same unicast/multicast target settings
            q->qtype      == question->qtype        &&          // type,
            q->qclass     == question->qclass       &&          // class,
            IsLLQ(q)      == IsLLQ(question)        &&          // and long-lived status matches
            (!q->AuthInfo || question->AuthInfo)    &&          // to avoid deadlock, don't make public query dup of a private one
            (q->AnonInfo  == question->AnonInfo)    &&          // Anonymous query not a dup of normal query
            (q->SuppressQuery == question->SuppressQuery) &&    // Questions that are suppressed/not suppressed
            (q->ValidationRequired == question->ValidationRequired) &&  // Questions that require DNSSEC validation
            (q->ValidatingResponse == question->ValidatingResponse) &&  // Questions that are validating responses using DNSSEC
            (q->DisallowPID == question->DisallowPID)     &&            // Disallowing a PID should not affect a PID that is allowed
            (q->BrowseThreshold == question->BrowseThreshold) &&  // browse thresholds must match
            q->qnamehash  == question->qnamehash    &&
            SameDomainName(&q->qname, &question->qname))        // and name
            return(q);
    return(mDNSNULL);
}

// This is called after a question is deleted, in case other identical questions were being suppressed as duplicates
mDNSlocal void UpdateQuestionDuplicates(mDNS *const m, DNSQuestion *const question)
{
    DNSQuestion *q;
    DNSQuestion *first = mDNSNULL;

    // This is referring to some other question as duplicate. No other question can refer to this
    // question as a duplicate.
    if (question->DuplicateOf)
    {
        LogInfo("UpdateQuestionDuplicates: question %p %##s (%s) duplicate of %p %##s (%s)",
                question, question->qname.c, DNSTypeName(question->qtype),
                question->DuplicateOf, question->DuplicateOf->qname.c, DNSTypeName(question->DuplicateOf->qtype));
        return;
    }

    for (q = m->Questions; q; q=q->next)        // Scan our list of questions
        if (q->DuplicateOf == question)         // To see if any questions were referencing this as their duplicate
        {
            q->DuplicateOf = first;
            if (!first)
            {
                first = q;
                // If q used to be a duplicate, but now is not,
                // then inherit the state from the question that's going away
                q->LastQTime         = question->LastQTime;
                q->ThisQInterval     = question->ThisQInterval;
                q->ExpectUnicastResp = question->ExpectUnicastResp;
                q->LastAnswerPktNum  = question->LastAnswerPktNum;
                q->RecentAnswerPkts  = question->RecentAnswerPkts;
                q->RequestUnicast    = question->RequestUnicast;
                q->LastQTxTime       = question->LastQTxTime;
                q->CNAMEReferrals    = question->CNAMEReferrals;
                q->nta               = question->nta;
                q->servAddr          = question->servAddr;
                q->servPort          = question->servPort;
                q->qDNSServer        = question->qDNSServer;
                q->validDNSServers   = question->validDNSServers;
                q->unansweredQueries = question->unansweredQueries;
                q->noServerResponse  = question->noServerResponse;
                q->triedAllServersOnce = question->triedAllServersOnce;

                q->TargetQID         = question->TargetQID;
                if (q->LocalSocket)
                {
                    mDNSPlatformUDPClose(q->LocalSocket);
                }
                    
                q->LocalSocket       = question->LocalSocket;

                q->state             = question->state;
                //	q->tcp               = question->tcp;
                q->ReqLease          = question->ReqLease;
                q->expire            = question->expire;
                q->ntries            = question->ntries;
                q->id                = question->id;

                question->LocalSocket = mDNSNULL;
                question->nta        = mDNSNULL;    // If we've got a GetZoneData in progress, transfer it to the newly active question
                //	question->tcp        = mDNSNULL;

                if (q->LocalSocket)
                    debugf("UpdateQuestionDuplicates transferred LocalSocket pointer for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));

                if (q->nta)
                {
                    LogInfo("UpdateQuestionDuplicates transferred nta pointer for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
                    q->nta->ZoneDataContext = q;
                }

                // Need to work out how to safely transfer this state too -- appropriate context pointers need to be updated or the code will crash
                if (question->tcp) LogInfo("UpdateQuestionDuplicates did not transfer tcp pointer");

                if (question->state == LLQ_Established)
                {
                    LogInfo("UpdateQuestionDuplicates transferred LLQ state for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
                    question->state = 0;    // Must zero question->state, or mDNS_StopQuery_internal will clean up and cancel our LLQ from the server
                }

                SetNextQueryTime(m,q);
            }
        }
}

mDNSexport McastResolver *mDNS_AddMcastResolver(mDNS *const m, const domainname *d, const mDNSInterfaceID interface, mDNSu32 timeout)
{
    McastResolver **p = &m->McastResolvers;
    McastResolver *tmp = mDNSNULL;

    if (!d) d = (const domainname *)"";

    LogInfo("mDNS_AddMcastResolver: Adding %##s, InterfaceID %p, timeout %u", d->c, interface, timeout);

    mDNS_CheckLock(m);

    while (*p)  // Check if we already have this {interface, domain} tuple registered
    {
        if ((*p)->interface == interface && SameDomainName(&(*p)->domain, d))
        {
            if (!((*p)->flags & DNSServer_FlagDelete)) LogMsg("Note: Mcast Resolver domain %##s (%p) registered more than once", d->c, interface);
            (*p)->flags &= ~DNSServer_FlagDelete;
            tmp = *p;
            *p = tmp->next;
            tmp->next = mDNSNULL;
        }
        else
            p=&(*p)->next;
    }

    if (tmp) *p = tmp; // move to end of list, to ensure ordering from platform layer
    else
    {
        // allocate, add to list
        *p = mDNSPlatformMemAllocate(sizeof(**p));
        if (!*p) LogMsg("mDNS_AddMcastResolver: ERROR!! - malloc");
        else
        {
            (*p)->interface = interface;
            (*p)->flags     = DNSServer_FlagNew;
            (*p)->timeout   = timeout;
            AssignDomainName(&(*p)->domain, d);
            (*p)->next = mDNSNULL;
        }
    }
    return(*p);
}

mDNSinline mDNSs32 PenaltyTimeForServer(mDNS *m, DNSServer *server)
{
    mDNSs32 ptime = 0;
    if (server->penaltyTime != 0)
    {
        ptime = server->penaltyTime - m->timenow;
        if (ptime < 0)
        {
            // This should always be a positive value between 0 and DNSSERVER_PENALTY_TIME
            // If it does not get reset in ResetDNSServerPenalties for some reason, we do it
            // here
            LogMsg("PenaltyTimeForServer: PenaltyTime negative %d, (server penaltyTime %d, timenow %d) resetting the penalty",
                   ptime, server->penaltyTime, m->timenow);
            server->penaltyTime = 0;
            ptime = 0;
        }
    }
    return ptime;
}

//Checks to see whether the newname is a better match for the name, given the best one we have
//seen so far (given in bestcount).
//Returns -1 if the newname is not a better match
//Returns 0 if the newname is the same as the old match
//Returns 1 if the newname is a better match
mDNSlocal int BetterMatchForName(const domainname *name, int namecount, const domainname *newname, int newcount,
                                 int bestcount)
{
    // If the name contains fewer labels than the new server's domain or the new name
    // contains fewer labels than the current best, then it can't possibly be a better match
    if (namecount < newcount || newcount < bestcount) return -1;

    // If there is no match, return -1 and the caller will skip this newname for
    // selection
    //
    // If we find a match and the number of labels is the same as bestcount, then
    // we return 0 so that the caller can do additional logic to pick one of
    // the best based on some other factors e.g., penaltyTime
    //
    // If we find a match and the number of labels is more than bestcount, then we
    // return 1 so that the caller can pick this over the old one.
    //
    // Note: newcount can either be equal or greater than bestcount beause of the
    // check above.

    if (SameDomainName(SkipLeadingLabels(name, namecount - newcount), newname))
        return bestcount == newcount ? 0 : 1;
    else
        return -1;
}

// Normally, we have McastResolvers for .local, in-addr.arpa and ip6.arpa. But there
// can be queries that can forced to multicast (ForceMCast) even though they don't end in these
// names. In that case, we give a default timeout of 5 seconds
#define DEFAULT_MCAST_TIMEOUT   5
mDNSlocal mDNSu32 GetTimeoutForMcastQuestion(mDNS *m, DNSQuestion *question)
{
    McastResolver *curmatch = mDNSNULL;
    int bestmatchlen = -1, namecount = CountLabels(&question->qname);
    McastResolver *curr;
    int bettermatch, currcount;
    for (curr = m->McastResolvers; curr; curr = curr->next)
    {
        currcount = CountLabels(&curr->domain);
        bettermatch = BetterMatchForName(&question->qname, namecount, &curr->domain, currcount, bestmatchlen);
        // Take the first best match. If there are multiple equally good matches (bettermatch = 0), we take
        // the timeout value from the first one
        if (bettermatch == 1)
        {
            curmatch = curr;
            bestmatchlen = currcount;
        }
    }
    LogInfo("GetTimeoutForMcastQuestion: question %##s curmatch %p, Timeout %d", question->qname.c, curmatch,
            curmatch ? curmatch->timeout : DEFAULT_MCAST_TIMEOUT);
    return ( curmatch ? curmatch->timeout : DEFAULT_MCAST_TIMEOUT);
}

// Returns true if it is a Domain Enumeration Query
mDNSexport mDNSBool DomainEnumQuery(const domainname *qname)
{
    const mDNSu8 *mDNS_DEQLabels[] = { (const mDNSu8 *)"\001b", (const mDNSu8 *)"\002db", (const mDNSu8 *)"\002lb",
                                       (const mDNSu8 *)"\001r", (const mDNSu8 *)"\002dr", (const mDNSu8 *)mDNSNULL, };
    const domainname *d = qname;
    const mDNSu8 *label;
    int i = 0;

    // We need at least 3 labels (DEQ prefix) + one more label to make a meaningful DE query
    if (CountLabels(qname) < 4) { debugf("DomainEnumQuery: question %##s, not enough labels", qname->c); return mDNSfalse; }

    label = (const mDNSu8 *)d;
    while (mDNS_DEQLabels[i] != (const mDNSu8 *)mDNSNULL)
    {
        if (SameDomainLabel(mDNS_DEQLabels[i], label)) {debugf("DomainEnumQuery: DEQ %##s, label1 match", qname->c); break;}
        i++;
    }
    if (mDNS_DEQLabels[i] == (const mDNSu8 *)mDNSNULL)
    {
        debugf("DomainEnumQuery: Not a DEQ %##s, label1 mismatch", qname->c);
        return mDNSfalse;
    }
    debugf("DomainEnumQuery: DEQ %##s, label1 match", qname->c);

    // CountLabels already verified the number of labels
    d = (const domainname *)(d->c + 1 + d->c[0]);   // Second Label
    label = (const mDNSu8 *)d;
    if (!SameDomainLabel(label, (const mDNSu8 *)"\007_dns-sd"))
    {
        debugf("DomainEnumQuery: Not a DEQ %##s, label2 mismatch", qname->c);
        return(mDNSfalse);
    }
    debugf("DomainEnumQuery: DEQ %##s, label2 match", qname->c);

    d = (const domainname *)(d->c + 1 + d->c[0]);   // Third Label
    label = (const mDNSu8 *)d;
    if (!SameDomainLabel(label, (const mDNSu8 *)"\004_udp"))
    {
        debugf("DomainEnumQuery: Not a DEQ %##s, label3 mismatch", qname->c);
        return(mDNSfalse);
    }
    debugf("DomainEnumQuery: DEQ %##s, label3 match", qname->c);

    debugf("DomainEnumQuery: Question %##s is a Domain Enumeration query", qname->c);

    return mDNStrue;
}

// Note: InterfaceID is the InterfaceID of the question
mDNSlocal mDNSBool DNSServerMatch(DNSServer *d, mDNSInterfaceID InterfaceID, mDNSs32 ServiceID)
{
    // 1) Unscoped questions (NULL InterfaceID) should  consider *only* unscoped DNSServers ( DNSServer
    // with "scoped" set to kScopeNone)
    //
    // 2) Scoped questions (non-NULL InterfaceID) should consider *only* scoped DNSServers (DNSServer
    // with "scoped" set to kScopeInterfaceId) and their InterfaceIDs should match.
    //
    // 3) Scoped questions (non-zero ServiceID) should consider *only* scoped DNSServers (DNSServer
    // with "scoped" set to kScopeServiceID) and their ServiceIDs should match.
    //    
    // The first condition in the "if" statement checks to see if both the question and the DNSServer are
    // unscoped. The question is unscoped only if InterfaceID is zero and ServiceID is -1.
    //
    // If the first condition fails, following are the possible cases (the notes below are using
    // InterfaceID for discussion and the same holds good for ServiceID):
    //
    // - DNSServer is not scoped, InterfaceID is not NULL - we should skip the current DNSServer entry
    //   as scoped questions should not pick non-scoped DNSServer entry (Refer to (2) above).
    //
    // - DNSServer is scoped, InterfaceID is NULL - we should skip the current DNSServer entry as
    //   unscoped question should not match scoped DNSServer (Refer to (1) above). The InterfaceID check
    //   would fail in this case.
    //
    // - DNSServer is scoped and InterfaceID is not NULL - the InterfaceID of the question and the DNSServer
    //   should match (Refer to (2) above).
    //
    // Note: mDNSInterface_Unicast is used only by .local unicast questions and are treated as unscoped.
    // If a question is scoped both to InterfaceID and ServiceID, the question will be scoped to InterfaceID.

    if (((d->scoped == kScopeNone) && ((!InterfaceID && ServiceID == -1) || InterfaceID == mDNSInterface_Unicast)) ||
        ((d->scoped == kScopeInterfaceID) && d->interface == InterfaceID) ||
        ((d->scoped == kScopeServiceID) && d->serviceID == ServiceID))
    {
        return mDNStrue;
    }
    return mDNSfalse;
}

// Sets all the Valid DNS servers for a question
mDNSexport mDNSu32 SetValidDNSServers(mDNS *m, DNSQuestion *question)
{
    int bestmatchlen = -1, namecount = CountLabels(&question->qname);
    DNSServer *curr;
    int bettermatch, currcount;
    int index = 0;
    mDNSu32 timeout = 0;
    mDNSBool DEQuery;

    question->validDNSServers = zeroOpaque64;
    DEQuery = DomainEnumQuery(&question->qname);
    for (curr = m->DNSServers; curr; curr = curr->next)
    {
        debugf("SetValidDNSServers: Parsing DNS server Address %#a (Domain %##s), Scope: %d", &curr->addr, curr->domain.c, curr->scoped);
        // skip servers that will soon be deleted
        if (curr->flags & DNSServer_FlagDelete)
        {
            debugf("SetValidDNSServers: Delete set for index %d, DNS server %#a (Domain %##s), scoped %d", index, &curr->addr, curr->domain.c, curr->scoped);
            continue;
        }

        // This happens normally when you unplug the interface where we reset the interfaceID to mDNSInterface_Any for all
        // the DNS servers whose scope match the interfaceID. Few seconds later, we also receive the updated DNS configuration.
        // But any questions that has mDNSInterface_Any scope that are started/restarted before we receive the update
        // (e.g., CheckSuppressUnusableQuestions is called when interfaces are deregistered with the core) should not
        // match the scoped entries by mistake.
        //
        // Note: DNS configuration change will help pick the new dns servers but currently it does not affect the timeout

        if (curr->scoped && curr->interface == mDNSInterface_Any)
        {
            debugf("SetValidDNSServers: Scoped DNS server %#a (Domain %##s) with Interface Any", &curr->addr, curr->domain.c);
            continue;
        }

        currcount = CountLabels(&curr->domain);
        if ((!DEQuery || !curr->cellIntf) && DNSServerMatch(curr, question->InterfaceID, question->ServiceID))
        {
            bettermatch = BetterMatchForName(&question->qname, namecount, &curr->domain, currcount, bestmatchlen);

            // If we found a better match (bettermatch == 1) then clear all the bits
            // corresponding to the old DNSServers that we have may set before and start fresh.
            // If we find an equal match, then include that DNSServer also by setting the corresponding
            // bit
            if ((bettermatch == 1) || (bettermatch == 0))
            {
                bestmatchlen = currcount;
                if (bettermatch)
                {
                    debugf("SetValidDNSServers: Resetting all the bits");
                    question->validDNSServers = zeroOpaque64;
                    timeout = 0;
                }
                debugf("SetValidDNSServers: question %##s Setting the bit for DNS server Address %#a (Domain %##s), Scoped:%d index %d,"
                       " Timeout %d, interface %p", question->qname.c, &curr->addr, curr->domain.c, curr->scoped, index, curr->timeout,
                       curr->interface);
                timeout += curr->timeout;
                if (DEQuery)
                    debugf("DomainEnumQuery: Question %##s, DNSServer %#a, cell %d", question->qname.c, &curr->addr, curr->cellIntf);
                bit_set_opaque64(question->validDNSServers, index);
            }
        }
        index++;
    }
    question->noServerResponse = 0;

    debugf("SetValidDNSServers: ValidDNSServer bits  0x%x%x for question %p %##s (%s)",
           question->validDNSServers.l[1], question->validDNSServers.l[0], question, question->qname.c, DNSTypeName(question->qtype));
    // If there are no matching resolvers, then use the default timeout value.
    // For ProxyQuestion, shorten the timeout so that dig does not timeout on us in case of no response.
    return ((question->ProxyQuestion || question->ValidatingResponse) ? DEFAULT_UDNSSEC_TIMEOUT : timeout ? timeout : DEFAULT_UDNS_TIMEOUT);
}

// Get the Best server that matches a name. If you find penalized servers, look for the one
// that will come out of the penalty box soon
mDNSlocal DNSServer *GetBestServer(mDNS *m, const domainname *name, mDNSInterfaceID InterfaceID, mDNSs32 ServiceID, mDNSOpaque64 validBits,
    int *selected, mDNSBool nameMatch)
{
    DNSServer *curmatch = mDNSNULL;
    int bestmatchlen = -1, namecount = name ? CountLabels(name) : 0;
    DNSServer *curr;
    mDNSs32 bestPenaltyTime, currPenaltyTime;
    int bettermatch, currcount;
    int index = 0;
    int currindex = -1;

    debugf("GetBestServer: ValidDNSServer bits  0x%x%x", validBits.l[1], validBits.l[0]);
    bestPenaltyTime = DNSSERVER_PENALTY_TIME + 1;
    for (curr = m->DNSServers; curr; curr = curr->next)
    {
        // skip servers that will soon be deleted
        if (curr->flags & DNSServer_FlagDelete)
        {
            debugf("GetBestServer: Delete set for index %d, DNS server %#a (Domain %##s), scoped %d", index, &curr->addr, curr->domain.c, curr->scoped);
            continue;
        }

        // Check if this is a valid DNSServer
        if (!bit_get_opaque64(validBits, index))
        {
            debugf("GetBestServer: continuing for index %d", index);
            index++;
            continue;
        }

        currcount = CountLabels(&curr->domain);
        currPenaltyTime = PenaltyTimeForServer(m, curr);

        debugf("GetBestServer: Address %#a (Domain %##s), PenaltyTime(abs) %d, PenaltyTime(rel) %d",
               &curr->addr, curr->domain.c, curr->penaltyTime, currPenaltyTime);

        // If there are multiple best servers for a given question, we will pick the first one
        // if none of them are penalized. If some of them are penalized in that list, we pick
        // the least penalized one. BetterMatchForName walks through all best matches and
        // "currPenaltyTime < bestPenaltyTime" check lets us either pick the first best server
        // in the list when there are no penalized servers and least one among them
        // when there are some penalized servers.

        if (DNSServerMatch(curr, InterfaceID, ServiceID))
        {

            // If we know that all the names are already equally good matches, then skip calling BetterMatchForName.
            // This happens when we initially walk all the DNS servers and set the validity bit on the question.
            // Actually we just need PenaltyTime match, but for the sake of readability we just skip the expensive
            // part and still do some redundant steps e.g., InterfaceID match

            if (nameMatch)
                bettermatch = BetterMatchForName(name, namecount, &curr->domain, currcount, bestmatchlen);
            else
                bettermatch = 0;

            // If we found a better match (bettermatch == 1) then we don't need to
            // compare penalty times. But if we found an equal match, then we compare
            // the penalty times to pick a better match

            if ((bettermatch == 1) || ((bettermatch == 0) && currPenaltyTime < bestPenaltyTime))
            {
                currindex = index;
                curmatch = curr;
                bestmatchlen = currcount;
                bestPenaltyTime = currPenaltyTime;
            }
        }
        index++;
    }
    if (selected) *selected = currindex;
    return curmatch;
}

// Look up a DNS Server, matching by name and InterfaceID
mDNSlocal DNSServer *GetServerForName(mDNS *m, const domainname *name, mDNSInterfaceID InterfaceID, mDNSs32 ServiceID)
{
    DNSServer *curmatch = mDNSNULL;
    char *ifname = mDNSNULL;    // for logging purposes only
    mDNSOpaque64 allValid;

    if ((InterfaceID == mDNSInterface_Unicast) || (InterfaceID == mDNSInterface_LocalOnly))
        InterfaceID = mDNSNULL;

    if (InterfaceID) ifname = InterfaceNameForID(m, InterfaceID);

    // By passing in all ones, we make sure that every DNS server is considered
    allValid.l[0] = allValid.l[1] = 0xFFFFFFFF;

    curmatch = GetBestServer(m, name, InterfaceID, ServiceID, allValid, mDNSNULL, mDNStrue);

    if (curmatch != mDNSNULL)
        LogInfo("GetServerForName: DNS server %#a:%d (Penalty Time Left %d) (Scope %s:%p) found for name %##s", &curmatch->addr,
                mDNSVal16(curmatch->port), (curmatch->penaltyTime ? (curmatch->penaltyTime - m->timenow) : 0), ifname ? ifname : "None",
                InterfaceID, name);
    else
        LogInfo("GetServerForName: no DNS server (Scope %s:%p) found for name %##s", ifname ? ifname : "None", InterfaceID, name);

    return(curmatch);
}

// Look up a DNS Server for a question within its valid DNSServer bits
mDNSexport DNSServer *GetServerForQuestion(mDNS *m, DNSQuestion *question)
{
    DNSServer *curmatch = mDNSNULL;
    char *ifname = mDNSNULL;    // for logging purposes only
    mDNSInterfaceID InterfaceID = question->InterfaceID;
    const domainname *name = &question->qname;
    int currindex;

    if ((InterfaceID == mDNSInterface_Unicast) || (InterfaceID == mDNSInterface_LocalOnly))
        InterfaceID = mDNSNULL;

    if (InterfaceID)
        ifname = InterfaceNameForID(m, InterfaceID);

    if (!mDNSOpaque64IsZero(&question->validDNSServers))
    {
        curmatch = GetBestServer(m, name, InterfaceID, question->ServiceID, question->validDNSServers, &currindex, mDNSfalse);
        if (currindex != -1)
            bit_clr_opaque64(question->validDNSServers, currindex);
    }

    if (curmatch != mDNSNULL)
    {
        LogInfo("GetServerForQuestion: %p DNS server (%p) %#a:%d (Penalty Time Left %d) (Scope %s:%p:%d) found for name %##s (%s)",
                question, curmatch, &curmatch->addr, mDNSVal16(curmatch->port),
                (curmatch->penaltyTime ? (curmatch->penaltyTime - m->timenow) : 0), ifname ? ifname : "None",
                InterfaceID, question->ServiceID, name, DNSTypeName(question->qtype));
    }
    else
    {
        LogInfo("GetServerForQuestion: %p no DNS server (Scope %s:%p:%d) found for name %##s (%s)",
            question, ifname ? ifname : "None", InterfaceID, question->ServiceID, name, DNSTypeName(question->qtype));
    }

    return(curmatch);
}


#define ValidQuestionTarget(Q) (((Q)->Target.type == mDNSAddrType_IPv4 || (Q)->Target.type == mDNSAddrType_IPv6) && \
                                (mDNSSameIPPort((Q)->TargetPort, UnicastDNSPort) || mDNSSameIPPort((Q)->TargetPort, MulticastDNSPort)))

#ifndef REMOVE_NAT_FUCTIONALITY
// Called in normal client context (lock not held)
mDNSlocal void LLQNATCallback(mDNS *m, NATTraversalInfo *n)
{
    DNSQuestion *q;
    mDNS_Lock(m);
    LogInfo("LLQNATCallback external address:port %.4a:%u, NAT result %d", &n->ExternalAddress, mDNSVal16(n->ExternalPort), n->Result);
    n->clientContext = mDNSNULL; // we received at least one callback since starting this NAT-T
    for (q = m->Questions; q; q=q->next)
        if (ActiveQuestion(q) && !mDNSOpaque16IsZero(q->TargetQID) && q->LongLived)
            startLLQHandshake(m, q);    // If ExternalPort is zero, will do StartLLQPolling instead
#if APPLE_OSX_mDNSResponder
    UpdateAutoTunnelDomainStatuses(m);
#endif
    mDNS_Unlock(m);
}
#endif //#ifndef REMOVE_NAT_FUCTIONALITY

#ifndef REMOVE_TUNNELING
mDNSlocal mDNSBool IsPrivateDomain(mDNS *const m, DNSQuestion *q)
{
    DomainAuthInfo *AuthInfo;
    // Skip Private domains as we have special addresses to get the hosts in the Private domain
    AuthInfo = GetAuthInfoForName_internal(m, &q->qname);
    if (AuthInfo && !AuthInfo->deltime && AuthInfo->AutoTunnel)
    {
        debugf("IsPrivateDomain: %##s true", q->qname.c);
        return mDNStrue;
    }
    else
    {
        debugf("IsPrivateDomain: %##s false", q->qname.c);
        return mDNSfalse;
    }
}
#endif // #ifndef REMOVE_TUNNELING

// This function takes the DNSServer as a separate argument because sometimes the
// caller has not yet assigned the DNSServer, but wants to evaluate the SuppressQuery
// status before switching to it.
mDNSlocal mDNSBool ShouldSuppressUnicastQuery(mDNS *const m, DNSQuestion *q, DNSServer *d)
{
    // Some callers don't check for the qtype
    if (q->qtype != kDNSType_A && q->qtype != kDNSType_AAAA)
    {
        LogInfo("ShouldSuppressUnicastQuery: Query not suppressed for %##s, qtype %s, not A/AAAA type", q->qname.c, DNSTypeName(q->qtype));
        return mDNSfalse;
    }

#ifndef REMOVE_TUNNELING
    // Private domains are exempted irrespective of what the DNSServer says
    if (IsPrivateDomain(m, q))
    {
        LogInfo("ShouldSuppressUnicastQuery: Query not suppressed for %##s, qtype %s, Private Domain", q->qname.c, DNSTypeName(q->qtype));
        return mDNSfalse;
    }

#endif // #ifndef REMOVE_TUNNELING
    if (!d)
    {
        LogInfo("ShouldSuppressUnicastQuery: Query suppressed for %##s, qtype %s, as the DNS server is NULL", q->qname.c, DNSTypeName(q->qtype));
        return mDNStrue;
    }
    
    // Check if the DNS Configuration allows A/AAAA queries to be sent
    if ((q->qtype == kDNSType_A) && (d->req_A))
    {
        LogInfo("ShouldSuppressUnicastQuery: Query not suppressed for %##s, qtype %s, DNSServer %##s %#a:%d allows A queries", q->qname.c,
                DNSTypeName(q->qtype), d->domain.c, &d->addr, mDNSVal16(d->port));
        return mDNSfalse;
    }
    if ((q->qtype == kDNSType_AAAA) && (d->req_AAAA))
    {
        LogInfo("ShouldSuppressUnicastQuery: Query not suppressed for %##s, qtype %s, DNSServer %##s %#a:%d allows AAAA queries", q->qname.c,
                DNSTypeName(q->qtype), d->domain.c, &d->addr, mDNSVal16(d->port));
        return mDNSfalse;
    }

    LogInfo("ShouldSuppressUnicastQuery: Query suppressed for %##s, qtype %s, since DNS Configuration does not allow (req_A is %s and req_AAAA is %s)",
        q->qname.c, DNSTypeName(q->qtype), d->req_A ? "true" : "false", d->req_AAAA ? "true" : "false");

    return mDNStrue;
}

mDNSlocal mDNSBool ShouldSuppressDotLocalQuery(mDNS *const m, DNSQuestion *q)
{
    NetworkInterfaceInfo *intf;
    AuthRecord *rr;
    mDNSBool ret;

    // Check to see if there is at least one interface other than loopback and don't suppress
    // .local questions if you find one. If we have at least one interface, it means that
    // we can send unicast queries for the .local name and we don't want to suppress
    // multicast in that case as upper layers don't know how to handle if we return a 
    // negative response for multicast followed by a positive response for unicast.
    //
    // Note: we used to check for multicast capable interfaces instead of just any interface
    // present. That did not work in the case where we have a valid interface for unicast
    // but not multicast capable e.g., cellular, as we ended up delivering a negative response 
    // first and the upper layer did not wait for the positive response that came later.
    for (intf = m->HostInterfaces; intf; intf = intf->next)
    {
        if (intf->InterfaceActive && !intf->Loopback)
        {
            LogInfo("ShouldSuppressDotLocalQuery: Found interface %s, not suppressing", intf->ifname);
            return mDNSfalse;
        }
    }

    // 1. If we find a LocalOnly or P2P record answering this question, then don't suppress it.
    //    Set m->CurrentQuestion as it is required by AnswerQuestionWithLORecord.
    m->CurrentQuestion = q;
    ret = AnswerQuestionWithLORecord(m, q, mDNStrue);
    m->CurrentQuestion = mDNSNULL;

    if (ret)
    {
        LogInfo("ShouldSuppressDotLocalQuery: Found LocalOnly record for %##s (%s), not suppressing", q->qname.c,
            DNSTypeName(q->qtype));
        return mDNSfalse;
    }

    // 2. If we find a local AuthRecord answering this question, then don't suppress it. 
    for (rr = m->ResourceRecords; rr; rr = rr->next)
    {
        if (ResourceRecordAnswersQuestion(&rr->resrec, q))
        {
            LogInfo("ShouldSuppressDotLocalQuery: Found resource record %s for %##s (%s) not suppressing", ARDisplayString(m, rr),
                q->qname.c, DNSTypeName(q->qtype));
            return mDNSfalse;
        }
    }
    return mDNStrue;
}

mDNSlocal mDNSBool ShouldSuppressQuery(mDNS *const m, DNSQuestion *q)
{
    if (q->qtype != kDNSType_A && q->qtype != kDNSType_AAAA)
    {
        LogInfo("ShouldSuppressQuery: Query not suppressed for %##s, qtype %s, not A/AAAA type", q->qname.c, DNSTypeName(q->qtype));
        return mDNSfalse;
    }

    // We still want the ability to be able to listen to the local services and hence
    // don't fail .local query if we have local records that can potentially answer
    // the question.
    if (q->InterfaceID != mDNSInterface_Unicast && IsLocalDomain(&q->qname))
    {
        if (!ShouldSuppressDotLocalQuery(m, q))
        {
            LogInfo("ShouldSuppressQuery: Query not suppressed for %##s, qtype %s, Local question", q->qname.c, DNSTypeName(q->qtype));
            return mDNSfalse;
        }
        else
        {
            LogInfo("ShouldSuppressQuery: Query suppressed for %##s, qtype %s, Local question", q->qname.c, DNSTypeName(q->qtype));
            return mDNStrue;
        }
    }

    return (ShouldSuppressUnicastQuery(m, q, q->qDNSServer));
}

mDNSlocal void CacheRecordRmvEventsForCurrentQuestion(mDNS *const m, DNSQuestion *q)
{
    CacheRecord *rr;
    mDNSu32 slot;
    CacheGroup *cg;

    slot = HashSlot(&q->qname);
    cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
    {
        // Don't deliver RMV events for negative records
        if (rr->resrec.RecordType == kDNSRecordTypePacketNegative)
        {
            LogInfo("CacheRecordRmvEventsForCurrentQuestion: CacheRecord %s Suppressing RMV events for question %p %##s (%s), CRActiveQuestion %p, CurrentAnswers %d",
                    CRDisplayString(m, rr), q, q->qname.c, DNSTypeName(q->qtype), rr->CRActiveQuestion, q->CurrentAnswers);
            continue;
        }

        if (SameNameRecordAnswersQuestion(&rr->resrec, q))
        {
            LogInfo("CacheRecordRmvEventsForCurrentQuestion: Calling AnswerCurrentQuestionWithResourceRecord (RMV) for question %##s using resource record %s LocalAnswers %d",
                    q->qname.c, CRDisplayString(m, rr), q->LOAddressAnswers);

            q->CurrentAnswers--;
            if (rr->resrec.rdlength > SmallRecordLimit) q->LargeAnswers--;
            if (rr->resrec.RecordType & kDNSRecordTypePacketUniqueMask) q->UniqueAnswers--;

            if (rr->CRActiveQuestion == q)
            {
                DNSQuestion *qptr;
                // If this was the active question for this cache entry, it was the one that was
                // responsible for keeping the cache entry fresh when the cache entry was reaching
                // its expiry. We need to handover the responsibility to someone else. Otherwise,
                // when the cache entry is about to expire, we won't find an active question
                // (pointed by CRActiveQuestion) to refresh the cache.
                for (qptr = m->Questions; qptr; qptr=qptr->next)
                    if (qptr != q && ActiveQuestion(qptr) && ResourceRecordAnswersQuestion(&rr->resrec, qptr))
                        break;

                if (qptr)
                    LogInfo("CacheRecordRmvEventsForCurrentQuestion: Updating CRActiveQuestion to %p for cache record %s, "
                            "Original question CurrentAnswers %d, new question CurrentAnswers %d, SuppressUnusable %d, SuppressQuery %d",
                            qptr, CRDisplayString(m,rr), q->CurrentAnswers, qptr->CurrentAnswers, qptr->SuppressUnusable, qptr->SuppressQuery);

                rr->CRActiveQuestion = qptr;        // Question used to be active; new value may or may not be null
                if (!qptr) m->rrcache_active--; // If no longer active, decrement rrcache_active count
            }
            AnswerCurrentQuestionWithResourceRecord(m, rr, QC_rmv);
            if (m->CurrentQuestion != q) break;     // If callback deleted q, then we're finished here
        }
    }
}

mDNSlocal mDNSBool IsQuestionNew(mDNS *const m, DNSQuestion *question)
{
    DNSQuestion *q;
    for (q = m->NewQuestions; q; q = q->next)
        if (q == question) return mDNStrue;
    return mDNSfalse;
}

mDNSlocal mDNSBool LocalRecordRmvEventsForQuestion(mDNS *const m, DNSQuestion *q)
{
    AuthRecord *rr;
    mDNSu32 slot;
    AuthGroup *ag;

    if (m->CurrentQuestion)
        LogMsg("LocalRecordRmvEventsForQuestion: ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));

    if (IsQuestionNew(m, q))
    {
        LogInfo("LocalRecordRmvEventsForQuestion: New Question %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        return mDNStrue;
    }
    m->CurrentQuestion = q;
    slot = AuthHashSlot(&q->qname);
    ag = AuthGroupForName(&m->rrauth, slot, q->qnamehash, &q->qname);
    if (ag)
    {
        for (rr = ag->members; rr; rr=rr->next)
            // Filter the /etc/hosts records - LocalOnly, Unique, A/AAAA/CNAME
            if (UniqueLocalOnlyRecord(rr) && LocalOnlyRecordAnswersQuestion(rr, q))
            {
                LogInfo("LocalRecordRmvEventsForQuestion: Delivering possible Rmv events with record %s",
                        ARDisplayString(m, rr));
                if (q->CurrentAnswers <= 0 || q->LOAddressAnswers <= 0)
                {
                    LogMsg("LocalRecordRmvEventsForQuestion: ERROR!! CurrentAnswers or LOAddressAnswers is zero %p %##s"
                           " (%s) CurrentAnswers %d, LOAddressAnswers %d", q, q->qname.c, DNSTypeName(q->qtype),
                           q->CurrentAnswers, q->LOAddressAnswers);
                    continue;
                }
                AnswerLocalQuestionWithLocalAuthRecord(m, rr, QC_rmv);      // MUST NOT dereference q again
                if (m->CurrentQuestion != q) { m->CurrentQuestion = mDNSNULL; return mDNSfalse; }
            }
    }
    m->CurrentQuestion = mDNSNULL;
    return mDNStrue;
}

// Returns false if the question got deleted while delivering the RMV events
// The caller should handle the case
mDNSlocal mDNSBool CacheRecordRmvEventsForQuestion(mDNS *const m, DNSQuestion *q)
{
    if (m->CurrentQuestion)
        LogMsg("CacheRecordRmvEventsForQuestion: ERROR m->CurrentQuestion already set: %##s (%s)",
               m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));

    // If it is a new question, we have not delivered any ADD events yet. So, don't deliver RMV events.
    // If this question was answered using local auth records, then you can't deliver RMVs using cache
    if (!IsQuestionNew(m, q) && !q->LOAddressAnswers)
    {
        m->CurrentQuestion = q;
        CacheRecordRmvEventsForCurrentQuestion(m, q);
        if (m->CurrentQuestion != q) { m->CurrentQuestion = mDNSNULL; return mDNSfalse; }
        m->CurrentQuestion = mDNSNULL;
    }
    else { LogInfo("CacheRecordRmvEventsForQuestion: Question %p %##s (%s) is a new question", q, q->qname.c, DNSTypeName(q->qtype)); }
    return mDNStrue;
}

mDNSlocal void SuppressStatusChanged(mDNS *const m, DNSQuestion *q, DNSQuestion **restart)
{
    // NOTE: CacheRecordRmvEventsForQuestion will not generate RMV events for queries that have non-zero
    // LOddressAnswers. Hence it is important that we call CacheRecordRmvEventsForQuestion before
    // LocalRecordRmvEventsForQuestion (which decrements LOAddressAnswers)
    if (q->SuppressQuery)
    {
        q->SuppressQuery = mDNSfalse;
        if (!CacheRecordRmvEventsForQuestion(m, q))
        {
            LogInfo("SuppressStatusChanged: Question deleted while delivering RMV events from cache");
            return;
        }
        q->SuppressQuery = mDNStrue;
    }

    // SuppressUnusable does not affect questions that are answered from the local records (/etc/hosts)
    // and SuppressQuery status does not mean anything for these questions. As we are going to stop the
    // question below, we need to deliver the RMV events so that the ADDs that will be delivered during
    // the restart will not be a duplicate ADD
    if (!LocalRecordRmvEventsForQuestion(m, q))
    {
        LogInfo("SuppressStatusChanged: Question deleted while delivering RMV events from Local AuthRecords");
        return;
    }

    // There are two cases here.
    //
    // 1. Previously it was suppressed and now it is not suppressed, restart the question so
    // that it will start as a new question. Note that we can't just call ActivateUnicastQuery
    // because when we get the response, if we had entries in the cache already, it will not answer
    // this question if the cache entry did not change. Hence, we need to restart
    // the query so that it can be answered from the cache.
    //
    // 2. Previously it was not suppressed and now it is suppressed. We need to restart the questions
    // so that we redo the duplicate checks in mDNS_StartQuery_internal. A SuppressUnusable question
    // is a duplicate of non-SuppressUnusable question if it is not suppressed (SuppressQuery is false).
    // A SuppressUnusable question is not a duplicate of non-SuppressUnusable question if it is suppressed
    // (SuppressQuery is true). The reason for this is that when a question is suppressed, we want an
    // immediate response and not want to be blocked behind a question that is querying DNS servers. When
    // the question is not suppressed, we don't want two active questions sending packets on the wire.
    // This affects both efficiency and also the current design where there is only one active question
    // pointed to from a cache entry.
    //
    // We restart queries in a two step process by first calling stop and build a temporary list which we
    // will restart at the end. The main reason for the two step process is to handle duplicate questions.
    // If there are duplicate questions, calling stop inherits the values from another question on the list (which
    // will soon become the real question) including q->ThisQInterval which might be zero if it was
    // suppressed before. At the end when we have restarted all questions, none of them is active as each
    // inherits from one another and we need to reactivate one of the questions here which is a little hacky.
    //
    // It is much cleaner and less error prone to build a list of questions and restart at the end.

    LogInfo("SuppressStatusChanged: Stop question %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
    mDNS_StopQuery_internal(m, q);
    q->next = *restart;
    *restart = q;
}

// The caller should hold the lock
mDNSexport void CheckSuppressUnusableQuestions(mDNS *const m)
{
    DNSQuestion *q;
    DNSQuestion *restart = mDNSNULL;

    // We look through all questions including new questions. During network change events,
    // we potentially restart questions here in this function that ends up as new questions,
    // which may be suppressed at this instance. Before it is handled we get another network
    // event that changes the status e.g., address becomes available. If we did not process
    // new questions, we would never change its SuppressQuery status.
    //
    // CurrentQuestion is used by RmvEventsForQuestion below. While delivering RMV events, the
    // application callback can potentially stop the current question (detected by CurrentQuestion) or
    // *any* other question which could be the next one that we may process here. RestartQuestion
    // points to the "next" question which will be automatically advanced in mDNS_StopQuery_internal
    // if the "next" question is stopped while the CurrentQuestion is stopped
    if (m->RestartQuestion)
        LogMsg("CheckSuppressUnusableQuestions: ERROR!! m->RestartQuestion already set: %##s (%s)",
               m->RestartQuestion->qname.c, DNSTypeName(m->RestartQuestion->qtype));
    m->RestartQuestion = m->Questions;
    while (m->RestartQuestion)
    {
        q = m->RestartQuestion;
        m->RestartQuestion = q->next;
        if (q->SuppressUnusable)
        {
            mDNSBool old = q->SuppressQuery;
            q->SuppressQuery = ShouldSuppressQuery(m, q);
            if (q->SuppressQuery != old)
            {
                // Previously it was not suppressed, Generate RMV events for the ADDs that we might have delivered before
                // followed by a negative cache response. Temporarily turn off suppression so that
                // AnswerCurrentQuestionWithResourceRecord can answer the question
                SuppressStatusChanged(m, q, &restart);
            }
        }
    }
    while (restart)
    {
        q = restart;
        restart = restart->next;
        q->next = mDNSNULL;
        LogInfo("CheckSuppressUnusableQuestions: Start question %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
        mDNS_StartQuery_internal(m, q);
    }
}

mDNSlocal void RestartUnicastQuestions(mDNS *const m)
{
    DNSQuestion *q;
    DNSQuestion *restart = mDNSNULL;

    if (m->RestartQuestion)
        LogMsg("RestartUnicastQuestions: ERROR!! m->RestartQuestion already set: %##s (%s)",
               m->RestartQuestion->qname.c, DNSTypeName(m->RestartQuestion->qtype));
    m->RestartQuestion = m->Questions;
    while (m->RestartQuestion)
    {
        q = m->RestartQuestion;
        m->RestartQuestion = q->next;
        if (q->Restart)
        {
            if (mDNSOpaque16IsZero(q->TargetQID))
                LogMsg("RestartUnicastQuestions: ERROR!! Restart set for multicast question %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
                
            q->Restart = 0;
            SuppressStatusChanged(m, q, &restart);
        }
    }
    while (restart)
    {
        q = restart;
        restart = restart->next;
        q->next = mDNSNULL;
        LogInfo("RestartUnicastQuestions: Start question %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
        mDNS_StartQuery_internal(m, q);
    }
}


// ValidateParameters() is called by mDNS_StartQuery_internal() to check the client parameters of 
// DNS Question that are already set by the client before calling mDNS_StartQuery() 
mDNSlocal mStatus ValidateParameters(mDNS *const m, DNSQuestion *const question)
{

    if (question->Target.type && !ValidQuestionTarget(question))
    {
        LogMsg("ValidateParameters: Warning! Target.type = %ld port = %u (Client forgot to initialize before calling mDNS_StartQuery? for question %##s)",
               question->Target.type, mDNSVal16(question->TargetPort), question->qname.c);
        question->Target.type = mDNSAddrType_None;
    }

    // If no question->Target specified, clear TargetPort
    if (!question->Target.type) 
        question->TargetPort = zeroIPPort;

    if (!ValidateDomainName(&question->qname))
    {
        LogMsg("ValidateParameters: Attempt to start query with invalid qname %##s (%s)", question->qname.c, DNSTypeName(question->qtype));
        return(mStatus_Invalid);
    }

    // If this question is referencing a specific interface, verify it exists 
    if (question->InterfaceID && question->InterfaceID != mDNSInterface_LocalOnly && question->InterfaceID != mDNSInterface_Unicast && question->InterfaceID != mDNSInterface_P2P)
    {
        NetworkInterfaceInfo *intf = FirstInterfaceForID(m, question->InterfaceID);
        if (!intf)
            LogMsg("ValidateParameters: Note: InterfaceID %p for question %##s (%s) not currently found in active interface list",
                    question->InterfaceID, question->qname.c, DNSTypeName(question->qtype));
    }
    
    return(mStatus_NoError);
}

// InitDNSConfig() is called by InitCommonState() to initialize the DNS configuration of the Question.    
// These are a subset of the internal uDNS fields. Must be done before ShouldSuppressQuery() & mDNS_PurgeForQuestion()
mDNSlocal void InitDNSConfig(mDNS *const m, DNSQuestion *const question)
{
    // First reset all DNS Configuration
    question->qDNSServer          = mDNSNULL;
    question->validDNSServers     = zeroOpaque64;
    question->triedAllServersOnce = 0;                                                                                                                                
    question->noServerResponse    = 0;  
    question->StopTime            = 0;

    // Need not initialize the DNS Configuration for Local Only OR P2P Questions
    if (question->InterfaceID == mDNSInterface_LocalOnly || question->InterfaceID == mDNSInterface_P2P)
        return;
    // Proceed to initialize DNS Configuration (some are set in SetValidDNSServers())
    if (!mDNSOpaque16IsZero(question->TargetQID))
    {     
        mDNSu32 timeout = SetValidDNSServers(m, question);
        // We set the timeout whenever mDNS_StartQuery_internal is called. This means if we have
        // a networking change/search domain change that calls this function again we keep
        // reinitializing the timeout value which means it may never timeout. If this becomes
        // a common case in the future, we can easily fix this by adding extra state that
        // indicates that we have already set the StopTime.
        //
        // Note that we set the timeout for all questions. If this turns out to be a duplicate, 
        // it gets a full timeout value even if the original question times out earlier.
        if (question->TimeoutQuestion)
        {
            question->StopTime = NonZeroTime(m->timenow + timeout * mDNSPlatformOneSecond);
            LogInfo("InitDNSConfig: Setting StopTime on question %p %##s (%s)", question, question->qname.c, DNSTypeName(question->qtype));
        }

        question->qDNSServer = GetServerForQuestion(m, question);
        LogInfo("InitDNSConfig: question %p %##s (%s) Timeout %d, DNS Server %#a:%d",
                 question, question->qname.c, DNSTypeName(question->qtype), timeout,
                 question->qDNSServer ? &question->qDNSServer->addr : mDNSNULL,
                 mDNSVal16(question->qDNSServer ? question->qDNSServer->port : zeroIPPort));
    }
    else
    {     
        if (question->TimeoutQuestion)
            question->StopTime = NonZeroTime(m->timenow + GetTimeoutForMcastQuestion(m, question) * mDNSPlatformOneSecond);
    }
    // Set StopTime here since it is a part of DNS Configuration 
    if (question->StopTime)
        SetNextQueryStopTime(m, question);
    // SetNextQueryTime() need not be initialized for LocalOnly OR P2P Questions since those questions
    // will never be transmitted on the wire. Hence we call SetNextQueryTime() here.
    SetNextQueryTime(m,question); 
}

// InitCommonState() is called by mDNS_StartQuery_internal() to initialize the common(uDNS/mDNS) internal
// state fields of the DNS Question. These are independent of the Client layer.
mDNSlocal mDNSBool InitCommonState(mDNS *const m, DNSQuestion *const question)
{
    mDNSBool purge;
    int i;

    // Note: In the case where we already have the answer to this question in our cache, that may be all the client
    // wanted, and they may immediately cancel their question. In this case, sending an actual query on the wire would
    // be a waste. For that reason, we schedule our first query to go out in half a second (InitialQuestionInterval).
    // If AnswerNewQuestion() finds that we have *no* relevant answers currently in our cache, then it will accelerate
    // that to go out immediately.
    question->next              = mDNSNULL;
    // ThisQInterval should be initialized before any memory allocations occur. If malloc
    // debugging is turned on within mDNSResponder (see mDNSDebug.h for details) it validates
    // the question list to check if ThisQInterval is negative which means the question has been
    // stopped and can't be on the list. The question is already on the list and ThisQInterval
    // can be negative if the caller just stopped it and starting it again. Hence, it always has to
    // be initialized. CheckForSoonToExpireRecords below prints the cache records when logging is
    // turned ON which can allocate memory e.g., base64 encoding, in the case of DNSSEC.
    question->ThisQInterval     = InitialQuestionInterval;                  // MUST be > zero for an active question
    question->qnamehash         = DomainNameHashValue(&question->qname);
    question->DelayAnswering    = CheckForSoonToExpireRecords(m, &question->qname, question->qnamehash, HashSlot(&question->qname), &purge);
    question->LastQTime         = m->timenow;
    question->ExpectUnicastResp = 0;
    question->LastAnswerPktNum  = m->PktNum;
    question->RecentAnswerPkts  = 0;
    question->CurrentAnswers    = 0;

#if APPLE_OSX_mDNSResponder

// Initial browse threshold used by Finder.
#define mDNSFinderBrowseThreshold 20

    // Set the threshold at which we move to a passive browse state,
    // not actively sending queries.
    if (question->flags & kDNSServiceFlagsThresholdOne)
        question->BrowseThreshold   = 1;
    else if (question->flags & kDNSServiceFlagsThresholdFinder)
        question->BrowseThreshold   = mDNSFinderBrowseThreshold;
    else
        question->BrowseThreshold   = 0;

#else   // APPLE_OSX_mDNSResponder
   question->BrowseThreshold   = 0;
#endif  // APPLE_OSX_mDNSResponder
    question->CachedAnswerNeedsUpdate = mDNSfalse;

    question->LargeAnswers      = 0;
    question->UniqueAnswers     = 0;
    question->LOAddressAnswers  = 0;
    question->FlappingInterface1 = mDNSNULL;
    question->FlappingInterface2 = mDNSNULL;

    question->ServiceID = mDNSPlatformGetServiceID(m, question);
    
    InitDNSConfig(m, question);

    question->AuthInfo          = GetAuthInfoForQuestion(m, question);
    question->SuppressQuery     = 0;
    if (question->SuppressUnusable)
        question->SuppressQuery = ShouldSuppressQuery(m, question);

    // If ServiceID is 0 or the policy disallows making DNS requests,
    // set DisallowPID
    question->DisallowPID       = (question->ServiceID == 0 || (mDNSPlatformAllowPID(m, question) == 0));
    if (question->DisallowPID)
    {
        LogInfo("InitCommonState: Query suppressed for %##s (%s), PID %d/ServiceID %d not allowed", question->qname.c,
            DNSTypeName(question->qtype), question->pid, question->ServiceID);
    }

    question->NextInDQList      = mDNSNULL;
    question->SendQNow          = mDNSNULL;
    question->SendOnAll         = mDNSfalse;

#if mDNS_REQUEST_UNICAST_RESPONSE
    question->RequestUnicast    = SET_QU_IN_FIRST_FOUR_QUERIES;
#else   // mDNS_REQUEST_UNICAST_RESPONSE
    question->RequestUnicast    = SET_QU_IN_FIRST_QUERY;
#endif  // mDNS_REQUEST_UNICAST_RESPONSE

#if APPLE_OSX_mDNSResponder
    // Request unicast response for first 4 queries to increase
    // reliability in an environment with high multicast packet loss.
    // Must set to one more than the number of unicast queries you want, since SendQueries()
    // decrements it before calling BuildQuestion() which acts on it.
    if (question->flags & kDNSServiceFlagsUnicastResponse)
    {
        question->RequestUnicast    = SET_QU_IN_FIRST_FOUR_QUERIES;
        LogInfo("InitCommonState: setting RequestUnicast = %d for %##s (%s)", question->RequestUnicast, question->qname.c,
            DNSTypeName(question->qtype));
    } 
    else if (question->flags & kDNSServiceFlagsThresholdFinder)
    {
        // always send one request with QU bit set when kDNSServiceFlagsThresholdFinder is set
#if mDNS_REQUEST_UNICAST_RESPONSE
        question->RequestUnicast    = SET_QU_IN_FIRST_FOUR_QUERIES;
#else   // mDNS_REQUEST_UNICAST_RESPONSE
        question->RequestUnicast    = SET_QU_IN_FIRST_QUERY;
#endif  // mDNS_REQUEST_UNICAST_RESPONSE

        LogInfo("InitCommonState: kDNSServiceFlagsThresholdFinder set, setting RequestUnicast = %d for %##s (%s)", 
            question->RequestUnicast, question->qname.c, DNSTypeName(question->qtype));
    }
#endif  // APPLE_OSX_mDNSResponder

    question->LastQTxTime       = m->timenow;
    question->CNAMEReferrals    = 0; 

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    question->WakeOnResolveCount = 0;
    if (question->WakeOnResolve)
    {     
        question->WakeOnResolveCount = InitialWakeOnResolveCount;
        purge = mDNStrue;
    }     
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    for (i=0; i<DupSuppressInfoSize; i++)
        question->DupSuppress[i].InterfaceID = mDNSNULL;

    question->Restart = 0;

    debugf("InitCommonState: Question %##s (%s) Interface %p Now %d Send in %d Answer in %d (%p) %s (%p)",
            question->qname.c, DNSTypeName(question->qtype), question->InterfaceID, m->timenow,
            NextQSendTime(question) - m->timenow,
            question->DelayAnswering ? question->DelayAnswering - m->timenow : 0,
            question, question->DuplicateOf ? "duplicate of" : "not duplicate", question->DuplicateOf);

    if (question->DelayAnswering)
        LogInfo("InitCommonState: Delaying answering for %d ticks while cache stabilizes for %##s (%s)",
                 question->DelayAnswering - m->timenow, question->qname.c, DNSTypeName(question->qtype));

    return(purge);
}
#ifndef REMOVE_NAT_FUCTIONALITY
// Excludes the DNS Config fields which are already handled by InitDNSConfig()
mDNSlocal void InitWABState(DNSQuestion *const question)
{
    // We'll create our question->LocalSocket on demand, if needed.
    // We won't need one for duplicate questions, or from questions answered immediately out of the cache.
    // We also don't need one for LLQs because (when we're using NAT) we want them all to share a single
    // NAT mapping for receiving inbound add/remove events.
    question->LocalSocket       = mDNSNULL; 
    question->unansweredQueries = 0;
    question->nta               = mDNSNULL;
    question->servAddr          = zeroAddr;
    question->servPort          = zeroIPPort;
    question->tcp               = mDNSNULL;
    question->NoAnswer          = NoAnswer_Normal;
}

mDNSlocal void InitLLQNATState(mDNS *const m)
{
    // If we don't have our NAT mapping active, start it now
    if (!m->LLQNAT.clientCallback)
    {
        m->LLQNAT.Protocol       = NATOp_MapUDP;
        m->LLQNAT.IntPort        = m->UnicastPort4;
        m->LLQNAT.RequestedPort  = m->UnicastPort4;
        m->LLQNAT.clientCallback = LLQNATCallback;
        m->LLQNAT.clientContext  = (void*)1; // Means LLQ NAT Traversal just started
        mDNS_StartNATOperation_internal(m, &m->LLQNAT);
    }
}
#endif //#ifndef REMOVE_NAT_FUCTIONALITY

mDNSlocal void InitLLQState(DNSQuestion *const question)
{
    question->state             = LLQ_InitialRequest;
    question->ReqLease          = 0;
    question->expire            = 0;
    question->ntries            = 0;
    question->id                = zeroOpaque64;
}

// InitDNSSECProxyState() is called by mDNS_StartQuery_internal() to initialize
// DNSSEC & DNS Proxy fields of the DNS Question. 
mDNSlocal void InitDNSSECProxyState(mDNS *const m, DNSQuestion *const question)
{
    (void) m;

    // DNS server selection affects DNSSEC. Turn off validation if req_DO is not set
    // or the request is going over cellular interface.
    //
    // Note: This needs to be done here before we call FindDuplicateQuestion as it looks
    // at ValidationRequired setting also.
    if (question->qDNSServer)
    {
        if (question->qDNSServer->cellIntf)
        {
            LogInfo("InitDNSSECProxyState: Turning off validation for %##s (%s); going over cell", question->qname.c, DNSTypeName(question->qtype));
            question->ValidationRequired = mDNSfalse;
        }
        if (DNSSECOptionalQuestion(question) && !(question->qDNSServer->req_DO))
        {
            LogInfo("InitDNSSECProxyState: Turning off validation for %##s (%s); req_DO false",
                question->qname.c, DNSTypeName(question->qtype));
            question->ValidationRequired = DNSSEC_VALIDATION_NONE;
        }
    }
    question->ValidationState = (question->ValidationRequired ? DNSSECValRequired : DNSSECValNotRequired);
    question->ValidationStatus = 0;
    question->responseFlags    = zeroID;
}

// Once the question is completely initialized including the duplicate logic, this function
// is called to finalize the unicast question which requires flushing the cache if needed,
// activating the query etc.
mDNSlocal void FinalizeUnicastQuestion(mDNS *const m, DNSQuestion *question, mDNSBool purge)
{
    // Ensure DNS related info of duplicate question is same as the orig question
    if (question->DuplicateOf)
    {
        question->validDNSServers = question->DuplicateOf->validDNSServers;
        question->qDNSServer = question->DuplicateOf->qDNSServer;
        LogInfo("FinalizeUnicastQuestion: Duplicate question %p (%p) %##s (%s), DNS Server %#a:%d",
                 question, question->DuplicateOf, question->qname.c, DNSTypeName(question->qtype),
                 question->qDNSServer ? &question->qDNSServer->addr : mDNSNULL,
                 mDNSVal16(question->qDNSServer ? question->qDNSServer->port : zeroIPPort));
    }

    ActivateUnicastQuery(m, question, mDNSfalse);

    // If purge was set above, flush the cache. Need to do this after we set the
    // DNS server on the question
    if (purge)
    {
        question->DelayAnswering = 0;
        mDNS_PurgeForQuestion(m, question);
    }
    else if (!question->DuplicateOf && DNSSECQuestion(question))
    {
        // For DNSSEC questions, we need to have the RRSIGs also for verification.
        CheckForDNSSECRecords(m, question);
    }
#ifndef REMOVE_NAT_FUCTIONALITY
    if (question->LongLived)
    {
        // Unlike other initializations, InitLLQNATState should be done after
        // we determine that it is a unicast question.  LongLived is set for
        // both multicast and unicast browse questions but we should initialize
        // the LLQ NAT state only for unicast. Otherwise we will unnecessarily
        // start the NAT traversal that is not needed.
        InitLLQNATState(m);
#if APPLE_OSX_mDNSResponder
        UpdateAutoTunnelDomainStatuses(m);
#endif
    }
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
}

mDNSexport mStatus mDNS_StartQuery_internal(mDNS *const m, DNSQuestion *const question)
{
    DNSQuestion **q;
    mStatus vStatus;
    mDNSBool purge;

    // First check for cache space (can't do queries if there is no cache space allocated)
    if (m->rrcache_size == 0)
        return(mStatus_NoCache);

    vStatus = ValidateParameters(m, question);
    if (vStatus)
        return(vStatus);
    
    question->TargetQID =
#ifndef UNICAST_DISABLED
                          (question->Target.type || Question_uDNS(question)) ? mDNS_NewMessageID(m) :
#endif // UNICAST_DISABLED
                          zeroID;
    debugf("mDNS_StartQuery_internal: %##s (%s)", question->qname.c, DNSTypeName(question->qtype)); 
    
    // Note: It important that new questions are appended at the *end* of the list, not prepended at the start
    q = &m->Questions;
    if (question->InterfaceID == mDNSInterface_LocalOnly || question->InterfaceID == mDNSInterface_P2P) 
        q = &m->LocalOnlyQuestions;
    while (*q && *q != question) 
        q=&(*q)->next;

    if (*q)
    {
        LogMsg("mDNS_StartQuery_internal: Error! Tried to add a question %##s (%s) %p that's already in the active list",
                question->qname.c, DNSTypeName(question->qtype), question);
        return(mStatus_AlreadyRegistered);
    }
    *q = question;
    

    // Intialize the question. The only ordering constraint we have today is that
    // InitDNSSECProxyState should be called after the DNS server is selected (in
    // InitCommonState -> InitDNSConfig) as DNS server selection affects DNSSEC
    // validation.

    purge = InitCommonState(m, question);
#ifndef REMOVE_NAT_FUCTIONALITY
    InitWABState(question);
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    InitLLQState(question);
    InitDNSSECProxyState(m, question);

    // FindDuplicateQuestion should be called last after all the intialization
    // as the duplicate logic could be potentially based on any field in the
    // question.
    question->DuplicateOf  = FindDuplicateQuestion(m, question);
    if (question->DuplicateOf) 
        question->AuthInfo = question->DuplicateOf->AuthInfo; 

    if (question->InterfaceID == mDNSInterface_LocalOnly || question->InterfaceID == mDNSInterface_P2P)
    {
        if (!m->NewLocalOnlyQuestions) 
            m->NewLocalOnlyQuestions = question;
    }
    else
    {
        if (!m->NewQuestions) 
            m->NewQuestions = question;

        // If the question's id is non-zero, then it's Wide Area
        // MUST NOT do this Wide Area setup until near the end of
        // mDNS_StartQuery_internal -- this code may itself issue queries (e.g. SOA,
        // NS, etc.) and if we haven't finished setting up our own question and setting
        // m->NewQuestions if necessary then we could end up recursively re-entering
        // this routine with the question list data structures in an inconsistent state.
        if (!mDNSOpaque16IsZero(question->TargetQID))
        {
            FinalizeUnicastQuestion(m, question, purge);
        }
        else
        {
            if (purge)
            {
                LogInfo("mDNS_StartQuery_internal: Purging for %##s", question->qname.c);
                mDNS_PurgeForQuestion(m, question);
            }
        }
    }

    return(mStatus_NoError);
}

// CancelGetZoneData is an internal routine (i.e. must be called with the lock already held)
mDNSexport void CancelGetZoneData(mDNS *const m, ZoneData *nta)
{
    debugf("CancelGetZoneData %##s (%s)", nta->question.qname.c, DNSTypeName(nta->question.qtype));
    // This function may be called anytime to free the zone information.The question may or may not have stopped.
    // If it was already stopped, mDNS_StopQuery_internal would have set q->ThisQInterval to -1 and should not
    // call it again
    if (nta->question.ThisQInterval != -1)
    {
        mDNS_StopQuery_internal(m, &nta->question);
        if (nta->question.ThisQInterval != -1)
            LogMsg("CancelGetZoneData: Question %##s (%s) ThisQInterval %d not -1", nta->question.qname.c, DNSTypeName(nta->question.qtype), nta->question.ThisQInterval);
    }
    mDNSPlatformMemFree(nta);
}

mDNSexport mStatus mDNS_StopQuery_internal(mDNS *const m, DNSQuestion *const question)
{
    const mDNSu32 slot = HashSlot(&question->qname);
    CacheGroup *cg = CacheGroupForName(m, slot, question->qnamehash, &question->qname);
    CacheRecord *rr;
    DNSQuestion **qp = &m->Questions;

    //LogInfo("mDNS_StopQuery_internal %##s (%s)", question->qname.c, DNSTypeName(question->qtype));

    if (question->InterfaceID == mDNSInterface_LocalOnly || question->InterfaceID == mDNSInterface_P2P) qp = &m->LocalOnlyQuestions;
    while (*qp && *qp != question) qp=&(*qp)->next;
    if (*qp) *qp = (*qp)->next;
    else
    {
#if !ForceAlerts
        if (question->ThisQInterval >= 0)   // Only log error message if the query was supposed to be active
#endif
        LogMsg("mDNS_StopQuery_internal: Question %##s (%s) not found in active list",
               question->qname.c, DNSTypeName(question->qtype));
#if ForceAlerts
        *(long*)0 = 0;
#endif
        return(mStatus_BadReferenceErr);
    }

    // Take care to cut question from list *before* calling UpdateQuestionDuplicates
    UpdateQuestionDuplicates(m, question);
    // But don't trash ThisQInterval until afterwards.
    question->ThisQInterval = -1;

    // If there are any cache records referencing this as their active question, then see if there is any
    // other question that is also referencing them, else their CRActiveQuestion needs to get set to NULL.
    for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
    {
        if (rr->CRActiveQuestion == question)
        {
            DNSQuestion *q;
            // Checking for ActiveQuestion filters questions that are suppressed also
            // as suppressed questions are not active
            for (q = m->Questions; q; q=q->next)        // Scan our list of questions
                if (ActiveQuestion(q) && ResourceRecordAnswersQuestion(&rr->resrec, q))
                    break;
            if (q)
                debugf("mDNS_StopQuery_internal: Updating CRActiveQuestion to %p for cache record %s, Original question CurrentAnswers %d, new question "
                       "CurrentAnswers %d, SuppressQuery %d", q, CRDisplayString(m,rr), question->CurrentAnswers, q->CurrentAnswers, q->SuppressQuery);
            rr->CRActiveQuestion = q;       // Question used to be active; new value may or may not be null
            if (!q) m->rrcache_active--;    // If no longer active, decrement rrcache_active count
        }
    }

    // If we just deleted the question that CacheRecordAdd() or CacheRecordRmv() is about to look at,
    // bump its pointer forward one question.
    if (m->CurrentQuestion == question)
    {
        debugf("mDNS_StopQuery_internal: Just deleted the currently active question: %##s (%s)",
               question->qname.c, DNSTypeName(question->qtype));
        m->CurrentQuestion = question->next;
    }

    if (m->NewQuestions == question)
    {
        debugf("mDNS_StopQuery_internal: Just deleted a new question that wasn't even answered yet: %##s (%s)",
               question->qname.c, DNSTypeName(question->qtype));
        m->NewQuestions = question->next;
    }

    if (m->NewLocalOnlyQuestions == question) m->NewLocalOnlyQuestions = question->next;

    if (m->RestartQuestion == question)
    {
        LogMsg("mDNS_StopQuery_internal: Just deleted the current restart question: %##s (%s)",
               question->qname.c, DNSTypeName(question->qtype));
        m->RestartQuestion = question->next;
    }

    if (m->ValidationQuestion == question)
    {
        LogInfo("mDNS_StopQuery_internal: Just deleted the current Validation question: %##s (%s)",
               question->qname.c, DNSTypeName(question->qtype));
        m->ValidationQuestion = question->next;
    }

    // Take care not to trash question->next until *after* we've updated m->CurrentQuestion and m->NewQuestions
    question->next = mDNSNULL;

    // LogMsg("mDNS_StopQuery_internal: Question %##s (%s) removed", question->qname.c, DNSTypeName(question->qtype));

    // And finally, cancel any associated GetZoneData operation that's still running.
    // Must not do this until last, because there's a good chance the GetZoneData question is the next in the list,
    // so if we delete it earlier in this routine, we could find that our "question->next" pointer above is already
    // invalid before we even use it. By making sure that we update m->CurrentQuestion and m->NewQuestions if necessary
    // *first*, then they're all ready to be updated a second time if necessary when we cancel our GetZoneData query.
    if (question->tcp) { DisposeTCPConn(question->tcp); question->tcp = mDNSNULL; }
    if (question->LocalSocket) { mDNSPlatformUDPClose(question->LocalSocket); question->LocalSocket = mDNSNULL; }
#ifndef REMOVE_NAT_FUCTIONALITY
    if (!mDNSOpaque16IsZero(question->TargetQID) && question->LongLived)
    {
        // Scan our list to see if any more wide-area LLQs remain. If not, stop our NAT Traversal.
        DNSQuestion *q;
        for (q = m->Questions; q; q=q->next)
            if (!mDNSOpaque16IsZero(q->TargetQID) && q->LongLived) break;
        if (!q)
        {
            if (!m->LLQNAT.clientCallback)       // Should never happen, but just in case...
            {
                LogMsg("mDNS_StopQuery ERROR LLQNAT.clientCallback NULL");
            }
            else
            {
                LogInfo("Stopping LLQNAT");
                mDNS_StopNATOperation_internal(m, &m->LLQNAT);
                m->LLQNAT.clientCallback = mDNSNULL; // Means LLQ NAT Traversal not running
            }
        }

        // If necessary, tell server it can delete this LLQ state
        if (question->state == LLQ_Established)
        {
            question->ReqLease = 0;
            sendLLQRefresh(m, question);
            // If we need need to make a TCP connection to cancel the LLQ, that's going to take a little while.
            // We clear the tcp->question backpointer so that when the TCP connection completes, it doesn't
            // crash trying to access our cancelled question, but we don't cancel the TCP operation itself --
            // we let that run out its natural course and complete asynchronously.
            if (question->tcp)
            {
                question->tcp->question = mDNSNULL;
                question->tcp           = mDNSNULL;
            }
        }
#if APPLE_OSX_mDNSResponder
        UpdateAutoTunnelDomainStatuses(m);
#endif
    }
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    // wait until we send the refresh above which needs the nta
    if (question->nta) { CancelGetZoneData(m, question->nta); question->nta = mDNSNULL; }

    if (question->ValidationRequired && question->DNSSECAuthInfo)
    {
        LogInfo("mDNS_StopQuery_internal: freeing DNSSECAuthInfo %##s", question->qname.c);
        question->DAIFreeCallback(m, question->DNSSECAuthInfo);
        question->DNSSECAuthInfo = mDNSNULL;
    }
    if (question->AnonInfo)
    {
        FreeAnonInfo(question->AnonInfo);
        question->AnonInfo = mDNSNULL;
    }

    return(mStatus_NoError);
}

mDNSexport mStatus mDNS_StartQuery(mDNS *const m, DNSQuestion *const question)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_StartQuery_internal(m, question);
    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_StopQuery(mDNS *const m, DNSQuestion *const question)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_StopQuery_internal(m, question);
    mDNS_Unlock(m);
    return(status);
}

// Note that mDNS_StopQueryWithRemoves() does not currently implement the full generality of the other APIs
// Specifically, question callbacks invoked as a result of this call cannot themselves make API calls.
// We invoke the callback without using mDNS_DropLockBeforeCallback/mDNS_ReclaimLockAfterCallback
// specifically to catch and report if the client callback does try to make API calls
mDNSexport mStatus mDNS_StopQueryWithRemoves(mDNS *const m, DNSQuestion *const question)
{
    mStatus status;
    DNSQuestion *qq;
    mDNS_Lock(m);

    // Check if question is new -- don't want to give remove events for a question we haven't even answered yet
    for (qq = m->NewQuestions; qq; qq=qq->next) if (qq == question) break;

    status = mDNS_StopQuery_internal(m, question);
    if (status == mStatus_NoError && !qq)
    {
        const CacheRecord *rr;
        const mDNSu32 slot = HashSlot(&question->qname);
        CacheGroup *const cg = CacheGroupForName(m, slot, question->qnamehash, &question->qname);
        LogInfo("Generating terminal removes for %##s (%s)", question->qname.c, DNSTypeName(question->qtype));
        for (rr = cg ? cg->members : mDNSNULL; rr; rr=rr->next)
            if (rr->resrec.RecordType != kDNSRecordTypePacketNegative && SameNameRecordAnswersQuestion(&rr->resrec, question))
            {
                // Don't use mDNS_DropLockBeforeCallback() here, since we don't allow API calls
                if (question->QuestionCallback)
                    question->QuestionCallback(m, question, &rr->resrec, mDNSfalse);
            }
    }
    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_Reconfirm(mDNS *const m, CacheRecord *const cr)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_Reconfirm_internal(m, cr, kDefaultReconfirmTimeForNoAnswer);
    if (status == mStatus_NoError) ReconfirmAntecedents(m, cr->resrec.name, cr->resrec.namehash, 0);
    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_ReconfirmByValue(mDNS *const m, ResourceRecord *const rr)
{
    mStatus status = mStatus_BadReferenceErr;
    CacheRecord *cr;
    mDNS_Lock(m);
    cr = FindIdenticalRecordInCache(m, rr);
    debugf("mDNS_ReconfirmByValue: %p %s", cr, RRDisplayString(m, rr));
    if (cr) status = mDNS_Reconfirm_internal(m, cr, kDefaultReconfirmTimeForNoAnswer);
    if (status == mStatus_NoError) ReconfirmAntecedents(m, cr->resrec.name, cr->resrec.namehash, 0);
    mDNS_Unlock(m);
    return(status);
}

mDNSlocal mStatus mDNS_StartBrowse_internal(mDNS *const m, DNSQuestion *const question,
                                            const domainname *const srv, const domainname *const domain,
                                            const mDNSu8 *anondata, const mDNSInterfaceID InterfaceID, mDNSu32 flags,
                                            mDNSBool ForceMCast, mDNSBool useBackgroundTrafficClass,
                                            mDNSQuestionCallback *Callback, void *Context)
{
    question->InterfaceID      = InterfaceID;
    question->flags            = flags;
    question->Target           = zeroAddr;
    question->qtype            = kDNSType_PTR;
    question->qclass           = kDNSClass_IN;
    question->LongLived        = mDNStrue;
    question->ExpectUnique     = mDNSfalse;
    question->ForceMCast       = ForceMCast;
    question->ReturnIntermed   = mDNSfalse;
    question->SuppressUnusable = mDNSfalse;
    question->SearchListIndex  = 0;
    question->AppendSearchDomains = 0;
    question->RetryWithSearchDomains = mDNSfalse;
    question->TimeoutQuestion  = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    question->WakeOnResolve    = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    question->UseBackgroundTrafficClass = useBackgroundTrafficClass;
    question->ValidationRequired = 0;
    question->ValidatingResponse = 0;
    question->ProxyQuestion    = 0;
    question->qnameOrig        = mDNSNULL;
    question->AnonInfo         = mDNSNULL;
    question->QuestionCallback = Callback;
    question->QuestionContext  = Context;

    if (!ConstructServiceName(&question->qname, mDNSNULL, srv, domain))
        return(mStatus_BadParamErr);

    if (anondata)
    {
        question->AnonInfo = AllocateAnonInfo(&question->qname, anondata, mDNSPlatformStrLen(anondata), mDNSNULL);
        if (!question->AnonInfo)
            return(mStatus_BadParamErr);
    }

    return(mDNS_StartQuery_internal(m, question));
}

mDNSexport mStatus mDNS_StartBrowse(mDNS *const m, DNSQuestion *const question,
                                    const domainname *const srv, const domainname *const domain,
                                    const mDNSu8 *anondata, const mDNSInterfaceID InterfaceID, mDNSu32 flags,
                                    mDNSBool ForceMCast, mDNSBool useBackgroundTrafficClass,
                                    mDNSQuestionCallback *Callback, void *Context)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_StartBrowse_internal(m, question, srv, domain, anondata, InterfaceID, flags, ForceMCast, useBackgroundTrafficClass, Callback, Context);
    mDNS_Unlock(m);
    return(status);
}

mDNSlocal mDNSBool MachineHasActiveIPv6(mDNS *const m)
{
    NetworkInterfaceInfo *intf;
    for (intf = m->HostInterfaces; intf; intf = intf->next)
        if (intf->ip.type == mDNSAddrType_IPv6) return(mDNStrue);
    return(mDNSfalse);
}

mDNSlocal void FoundServiceInfoSRV(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    ServiceInfoQuery *query = (ServiceInfoQuery *)question->QuestionContext;
    mDNSBool PortChanged = !mDNSSameIPPort(query->info->port, answer->rdata->u.srv.port);
    if (!AddRecord) return;
    if (answer->rrtype != kDNSType_SRV) return;

    query->info->port = answer->rdata->u.srv.port;

    // If this is our first answer, then set the GotSRV flag and start the address query
    if (!query->GotSRV)
    {
        query->GotSRV             = mDNStrue;
        query->qAv4.InterfaceID   = answer->InterfaceID;
        AssignDomainName(&query->qAv4.qname, &answer->rdata->u.srv.target);
        query->qAv6.InterfaceID   = answer->InterfaceID;
        AssignDomainName(&query->qAv6.qname, &answer->rdata->u.srv.target);
        mDNS_StartQuery(m, &query->qAv4);
        // Only do the AAAA query if this machine actually has IPv6 active
        if (MachineHasActiveIPv6(m)) mDNS_StartQuery(m, &query->qAv6);
    }
    // If this is not our first answer, only re-issue the address query if the target host name has changed
    else if ((query->qAv4.InterfaceID != query->qSRV.InterfaceID && query->qAv4.InterfaceID != answer->InterfaceID) ||
             !SameDomainName(&query->qAv4.qname, &answer->rdata->u.srv.target))
    {
        mDNS_StopQuery(m, &query->qAv4);
        if (query->qAv6.ThisQInterval >= 0) mDNS_StopQuery(m, &query->qAv6);
        if (SameDomainName(&query->qAv4.qname, &answer->rdata->u.srv.target) && !PortChanged)
        {
            // If we get here, it means:
            // 1. This is not our first SRV answer
            // 2. The interface ID is different, but the target host and port are the same
            // This implies that we're seeing the exact same SRV record on more than one interface, so we should
            // make our address queries at least as broad as the original SRV query so that we catch all the answers.
            query->qAv4.InterfaceID = query->qSRV.InterfaceID;  // Will be mDNSInterface_Any, or a specific interface
            query->qAv6.InterfaceID = query->qSRV.InterfaceID;
        }
        else
        {
            query->qAv4.InterfaceID   = answer->InterfaceID;
            AssignDomainName(&query->qAv4.qname, &answer->rdata->u.srv.target);
            query->qAv6.InterfaceID   = answer->InterfaceID;
            AssignDomainName(&query->qAv6.qname, &answer->rdata->u.srv.target);
        }
        debugf("FoundServiceInfoSRV: Restarting address queries for %##s (%s)", query->qAv4.qname.c, DNSTypeName(query->qAv4.qtype));
        mDNS_StartQuery(m, &query->qAv4);
        // Only do the AAAA query if this machine actually has IPv6 active
        if (MachineHasActiveIPv6(m)) mDNS_StartQuery(m, &query->qAv6);
    }
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    else if (query->ServiceInfoQueryCallback && query->GotADD && query->GotTXT && PortChanged)
#else
    else if (query->ServiceInfoQueryCallback && query->GotADD && PortChanged)
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    {
        if (++query->Answers >= 100)
            debugf("**** WARNING **** Have given %lu answers for %##s (SRV) %##s %u",
                   query->Answers, query->qSRV.qname.c, answer->rdata->u.srv.target.c,
                   mDNSVal16(answer->rdata->u.srv.port));
        query->ServiceInfoQueryCallback(m, query);
    }
    // CAUTION: MUST NOT do anything more with query after calling query->Callback(), because the client's
    // callback function is allowed to do anything, including deleting this query and freeing its memory.
}

mDNSlocal void FoundServiceInfoTXT(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    ServiceInfoQuery *query = (ServiceInfoQuery *)question->QuestionContext;
    if (!AddRecord) return;
    if (answer->rrtype != kDNSType_TXT) return;
    if (answer->rdlength > sizeof(query->info->TXTinfo)) return;

#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    query->GotTXT       = mDNStrue;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    query->info->TXTlen = answer->rdlength;
    query->info->TXTinfo[0] = 0;        // In case answer->rdlength is zero
    mDNSPlatformMemCopy(query->info->TXTinfo, answer->rdata->u.txt.c, answer->rdlength);

    verbosedebugf("FoundServiceInfoTXT: %##s GotADD=%d", query->info->name.c, query->GotADD);

    // CAUTION: MUST NOT do anything more with query after calling query->Callback(), because the client's
    // callback function is allowed to do anything, including deleting this query and freeing its memory.
    if (query->ServiceInfoQueryCallback && query->GotADD)
    {
        if (++query->Answers >= 100)
            debugf("**** WARNING **** have given %lu answers for %##s (TXT) %#s...",
                   query->Answers, query->qSRV.qname.c, answer->rdata->u.txt.c);
        query->ServiceInfoQueryCallback(m, query);
    }
}

mDNSlocal void FoundServiceInfo(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    ServiceInfoQuery *query = (ServiceInfoQuery *)question->QuestionContext;
    //LogInfo("FoundServiceInfo %d %s", AddRecord, RRDisplayString(m, answer));
    if (!AddRecord) return;

    if (answer->rrtype == kDNSType_A)
    {
        query->info->ip.type = mDNSAddrType_IPv4;
        query->info->ip.ip.v4 = answer->rdata->u.ipv4;
    }
    else if (answer->rrtype == kDNSType_AAAA)
    {
        query->info->ip.type = mDNSAddrType_IPv6;
        query->info->ip.ip.v6 = answer->rdata->u.ipv6;
    }
    else
    {
        debugf("FoundServiceInfo: answer %##s type %d (%s) unexpected", answer->name->c, answer->rrtype, DNSTypeName(answer->rrtype));
        return;
    }

    query->GotADD = mDNStrue;
    query->info->InterfaceID = answer->InterfaceID;

#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    verbosedebugf("FoundServiceInfo v%ld: %##s GotTXT=%d", query->info->ip.type, query->info->name.c, query->GotTXT);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // CAUTION: MUST NOT do anything more with query after calling query->Callback(), because the client's
    // callback function is allowed to do anything, including deleting this query and freeing its memory.
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (query->ServiceInfoQueryCallback && query->GotTXT)
#else
    if (query->ServiceInfoQueryCallback)
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    {
        if (++query->Answers >= 100)
            debugf(answer->rrtype == kDNSType_A ?
                   "**** WARNING **** have given %lu answers for %##s (A) %.4a" :
                   "**** WARNING **** have given %lu answers for %##s (AAAA) %.16a",
                   query->Answers, query->qSRV.qname.c, &answer->rdata->u.data);
        query->ServiceInfoQueryCallback(m, query);
    }
}

// On entry, the client must have set the name and InterfaceID fields of the ServiceInfo structure
// If the query is not interface-specific, then InterfaceID may be zero
// Each time the Callback is invoked, the remainder of the fields will have been filled in
// In addition, InterfaceID will be updated to give the interface identifier corresponding to that response
mDNSexport mStatus mDNS_StartResolveService(mDNS *const m,
                                            ServiceInfoQuery *query, ServiceInfo *info, mDNSServiceInfoQueryCallback *Callback, void *Context)
{
    mStatus status;
    mDNS_Lock(m);

    query->qSRV.ThisQInterval       = -1;       // So that mDNS_StopResolveService() knows whether to cancel this question
    query->qSRV.InterfaceID         = info->InterfaceID;
    query->qSRV.flags               = 0;
    query->qSRV.Target              = zeroAddr;
    AssignDomainName(&query->qSRV.qname, &info->name);
    query->qSRV.qtype               = kDNSType_SRV;
    query->qSRV.qclass              = kDNSClass_IN;
    query->qSRV.LongLived           = mDNSfalse;
    query->qSRV.ExpectUnique        = mDNStrue;
    query->qSRV.ForceMCast          = mDNSfalse;
    query->qSRV.ReturnIntermed      = mDNSfalse;
    query->qSRV.SuppressUnusable    = mDNSfalse;
    query->qSRV.SearchListIndex     = 0;
    query->qSRV.AppendSearchDomains = 0;
    query->qSRV.RetryWithSearchDomains = mDNSfalse;
    query->qSRV.TimeoutQuestion     = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    query->qSRV.WakeOnResolve       = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    query->qSRV.UseBackgroundTrafficClass = mDNSfalse;
    query->qSRV.ValidationRequired  = 0;
    query->qSRV.ValidatingResponse  = 0;
    query->qSRV.ProxyQuestion       = 0;
    query->qSRV.qnameOrig           = mDNSNULL;
    query->qSRV.AnonInfo            = mDNSNULL;
    query->qSRV.QuestionCallback    = FoundServiceInfoSRV;
    query->qSRV.QuestionContext     = query;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    query->qTXT.ThisQInterval       = -1;       // So that mDNS_StopResolveService() knows whether to cancel this question
    query->qTXT.InterfaceID         = info->InterfaceID;
    query->qTXT.flags               = 0;
    query->qTXT.Target              = zeroAddr;
    AssignDomainName(&query->qTXT.qname, &info->name);
    query->qTXT.qtype               = kDNSType_TXT;
    query->qTXT.qclass              = kDNSClass_IN;
    query->qTXT.LongLived           = mDNSfalse;
    query->qTXT.ExpectUnique        = mDNStrue;
    query->qTXT.ForceMCast          = mDNSfalse;
    query->qTXT.ReturnIntermed      = mDNSfalse;
    query->qTXT.SuppressUnusable    = mDNSfalse;
    query->qTXT.SearchListIndex     = 0;
    query->qTXT.AppendSearchDomains = 0;
    query->qTXT.RetryWithSearchDomains = mDNSfalse;
    query->qTXT.TimeoutQuestion     = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    query->qTXT.WakeOnResolve       = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    query->qTXT.UseBackgroundTrafficClass = mDNSfalse;
    query->qTXT.ValidationRequired  = 0;
    query->qTXT.ValidatingResponse  = 0;
    query->qTXT.ProxyQuestion       = 0;
    query->qTXT.qnameOrig           = mDNSNULL;
    query->qTXT.AnonInfo            = mDNSNULL;
    query->qTXT.QuestionCallback    = FoundServiceInfoTXT;
    query->qTXT.QuestionContext     = query;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    query->qAv4.ThisQInterval       = -1;       // So that mDNS_StopResolveService() knows whether to cancel this question
    query->qAv4.InterfaceID         = info->InterfaceID;
    query->qAv4.flags               = 0;
    query->qAv4.Target              = zeroAddr;
    query->qAv4.qname.c[0]          = 0;
    query->qAv4.qtype               = kDNSType_A;
    query->qAv4.qclass              = kDNSClass_IN;
    query->qAv4.LongLived           = mDNSfalse;
    query->qAv4.ExpectUnique        = mDNStrue;
    query->qAv4.ForceMCast          = mDNSfalse;
    query->qAv4.ReturnIntermed      = mDNSfalse;
    query->qAv4.SuppressUnusable    = mDNSfalse;
    query->qAv4.SearchListIndex     = 0;
    query->qAv4.AppendSearchDomains = 0;
    query->qAv4.RetryWithSearchDomains = mDNSfalse;
    query->qAv4.TimeoutQuestion     = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    query->qAv4.WakeOnResolve       = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    query->qAv4.UseBackgroundTrafficClass = mDNSfalse;
    query->qAv4.ValidationRequired  = 0;
    query->qAv4.ValidatingResponse  = 0;
    query->qAv4.ProxyQuestion       = 0;
    query->qAv4.qnameOrig           = mDNSNULL;
    query->qAv4.AnonInfo            = mDNSNULL;
    query->qAv4.QuestionCallback    = FoundServiceInfo;
    query->qAv4.QuestionContext     = query;

    query->qAv6.ThisQInterval       = -1;       // So that mDNS_StopResolveService() knows whether to cancel this question
    query->qAv6.InterfaceID         = info->InterfaceID;
    query->qAv6.flags               = 0;
    query->qAv6.Target              = zeroAddr;
    query->qAv6.qname.c[0]          = 0;
    query->qAv6.qtype               = kDNSType_AAAA;
    query->qAv6.qclass              = kDNSClass_IN;
    query->qAv6.LongLived           = mDNSfalse;
    query->qAv6.ExpectUnique        = mDNStrue;
    query->qAv6.ForceMCast          = mDNSfalse;
    query->qAv6.ReturnIntermed      = mDNSfalse;
    query->qAv6.SuppressUnusable    = mDNSfalse;
    query->qAv6.SearchListIndex     = 0;
    query->qAv6.AppendSearchDomains = 0;
    query->qAv6.RetryWithSearchDomains = mDNSfalse;
    query->qAv6.TimeoutQuestion     = 0;
    query->qAv6.UseBackgroundTrafficClass = mDNSfalse;
    query->qAv6.ValidationRequired  = 0;
    query->qAv6.ValidatingResponse  = 0;
    query->qAv6.ProxyQuestion       = 0;
    query->qAv6.qnameOrig           = mDNSNULL;
    query->qAv6.AnonInfo            = mDNSNULL;
    query->qAv6.QuestionCallback    = FoundServiceInfo;
    query->qAv6.QuestionContext     = query;

    query->GotSRV                   = mDNSfalse;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    query->GotTXT                   = mDNSfalse;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    query->GotADD                   = mDNSfalse;
    query->Answers                  = 0;

    query->info                     = info;
    query->ServiceInfoQueryCallback = Callback;
    query->ServiceInfoQueryContext  = Context;

//	info->name      = Must already be set up by client
//	info->interface = Must already be set up by client
    info->ip        = zeroAddr;
    info->port      = zeroIPPort;
    info->TXTlen    = 0;

    // We use mDNS_StartQuery_internal here because we're already holding the lock
    status = mDNS_StartQuery_internal(m, &query->qSRV);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (status == mStatus_NoError) status = mDNS_StartQuery_internal(m, &query->qTXT);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (status != mStatus_NoError) mDNS_StopResolveService(m, query);

    mDNS_Unlock(m);
    return(status);
}

mDNSexport void    mDNS_StopResolveService (mDNS *const m, ServiceInfoQuery *q)
{
    mDNS_Lock(m);
    // We use mDNS_StopQuery_internal here because we're already holding the lock
    if (q->qSRV.ThisQInterval >= 0) mDNS_StopQuery_internal(m, &q->qSRV);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (q->qTXT.ThisQInterval >= 0) mDNS_StopQuery_internal(m, &q->qTXT);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (q->qAv4.ThisQInterval >= 0) mDNS_StopQuery_internal(m, &q->qAv4);
    if (q->qAv6.ThisQInterval >= 0) mDNS_StopQuery_internal(m, &q->qAv6);
    mDNS_Unlock(m);
}

mDNSexport mStatus mDNS_GetDomains(mDNS *const m, DNSQuestion *const question, mDNS_DomainType DomainType, const domainname *dom,
                                   const mDNSInterfaceID InterfaceID, mDNSQuestionCallback *Callback, void *Context)
{
    question->InterfaceID      = InterfaceID;
    question->flags            = 0;
    question->Target           = zeroAddr;
    question->qtype            = kDNSType_PTR;
    question->qclass           = kDNSClass_IN;
    question->LongLived        = mDNSfalse;
    question->ExpectUnique     = mDNSfalse;
    question->ForceMCast       = mDNSfalse;
    question->ReturnIntermed   = mDNSfalse;
    question->SuppressUnusable = mDNSfalse;
    question->SearchListIndex  = 0;
    question->AppendSearchDomains = 0;
    question->RetryWithSearchDomains = mDNSfalse;
    question->TimeoutQuestion  = 0;
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    question->WakeOnResolve    = 0;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    question->UseBackgroundTrafficClass = mDNSfalse;
    question->ValidationRequired = 0;
    question->ValidatingResponse = 0;
    question->ProxyQuestion    = 0;
    question->qnameOrig        = mDNSNULL;
    question->AnonInfo         = mDNSNULL;
    question->pid              = mDNSPlatformGetPID();
    question->QuestionCallback = Callback;
    question->QuestionContext  = Context;
    if (DomainType > mDNS_DomainTypeMax) return(mStatus_BadParamErr);
    if (!MakeDomainNameFromDNSNameString(&question->qname, mDNS_DomainTypeNames[DomainType])) return(mStatus_BadParamErr);
    if (!dom) dom = &localdomain;
    if (!AppendDomainName(&question->qname, dom)) return(mStatus_BadParamErr);
    return(mDNS_StartQuery(m, question));
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Responder Functions
#endif

mDNSexport mStatus mDNS_Register(mDNS *const m, AuthRecord *const rr)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_Register_internal(m, rr);
    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_Update(mDNS *const m, AuthRecord *const rr, mDNSu32 newttl,
                               const mDNSu16 newrdlength, RData *const newrdata, mDNSRecordUpdateCallback *Callback)
{
    if (!ValidateRData(rr->resrec.rrtype, newrdlength, newrdata))
    {
        LogMsg("Attempt to update record with invalid rdata: %s", GetRRDisplayString_rdb(&rr->resrec, &newrdata->u, m->MsgBuffer));
        return(mStatus_Invalid);
    }

    mDNS_Lock(m);

    // If TTL is unspecified, leave TTL unchanged
    if (newttl == 0) newttl = rr->resrec.rroriginalttl;

    // If we already have an update queued up which has not gone through yet, give the client a chance to free that memory
    if (rr->NewRData)
    {
        RData *n = rr->NewRData;
        rr->NewRData = mDNSNULL;                            // Clear the NewRData pointer ...
        if (rr->UpdateCallback)
            rr->UpdateCallback(m, rr, n, rr->newrdlength);  // ...and let the client free this memory, if necessary
    }

    rr->NewRData             = newrdata;
    rr->newrdlength          = newrdlength;
    rr->UpdateCallback       = Callback;

#ifndef UNICAST_DISABLED
    if (rr->ARType != AuthRecordLocalOnly && rr->ARType != AuthRecordP2P && !IsLocalDomain(rr->resrec.name))
    {
        mStatus status = uDNS_UpdateRecord(m, rr);
        // The caller frees the memory on error, don't retain stale pointers
        if (status != mStatus_NoError) { rr->NewRData = mDNSNULL; rr->newrdlength = 0; }
        mDNS_Unlock(m);
        return(status);
    }
#endif

    if (RRLocalOnly(rr) || (rr->resrec.rroriginalttl == newttl &&
                            rr->resrec.rdlength == newrdlength && mDNSPlatformMemSame(rr->resrec.rdata->u.data, newrdata->u.data, newrdlength)))
        CompleteRDataUpdate(m, rr);
    else
    {
        rr->AnnounceCount = InitialAnnounceCount;
        InitializeLastAPTime(m, rr);
        while (rr->NextUpdateCredit && m->timenow - rr->NextUpdateCredit >= 0) GrantUpdateCredit(rr);
        if (!rr->UpdateBlocked && rr->UpdateCredits) rr->UpdateCredits--;
        if (!rr->NextUpdateCredit) rr->NextUpdateCredit = NonZeroTime(m->timenow + kUpdateCreditRefreshInterval);
        if (rr->AnnounceCount > rr->UpdateCredits + 1) rr->AnnounceCount = (mDNSu8)(rr->UpdateCredits + 1);
        if (rr->UpdateCredits <= 5)
        {
            mDNSu32 delay = 6 - rr->UpdateCredits;      // Delay 1 second, then 2, then 3, etc. up to 6 seconds maximum
            if (!rr->UpdateBlocked) rr->UpdateBlocked = NonZeroTime(m->timenow + (mDNSs32)delay * mDNSPlatformOneSecond);
            rr->ThisAPInterval *= 4;
            rr->LastAPTime = rr->UpdateBlocked - rr->ThisAPInterval;
            LogMsg("Excessive update rate for %##s; delaying announcement by %ld second%s",
                   rr->resrec.name->c, delay, delay > 1 ? "s" : "");
        }
        rr->resrec.rroriginalttl = newttl;
    }

    mDNS_Unlock(m);
    return(mStatus_NoError);
}

// Note: mDNS_Deregister calls mDNS_Deregister_internal which can call a user callback, which may change
// the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSexport mStatus mDNS_Deregister(mDNS *const m, AuthRecord *const rr)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_Deregister_internal(m, rr, mDNS_Dereg_normal);
    mDNS_Unlock(m);
    return(status);
}

// Circular reference: AdvertiseInterface references mDNS_HostNameCallback, which calls mDNS_SetFQDN, which call AdvertiseInterface
mDNSlocal void mDNS_HostNameCallback(mDNS *const m, AuthRecord *const rr, mStatus result);

mDNSlocal NetworkInterfaceInfo *FindFirstAdvertisedInterface(mDNS *const m)
{
    NetworkInterfaceInfo *intf;
    for (intf = m->HostInterfaces; intf; intf = intf->next)
        if (intf->Advertise) break;
    return(intf);
}

mDNSlocal void AdvertiseInterface(mDNS *const m, NetworkInterfaceInfo *set)
{
    char buffer[MAX_REVERSE_MAPPING_NAME];
    NetworkInterfaceInfo *primary;

    mdnslogInfo("AdvertiseInterface");

    if (!set->McastTxRx)
    {
        LogInfo("AdvertiseInterface: Returning, not multicast capable %s", set->ifname);
        return;
    }
#if TARGET_OS_EMBEDDED
    if (!m->AutoTargetServices)
    {
        LogInfo("AdvertiseInterface: Returning due to AutoTargetServices zero for %s", set->ifname);
        return;
    }
#endif

    primary = FindFirstAdvertisedInterface(m);
    if (!primary) primary = set; // If no existing advertised interface, this new NetworkInterfaceInfo becomes our new primary

    // If interface is marked as a direct link, we can assume the address record is unique
    // and does not need to go through the probe phase of the probe/announce packet sequence.
    mDNSu8 recordType = (set->DirectLink ? kDNSRecordTypeKnownUnique : kDNSRecordTypeUnique);

    if (set->DirectLink)
        LogInfo("AdvertiseInterface: Marking address record as kDNSRecordTypeKnownUnique for %s", set->ifname);

    mdnslogInfo("AdvertiseInterface - InterfaceID %d", set->InterfaceID);

    // Send dynamic update for non-linklocal IPv4 Addresses
    mDNS_SetupResourceRecord(&set->RR_A,     mDNSNULL, set->InterfaceID, kDNSType_A,     kHostNameTTL, recordType,      AuthRecordAny, mDNS_HostNameCallback, set);
    mDNS_SetupResourceRecord(&set->RR_PTR,   mDNSNULL, set->InterfaceID, kDNSType_PTR,   kHostNameTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
#ifndef REMOVE_RR_HINFO
    mDNS_SetupResourceRecord(&set->RR_HINFO, mDNSNULL, set->InterfaceID, kDNSType_HINFO, kHostNameTTL, kDNSRecordTypeUnique,      AuthRecordAny, mDNSNULL, mDNSNULL);
#endif

#if ANSWER_REMOTE_HOSTNAME_QUERIES
    set->RR_A.AllowRemoteQuery  = mDNStrue;
    set->RR_PTR.AllowRemoteQuery  = mDNStrue;
#ifndef REMOVE_RR_HINFO
    set->RR_HINFO.AllowRemoteQuery  = mDNStrue;
#endif
#endif
    // 1. Set up Address record to map from host name ("foo.local.") to IP address
    // 2. Set up reverse-lookup PTR record to map from our address back to our host name
    AssignDomainName(&set->RR_A.namestorage, &m->MulticastHostname);
    if (set->ip.type == mDNSAddrType_IPv4)
    {
        set->RR_A.resrec.rrtype = kDNSType_A;
        set->RR_A.resrec.rdata->u.ipv4 = set->ip.ip.v4;
        // Note: This is reverse order compared to a normal dotted-decimal IP address, so we can't use our customary "%.4a" format code
        mDNS_snprintf(buffer, sizeof(buffer), "%d.%d.%d.%d.in-addr.arpa.",
                      set->ip.ip.v4.b[3], set->ip.ip.v4.b[2], set->ip.ip.v4.b[1], set->ip.ip.v4.b[0]);
    }
    else if (set->ip.type == mDNSAddrType_IPv6)
    {
        int i;
        set->RR_A.resrec.rrtype = kDNSType_AAAA;
        set->RR_A.resrec.rdata->u.ipv6 = set->ip.ip.v6;
        for (i = 0; i < 16; i++)
        {
            static const char hexValues[] = "0123456789ABCDEF";
            buffer[i * 4    ] = hexValues[set->ip.ip.v6.b[15 - i] & 0x0F];
            buffer[i * 4 + 1] = '.';
            buffer[i * 4 + 2] = hexValues[set->ip.ip.v6.b[15 - i] >> 4];
            buffer[i * 4 + 3] = '.';
        }
        mDNS_snprintf(&buffer[64], sizeof(buffer)-64, "ip6.arpa.");
    }

    MakeDomainNameFromDNSNameString(&set->RR_PTR.namestorage, buffer);
    set->RR_PTR.AutoTarget = Target_AutoHost;   // Tell mDNS that the target of this PTR is to be kept in sync with our host name
    set->RR_PTR.ForceMCast = mDNStrue;          // This PTR points to our dot-local name, so don't ever try to write it into a uDNS server

    set->RR_A.RRSet = &primary->RR_A;           // May refer to self

    mDNS_Register_internal(m, &set->RR_A);
    mDNS_Register_internal(m, &set->RR_PTR);

#if APPLE_OSX_mDNSResponder
    // must be after the mDNS_Register_internal() calls so that records have complete rdata fields, etc
    D2D_start_advertising_interface(set);
#endif // APPLE_OSX_mDNSResponder

    if (!NO_HINFO && m->HIHardware.c[0] > 0 && m->HISoftware.c[0] > 0 && m->HIHardware.c[0] + m->HISoftware.c[0] <= 254)
    {
	debugf("Doesn't Support HINFO Record.\n");
   #ifndef REMOVE_RR_HINFO
        mDNSu8 *p = set->RR_HINFO.resrec.rdata->u.data;
        AssignDomainName(&set->RR_HINFO.namestorage, &m->MulticastHostname);
        set->RR_HINFO.DependentOn = &set->RR_A;
        mDNSPlatformMemCopy(p, &m->HIHardware, 1 + (mDNSu32)m->HIHardware.c[0]);
        p += 1 + (int)p[0];
        mDNSPlatformMemCopy(p, &m->HISoftware, 1 + (mDNSu32)m->HISoftware.c[0]);
        mDNS_Register_internal(m, &set->RR_HINFO);
   #endif
    }
    else
    {
        debugf("Not creating HINFO record: platform support layer provided no information");
    #ifndef REMOVE_RR_HINFO
        set->RR_HINFO.resrec.RecordType = kDNSRecordTypeUnregistered;
    #endif
    }
}

mDNSlocal void DeadvertiseInterface(mDNS *const m, NetworkInterfaceInfo *set)
{
    NetworkInterfaceInfo *intf;

    // If we still have address records referring to this one, update them
    NetworkInterfaceInfo *primary = FindFirstAdvertisedInterface(m);
    AuthRecord *A = primary ? &primary->RR_A : mDNSNULL;
    for (intf = m->HostInterfaces; intf; intf = intf->next)
        if (intf->RR_A.RRSet == &set->RR_A)
            intf->RR_A.RRSet = A;

    // Unregister these records.
    // When doing the mDNS_Exit processing, we first call DeadvertiseInterface for each interface, so by the time the platform
    // support layer gets to call mDNS_DeregisterInterface, the address and PTR records have already been deregistered for it.
    // Also, in the event of a name conflict, one or more of our records will have been forcibly deregistered.
    // To avoid unnecessary and misleading warning messages, we check the RecordType before calling mDNS_Deregister_internal().
    if (set->RR_A.resrec.RecordType) mDNS_Deregister_internal(m, &set->RR_A,     mDNS_Dereg_normal);
    if (set->RR_PTR.resrec.RecordType) mDNS_Deregister_internal(m, &set->RR_PTR,   mDNS_Dereg_normal);
#ifndef REMOVE_RR_HINFO
    if (set->RR_HINFO.resrec.RecordType) mDNS_Deregister_internal(m, &set->RR_HINFO, mDNS_Dereg_normal);
#endif 

#if APPLE_OSX_mDNSResponder
    D2D_stop_advertising_interface(set);
#endif // APPLE_OSX_mDNSResponder

}

mDNSlocal void AdvertiseAllInterfaceRecords(mDNS *const m)
{
    NetworkInterfaceInfo *intf;
    
    mdnslogInfo("AdvertiseAllInterfaceRecords");
    
    for (intf = m->HostInterfaces; intf; intf = intf->next)
    {
        if (intf->Advertise)
        {
            LogInfo("AdvertiseInterface: Advertising for ifname %s", intf->ifname);
            AdvertiseInterface(m, intf);
        }
    }
}

mDNSlocal void DeadvertiseAllInterfaceRecords(mDNS *const m)
{
#if TARGET_OS_EMBEDDED
    NetworkInterfaceInfo *intf;
    for (intf = m->HostInterfaces; intf; intf = intf->next)
    {
        if (intf->Advertise)
        {
            LogInfo("DeadvertiseInterface: Deadvertising for ifname %s", intf->ifname);
            DeadvertiseInterface(m, intf);
        }
    }
#else
    (void) m; //unused
#endif
}

mDNSexport void mDNS_SetFQDN(mDNS *const m)
{
    domainname newmname;
    NetworkInterfaceInfo *intf;
    AuthRecord *rr;
    newmname.c[0] = 0;

    if (!AppendDomainLabel(&newmname, &m->hostlabel))  { LogMsg("ERROR: mDNS_SetFQDN: Cannot create MulticastHostname"); return; }
    if (!AppendLiteralLabelString(&newmname, "local")) { LogMsg("ERROR: mDNS_SetFQDN: Cannot create MulticastHostname"); return; }

    static char CString[MAX_ESCAPED_DOMAIN_NAME];
    ConvertDomainNameToCString(&newmname, CString);
    debugf("mDNS_SetFQDN - %s",  CString);

    mDNS_Lock(m);

    if (SameDomainNameCS(&m->MulticastHostname, &newmname)) debugf("mDNS_SetFQDN - hostname unchanged");
    else
    {
        AssignDomainName(&m->MulticastHostname, &newmname);

        // 1. Stop advertising our address records on all interfaces
        for (intf = m->HostInterfaces; intf; intf = intf->next)
            if (intf->Advertise) DeadvertiseInterface(m, intf);

        // 2. Start advertising our address records using the new name
        for (intf = m->HostInterfaces; intf; intf = intf->next)
            if (intf->Advertise) AdvertiseInterface(m, intf);
    }

    // 3. Make sure that any AutoTarget SRV records (and the like) get updated
    for (rr = m->ResourceRecords;  rr; rr=rr->next) if (rr->AutoTarget) SetTargetToHostName(m, rr);
    for (rr = m->DuplicateRecords; rr; rr=rr->next) if (rr->AutoTarget) SetTargetToHostName(m, rr);

    mDNS_Unlock(m);
}

mDNSlocal void mDNS_HostNameCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    (void)rr;   // Unused parameter

    #if MDNS_DEBUGMSGS
    {
        char *msg = "Unknown result";
        if      (result == mStatus_NoError) msg = "Name registered";
        else if (result == mStatus_NameConflict) msg = "Name conflict";
        debugf("mDNS_HostNameCallback: %##s (%s) %s (%ld)", rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), msg, result);
    }
    #endif

    if (result == mStatus_NoError)
    {
        // Notify the client that the host name is successfully registered
        if (m->MainCallback)
            m->MainCallback(m, mStatus_NoError);
    }
    else if (result == mStatus_NameConflict)
    {
        domainlabel oldlabel = m->hostlabel;

        // 1. First give the client callback a chance to pick a new name
        if (m->MainCallback)
            m->MainCallback(m, mStatus_NameConflict);

        // 2. If the client callback didn't do it, add (or increment) an index ourselves
        // This needs to be case-INSENSITIVE compare, because we need to know that the name has been changed so as to
        // remedy the conflict, and a name that differs only in capitalization will just suffer the exact same conflict again.
        if (SameDomainLabel(m->hostlabel.c, oldlabel.c))
            IncrementLabelSuffix(&m->hostlabel, mDNSfalse);

        // 3. Generate the FQDNs from the hostlabel,
        // and make sure all SRV records, etc., are updated to reference our new hostname
        mDNS_SetFQDN(m);
        LogMsg("Local Hostname %#s.local already in use; will try %#s.local instead", oldlabel.c, m->hostlabel.c);
    }
    else if (result == mStatus_MemFree)
    {
        // .local hostnames do not require goodbyes - we ignore the MemFree (which is sent directly by
        // mDNS_Deregister_internal), and allow the caller to deallocate immediately following mDNS_DeadvertiseInterface
        debugf("mDNS_HostNameCallback: MemFree (ignored)");
    }
    else
        LogMsg("mDNS_HostNameCallback: Unknown error %d for registration of record %s", result,  rr->resrec.name->c);
}

mDNSlocal void UpdateInterfaceProtocols(mDNS *const m, NetworkInterfaceInfo *active)
{
    NetworkInterfaceInfo *intf;
    active->IPv4Available = mDNSfalse;
    active->IPv6Available = mDNSfalse;
    for (intf = m->HostInterfaces; intf; intf = intf->next)
        if (intf->InterfaceID == active->InterfaceID)
        {
            if (intf->ip.type == mDNSAddrType_IPv4 && intf->McastTxRx) active->IPv4Available = mDNStrue;
            if (intf->ip.type == mDNSAddrType_IPv6 && intf->McastTxRx) active->IPv6Available = mDNStrue;
        }
}

mDNSlocal void RestartRecordGetZoneData(mDNS * const m)
{
    AuthRecord *rr;
    LogInfo("RestartRecordGetZoneData: ResourceRecords");
    for (rr = m->ResourceRecords; rr; rr=rr->next)
        if (AuthRecord_uDNS(rr) && rr->state != regState_NoTarget)
        {
            debugf("RestartRecordGetZoneData: StartGetZoneData for %##s", rr->resrec.name->c);
            // Zero out the updateid so that if we have a pending response from the server, it won't
            // be accepted as a valid response. If we accept the response, we might free the new "nta"
            if (rr->nta) { rr->updateid = zeroID; CancelGetZoneData(m, rr->nta); }
            rr->nta = StartGetZoneData(m, rr->resrec.name, ZoneServiceUpdate, RecordRegistrationGotZoneData, rr);
        }
}
#ifndef REMOVE_NETWAKE
mDNSlocal void InitializeNetWakeState(mDNS *const m, NetworkInterfaceInfo *set)
{

    int i;
    // We initialize ThisQInterval to -1 indicating that the question has not been started
    // yet. If the question (browse) is started later during interface registration, it will
    // be stopped during interface deregistration. We can't sanity check to see if the
    // question has been stopped or not before initializing it to -1 because we need to
    // initialize it to -1 the very first time.
    
    set->NetWakeBrowse.ThisQInterval = -1;
#ifdef ONLY_ONE_NetWakeResolve
     for (i=0; i<3; i++)
    {
        set->NetWakeResolve.ThisQInterval = -1;
        set->SPSAddr[i].type = mDNSAddrType_None;
    }

#else
    for (i=0; i<3; i++)
    {
        set->NetWakeResolve[i].ThisQInterval = -1;
        set->SPSAddr[i].type = mDNSAddrType_None;
    }
#endif
    set->NextSPSAttempt     = -1;
    set->NextSPSAttemptTime = m->timenow;
}

mDNSexport void mDNS_ActivateNetWake_internal(mDNS *const m, NetworkInterfaceInfo *set)
{
    NetworkInterfaceInfo *p = m->HostInterfaces;
    while (p && p != set) p=p->next;
    if (!p) { LogMsg("mDNS_ActivateNetWake_internal: NetworkInterfaceInfo %p not found in active list", set); return; }

    if (set->InterfaceActive)
    {
        LogSPS("ActivateNetWake for %s (%#a)", set->ifname, &set->ip);
        mDNS_StartBrowse_internal(m, &set->NetWakeBrowse, &SleepProxyServiceType, &localdomain, mDNSNULL, set->InterfaceID, 0, mDNSfalse, mDNSfalse, m->SPSBrowseCallback, set);
    }

}
#endif // #ifndef REMOVE_NETWAKE
mDNSexport void mDNS_DeactivateNetWake_internal(mDNS *const m, NetworkInterfaceInfo *set)
{
#ifndef REMOVE_NETWAKE
    NetworkInterfaceInfo *p = m->HostInterfaces;
    while (p && p != set) p=p->next;
    if (!p) { LogMsg("mDNS_DeactivateNetWake_internal: NetworkInterfaceInfo %p not found in active list", set); return; }

    // Note: We start the browse only if the interface is NetWake capable and we use this to
    // stop the resolves also. Hence, the resolves should not be started without the browse
    // being started i.e, resolves should not happen unless NetWake capable which is
    // guaranteed by BeginSleepProcessing.
    if (set->NetWakeBrowse.ThisQInterval >= 0)
    {
        int i;
        LogSPS("DeactivateNetWake for %s (%#a)", set->ifname, &set->ip);

        // Stop our browse and resolve operations
        mDNS_StopQuery_internal(m, &set->NetWakeBrowse);
#ifdef ONLY_ONE_NetWakeResolve
        if (set->NetWakeResolve.ThisQInterval >= 0) mDNS_StopQuery_internal(m, &set->NetWakeResolve);
#else
        for (i=0; i<3; i++) if (set->NetWakeResolve[i].ThisQInterval >= 0) mDNS_StopQuery_internal(m, &set->NetWakeResolve[i]);
#endif
        // Make special call to the browse callback to let it know it can to remove all records for this interface
        if (m->SPSBrowseCallback)
        {
            mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
            m->SPSBrowseCallback(m, &set->NetWakeBrowse, mDNSNULL, mDNSfalse);
            mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
        }

        // Reset our variables back to initial state, so we're ready for when NetWake is turned back on
        // (includes resetting NetWakeBrowse.ThisQInterval back to -1)
        InitializeNetWakeState(m, set);
    }
#endif // #ifndef REMOVE_NETWAKE
}

mDNSexport mStatus mDNS_RegisterInterface(mDNS *const m, NetworkInterfaceInfo *set, mDNSBool flapping)
{
    AuthRecord *rr;
    mDNSBool FirstOfType = mDNStrue;
    NetworkInterfaceInfo **p = &m->HostInterfaces;

    if (!set->InterfaceID)
    { LogMsg("mDNS_RegisterInterface: Error! Tried to register a NetworkInterfaceInfo %#a with zero InterfaceID", &set->ip); return(mStatus_Invalid); }

    if (!mDNSAddressIsValidNonZero(&set->mask))
    { LogMsg("mDNS_RegisterInterface: Error! Tried to register a NetworkInterfaceInfo %#a with invalid mask %#a", &set->ip, &set->mask); return(mStatus_Invalid); }

    mDNS_Lock(m);

    // Assume this interface will be active now, unless we find a duplicate already in the list
    set->InterfaceActive = mDNStrue;
    set->IPv4Available   = (mDNSu8)(set->ip.type == mDNSAddrType_IPv4 && set->McastTxRx);
    set->IPv6Available   = (mDNSu8)(set->ip.type == mDNSAddrType_IPv6 && set->McastTxRx);

#ifndef REMOVE_NETWAKE
    InitializeNetWakeState(m, set);
#endif // #ifndef REMOVE_NETWAKE
    // Scan list to see if this InterfaceID is already represented
    while (*p)
    {
        if (*p == set)
        {
            LogMsg("mDNS_RegisterInterface: Error! Tried to register a NetworkInterfaceInfo that's already in the list");
            mDNS_Unlock(m);
            return(mStatus_AlreadyRegistered);
        }

        if ((*p)->InterfaceID == set->InterfaceID)
        {
            // This InterfaceID already represented by a different interface in the list, so mark this instance inactive for now
            set->InterfaceActive = mDNSfalse;
            if (set->ip.type == (*p)->ip.type) FirstOfType = mDNSfalse;
            if (set->ip.type == mDNSAddrType_IPv4 && set->McastTxRx) (*p)->IPv4Available = mDNStrue;
            if (set->ip.type == mDNSAddrType_IPv6 && set->McastTxRx) (*p)->IPv6Available = mDNStrue;
        }

        p=&(*p)->next;
    }

    set->next = mDNSNULL;
    *p = set;

    if (set->Advertise)
        AdvertiseInterface(m, set);

    LogInfo("mDNS_RegisterInterface: InterfaceID %p %s (%#a) %s", set->InterfaceID, set->ifname, &set->ip,
            set->InterfaceActive ?
            "not represented in list; marking active and retriggering queries" :
            "already represented in list; marking inactive for now");

#ifndef REMOVE_NETWAKE
    if (set->NetWake) mDNS_ActivateNetWake_internal(m, set);
#endif // #ifndef REMOVE_NETWAKE

    // In early versions of OS X the IPv6 address remains on an interface even when the interface is turned off,
    // giving the false impression that there's an active representative of this interface when there really isn't.
    // Therefore, when registering an interface, we want to re-trigger our questions and re-probe our Resource Records,
    // even if we believe that we previously had an active representative of this interface.
    if (set->McastTxRx && (FirstOfType || set->InterfaceActive))
    {
        DNSQuestion *q;
        // Normally, after an interface comes up, we pause half a second before beginning probing.
        // This is to guard against cases where there's rapid interface changes, where we could be confused by
        // seeing packets we ourselves sent just moments ago (perhaps when this interface had a different address)
        // which are then echoed back after a short delay by some Ethernet switches and some 802.11 base stations.
        // We don't want to do a probe, and then see a stale echo of an announcement we ourselves sent,
        // and think it's a conflicting answer to our probe.
        // In the case of a flapping interface, we pause for five seconds, and reduce the announcement count to one packet.
        const mDNSs32 probedelay  = flapping ? mDNSPlatformOneSecond * 5 : mDNSPlatformOneSecond / 2;
        const mDNSu8 numannounce = flapping ? (mDNSu8)1                 : InitialAnnounceCount;

        // Use a small amount of randomness:
        // In the case of a network administrator turning on an Ethernet hub so that all the
        // connected machines establish link at exactly the same time, we don't want them all
        // to go and hit the network with identical queries at exactly the same moment.
        // We set a random delay of up to InitialQuestionInterval (1/3 second).
        // We must *never* set m->SuppressSending to more than that (or set it repeatedly in a way
        // that causes mDNSResponder to remain in a prolonged state of SuppressSending, because
        // suppressing packet sending for more than about 1/3 second can cause protocol correctness
        // to start to break down (e.g. we don't answer probes fast enough, and get name conflicts).
        // See <rdar://problem/4073853> mDNS: m->SuppressSending set too enthusiastically
        if (!m->SuppressSending) m->SuppressSending = m->timenow + (mDNSs32)mDNSRandom((mDNSu32)InitialQuestionInterval);

        if (flapping)
        {
            LogMsg("mDNS_RegisterInterface: Frequent transitions for interface %s (%#a)", set->ifname, &set->ip);
            m->mDNSStats.InterfaceUpFlap++;
        }

        LogInfo("mDNS_RegisterInterface: %s (%#a) probedelay %d", set->ifname, &set->ip, probedelay);
        if (m->SuppressProbes == 0 ||
            m->SuppressProbes - NonZeroTime(m->timenow + probedelay) < 0)
            m->SuppressProbes = NonZeroTime(m->timenow + probedelay);

        // Include OWNER option in packets for 60 seconds after connecting to the network. Setting
        // it here also handles the wake up case as the network link comes UP after waking causing
        // us to reconnect to the network. If we do this as part of the wake up code, it is possible
        // that the network link comes UP after 60 seconds and we never set the OWNER option
        m->AnnounceOwner = NonZeroTime(m->timenow + 60 * mDNSPlatformOneSecond);
        LogInfo("mDNS_RegisterInterface: Setting AnnounceOwner");

        m->mDNSStats.InterfaceUp++;
        for (q = m->Questions; q; q=q->next)                                // Scan our list of questions
        {
            if (mDNSOpaque16IsZero(q->TargetQID))
            {
                if (!q->InterfaceID || q->InterfaceID == set->InterfaceID)      // If non-specific Q, or Q on this specific interface,
                {                                                               // then reactivate this question
                    // If flapping, delay between first and second queries is nine seconds instead of one second
                    mDNSBool dodelay = flapping && (q->FlappingInterface1 == set->InterfaceID || q->FlappingInterface2 == set->InterfaceID);
                    mDNSs32 initial  = dodelay ? InitialQuestionInterval * QuestionIntervalStep2 : InitialQuestionInterval;
                    mDNSs32 qdelay   = dodelay ? mDNSPlatformOneSecond * 5 : 0;
                    if (dodelay) LogInfo("No cache records expired for %##s (%s); okay to delay questions a little", q->qname.c, DNSTypeName(q->qtype));

                    if (!q->ThisQInterval || q->ThisQInterval > initial)
                    {
                        q->ThisQInterval = initial;

#if mDNS_REQUEST_UNICAST_RESPONSE
                        q->RequestUnicast = SET_QU_IN_FIRST_FOUR_QUERIES;
#else   // mDNS_REQUEST_UNICAST_RESPONSE
                        q->RequestUnicast = SET_QU_IN_FIRST_QUERY;
#endif  // mDNS_REQUEST_UNICAST_RESPONSE

                    }
                    q->LastQTime = m->timenow - q->ThisQInterval + qdelay;
                    q->RecentAnswerPkts = 0;
                    // Change the salt
                    ReInitAnonInfo(&q->AnonInfo, &q->qname);
                    SetNextQueryTime(m,q);
                }
            }
        }

        // For all our non-specific authoritative resource records (and any dormant records specific to this interface)
        // we now need them to re-probe if necessary, and then re-announce.
        for (rr = m->ResourceRecords; rr; rr=rr->next)
        {
            if (!rr->resrec.InterfaceID || rr->resrec.InterfaceID == set->InterfaceID)
            {
                // Change the salt
                ReInitAnonInfo(&rr->resrec.AnonInfo, rr->resrec.name);
                mDNSCoreRestartRegistration(m, rr, numannounce);
            }
        }
#if APPLE_OSX_mDNSResponder && !TARGET_OS_IPHONE
        DNSSECProbe(m);
#endif
    }

    RestartRecordGetZoneData(m);

    mDNS_UpdateAllowSleep(m);

    mDNS_Unlock(m);
    return(mStatus_NoError);
}

// Note: mDNS_DeregisterInterface calls mDNS_Deregister_internal which can call a user callback, which may change
// the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSexport void mDNS_DeregisterInterface(mDNS *const m, NetworkInterfaceInfo *set, mDNSBool flapping)
{
    NetworkInterfaceInfo **p = &m->HostInterfaces;
    mDNSBool revalidate = mDNSfalse;

    mDNS_Lock(m);

    // Find this record in our list
    while (*p && *p != set) p=&(*p)->next;
    if (!*p) { debugf("mDNS_DeregisterInterface: NetworkInterfaceInfo not found in list"); mDNS_Unlock(m); return; }

#ifndef REMOVE_NETWAKE
    mDNS_DeactivateNetWake_internal(m, set);
#endif // #ifndef REMOVE_NETWAKE

    // Unlink this record from our list
    *p = (*p)->next;
    set->next = mDNSNULL;

    if (!set->InterfaceActive)
    {
        // If this interface not the active member of its set, update the v4/v6Available flags for the active member
        NetworkInterfaceInfo *intf;
        for (intf = m->HostInterfaces; intf; intf = intf->next)
            if (intf->InterfaceActive && intf->InterfaceID == set->InterfaceID)
                UpdateInterfaceProtocols(m, intf);
    }
    else
    {
        NetworkInterfaceInfo *intf = FirstInterfaceForID(m, set->InterfaceID);
        if (intf)
        {
            LogInfo("mDNS_DeregisterInterface: Another representative of InterfaceID %p %s (%#a) exists;"
                    " making it active", set->InterfaceID, set->ifname, &set->ip);
            if (intf->InterfaceActive)
                LogMsg("mDNS_DeregisterInterface: ERROR intf->InterfaceActive already set for %s (%#a)", set->ifname, &set->ip);
            intf->InterfaceActive = mDNStrue;
            UpdateInterfaceProtocols(m, intf);

#ifndef REMOVE_NETWAKE
            if (intf->NetWake) mDNS_ActivateNetWake_internal(m, intf);
#endif // #ifndef REMOVE_NETWAKE
            // See if another representative *of the same type* exists. If not, we mave have gone from
            // dual-stack to v6-only (or v4-only) so we need to reconfirm which records are still valid.
            for (intf = m->HostInterfaces; intf; intf = intf->next)
                if (intf->InterfaceID == set->InterfaceID && intf->ip.type == set->ip.type)
                    break;
            if (!intf) revalidate = mDNStrue;
        }
        else
        {
            mDNSu32 slot;
            CacheGroup *cg;
            CacheRecord *rr;
            DNSQuestion *q;

            LogInfo("mDNS_DeregisterInterface: Last representative of InterfaceID %p %s (%#a) deregistered;"
                    " marking questions etc. dormant", set->InterfaceID, set->ifname, &set->ip);

            m->mDNSStats.InterfaceDown++;

            if (set->McastTxRx && flapping)
            {
                LogMsg("DeregisterInterface: Frequent transitions for interface %s (%#a)", set->ifname, &set->ip);
                m->mDNSStats.InterfaceDownFlap++;
            }

            // 1. Deactivate any questions specific to this interface, and tag appropriate questions
            // so that mDNS_RegisterInterface() knows how swiftly it needs to reactivate them
            for (q = m->Questions; q; q=q->next)
            {
                if (q->InterfaceID == set->InterfaceID) q->ThisQInterval = 0;
                if (!q->InterfaceID || q->InterfaceID == set->InterfaceID)
                {
                    q->FlappingInterface2 = q->FlappingInterface1;
                    q->FlappingInterface1 = set->InterfaceID;       // Keep history of the last two interfaces to go away
                }
            }

            // 2. Flush any cache records received on this interface
            revalidate = mDNSfalse;     // Don't revalidate if we're flushing the records
            FORALL_CACHERECORDS(slot, cg, rr)
            {
                if (rr->resrec.InterfaceID == set->InterfaceID)
                {
                    // If this interface is deemed flapping,
                    // postpone deleting the cache records in case the interface comes back again
                    if (set->McastTxRx && flapping)
                    {
                        // For a flapping interface we want these record to go away after 30 seconds
                        mDNS_Reconfirm_internal(m, rr, kDefaultReconfirmTimeForFlappingInterface);
                        // We set UnansweredQueries = MaxUnansweredQueries so we don't waste time doing any queries for them --
                        // if the interface does come back, any relevant questions will be reactivated anyway
                        rr->UnansweredQueries = MaxUnansweredQueries;
                    }
                    else
                    {
                        mDNS_PurgeCacheResourceRecord(m, rr);
                    }
                }
            }
        }
    }

    // If we were advertising on this interface, deregister those address and reverse-lookup records now
    if (set->Advertise) DeadvertiseInterface(m, set);

    // If we have any cache records received on this interface that went away, then re-verify them.
    // In some versions of OS X the IPv6 address remains on an interface even when the interface is turned off,
    // giving the false impression that there's an active representative of this interface when there really isn't.
    // Don't need to do this when shutting down, because *all* interfaces are about to go away
    if (revalidate && !m->ShutdownTime)
    {
        mDNSu32 slot;
        CacheGroup *cg;
        CacheRecord *rr;
        FORALL_CACHERECORDS(slot, cg, rr)
        if (rr->resrec.InterfaceID == set->InterfaceID)
            mDNS_Reconfirm_internal(m, rr, kDefaultReconfirmTimeForFlappingInterface);
    }

    mDNS_UpdateAllowSleep(m);

    mDNS_Unlock(m);
}

mDNSlocal void SetAnonInfoSRS(ServiceRecordSet *sr, int NumSubTypes)
{
    int i, len;

    if (!sr->AnonData)
        return;

    len = mDNSPlatformStrLen(sr->AnonData);
    if (sr->RR_PTR.resrec.AnonInfo)
    {
        LogMsg("SetAnonInfoSRS: Freeing AnonInfo for PTR record %##s, should have been freed already", sr->RR_PTR.resrec.name->c);
        FreeAnonInfo(sr->RR_PTR.resrec.AnonInfo);
    }
    sr->RR_PTR.resrec.AnonInfo = AllocateAnonInfo(sr->RR_PTR.resrec.name, sr->AnonData, len, mDNSNULL);
    for (i=0; i<NumSubTypes; i++)
    {
        if (sr->SubTypes[i].resrec.AnonInfo)
        {
            LogMsg("SetAnonInfoSRS: Freeing AnonInfo for subtype record %##s, should have been freed already", sr->SubTypes[i].resrec.name->c);
            FreeAnonInfo(sr->SubTypes[i].resrec.AnonInfo);
        }
        sr->SubTypes[i].resrec.AnonInfo = AllocateAnonInfo(sr->SubTypes[i].resrec.name, sr->AnonData, len, mDNSNULL);
    }
}

mDNSlocal void ResetAnonInfoSRS(ServiceRecordSet *sr, int NumSubTypes)
{
    int i;

    if (!sr->AnonData)
        return;
    if (sr->RR_PTR.resrec.AnonInfo)
    {
        FreeAnonInfo(sr->RR_PTR.resrec.AnonInfo);
        sr->RR_PTR.resrec.AnonInfo = mDNSNULL;
    }
    for (i=0; i<NumSubTypes; i++)
    {
        if (sr->SubTypes[i].resrec.AnonInfo)
        {
            FreeAnonInfo(sr->SubTypes[i].resrec.AnonInfo);
            sr->SubTypes[i].resrec.AnonInfo = mDNSNULL;
        }
    }
}

mDNSlocal void ServiceCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    ServiceRecordSet *sr = (ServiceRecordSet *)rr->RecordContext;
    (void)m;    // Unused parameter

    #if MDNS_DEBUGMSGS
    {
        char *msg = "Unknown result";
        if      (result == mStatus_NoError) msg = "Name Registered";
        else if (result == mStatus_NameConflict) msg = "Name Conflict";
        else if (result == mStatus_MemFree) msg = "Memory Free";
        debugf("ServiceCallback: %##s (%s) %s (%d)", rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), msg, result);
    }
    #endif

    // Only pass on the NoError acknowledgement for the SRV record (when it finishes probing)
    if (result == mStatus_NoError && rr != &sr->RR_SRV) return;

    // If we got a name conflict on either SRV or TXT, forcibly deregister this service, and record that we did that
    if (result == mStatus_NameConflict)
    {
        sr->Conflict = mDNStrue;                // Record that this service set had a conflict
        mDNS_DeregisterService(m, sr);          // Unlink the records from our list
        return;
    }

    if (result == mStatus_MemFree)
    {
        // If the SRV/TXT/PTR records, or the _services._dns-sd._udp record, or any of the subtype PTR records,
        // are still in the process of deregistering, don't pass on the NameConflict/MemFree message until
        // every record is finished cleaning up.
        mDNSu32 i;
        ExtraResourceRecord *e = sr->Extras;

        if (sr->RR_SRV.resrec.RecordType != kDNSRecordTypeUnregistered) return;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
        if (sr->RR_TXT.resrec.RecordType != kDNSRecordTypeUnregistered) return;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
        if (sr->RR_PTR.resrec.RecordType != kDNSRecordTypeUnregistered) return;
        if (sr->RR_ADV.resrec.RecordType != kDNSRecordTypeUnregistered) return;
        for (i=0; i<sr->NumSubTypes; i++) if (sr->SubTypes[i].resrec.RecordType != kDNSRecordTypeUnregistered) return;

        while (e)
        {
            if (e->r.resrec.RecordType != kDNSRecordTypeUnregistered) return;
            e = e->next;
        }
        ResetAnonInfoSRS(sr, sr->NumSubTypes);

        // If this ServiceRecordSet was forcibly deregistered, and now its memory is ready for reuse,
        // then we can now report the NameConflict to the client
        if (sr->Conflict) result = mStatus_NameConflict;

    }

    LogInfo("ServiceCallback: All records %s for %##s", (result == mStatus_MemFree ? "Unregistered" : "Registered"), sr->RR_PTR.resrec.name->c);
    // CAUTION: MUST NOT do anything more with sr after calling sr->Callback(), because the client's callback
    // function is allowed to do anything, including deregistering this service and freeing its memory.
    if (sr->ServiceCallback)
        sr->ServiceCallback(m, sr, result);
}

mDNSlocal void NSSCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    ServiceRecordSet *sr = (ServiceRecordSet *)rr->RecordContext;
    if (sr->ServiceCallback)
        sr->ServiceCallback(m, sr, result);
}


mDNSlocal AuthRecType setAuthRecType(mDNSInterfaceID InterfaceID, mDNSu32 flags)
{
    AuthRecType artype;

    if (InterfaceID == mDNSInterface_LocalOnly)
        artype = AuthRecordLocalOnly;
    else if (InterfaceID == mDNSInterface_P2P)
        artype = AuthRecordP2P;
    else if ((InterfaceID == mDNSInterface_Any) && (flags & coreFlagIncludeP2P)
            && (flags & coreFlagIncludeAWDL))
        artype = AuthRecordAnyIncludeAWDLandP2P;
    else if ((InterfaceID == mDNSInterface_Any) && (flags & coreFlagIncludeP2P))
        artype = AuthRecordAnyIncludeP2P;
    else if ((InterfaceID == mDNSInterface_Any) && (flags & coreFlagIncludeAWDL))
        artype = AuthRecordAnyIncludeAWDL;
    else
        artype = AuthRecordAny;

    return artype;
}

// Note:
// Name is first label of domain name (any dots in the name are actual dots, not label separators)
// Type is service type (e.g. "_ipp._tcp.")
// Domain is fully qualified domain name (i.e. ending with a null label)
// We always register a TXT, even if it is empty (so that clients are not
// left waiting forever looking for a nonexistent record.)
// If the host parameter is mDNSNULL or the root domain (ASCII NUL),
// then the default host name (m->MulticastHostname) is automatically used
// If the optional target host parameter is set, then the storage it points to must remain valid for the lifetime of the service registration
mDNSexport mStatus mDNS_RegisterService(mDNS *const m, ServiceRecordSet *sr,
                                        const domainlabel *const name, const domainname *const type, const domainname *const domain,
                                        const domainname *const host, mDNSIPPort port, const mDNSu8 txtinfo[], mDNSu16 txtlen,
                                        AuthRecord *SubTypes, mDNSu32 NumSubTypes,
                                        mDNSInterfaceID InterfaceID, mDNSServiceCallback Callback, void *Context, mDNSu32 flags)
{
    mStatus err;
    mDNSu32 i;
    mDNSu32 hostTTL;
    AuthRecType artype;
    mDNSu8 recordType = (flags & coreFlagKnownUnique) ? kDNSRecordTypeKnownUnique : kDNSRecordTypeUnique;

    sr->ServiceCallback = Callback;
    sr->ServiceContext  = Context;
    sr->Conflict        = mDNSfalse;

    sr->Extras          = mDNSNULL;
    sr->NumSubTypes     = NumSubTypes;
    sr->SubTypes        = SubTypes;
    sr->flags           = flags;

	debugf("%s %d", __PRETTY_FUNCTION__, __LINE__);

    artype = setAuthRecType(InterfaceID, flags);

    // Initialize the AuthRecord objects to sane values
    // Need to initialize everything correctly *before* making the decision whether to do a RegisterNoSuchService and bail out
    mDNS_SetupResourceRecord(&sr->RR_ADV, mDNSNULL, InterfaceID, kDNSType_PTR, kStandardTTL, kDNSRecordTypeAdvisory, artype, ServiceCallback, sr);
    mDNS_SetupResourceRecord(&sr->RR_PTR, mDNSNULL, InterfaceID, kDNSType_PTR, kStandardTTL, kDNSRecordTypeShared,   artype, ServiceCallback, sr);

    if (flags & coreFlagWakeOnly)
    {
        sr->RR_PTR.AuthFlags = AuthFlagsWakeOnly;
    }

    if (SameDomainName(type, (const domainname *) "\x4" "_ubd" "\x4" "_tcp"))
        hostTTL = kHostNameSmallTTL;
    else
        hostTTL = kHostNameTTL;

    mDNS_SetupResourceRecord(&sr->RR_SRV, mDNSNULL, InterfaceID, kDNSType_SRV, hostTTL, recordType, artype, ServiceCallback, sr);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    mDNS_SetupResourceRecord(&sr->RR_TXT, mDNSNULL, InterfaceID, kDNSType_TXT, kStandardTTL, kDNSRecordTypeUnique, artype, ServiceCallback, sr);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

    // If port number is zero, that means the client is really trying to do a RegisterNoSuchService
    if (mDNSIPPortIsZero(port))
        return(mDNS_RegisterNoSuchService(m, &sr->RR_SRV, name, type, domain, mDNSNULL, InterfaceID, NSSCallback, sr, flags));

#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // If the client is registering an oversized TXT record,
    // it is the client's responsibility to alloate a ServiceRecordSet structure that is large enough for it
    if (sr->RR_TXT.resrec.rdata->MaxRDLength < txtlen)
        sr->RR_TXT.resrec.rdata->MaxRDLength = txtlen;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

    // Set up the record names
    // For now we only create an advisory record for the main type, not for subtypes
    // We need to gain some operational experience before we decide if there's a need to create them for subtypes too
    if (ConstructServiceName(&sr->RR_ADV.namestorage, (const domainlabel*)"\x09_services", (const domainname*)"\x07_dns-sd\x04_udp", domain) == mDNSNULL)
        return(mStatus_BadParamErr);
    if (ConstructServiceName(&sr->RR_PTR.namestorage, mDNSNULL, type, domain) == mDNSNULL) return(mStatus_BadParamErr);
    if (ConstructServiceName(&sr->RR_SRV.namestorage, name,     type, domain) == mDNSNULL) return(mStatus_BadParamErr);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    AssignDomainName(&sr->RR_TXT.namestorage, sr->RR_SRV.resrec.name);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

    // 1. Set up the ADV record rdata to advertise our service type
    AssignDomainName(&sr->RR_ADV.resrec.rdata->u.name, sr->RR_PTR.resrec.name);

    // 2. Set up the PTR record rdata to point to our service name
    // We set up two additionals, so when a client asks for this PTR we automatically send the SRV and the TXT too
    // Note: uDNS registration code assumes that Additional1 points to the SRV record
    AssignDomainName(&sr->RR_PTR.resrec.rdata->u.name, sr->RR_SRV.resrec.name);
    sr->RR_PTR.Additional1 = &sr->RR_SRV;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    sr->RR_PTR.Additional2 = &sr->RR_TXT;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

    // 2a. Set up any subtype PTRs to point to our service name
    // If the client is using subtypes, it is the client's responsibility to have
    // already set the first label of the record name to the subtype being registered
    for (i=0; i<NumSubTypes; i++)
    {
        domainname st;
        AssignDomainName(&st, sr->SubTypes[i].resrec.name);
        st.c[1+st.c[0]] = 0;            // Only want the first label, not the whole FQDN (particularly for mDNS_RenameAndReregisterService())
        AppendDomainName(&st, type);
        mDNS_SetupResourceRecord(&sr->SubTypes[i], mDNSNULL, InterfaceID, kDNSType_PTR, kStandardTTL, kDNSRecordTypeShared, artype, ServiceCallback, sr);
        if (ConstructServiceName(&sr->SubTypes[i].namestorage, mDNSNULL, &st, domain) == mDNSNULL) return(mStatus_BadParamErr);
        AssignDomainName(&sr->SubTypes[i].resrec.rdata->u.name, &sr->RR_SRV.namestorage);
        sr->SubTypes[i].Additional1 = &sr->RR_SRV;
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
        sr->SubTypes[i].Additional2 = &sr->RR_TXT;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    }
    
    SetAnonInfoSRS(sr, NumSubTypes);

    // 3. Set up the SRV record rdata.
    sr->RR_SRV.resrec.rdata->u.srv.priority = 0;
    sr->RR_SRV.resrec.rdata->u.srv.weight   = 0;
    sr->RR_SRV.resrec.rdata->u.srv.port     = port;

    // Setting AutoTarget tells DNS that the target of this SRV is to be automatically kept in sync with our host name
    if (host && host->c[0]) AssignDomainName(&sr->RR_SRV.resrec.rdata->u.srv.target, host);
    else { sr->RR_SRV.AutoTarget = Target_AutoHost; sr->RR_SRV.resrec.rdata->u.srv.target.c[0] = '\0'; }

#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // 4. Set up the TXT record rdata,
    // and set DependentOn because we're depending on the SRV record to find and resolve conflicts for us
    // Note: uDNS registration code assumes that DependentOn points to the SRV record
    if (txtinfo == mDNSNULL) sr->RR_TXT.resrec.rdlength = 0;
    else if (txtinfo != sr->RR_TXT.resrec.rdata->u.txt.c)
    {
        sr->RR_TXT.resrec.rdlength = txtlen;
        if (sr->RR_TXT.resrec.rdlength > sr->RR_TXT.resrec.rdata->MaxRDLength) return(mStatus_BadParamErr);
        mDNSPlatformMemCopy(sr->RR_TXT.resrec.rdata->u.txt.c, txtinfo, txtlen);
    }
    sr->RR_TXT.DependentOn = &sr->RR_SRV;
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    mDNS_Lock(m);
    // It is important that we register SRV first. uDNS assumes that SRV is registered first so
    // that if the SRV cannot find a target, rest of the records that belong to this service
    // will not be activated.
    err = mDNS_Register_internal(m, &sr->RR_SRV);
    // If we can't register the SRV record due to errors, bail out. It has not been inserted in
    // any list and hence no need to deregister. We could probably do similar checks for other
    // records below and bail out. For now, this seems to be sufficient to address rdar://9304275
    if (err)
    {
        mDNS_Unlock(m);
        return err;
    }
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    if (!err) err = mDNS_Register_internal(m, &sr->RR_TXT);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // We register the RR_PTR last, because we want to be sure that in the event of a forced call to
    // mDNS_StartExit, the RR_PTR will be the last one to be forcibly deregistered, since that is what triggers
    // the mStatus_MemFree callback to ServiceCallback, which in turn passes on the mStatus_MemFree back to
    // the client callback, which is then at liberty to free the ServiceRecordSet memory at will. We need to
    // make sure we've deregistered all our records and done any other necessary cleanup before that happens.
    if (!err) err = mDNS_Register_internal(m, &sr->RR_ADV);
    for (i=0; i<NumSubTypes; i++) if (!err) err = mDNS_Register_internal(m, &sr->SubTypes[i]);
    mdnslogInfo("mDNS_Register_internal RR_PTR Begin");
    if (!err) err = mDNS_Register_internal(m, &sr->RR_PTR);

    mDNS_Unlock(m);

    if (err) mDNS_DeregisterService(m, sr);
    return(err);
}

mDNSexport mStatus mDNS_AddRecordToService(mDNS *const m, ServiceRecordSet *sr,
                                           ExtraResourceRecord *extra, RData *rdata, mDNSu32 ttl,  mDNSu32 flags)
{
    ExtraResourceRecord **e;
    mStatus status;
    AuthRecType artype;
    mDNSInterfaceID InterfaceID = sr->RR_PTR.resrec.InterfaceID;

    artype = setAuthRecType(InterfaceID, flags);

    extra->next = mDNSNULL;
    mDNS_SetupResourceRecord(&extra->r, rdata, sr->RR_PTR.resrec.InterfaceID,
                             extra->r.resrec.rrtype, ttl, kDNSRecordTypeUnique, artype, ServiceCallback, sr);
    AssignDomainName(&extra->r.namestorage, sr->RR_SRV.resrec.name);

    mDNS_Lock(m);
    e = &sr->Extras;
    while (*e) e = &(*e)->next;

    if (ttl == 0) ttl = kStandardTTL;

    extra->r.DependentOn = &sr->RR_SRV;

    debugf("mDNS_AddRecordToService adding record to %##s %s %d",
           extra->r.resrec.name->c, DNSTypeName(extra->r.resrec.rrtype), extra->r.resrec.rdlength);

    status = mDNS_Register_internal(m, &extra->r);
    if (status == mStatus_NoError) *e = extra;

    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_RemoveRecordFromService(mDNS *const m, ServiceRecordSet *sr, ExtraResourceRecord *extra,
                                                mDNSRecordCallback MemFreeCallback, void *Context)
{
    ExtraResourceRecord **e;
    mStatus status;

    mDNS_Lock(m);
    e = &sr->Extras;
    while (*e && *e != extra) e = &(*e)->next;
    if (!*e)
    {
        debugf("mDNS_RemoveRecordFromService failed to remove record from %##s", extra->r.resrec.name->c);
        status = mStatus_BadReferenceErr;
    }
    else
    {
        debugf("mDNS_RemoveRecordFromService removing record from %##s", extra->r.resrec.name->c);
        extra->r.RecordCallback = MemFreeCallback;
        extra->r.RecordContext  = Context;
        *e = (*e)->next;
        status = mDNS_Deregister_internal(m, &extra->r, mDNS_Dereg_normal);
    }
    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_RenameAndReregisterService(mDNS *const m, ServiceRecordSet *const sr, const domainlabel *newname)
{
	debugf("%s %d", __PRETTY_FUNCTION__, __LINE__);
	
    // Note: Don't need to use mDNS_Lock(m) here, because this code is just using public routines
    // mDNS_RegisterService() and mDNS_AddRecordToService(), which do the right locking internally.
    domainlabel name1, name2;
    domainname type, domain;
    const domainname *host = sr->RR_SRV.AutoTarget ? mDNSNULL : &sr->RR_SRV.resrec.rdata->u.srv.target;
    ExtraResourceRecord *extras = sr->Extras;
    mStatus err;

    DeconstructServiceName(sr->RR_SRV.resrec.name, &name1, &type, &domain);
    if (!newname)
    {
        name2 = name1;
        IncrementLabelSuffix(&name2, mDNStrue);
        newname = &name2;
    }

    if (SameDomainName(&domain, &localdomain))
        debugf("%##s service renamed from \"%#s\" to \"%#s\"", type.c, name1.c, newname->c);
    else debugf("%##s service (domain %##s) renamed from \"%#s\" to \"%#s\"",type.c, domain.c, name1.c, newname->c);

#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
  err = mDNS_RegisterService(m, sr, newname, &type, &domain,
                               host, sr->RR_SRV.resrec.rdata->u.srv.port, sr->RR_TXT.resrec.rdata->u.txt.c, sr->RR_TXT.resrec.rdlength,
                               sr->SubTypes, sr->NumSubTypes,
                               sr->RR_PTR.resrec.InterfaceID, sr->ServiceCallback, sr->ServiceContext, sr->flags);
#else
  err = mDNS_RegisterService(m, sr, newname, &type, &domain,
                               host, sr->RR_SRV.resrec.rdata->u.srv.port, mDNSNULL, mDNSNULL,
                               sr->SubTypes, sr->NumSubTypes,
                               sr->RR_PTR.resrec.InterfaceID, sr->ServiceCallback, sr->ServiceContext, sr->flags);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
    // mDNS_RegisterService() just reset sr->Extras to NULL.
    // Fortunately we already grabbed ourselves a copy of this pointer (above), so we can now run
    // through the old list of extra records, and re-add them to our freshly created service registration
    while (!err && extras)
    {
        ExtraResourceRecord *e = extras;
        extras = extras->next;
        err = mDNS_AddRecordToService(m, sr, e, e->r.resrec.rdata, e->r.resrec.rroriginalttl, 0);
    }

    return(err);
}

// Note: mDNS_DeregisterService calls mDNS_Deregister_internal which can call a user callback,
// which may change the record list and/or question list.
// Any code walking either list must use the CurrentQuestion and/or CurrentRecord mechanism to protect against this.
mDNSexport mStatus mDNS_DeregisterService_drt(mDNS *const m, ServiceRecordSet *sr, mDNS_Dereg_type drt)
{
    // If port number is zero, that means this was actually registered using mDNS_RegisterNoSuchService()
    if (mDNSIPPortIsZero(sr->RR_SRV.resrec.rdata->u.srv.port)) return(mDNS_DeregisterNoSuchService(m, &sr->RR_SRV));

    if (sr->RR_PTR.resrec.RecordType == kDNSRecordTypeUnregistered)
    {
        debugf("Service set for %##s already deregistered", sr->RR_SRV.resrec.name->c);
        return(mStatus_BadReferenceErr);
    }
    else if (sr->RR_PTR.resrec.RecordType == kDNSRecordTypeDeregistering)
    {
        LogInfo("Service set for %##s already in the process of deregistering", sr->RR_SRV.resrec.name->c);
        // Avoid race condition:
        // If a service gets a conflict, then we set the Conflict flag to tell us to generate
        // an mStatus_NameConflict message when we get the mStatus_MemFree for our PTR record.
        // If the client happens to deregister the service in the middle of that process, then
        // we clear the flag back to the normal state, so that we deliver a plain mStatus_MemFree
        // instead of incorrectly promoting it to mStatus_NameConflict.
        // This race condition is exposed particularly when the conformance test generates
        // a whole batch of simultaneous conflicts across a range of services all advertised
        // using the same system default name, and if we don't take this precaution then
        // we end up incrementing m->nicelabel multiple times instead of just once.
        // <rdar://problem/4060169> Bug when auto-renaming Computer Name after name collision
        sr->Conflict = mDNSfalse;
        return(mStatus_NoError);
    }
    else
    {
        mDNSu32 i;
        mStatus status;
        ExtraResourceRecord *e;
        mDNS_Lock(m);
        e = sr->Extras;

        // We use mDNS_Dereg_repeat because, in the event of a collision, some or all of the
        // SRV, TXT, or Extra records could have already been automatically deregistered, and that's okay
        mDNS_Deregister_internal(m, &sr->RR_SRV, mDNS_Dereg_repeat);
#ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT
        mDNS_Deregister_internal(m, &sr->RR_TXT, mDNS_Dereg_repeat);
#endif // #ifndef REMOVE_QTXT_RECORD_TYPE_SUPPORT

        mDNS_Deregister_internal(m, &sr->RR_ADV, drt);

        // We deregister all of the extra records, but we leave the sr->Extras list intact
        // in case the client wants to do a RenameAndReregister and reinstate the registration
        while (e)
        {
            mDNS_Deregister_internal(m, &e->r, mDNS_Dereg_repeat);
            e = e->next;
        }

        for (i=0; i<sr->NumSubTypes; i++)
            mDNS_Deregister_internal(m, &sr->SubTypes[i], drt);

        status = mDNS_Deregister_internal(m, &sr->RR_PTR, drt);
        mDNS_Unlock(m);
        return(status);
    }
}

// Create a registration that asserts that no such service exists with this name.
// This can be useful where there is a given function is available through several protocols.
// For example, a printer called "Stuart's Printer" may implement printing via the "pdl-datastream" and "IPP"
// protocols, but not via "LPR". In this case it would be prudent for the printer to assert the non-existence of an
// "LPR" service called "Stuart's Printer". Without this precaution, another printer than offers only "LPR" printing
// could inadvertently advertise its service under the same name "Stuart's Printer", which might be confusing for users.
mDNSexport mStatus mDNS_RegisterNoSuchService(mDNS *const m, AuthRecord *const rr,
                                              const domainlabel *const name, const domainname *const type, const domainname *const domain,
                                              const domainname *const host,
                                              const mDNSInterfaceID InterfaceID, mDNSRecordCallback Callback, void *Context, mDNSu32 flags)
{
    AuthRecType artype;

    artype = setAuthRecType(InterfaceID, flags);

    mDNS_SetupResourceRecord(rr, mDNSNULL, InterfaceID, kDNSType_SRV, kHostNameTTL, kDNSRecordTypeUnique, artype, Callback, Context);
    if (ConstructServiceName(&rr->namestorage, name, type, domain) == mDNSNULL) return(mStatus_BadParamErr);
    rr->resrec.rdata->u.srv.priority    = 0;
    rr->resrec.rdata->u.srv.weight      = 0;
    rr->resrec.rdata->u.srv.port        = zeroIPPort;
    if (host && host->c[0]) AssignDomainName(&rr->resrec.rdata->u.srv.target, host);
    else rr->AutoTarget = Target_AutoHost;
    return(mDNS_Register(m, rr));
}

mDNSexport mStatus mDNS_AdvertiseDomains(mDNS *const m, AuthRecord *rr,
                                         mDNS_DomainType DomainType, const mDNSInterfaceID InterfaceID, char *domname)
{
    AuthRecType artype;

    if (InterfaceID == mDNSInterface_LocalOnly)
        artype = AuthRecordLocalOnly;
    else if (InterfaceID == mDNSInterface_P2P)
        artype = AuthRecordP2P;
    else
        artype = AuthRecordAny;
    mDNS_SetupResourceRecord(rr, mDNSNULL, InterfaceID, kDNSType_PTR, kStandardTTL, kDNSRecordTypeShared, artype, mDNSNULL, mDNSNULL);
    if (!MakeDomainNameFromDNSNameString(&rr->namestorage, mDNS_DomainTypeNames[DomainType])) return(mStatus_BadParamErr);
    if (!MakeDomainNameFromDNSNameString(&rr->resrec.rdata->u.name, domname)) return(mStatus_BadParamErr);
    return(mDNS_Register(m, rr));
}

mDNSlocal mDNSBool mDNS_IdUsedInResourceRecordsList(mDNS * const m, mDNSOpaque16 id)
{
    AuthRecord *r;
    for (r = m->ResourceRecords; r; r=r->next) if (mDNSSameOpaque16(id, r->updateid)) return mDNStrue;
    return mDNSfalse;
}

mDNSlocal mDNSBool mDNS_IdUsedInQuestionsList(mDNS * const m, mDNSOpaque16 id)
{
    DNSQuestion *q;
    for (q = m->Questions; q; q=q->next) if (mDNSSameOpaque16(id, q->TargetQID)) return mDNStrue;
    return mDNSfalse;
}

mDNSexport mDNSOpaque16 mDNS_NewMessageID(mDNS * const m)
{
    mDNSOpaque16 id;
    int i;

    for (i=0; i<10; i++)
    {
        id = mDNSOpaque16fromIntVal(1 + (mDNSu16)mDNSRandom(0xFFFE));
        if (!mDNS_IdUsedInResourceRecordsList(m, id) && !mDNS_IdUsedInQuestionsList(m, id)) break;
    }

    debugf("mDNS_NewMessageID: %5d", mDNSVal16(id));

    return id;
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Sleep Proxy Server
#endif

mDNSlocal void RestartARPProbing(mDNS *const m, AuthRecord *const rr)
{
    // If we see an ARP from a machine we think is sleeping, then either
    // (i) the machine has woken, or
    // (ii) it's just a stray old packet from before the machine slept
    // To handle the second case, we reset ProbeCount, so we'll suppress our own answers for a while, to avoid
    // generating ARP conflicts with a waking machine, and set rr->LastAPTime so we'll start probing again in 10 seconds.
    // If the machine has just woken then we'll discard our records when we see the first new mDNS probe from that machine.
    // If it was a stray old packet, then after 10 seconds we'll probe again and then start answering ARPs again. In this case we *do*
    // need to send new ARP Announcements, because the owner's ARP broadcasts will have updated neighboring ARP caches, so we need to
    // re-assert our (temporary) ownership of that IP address in order to receive subsequent packets addressed to that IPv4 address.

    rr->resrec.RecordType = kDNSRecordTypeUnique;
    rr->ProbeCount        = DefaultProbeCountForTypeUnique;
    rr->ProbeRestartCount++;

    // If we haven't started announcing yet (and we're not already in ten-second-delay mode) the machine is probably
    // still going to sleep, so we just reset rr->ProbeCount so we'll continue probing until it stops responding.
    // If we *have* started announcing, the machine is probably in the process of waking back up, so in that case
    // we're more cautious and we wait ten seconds before probing it again. We do this because while waking from
    // sleep, some network interfaces tend to lose or delay inbound packets, and without this delay, if the waking machine
    // didn't answer our three probes within three seconds then we'd announce and cause it an unnecessary address conflict.
    if (rr->AnnounceCount == InitialAnnounceCount && m->timenow - rr->LastAPTime >= 0)
        InitializeLastAPTime(m, rr);
    else
    {
        rr->AnnounceCount  = InitialAnnounceCount;
        rr->ThisAPInterval = mDNSPlatformOneSecond;
        rr->LastAPTime     = m->timenow + mDNSPlatformOneSecond * 9;    // Send first packet at rr->LastAPTime + rr->ThisAPInterval, i.e. 10 seconds from now
        SetNextAnnounceProbeTime(m, rr);
    }
}

mDNSlocal void mDNSCoreReceiveRawARP(mDNS *const m, const ARP_EthIP *const arp, const mDNSInterfaceID InterfaceID)
{
    static const mDNSOpaque16 ARP_op_request = { { 0, 1 } };
    AuthRecord *rr;
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, InterfaceID);
    if (!intf) return;

    mDNS_Lock(m);

    // Pass 1:
    // Process ARP Requests and Probes (but not Announcements), and generate an ARP Reply if necessary.
    // We also process ARPs from our own kernel (and 'answer' them by injecting a local ARP table entry)
    // We ignore ARP Announcements here -- Announcements are not questions, they're assertions, so we don't need to answer them.
    // The times we might need to react to an ARP Announcement are:
    // (i) as an indication that the host in question has not gone to sleep yet (so we should delay beginning to proxy for it) or
    // (ii) if it's a conflicting Announcement from another host
    // -- and we check for these in Pass 2 below.
    if (mDNSSameOpaque16(arp->op, ARP_op_request) && !mDNSSameIPv4Address(arp->spa, arp->tpa))
    {
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->resrec.InterfaceID == InterfaceID && rr->resrec.RecordType != kDNSRecordTypeDeregistering &&
                rr->AddressProxy.type == mDNSAddrType_IPv4 && mDNSSameIPv4Address(rr->AddressProxy.ip.v4, arp->tpa))
            {
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                static const char msg1[] = "ARP Req from owner -- re-probing";
                static const char msg2[] = "Ignoring  ARP Request from      ";
                static const char msg3[] = "Creating Local ARP Cache entry  ";
                static const char msg4[] = "Answering ARP Request from      ";
                const char *const msg = mDNSSameEthAddress(&arp->sha, &rr->WakeUp.IMAC) ? msg1 :
                                        (rr->AnnounceCount == InitialAnnounceCount)     ? msg2 :
                                        mDNSSameEthAddress(&arp->sha, &intf->MAC)       ? msg3 : msg4;
                LogSPS("%-7s %s %.6a %.4a for %.4a -- H-MAC %.6a I-MAC %.6a %s",
                       intf->ifname, msg, &arp->sha, &arp->spa, &arp->tpa, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
                if (msg == msg1)
                {
                    if ( rr->ProbeRestartCount < MAX_PROBE_RESTARTS)
                        RestartARPProbing(m, rr);
                    else
                        LogSPS("Reached maximum number of restarts for probing - %s", ARDisplayString(m,rr));
                }
                else if (msg == msg3)
                {
                    mDNSPlatformSetLocalAddressCacheEntry(m, &rr->AddressProxy, &rr->WakeUp.IMAC, InterfaceID);
                }
                else if (msg == msg4)
                {
                    SendARP(m, 2, rr, &arp->tpa, &arp->sha, &arp->spa, &arp->sha);
                }
#else 
                SendARP(m, 2, rr, &arp->tpa, &arp->sha, &arp->spa, &arp->sha);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
            }
    }

    // Pass 2:
    // For all types of ARP packet we check the Sender IP address to make sure it doesn't conflict with any AddressProxy record we're holding.
    // (Strictly speaking we're only checking Announcement/Request/Reply packets, since ARP Probes have zero Sender IP address,
    // so by definition (and by design) they can never conflict with any real (i.e. non-zero) IP address).
    // We ignore ARPs we sent ourselves (Sender MAC address is our MAC address) because our own proxy ARPs do not constitute a conflict that we need to handle.
    // If we see an apparently conflicting ARP, we check the sender hardware address:
    //   If the sender hardware address is the original owner this is benign, so we just suppress our own proxy answering for a while longer.
    //   If the sender hardware address is *not* the original owner, then this is a conflict, and we need to wake the sleeping machine to handle it.
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    if (mDNSSameEthAddress(&arp->sha, &intf->MAC))
        debugf("ARP from self for %.4a", &arp->tpa);
    else
    {
        if (!mDNSSameIPv4Address(arp->spa, zerov4Addr))
            for (rr = m->ResourceRecords; rr; rr=rr->next)
                if (rr->resrec.InterfaceID == InterfaceID && rr->resrec.RecordType != kDNSRecordTypeDeregistering &&
                    rr->AddressProxy.type == mDNSAddrType_IPv4 && mDNSSameIPv4Address(rr->AddressProxy.ip.v4, arp->spa) && (rr->ProbeRestartCount < MAX_PROBE_RESTARTS))
                {
                    if (mDNSSameEthAddress(&zeroEthAddr, &rr->WakeUp.HMAC))
                    {
                        LogSPS("%-7s ARP from %.6a %.4a for %.4a -- Invalid H-MAC %.6a I-MAC %.6a %s", intf->ifname,
                                &arp->sha, &arp->spa, &arp->tpa, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
                    }
                    else
                    {
                        RestartARPProbing(m, rr);
                        if (mDNSSameEthAddress(&arp->sha, &rr->WakeUp.IMAC))
                        {
                            LogSPS("%-7s ARP %s from owner %.6a %.4a for %-15.4a -- re-starting probing for %s", intf->ifname,
                                    mDNSSameIPv4Address(arp->spa, arp->tpa) ? "Announcement " : mDNSSameOpaque16(arp->op, ARP_op_request) ? "Request      " : "Response     ",
                                    &arp->sha, &arp->spa, &arp->tpa, ARDisplayString(m, rr));
                        }
                        else
                        {
                            LogMsg("%-7s Conflicting ARP from %.6a %.4a for %.4a -- waking H-MAC %.6a I-MAC %.6a %s", intf->ifname,
                                    &arp->sha, &arp->spa, &arp->tpa, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
                            ScheduleWakeup(m, rr->resrec.InterfaceID, &rr->WakeUp.HMAC);
                        }
                    }
                }
    }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY

    mDNS_Unlock(m);
}

/*
   // Option 1 is Source Link Layer Address Option
   // Option 2 is Target Link Layer Address Option
   mDNSlocal const mDNSEthAddr *GetLinkLayerAddressOption(const IPv6NDP *const ndp, const mDNSu8 *const end, mDNSu8 op)
    {
    const mDNSu8 *options = (mDNSu8 *)(ndp+1);
    while (options < end)
        {
        debugf("NDP Option %02X len %2d %d", options[0], options[1], end - options);
        if (options[0] == op && options[1] == 1) return (const mDNSEthAddr*)(options+2);
        options += options[1] * 8;
        }
    return mDNSNULL;
    }
 */

mDNSlocal void mDNSCoreReceiveRawND(mDNS *const m, const mDNSEthAddr *const sha, const mDNSv6Addr *spa,
                                    const IPv6NDP *const ndp, const mDNSu8 *const end, const mDNSInterfaceID InterfaceID)
{
    AuthRecord *rr;
    NetworkInterfaceInfo *intf = FirstInterfaceForID(m, InterfaceID);
    if (!intf) return;

    mDNS_Lock(m);

    // Pass 1: Process Neighbor Solicitations, and generate a Neighbor Advertisement if necessary.
    if (ndp->type == NDP_Sol)
    {
        //const mDNSEthAddr *const sha = GetLinkLayerAddressOption(ndp, end, NDP_SrcLL);
        (void)end;
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->resrec.InterfaceID == InterfaceID && rr->resrec.RecordType != kDNSRecordTypeDeregistering &&
                rr->AddressProxy.type == mDNSAddrType_IPv6 && mDNSSameIPv6Address(rr->AddressProxy.ip.v6, ndp->target))
            {
                static const char msg1[] = "NDP Req from owner -- re-probing";
                static const char msg2[] = "Ignoring  NDP Request from      ";
                static const char msg3[] = "Creating Local NDP Cache entry  ";
                static const char msg4[] = "Answering NDP Request from      ";
                static const char msg5[] = "Answering NDP Probe   from      ";
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                const char *const msg = sha && mDNSSameEthAddress(sha, &rr->WakeUp.IMAC) ? msg1 :
                                        (rr->AnnounceCount == InitialAnnounceCount)      ? msg2 :
                                        sha && mDNSSameEthAddress(sha, &intf->MAC)       ? msg3 :
                                        spa && mDNSIPv6AddressIsZero(*spa)               ? msg4 : msg5;
                LogSPS("%-7s %s %.6a %.16a for %.16a -- H-MAC %.6a I-MAC %.6a %s",
                       intf->ifname, msg, sha, spa, &ndp->target, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
#else
                const char *const msg = (rr->AnnounceCount == InitialAnnounceCount)      ? msg2 :
                                        sha && mDNSSameEthAddress(sha, &intf->MAC)       ? msg3 :
                                        spa && mDNSIPv6AddressIsZero(*spa)               ? msg4 : msg5;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                if (msg == msg1)
                {
                    if (rr->ProbeRestartCount < MAX_PROBE_RESTARTS)
                        RestartARPProbing(m, rr);
                    else
                        LogSPS("Reached maximum number of restarts for probing - %s", ARDisplayString(m,rr));
                }
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                else if (msg == msg3)
                    mDNSPlatformSetLocalAddressCacheEntry(m, &rr->AddressProxy, &rr->WakeUp.IMAC, InterfaceID);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                else if (msg == msg4) 
                    SendNDP(m, NDP_Adv, NDP_Solicited, rr, &ndp->target, mDNSNULL, spa, sha);
                else if (msg == msg5) 
                    SendNDP(m, NDP_Adv, 0, rr, &ndp->target, mDNSNULL, &AllHosts_v6, &AllHosts_v6_Eth);
            }
    }

    // Pass 2: For all types of NDP packet we check the Sender IP address to make sure it doesn't conflict with any AddressProxy record we're holding.
    if (mDNSSameEthAddress(sha, &intf->MAC))
        debugf("NDP from self for %.16a", &ndp->target);
    else
    {
        // For Neighbor Advertisements we check the Target address field, not the actual IPv6 source address.
        // When a machine has both link-local and routable IPv6 addresses, it may send NDP packets making assertions
        // about its routable IPv6 address, using its link-local address as the source address for all NDP packets.
        // Hence it is the NDP target address we care about, not the actual packet source address.
        if (ndp->type == NDP_Adv) spa = &ndp->target;
 #ifndef REMOVE_WAKEUP_FUNCTIONALITY
        if (!mDNSSameIPv6Address(*spa, zerov6Addr))
            for (rr = m->ResourceRecords; rr; rr=rr->next)
                if (rr->resrec.InterfaceID == InterfaceID && rr->resrec.RecordType != kDNSRecordTypeDeregistering &&
                    rr->AddressProxy.type == mDNSAddrType_IPv6 && mDNSSameIPv6Address(rr->AddressProxy.ip.v6, *spa) && (rr->ProbeRestartCount < MAX_PROBE_RESTARTS))
                {
                   if (mDNSSameEthAddress(&zeroEthAddr, &rr->WakeUp.HMAC))
                    {
                        LogSPS("%-7s NDP from %.6a %.16a for %.16a -- Invalid H-MAC %.6a I-MAC %.6a %s", intf->ifname,
                                    sha, spa, &ndp->target, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
                    }
                    else
                    {
                        RestartARPProbing(m, rr);
                        if (mDNSSameEthAddress(sha, &rr->WakeUp.IMAC))
                        {
                            LogSPS("%-7s NDP %s from owner %.6a %.16a for %.16a -- re-starting probing for %s", intf->ifname,
                                    ndp->type == NDP_Sol ? "Solicitation " : "Advertisement", sha, spa, &ndp->target, ARDisplayString(m, rr));
                        }
                        else
                        {
                            LogMsg("%-7s Conflicting NDP from %.6a %.16a for %.16a -- waking H-MAC %.6a I-MAC %.6a %s", intf->ifname,
                                    sha, spa, &ndp->target, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, rr));
                            ScheduleWakeup(m, rr->resrec.InterfaceID, &rr->WakeUp.HMAC);
                        }
                    }
                }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    }

    mDNS_Unlock(m);
}

mDNSlocal void mDNSCoreReceiveRawTransportPacket(mDNS *const m, const mDNSEthAddr *const sha, const mDNSAddr *const src, const mDNSAddr *const dst, const mDNSu8 protocol,
                                                 const mDNSu8 *const p, const TransportLayerPacket *const t, const mDNSu8 *const end, const mDNSInterfaceID InterfaceID, const mDNSu16 len)
{
    const mDNSIPPort port = (protocol == 0x06) ? t->tcp.dst : (protocol == 0x11) ? t->udp.dst : zeroIPPort;
    mDNSBool wake = mDNSfalse;
    mDNSBool kaWake = mDNSfalse;

    switch (protocol)
    {
        #define XX wake ? "Received" : "Ignoring", end-p
    case 0x01:  LogSPS("Ignoring %d-byte ICMP from %#a to %#a", end-p, src, dst);
        break;

#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    case 0x06:  {
        AuthRecord *kr;
        mDNSu32 seq, ack;
                    #define TH_FIN  0x01
                    #define TH_SYN  0x02
                    #define TH_RST  0x04
                    #define TH_ACK  0x10


        kr = mDNS_MatchKeepaliveInfo(m, dst, src, port, t->tcp.src, &seq, &ack);
        if (kr)
        {
            LogSPS("mDNSCoreReceiveRawTransportPacket: Found a Keepalive record from %#a:%d  to %#a:%d", src, mDNSVal16(t->tcp.src), dst, mDNSVal16(port));
            // Plan to wake if
            // (a) RST or FIN is set (the keepalive that we sent could have caused a reset)
            // (b) packet that contains new data and acks a sequence number higher than the one
            //     we have been sending in the keepalive

            wake = ((t->tcp.flags & TH_RST) || (t->tcp.flags & TH_FIN)) ;
            if (!wake)
            {
                mDNSu8 *ptr;
                mDNSu32 pseq, pack;
                mDNSBool data = mDNSfalse;
                mDNSu8 tcphlen;

                // Convert to host order
                ptr = (mDNSu8 *)&seq;
                seq = ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];

                ptr = (mDNSu8 *)&ack;
                ack = ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];

                pseq = t->tcp.seq;
                ptr = (mDNSu8 *)&pseq;
                pseq = ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];

                pack = t->tcp.ack;
                ptr = (mDNSu8 *)&pack;
                pack = ptr[0] << 24 | ptr[1] << 16 | ptr[2] << 8 | ptr[3];

                // If the other side is acking one more than our sequence number (keepalive is one
                // less than the last valid sequence sent) and it's sequence is more than what we
                // acked before
                //if (end - p - 34  - ((t->tcp.offset >> 4) * 4) > 0) data = mDNStrue;
                tcphlen = ((t->tcp.offset >> 4) * 4);
                if (end - ((mDNSu8 *)t + tcphlen) > 0) data = mDNStrue;
                wake = ((int)(pack - seq) > 0) && ((int)(pseq - ack) >= 0) && data;

                // If we got a regular keepalive on a connection that was registed with the KeepAlive API, respond with an ACK
                if ((t->tcp.flags & TH_ACK) && (data == mDNSfalse) &&
                    ((int)(ack - pseq) == 1))
                {
                    // Send an ACK;
                    mDNS_SendKeepaliveACK(m, kr);
                }
                LogSPS("mDNSCoreReceiveRawTransportPacket: End %p, hlen %d, Datalen %d, pack %u, seq %u, pseq %u, ack %u, wake %d",
                       end, tcphlen, end - ((mDNSu8 *)t + tcphlen), pack, seq, pseq, ack, wake);
            }
            else { LogSPS("mDNSCoreReceiveRawTransportPacket: waking because of RST or FIN th_flags %d", t->tcp.flags); }
            kaWake = wake;
        }
        else
        {
            // Plan to wake if
            // (a) RST is not set, AND
            // (b) packet is SYN, SYN+FIN, or plain data packet (no SYN or FIN). We won't wake for FIN alone.
            wake = (!(t->tcp.flags & TH_RST) && (t->tcp.flags & (TH_FIN|TH_SYN)) != TH_FIN);

            // For now, to reduce spurious wakeups, we wake only for TCP SYN,
            // except for ssh connections, where we'll wake for plain data packets too
            if  (!mDNSSameIPPort(port, SSHPort) && !(t->tcp.flags & 2)) wake = mDNSfalse;

            LogSPS("%s %d-byte TCP from %#a:%d to %#a:%d%s%s%s", XX,
                   src, mDNSVal16(t->tcp.src), dst, mDNSVal16(port),
                   (t->tcp.flags & 2) ? " SYN" : "",
                   (t->tcp.flags & 1) ? " FIN" : "",
                   (t->tcp.flags & 4) ? " RST" : "");
        }
        break;
    }
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
    case 0x11:  {
                    #define ARD_AsNumber 3283
        static const mDNSIPPort ARD = { { ARD_AsNumber >> 8, ARD_AsNumber & 0xFF } };
        const mDNSu16 udplen = (mDNSu16)((mDNSu16)t->bytes[4] << 8 | t->bytes[5]);                  // Length *including* 8-byte UDP header
        if (udplen >= sizeof(UDPHeader))
        {
            const mDNSu16 datalen = udplen - sizeof(UDPHeader);
            wake = mDNStrue;

            // For Back to My Mac UDP port 4500 (IPSEC) packets, we do some special handling
            if (mDNSSameIPPort(port, IPSECPort))
            {
                // Specifically ignore NAT keepalive packets
                if (datalen == 1 && end >= &t->bytes[9] && t->bytes[8] == 0xFF) wake = mDNSfalse;
                else
                {
                    // Skip over the Non-ESP Marker if present
                    const mDNSBool NonESP = (end >= &t->bytes[12] && t->bytes[8] == 0 && t->bytes[9] == 0 && t->bytes[10] == 0 && t->bytes[11] == 0);
                    const IKEHeader *const ike    = (IKEHeader *)(t + (NonESP ? 12 : 8));
                    const mDNSu16 ikelen = datalen - (NonESP ? 4 : 0);
                    if (ikelen >= sizeof(IKEHeader) && end >= ((mDNSu8 *)ike) + sizeof(IKEHeader))
                        if ((ike->Version & 0x10) == 0x10)
                        {
                            // ExchangeType ==  5 means 'Informational' <http://www.ietf.org/rfc/rfc2408.txt>
                            // ExchangeType == 34 means 'IKE_SA_INIT'   <http://www.iana.org/assignments/ikev2-parameters>
                            if (ike->ExchangeType == 5 || ike->ExchangeType == 34) wake = mDNSfalse;
                            LogSPS("%s %d-byte IKE ExchangeType %d", XX, ike->ExchangeType);
                        }
                }
            }

            // For now, because we haven't yet worked out a clean elegant way to do this, we just special-case the
            // Apple Remote Desktop port number -- we ignore all packets to UDP 3283 (the "Net Assistant" port),
            // except for Apple Remote Desktop's explicit manual wakeup packet, which looks like this:
            // UDP header (8 bytes)
            // Payload: 13 88 00 6a 41 4e 41 20 (8 bytes) ffffffffffff (6 bytes) 16xMAC (96 bytes) = 110 bytes total
            if (mDNSSameIPPort(port, ARD)) wake = (datalen >= 110 && end >= &t->bytes[10] && t->bytes[8] == 0x13 && t->bytes[9] == 0x88);

            LogSPS("%s %d-byte UDP from %#a:%d to %#a:%d", XX, src, mDNSVal16(t->udp.src), dst, mDNSVal16(port));
        }
    }
    break;

    case 0x3A:  if (&t->bytes[len] <= end)
        {
            mDNSu16 checksum = IPv6CheckSum(&src->ip.v6, &dst->ip.v6, protocol, t->bytes, len);
            if (!checksum) mDNSCoreReceiveRawND(m, sha, &src->ip.v6, &t->ndp, &t->bytes[len], InterfaceID);
            else LogInfo("IPv6CheckSum bad %04X %02X%02X from %#a to %#a", checksum, t->bytes[2], t->bytes[3], src, dst);
        }
        break;

    default:    LogSPS("Ignoring %d-byte IP packet unknown protocol %d from %#a to %#a", end-p, protocol, src, dst);
        break;
    }

    if (wake)
    {
        AuthRecord *rr, *r2;

        mDNS_Lock(m);
        for (rr = m->ResourceRecords; rr; rr=rr->next)
            if (rr->resrec.InterfaceID == InterfaceID &&
                rr->resrec.RecordType != kDNSRecordTypeDeregistering &&
                rr->AddressProxy.type && mDNSSameAddress(&rr->AddressProxy, dst))
            {
                const mDNSu8 *const tp = (protocol == 6) ? (const mDNSu8 *)"\x4_tcp" : (const mDNSu8 *)"\x4_udp";
                for (r2 = m->ResourceRecords; r2; r2=r2->next)
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    if (r2->resrec.InterfaceID == InterfaceID && mDNSSameEthAddress(&r2->WakeUp.HMAC, &rr->WakeUp.HMAC) &&
                        r2->resrec.RecordType != kDNSRecordTypeDeregistering &&
                        r2->resrec.rrtype == kDNSType_SRV && mDNSSameIPPort(r2->resrec.rdata->u.srv.port, port) &&
                        SameDomainLabel(ThirdLabel(r2->resrec.name)->c, tp))
                        break;
#else
                    if (r2->resrec.InterfaceID == InterfaceID &&
                        r2->resrec.RecordType != kDNSRecordTypeDeregistering &&
                        r2->resrec.rrtype == kDNSType_SRV && mDNSSameIPPort(r2->resrec.rdata->u.srv.port, port) &&
                        SameDomainLabel(ThirdLabel(r2->resrec.name)->c, tp))
                        break;
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                if (!r2 && mDNSSameIPPort(port, IPSECPort)) r2 = rr;    // So that we wake for BTMM IPSEC packets, even without a matching SRV record
                if (!r2 && kaWake) r2 = rr;                             // So that we wake for keepalive packets, even without a matching SRV record
                if (r2)
                {
                    LogMsg("Waking host at %s %#a H-MAC %.6a I-MAC %.6a for %s",
                           InterfaceNameForID(m, rr->resrec.InterfaceID), dst, &rr->WakeUp.HMAC, &rr->WakeUp.IMAC, ARDisplayString(m, r2));
#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                    ScheduleWakeup(m, rr->resrec.InterfaceID, &rr->WakeUp.HMAC);
#endif //#ifndef REMOVE_WAKEUP_FUNCTIONALITY
                }
                else
                    LogSPS("Sleeping host at %s %#a %.6a has no service on %#s %d",
                           InterfaceNameForID(m, rr->resrec.InterfaceID), dst, &rr->WakeUp.HMAC, tp, mDNSVal16(port));
            }
        mDNS_Unlock(m);
    }
}

mDNSexport void mDNSCoreReceiveRawPacket(mDNS *const m, const mDNSu8 *const p, const mDNSu8 *const end, const mDNSInterfaceID InterfaceID)
{
    static const mDNSOpaque16 Ethertype_ARP  = { { 0x08, 0x06 } };  // Ethertype 0x0806 = ARP
    static const mDNSOpaque16 Ethertype_IPv4 = { { 0x08, 0x00 } };  // Ethertype 0x0800 = IPv4
    static const mDNSOpaque16 Ethertype_IPv6 = { { 0x86, 0xDD } };  // Ethertype 0x86DD = IPv6
    static const mDNSOpaque16 ARP_hrd_eth    = { { 0x00, 0x01 } };  // Hardware address space (Ethernet = 1)
    static const mDNSOpaque16 ARP_pro_ip     = { { 0x08, 0x00 } };  // Protocol address space (IP = 0x0800)

    // Note: BPF guarantees that the NETWORK LAYER header will be word aligned, not the link-layer header.
    // In other words, we can safely assume that pkt below (ARP, IPv4 or IPv6) is properly word aligned,
    // but if pkt is 4-byte aligned, that necessarily means that eth CANNOT also be 4-byte aligned
    // since it points to a an address 14 bytes before pkt.
    const EthernetHeader     *const eth = (const EthernetHeader *)p;
    const NetworkLayerPacket *const pkt = (const NetworkLayerPacket *)(eth+1);
    mDNSAddr src, dst;
    #define RequiredCapLen(P) ((P)==0x01 ? 4 : (P)==0x06 ? 20 : (P)==0x11 ? 8 : (P)==0x3A ? 24 : 0)

    // Is ARP? Length must be at least 14 + 28 = 42 bytes
    if (end >= p+42 && mDNSSameOpaque16(eth->ethertype, Ethertype_ARP) && mDNSSameOpaque16(pkt->arp.hrd, ARP_hrd_eth) && mDNSSameOpaque16(pkt->arp.pro, ARP_pro_ip))
        mDNSCoreReceiveRawARP(m, &pkt->arp, InterfaceID);
    // Is IPv4 with zero fragmentation offset? Length must be at least 14 + 20 = 34 bytes
    else if (end >= p+34 && mDNSSameOpaque16(eth->ethertype, Ethertype_IPv4) && (pkt->v4.flagsfrags.b[0] & 0x1F) == 0 && pkt->v4.flagsfrags.b[1] == 0)
    {
        const mDNSu8 *const trans = p + 14 + (pkt->v4.vlen & 0xF) * 4;
        debugf("Got IPv4 %02X from %.4a to %.4a", pkt->v4.protocol, &pkt->v4.src, &pkt->v4.dst);
        src.type = mDNSAddrType_IPv4; src.ip.v4 = pkt->v4.src;
        dst.type = mDNSAddrType_IPv4; dst.ip.v4 = pkt->v4.dst;
        if (end >= trans + RequiredCapLen(pkt->v4.protocol))
            mDNSCoreReceiveRawTransportPacket(m, &eth->src, &src, &dst, pkt->v4.protocol, p, (TransportLayerPacket*)trans, end, InterfaceID, 0);
    }
    // Is IPv6? Length must be at least 14 + 28 = 42 bytes
    else if (end >= p+54 && mDNSSameOpaque16(eth->ethertype, Ethertype_IPv6))
    {
        const mDNSu8 *const trans = p + 54;
        debugf("Got IPv6  %02X from %.16a to %.16a", pkt->v6.pro, &pkt->v6.src, &pkt->v6.dst);
        src.type = mDNSAddrType_IPv6; src.ip.v6 = pkt->v6.src;
        dst.type = mDNSAddrType_IPv6; dst.ip.v6 = pkt->v6.dst;
        if (end >= trans + RequiredCapLen(pkt->v6.pro))
            mDNSCoreReceiveRawTransportPacket(m, &eth->src, &src, &dst, pkt->v6.pro, p, (TransportLayerPacket*)trans, end, InterfaceID,
                                              (mDNSu16)pkt->bytes[4] << 8 | pkt->bytes[5]);
    }
}

mDNSlocal void ConstructSleepProxyServerName(mDNS *const m, domainlabel *name)
{
    name->c[0] = (mDNSu8)mDNS_snprintf((char*)name->c+1, 62, "%d-%d-%d-%d.%d %#s",
                                       m->SPSType, m->SPSPortability, m->SPSMarginalPower, m->SPSTotalPower, m->SPSFeatureFlags, &m->nicelabel);
}

#ifndef SPC_DISABLED
mDNSlocal void SleepProxyServerCallback(mDNS *const m, ServiceRecordSet *const srs, mStatus result)
{
    if (result == mStatus_NameConflict)
        mDNS_RenameAndReregisterService(m, srs, mDNSNULL);
    else if (result == mStatus_MemFree)
    {
        if (m->SleepState)
            m->SPSState = 3;
        else
        {
            m->SPSState = (mDNSu8)(m->SPSSocket != mDNSNULL);
            if (m->SPSState)
            {
                domainlabel name;
                ConstructSleepProxyServerName(m, &name);
                mDNS_RegisterService(m, srs,
                                     &name, &SleepProxyServiceType, &localdomain,
                                     mDNSNULL, m->SPSSocket->port, // Host, port
                                     (mDNSu8 *)"", 1,           // TXT data, length
                                     mDNSNULL, 0,               // Subtypes (none)
                                     mDNSInterface_Any,         // Interface ID
                                     SleepProxyServerCallback, mDNSNULL, 0); // Callback, context, flags
            }
            LogSPS("Sleep Proxy Server %#s %s", srs->RR_SRV.resrec.name->c, m->SPSState ? "started" : "stopped");
        }
    }
}
#endif
#ifndef REMOVE_MACHINE_SLEEP
// Called with lock held
mDNSexport void mDNSCoreBeSleepProxyServer_internal(mDNS *const m, mDNSu8 sps, mDNSu8 port, mDNSu8 marginalpower, mDNSu8 totpower, mDNSu8 features)
{
    // This routine uses mDNS_DeregisterService and calls SleepProxyServerCallback, so we execute in user callback context
    mDNS_DropLockBeforeCallback();

    // If turning off SPS, close our socket
    // (Do this first, BEFORE calling mDNS_DeregisterService below)
    if (!sps && m->SPSSocket) { mDNSPlatformUDPClose(m->SPSSocket); m->SPSSocket = mDNSNULL; }

    // If turning off, or changing type, deregister old name
#ifndef SPC_DISABLED
    if (m->SPSState == 1 && sps != m->SPSType)
    { m->SPSState = 2; mDNS_DeregisterService_drt(m, &m->SPSRecords, sps ? mDNS_Dereg_rapid : mDNS_Dereg_normal); }
#endif // SPC_DISABLED

    // Record our new SPS parameters
    m->SPSType          = sps;
    m->SPSPortability   = port;
    m->SPSMarginalPower = marginalpower;
    m->SPSTotalPower    = totpower;
    m->SPSFeatureFlags  = features;
    // If turning on, open socket and advertise service
    if (sps)
    {
        if (!m->SPSSocket)
        {
            m->SPSSocket = mDNSPlatformUDPSocket(m, zeroIPPort);
            if (!m->SPSSocket) { LogMsg("mDNSCoreBeSleepProxyServer: Failed to allocate SPSSocket"); goto fail; }
        }
#ifndef SPC_DISABLED
        if (m->SPSState == 0) SleepProxyServerCallback(m, &m->SPSRecords, mStatus_MemFree);
#endif // SPC_DISABLED
    }
    else if (m->SPSState)
    {
        LogSPS("mDNSCoreBeSleepProxyServer turning off from state %d; will wake clients", m->SPSState);
        m->NextScheduledSPS = m->timenow;
    }
fail:
    mDNS_ReclaimLockAfterCallback();
}
#endif //#ifndef REMOVE_MACHINE_SLEEP
// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Startup and Shutdown
#endif

mDNSlocal void mDNS_GrowCache_internal(mDNS *const m, CacheEntity *storage, mDNSu32 numrecords)
{
    if (storage && numrecords)
    {
        mDNSu32 i;
        debugf("Adding cache storage for %d more records (%d bytes)", numrecords, numrecords*sizeof(CacheEntity));
        for (i=0; i<numrecords; i++) storage[i].next = &storage[i+1];
        storage[numrecords-1].next = m->rrcache_free;
        m->rrcache_free = storage;
        m->rrcache_size += numrecords;
    }
}

mDNSexport void mDNS_GrowCache(mDNS *const m, CacheEntity *storage, mDNSu32 numrecords)
{
    mDNS_Lock(m);
    mDNS_GrowCache_internal(m, storage, numrecords);
    mDNS_Unlock(m);
}

mDNSexport mStatus mDNS_Init(mDNS *const m, mDNS_PlatformSupport *const p,
                             CacheEntity *rrcachestorage, mDNSu32 rrcachesize,
                             mDNSBool AdvertiseLocalAddresses, mDNSCallback *Callback, void *Context)
{
    mDNSu32 slot;
    mDNSs32 timenow;
    mStatus result;

    if (!rrcachestorage) rrcachesize = 0;

    m->p                             = p;
    m->CanReceiveUnicastOn5353       = mDNSfalse; // Assume we can't receive unicasts on 5353, unless platform layer tells us otherwise
    m->AdvertiseLocalAddresses       = AdvertiseLocalAddresses;
    m->DivertMulticastAdvertisements = mDNSfalse;
    m->mDNSPlatformStatus            = mStatus_Waiting;
    m->UnicastPort4                  = zeroIPPort;
    m->UnicastPort6                  = zeroIPPort;
    m->PrimaryMAC                    = zeroEthAddr;
    m->MainCallback                  = Callback;
    m->MainContext                   = Context;
    m->rec.r.resrec.RecordType       = 0;
    m->rec.r.resrec.AnonInfo         = mDNSNULL;

    // For debugging: To catch and report locking failures
    m->mDNS_busy               = 0;
    m->mDNS_reentrancy         = 0;
    m->ShutdownTime            = 0;
    m->lock_rrcache            = 0;
    m->lock_Questions          = 0;
    m->lock_Records            = 0;

    // Task Scheduling variables
    result = mDNSPlatformTimeInit();
    if (result != mStatus_NoError) return(result);
    m->timenow_adjust = (mDNSs32)mDNSRandom(0xFFFFFFFF);
    timenow = mDNS_TimeNow_NoLock(m);

    m->timenow                 = 0;     // MUST only be set within mDNS_Lock/mDNS_Unlock section
    m->timenow_last            = timenow;
    m->NextScheduledEvent      = timenow;
    m->SuppressSending         = timenow;
    m->NextCacheCheck          = timenow + 0x78000000;
    m->NextScheduledQuery      = timenow + 0x78000000;
    m->NextScheduledProbe      = timenow + 0x78000000;
    m->NextScheduledResponse   = timenow + 0x78000000;
#ifndef REMOVE_NAT_FUCTIONALITY
    m->NextScheduledNATOp      = timenow + 0x78000000;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    m->NextScheduledSPS        = timenow + 0x78000000;
    m->NextScheduledKA         = timenow + 0x78000000;
    m->NextScheduledStopTime   = timenow + 0x78000000;
    m->RandomQueryDelay        = 0;
    m->RandomReconfirmDelay    = 0;
    m->PktNum                  = 0;
    m->MPktNum                 = 0;
    m->LocalRemoveEvents       = mDNSfalse;
    m->SleepState              = SleepState_Awake;
    m->SleepSeqNum             = 0;
    m->SystemWakeOnLANEnabled  = mDNSfalse;
    m->AnnounceOwner           = NonZeroTime(timenow + 60 * mDNSPlatformOneSecond);
    m->DelaySleep              = 0;
    m->SleepLimit              = 0;

#if APPLE_OSX_mDNSResponder
    m->StatStartTime           = mDNSPlatformUTC();
    m->NextStatLogTime         = m->StatStartTime + kDefaultNextStatsticsLogTime;
    m->ActiveStatTime          = 0;
    m->UnicastPacketsSent      = 0;
    m->MulticastPacketsSent    = 0;
    m->RemoteSubnet            = 0;
#endif // APPLE_OSX_mDNSResponder

    // These fields only required for mDNS Searcher...
    m->Questions               = mDNSNULL;
    m->NewQuestions            = mDNSNULL;
    m->CurrentQuestion         = mDNSNULL;
    m->LocalOnlyQuestions      = mDNSNULL;
    m->NewLocalOnlyQuestions   = mDNSNULL;
    m->RestartQuestion         = mDNSNULL;
    m->ValidationQuestion      = mDNSNULL;
    m->rrcache_size            = 0;
    m->rrcache_totalused       = 0;
    m->rrcache_active          = 0;
    m->rrcache_report          = 10;
    m->rrcache_free            = mDNSNULL;

    for (slot = 0; slot < CACHE_HASH_SLOTS; slot++)
    {
        m->rrcache_hash[slot]      = mDNSNULL;
        m->rrcache_nextcheck[slot] = timenow + 0x78000000;;
    }

    mDNS_GrowCache_internal(m, rrcachestorage, rrcachesize);
    m->rrauth.rrauth_free            = mDNSNULL;

    for (slot = 0; slot < AUTH_HASH_SLOTS; slot++)
        m->rrauth.rrauth_hash[slot] = mDNSNULL;

    // Fields below only required for mDNS Responder...
    m->hostlabel.c[0]          = 0;
    m->nicelabel.c[0]          = 0;
    m->MulticastHostname.c[0]  = 0;
    m->HIHardware.c[0]         = 0;
    m->HISoftware.c[0]         = 0;
    m->ResourceRecords         = mDNSNULL;
    m->DuplicateRecords        = mDNSNULL;
    m->NewLocalRecords         = mDNSNULL;
    m->NewLocalOnlyRecords     = mDNSfalse;
    m->CurrentRecord           = mDNSNULL;
    m->HostInterfaces          = mDNSNULL;
    m->ProbeFailTime           = 0;
    m->NumFailedProbes         = 0;
    m->SuppressProbes          = 0;

#ifndef UNICAST_DISABLED
    m->NextuDNSEvent            = timenow + 0x78000000;
    m->NextSRVUpdate            = timenow + 0x78000000;

    m->DNSServers               = mDNSNULL;

    m->Router                   = zeroAddr;
    m->AdvertisedV4             = zeroAddr;
    m->AdvertisedV6             = zeroAddr;

    m->AuthInfoList             = mDNSNULL;

    m->ReverseMap.ThisQInterval = -1;
    m->StaticHostname.c[0]      = 0;
    m->FQDN.c[0]                = 0;
    m->Hostnames                = mDNSNULL;
    m->AutoTunnelNAT.clientContext = mDNSNULL;

    m->WABBrowseQueriesCount    = 0;
    m->WABLBrowseQueriesCount   = 0;
    m->WABRegQueriesCount       = 0;
#if !TARGET_OS_EMBEDDED
    m->mDNSOppCaching           = mDNStrue;
#else
    m->mDNSOppCaching           = mDNSfalse;
#endif
    m->AutoTargetServices       = 0;
#ifndef REMOVE_NAT_FUCTIONALITY
    // NAT traversal fields
    m->LLQNAT.clientCallback    = mDNSNULL;
    m->LLQNAT.clientContext     = mDNSNULL;
    m->NATTraversals            = mDNSNULL;
    m->CurrentNATTraversal      = mDNSNULL;
    m->retryIntervalGetAddr     = 0;    // delta between time sent and retry
    m->retryGetAddr             = timenow + 0x78000000; // absolute time when we retry
    m->ExtAddress               = zerov4Addr;
    m->PCPNonce[0]              = mDNSRandom(-1);
    m->PCPNonce[1]              = mDNSRandom(-1);
    m->PCPNonce[2]              = mDNSRandom(-1);

    m->NATMcastRecvskt          = mDNSNULL;
    m->LastNATupseconds         = 0;
    m->LastNATReplyLocalTime    = timenow;
    m->LastNATMapResultCode     = NATErr_None;
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
    m->UPnPInterfaceID          = 0;
    m->SSDPSocket               = mDNSNULL;
    m->SSDPWANPPPConnection     = mDNSfalse;
    m->UPnPRouterPort           = zeroIPPort;
    m->UPnPSOAPPort             = zeroIPPort;
    m->UPnPRouterURL            = mDNSNULL;
    m->UPnPWANPPPConnection     = mDNSfalse;
    m->UPnPSOAPURL              = mDNSNULL;
    m->UPnPRouterAddressString  = mDNSNULL;
    m->UPnPSOAPAddressString    = mDNSNULL;
    m->SPSType                  = 0;
    m->SPSPortability           = 0;
    m->SPSMarginalPower         = 0;
    m->SPSTotalPower            = 0;
    m->SPSFeatureFlags          = 0;
    m->SPSState                 = 0;
    m->SPSProxyListChanged      = mDNSNULL;
    m->SPSSocket                = mDNSNULL;
    m->SPSBrowseCallback        = mDNSNULL;
    m->ProxyRecords             = 0;

#endif //#ifndef REMOVE_NAT_FUCTIONALITY

#if APPLE_OSX_mDNSResponder
    m->TunnelClients            = mDNSNULL;

#if !NO_WCF
    CHECK_WCF_FUNCTION(WCFConnectionNew)
    {
        m->WCF = WCFConnectionNew();
        if (!m->WCF) { LogMsg("WCFConnectionNew failed"); return -1; }
    }
#endif

#endif

    result = mDNSPlatformInit(m);

#ifndef UNICAST_DISABLED
    // It's better to do this *after* the platform layer has set up the
    // interface list and security credentials
    uDNS_SetupDNSConfig(m);                     // Get initial DNS configuration
#endif

    return(result);
}

mDNSexport void mDNS_ConfigChanged(mDNS *const m)
{
    if (m->SPSState == 1)
    {
        domainlabel name, newname;
#ifndef SPC_DISABLED
        domainname type, domain;
        DeconstructServiceName(m->SPSRecords.RR_SRV.resrec.name, &name, &type, &domain);
#endif // SPC_DISABLED
        ConstructSleepProxyServerName(m, &newname);
        if (!SameDomainLabelCS(name.c, newname.c))
        {
            LogSPS("Renaming SPS from %s to %s", name.c, newname.c);
            // When SleepProxyServerCallback gets the mStatus_MemFree message,
            // it will reregister the service under the new name
            m->SPSState = 2;
#ifndef SPC_DISABLED
            mDNS_DeregisterService_drt(m, &m->SPSRecords, mDNS_Dereg_rapid);
#endif // SPC_DISABLED
        }
    }

    if (m->MainCallback)
        m->MainCallback(m, mStatus_ConfigChanged);
}

mDNSlocal void DynDNSHostNameCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    (void)m;    // unused
    debugf("NameStatusCallback: result %d for registration of name %##s", result, rr->resrec.name->c);
    mDNSPlatformDynDNSHostNameStatusChanged(rr->resrec.name, result);
}

mDNSlocal void PurgeOrReconfirmCacheRecord(mDNS *const m, CacheRecord *cr, const DNSServer * const ptr, mDNSBool lameduck)
{
    mDNSBool purge = cr->resrec.RecordType == kDNSRecordTypePacketNegative ||
                     cr->resrec.rrtype     == kDNSType_A ||
                     cr->resrec.rrtype     == kDNSType_AAAA ||
                     cr->resrec.rrtype     == kDNSType_SRV;

    (void) lameduck;
    (void) ptr;
    debugf("PurgeOrReconfirmCacheRecord: %s cache record due to %s server %p %#a:%d (%##s): %s",
           purge    ? "purging"   : "reconfirming",
           lameduck ? "lame duck" : "new",
           ptr, &ptr->addr, mDNSVal16(ptr->port), ptr->domain.c, CRDisplayString(m, cr));

    if (purge)
    {
        LogInfo("PurgeorReconfirmCacheRecord: Purging Resourcerecord %s, RecordType %x", CRDisplayString(m, cr), cr->resrec.RecordType);
        mDNS_PurgeCacheResourceRecord(m, cr);
    }
    else
    {
        LogInfo("PurgeorReconfirmCacheRecord: Reconfirming Resourcerecord %s, RecordType %x", CRDisplayString(m, cr), cr->resrec.RecordType);
        mDNS_Reconfirm_internal(m, cr, kDefaultReconfirmTimeForNoAnswer);
    }
}

mDNSlocal void mDNS_PurgeForQuestion(mDNS *const m, DNSQuestion *q)
{
    const mDNSu32 slot = HashSlot(&q->qname);
    CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
    CacheRecord *rp;
    mDNSu8 validatingResponse = 0;

    // For DNSSEC questions, purge the corresponding RRSIGs also.
    if (DNSSECQuestion(q))
    {
        validatingResponse = q->ValidatingResponse;
        q->ValidatingResponse = mDNStrue;
    }
    for (rp = cg ? cg->members : mDNSNULL; rp; rp = rp->next)
    {
        if (SameNameRecordAnswersQuestion(&rp->resrec, q))
        {
            LogInfo("mDNS_PurgeForQuestion: Flushing %s", CRDisplayString(m, rp));
            mDNS_PurgeCacheResourceRecord(m, rp);
        }
    }
    if (DNSSECQuestion(q))
    {
        q->ValidatingResponse = validatingResponse;
    }
}

// For DNSSEC question, we need the DNSSEC records also. If the cache does not
// have the DNSSEC records, we need to re-issue the question with EDNS0/DO bit set.
// Just re-issuing the question for RRSIGs does not work in practice as the response
// may not contain the RRSIGs whose typeCovered field matches the question's qtype.
//
// For negative responses, we need the NSECs to prove the non-existence. If we don't
// have the cached NSECs, purge them. For positive responses, if we don't have the
// RRSIGs and if we have not already issued the question with EDNS0/DO bit set, purge
// them.
mDNSlocal void CheckForDNSSECRecords(mDNS *const m, DNSQuestion *q)
{
    const mDNSu32 slot = HashSlot(&q->qname);
    CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
    CacheRecord *rp;

    for (rp = cg ? cg->members : mDNSNULL; rp; rp = rp->next)
    {
        if (SameNameRecordAnswersQuestion(&rp->resrec, q))
        {
            if (rp->resrec.RecordType != kDNSRecordTypePacketNegative || !rp->nsec)
            {
                if (!rp->CRDNSSECQuestion)
                {
                    LogInfo("CheckForDNSSECRecords: Flushing %s", CRDisplayString(m, rp));
                    mDNS_PurgeCacheResourceRecord(m, rp);
                }
            }
        }
    }
}

// Check for a positive unicast response to the question but with qtype
mDNSexport mDNSBool mDNS_CheckForCacheRecord(mDNS *const m, DNSQuestion *q, mDNSu16 qtype)
{
    DNSQuestion question;
    const mDNSu32 slot = HashSlot(&q->qname);
    CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);
    CacheRecord *rp;

    // Create an identical question but with qtype
    mDNS_SetupQuestion(&question, q->InterfaceID, &q->qname, qtype, mDNSNULL, mDNSNULL);
    question.qDNSServer = q->qDNSServer;

    for (rp = cg ? cg->members : mDNSNULL; rp; rp = rp->next)
    {
        if (!rp->resrec.InterfaceID && rp->resrec.RecordType != kDNSRecordTypePacketNegative &&
            SameNameRecordAnswersQuestion(&rp->resrec, &question))
        {
            LogInfo("mDNS_CheckForCacheRecord: Found %s", CRDisplayString(m, rp));
            return mDNStrue;
        }
    }
    return mDNSfalse;
}

mDNSexport void DNSServerChangeForQuestion(mDNS *const m, DNSQuestion *q, DNSServer *new)
{
    DNSQuestion *qptr;

    (void) m;

    if (q->DuplicateOf)
        LogMsg("DNSServerChangeForQuestion: ERROR: Called for duplicate question %##s", q->qname.c);

    // Make sure all the duplicate questions point to the same DNSServer so that delivery
    // of events for all of them are consistent. Duplicates for a question are always inserted
    // after in the list.
    q->qDNSServer = new;
    for (qptr = q->next ; qptr; qptr = qptr->next)
    {
        if (qptr->DuplicateOf == q) { qptr->validDNSServers = q->validDNSServers; qptr->qDNSServer = new; }
    }
}

mDNSlocal void SetConfigState(mDNS *const m, mDNSBool delete)
{
    McastResolver *mr;
    DNSServer *ptr;

    if (delete)
    {
        for (ptr = m->DNSServers; ptr; ptr = ptr->next)
        {
            ptr->penaltyTime = 0;
            NumUnicastDNSServers--;
            ptr->flags |= DNSServer_FlagDelete;
        }
        // We handle the mcast resolvers here itself as mDNSPlatformSetDNSConfig looks at
        // mcast resolvers. Today we get both mcast and ucast configuration using the same
        // API
        for (mr = m->McastResolvers; mr; mr = mr->next)
            mr->flags |= McastResolver_FlagDelete;
    }
    else
    {
        for (ptr = m->DNSServers; ptr; ptr = ptr->next)
        {
            ptr->penaltyTime = 0;
            NumUnicastDNSServers++;
            ptr->flags &= ~DNSServer_FlagDelete;
        }
        for (mr = m->McastResolvers; mr; mr = mr->next)
            mr->flags &= ~McastResolver_FlagDelete;
    }
}

mDNSexport mStatus uDNS_SetupDNSConfig(mDNS *const m)
{
    mDNSu32 slot;
    CacheGroup *cg;
    CacheRecord *cr;
    mDNSBool Restart = mDNSfalse;
    mDNSAddr v4, v6, r;
    domainname fqdn;
    DNSServer   *ptr, **p = &m->DNSServers;
    const DNSServer *oldServers = m->DNSServers;
    DNSQuestion *q;
    McastResolver *mr, **mres = &m->McastResolvers;

    debugf("uDNS_SetupDNSConfig: entry");

    // Let the platform layer get the current DNS information and setup the WAB queries if needed.
    uDNS_SetupWABQueries(m);

    mDNS_Lock(m);

    // We need to first mark all the entries to be deleted. If the configuration changed, then
    // the entries would be undeleted appropriately. Otherwise, we need to clear them.
    //
    // Note: The last argument to mDNSPlatformSetDNSConfig is "mDNStrue" which means ack the
    // configuration. We already processed search domains in uDNS_SetupWABQueries above and
    // hence we are ready to ack the configuration as this is the last call to mDNSPlatformSetConfig
    // for the dns configuration change notification.
    SetConfigState(m, mDNStrue);
    if (!mDNSPlatformSetDNSConfig(m, mDNStrue, mDNSfalse, &fqdn, mDNSNULL, mDNSNULL, mDNStrue))
    {
        SetConfigState(m, mDNSfalse);
        mDNS_Unlock(m);
        LogInfo("uDNS_SetupDNSConfig: No configuration change");
        return mStatus_NoError;
    }

    // For now, we just delete the mcast resolvers. We don't deal with cache or
    // questions here. Neither question nor cache point to mcast resolvers. Questions
    // do inherit the timeout values from mcast resolvers. But we don't bother
    // affecting them as they never change.
    while (*mres)
    {
        if (((*mres)->flags & DNSServer_FlagDelete) != 0)
        {
            mr = *mres;
            *mres = (*mres)->next;
            debugf("uDNS_SetupDNSConfig: Deleting mcast resolver %##s", mr, mr->domain.c);
            mDNSPlatformMemFree(mr);
        }
        else
        {
            (*mres)->flags &= ~McastResolver_FlagNew;
            mres = &(*mres)->next;
        }
    }

    // Update our qDNSServer pointers before we go and free the DNSServer object memory
    //
    // All non-scoped resolvers share the same resGroupID. At no point in time a cache entry using DNSServer
    // from scoped resolver will be used to answer non-scoped questions and vice versa, as scoped and non-scoped
    // resolvers don't share the same resGroupID. A few examples to describe the interaction with how we pick
    // DNSServers and flush the cache.
    //
    // - A non-scoped question picks DNSServer X, creates a cache entry with X. If a new resolver gets added later that
    //   is a better match, we pick the new DNSServer for the question and activate the unicast query. We may or may not
    //   flush the cache (See PurgeOrReconfirmCacheRecord). In either case, we don't change the cache record's DNSServer
    //   pointer immediately (qDNSServer and rDNSServer may be different but still share the same resGroupID). If we don't
    //   flush the cache immediately, the record's rDNSServer pointer will be updated (in mDNSCoreReceiveResponse)
    //   later when we get the response. If we purge the cache, we still deliver a RMV when it is purged even though
    //   we don't update the cache record's DNSServer pointer to match the question's DNSSever, as they both point to
    //   the same resGroupID.
    //
    //   Note: If the new DNSServer comes back with a different response than what we have in the cache, we will deliver a RMV
    //   of the old followed by ADD of the new records.
    //
    // - A non-scoped question picks DNSServer X,  creates a cache entry with X. If the resolver gets removed later, we will
    //   pick a new DNSServer for the question which may or may not be NULL and set the cache record's pointer to the same
    //   as in question's qDNSServer if the cache record is not flushed. If there is no active question, it will be set to NULL.
    //
    // - Two questions scoped and non-scoped for the same name will pick two different DNSServer and will end up creating separate
    //   cache records and as the resGroupID is different, you can't use the cache record from the scoped DNSServer to answer the
    //   non-scoped question and vice versa.
    //
    for (q = m->Questions; q; q=q->next)
    {
        if (!mDNSOpaque16IsZero(q->TargetQID))
        {
            DNSServer *s, *t;
            DNSQuestion *qptr;
            if (q->DuplicateOf) continue;
            SetValidDNSServers(m, q);
            q->triedAllServersOnce = 0;
            s = GetServerForQuestion(m, q);
            t = q->qDNSServer;
            if (t != s)
            {
                mDNSBool old, new;
                // If DNS Server for this question has changed, reactivate it
                LogInfo("uDNS_SetupDNSConfig: Updating DNS Server from %#a:%d (%##s) to %#a:%d (%##s) for question %##s (%s) (scope:%p)",
                        t ? &t->addr : mDNSNULL, mDNSVal16(t ? t->port : zeroIPPort), t ? t->domain.c : (mDNSu8*)"",
                        s ? &s->addr : mDNSNULL, mDNSVal16(s ? s->port : zeroIPPort), s ? s->domain.c : (mDNSu8*)"",
                        q->qname.c, DNSTypeName(q->qtype), q->InterfaceID);

                old = q->SuppressQuery;
                new = ShouldSuppressUnicastQuery(m, q, s);
                if (old != new)
                {
                    // Changing the DNS server affected the SuppressQuery status. We need to
                    // deliver RMVs for the previous ADDs (if any) before switching to the new
                    // DNSServer.  To keep it simple, we walk all the questions and mark them
                    // to be restarted and then handle all of them at once.
                    q->Restart = 1;
                    q->SuppressQuery = new;
                    for (qptr = q->next ; qptr; qptr = qptr->next)
                    {
                        if (qptr->DuplicateOf == q)
                            qptr->Restart = 1;
                    }
                    Restart = mDNStrue;
                }
                else
                {
                    DNSServerChangeForQuestion(m, q, s);
                    q->unansweredQueries = 0;

                    // If we had sent a query out to DNSServer "t" and we are changing to "s", we
                    // need to ignore the responses coming back from "t" as the DNS configuration
                    // has changed e.g., when a new interface is coming up and that becomes the primary
                    // interface, we switch to the DNS servers configured for the primary interface. In
                    // this case, we should not accept responses associated with the previous interface as
                    // the "name" could resolve differently on this new primary interface. Hence, discard
                    // in-flight responses.
                    q->TargetQID = mDNS_NewMessageID(m);

                    if (!QuerySuppressed(q))
                    {
                        debugf("uDNS_SetupDNSConfig: Activating query %p %##s (%s)", q, q->qname.c, DNSTypeName(q->qtype));
                        ActivateUnicastQuery(m, q, mDNStrue);
                        // ActivateUnicastQuery is called for duplicate questions also as it does something
                        // special for AutoTunnel questions
                        for (qptr = q->next ; qptr; qptr = qptr->next)
                        {
                            if (qptr->DuplicateOf == q) ActivateUnicastQuery(m, qptr, mDNStrue);
                        }
                    }
                }
            }
            else
            {
                debugf("uDNS_SetupDNSConfig: Not Updating DNS server question %p %##s (%s) DNS server %#a:%d %p %d",
                       q, q->qname.c, DNSTypeName(q->qtype), t ? &t->addr : mDNSNULL, mDNSVal16(t ? t->port : zeroIPPort), q->DuplicateOf, q->SuppressUnusable);
                for (qptr = q->next ; qptr; qptr = qptr->next)
                    if (qptr->DuplicateOf == q) { qptr->validDNSServers = q->validDNSServers; qptr->qDNSServer = q->qDNSServer; }
            }
        }
    }
    if (Restart)
        RestartUnicastQuestions(m);

    FORALL_CACHERECORDS(slot, cg, cr)
    {
        if (cr->resrec.InterfaceID)
            continue;

        // We already walked the questions and restarted/reactivated them if the dns server
        // change affected the question. That should take care of updating the cache. But
        // what if there is no active question at this point when the DNS server change
        // happened ? There could be old cache entries lying around and if we don't flush
        // them, a new question after the DNS server change could pick up these stale
        // entries and get a wrong answer.
        //
        // For cache entries that have active questions we might have skipped rescheduling
        // the questions if they were suppressed (see above). To keep it simple, we walk
        // all the cache entries to make sure that there are no stale entries. We use the
        // active question's InterfaceID/ServiceID for looking up the right DNS server.
        // Note that the unscoped value for ServiceID is -1.
        //
        // Note: If GetServerForName returns NULL, it could either mean that there are no
        // DNS servers or no matching DNS servers for this question. In either case,
        // the cache should get purged below when we process deleted DNS servers.

        ptr = GetServerForName(m, cr->resrec.name,
                               (cr->CRActiveQuestion ? cr->CRActiveQuestion->InterfaceID : mDNSNULL),
                               (cr->CRActiveQuestion ? cr->CRActiveQuestion->ServiceID   : -1));

        // Purge or Reconfirm if this cache entry would use the new DNS server
        if (ptr && (ptr != cr->resrec.rDNSServer))
        {
            // As the DNSServers for this cache record is not the same anymore, we don't
            // want any new questions to pick this old value. If there is no active question,
            // we can't possibly re-confirm, so purge in that case. If it is a DNSSEC question,
            // purge the cache as the DNSSEC capabilities of the DNS server may have changed.

            if (cr->CRActiveQuestion == mDNSNULL || DNSSECQuestion(cr->CRActiveQuestion))
            {
                LogInfo("uDNS_SetupDNSConfig: Purging Resourcerecord %s, New DNS server %#a , Old DNS server %#a", CRDisplayString(m, cr),
                        &ptr->addr, (cr->resrec.rDNSServer != mDNSNULL ?  &cr->resrec.rDNSServer->addr : mDNSNULL));
                mDNS_PurgeCacheResourceRecord(m, cr);
            }
            else
            {
                LogInfo("uDNS_SetupDNSConfig: Purging/Reconfirming Resourcerecord %s, New DNS server %#a, Old DNS server %#a", CRDisplayString(m, cr),
                        &ptr->addr, (cr->resrec.rDNSServer != mDNSNULL ?  &cr->resrec.rDNSServer->addr : mDNSNULL));
                PurgeOrReconfirmCacheRecord(m, cr, ptr, mDNSfalse);
            }
        }
    }

    while (*p)
    {
        if (((*p)->flags & DNSServer_FlagDelete) != 0)
        {
            // Scan our cache, looking for uDNS records that we would have queried this server for.
            // We reconfirm any records that match, because in this world of split DNS, firewalls, etc.
            // different DNS servers can give different answers to the same question.
            ptr = *p;
            FORALL_CACHERECORDS(slot, cg, cr)
            {
                if (cr->resrec.InterfaceID) continue;
                if (cr->resrec.rDNSServer == ptr)
                {
                    // If we don't have an active question for this cache record, neither Purge can
                    // generate RMV events nor Reconfirm can send queries out. Just set the DNSServer
                    // pointer on the record NULL so that we don't point to freed memory (We might dereference
                    // DNSServer pointers from resource record for logging purposes).
                    //
                    // If there is an active question, point to its DNSServer as long as it does not point to the
                    // freed one. We already went through the questions above and made them point at either the
                    // new server or NULL if there is no server.

                    if (cr->CRActiveQuestion)
                    {
                        DNSQuestion *qptr = cr->CRActiveQuestion;

                        if (qptr->qDNSServer == ptr)
                        {
                            LogMsg("uDNS_SetupDNSConfig: ERROR!! Cache Record %s  Active question %##s (%s) (scope:%p) poining to DNSServer Address %#a"
                                   " to be freed", CRDisplayString(m, cr),  qptr->qname.c, DNSTypeName(qptr->qtype), qptr->InterfaceID, &ptr->addr);
                            qptr->validDNSServers = zeroOpaque64;
                            qptr->qDNSServer = mDNSNULL;
                            cr->resrec.rDNSServer = mDNSNULL;
                        }
                        else
                        {
                            LogInfo("uDNS_SetupDNSConfig: Cache Record %s,  Active question %##s (%s) (scope:%p), pointing to DNSServer %#a (to be deleted),"
                                    " resetting to  question's DNSServer Address %#a", CRDisplayString(m, cr),  qptr->qname.c, DNSTypeName(qptr->qtype),
                                    qptr->InterfaceID, &ptr->addr, (qptr->qDNSServer ? &qptr->qDNSServer->addr : mDNSNULL));
                            cr->resrec.rDNSServer = qptr->qDNSServer;
                        }
                    }
                    else
                    {
                        LogInfo("uDNS_SetupDNSConfig: Cache Record %##s has no Active question, Record's DNSServer Address %#a, Server to be deleted %#a",
                                cr->resrec.name, &cr->resrec.rDNSServer->addr, &ptr->addr);
                        cr->resrec.rDNSServer = mDNSNULL;
                    }

                    PurgeOrReconfirmCacheRecord(m, cr, ptr, mDNStrue);
                }
            }
            *p = (*p)->next;
            LogInfo("uDNS_SetupDNSConfig: Deleting server %p %#a:%d (%##s) %d", ptr, &ptr->addr, mDNSVal16(ptr->port), ptr->domain.c, NumUnicastDNSServers);
            mDNSPlatformMemFree(ptr);
        }
        else
        {
            (*p)->flags &= ~DNSServer_FlagNew;
            p = &(*p)->next;
        }
    }

    // If we now have no DNS servers at all and we used to have some, then immediately purge all unicast cache records (including for LLQs).
    // This is important for giving prompt remove events when the user disconnects the Ethernet cable or turns off wireless.
    // Otherwise, stale data lingers for 5-10 seconds, which is not the user-experience people expect from Bonjour.
    // Similarly, if we now have some DNS servers and we used to have none, we want to purge any fake negative results we may have generated.
    if ((m->DNSServers != mDNSNULL) != (oldServers != mDNSNULL))
    {
        int count = 0;
        FORALL_CACHERECORDS(slot, cg, cr)
        {
            if (!cr->resrec.InterfaceID)
            {
                mDNS_PurgeCacheResourceRecord(m, cr);
                count++;
            }
        }
        LogInfo("uDNS_SetupDNSConfig: %s available; purged %d unicast DNS records from cache",
                m->DNSServers ? "DNS server became" : "No DNS servers", count);

        // Force anything that needs to get zone data to get that information again
        RestartRecordGetZoneData(m);
    }

    // Did our FQDN change?
    if (!SameDomainName(&fqdn, &m->FQDN))
    {
        if (m->FQDN.c[0]) mDNS_RemoveDynDNSHostName(m, &m->FQDN);

        AssignDomainName(&m->FQDN, &fqdn);

        if (m->FQDN.c[0])
        {
            mDNSPlatformDynDNSHostNameStatusChanged(&m->FQDN, 1);
            mDNS_AddDynDNSHostName(m, &m->FQDN, DynDNSHostNameCallback, mDNSNULL);
        }
    }

    mDNS_Unlock(m);

    // handle router and primary interface changes
    v4 = v6 = r = zeroAddr;
    v4.type = r.type = mDNSAddrType_IPv4;

    if (mDNSPlatformGetPrimaryInterface(m, &v4, &v6, &r) == mStatus_NoError && !mDNSv4AddressIsLinkLocal(&v4.ip.v4))
    {
        mDNS_SetPrimaryInterfaceInfo(m,
                                     !mDNSIPv4AddressIsZero(v4.ip.v4) ? &v4 : mDNSNULL,
                                     !mDNSIPv6AddressIsZero(v6.ip.v6) ? &v6 : mDNSNULL,
                                     !mDNSIPv4AddressIsZero(r.ip.v4) ? &r  : mDNSNULL);
    }
    else
    {
        mDNS_SetPrimaryInterfaceInfo(m, mDNSNULL, mDNSNULL, mDNSNULL);
        if (m->FQDN.c[0]) mDNSPlatformDynDNSHostNameStatusChanged(&m->FQDN, 1); // Set status to 1 to indicate temporary failure
    }

    debugf("uDNS_SetupDNSConfig: number of unicast DNS servers %d", NumUnicastDNSServers);
    return mStatus_NoError;
}

mDNSexport void mDNSCoreInitComplete(mDNS *const m, mStatus result)
{
    m->mDNSPlatformStatus = result;
    if (m->MainCallback)
    {
        mDNS_Lock(m);
        mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
        m->MainCallback(m, mStatus_NoError);
        mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
        mDNS_Unlock(m);
    }
}

mDNSlocal void DeregLoop(mDNS *const m, AuthRecord *const start)
{
    m->CurrentRecord = start;
    while (m->CurrentRecord)
    {
        AuthRecord *rr = m->CurrentRecord;
        LogInfo("DeregLoop: %s deregistration for %p %02X %s",
                (rr->resrec.RecordType != kDNSRecordTypeDeregistering) ? "Initiating  " : "Accelerating",
                rr, rr->resrec.RecordType, ARDisplayString(m, rr));
        if (rr->resrec.RecordType != kDNSRecordTypeDeregistering)
            mDNS_Deregister_internal(m, rr, mDNS_Dereg_rapid);
        else if (rr->AnnounceCount > 1)
        {
            rr->AnnounceCount = 1;
            rr->LastAPTime = m->timenow - rr->ThisAPInterval;
        }
        // Mustn't advance m->CurrentRecord until *after* mDNS_Deregister_internal, because
        // new records could have been added to the end of the list as a result of that call.
        if (m->CurrentRecord == rr) // If m->CurrentRecord was not advanced for us, do it now
            m->CurrentRecord = rr->next;
    }
}

mDNSexport void mDNS_StartExit(mDNS *const m)
{
    NetworkInterfaceInfo *intf;
    AuthRecord *rr;

    mDNS_Lock(m);

    LogInfo("mDNS_StartExit");
    m->ShutdownTime = NonZeroTime(m->timenow + mDNSPlatformOneSecond * 5);

#ifndef REMOVE_MACHINE_SLEEP
    mDNSCoreBeSleepProxyServer_internal(m, 0, 0, 0, 0, 0);
#endif // #ifndef REMOVE_MACHINE_SLEEP
#if APPLE_OSX_mDNSResponder
#if !NO_WCF
    CHECK_WCF_FUNCTION(WCFConnectionDealloc)
    {
        if (m->WCF) WCFConnectionDealloc((WCFConnection *)m->WCF);
    }
#endif
#endif

#ifndef UNICAST_DISABLED
    {
        SearchListElem *s;
        SuspendLLQs(m);
        // Don't need to do SleepRecordRegistrations() here
        // because we deregister all records and services later in this routine
        while (m->Hostnames) mDNS_RemoveDynDNSHostName(m, &m->Hostnames->fqdn);

        // For each member of our SearchList, deregister any records it may have created, and cut them from the list.
        // Otherwise they'll be forcibly deregistered for us (without being cut them from the appropriate list)
        // and we may crash because the list still contains dangling pointers.
        for (s = SearchList; s; s = s->next)
            while (s->AuthRecs)
            {
                ARListElem *dereg = s->AuthRecs;
                s->AuthRecs = s->AuthRecs->next;
                mDNS_Deregister_internal(m, &dereg->ar, mDNS_Dereg_normal); // Memory will be freed in the FreeARElemCallback
            }
    }
#endif

    for (intf = m->HostInterfaces; intf; intf = intf->next)
        if (intf->Advertise)
            DeadvertiseInterface(m, intf);
#ifndef REMOVE_NAT_FUCTIONALITY
    // Shut down all our active NAT Traversals
    while (m->NATTraversals)
    {
        NATTraversalInfo *t = m->NATTraversals;
        mDNS_StopNATOperation_internal(m, t);       // This will cut 't' from the list, thereby advancing m->NATTraversals in the process

        // After stopping the NAT Traversal, we zero out the fields.
        // This has particularly important implications for our AutoTunnel records --
        // when we deregister our AutoTunnel records below, we don't want their mStatus_MemFree
        // handlers to just turn around and attempt to re-register those same records.
        // Clearing t->ExternalPort/t->RequestedPort will cause the mStatus_MemFree callback handlers
        // to not do this.
        t->ExternalAddress = zerov4Addr;
        t->NewAddress      = zerov4Addr;
        t->ExternalPort    = zeroIPPort;
        t->RequestedPort   = zeroIPPort;
        t->Lifetime        = 0;
        t->Result          = mStatus_NoError;
    }
#endif //#ifndef REMOVE_NAT_FUCTIONALITY

    // Make sure there are nothing but deregistering records remaining in the list
    if (m->CurrentRecord)
        LogMsg("mDNS_StartExit: ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));

    // We're in the process of shutting down, so queries, etc. are no longer available.
    // Consequently, determining certain information, e.g. the uDNS update server's IP
    // address, will not be possible.  The records on the main list are more likely to
    // already contain such information, so we deregister the duplicate records first.
    LogInfo("mDNS_StartExit: Deregistering duplicate resource records");
    DeregLoop(m, m->DuplicateRecords);
    LogInfo("mDNS_StartExit: Deregistering resource records");
    DeregLoop(m, m->ResourceRecords);

    // If we scheduled a response to send goodbye packets, we set NextScheduledResponse to now. Normally when deregistering records,
    // we allow up to 100ms delay (to help improve record grouping) but when shutting down we don't want any such delay.
    if (m->NextScheduledResponse - m->timenow < mDNSPlatformOneSecond)
    {
        m->NextScheduledResponse = m->timenow;
        m->SuppressSending = 0;
    }

    if (m->ResourceRecords) LogInfo("mDNS_StartExit: Sending final record deregistrations");
    else LogInfo("mDNS_StartExit: No deregistering records remain");

    for (rr = m->DuplicateRecords; rr; rr = rr->next)
        LogMsg("mDNS_StartExit: Should not still have Duplicate Records remaining: %02X %s", rr->resrec.RecordType, ARDisplayString(m, rr));

    // If any deregistering records remain, send their deregistration announcements before we exit
    if (m->mDNSPlatformStatus != mStatus_NoError) DiscardDeregistrations(m);

    mDNS_Unlock(m);

    LogInfo("mDNS_StartExit: done");
}

mDNSexport void mDNS_FinalExit(mDNS *const m)
{
    mDNSu32 rrcache_active = 0;
#ifndef REMOVE_UNUSED_VARS
    mDNSu32 rrcache_totalused = 0;
#endif //#ifndef REMOVE_UNUSED_VARS
    mDNSu32 slot;
    AuthRecord *rr;

    LogInfo("mDNS_FinalExit: mDNSPlatformClose");
    mDNSPlatformClose(m);

#ifndef REMOVE_UNUSED_VARS
//    rrcache_totalused = m->rrcache_totalused;
#endif //#ifndef REMOVE_UNUSED_VARS
    for (slot = 0; slot < CACHE_HASH_SLOTS; slot++)
    {
        while (m->rrcache_hash[slot])
        {
            CacheGroup *cg = m->rrcache_hash[slot];
            while (cg->members)
            {
                CacheRecord *cr = cg->members;
                cg->members = cg->members->next;
                if (cr->CRActiveQuestion) rrcache_active++;
                ReleaseCacheRecord(m, cr);
            }
            cg->rrcache_tail = &cg->members;
            ReleaseCacheGroup(m, &m->rrcache_hash[slot]);
        }
    }
//    debugf("mDNS_FinalExit: RR Cache was using %ld records, %lu active", rrcache_totalused, rrcache_active);
    if (rrcache_active != m->rrcache_active)
        LogMsg("*** ERROR *** rrcache_active %lu != m->rrcache_active %lu", rrcache_active, m->rrcache_active);

    for (rr = m->ResourceRecords; rr; rr = rr->next)
        LogMsg("mDNS_FinalExit failed to send goodbye for: %p %02X %s", rr, rr->resrec.RecordType, ARDisplayString(m, rr));

    LogInfo("mDNS_FinalExit: done");
}
