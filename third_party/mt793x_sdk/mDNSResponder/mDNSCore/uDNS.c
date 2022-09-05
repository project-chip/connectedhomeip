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

 * To Do:
 * Elimate all mDNSPlatformMemAllocate/mDNSPlatformMemFree from this code -- the core code
 * is supposed to be malloc-free so that it runs in constant memory determined at compile-time.
 * Any dynamic run-time requirements should be handled by the platform layer below or client layer above
 */

#if APPLE_OSX_mDNSResponder
#include <TargetConditionals.h>
#endif
#include "uDNS.h"

#ifdef REMOVE_NAT_FUCTIONALITY
#include "mDNSEmbeddedAPI.h"
#endif //#ifndef REMOVE_NAT_FUCTIONALITY

#if (defined(_MSC_VER))
// Disable "assignment within conditional expression".
// Other compilers understand the convention that if you place the assignment expression within an extra pair
// of parentheses, this signals to the compiler that you really intended an assignment and no warning is necessary.
// The Microsoft compiler doesn't understand this convention, so in the absense of any other way to signal
// to the compiler that the assignment is intentional, we have to just turn this warning off completely.
    #pragma warning(disable:4706)
#endif

// For domain enumeration and automatic browsing
// This is the user's DNS search list.
// In each of these domains we search for our special pointer records (lb._dns-sd._udp.<domain>, etc.)
// to discover recommended domains for domain enumeration (browse, default browse, registration,
// default registration) and possibly one or more recommended automatic browsing domains.
mDNSexport SearchListElem *SearchList = mDNSNULL;

// The value can be set to true by the Platform code e.g., MacOSX uses the plist mechanism
mDNSBool StrictUnicastOrdering = mDNSfalse;

// We keep track of the number of unicast DNS servers and log a message when we exceed 64.
// Currently the unicast queries maintain a 64 bit map to track the valid DNS servers for that
// question. Bit position is the index into the DNS server list. This is done so to try all
// the servers exactly once before giving up. If we could allocate memory in the core, then
// arbitrary limitation of 64 DNSServers can be removed.
mDNSu8 NumUnicastDNSServers = 0;
#define MAX_UNICAST_DNS_SERVERS 64

#define SetNextuDNSEvent(m, rr) { \
        if ((m)->NextuDNSEvent - ((rr)->LastAPTime + (rr)->ThisAPInterval) >= 0)                                                                              \
            (m)->NextuDNSEvent = ((rr)->LastAPTime + (rr)->ThisAPInterval);                                                                         \
}

#ifndef UNICAST_DISABLED

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - General Utility Functions
#endif

// set retry timestamp for record with exponential backoff
mDNSlocal void SetRecordRetry(mDNS *const m, AuthRecord *rr, mDNSu32 random)
{
    rr->LastAPTime = m->timenow;

    if (rr->expire && rr->refreshCount < MAX_UPDATE_REFRESH_COUNT)
    {
        mDNSs32 remaining = rr->expire - m->timenow;
        rr->refreshCount++;
        if (remaining > MIN_UPDATE_REFRESH_TIME)
        {
            // Refresh at 70% + random (currently it is 0 to 10%)
            rr->ThisAPInterval =  7 * (remaining/10) + (random ? random : mDNSRandom(remaining/10));
            // Don't update more often than 5 minutes
            if (rr->ThisAPInterval < MIN_UPDATE_REFRESH_TIME)
                rr->ThisAPInterval = MIN_UPDATE_REFRESH_TIME;
            LogInfo("SetRecordRetry refresh in %d of %d for %s",
                    rr->ThisAPInterval/mDNSPlatformOneSecond, (rr->expire - m->timenow)/mDNSPlatformOneSecond, ARDisplayString(m, rr));
        }
        else
        {
            rr->ThisAPInterval = MIN_UPDATE_REFRESH_TIME;
            LogInfo("SetRecordRetry clamping to min refresh in %d of %d for %s",
                    rr->ThisAPInterval/mDNSPlatformOneSecond, (rr->expire - m->timenow)/mDNSPlatformOneSecond, ARDisplayString(m, rr));
        }
        return;
    }

    rr->expire = 0;

    rr->ThisAPInterval = rr->ThisAPInterval * QuestionIntervalStep; // Same Retry logic as Unicast Queries
    if (rr->ThisAPInterval < INIT_RECORD_REG_INTERVAL)
        rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
    if (rr->ThisAPInterval > MAX_RECORD_REG_INTERVAL)
        rr->ThisAPInterval = MAX_RECORD_REG_INTERVAL;

    LogInfo("SetRecordRetry retry in %d ms for %s", rr->ThisAPInterval, ARDisplayString(m, rr));
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Name Server List Management
#endif

mDNSexport DNSServer *mDNS_AddDNSServer(mDNS *const m, const domainname *d, const mDNSInterfaceID interface, const mDNSs32 serviceID, const mDNSAddr *addr,
                                        const mDNSIPPort port, mDNSu32 scoped, mDNSu32 timeout, mDNSBool cellIntf, mDNSu16 resGroupID, mDNSBool reqA,
                                        mDNSBool reqAAAA, mDNSBool reqDO)
{
    DNSServer **p = &m->DNSServers;
    DNSServer *tmp = mDNSNULL;

    if ((NumUnicastDNSServers + 1) > MAX_UNICAST_DNS_SERVERS)
    {
        LogMsg("mDNS_AddDNSServer: DNS server limit of %d reached, not adding this server", MAX_UNICAST_DNS_SERVERS);
        return mDNSNULL;
    }

    if (!d) 
        d = (const domainname *)"";

    LogInfo("mDNS_AddDNSServer(%d): Adding %#a for %##s, InterfaceID %p, serviceID %u, scoped %d, resGroupID %d req_A is %s req_AAAA is %s cell %s req_DO is %s",
        NumUnicastDNSServers, addr, d->c, interface, serviceID, scoped, resGroupID, reqA ? "True" : "False", reqAAAA ? "True" : "False",
        cellIntf ? "True" : "False", reqDO ? "True" : "False");

    mDNS_CheckLock(m);

    while (*p)  // Check if we already have this {interface,address,port,domain} tuple registered + reqA/reqAAAA bits
    {
        if ((*p)->scoped == scoped && (*p)->interface == interface && (*p)->serviceID == serviceID && (*p)->teststate != DNSServer_Disabled &&
            mDNSSameAddress(&(*p)->addr, addr) && mDNSSameIPPort((*p)->port, port) && SameDomainName(&(*p)->domain, d) && 
            (*p)->req_A == reqA && (*p)->req_AAAA == reqAAAA)
        {
            if (!((*p)->flags & DNSServer_FlagDelete)) 
                debugf("Note: DNS Server %#a:%d for domain %##s (%p) registered more than once", addr, mDNSVal16(port), d->c, interface);
            tmp = *p;
            *p = tmp->next;
            tmp->next = mDNSNULL;
        }
        else
        {
            p=&(*p)->next;
        }
    }

    // NumUnicastDNSServers is the count of active DNS servers i.e., ones that are not marked
    // with DNSServer_FlagDelete. We should increment it:
    //
    // 1) When we add a new DNS server
    // 2) When we resurrect a old DNS server that is marked with DNSServer_FlagDelete
    //
    // Don't increment when we resurrect a DNS server that is not marked with DNSServer_FlagDelete.
    // We have already accounted for it when it was added for the first time. This case happens when
    // we add DNS servers with the same address multiple times (mis-configuration).

    if (!tmp || (tmp->flags & DNSServer_FlagDelete))
        NumUnicastDNSServers++;


    if (tmp)
    {
        tmp->flags &= ~DNSServer_FlagDelete;
        *p = tmp; // move to end of list, to ensure ordering from platform layer
    }
    else
    {
        // allocate, add to list
        *p = mDNSPlatformMemAllocate(sizeof(**p));
        if (!*p)
        {
            LogMsg("Error: mDNS_AddDNSServer - malloc");
        }
        else
        {
            (*p)->scoped    = scoped;
            (*p)->interface = interface;
            (*p)->serviceID = serviceID;
            (*p)->addr      = *addr;
            (*p)->port      = port;
            (*p)->flags     = DNSServer_FlagNew;
            (*p)->teststate = /* DNSServer_Untested */ DNSServer_Passed;
            (*p)->lasttest  = m->timenow - INIT_UCAST_POLL_INTERVAL;
            (*p)->timeout   = timeout;
            (*p)->cellIntf  = cellIntf;
            (*p)->req_A     = reqA;
            (*p)->req_AAAA  = reqAAAA;
            (*p)->req_DO    = reqDO;
            // We start off assuming that the DNS server is not DNSSEC aware and
            // when we receive the first response to a DNSSEC question, we set
            // it to true.
            (*p)->DNSSECAware = mDNSfalse;
            (*p)->retransDO = 0;
            AssignDomainName(&(*p)->domain, d);
            (*p)->next = mDNSNULL;
        }
    }
    (*p)->penaltyTime = 0;
    // We always update the ID (not just when we allocate a new instance) because we could
    // be adding a new non-scoped resolver with a new ID and we want all the non-scoped
    // resolvers belong to the same group.
    (*p)->resGroupID  = resGroupID;
    return(*p);
}

// PenalizeDNSServer is called when the number of queries to the unicast
// DNS server exceeds MAX_UCAST_UNANSWERED_QUERIES or when we receive an
// error e.g., SERV_FAIL from DNS server.
mDNSexport void PenalizeDNSServer(mDNS *const m, DNSQuestion *q, mDNSOpaque16 responseFlags)
{
    DNSServer *new;
    DNSServer *orig = q->qDNSServer;

    mDNS_CheckLock(m);

    LogInfo("PenalizeDNSServer: Penalizing DNS server %#a question for question %p %##s (%s) SuppressUnusable %d",
            (q->qDNSServer ? &q->qDNSServer->addr : mDNSNULL), q, q->qname.c, DNSTypeName(q->qtype), q->SuppressUnusable);

    // If we get error from any DNS server, remember the error. If all of the servers,
    // return the error, then return the first error. 
    if (mDNSOpaque16IsZero(q->responseFlags))
        q->responseFlags = responseFlags;

    // After we reset the qDNSServer to NULL, we could get more SERV_FAILS that might end up
    // peanlizing again.
    if (!q->qDNSServer) goto end;

    // If strict ordering of unicast servers needs to be preserved, we just lookup
    // the next best match server below
    //
    // If strict ordering is not required which is the default behavior, we penalize the server
    // for DNSSERVER_PENALTY_TIME. We may also use additional logic e.g., don't penalize for PTR
    // in the future.

    if (!StrictUnicastOrdering)
    {
        LogInfo("PenalizeDNSServer: Strict Unicast Ordering is FALSE");
        // We penalize the server so that new queries don't pick this server for DNSSERVER_PENALTY_TIME
        // XXX Include other logic here to see if this server should really be penalized
        //
        if (q->qtype == kDNSType_PTR)
        {
            LogInfo("PenalizeDNSServer: Not Penalizing PTR question");
        }
        else
        {
            LogInfo("PenalizeDNSServer: Penalizing question type %d", q->qtype);
            q->qDNSServer->penaltyTime = NonZeroTime(m->timenow + DNSSERVER_PENALTY_TIME);
        }
    }
    else
    {
        LogInfo("PenalizeDNSServer: Strict Unicast Ordering is TRUE");
    }

end:
    new = GetServerForQuestion(m, q);

    if (new == orig)
    {
        if (new)
        {
            LogMsg("PenalizeDNSServer: ERROR!! GetServerForQuestion returned the same server %#a:%d", &new->addr,
                   mDNSVal16(new->port));
            q->ThisQInterval = 0;   // Inactivate this question so that we dont bombard the network
        }
        else
        {
            // When we have no more DNS servers, we might end up calling PenalizeDNSServer multiple
            // times when we receive SERVFAIL from delayed packets in the network e.g., DNS server
            // is slow in responding and we have sent three queries. When we repeatedly call, it is
            // okay to receive the same NULL DNS server. Next time we try to send the query, we will
            // realize and re-initialize the DNS servers.
            LogInfo("PenalizeDNSServer: GetServerForQuestion returned the same server NULL");
        }
    }
    else
    {
        // The new DNSServer is set in DNSServerChangeForQuestion
        DNSServerChangeForQuestion(m, q, new);

        if (new)
        {
            LogInfo("PenalizeDNSServer: Server for %##s (%s) changed to %#a:%d (%##s)",
                    q->qname.c, DNSTypeName(q->qtype), &q->qDNSServer->addr, mDNSVal16(q->qDNSServer->port), q->qDNSServer->domain.c);
            // We want to try the next server immediately. As the question may already have backed off, reset
            // the interval. We do this only the first time when we try all the DNS servers. Once we reached the end of
            // list and retrying all the servers again e.g., at least one server failed to respond in the previous try, we
            // use the normal backoff which is done in uDNS_CheckCurrentQuestion when we send the packet out.
            if (!q->triedAllServersOnce)
            {
                q->ThisQInterval = InitialQuestionInterval;
                q->LastQTime  = m->timenow - q->ThisQInterval;
                SetNextQueryTime(m, q);
            }
        }
        else
        {
            // We don't have any more DNS servers for this question. If some server in the list did not return
            // any response, we need to keep retrying till we get a response. uDNS_CheckCurrentQuestion handles
            // this case.
            //
            // If all servers responded with a negative response, We need to do two things. First, generate a
            // negative response so that applications get a reply. We also need to reinitialize the DNS servers
            // so that when the cache expires, we can restart the query.  We defer this up until we generate
            // a negative cache response in uDNS_CheckCurrentQuestion.
            //
            // Be careful not to touch the ThisQInterval here. For a normal question, when we answer the question
            // in AnswerCurrentQuestionWithResourceRecord will set ThisQInterval to MaxQuestionInterval and hence
            // the next query will not happen until cache expiry. If it is a long lived question,
            // AnswerCurrentQuestionWithResourceRecord will not set it to MaxQuestionInterval. In that case,
            // we want the normal backoff to work.
            LogInfo("PenalizeDNSServer: Server for %p, %##s (%s) changed to NULL, Interval %d", q, q->qname.c, DNSTypeName(q->qtype), q->ThisQInterval);
        }
        q->unansweredQueries = 0;

    }
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - authorization management
#endif

mDNSlocal DomainAuthInfo *GetAuthInfoForName_direct(mDNS *m, const domainname *const name)
{
    const domainname *n = name;
    while (n->c[0])
    {
        DomainAuthInfo *ptr;
        for (ptr = m->AuthInfoList; ptr; ptr = ptr->next)
            if (SameDomainName(&ptr->domain, n))
            {
                debugf("GetAuthInfoForName %##s Matched %##s Key name %##s", name->c, ptr->domain.c, ptr->keyname.c);
                return(ptr);
            }
        n = (const domainname *)(n->c + 1 + n->c[0]);
    }
    //LogInfo("GetAuthInfoForName none found for %##s", name->c);
    return mDNSNULL;
}

// MUST be called with lock held
mDNSexport DomainAuthInfo *GetAuthInfoForName_internal(mDNS *m, const domainname *const name)
{
    DomainAuthInfo **p = &m->AuthInfoList;

    mDNS_CheckLock(m);

    // First purge any dead keys from the list
    while (*p)
    {
#ifndef REMOVE_TUNNELING
        if ((*p)->deltime && m->timenow - (*p)->deltime >= 0 && AutoTunnelUnregistered(*p))
#else
        if ((*p)->deltime && m->timenow - (*p)->deltime >= 0)
#endif // #ifndef REMOVE_TUNNELING
        {
            DNSQuestion *q;
            DomainAuthInfo *info = *p;
            LogInfo("GetAuthInfoForName_internal deleting expired key %##s %##s", info->domain.c, info->keyname.c);
            *p = info->next;    // Cut DomainAuthInfo from list *before* scanning our question list updating AuthInfo pointers
            for (q = m->Questions; q; q=q->next)
                if (q->AuthInfo == info)
                {
                    q->AuthInfo = GetAuthInfoForName_direct(m, &q->qname);
                    debugf("GetAuthInfoForName_internal updated q->AuthInfo from %##s to %##s for %##s (%s)",
                           info->domain.c, q->AuthInfo ? q->AuthInfo->domain.c : mDNSNULL, q->qname.c, DNSTypeName(q->qtype));
                }

            // Probably not essential, but just to be safe, zero out the secret key data
            // so we don't leave it hanging around in memory
            // (where it could potentially get exposed via some other bug)
            mDNSPlatformMemZero(info, sizeof(*info));
            mDNSPlatformMemFree(info);
        }
        else
            p = &(*p)->next;
    }

    return(GetAuthInfoForName_direct(m, name));
}

mDNSexport DomainAuthInfo *GetAuthInfoForName(mDNS *m, const domainname *const name)
{
    DomainAuthInfo *d;
    mDNS_Lock(m);
    d = GetAuthInfoForName_internal(m, name);
    mDNS_Unlock(m);
    return(d);
}

// MUST be called with the lock held
mDNSexport mStatus mDNS_SetSecretForDomain(mDNS *m, DomainAuthInfo *info,
                                           const domainname *domain, const domainname *keyname, const char *b64keydata, const domainname *hostname, mDNSIPPort *port, mDNSBool autoTunnel)
{
    DNSQuestion *q;
    DomainAuthInfo **p = &m->AuthInfoList;
    if (!info || !b64keydata) { LogMsg("mDNS_SetSecretForDomain: ERROR: info %p b64keydata %p", info, b64keydata); return(mStatus_BadParamErr); }

    LogInfo("mDNS_SetSecretForDomain: domain %##s key %##s%s", domain->c, keyname->c, autoTunnel ? " AutoTunnel" : "");

    info->AutoTunnel = autoTunnel;
    AssignDomainName(&info->domain,  domain);
    AssignDomainName(&info->keyname, keyname);
    if (hostname)
        AssignDomainName(&info->hostname, hostname);
    else
        info->hostname.c[0] = 0;
    if (port)
        info->port = *port;
    else
        info->port = zeroIPPort;
    mDNS_snprintf(info->b64keydata, sizeof(info->b64keydata), "%s", b64keydata);

    if (DNSDigest_ConstructHMACKeyfromBase64(info, b64keydata) < 0)
    {
        LogMsg("mDNS_SetSecretForDomain: ERROR: Could not convert shared secret from base64: domain %##s key %##s %s", domain->c, keyname->c, mDNS_LoggingEnabled ? b64keydata : "");
        return(mStatus_BadParamErr);
    }

    // Don't clear deltime until after we've ascertained that b64keydata is valid
    info->deltime = 0;

    while (*p && (*p) != info) p=&(*p)->next;
    if (*p) {LogInfo("mDNS_SetSecretForDomain: Domain %##s Already in list", (*p)->domain.c); return(mStatus_AlreadyRegistered);}

    // Caution: Only zero AutoTunnelHostRecord.namestorage AFTER we've determined that this is a NEW DomainAuthInfo
    // being added to the list. Otherwise we risk smashing our AutoTunnel host records that are already active and in use.
#ifndef REMOVE_TUNNELING
    info->AutoTunnelHostRecord.resrec.RecordType = kDNSRecordTypeUnregistered;
    info->AutoTunnelHostRecord.namestorage.c[0] = 0;
    info->AutoTunnelTarget.resrec.RecordType = kDNSRecordTypeUnregistered;
    info->AutoTunnelDeviceInfo.resrec.RecordType = kDNSRecordTypeUnregistered;
    info->AutoTunnelService.resrec.RecordType = kDNSRecordTypeUnregistered;
    info->AutoTunnel6Record.resrec.RecordType = kDNSRecordTypeUnregistered;
    info->AutoTunnelServiceStarted = mDNSfalse;
    info->AutoTunnelInnerAddress = zerov6Addr;
#endif // #ifndef REMOVE_TUNNELING
    info->next = mDNSNULL;
    *p = info;

    // Check to see if adding this new DomainAuthInfo has changed the credentials for any of our questions
    for (q = m->Questions; q; q=q->next)
    {
        DomainAuthInfo *newinfo = GetAuthInfoForQuestion(m, q);
        if (q->AuthInfo != newinfo)
        {
            debugf("mDNS_SetSecretForDomain updating q->AuthInfo from %##s to %##s for %##s (%s)",
                   q->AuthInfo ? q->AuthInfo->domain.c : mDNSNULL,
                   newinfo     ? newinfo->domain.c : mDNSNULL, q->qname.c, DNSTypeName(q->qtype));
            q->AuthInfo = newinfo;
        }
    }

    return(mStatus_NoError);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - NAT Traversal
#endif

// Keep track of when to request/refresh the external address using NAT-PMP or UPnP/IGD,
// and do so when necessary
mDNSlocal mStatus uDNS_RequestAddress(mDNS *m)
{
    mStatus err = mStatus_NoError;
    
    if (!m->NATTraversals)
    {
        m->retryGetAddr = NonZeroTime(m->timenow + 0x78000000);
        LogInfo("uDNS_RequestAddress: Setting retryGetAddr to future");
    }
    else if (m->timenow - m->retryGetAddr >= 0)
    {
        if (mDNSv4AddrIsRFC1918(&m->Router.ip.v4))
        {
            static NATAddrRequest req = {NATMAP_VERS, NATOp_AddrRequest};
            static mDNSu8* start = (mDNSu8*)&req;
            mDNSu8* end = start + sizeof(NATAddrRequest);
            err = mDNSPlatformSendUDP(m, start, end, 0, mDNSNULL, &m->Router, NATPMPPort, mDNSfalse);
            debugf("uDNS_RequestAddress: Sent NAT-PMP external address request %d", err);
            
#ifdef _LEGACY_NAT_TRAVERSAL_
            if (mDNSIPPortIsZero(m->UPnPRouterPort) || mDNSIPPortIsZero(m->UPnPSOAPPort))
            {
                LNT_SendDiscoveryMsg(m);
                debugf("uDNS_RequestAddress: LNT_SendDiscoveryMsg");
            }
            else
            {
                mStatus lnterr = LNT_GetExternalAddress(m);
                if (lnterr)
                    LogMsg("uDNS_RequestAddress: LNT_GetExternalAddress returned error %d", lnterr);
                
                err = err ? err : lnterr; // NAT-PMP error takes precedence
            }
#endif // _LEGACY_NAT_TRAVERSAL_
        }

        // Always update the interval and retry time, so that even if we fail to send the
        // packet, we won't spin in an infinite loop repeatedly failing to send the packet
        if (m->retryIntervalGetAddr < NATMAP_INIT_RETRY)
        {
            m->retryIntervalGetAddr = NATMAP_INIT_RETRY;
        }
        else if (m->retryIntervalGetAddr < NATMAP_MAX_RETRY_INTERVAL / 2)
        {
            m->retryIntervalGetAddr *= 2;
        }
        else
        {
            m->retryIntervalGetAddr = NATMAP_MAX_RETRY_INTERVAL;
        }

        m->retryGetAddr = NonZeroTime(m->timenow + m->retryIntervalGetAddr);
    }
    else
    {
        debugf("uDNS_RequestAddress: Not time to send address request");
    }

    // Always update NextScheduledNATOp, even if we didn't change retryGetAddr, so we'll
    // be called when we need to send the request(s)
    if (m->NextScheduledNATOp - m->retryGetAddr > 0)
        m->NextScheduledNATOp = m->retryGetAddr;

    return err;
}

mDNSlocal mStatus uDNS_SendNATMsg(mDNS *m, NATTraversalInfo *info, mDNSBool usePCP)
{
    mStatus err = mStatus_NoError;

    if (!info)
    {
        LogMsg("uDNS_SendNATMsg called unexpectedly with NULL info");
        return mStatus_BadParamErr;
    }
    
    // send msg if the router's address is private (which means it's non-zero)
    if (mDNSv4AddrIsRFC1918(&m->Router.ip.v4))
    {
        if (!usePCP)
        {
            if (!info->sentNATPMP)
            {
                if (info->Protocol)
                {
                    static NATPortMapRequest NATPortReq;
                    static const mDNSu8* end = (mDNSu8 *)&NATPortReq + sizeof(NATPortMapRequest);
                    mDNSu8 *p = (mDNSu8 *)&NATPortReq.NATReq_lease;
                    
                    NATPortReq.vers    = NATMAP_VERS;
                    NATPortReq.opcode  = info->Protocol;
                    NATPortReq.unused  = zeroID;
                    NATPortReq.intport = info->IntPort;
                    NATPortReq.extport = info->RequestedPort;
                    p[0] = (mDNSu8)((info->NATLease >> 24) &  0xFF);
                    p[1] = (mDNSu8)((info->NATLease >> 16) &  0xFF);
                    p[2] = (mDNSu8)((info->NATLease >>  8) &  0xFF);
                    p[3] = (mDNSu8)( info->NATLease        &  0xFF);
            
                    err = mDNSPlatformSendUDP(m, (mDNSu8 *)&NATPortReq, end, 0, mDNSNULL, &m->Router, NATPMPPort, mDNSfalse);
                    debugf("uDNS_SendNATMsg: Sent NAT-PMP mapping request %d", err);
                }

                // In case the address request already went out for another NAT-T,
                // set the NewAddress to the currently known global external address, so
                // Address-only operations will get the callback immediately
                info->NewAddress = m->ExtAddress;

                // Remember that we just sent a NAT-PMP packet, so we won't resend one later.
                // We do this because the NAT-PMP "Unsupported Version" response has no
                // information about the (PCP) request that triggered it, so we must send
                // NAT-PMP requests for all operations. Without this, we'll send n PCP
                // requests for n operations, receive n NAT-PMP "Unsupported Version"
                // responses, and send n NAT-PMP requests for each of those responses,
                // resulting in (n + n^2) packets sent. We only want to send 2n packets:
                // n PCP requests followed by n NAT-PMP requests.
                info->sentNATPMP = mDNStrue;
            }
        }
        else
        {
            PCPMapRequest req;
            mDNSu8* start = (mDNSu8*)&req;
            mDNSu8* end = start + sizeof(req);
            mDNSu8* p = (mDNSu8*)&req.lifetime;
            
            req.version = PCP_VERS;
            req.opCode = PCPOp_Map;
            req.reserved = zeroID;
            
            p[0] = (mDNSu8)((info->NATLease >> 24) &  0xFF);
            p[1] = (mDNSu8)((info->NATLease >> 16) &  0xFF);
            p[2] = (mDNSu8)((info->NATLease >>  8) &  0xFF);
            p[3] = (mDNSu8)( info->NATLease        &  0xFF);
            
            mDNSAddrMapIPv4toIPv6(&m->AdvertisedV4.ip.v4, &req.clientAddr);
            
            req.nonce[0] = m->PCPNonce[0];
            req.nonce[1] = m->PCPNonce[1];
            req.nonce[2] = m->PCPNonce[2];
            
            req.protocol = (info->Protocol == NATOp_MapUDP ? PCPProto_UDP : PCPProto_TCP);
            
            req.reservedMapOp[0] = 0;
            req.reservedMapOp[1] = 0;
            req.reservedMapOp[2] = 0;
            
            req.intPort = info->Protocol ? info->IntPort : DiscardPort;
            req.extPort = info->RequestedPort;
            
            // Since we only support IPv4, even if using the all-zeros address, map it, so
            // the PCP gateway will give us an IPv4 address & not an IPv6 address.
            mDNSAddrMapIPv4toIPv6(&info->NewAddress, &req.extAddress);

            err = mDNSPlatformSendUDP(m, start, end, 0, mDNSNULL, &m->Router, NATPMPPort, mDNSfalse);
            debugf("uDNS_SendNATMsg: Sent PCP Mapping request %d", err);

            // Unset the sentNATPMP flag, so that we'll send a NAT-PMP packet if we
            // receive a NAT-PMP "Unsupported Version" packet. This will result in every
            // renewal, retransmission, etc. being tried first as PCP, then if a NAT-PMP
            // "Unsupported Version" response is received, fall-back & send the request
            // using NAT-PMP.
            info->sentNATPMP = mDNSfalse;

#ifdef _LEGACY_NAT_TRAVERSAL_
            if (mDNSIPPortIsZero(m->UPnPRouterPort) || mDNSIPPortIsZero(m->UPnPSOAPPort))
            {
                LNT_SendDiscoveryMsg(m);
                debugf("uDNS_SendNATMsg: LNT_SendDiscoveryMsg");
            }
            else
            {
                mStatus lnterr = LNT_MapPort(m, info);
                if (lnterr)
                    LogMsg("uDNS_SendNATMsg: LNT_MapPort returned error %d", lnterr);
                
                err = err ? err : lnterr; // PCP error takes precedence
            }
#endif // _LEGACY_NAT_TRAVERSAL_
        }
    }

    return(err);
}

mDNSexport void RecreateNATMappings(mDNS *const m, const mDNSu32 waitTicks)
{
    mDNSu32 when = NonZeroTime(m->timenow + waitTicks);
    NATTraversalInfo *n;
    for (n = m->NATTraversals; n; n=n->next)
    {
        n->ExpiryTime    = 0;       // Mark this mapping as expired
        n->retryInterval = NATMAP_INIT_RETRY;
        n->retryPortMap  = when;
        n->lastSuccessfulProtocol = NATTProtocolNone;
        if (!n->Protocol) n->NewResult = mStatus_NoError;
#ifdef _LEGACY_NAT_TRAVERSAL_
        if (n->tcpInfo.sock) { mDNSPlatformTCPCloseConnection(n->tcpInfo.sock); n->tcpInfo.sock = mDNSNULL; }
#endif // _LEGACY_NAT_TRAVERSAL_
    }

    m->PCPNonce[0] = mDNSRandom(-1);
    m->PCPNonce[1] = mDNSRandom(-1);
    m->PCPNonce[2] = mDNSRandom(-1);
    m->retryIntervalGetAddr = 0;
    m->retryGetAddr = when;

#ifdef _LEGACY_NAT_TRAVERSAL_
    LNT_ClearState(m);
#endif // _LEGACY_NAT_TRAVERSAL_

    m->NextScheduledNATOp = m->timenow;     // Need to send packets immediately
}

mDNSexport void natTraversalHandleAddressReply(mDNS *const m, mDNSu16 err, mDNSv4Addr ExtAddr)
{
    static mDNSu16 last_err = 0;
    NATTraversalInfo *n;

    if (err)
    {
        if (err != last_err) LogMsg("Error getting external address %d", err);
        ExtAddr = zerov4Addr;
    }
    else
    {
        LogInfo("Received external IP address %.4a from NAT", &ExtAddr);
        if (mDNSv4AddrIsRFC1918(&ExtAddr))
            LogMsg("Double NAT (external NAT gateway address %.4a is also a private RFC 1918 address)", &ExtAddr);
        if (mDNSIPv4AddressIsZero(ExtAddr))
            err = NATErr_NetFail; // fake error to handle routers that pathologically report success with the zero address
    }

    // Globally remember the most recently discovered address, so it can be used in each
    // new NATTraversal structure
    m->ExtAddress = ExtAddr;

    if (!err) // Success, back-off to maximum interval
        m->retryIntervalGetAddr = NATMAP_MAX_RETRY_INTERVAL;
    else if (!last_err) // Failure after success, retry quickly (then back-off exponentially)
        m->retryIntervalGetAddr = NATMAP_INIT_RETRY;
    // else back-off normally in case of pathological failures

    m->retryGetAddr = m->timenow + m->retryIntervalGetAddr;
    if (m->NextScheduledNATOp - m->retryGetAddr > 0)
        m->NextScheduledNATOp = m->retryGetAddr;

    last_err = err;
    
    for (n = m->NATTraversals; n; n=n->next)
    {
        // We should change n->NewAddress only when n is one of:
        // 1) a mapping operation that most recently succeeded using NAT-PMP or UPnP/IGD,
        //    because such an operation needs the update now. If the lastSuccessfulProtocol
        //    is currently none, then natTraversalHandlePortMapReplyWithAddress() will be
        //    called should NAT-PMP or UPnP/IGD succeed in the future.
        // 2) an address-only operation that did not succeed via PCP, because when such an
        //    operation succeeds via PCP, it's for the TCP discard port just to learn the
        //    address. And that address may be different than the external address
        //    discovered via NAT-PMP or UPnP/IGD. If the lastSuccessfulProtocol
        //    is currently none, we must update the NewAddress as PCP may not succeed.
        if (!mDNSSameIPv4Address(n->NewAddress, ExtAddr) &&
             (n->Protocol ?
               (n->lastSuccessfulProtocol == NATTProtocolNATPMP || n->lastSuccessfulProtocol == NATTProtocolUPNPIGD) :
               (n->lastSuccessfulProtocol != NATTProtocolPCP)))
        {
            // Needs an update immediately
            n->NewAddress    = ExtAddr;
            n->ExpiryTime    = 0;
            n->retryInterval = NATMAP_INIT_RETRY;
            n->retryPortMap  = m->timenow;
#ifdef _LEGACY_NAT_TRAVERSAL_
            if (n->tcpInfo.sock) { mDNSPlatformTCPCloseConnection(n->tcpInfo.sock); n->tcpInfo.sock = mDNSNULL; }
#endif // _LEGACY_NAT_TRAVERSAL_

            m->NextScheduledNATOp = m->timenow;     // Need to send packets immediately
        }
    }
}

// Both places that call NATSetNextRenewalTime() update m->NextScheduledNATOp correctly afterwards
mDNSlocal void NATSetNextRenewalTime(mDNS *const m, NATTraversalInfo *n)
{
    n->retryInterval = (n->ExpiryTime - m->timenow)/2;
    if (n->retryInterval < NATMAP_MIN_RETRY_INTERVAL)   // Min retry interval is 2 seconds
        n->retryInterval = NATMAP_MIN_RETRY_INTERVAL;
    n->retryPortMap = m->timenow + n->retryInterval;
}

mDNSlocal void natTraversalHandlePortMapReplyWithAddress(mDNS *const m, NATTraversalInfo *n, const mDNSInterfaceID InterfaceID, mDNSu16 err, mDNSv4Addr extaddr, mDNSIPPort extport, mDNSu32 lease, NATTProtocol protocol)
{
    const char *prot = n->Protocol == 0 ? "Add" : n->Protocol == NATOp_MapUDP ? "UDP" : n->Protocol == NATOp_MapTCP ? "TCP" : "???";
    (void)prot;
    n->NewResult = err;
    if (err || lease == 0 || mDNSIPPortIsZero(extport))
    {
        LogInfo("natTraversalHandlePortMapReplyWithAddress: %p Response %s Port %5d External %.4a:%d lease %d error %d",
                n, prot, mDNSVal16(n->IntPort), &extaddr, mDNSVal16(extport), lease, err);
        n->retryInterval = NATMAP_MAX_RETRY_INTERVAL;
        n->retryPortMap = m->timenow + NATMAP_MAX_RETRY_INTERVAL;
        // No need to set m->NextScheduledNATOp here, since we're only ever extending the m->retryPortMap time
        if      (err == NATErr_Refused) n->NewResult = mStatus_NATPortMappingDisabled;
        else if (err > NATErr_None && err <= NATErr_Opcode) n->NewResult = mStatus_NATPortMappingUnsupported;
    }
    else
    {
        if (lease > 999999999UL / mDNSPlatformOneSecond)
            lease = 999999999UL / mDNSPlatformOneSecond;
        n->ExpiryTime = NonZeroTime(m->timenow + lease * mDNSPlatformOneSecond);

        if (!mDNSSameIPv4Address(n->NewAddress, extaddr) || !mDNSSameIPPort(n->RequestedPort, extport))
            LogInfo("natTraversalHandlePortMapReplyWithAddress: %p %s Response %s Port %5d External %.4a:%d changed to %.4a:%d lease %d",
                    n,
                    (n->lastSuccessfulProtocol == NATTProtocolNone    ? "None    " :
                     n->lastSuccessfulProtocol == NATTProtocolNATPMP  ? "NAT-PMP " :
                     n->lastSuccessfulProtocol == NATTProtocolUPNPIGD ? "UPnP/IGD" :
                     n->lastSuccessfulProtocol == NATTProtocolPCP     ? "PCP     " :
                     /* else */                                         "Unknown " ),
                    prot, mDNSVal16(n->IntPort), &n->NewAddress, mDNSVal16(n->RequestedPort),
                    &extaddr, mDNSVal16(extport), lease);

        n->InterfaceID   = InterfaceID;
        n->NewAddress    = extaddr;
        if (n->Protocol) n->RequestedPort = extport; // Don't report the (PCP) external port to address-only operations
        n->lastSuccessfulProtocol = protocol;

        NATSetNextRenewalTime(m, n);            // Got our port mapping; now set timer to renew it at halfway point
        m->NextScheduledNATOp = m->timenow;     // May need to invoke client callback immediately
    }
}

// To be called for NAT-PMP or UPnP/IGD mappings, to use currently discovered (global) address
mDNSexport void natTraversalHandlePortMapReply(mDNS *const m, NATTraversalInfo *n, const mDNSInterfaceID InterfaceID, mDNSu16 err, mDNSIPPort extport, mDNSu32 lease, NATTProtocol protocol)
{
    natTraversalHandlePortMapReplyWithAddress(m, n, InterfaceID, err, m->ExtAddress, extport, lease, protocol);
}

// Must be called with the mDNS_Lock held
mDNSexport mStatus mDNS_StartNATOperation_internal(mDNS *const m, NATTraversalInfo *traversal)
{
    NATTraversalInfo **n;

    LogInfo("mDNS_StartNATOperation_internal %p Protocol %d IntPort %d RequestedPort %d NATLease %d", traversal,
            traversal->Protocol, mDNSVal16(traversal->IntPort), mDNSVal16(traversal->RequestedPort), traversal->NATLease);

    // Note: It important that new traversal requests are appended at the *end* of the list, not prepended at the start
    for (n = &m->NATTraversals; *n; n=&(*n)->next)
    {
        if (traversal == *n)
        {
            LogMsg("Error! Tried to add a NAT traversal that's already in the active list: request %p Prot %d Int %d TTL %d",
                   traversal, traversal->Protocol, mDNSVal16(traversal->IntPort), traversal->NATLease);
            #if ForceAlerts
            *(long*)0 = 0;
            #endif
            return(mStatus_AlreadyRegistered);
        }
        if (traversal->Protocol && traversal->Protocol == (*n)->Protocol && mDNSSameIPPort(traversal->IntPort, (*n)->IntPort) &&
            !mDNSSameIPPort(traversal->IntPort, SSHPort))
            LogMsg("Warning: Created port mapping request %p Prot %d Int %d TTL %d "
                   "duplicates existing port mapping request %p Prot %d Int %d TTL %d",
                   traversal, traversal->Protocol, mDNSVal16(traversal->IntPort), traversal->NATLease,
                   *n,        (*n)->Protocol, mDNSVal16((*n)->IntPort), (*n)->NATLease);
    }

    // Initialize necessary fields
    traversal->next            = mDNSNULL;
    traversal->ExpiryTime      = 0;
    traversal->retryInterval   = NATMAP_INIT_RETRY;
    traversal->retryPortMap    = m->timenow;
    traversal->NewResult       = mStatus_NoError;
    traversal->lastSuccessfulProtocol = NATTProtocolNone;
    traversal->sentNATPMP      = mDNSfalse;
    traversal->ExternalAddress = onesIPv4Addr;
    traversal->NewAddress      = zerov4Addr;
    traversal->ExternalPort    = zeroIPPort;
    traversal->Lifetime        = 0;
    traversal->Result          = mStatus_NoError;

    // set default lease if necessary
    if (!traversal->NATLease) traversal->NATLease = NATMAP_DEFAULT_LEASE;

#ifdef _LEGACY_NAT_TRAVERSAL_
    mDNSPlatformMemZero(&traversal->tcpInfo, sizeof(traversal->tcpInfo));
#endif // _LEGACY_NAT_TRAVERSAL_

    if (!m->NATTraversals)      // If this is our first NAT request, kick off an address request too
    {
        m->retryGetAddr         = m->timenow;
        m->retryIntervalGetAddr = NATMAP_INIT_RETRY;
    }

    // If this is an address-only operation, initialize to the current global address,
    // or (in non-PCP environments) we won't know the address until the next external
    // address request/response.
    if (!traversal->Protocol)
    {
        traversal->NewAddress = m->ExtAddress;
    }

    m->NextScheduledNATOp = m->timenow; // This will always trigger sending the packet ASAP, and generate client callback if necessary

    *n = traversal;     // Append new NATTraversalInfo to the end of our list

    return(mStatus_NoError);
}

// Must be called with the mDNS_Lock held
mDNSexport mStatus mDNS_StopNATOperation_internal(mDNS *m, NATTraversalInfo *traversal)
{
    mDNSBool unmap = mDNStrue;
    NATTraversalInfo *p;
    NATTraversalInfo **ptr = &m->NATTraversals;

    while (*ptr && *ptr != traversal) ptr=&(*ptr)->next;
    if (*ptr) *ptr = (*ptr)->next;      // If we found it, cut this NATTraversalInfo struct from our list
    else
    {
        LogMsg("mDNS_StopNATOperation_internal: NATTraversalInfo %p not found in list", traversal);
        return(mStatus_BadReferenceErr);
    }

    LogInfo("mDNS_StopNATOperation_internal %p %d %d %d %d", traversal,
            traversal->Protocol, mDNSVal16(traversal->IntPort), mDNSVal16(traversal->RequestedPort), traversal->NATLease);

    if (m->CurrentNATTraversal == traversal)
        m->CurrentNATTraversal = m->CurrentNATTraversal->next;

    // If there is a match for the operation being stopped, don't send a deletion request (unmap)
    for (p = m->NATTraversals; p; p=p->next)
    {
        if (traversal->Protocol ?
            ((traversal->Protocol == p->Protocol && mDNSSameIPPort(traversal->IntPort, p->IntPort)) ||
             (!p->Protocol && traversal->Protocol == NATOp_MapTCP && mDNSSameIPPort(traversal->IntPort, DiscardPort))) :
            (!p->Protocol || (p->Protocol == NATOp_MapTCP && mDNSSameIPPort(p->IntPort, DiscardPort))))
        {
            LogInfo("Warning: Removed port mapping request %p Prot %d Int %d TTL %d "
                    "duplicates existing port mapping request %p Prot %d Int %d TTL %d",
                    traversal, traversal->Protocol, mDNSVal16(traversal->IntPort), traversal->NATLease,
                            p,         p->Protocol, mDNSVal16(        p->IntPort),         p->NATLease);
            unmap = mDNSfalse;
        }
    }

    if (traversal->ExpiryTime && unmap)
    {
        traversal->NATLease = 0;
        traversal->retryInterval = 0;
        
        // In case we most recently sent NAT-PMP, we need to set sentNATPMP to false so
        // that we'll send a NAT-PMP request to destroy the mapping. We do this because
        // the NATTraversal struct has already been cut from the list, and the client
        // layer will destroy the memory upon returning from this function, so we can't
        // try PCP first and then fall-back to NAT-PMP. That is, if we most recently
        // created/renewed the mapping using NAT-PMP, we need to destroy it using NAT-PMP
        // now, because we won't get a chance later.
        traversal->sentNATPMP = mDNSfalse;

        // Both NAT-PMP & PCP RFCs state that the suggested port in deletion requests
        // should be zero. And for PCP, the suggested external address should also be
        // zero, specifically, the all-zeros IPv4-mapped address, since we would only
        // would have requested an IPv4 address.
        traversal->RequestedPort = zeroIPPort;
        traversal->NewAddress = zerov4Addr;
        
        uDNS_SendNATMsg(m, traversal, traversal->lastSuccessfulProtocol != NATTProtocolNATPMP);
    }

    // Even if we DIDN'T make a successful UPnP mapping yet, we might still have a partially-open TCP connection we need to clean up
    #ifdef _LEGACY_NAT_TRAVERSAL_
    {
        mStatus err = LNT_UnmapPort(m, traversal);
        if (err) LogMsg("Legacy NAT Traversal - unmap request failed with error %d", err);
    }
    #endif // _LEGACY_NAT_TRAVERSAL_

    return(mStatus_NoError);
}

mDNSexport mStatus mDNS_StartNATOperation(mDNS *const m, NATTraversalInfo *traversal)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_StartNATOperation_internal(m, traversal);
    mDNS_Unlock(m);
    return(status);
}

mDNSexport mStatus mDNS_StopNATOperation(mDNS *const m, NATTraversalInfo *traversal)
{
    mStatus status;
    mDNS_Lock(m);
    status = mDNS_StopNATOperation_internal(m, traversal);
    mDNS_Unlock(m);
    return(status);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark -
#pragma mark - Long-Lived Queries
#endif

// Lock must be held -- otherwise m->timenow is undefined
mDNSlocal void StartLLQPolling(mDNS *const m, DNSQuestion *q)
{
    debugf("StartLLQPolling: %##s", q->qname.c);
    q->state = LLQ_Poll;
    q->ThisQInterval = INIT_UCAST_POLL_INTERVAL;
    // We want to send our poll query ASAP, but the "+ 1" is because if we set the time to now,
    // we risk causing spurious "SendQueries didn't send all its queries" log messages
    q->LastQTime     = m->timenow - q->ThisQInterval + 1;
    SetNextQueryTime(m, q);
#if APPLE_OSX_mDNSResponder
    UpdateAutoTunnelDomainStatuses(m);
#endif
}

mDNSlocal mDNSu8 *putLLQ(DNSMessage *const msg, mDNSu8 *ptr, const DNSQuestion *const question, const LLQOptData *const data)
{
    AuthRecord rr;
    ResourceRecord *opt = &rr.resrec;
    rdataOPT *optRD;

    //!!!KRS when we implement multiple llqs per message, we'll need to memmove anything past the question section
    ptr = putQuestion(msg, ptr, msg->data + AbsoluteMaxDNSMessageData, &question->qname, question->qtype, question->qclass);
    if (!ptr) { LogMsg("ERROR: putLLQ - putQuestion"); return mDNSNULL; }

    // locate OptRR if it exists, set pointer to end
    // !!!KRS implement me

    // format opt rr (fields not specified are zero-valued)
    mDNS_SetupResourceRecord(&rr, mDNSNULL, mDNSInterface_Any, kDNSType_OPT, kStandardTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, mDNSNULL, mDNSNULL);
    opt->rrclass    = NormalMaxDNSMessageData;
    opt->rdlength   = sizeof(rdataOPT); // One option in this OPT record
    opt->rdestimate = sizeof(rdataOPT);

    optRD = &rr.resrec.rdata->u.opt[0];
    optRD->opt = kDNSOpt_LLQ;
    optRD->u.llq = *data;
    ptr = PutResourceRecordTTLJumbo(msg, ptr, &msg->h.numAdditionals, opt, 0);
    if (!ptr) { LogMsg("ERROR: putLLQ - PutResourceRecordTTLJumbo"); return mDNSNULL; }

    return ptr;
}

// Normally we'd just request event packets be sent directly to m->LLQNAT.ExternalPort, except...
// with LLQs over TLS/TCP we're doing a weird thing where instead of requesting packets be sent to ExternalAddress:ExternalPort
// we're requesting that packets be sent to ExternalPort, but at the source address of our outgoing TCP connection.
// Normally, after going through the NAT gateway, the source address of our outgoing TCP connection is the same as ExternalAddress,
// so this is fine, except when the TCP connection ends up going over a VPN tunnel instead.
// To work around this, if we find that the source address for our TCP connection is not a private address, we tell the Dot Mac
// LLQ server to send events to us directly at port 5353 on that address, instead of at our mapped external NAT port.

mDNSlocal mDNSu16 GetLLQEventPort(const mDNS *const m, const mDNSAddr *const dst)
{
    mDNSAddr src;
    mDNSPlatformSourceAddrForDest(&src, dst);
    //LogMsg("GetLLQEventPort: src %#a for dst %#a (%d)", &src, dst, mDNSv4AddrIsRFC1918(&src.ip.v4) ? mDNSVal16(m->LLQNAT.ExternalPort) : 0);
    return(mDNSv4AddrIsRFC1918(&src.ip.v4) ? mDNSVal16(m->LLQNAT.ExternalPort) : mDNSVal16(MulticastDNSPort));
}

// Normally called with llq set.
// May be called with llq NULL, when retransmitting a lost Challenge Response
mDNSlocal void sendChallengeResponse(mDNS *const m, DNSQuestion *const q, const LLQOptData *llq)
{
    mDNSu8 *responsePtr = m->omsg.data;
    LLQOptData llqBuf;

    if (q->tcp) { LogMsg("sendChallengeResponse: ERROR!!: question %##s (%s) tcp non-NULL", q->qname.c, DNSTypeName(q->qtype)); return; }

    if (PrivateQuery(q)) { LogMsg("sendChallengeResponse: ERROR!!: Private Query %##s (%s)", q->qname.c, DNSTypeName(q->qtype)); return; }

    if (q->ntries++ == kLLQ_MAX_TRIES)
    {
        LogMsg("sendChallengeResponse: %d failed attempts for LLQ %##s", kLLQ_MAX_TRIES, q->qname.c);
        StartLLQPolling(m,q);
        return;
    }

    if (!llq)       // Retransmission: need to make a new LLQOptData
    {
        llqBuf.vers     = kLLQ_Vers;
        llqBuf.llqOp    = kLLQOp_Setup;
        llqBuf.err      = LLQErr_NoError;   // Don't need to tell server UDP notification port when sending over UDP
        llqBuf.id       = q->id;
        llqBuf.llqlease = q->ReqLease;
        llq = &llqBuf;
    }

    q->LastQTime     = m->timenow;
    q->ThisQInterval = q->tcp ? 0 : (kLLQ_INIT_RESEND * q->ntries * mDNSPlatformOneSecond);     // If using TCP, don't need to retransmit
    SetNextQueryTime(m, q);

    // To simulate loss of challenge response packet, uncomment line below
    //if (q->ntries == 1) return;

    InitializeDNSMessage(&m->omsg.h, q->TargetQID, uQueryFlags);
    responsePtr = putLLQ(&m->omsg, responsePtr, q, llq);
    if (responsePtr)
    {
        mStatus err = mDNSSendDNSMessage(m, &m->omsg, responsePtr, mDNSInterface_Any, q->LocalSocket, &q->servAddr, q->servPort, mDNSNULL, mDNSNULL, mDNSfalse);
        if (err) { LogMsg("sendChallengeResponse: mDNSSendDNSMessage%s failed: %d", q->tcp ? " (TCP)" : "", err); }
    }
    else StartLLQPolling(m,q);
}

mDNSlocal void SetLLQTimer(mDNS *const m, DNSQuestion *const q, const LLQOptData *const llq)
{
    mDNSs32 lease = (mDNSs32)llq->llqlease * mDNSPlatformOneSecond;
    q->ReqLease      = llq->llqlease;
    q->LastQTime     = m->timenow;
    q->expire        = m->timenow + lease;
    q->ThisQInterval = lease/2 + mDNSRandom(lease/10);
    debugf("SetLLQTimer setting %##s (%s) to %d %d", q->qname.c, DNSTypeName(q->qtype), lease/mDNSPlatformOneSecond, q->ThisQInterval/mDNSPlatformOneSecond);
    SetNextQueryTime(m, q);
}

mDNSlocal void recvSetupResponse(mDNS *const m, mDNSu8 rcode, DNSQuestion *const q, const LLQOptData *const llq)
{
    if (rcode && rcode != kDNSFlag1_RC_NXDomain)
    { LogMsg("ERROR: recvSetupResponse %##s (%s) - rcode && rcode != kDNSFlag1_RC_NXDomain", q->qname.c, DNSTypeName(q->qtype)); return; }

    if (llq->llqOp != kLLQOp_Setup)
    { LogMsg("ERROR: recvSetupResponse %##s (%s) - bad op %d", q->qname.c, DNSTypeName(q->qtype), llq->llqOp); return; }

    if (llq->vers != kLLQ_Vers)
    { LogMsg("ERROR: recvSetupResponse %##s (%s) - bad vers %d", q->qname.c, DNSTypeName(q->qtype), llq->vers); return; }

    if (q->state == LLQ_InitialRequest)
    {
        //LogInfo("Got LLQ_InitialRequest");

        if (llq->err) { LogMsg("recvSetupResponse - received llq->err %d from server", llq->err); StartLLQPolling(m,q); return; }

        if (q->ReqLease != llq->llqlease)
            debugf("recvSetupResponse: requested lease %lu, granted lease %lu", q->ReqLease, llq->llqlease);

        // cache expiration in case we go to sleep before finishing setup
        q->ReqLease = llq->llqlease;
        q->expire = m->timenow + ((mDNSs32)llq->llqlease * mDNSPlatformOneSecond);

        // update state
        q->state  = LLQ_SecondaryRequest;
        q->id     = llq->id;
        q->ntries = 0; // first attempt to send response
        sendChallengeResponse(m, q, llq);
    }
    else if (q->state == LLQ_SecondaryRequest)
    {
        //LogInfo("Got LLQ_SecondaryRequest");

        // Fix this immediately if not sooner.  Copy the id from the LLQOptData into our DNSQuestion struct.  This is only
        // an issue for private LLQs, because we skip parts 2 and 3 of the handshake.  This is related to a bigger
        // problem of the current implementation of TCP LLQ setup: we're not handling state transitions correctly
        // if the server sends back SERVFULL or STATIC.
        if (PrivateQuery(q))
        {
            LogInfo("Private LLQ_SecondaryRequest; copying id %08X%08X", llq->id.l[0], llq->id.l[1]);
            q->id = llq->id;
        }

        if (llq->err) { LogMsg("ERROR: recvSetupResponse %##s (%s) code %d from server", q->qname.c, DNSTypeName(q->qtype), llq->err); StartLLQPolling(m,q); return; }
        if (!mDNSSameOpaque64(&q->id, &llq->id))
        { LogMsg("recvSetupResponse - ID changed.  discarding"); return; }     // this can happen rarely (on packet loss + reordering)
        q->state         = LLQ_Established;
        q->ntries        = 0;
        SetLLQTimer(m, q, llq);
#if APPLE_OSX_mDNSResponder
        UpdateAutoTunnelDomainStatuses(m);
#endif
    }
}

mDNSexport uDNS_LLQType uDNS_recvLLQResponse(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end,
                                             const mDNSAddr *const srcaddr, const mDNSIPPort srcport, DNSQuestion **matchQuestion)
{
    DNSQuestion pktQ, *q;
    if (msg->h.numQuestions && getQuestion(msg, msg->data, end, 0, &pktQ))
    {
        const rdataOPT *opt = GetLLQOptData(m, msg, end);

        for (q = m->Questions; q; q = q->next)
        {
            if (!mDNSOpaque16IsZero(q->TargetQID) && q->LongLived && q->qtype == pktQ.qtype && q->qnamehash == pktQ.qnamehash && SameDomainName(&q->qname, &pktQ.qname))
            {
                debugf("uDNS_recvLLQResponse found %##s (%s) %d %#a %#a %X %X %X %X %d",
                       q->qname.c, DNSTypeName(q->qtype), q->state, srcaddr, &q->servAddr,
                       opt ? opt->u.llq.id.l[0] : 0, opt ? opt->u.llq.id.l[1] : 0, q->id.l[0], q->id.l[1], opt ? opt->u.llq.llqOp : 0);
                if (q->state == LLQ_Poll) debugf("uDNS_LLQ_Events: q->state == LLQ_Poll msg->h.id %d q->TargetQID %d", mDNSVal16(msg->h.id), mDNSVal16(q->TargetQID));
                if (q->state == LLQ_Poll && mDNSSameOpaque16(msg->h.id, q->TargetQID))
                {
                    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it

                    // Don't reset the state to IntialRequest as we may write that to the dynamic store
                    // and PrefPane might wrongly think that we are "Starting" instead of "Polling". If
                    // we are in polling state because of PCP/NAT-PMP disabled or DoubleNAT, next LLQNATCallback
                    // would kick us back to LLQInitialRequest. So, resetting the state here may not be useful.
                    //
                    // If we have a good NAT (neither PCP/NAT-PMP disabled nor Double-NAT), then we should not be
                    // possibly in polling state. To be safe, we want to retry from the start in that case
                    // as there may not be another LLQNATCallback
                    //
                    // NOTE: We can be in polling state if we cannot resolve the SOA record i.e, servAddr is set to
                    // all ones. In that case, we would set it in LLQ_InitialRequest as it overrides the PCP/NAT-PMP or
                    // Double-NAT state.
                    if (!mDNSAddressIsOnes(&q->servAddr) && !mDNSIPPortIsZero(m->LLQNAT.ExternalPort) &&
                        !m->LLQNAT.Result)
                    {
                        debugf("uDNS_recvLLQResponse got poll response; moving to LLQ_InitialRequest for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
                        q->state         = LLQ_InitialRequest;
                    }
                    q->servPort      = zeroIPPort;      // Clear servPort so that startLLQHandshake will retry the GetZoneData processing
                    q->ThisQInterval = LLQ_POLL_INTERVAL + mDNSRandom(LLQ_POLL_INTERVAL/10);    // Retry LLQ setup in approx 15 minutes
                    q->LastQTime     = m->timenow;
                    SetNextQueryTime(m, q);
                    *matchQuestion = q;
                    return uDNS_LLQ_Entire;     // uDNS_LLQ_Entire means flush stale records; assume a large effective TTL
                }
                // Note: In LLQ Event packets, the msg->h.id does not match our q->TargetQID, because in that case the msg->h.id nonce is selected by the server
                else if (opt && q->state == LLQ_Established && opt->u.llq.llqOp == kLLQOp_Event && mDNSSameOpaque64(&opt->u.llq.id, &q->id))
                {
                    mDNSu8 *ackEnd;
                    //debugf("Sending LLQ ack for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
                    InitializeDNSMessage(&m->omsg.h, msg->h.id, ResponseFlags);
                    ackEnd = putLLQ(&m->omsg, m->omsg.data, q, &opt->u.llq);
                    if (ackEnd) mDNSSendDNSMessage(m, &m->omsg, ackEnd, mDNSInterface_Any, q->LocalSocket, srcaddr, srcport, mDNSNULL, mDNSNULL, mDNSfalse);
                    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
                    debugf("uDNS_LLQ_Events: q->state == LLQ_Established msg->h.id %d q->TargetQID %d", mDNSVal16(msg->h.id), mDNSVal16(q->TargetQID));
                    *matchQuestion = q;
                    return uDNS_LLQ_Events;
                }
                if (opt && mDNSSameOpaque16(msg->h.id, q->TargetQID))
                {
                    if (q->state == LLQ_Established && opt->u.llq.llqOp == kLLQOp_Refresh && mDNSSameOpaque64(&opt->u.llq.id, &q->id) && msg->h.numAdditionals && !msg->h.numAnswers)
                    {
                        if (opt->u.llq.err != LLQErr_NoError) LogMsg("recvRefreshReply: received error %d from server", opt->u.llq.err);
                        else
                        {
                            //LogInfo("Received refresh confirmation ntries %d for %##s (%s)", q->ntries, q->qname.c, DNSTypeName(q->qtype));
                            // If we're waiting to go to sleep, then this LLQ deletion may have been the thing
                            // we were waiting for, so schedule another check to see if we can sleep now.
                            if (opt->u.llq.llqlease == 0 && m->SleepLimit) m->NextScheduledSPRetry = m->timenow;
                            GrantCacheExtensions(m, q, opt->u.llq.llqlease);
                            SetLLQTimer(m, q, &opt->u.llq);
                            q->ntries = 0;
                        }
                        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
                        *matchQuestion = q;
                        return uDNS_LLQ_Ignore;
                    }
                    if (q->state < LLQ_Established && mDNSSameAddress(srcaddr, &q->servAddr))
                    {
                        LLQ_State oldstate = q->state;
                        recvSetupResponse(m, msg->h.flags.b[1] & kDNSFlag1_RC_Mask, q, &opt->u.llq);
                        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
                        // We have a protocol anomaly here in the LLQ definition.
                        // Both the challenge packet from the server and the ack+answers packet have opt->u.llq.llqOp == kLLQOp_Setup.
                        // However, we need to treat them differently:
                        // The challenge packet has no answers in it, and tells us nothing about whether our cache entries
                        // are still valid, so this packet should not cause us to do anything that messes with our cache.
                        // The ack+answers packet gives us the whole truth, so we should handle it by updating our cache
                        // to match the answers in the packet, and only the answers in the packet.
                        *matchQuestion = q;
                        return (oldstate == LLQ_SecondaryRequest ? uDNS_LLQ_Entire : uDNS_LLQ_Ignore);
                    }
                }
            }
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }
    *matchQuestion = mDNSNULL;
    return uDNS_LLQ_Not;
}

// Stub definition of TCPSocket_struct so we can access flags field. (Rest of TCPSocket_struct is platform-dependent.)
struct TCPSocket_struct { TCPSocketFlags flags; /* ... */ };

// tcpCallback is called to handle events (e.g. connection opening and data reception) on TCP connections for
// Private DNS operations -- private queries, private LLQs, private record updates and private service updates
mDNSlocal void tcpCallback(TCPSocket *sock, void *context, mDNSBool ConnectionEstablished, mStatus err)
{
    tcpInfo_t *tcpInfo = (tcpInfo_t *)context;
    mDNSBool closed  = mDNSfalse;
    mDNS      *m       = tcpInfo->m;
    DNSQuestion *const q = tcpInfo->question;
    tcpInfo_t **backpointer =
        q                 ? &q->tcp :
        tcpInfo->rr       ? &tcpInfo->rr->tcp : mDNSNULL;
    if (backpointer && *backpointer != tcpInfo)
        LogMsg("tcpCallback: %d backpointer %p incorrect tcpInfo %p question %p rr %p",
               mDNSPlatformTCPGetFD(tcpInfo->sock), *backpointer, tcpInfo, q, tcpInfo->rr);

    if (err) goto exit;

    if (ConnectionEstablished)
    {
        mDNSu8    *end = ((mDNSu8*) &tcpInfo->request) + tcpInfo->requestLen;
        DomainAuthInfo *AuthInfo;

        // Defensive coding for <rdar://problem/5546824> Crash in mDNSResponder at GetAuthInfoForName_internal + 366
        // Don't know yet what's causing this, but at least we can be cautious and try to avoid crashing if we find our pointers in an unexpected state
        if (tcpInfo->rr && tcpInfo->rr->resrec.name != &tcpInfo->rr->namestorage)
            LogMsg("tcpCallback: ERROR: tcpInfo->rr->resrec.name %p != &tcpInfo->rr->namestorage %p",
                   tcpInfo->rr->resrec.name, &tcpInfo->rr->namestorage);
        if (tcpInfo->rr  && tcpInfo->rr->resrec.name != &tcpInfo->rr->namestorage) return;

        AuthInfo =  tcpInfo->rr  ? GetAuthInfoForName(m, tcpInfo->rr->resrec.name)         : mDNSNULL;

        // connection is established - send the message
        if (q && q->LongLived && q->state == LLQ_Established)
        {
            // Lease renewal over TCP, resulting from opening a TCP connection in sendLLQRefresh
            end = ((mDNSu8*) &tcpInfo->request) + tcpInfo->requestLen;
        }
        else if (q && q->LongLived && q->state != LLQ_Poll && !mDNSIPPortIsZero(m->LLQNAT.ExternalPort) && !mDNSIPPortIsZero(q->servPort))
        {
            // Notes:
            // If we have a NAT port mapping, ExternalPort is the external port
            // If we have a routable address so we don't need a port mapping, ExternalPort is the same as our own internal port
            // If we need a NAT port mapping but can't get one, then ExternalPort is zero
            LLQOptData llqData;         // set llq rdata
            llqData.vers  = kLLQ_Vers;
            llqData.llqOp = kLLQOp_Setup;
            llqData.err   = GetLLQEventPort(m, &tcpInfo->Addr); // We're using TCP; tell server what UDP port to send notifications to
            LogInfo("tcpCallback: eventPort %d", llqData.err);
            llqData.id    = zeroOpaque64;
            llqData.llqlease = kLLQ_DefLease;
            InitializeDNSMessage(&tcpInfo->request.h, q->TargetQID, uQueryFlags);
            end = putLLQ(&tcpInfo->request, tcpInfo->request.data, q, &llqData);
            if (!end) { LogMsg("ERROR: tcpCallback - putLLQ"); err = mStatus_UnknownErr; goto exit; }
            AuthInfo = q->AuthInfo;     // Need to add TSIG to this message
            q->ntries = 0; // Reset ntries so that tcp/tls connection failures don't affect sendChallengeResponse failures
        }
        else if (q)
        {
            // LLQ Polling mode or non-LLQ uDNS over TCP
            InitializeDNSMessage(&tcpInfo->request.h, q->TargetQID, (DNSSECQuestion(q) ? DNSSecQFlags : uQueryFlags));
            end = putQuestion(&tcpInfo->request, tcpInfo->request.data, tcpInfo->request.data + AbsoluteMaxDNSMessageData, &q->qname, q->qtype, q->qclass);
            if (DNSSECQuestion(q) && q->qDNSServer && !q->qDNSServer->cellIntf)
            {
                if (q->ProxyQuestion)
                    end = DNSProxySetAttributes(q, &tcpInfo->request.h, &tcpInfo->request, end, tcpInfo->request.data + AbsoluteMaxDNSMessageData);
                else
                    end = putDNSSECOption(&tcpInfo->request, end, tcpInfo->request.data + AbsoluteMaxDNSMessageData);
            }

            AuthInfo = q->AuthInfo;     // Need to add TSIG to this message
        }

        err = mDNSSendDNSMessage(m, &tcpInfo->request, end, mDNSInterface_Any, mDNSNULL, &tcpInfo->Addr, tcpInfo->Port, sock, AuthInfo, mDNSfalse);
        if (err) { debugf("ERROR: tcpCallback: mDNSSendDNSMessage - %d", err); err = mStatus_UnknownErr; goto exit; }

        // Record time we sent this question
        if (q)
        {
            mDNS_Lock(m);
            q->LastQTime = m->timenow;
            if (q->ThisQInterval < (256 * mDNSPlatformOneSecond))   // Now we have a TCP connection open, make sure we wait at least 256 seconds before retrying
                q->ThisQInterval = (256 * mDNSPlatformOneSecond);
            SetNextQueryTime(m, q);
            mDNS_Unlock(m);
        }
    }
    else
    {
        long n;
        if (tcpInfo->nread < 2)         // First read the two-byte length preceeding the DNS message
        {
            mDNSu8 *lenptr = (mDNSu8 *)&tcpInfo->replylen;
            n = mDNSPlatformReadTCP(sock, lenptr + tcpInfo->nread, 2 - tcpInfo->nread, &closed);
            if (n < 0)
            {
                LogMsg("ERROR: tcpCallback - attempt to read message length failed (%d)", n);
                err = mStatus_ConnFailed;
                goto exit;
            }
            else if (closed)
            {
                // It's perfectly fine for this socket to close after the first reply. The server might
                // be sending gratuitous replies using UDP and doesn't have a need to leave the TCP socket open.
                // We'll only log this event if we've never received a reply before.
                // BIND 9 appears to close an idle connection after 30 seconds.
                if (tcpInfo->numReplies == 0)
                {
                    LogMsg("ERROR: socket closed prematurely tcpInfo->nread = %d", tcpInfo->nread);
                    err = mStatus_ConnFailed;
                    goto exit;
                }
                else
                {
                    // Note that we may not be doing the best thing if an error occurs after we've sent a second request
                    // over this tcp connection.  That is, we only track whether we've received at least one response
                    // which may have been to a previous request sent over this tcp connection.
                    if (backpointer) *backpointer = mDNSNULL; // Clear client backpointer FIRST so we don't risk double-disposing our tcpInfo_t
                    DisposeTCPConn(tcpInfo);
                    return;
                }
            }

            tcpInfo->nread += n;
            if (tcpInfo->nread < 2) goto exit;

            tcpInfo->replylen = (mDNSu16)((mDNSu16)lenptr[0] << 8 | lenptr[1]);
            if (tcpInfo->replylen < sizeof(DNSMessageHeader))
            { LogMsg("ERROR: tcpCallback - length too short (%d bytes)", tcpInfo->replylen); err = mStatus_UnknownErr; goto exit; }

            tcpInfo->reply = mDNSPlatformMemAllocate(tcpInfo->replylen);
            if (!tcpInfo->reply) { LogMsg("ERROR: tcpCallback - malloc failed"); err = mStatus_NoMemoryErr; goto exit; }
        }

        n = mDNSPlatformReadTCP(sock, ((char *)tcpInfo->reply) + (tcpInfo->nread - 2), tcpInfo->replylen - (tcpInfo->nread - 2), &closed);

        if (n < 0)
        {
            LogMsg("ERROR: tcpCallback - read returned %d", n);
            err = mStatus_ConnFailed;
            goto exit;
        }
        else if (closed)
        {
            if (tcpInfo->numReplies == 0)
            {
                LogMsg("ERROR: socket closed prematurely tcpInfo->nread = %d", tcpInfo->nread);
                err = mStatus_ConnFailed;
                goto exit;
            }
            else
            {
                // Note that we may not be doing the best thing if an error occurs after we've sent a second request
                // over this tcp connection.  That is, we only track whether we've received at least one response
                // which may have been to a previous request sent over this tcp connection.
                if (backpointer) *backpointer = mDNSNULL; // Clear client backpointer FIRST so we don't risk double-disposing our tcpInfo_t
                DisposeTCPConn(tcpInfo);
                return;
            }
        }

        tcpInfo->nread += n;

        if ((tcpInfo->nread - 2) == tcpInfo->replylen)
        {
            mDNSBool tls;
            DNSMessage *reply = tcpInfo->reply;
            mDNSu8     *end   = (mDNSu8 *)tcpInfo->reply + tcpInfo->replylen;
            mDNSAddr Addr  = tcpInfo->Addr;
            mDNSIPPort Port  = tcpInfo->Port;
            mDNSIPPort srcPort = zeroIPPort;
            tcpInfo->numReplies++;
            tcpInfo->reply    = mDNSNULL;   // Detach reply buffer from tcpInfo_t, to make sure client callback can't cause it to be disposed
            tcpInfo->nread    = 0;
            tcpInfo->replylen = 0;

            // If we're going to dispose this connection, do it FIRST, before calling client callback
            // Note: Sleep code depends on us clearing *backpointer here -- it uses the clearing of rr->tcp
            // as the signal that the DNS deregistration operation with the server has completed, and the machine may now sleep
            // If we clear the tcp pointer in the question, mDNSCoreReceiveResponse cannot find a matching question. Hence
            // we store the minimal information i.e., the source port of the connection in the question itself.
            // Dereference sock before it is disposed in DisposeTCPConn below.

            if (sock->flags & kTCPSocketFlags_UseTLS) tls = mDNStrue;
            else tls = mDNSfalse;

            if (q && q->tcp) {srcPort = q->tcp->SrcPort; q->tcpSrcPort = srcPort;}

            if (backpointer)
                if (!q || !q->LongLived || m->SleepState)
                { *backpointer = mDNSNULL; DisposeTCPConn(tcpInfo); }

            mDNSCoreReceive(m, reply, end, &Addr, Port, tls ? (mDNSAddr *)1 : mDNSNULL, srcPort, 0);
            // USE CAUTION HERE: Invoking mDNSCoreReceive may have caused the environment to change, including canceling this operation itself

            mDNSPlatformMemFree(reply);
            return;
        }
    }

exit:

    if (err)
    {
        // Clear client backpointer FIRST -- that way if one of the callbacks cancels its operation
        // we won't end up double-disposing our tcpInfo_t
        if (backpointer) *backpointer = mDNSNULL;

        mDNS_Lock(m);       // Need to grab the lock to get m->timenow

        if (q)
        {
            if (q->ThisQInterval == 0)
            {
                // We get here when we fail to establish a new TCP/TLS connection that would have been used for a new LLQ request or an LLQ renewal.
                // Note that ThisQInterval is also zero when sendChallengeResponse resends the LLQ request on an extant TCP/TLS connection.
                q->LastQTime = m->timenow;
                if (q->LongLived)
                {
                    // We didn't get the chance to send our request packet before the TCP/TLS connection failed.
                    // We want to retry quickly, but want to back off exponentially in case the server is having issues.
                    // Since ThisQInterval was 0, we can't just multiply by QuestionIntervalStep, we must track the number
                    // of TCP/TLS connection failures using ntries.
                    mDNSu32 count = q->ntries + 1; // want to wait at least 1 second before retrying

                    q->ThisQInterval = InitialQuestionInterval;

                    for (; count; count--)
                        q->ThisQInterval *= QuestionIntervalStep;

                    if (q->ThisQInterval > LLQ_POLL_INTERVAL)
                        q->ThisQInterval = LLQ_POLL_INTERVAL;
                    else
                        q->ntries++;

                    LogMsg("tcpCallback: stream connection for LLQ %##s (%s) failed %d times, retrying in %d ms", q->qname.c, DNSTypeName(q->qtype), q->ntries, q->ThisQInterval);
                }
                else
                {
                    q->ThisQInterval = MAX_UCAST_POLL_INTERVAL;
                    LogMsg("tcpCallback: stream connection for %##s (%s) failed, retrying in %d ms", q->qname.c, DNSTypeName(q->qtype), q->ThisQInterval);
                }
                SetNextQueryTime(m, q);
            }
            else if (NextQSendTime(q) - m->timenow > (q->LongLived ? LLQ_POLL_INTERVAL : MAX_UCAST_POLL_INTERVAL))
            {
                // If we get an error and our next scheduled query for this question is more than the max interval from now,
                // reset the next query to ensure we wait no longer the maximum interval from now before trying again.
                q->LastQTime     = m->timenow;
                q->ThisQInterval = q->LongLived ? LLQ_POLL_INTERVAL : MAX_UCAST_POLL_INTERVAL;
                SetNextQueryTime(m, q);
                LogMsg("tcpCallback: stream connection for %##s (%s) failed, retrying in %d ms", q->qname.c, DNSTypeName(q->qtype), q->ThisQInterval);
            }

            // We're about to dispose of the TCP connection, so we must reset the state to retry over TCP/TLS
            // because sendChallengeResponse will send the query via UDP if we don't have a tcp pointer.
            // Resetting to LLQ_InitialRequest will cause uDNS_CheckCurrentQuestion to call startLLQHandshake, which
            // will attempt to establish a new tcp connection.
            if (q->LongLived && q->state == LLQ_SecondaryRequest)
                q->state = LLQ_InitialRequest;

            // ConnFailed may happen if the server sends a TCP reset or TLS fails, in which case we want to retry establishing the LLQ
            // quickly rather than switching to polling mode.  This case is handled by the above code to set q->ThisQInterval just above.
            // If the error isn't ConnFailed, then the LLQ is in bad shape, so we switch to polling mode.
            if (err != mStatus_ConnFailed)
            {
                if (q->LongLived && q->state != LLQ_Poll) StartLLQPolling(m, q);
            }
        }

        mDNS_Unlock(m);

        DisposeTCPConn(tcpInfo);
    }
}

mDNSlocal tcpInfo_t *MakeTCPConn(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end,
                                 TCPSocketFlags flags, const mDNSAddr *const Addr, const mDNSIPPort Port, domainname *hostname,
                                 DNSQuestion *const question, AuthRecord *const rr)
{
    mStatus err;
    mDNSIPPort srcport = zeroIPPort;
    tcpInfo_t *info;
    mDNSBool useBackgroundTrafficClass;

    useBackgroundTrafficClass = question ? question->UseBackgroundTrafficClass : mDNSfalse;

    if ((flags & kTCPSocketFlags_UseTLS) && (!hostname || !hostname->c[0]))
    { LogMsg("MakeTCPConn: TLS connection being setup with NULL hostname"); return mDNSNULL; }

    info = (tcpInfo_t *)mDNSPlatformMemAllocate(sizeof(tcpInfo_t));
    if (!info) { LogMsg("ERROR: MakeTCP - memallocate failed"); return(mDNSNULL); }
    mDNSPlatformMemZero(info, sizeof(tcpInfo_t));

    info->m          = m;
    info->sock       = mDNSPlatformTCPSocket(m, flags, &srcport, useBackgroundTrafficClass);
    info->requestLen = 0;
    info->question   = question;
    info->rr         = rr;
    info->Addr       = *Addr;
    info->Port       = Port;
    info->reply      = mDNSNULL;
    info->replylen   = 0;
    info->nread      = 0;
    info->numReplies = 0;
    info->SrcPort = srcport;

    if (msg)
    {
        info->requestLen = (int) (end - ((mDNSu8*)msg));
        mDNSPlatformMemCopy(&info->request, msg, info->requestLen);
    }

    if (!info->sock) { LogMsg("MakeTCPConn: unable to create TCP socket"); mDNSPlatformMemFree(info); return(mDNSNULL); }
    err = mDNSPlatformTCPConnect(info->sock, Addr, Port, hostname, (question ? question->InterfaceID : mDNSNULL), tcpCallback, info);

    // Probably suboptimal here.
    // Instead of returning mDNSNULL here on failure, we should probably invoke the callback with an error code.
    // That way clients can put all the error handling and retry/recovery code in one place,
    // instead of having to handle immediate errors in one place and async errors in another.
    // Also: "err == mStatus_ConnEstablished" probably never happens.

    // Don't need to log "connection failed" in customer builds -- it happens quite often during sleep, wake, configuration changes, etc.
    if      (err == mStatus_ConnEstablished) { tcpCallback(info->sock, info, mDNStrue, mStatus_NoError); }
    else if (err != mStatus_ConnPending    ) { LogInfo("MakeTCPConn: connection failed"); DisposeTCPConn(info); return(mDNSNULL); }
    return(info);
}

mDNSexport void DisposeTCPConn(struct tcpInfo_t *tcp)
{
    mDNSPlatformTCPCloseConnection(tcp->sock);
    if (tcp->reply) mDNSPlatformMemFree(tcp->reply);
    mDNSPlatformMemFree(tcp);
}

// Lock must be held
mDNSexport void startLLQHandshake(mDNS *m, DNSQuestion *q)
{
    if (m->LLQNAT.clientContext != mDNSNULL) // LLQNAT just started, give it some time
    {
        LogInfo("startLLQHandshake: waiting for NAT status for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        q->ThisQInterval = LLQ_POLL_INTERVAL + mDNSRandom(LLQ_POLL_INTERVAL/10);    // Retry in approx 15 minutes
        q->LastQTime = m->timenow;
        SetNextQueryTime(m, q);
        return;
    }

    // Either we don't have {PCP, NAT-PMP, UPnP/IGD} support (ExternalPort is zero) or behind a Double NAT that may or
    // may not have {PCP, NAT-PMP, UPnP/IGD} support (NATResult is non-zero)
    if (mDNSIPPortIsZero(m->LLQNAT.ExternalPort) || m->LLQNAT.Result)
    {
        LogInfo("startLLQHandshake: Cannot receive inbound packets; will poll for %##s (%s) External Port %d, NAT Result %d",
                q->qname.c, DNSTypeName(q->qtype), mDNSVal16(m->LLQNAT.ExternalPort), m->LLQNAT.Result);
        StartLLQPolling(m, q);
        return;
    }

    if (mDNSIPPortIsZero(q->servPort))
    {
        debugf("startLLQHandshake: StartGetZoneData for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        q->ThisQInterval = LLQ_POLL_INTERVAL + mDNSRandom(LLQ_POLL_INTERVAL/10);    // Retry in approx 15 minutes
        q->LastQTime     = m->timenow;
        SetNextQueryTime(m, q);
        q->servAddr = zeroAddr;
        // We know q->servPort is zero because of check above
        if (q->nta) CancelGetZoneData(m, q->nta);
        q->nta = StartGetZoneData(m, &q->qname, ZoneServiceLLQ, LLQGotZoneData, q);
        return;
    }

    if (PrivateQuery(q))
    {
        if (q->tcp) LogInfo("startLLQHandshake: Disposing existing TCP connection for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        if (q->tcp) { DisposeTCPConn(q->tcp); q->tcp = mDNSNULL; }
        if (!q->nta)
        {
            // Normally we lookup the zone data and then call this function. And we never free the zone data
            // for "PrivateQuery". But sometimes this can happen due to some race conditions. When we
            // switch networks, we might end up "Polling" the network e.g., we are behind a Double NAT.
            // When we poll, we free the zone information as we send the query to the server (See
            // PrivateQueryGotZoneData). The NAT callback (LLQNATCallback) may happen soon after that. If we
            // are still behind Double NAT, we would have returned early in this function. But we could
            // have switched to a network with no NATs and we should get the zone data again.
            LogInfo("startLLQHandshake: nta is NULL for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
            q->nta = StartGetZoneData(m, &q->qname, ZoneServiceLLQ, LLQGotZoneData, q);
            return;
        }
        else if (!q->nta->Host.c[0])
        {
            // This should not happen. If it happens, we print a log and MakeTCPConn will fail if it can't find a hostname
            LogMsg("startLLQHandshake: ERROR!!: nta non NULL for %##s (%s) but HostName %d NULL, LongLived %d", q->qname.c, DNSTypeName(q->qtype), q->nta->Host.c[0], q->LongLived);
        }
        q->tcp = MakeTCPConn(m, mDNSNULL, mDNSNULL, kTCPSocketFlags_UseTLS, &q->servAddr, q->servPort, &q->nta->Host, q, mDNSNULL);
        if (!q->tcp)
            q->ThisQInterval = mDNSPlatformOneSecond * 5;   // If TCP failed (transient networking glitch) try again in five seconds
        else
        {
            q->state         = LLQ_SecondaryRequest;        // Right now, for private DNS, we skip the four-way LLQ handshake
            q->ReqLease      = kLLQ_DefLease;
            q->ThisQInterval = 0;
        }
        q->LastQTime     = m->timenow;
        SetNextQueryTime(m, q);
    }
    else
    {
        debugf("startLLQHandshake: m->AdvertisedV4 %#a%s Server %#a:%d%s %##s (%s)",
               &m->AdvertisedV4,                     mDNSv4AddrIsRFC1918(&m->AdvertisedV4.ip.v4) ? " (RFC 1918)" : "",
               &q->servAddr, mDNSVal16(q->servPort), mDNSAddrIsRFC1918(&q->servAddr)             ? " (RFC 1918)" : "",
               q->qname.c, DNSTypeName(q->qtype));

        if (q->ntries++ >= kLLQ_MAX_TRIES)
        {
            LogMsg("startLLQHandshake: %d failed attempts for LLQ %##s Polling.", kLLQ_MAX_TRIES, q->qname.c);
            StartLLQPolling(m, q);
        }
        else
        {
            mDNSu8 *end;
            LLQOptData llqData;

            // set llq rdata
            llqData.vers  = kLLQ_Vers;
            llqData.llqOp = kLLQOp_Setup;
            llqData.err   = LLQErr_NoError; // Don't need to tell server UDP notification port when sending over UDP
            llqData.id    = zeroOpaque64;
            llqData.llqlease = kLLQ_DefLease;

            InitializeDNSMessage(&m->omsg.h, q->TargetQID, uQueryFlags);
            end = putLLQ(&m->omsg, m->omsg.data, q, &llqData);
            if (!end) { LogMsg("ERROR: startLLQHandshake - putLLQ"); StartLLQPolling(m,q); return; }

            mDNSSendDNSMessage(m, &m->omsg, end, mDNSInterface_Any, q->LocalSocket, &q->servAddr, q->servPort, mDNSNULL, mDNSNULL, mDNSfalse);

            // update question state
            q->state         = LLQ_InitialRequest;
            q->ReqLease      = kLLQ_DefLease;
            q->ThisQInterval = (kLLQ_INIT_RESEND * mDNSPlatformOneSecond);
            q->LastQTime     = m->timenow;
            SetNextQueryTime(m, q);
        }
    }
}

// forward declaration so GetServiceTarget can do reverse lookup if needed
mDNSlocal void GetStaticHostname(mDNS *m);

mDNSexport const domainname *GetServiceTarget(mDNS *m, AuthRecord *const rr)
{
    debugf("GetServiceTarget %##s", rr->resrec.name->c);

    if (!rr->AutoTarget)        // If not automatically tracking this host's current name, just return the existing target
        return(&rr->resrec.rdata->u.srv.target);
    else
    {
#if APPLE_OSX_mDNSResponder
        DomainAuthInfo *AuthInfo = GetAuthInfoForName_internal(m, rr->resrec.name);
        if (AuthInfo && AuthInfo->AutoTunnel)
        {
            StartServerTunnel(m, AuthInfo);
            if (AuthInfo->AutoTunnelHostRecord.namestorage.c[0] == 0) return(mDNSNULL);
            debugf("GetServiceTarget: Returning %##s", AuthInfo->AutoTunnelHostRecord.namestorage.c);
            return(&AuthInfo->AutoTunnelHostRecord.namestorage);
        }
        else
#endif // APPLE_OSX_mDNSResponder
        {
            const int srvcount = CountLabels(rr->resrec.name);
            HostnameInfo *besthi = mDNSNULL, *hi;
            int best = 0;
            for (hi = m->Hostnames; hi; hi = hi->next)
                if (hi->arv4.state == regState_Registered || hi->arv4.state == regState_Refresh ||
                    hi->arv6.state == regState_Registered || hi->arv6.state == regState_Refresh)
                {
                    int x, hostcount = CountLabels(&hi->fqdn);
                    for (x = hostcount < srvcount ? hostcount : srvcount; x > 0 && x > best; x--)
                        if (SameDomainName(SkipLeadingLabels(rr->resrec.name, srvcount - x), SkipLeadingLabels(&hi->fqdn, hostcount - x)))
                        { best = x; besthi = hi; }
                }

            if (besthi) return(&besthi->fqdn);
        }
        if (m->StaticHostname.c[0]) return(&m->StaticHostname);
        else GetStaticHostname(m); // asynchronously do reverse lookup for primary IPv4 address
        LogInfo("GetServiceTarget: Returning NULL for %s", ARDisplayString(m, rr));
        return(mDNSNULL);
    }
}

mDNSlocal const domainname *PUBLIC_UPDATE_SERVICE_TYPE  = (const domainname*)"\x0B_dns-update"     "\x04_udp";
mDNSlocal const domainname *PUBLIC_LLQ_SERVICE_TYPE     = (const domainname*)"\x08_dns-llq"        "\x04_udp";

mDNSlocal const domainname *PRIVATE_UPDATE_SERVICE_TYPE = (const domainname*)"\x0F_dns-update-tls" "\x04_tcp";
mDNSlocal const domainname *PRIVATE_QUERY_SERVICE_TYPE  = (const domainname*)"\x0E_dns-query-tls"  "\x04_tcp";
mDNSlocal const domainname *PRIVATE_LLQ_SERVICE_TYPE    = (const domainname*)"\x0C_dns-llq-tls"    "\x04_tcp";

#define ZoneDataSRV(X) ( \
        (X)->ZoneService == ZoneServiceUpdate ? ((X)->ZonePrivate ? PRIVATE_UPDATE_SERVICE_TYPE : PUBLIC_UPDATE_SERVICE_TYPE) : \
        (X)->ZoneService == ZoneServiceQuery  ? ((X)->ZonePrivate ? PRIVATE_QUERY_SERVICE_TYPE  : (const domainname*)""     ) : \
        (X)->ZoneService == ZoneServiceLLQ    ? ((X)->ZonePrivate ? PRIVATE_LLQ_SERVICE_TYPE    : PUBLIC_LLQ_SERVICE_TYPE   ) : (const domainname*)"")

// Forward reference: GetZoneData_StartQuery references GetZoneData_QuestionCallback, and
// GetZoneData_QuestionCallback calls GetZoneData_StartQuery
mDNSlocal mStatus GetZoneData_StartQuery(mDNS *const m, ZoneData *zd, mDNSu16 qtype);

// GetZoneData_QuestionCallback is called from normal client callback context (core API calls allowed)
mDNSlocal void GetZoneData_QuestionCallback(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    ZoneData *zd = (ZoneData*)question->QuestionContext;

    debugf("GetZoneData_QuestionCallback: %s %s", AddRecord ? "Add" : "Rmv", RRDisplayString(m, answer));

    if (!AddRecord) return;                                             // Don't care about REMOVE events
    if (AddRecord == QC_addnocache && answer->rdlength == 0) return;    // Don't care about transient failure indications
    if (answer->rrtype != question->qtype) return;                      // Don't care about CNAMEs

    if (answer->rrtype == kDNSType_SOA)
    {
        debugf("GetZoneData GOT SOA %s", RRDisplayString(m, answer));
        mDNS_StopQuery(m, question);
        if (question->ThisQInterval != -1)
            LogMsg("GetZoneData_QuestionCallback: Question %##s (%s) ThisQInterval %d not -1", question->qname.c, DNSTypeName(question->qtype), question->ThisQInterval);
        if (answer->rdlength)
        {
            AssignDomainName(&zd->ZoneName, answer->name);
            zd->ZoneClass = answer->rrclass;
            AssignDomainName(&zd->question.qname, &zd->ZoneName);
            GetZoneData_StartQuery(m, zd, kDNSType_SRV);
        }
        else if (zd->CurrentSOA->c[0])
        {
            DomainAuthInfo *AuthInfo = GetAuthInfoForName(m, zd->CurrentSOA);
            if (AuthInfo && AuthInfo->AutoTunnel)
            {
                // To keep the load on the server down, we don't chop down on
                // SOA lookups for AutoTunnels
                LogInfo("GetZoneData_QuestionCallback: not chopping labels for %##s", zd->CurrentSOA->c);
                zd->ZoneDataCallback(m, mStatus_NoSuchNameErr, zd);
            }
            else
            {
                zd->CurrentSOA = (domainname *)(zd->CurrentSOA->c + zd->CurrentSOA->c[0]+1);
                AssignDomainName(&zd->question.qname, zd->CurrentSOA);
                GetZoneData_StartQuery(m, zd, kDNSType_SOA);
            }
        }
        else
        {
            LogInfo("GetZoneData recursed to root label of %##s without finding SOA", zd->ChildName.c);
            zd->ZoneDataCallback(m, mStatus_NoSuchNameErr, zd);
        }
    }
    else if (answer->rrtype == kDNSType_SRV)
    {
        debugf("GetZoneData GOT SRV %s", RRDisplayString(m, answer));
        mDNS_StopQuery(m, question);
        if (question->ThisQInterval != -1)
            LogMsg("GetZoneData_QuestionCallback: Question %##s (%s) ThisQInterval %d not -1", question->qname.c, DNSTypeName(question->qtype), question->ThisQInterval);
// Right now we don't want to fail back to non-encrypted operations
// If the AuthInfo has the AutoTunnel field set, then we want private or nothing
// <rdar://problem/5687667> BTMM: Don't fallback to unencrypted operations when SRV lookup fails
#if 0
        if (!answer->rdlength && zd->ZonePrivate && zd->ZoneService != ZoneServiceQuery)
        {
            zd->ZonePrivate = mDNSfalse;    // Causes ZoneDataSRV() to yield a different SRV name when building the query
            GetZoneData_StartQuery(m, zd, kDNSType_SRV);        // Try again, non-private this time
        }
        else
#endif
        {
            if (answer->rdlength)
            {
                AssignDomainName(&zd->Host, &answer->rdata->u.srv.target);
                zd->Port = answer->rdata->u.srv.port;
                AssignDomainName(&zd->question.qname, &zd->Host);
                GetZoneData_StartQuery(m, zd, kDNSType_A);
            }
            else
            {
                zd->ZonePrivate = mDNSfalse;
                zd->Host.c[0] = 0;
                zd->Port = zeroIPPort;
                zd->Addr = zeroAddr;
                zd->ZoneDataCallback(m, mStatus_NoError, zd);
            }
        }
    }
    else if (answer->rrtype == kDNSType_A)
    {
        debugf("GetZoneData GOT A %s", RRDisplayString(m, answer));
        mDNS_StopQuery(m, question);
        if (question->ThisQInterval != -1)
            LogMsg("GetZoneData_QuestionCallback: Question %##s (%s) ThisQInterval %d not -1", question->qname.c, DNSTypeName(question->qtype), question->ThisQInterval);
        zd->Addr.type  = mDNSAddrType_IPv4;
        zd->Addr.ip.v4 = (answer->rdlength == 4) ? answer->rdata->u.ipv4 : zerov4Addr;
        // In order to simulate firewalls blocking our outgoing TCP connections, returning immediate ICMP errors or TCP resets,
        // the code below will make us try to connect to loopback, resulting in an immediate "port unreachable" failure.
        // This helps us test to make sure we handle this case gracefully
        // <rdar://problem/5607082> BTMM: mDNSResponder taking 100 percent CPU after upgrading to 10.5.1
#if 0
        zd->Addr.ip.v4.b[0] = 127;
        zd->Addr.ip.v4.b[1] = 0;
        zd->Addr.ip.v4.b[2] = 0;
        zd->Addr.ip.v4.b[3] = 1;
#endif
        // The caller needs to free the memory when done with zone data
        zd->ZoneDataCallback(m, mStatus_NoError, zd);
    }
}

// GetZoneData_StartQuery is called from normal client context (lock not held, or client callback)
mDNSlocal mStatus GetZoneData_StartQuery(mDNS *const m, ZoneData *zd, mDNSu16 qtype)
{
    if (qtype == kDNSType_SRV)
    {
        AssignDomainName(&zd->question.qname, ZoneDataSRV(zd));
        AppendDomainName(&zd->question.qname, &zd->ZoneName);
        debugf("lookupDNSPort %##s", zd->question.qname.c);
    }

    // CancelGetZoneData can get called at any time. We should stop the question if it has not been
    // stopped already. A value of -1 for ThisQInterval indicates that the question is not active
    // yet.
    zd->question.ThisQInterval       = -1;
    zd->question.InterfaceID         = mDNSInterface_Any;
    zd->question.flags               = 0;
    zd->question.Target              = zeroAddr;
    //zd->question.qname.c[0]        = 0;			// Already set
    zd->question.qtype               = qtype;
    zd->question.qclass              = kDNSClass_IN;
    zd->question.LongLived           = mDNSfalse;
    zd->question.ExpectUnique        = mDNStrue;
    zd->question.ForceMCast          = mDNSfalse;
    zd->question.ReturnIntermed      = mDNStrue;
    zd->question.SuppressUnusable    = mDNSfalse;
    zd->question.SearchListIndex     = 0;
    zd->question.AppendSearchDomains = 0;
    zd->question.RetryWithSearchDomains = mDNSfalse;
    zd->question.TimeoutQuestion     = 0;
    zd->question.WakeOnResolve       = 0;
    zd->question.UseBackgroundTrafficClass = mDNSfalse;
    zd->question.ValidationRequired = 0;
    zd->question.ValidatingResponse = 0;
    zd->question.ProxyQuestion      = 0;
    zd->question.qnameOrig           = mDNSNULL;
    zd->question.AnonInfo            = mDNSNULL;
    zd->question.pid                 = mDNSPlatformGetPID();
    zd->question.QuestionCallback    = GetZoneData_QuestionCallback;
    zd->question.QuestionContext     = zd;

    //LogMsg("GetZoneData_StartQuery %##s (%s) %p", zd->question.qname.c, DNSTypeName(zd->question.qtype), zd->question.Private);
    return(mDNS_StartQuery(m, &zd->question));
}

// StartGetZoneData is an internal routine (i.e. must be called with the lock already held)
mDNSexport ZoneData *StartGetZoneData(mDNS *const m, const domainname *const name, const ZoneService target, ZoneDataCallback callback, void *ZoneDataContext)
{
    DomainAuthInfo *AuthInfo = GetAuthInfoForName_internal(m, name);
    int initialskip = (AuthInfo && AuthInfo->AutoTunnel) ? DomainNameLength(name) - DomainNameLength(&AuthInfo->domain) : 0;
    ZoneData *zd = (ZoneData*)mDNSPlatformMemAllocate(sizeof(ZoneData));
    if (!zd) { LogMsg("ERROR: StartGetZoneData - mDNSPlatformMemAllocate failed"); return mDNSNULL; }
    mDNSPlatformMemZero(zd, sizeof(ZoneData));
    AssignDomainName(&zd->ChildName, name);
    zd->ZoneService      = target;
    zd->CurrentSOA       = (domainname *)(&zd->ChildName.c[initialskip]);
    zd->ZoneName.c[0]    = 0;
    zd->ZoneClass        = 0;
    zd->Host.c[0]        = 0;
    zd->Port             = zeroIPPort;
    zd->Addr             = zeroAddr;
    zd->ZonePrivate      = AuthInfo && AuthInfo->AutoTunnel ? mDNStrue : mDNSfalse;
    zd->ZoneDataCallback = callback;
    zd->ZoneDataContext  = ZoneDataContext;

    zd->question.QuestionContext = zd;

    mDNS_DropLockBeforeCallback();      // GetZoneData_StartQuery expects to be called from a normal callback, so we emulate that here
    if (AuthInfo && AuthInfo->AutoTunnel && !mDNSIPPortIsZero(AuthInfo->port))
    {
        LogInfo("StartGetZoneData: Bypassing SOA, SRV query for %##s", AuthInfo->domain.c);
        // We bypass SOA and SRV queries if we know the hostname and port already from the configuration.
        // Today this is only true for AutoTunnel. As we bypass, we need to infer a few things:
        //
        // 1. Zone name is the same as the AuthInfo domain
        // 2. ZoneClass is kDNSClass_IN which should be a safe assumption
        //
        // If we want to make this bypass mechanism work for non-AutoTunnels also, (1) has to hold
        // good. Otherwise, it has to be configured also.

        AssignDomainName(&zd->ZoneName, &AuthInfo->domain);
        zd->ZoneClass = kDNSClass_IN;
        AssignDomainName(&zd->Host, &AuthInfo->hostname);
        zd->Port = AuthInfo->port;
        AssignDomainName(&zd->question.qname, &zd->Host);
        GetZoneData_StartQuery(m, zd, kDNSType_A);
    }
    else
    {
        if (AuthInfo && AuthInfo->AutoTunnel) LogInfo("StartGetZoneData: Not Bypassing SOA, SRV query for %##s", AuthInfo->domain.c);
        AssignDomainName(&zd->question.qname, zd->CurrentSOA);
        GetZoneData_StartQuery(m, zd, kDNSType_SOA);
    }
    mDNS_ReclaimLockAfterCallback();

    return zd;
}

// Returns if the question is a GetZoneData question. These questions are special in
// that they are created internally while resolving a private query or LLQs.
mDNSexport mDNSBool IsGetZoneDataQuestion(DNSQuestion *q)
{
    if (q->QuestionCallback == GetZoneData_QuestionCallback) return(mDNStrue);
    else return(mDNSfalse);
}

// GetZoneData queries are a special case -- even if we have a key for them, we don't do them privately,
// because that would result in an infinite loop (i.e. to do a private query we first need to get
// the _dns-query-tls SRV record for the zone, and we can't do *that* privately because to do so
// we'd need to already know the _dns-query-tls SRV record.
// Also, as a general rule, we never do SOA queries privately
mDNSexport DomainAuthInfo *GetAuthInfoForQuestion(mDNS *m, const DNSQuestion *const q)  // Must be called with lock held
{
    if (q->QuestionCallback == GetZoneData_QuestionCallback) return(mDNSNULL);
    if (q->qtype            == kDNSType_SOA                ) return(mDNSNULL);
    return(GetAuthInfoForName_internal(m, &q->qname));
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - host name and interface management
#endif

mDNSlocal void SendRecordRegistration(mDNS *const m, AuthRecord *rr);
mDNSlocal void SendRecordDeregistration(mDNS *m, AuthRecord *rr);
mDNSlocal mDNSBool IsRecordMergeable(mDNS *const m, AuthRecord *rr, mDNSs32 time);

// When this function is called, service record is already deregistered. We just
// have to deregister the PTR and TXT records.
mDNSlocal void UpdateAllServiceRecords(mDNS *const m, AuthRecord *rr, mDNSBool reg)
{
    AuthRecord *r, *srvRR;

    if (rr->resrec.rrtype != kDNSType_SRV) { LogMsg("UpdateAllServiceRecords:ERROR!! ResourceRecord not a service record %s", ARDisplayString(m, rr)); return; }

    if (reg && rr->state == regState_NoTarget) { LogMsg("UpdateAllServiceRecords:ERROR!! SRV record %s in noTarget state during registration", ARDisplayString(m, rr)); return; }

    LogInfo("UpdateAllServiceRecords: ResourceRecord %s", ARDisplayString(m, rr));

    for (r = m->ResourceRecords; r; r=r->next)
    {
        if (!AuthRecord_uDNS(r)) continue;
        srvRR = mDNSNULL;
        if (r->resrec.rrtype == kDNSType_PTR)
            srvRR = r->Additional1;
        else if (r->resrec.rrtype == kDNSType_TXT)
            srvRR = r->DependentOn;
        if (srvRR && srvRR->resrec.rrtype != kDNSType_SRV)
            LogMsg("UpdateAllServiceRecords: ERROR!! Resource record %s wrong, expecting SRV type", ARDisplayString(m, srvRR));
        if (srvRR == rr)
        {
            if (!reg)
            {
                LogInfo("UpdateAllServiceRecords: deregistering %s", ARDisplayString(m, r));
                r->SRVChanged = mDNStrue;
                r->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
                r->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
                r->state = regState_DeregPending;
            }
            else
            {
                // Clearing SRVchanged is a safety measure. If our pevious dereg never
                // came back and we had a target change, we are starting fresh
                r->SRVChanged = mDNSfalse;
                // if it is already registered or in the process of registering, then don't
                // bother re-registering. This happens today for non-BTMM domains where the
                // TXT and PTR get registered before SRV records because of the delay in
                // getting the port mapping. There is no point in re-registering the TXT
                // and PTR records.
                if ((r->state == regState_Registered) ||
                    (r->state == regState_Pending && r->nta && !mDNSIPv4AddressIsZero(r->nta->Addr.ip.v4)))
                    LogInfo("UpdateAllServiceRecords: not registering %s, state %d", ARDisplayString(m, r), r->state);
                else
                {
                    LogInfo("UpdateAllServiceRecords: registering %s, state %d", ARDisplayString(m, r), r->state);
                    ActivateUnicastRegistration(m, r);
                }
            }
        }
    }
}

// Called in normal client context (lock not held)
// Currently only supports SRV records for nat mapping
mDNSlocal void CompleteRecordNatMap(mDNS *m, NATTraversalInfo *n)
{
    const domainname *target;
    domainname *srvt;
    AuthRecord *rr = (AuthRecord *)n->clientContext;
    debugf("SRVNatMap complete %.4a IntPort %u ExternalPort %u NATLease %u", &n->ExternalAddress, mDNSVal16(n->IntPort), mDNSVal16(n->ExternalPort), n->NATLease);

    if (!rr) { LogMsg("CompleteRecordNatMap called with unknown AuthRecord object"); return; }
    if (!n->NATLease) { LogMsg("CompleteRecordNatMap No NATLease for %s", ARDisplayString(m, rr)); return; }

    if (rr->resrec.rrtype != kDNSType_SRV) {LogMsg("CompleteRecordNatMap: Not a service record %s", ARDisplayString(m, rr)); return; }

    if (rr->resrec.RecordType == kDNSRecordTypeDeregistering) { LogInfo("CompleteRecordNatMap called for %s, Service deregistering", ARDisplayString(m, rr)); return; }

    if (rr->state == regState_DeregPending) { LogInfo("CompleteRecordNatMap called for %s, record in DeregPending", ARDisplayString(m, rr)); return; }

    // As we free the zone info after registering/deregistering with the server (See hndlRecordUpdateReply),
    // we need to restart the get zone data and nat mapping request to get the latest mapping result as we can't handle it
    // at this moment. Restart from the beginning.
    if (!rr->nta || mDNSIPv4AddressIsZero(rr->nta->Addr.ip.v4))
    {
        LogInfo("CompleteRecordNatMap called for %s but no zone information!", ARDisplayString(m, rr));
        // We need to clear out the NATinfo state so that it will result in re-acquiring the mapping
        // and hence this callback called again.
        if (rr->NATinfo.clientContext)
        {
            mDNS_StopNATOperation_internal(m, &rr->NATinfo);
            rr->NATinfo.clientContext = mDNSNULL;
        }
        rr->state = regState_Pending;
        rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
        rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
        return;
    }

    mDNS_Lock(m);
    // Reevaluate the target always as Target could have changed while
    // we were getting the port mapping (See UpdateOneSRVRecord)
    target = GetServiceTarget(m, rr);
    srvt = GetRRDomainNameTarget(&rr->resrec);
    if (!target || target->c[0] == 0 || mDNSIPPortIsZero(n->ExternalPort))
    {
        if (target && target->c[0])
            LogInfo("CompleteRecordNatMap - Target %##s for ResourceRecord %##s, ExternalPort %d", target->c, rr->resrec.name->c, mDNSVal16(n->ExternalPort));
        else
            LogInfo("CompleteRecordNatMap - no target for %##s, ExternalPort %d", rr->resrec.name->c, mDNSVal16(n->ExternalPort));
        if (srvt) srvt->c[0] = 0;
        rr->state = regState_NoTarget;
        rr->resrec.rdlength = rr->resrec.rdestimate = 0;
        mDNS_Unlock(m);
        UpdateAllServiceRecords(m, rr, mDNSfalse);
        return;
    }
    LogInfo("CompleteRecordNatMap - Target %##s for ResourceRecord %##s, ExternalPort %d", target->c, rr->resrec.name->c, mDNSVal16(n->ExternalPort));
    // This function might get called multiple times during a network transition event. Previosuly, we could
    // have put the SRV record in NoTarget state above and deregistered all the other records. When this
    // function gets called again with a non-zero ExternalPort, we need to set the target and register the
    // other records again.
    if (srvt && !SameDomainName(srvt, target))
    {
        AssignDomainName(srvt, target);
        SetNewRData(&rr->resrec, mDNSNULL, 0);      // Update rdlength, rdestimate, rdatahash
    }

    // SRVChanged is set when when the target of the SRV record changes (See UpdateOneSRVRecord).
    // As a result of the target change, we might register just that SRV Record if it was
    // previously registered and we have a new target OR deregister SRV (and the associated
    // PTR/TXT records) if we don't have a target anymore. When we get a response from the server,
    // SRVChanged state tells that we registered/deregistered because of a target change
    // and hence handle accordingly e.g., if we deregistered, put the records in NoTarget state OR
    // if we registered then put it in Registered state.
    //
    // Here, we are registering all the records again from the beginning. Treat this as first time
    // registration rather than a temporary target change.
    rr->SRVChanged = mDNSfalse;

    // We want IsRecordMergeable to check whether it is a record whose update can be
    // sent with others. We set the time before we call IsRecordMergeable, so that
    // it does not fail this record based on time. We are interested in other checks
    // at this time
    rr->state = regState_Pending;
    rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
    rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
    if (IsRecordMergeable(m, rr, m->timenow + MERGE_DELAY_TIME))
        // Delay the record registration by MERGE_DELAY_TIME so that we can merge them
        // into one update
        rr->LastAPTime += MERGE_DELAY_TIME;
    mDNS_Unlock(m);
    // We call this always even though it may not be necessary always e.g., normal registration
    // process where TXT and PTR gets registered followed by the SRV record after it gets
    // the port mapping. In that case, UpdateAllServiceRecords handles the optimization. The
    // update of TXT and PTR record is required if we entered noTargetState before as explained
    // above.
    UpdateAllServiceRecords(m, rr, mDNStrue);
}

mDNSlocal void StartRecordNatMap(mDNS *m, AuthRecord *rr)
{
    const mDNSu8 *p;
    mDNSu8 protocol;

    if (rr->resrec.rrtype != kDNSType_SRV)
    {
        LogInfo("StartRecordNatMap: Resource Record %##s type %d, not supported", rr->resrec.name->c, rr->resrec.rrtype);
        return;
    }
    p = rr->resrec.name->c;
    //Assume <Service Instance>.<App Protocol>.<Transport protocol>.<Name>
    // Skip the first two labels to get to the transport protocol
    if (p[0]) p += 1 + p[0];
    if (p[0]) p += 1 + p[0];
    if      (SameDomainLabel(p, (mDNSu8 *)"\x4" "_tcp")) protocol = NATOp_MapTCP;
    else if (SameDomainLabel(p, (mDNSu8 *)"\x4" "_udp")) protocol = NATOp_MapUDP;
    else { LogMsg("StartRecordNatMap: could not determine transport protocol of service %##s", rr->resrec.name->c); return; }

    //LogMsg("StartRecordNatMap: clientContext %p IntPort %d srv.port %d %s",
    //	rr->NATinfo.clientContext, mDNSVal16(rr->NATinfo.IntPort), mDNSVal16(rr->resrec.rdata->u.srv.port), ARDisplayString(m, rr));
    if (rr->NATinfo.clientContext) mDNS_StopNATOperation_internal(m, &rr->NATinfo);
    rr->NATinfo.Protocol       = protocol;

    // Shouldn't be trying to set IntPort here --
    // BuildUpdateMessage overwrites srs->RR_SRV.resrec.rdata->u.srv.port with external (mapped) port number
    rr->NATinfo.IntPort        = rr->resrec.rdata->u.srv.port;
    rr->NATinfo.RequestedPort  = rr->resrec.rdata->u.srv.port;
    rr->NATinfo.NATLease       = 0;     // Request default lease
    rr->NATinfo.clientCallback = CompleteRecordNatMap;
    rr->NATinfo.clientContext  = rr;
    mDNS_StartNATOperation_internal(m, &rr->NATinfo);
}

// Unlink an Auth Record from the m->ResourceRecords list.
// When a resource record enters regState_NoTarget initially, mDNS_Register_internal
// does not initialize completely e.g., it cannot check for duplicates etc. The resource
// record is temporarily left in the ResourceRecords list so that we can initialize later
// when the target is resolvable. Similarly, when host name changes, we enter regState_NoTarget
// and we do the same.

// This UnlinkResourceRecord routine is very worrying. It bypasses all the normal cleanup performed
// by mDNS_Deregister_internal and just unceremoniously cuts the record from the active list.
// This is why re-regsitering this record was producing syslog messages like this:
// "Error! Tried to add a NAT traversal that's already in the active list"
// Right now UnlinkResourceRecord is fortunately only called by RegisterAllServiceRecords,
// which then immediately calls mDNS_Register_internal to re-register the record, which probably
// masked more serious problems. Any other use of UnlinkResourceRecord is likely to lead to crashes.
// For now we'll workaround that specific problem by explicitly calling mDNS_StopNATOperation_internal,
// but long-term we should either stop cancelling the record registration and then re-registering it,
// or if we really do need to do this for some reason it should be done via the usual
// mDNS_Deregister_internal path instead of just cutting the record from the list.

mDNSlocal mStatus UnlinkResourceRecord(mDNS *const m, AuthRecord *const rr)
{
    AuthRecord **list = &m->ResourceRecords;
    while (*list && *list != rr) list = &(*list)->next;
    if (*list)
    {
        *list = rr->next;
        rr->next = mDNSNULL;

        // Temporary workaround to cancel any active NAT mapping operation
        if (rr->NATinfo.clientContext)
        {
            mDNS_StopNATOperation_internal(m, &rr->NATinfo);
            rr->NATinfo.clientContext = mDNSNULL;
            if (rr->resrec.rrtype == kDNSType_SRV) rr->resrec.rdata->u.srv.port = rr->NATinfo.IntPort;
        }

        return(mStatus_NoError);
    }
    LogMsg("UnlinkResourceRecord:ERROR!! - no such active record %##s", rr->resrec.name->c);
    return(mStatus_NoSuchRecord);
}

// We need to go through mDNS_Register again as we did not complete the
// full initialization last time e.g., duplicate checks.
// After we register, we will be in regState_GetZoneData.
mDNSlocal void RegisterAllServiceRecords(mDNS *const m, AuthRecord *rr)
{
    LogInfo("RegisterAllServiceRecords: Service Record %##s", rr->resrec.name->c);
    // First Register the service record, we do this differently from other records because
    // when it entered NoTarget state, it did not go through complete initialization
    rr->SRVChanged = mDNSfalse;
    UnlinkResourceRecord(m, rr);
    mDNS_Register_internal(m, rr);
    // Register the other records
    UpdateAllServiceRecords(m, rr, mDNStrue);
}

// Called with lock held
mDNSlocal void UpdateOneSRVRecord(mDNS *m, AuthRecord *rr)
{
    // Target change if:
    // We have a target and were previously waiting for one, or
    // We had a target and no longer do, or
    // The target has changed

    domainname *curtarget = &rr->resrec.rdata->u.srv.target;
    const domainname *const nt = GetServiceTarget(m, rr);
    const domainname *const newtarget = nt ? nt : (domainname*)"";
    mDNSBool TargetChanged = (newtarget->c[0] && rr->state == regState_NoTarget) || !SameDomainName(curtarget, newtarget);
    mDNSBool HaveZoneData  = rr->nta && !mDNSIPv4AddressIsZero(rr->nta->Addr.ip.v4);

    // Nat state change if:
    // We were behind a NAT, and now we are behind a new NAT, or
    // We're not behind a NAT but our port was previously mapped to a different external port
    // We were not behind a NAT and now we are

    mDNSIPPort port        = rr->resrec.rdata->u.srv.port;
    mDNSBool NowNeedNATMAP = (rr->AutoTarget == Target_AutoHostAndNATMAP && !mDNSIPPortIsZero(port) && mDNSv4AddrIsRFC1918(&m->AdvertisedV4.ip.v4) && rr->nta && !mDNSAddrIsRFC1918(&rr->nta->Addr));
    mDNSBool WereBehindNAT = (rr->NATinfo.clientContext != mDNSNULL);
    mDNSBool PortWasMapped = (rr->NATinfo.clientContext && !mDNSSameIPPort(rr->NATinfo.RequestedPort, port));       // I think this is always false -- SC Sept 07
    mDNSBool NATChanged    = (!WereBehindNAT && NowNeedNATMAP) || (!NowNeedNATMAP && PortWasMapped);

    (void)HaveZoneData; //unused

    LogInfo("UpdateOneSRVRecord: Resource Record %s TargetChanged %d, NewTarget %##s", ARDisplayString(m, rr), TargetChanged, nt->c);

    debugf("UpdateOneSRVRecord: %##s newtarget %##s TargetChanged %d HaveZoneData %d port %d NowNeedNATMAP %d WereBehindNAT %d PortWasMapped %d NATChanged %d",
           rr->resrec.name->c, newtarget,
           TargetChanged, HaveZoneData, mDNSVal16(port), NowNeedNATMAP, WereBehindNAT, PortWasMapped, NATChanged);

    mDNS_CheckLock(m);

    if (!TargetChanged && !NATChanged) return;

    // If we are deregistering the record, then ignore any NAT/Target change.
    if (rr->resrec.RecordType == kDNSRecordTypeDeregistering)
    {
        LogInfo("UpdateOneSRVRecord: Deregistering record, Ignoring TargetChanged %d, NATChanged %d for %##s, state %d", TargetChanged, NATChanged,
                rr->resrec.name->c, rr->state);
        return;
    }

    if (newtarget)
        LogInfo("UpdateOneSRVRecord: TargetChanged %d, NATChanged %d for %##s, state %d, newtarget %##s", TargetChanged, NATChanged, rr->resrec.name->c, rr->state, newtarget->c);
    else
        LogInfo("UpdateOneSRVRecord: TargetChanged %d, NATChanged %d for %##s, state %d, null newtarget", TargetChanged, NATChanged, rr->resrec.name->c, rr->state);
    switch(rr->state)
    {
    case regState_NATMap:
        // In these states, the SRV has either not yet been registered (it will get up-to-date information when it is)
        // or is in the process of, or has already been, deregistered. This assumes that whenever we transition out
        // of this state, we need to look at the target again.
        return;

    case regState_UpdatePending:
        // We are getting a Target change/NAT change while the SRV record is being updated ?
        // let us not do anything for now.
        return;

    case regState_NATError:
        if (!NATChanged) return;
    // if nat changed, register if we have a target (below)

    case regState_NoTarget:
        if (!newtarget->c[0])
        {
            LogInfo("UpdateOneSRVRecord: No target yet for Resource Record %s", ARDisplayString(m, rr));
            return;
        }
        RegisterAllServiceRecords(m, rr);
        return;
    case regState_DeregPending:
    // We are in DeregPending either because the service was deregistered from above or we handled
    // a NAT/Target change before and sent the deregistration below. There are a few race conditions
    // possible
    //
    // 1. We are handling a second NAT/Target change while the first dereg is in progress. It is possible
    //    that first dereg never made it through because there was no network connectivity e.g., disconnecting
    //    from network triggers this function due to a target change and later connecting to the network
    //    retriggers this function but the deregistration never made it through yet. Just fall through.
    //    If there is a target register otherwise deregister.
    //
    // 2. While we sent the dereg during a previous NAT/Target change, uDNS_DeregisterRecord gets
    //    called as part of service deregistration. When the response comes back, we call
    //    CompleteDeregistration rather than handle NAT/Target change because the record is in
    //    kDNSRecordTypeDeregistering state.
    //
    // 3. If the upper layer deregisters the service, we check for kDNSRecordTypeDeregistering both
    //    here in this function to avoid handling NAT/Target change and in hndlRecordUpdateReply to call
    //    CompleteDeregistration instead of handling NAT/Target change. Hence, we are not concerned
    //    about that case here.
    //
    // We just handle case (1) by falling through
    case regState_Pending:
    case regState_Refresh:
    case regState_Registered:
        // target or nat changed.  deregister service.  upon completion, we'll look for a new target
        rr->SRVChanged = mDNStrue;
        rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
        rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
        if (newtarget->c[0])
        {
            LogInfo("UpdateOneSRVRecord: SRV record changed for service %##s, registering with new target %##s",
                    rr->resrec.name->c, newtarget->c);
            rr->state = regState_Pending;
        }
        else
        {
            LogInfo("UpdateOneSRVRecord: SRV record changed for service %##s de-registering", rr->resrec.name->c);
            rr->state = regState_DeregPending;
            UpdateAllServiceRecords(m, rr, mDNSfalse);
        }
        return;
    case regState_Unregistered:
    default: LogMsg("UpdateOneSRVRecord: Unknown state %d for %##s", rr->state, rr->resrec.name->c);
    }
}

mDNSexport void UpdateAllSRVRecords(mDNS *m)
{
    m->NextSRVUpdate = 0;
    LogInfo("UpdateAllSRVRecords %d", m->SleepState);

    if (m->CurrentRecord)
        LogMsg("UpdateAllSRVRecords ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
    m->CurrentRecord = m->ResourceRecords;
    while (m->CurrentRecord)
    {
        AuthRecord *rptr = m->CurrentRecord;
        m->CurrentRecord = m->CurrentRecord->next;
        if (AuthRecord_uDNS(rptr) && rptr->resrec.rrtype == kDNSType_SRV)
            UpdateOneSRVRecord(m, rptr);
    }
}

// Forward reference: AdvertiseHostname references HostnameCallback, and HostnameCallback calls AdvertiseHostname
mDNSlocal void HostnameCallback(mDNS *const m, AuthRecord *const rr, mStatus result);

// Called in normal client context (lock not held)
mDNSlocal void hostnameGetPublicAddressCallback(mDNS *m, NATTraversalInfo *n)
{
    HostnameInfo *h = (HostnameInfo *)n->clientContext;

    if (!h) { LogMsg("RegisterHostnameRecord: registration cancelled"); return; }

    if (!n->Result)
    {
        if (mDNSIPv4AddressIsZero(n->ExternalAddress) || mDNSv4AddrIsRFC1918(&n->ExternalAddress)) return;

        if (h->arv4.resrec.RecordType)
        {
            if (mDNSSameIPv4Address(h->arv4.resrec.rdata->u.ipv4, n->ExternalAddress)) return;  // If address unchanged, do nothing
            LogInfo("Updating hostname %p %##s IPv4 from %.4a to %.4a (NAT gateway's external address)",n,
                    h->arv4.resrec.name->c, &h->arv4.resrec.rdata->u.ipv4, &n->ExternalAddress);
            mDNS_Deregister(m, &h->arv4);   // mStatus_MemFree callback will re-register with new address
        }
        else
        {
            LogInfo("Advertising hostname %##s IPv4 %.4a (NAT gateway's external address)", h->arv4.resrec.name->c, &n->ExternalAddress);
            h->arv4.resrec.RecordType = kDNSRecordTypeKnownUnique;
            h->arv4.resrec.rdata->u.ipv4 = n->ExternalAddress;
            mDNS_Register(m, &h->arv4);
        }
    }
}

// register record or begin NAT traversal
mDNSlocal void AdvertiseHostname(mDNS *m, HostnameInfo *h)
{
    if (!mDNSIPv4AddressIsZero(m->AdvertisedV4.ip.v4) && h->arv4.resrec.RecordType == kDNSRecordTypeUnregistered)
    {
        mDNS_SetupResourceRecord(&h->arv4, mDNSNULL, mDNSInterface_Any, kDNSType_A, kHostNameTTL, kDNSRecordTypeUnregistered, AuthRecordAny, HostnameCallback, h);
        AssignDomainName(&h->arv4.namestorage, &h->fqdn);
        h->arv4.resrec.rdata->u.ipv4 = m->AdvertisedV4.ip.v4;
        h->arv4.state = regState_Unregistered;
        if (mDNSv4AddrIsRFC1918(&m->AdvertisedV4.ip.v4))
        {
            // If we already have a NAT query active, stop it and restart it to make sure we get another callback
            if (h->natinfo.clientContext) mDNS_StopNATOperation_internal(m, &h->natinfo);
            h->natinfo.Protocol         = 0;
            h->natinfo.IntPort          = zeroIPPort;
            h->natinfo.RequestedPort    = zeroIPPort;
            h->natinfo.NATLease         = 0;
            h->natinfo.clientCallback   = hostnameGetPublicAddressCallback;
            h->natinfo.clientContext    = h;
            mDNS_StartNATOperation_internal(m, &h->natinfo);
        }
        else
        {
            LogInfo("Advertising hostname %##s IPv4 %.4a", h->arv4.resrec.name->c, &m->AdvertisedV4.ip.v4);
            h->arv4.resrec.RecordType = kDNSRecordTypeKnownUnique;
            mDNS_Register_internal(m, &h->arv4);
        }
    }

    if (!mDNSIPv6AddressIsZero(m->AdvertisedV6.ip.v6) && h->arv6.resrec.RecordType == kDNSRecordTypeUnregistered)
    {
        mDNS_SetupResourceRecord(&h->arv6, mDNSNULL, mDNSInterface_Any, kDNSType_AAAA, kHostNameTTL, kDNSRecordTypeKnownUnique, AuthRecordAny, HostnameCallback, h);
        AssignDomainName(&h->arv6.namestorage, &h->fqdn);
        h->arv6.resrec.rdata->u.ipv6 = m->AdvertisedV6.ip.v6;
        h->arv6.state = regState_Unregistered;
        LogInfo("Advertising hostname %##s IPv6 %.16a", h->arv6.resrec.name->c, &m->AdvertisedV6.ip.v6);
        mDNS_Register_internal(m, &h->arv6);
    }
}

mDNSlocal void HostnameCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    HostnameInfo *hi = (HostnameInfo *)rr->RecordContext;

    if (result == mStatus_MemFree)
    {
        if (hi)
        {
            // If we're still in the Hostnames list, update to new address
            HostnameInfo *i;
            LogInfo("HostnameCallback: Got mStatus_MemFree for %p %p %s", hi, rr, ARDisplayString(m, rr));
            for (i = m->Hostnames; i; i = i->next)
                if (rr == &i->arv4 || rr == &i->arv6)
                { mDNS_Lock(m); AdvertiseHostname(m, i); mDNS_Unlock(m); return; }

            // Else, we're not still in the Hostnames list, so free the memory
            if (hi->arv4.resrec.RecordType == kDNSRecordTypeUnregistered &&
                hi->arv6.resrec.RecordType == kDNSRecordTypeUnregistered)
            {
                if (hi->natinfo.clientContext) mDNS_StopNATOperation_internal(m, &hi->natinfo);
                hi->natinfo.clientContext = mDNSNULL;
                mDNSPlatformMemFree(hi);    // free hi when both v4 and v6 AuthRecs deallocated
            }
        }
        return;
    }

    if (result)
    {
        // don't unlink or free - we can retry when we get a new address/router
        if (rr->resrec.rrtype == kDNSType_A)
            LogMsg("HostnameCallback: Error %d for registration of %##s IP %.4a", result, rr->resrec.name->c, &rr->resrec.rdata->u.ipv4);
        else
            LogMsg("HostnameCallback: Error %d for registration of %##s IP %.16a", result, rr->resrec.name->c, &rr->resrec.rdata->u.ipv6);
        if (!hi) { mDNSPlatformMemFree(rr); return; }
        if (rr->state != regState_Unregistered) LogMsg("Error: HostnameCallback invoked with error code for record not in regState_Unregistered!");

        if (hi->arv4.state == regState_Unregistered &&
            hi->arv6.state == regState_Unregistered)
        {
            // only deliver status if both v4 and v6 fail
            rr->RecordContext = (void *)hi->StatusContext;
            if (hi->StatusCallback)
                hi->StatusCallback(m, rr, result); // client may NOT make API calls here
            rr->RecordContext = (void *)hi;
        }
        return;
    }

    // register any pending services that require a target
    mDNS_Lock(m);
    m->NextSRVUpdate = NonZeroTime(m->timenow);
    mDNS_Unlock(m);

    // Deliver success to client
    if (!hi) { LogMsg("HostnameCallback invoked with orphaned address record"); return; }
    if (rr->resrec.rrtype == kDNSType_A)
        LogInfo("Registered hostname %##s IP %.4a", rr->resrec.name->c, &rr->resrec.rdata->u.ipv4);
    else
        LogInfo("Registered hostname %##s IP %.16a", rr->resrec.name->c, &rr->resrec.rdata->u.ipv6);

    rr->RecordContext = (void *)hi->StatusContext;
    if (hi->StatusCallback)
        hi->StatusCallback(m, rr, result); // client may NOT make API calls here
    rr->RecordContext = (void *)hi;
}

mDNSlocal void FoundStaticHostname(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    const domainname *pktname = &answer->rdata->u.name;
    domainname *storedname = &m->StaticHostname;
    HostnameInfo *h = m->Hostnames;

    (void)question;

    if (answer->rdlength != 0)
        LogInfo("FoundStaticHostname: question %##s -> answer %##s (%s)", question->qname.c, answer->rdata->u.name.c, AddRecord ? "ADD" : "RMV");
    else
        LogInfo("FoundStaticHostname: question %##s -> answer NULL (%s)", question->qname.c, AddRecord ? "ADD" : "RMV");

    if (AddRecord && answer->rdlength != 0 && !SameDomainName(pktname, storedname))
    {
        AssignDomainName(storedname, pktname);
        while (h)
        {
            if (h->arv4.state == regState_Pending || h->arv4.state == regState_NATMap || h->arv6.state == regState_Pending)
            {
                // if we're in the process of registering a dynamic hostname, delay SRV update so we don't have to reregister services if the dynamic name succeeds
                m->NextSRVUpdate = NonZeroTime(m->timenow + 5 * mDNSPlatformOneSecond);
                debugf("FoundStaticHostname: NextSRVUpdate in %d %d", m->NextSRVUpdate - m->timenow, m->timenow);
                return;
            }
            h = h->next;
        }
        mDNS_Lock(m);
        m->NextSRVUpdate = NonZeroTime(m->timenow);
        mDNS_Unlock(m);
    }
    else if (!AddRecord && SameDomainName(pktname, storedname))
    {
        mDNS_Lock(m);
        storedname->c[0] = 0;
        m->NextSRVUpdate = NonZeroTime(m->timenow);
        mDNS_Unlock(m);
    }
}

// Called with lock held
mDNSlocal void GetStaticHostname(mDNS *m)
{
    char buf[MAX_REVERSE_MAPPING_NAME_V4];
    DNSQuestion *q = &m->ReverseMap;
    mDNSu8 *ip = m->AdvertisedV4.ip.v4.b;
    mStatus err;

    if (m->ReverseMap.ThisQInterval != -1) return; // already running
    if (mDNSIPv4AddressIsZero(m->AdvertisedV4.ip.v4)) return;

    mDNSPlatformMemZero(q, sizeof(*q));
    // Note: This is reverse order compared to a normal dotted-decimal IP address, so we can't use our customary "%.4a" format code
    mDNS_snprintf(buf, sizeof(buf), "%d.%d.%d.%d.in-addr.arpa.", ip[3], ip[2], ip[1], ip[0]);
    if (!MakeDomainNameFromDNSNameString(&q->qname, buf)) { LogMsg("Error: GetStaticHostname - bad name %s", buf); return; }

    q->InterfaceID      = mDNSInterface_Any;
    q->flags            = 0;
    q->Target           = zeroAddr;
    q->qtype            = kDNSType_PTR;
    q->qclass           = kDNSClass_IN;
    q->LongLived        = mDNSfalse;
    q->ExpectUnique     = mDNSfalse;
    q->ForceMCast       = mDNSfalse;
    q->ReturnIntermed   = mDNStrue;
    q->SuppressUnusable = mDNSfalse;
    q->SearchListIndex  = 0;
    q->AppendSearchDomains = 0;
    q->RetryWithSearchDomains = mDNSfalse;
    q->TimeoutQuestion  = 0;
    q->WakeOnResolve    = 0;
    q->UseBackgroundTrafficClass = mDNSfalse;
    q->ValidationRequired = 0;
    q->ValidatingResponse = 0;
    q->ProxyQuestion      = 0;
    q->qnameOrig        = mDNSNULL;
    q->AnonInfo         = mDNSNULL;
    q->pid              = mDNSPlatformGetPID();
    q->QuestionCallback = FoundStaticHostname;
    q->QuestionContext  = mDNSNULL;

    LogInfo("GetStaticHostname: %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
    err = mDNS_StartQuery_internal(m, q);
    if (err) LogMsg("Error: GetStaticHostname - StartQuery returned error %d", err);
}

mDNSexport void mDNS_AddDynDNSHostName(mDNS *m, const domainname *fqdn, mDNSRecordCallback *StatusCallback, const void *StatusContext)
{
    HostnameInfo **ptr = &m->Hostnames;

    LogInfo("mDNS_AddDynDNSHostName %##s", fqdn);

    while (*ptr && !SameDomainName(fqdn, &(*ptr)->fqdn)) ptr = &(*ptr)->next;
    if (*ptr) { LogMsg("DynDNSHostName %##s already in list", fqdn->c); return; }

    // allocate and format new address record
    *ptr = mDNSPlatformMemAllocate(sizeof(**ptr));
    if (!*ptr) { LogMsg("ERROR: mDNS_AddDynDNSHostName - malloc"); return; }

    mDNSPlatformMemZero(*ptr, sizeof(**ptr));
    AssignDomainName(&(*ptr)->fqdn, fqdn);
    (*ptr)->arv4.state     = regState_Unregistered;
    (*ptr)->arv6.state     = regState_Unregistered;
    (*ptr)->StatusCallback = StatusCallback;
    (*ptr)->StatusContext  = StatusContext;

    AdvertiseHostname(m, *ptr);
}

mDNSexport void mDNS_RemoveDynDNSHostName(mDNS *m, const domainname *fqdn)
{
    HostnameInfo **ptr = &m->Hostnames;

    LogInfo("mDNS_RemoveDynDNSHostName %##s", fqdn);

    while (*ptr && !SameDomainName(fqdn, &(*ptr)->fqdn)) ptr = &(*ptr)->next;
    if (!*ptr) LogMsg("mDNS_RemoveDynDNSHostName: no such domainname %##s", fqdn->c);
    else
    {
        HostnameInfo *hi = *ptr;
        // We do it this way because, if we have no active v6 record, the "mDNS_Deregister_internal(m, &hi->arv4);"
        // below could free the memory, and we have to make sure we don't touch hi fields after that.
        mDNSBool f4 = hi->arv4.resrec.RecordType != kDNSRecordTypeUnregistered && hi->arv4.state != regState_Unregistered;
        mDNSBool f6 = hi->arv6.resrec.RecordType != kDNSRecordTypeUnregistered && hi->arv6.state != regState_Unregistered;
        if (f4) LogInfo("mDNS_RemoveDynDNSHostName removing v4 %##s", fqdn);
        if (f6) LogInfo("mDNS_RemoveDynDNSHostName removing v6 %##s", fqdn);
        *ptr = (*ptr)->next; // unlink
        if (f4) mDNS_Deregister_internal(m, &hi->arv4, mDNS_Dereg_normal);
        if (f6) mDNS_Deregister_internal(m, &hi->arv6, mDNS_Dereg_normal);
        // When both deregistrations complete we'll free the memory in the mStatus_MemFree callback
    }
    mDNS_CheckLock(m);
    m->NextSRVUpdate = NonZeroTime(m->timenow);
}

// Currently called without holding the lock
// Maybe we should change that?
mDNSexport void mDNS_SetPrimaryInterfaceInfo(mDNS *m, const mDNSAddr *v4addr, const mDNSAddr *v6addr, const mDNSAddr *router)
{
    mDNSBool v4Changed, v6Changed, RouterChanged;

    if (m->mDNS_busy != m->mDNS_reentrancy)
        LogMsg("mDNS_SetPrimaryInterfaceInfo: mDNS_busy (%ld) != mDNS_reentrancy (%ld)", m->mDNS_busy, m->mDNS_reentrancy);

    if (v4addr && v4addr->type != mDNSAddrType_IPv4) { LogMsg("mDNS_SetPrimaryInterfaceInfo v4 address - incorrect type.  Discarding. %#a", v4addr); return; }
    if (v6addr && v6addr->type != mDNSAddrType_IPv6) { LogMsg("mDNS_SetPrimaryInterfaceInfo v6 address - incorrect type.  Discarding. %#a", v6addr); return; }
    if (router && router->type != mDNSAddrType_IPv4) { LogMsg("mDNS_SetPrimaryInterfaceInfo passed non-v4 router.  Discarding. %#a",        router); return; }

    mDNS_Lock(m);

    v4Changed     = !mDNSSameIPv4Address(m->AdvertisedV4.ip.v4, v4addr ? v4addr->ip.v4 : zerov4Addr);
    v6Changed     = !mDNSSameIPv6Address(m->AdvertisedV6.ip.v6, v6addr ? v6addr->ip.v6 : zerov6Addr);
    RouterChanged = !mDNSSameIPv4Address(m->Router.ip.v4,       router ? router->ip.v4 : zerov4Addr);

    if (v4addr && (v4Changed || RouterChanged))
        debugf("mDNS_SetPrimaryInterfaceInfo: address changed from %#a to %#a", &m->AdvertisedV4, v4addr);

    if (v4addr) m->AdvertisedV4 = *v4addr;else m->AdvertisedV4.ip.v4 = zerov4Addr;
    if (v6addr) m->AdvertisedV6 = *v6addr;else m->AdvertisedV6.ip.v6 = zerov6Addr;
    if (router) m->Router       = *router;else m->Router.ip.v4 = zerov4Addr;
    // setting router to zero indicates that nat mappings must be reestablished when router is reset

    if (v4Changed || RouterChanged || v6Changed)
    {
        HostnameInfo *i;
        LogInfo("mDNS_SetPrimaryInterfaceInfo: %s%s%s%#a %#a %#a",
                v4Changed     ? "v4Changed "     : "",
                RouterChanged ? "RouterChanged " : "",
                v6Changed     ? "v6Changed "     : "", v4addr, v6addr, router);

        for (i = m->Hostnames; i; i = i->next)
        {
            LogInfo("mDNS_SetPrimaryInterfaceInfo updating host name registrations for %##s", i->fqdn.c);

            if (i->arv4.resrec.RecordType > kDNSRecordTypeDeregistering &&
                !mDNSSameIPv4Address(i->arv4.resrec.rdata->u.ipv4, m->AdvertisedV4.ip.v4))
            {
                LogInfo("mDNS_SetPrimaryInterfaceInfo deregistering %s", ARDisplayString(m, &i->arv4));
                mDNS_Deregister_internal(m, &i->arv4, mDNS_Dereg_normal);
            }

            if (i->arv6.resrec.RecordType > kDNSRecordTypeDeregistering &&
                !mDNSSameIPv6Address(i->arv6.resrec.rdata->u.ipv6, m->AdvertisedV6.ip.v6))
            {
                LogInfo("mDNS_SetPrimaryInterfaceInfo deregistering %s", ARDisplayString(m, &i->arv6));
                mDNS_Deregister_internal(m, &i->arv6, mDNS_Dereg_normal);
            }

            // AdvertiseHostname will only register new address records.
            // For records still in the process of deregistering it will ignore them, and let the mStatus_MemFree callback handle them.
            AdvertiseHostname(m, i);
        }

        if (v4Changed || RouterChanged)
        {
            // If we have a non-zero IPv4 address, we should try immediately to see if we have a NAT gateway
            // If we have no IPv4 address, we don't want to be in quite such a hurry to report failures to our clients
            // <rdar://problem/6935929> Sleeping server sometimes briefly disappears over Back to My Mac after it wakes up
            mDNSu32 waitSeconds = v4addr ? 0 : 5;
            NATTraversalInfo *n;
            m->ExtAddress           = zerov4Addr;
            m->LastNATMapResultCode = NATErr_None;

            RecreateNATMappings(m, mDNSPlatformOneSecond * waitSeconds);

            for (n = m->NATTraversals; n; n=n->next)
                n->NewAddress = zerov4Addr;

            LogInfo("mDNS_SetPrimaryInterfaceInfo:%s%s: recreating NAT mappings in %d seconds",
                    v4Changed     ? " v4Changed"     : "",
                    RouterChanged ? " RouterChanged" : "",
                    waitSeconds);
        }

        if (m->ReverseMap.ThisQInterval != -1) mDNS_StopQuery_internal(m, &m->ReverseMap);
        m->StaticHostname.c[0] = 0;

        m->NextSRVUpdate = NonZeroTime(m->timenow);

#if APPLE_OSX_mDNSResponder
        if (RouterChanged) uuid_generate(m->asl_uuid);
        UpdateAutoTunnelDomainStatuses(m);
#endif
    }

    mDNS_Unlock(m);
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Incoming Message Processing
#endif

mDNSlocal mStatus ParseTSIGError(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end, const domainname *const displayname)
{
    const mDNSu8 *ptr;
    mStatus err = mStatus_NoError;
    int i;

    ptr = LocateAdditionals(msg, end);
    if (!ptr) goto finish;

    for (i = 0; i < msg->h.numAdditionals; i++)
    {
        ptr = GetLargeResourceRecord(m, msg, ptr, end, 0, kDNSRecordTypePacketAdd, &m->rec);
        if (!ptr) goto finish;
        if (m->rec.r.resrec.RecordType != kDNSRecordTypePacketNegative && m->rec.r.resrec.rrtype == kDNSType_TSIG)
        {
            mDNSu32 macsize;
            mDNSu8 *rd = m->rec.r.resrec.rdata->u.data;
            mDNSu8 *rdend = rd + m->rec.r.resrec.rdlength;
            int alglen = DomainNameLengthLimit(&m->rec.r.resrec.rdata->u.name, rdend);
            if (alglen > MAX_DOMAIN_NAME) goto finish;
            rd += alglen;                                       // algorithm name
            if (rd + 6 > rdend) goto finish;
            rd += 6;                                            // 48-bit timestamp
            if (rd + sizeof(mDNSOpaque16) > rdend) goto finish;
            rd += sizeof(mDNSOpaque16);                         // fudge
            if (rd + sizeof(mDNSOpaque16) > rdend) goto finish;
            macsize = mDNSVal16(*(mDNSOpaque16 *)rd);
            rd += sizeof(mDNSOpaque16);                         // MAC size
            if (rd + macsize > rdend) goto finish;
            rd += macsize;
            if (rd + sizeof(mDNSOpaque16) > rdend) goto finish;
            rd += sizeof(mDNSOpaque16);                         // orig id
            if (rd + sizeof(mDNSOpaque16) > rdend) goto finish;
            err = mDNSVal16(*(mDNSOpaque16 *)rd);               // error code

            if      (err == TSIG_ErrBadSig)  { LogMsg("%##s: bad signature", displayname->c);              err = mStatus_BadSig;     }
            else if (err == TSIG_ErrBadKey)  { LogMsg("%##s: bad key", displayname->c);                    err = mStatus_BadKey;     }
            else if (err == TSIG_ErrBadTime) { LogMsg("%##s: bad time", displayname->c);                   err = mStatus_BadTime;    }
            else if (err)                    { LogMsg("%##s: unknown tsig error %d", displayname->c, err); err = mStatus_UnknownErr; }
            goto finish;
        }
        m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    }

finish:
    m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
    return err;
}

mDNSlocal mStatus checkUpdateResult(mDNS *const m, const domainname *const displayname, const mDNSu8 rcode, const DNSMessage *const msg, const mDNSu8 *const end)
{
    (void)msg;  // currently unused, needed for TSIG errors
    if (!rcode) return mStatus_NoError;
    else if (rcode == kDNSFlag1_RC_YXDomain)
    {
        debugf("name in use: %##s", displayname->c);
        return mStatus_NameConflict;
    }
    else if (rcode == kDNSFlag1_RC_Refused)
    {
        LogMsg("Update %##s refused", displayname->c);
        return mStatus_Refused;
    }
    else if (rcode == kDNSFlag1_RC_NXRRSet)
    {
        LogMsg("Reregister refused (NXRRSET): %##s", displayname->c);
        return mStatus_NoSuchRecord;
    }
    else if (rcode == kDNSFlag1_RC_NotAuth)
    {
        // TSIG errors should come with FormErr as per RFC 2845, but BIND 9 sends them with NotAuth so we look here too
        mStatus tsigerr = ParseTSIGError(m, msg, end, displayname);
        if (!tsigerr)
        {
            LogMsg("Permission denied (NOAUTH): %##s", displayname->c);
            return mStatus_UnknownErr;
        }
        else return tsigerr;
    }
    else if (rcode == kDNSFlag1_RC_FormErr)
    {
        mStatus tsigerr = ParseTSIGError(m, msg, end, displayname);
        if (!tsigerr)
        {
            LogMsg("Format Error: %##s", displayname->c);
            return mStatus_UnknownErr;
        }
        else return tsigerr;
    }
    else
    {
        LogMsg("Update %##s failed with rcode %d", displayname->c, rcode);
        return mStatus_UnknownErr;
    }
}

// We add three Additional Records for unicast resource record registrations
// which is a function of AuthInfo and AutoTunnel properties
mDNSlocal mDNSu32 RRAdditionalSize(mDNS *const m, DomainAuthInfo *AuthInfo)
{
    mDNSu32 leaseSize, hinfoSize, tsigSize;
    mDNSu32 rr_base_size = 10; // type (2) class (2) TTL (4) rdlength (2)

    // OPT RR : Emptyname(.) + base size + rdataOPT
    leaseSize = 1 + rr_base_size + sizeof(rdataOPT);

    // HINFO: Resource Record Name + base size + RDATA
    // HINFO is added only for autotunnels
    hinfoSize = 0;
    if (AuthInfo && AuthInfo->AutoTunnel)
        hinfoSize = (m->hostlabel.c[0] + 1) + DomainNameLength(&AuthInfo->domain) +
                    rr_base_size + (2 + m->HIHardware.c[0] + m->HISoftware.c[0]);

    //TSIG: Resource Record Name + base size + RDATA
    // RDATA:
    //  Algorithm name: hmac-md5.sig-alg.reg.int (8+7+3+3 + 5 bytes for length = 26 bytes)
    //  Time: 6 bytes
    //  Fudge: 2 bytes
    //  Mac Size: 2 bytes
    //  Mac: 16 bytes
    //  ID: 2 bytes
    //  Error: 2 bytes
    //  Len: 2 bytes
    //  Total: 58 bytes
    tsigSize = 0;
    if (AuthInfo) tsigSize = DomainNameLength(&AuthInfo->keyname) + rr_base_size + 58;

    return (leaseSize + hinfoSize + tsigSize);
}

//Note: Make sure that RREstimatedSize is updated accordingly if anything that is done here
//would modify rdlength/rdestimate
mDNSlocal mDNSu8* BuildUpdateMessage(mDNS *const m, mDNSu8 *ptr, AuthRecord *rr, mDNSu8 *limit)
{
    //If this record is deregistering, then just send the deletion record
    if (rr->state == regState_DeregPending)
    {
        rr->expire = 0;     // Indicate that we have no active registration any more
        ptr = putDeletionRecordWithLimit(&m->omsg, ptr, &rr->resrec, limit);
        if (!ptr) goto exit;
        return ptr;
    }

    // This is a common function to both sending an update in a group or individual
    // records separately. Hence, we change the state here.
    if (rr->state == regState_Registered) rr->state = regState_Refresh;
    if (rr->state != regState_Refresh && rr->state != regState_UpdatePending)
        rr->state = regState_Pending;

    // For Advisory records like e.g., _services._dns-sd, which is shared, don't send goodbyes as multiple
    // host might be registering records and deregistering from one does not make sense
    if (rr->resrec.RecordType != kDNSRecordTypeAdvisory) rr->RequireGoodbye = mDNStrue;

    if ((rr->resrec.rrtype == kDNSType_SRV) && (rr->AutoTarget == Target_AutoHostAndNATMAP) &&
        !mDNSIPPortIsZero(rr->NATinfo.ExternalPort))
    {
        rr->resrec.rdata->u.srv.port = rr->NATinfo.ExternalPort;
    }

    if (rr->state == regState_UpdatePending)
    {
        // delete old RData
        SetNewRData(&rr->resrec, rr->OrigRData, rr->OrigRDLen);
        if (!(ptr = putDeletionRecordWithLimit(&m->omsg, ptr, &rr->resrec, limit))) goto exit; // delete old rdata

        // add new RData
        SetNewRData(&rr->resrec, rr->InFlightRData, rr->InFlightRDLen);
        if (!(ptr = PutResourceRecordTTLWithLimit(&m->omsg, ptr, &m->omsg.h.mDNS_numUpdates, &rr->resrec, rr->resrec.rroriginalttl, limit))) goto exit;
    }
    else
    {
        if (rr->resrec.RecordType == kDNSRecordTypeKnownUnique || rr->resrec.RecordType == kDNSRecordTypeVerified)
        {
            // KnownUnique : Delete any previous value
            // For Unicast registrations, we don't verify that it is unique, but set to verified and hence we want to
            // delete any previous value
            ptr = putDeleteRRSetWithLimit(&m->omsg, ptr, rr->resrec.name, rr->resrec.rrtype, limit);
            if (!ptr) goto exit;
        }
        else if (rr->resrec.RecordType != kDNSRecordTypeShared)
        {
            // For now don't do this, until we have the logic for intelligent grouping of individual records into logical service record sets
            //ptr = putPrereqNameNotInUse(rr->resrec.name, &m->omsg, ptr, end);
            if (!ptr) goto exit;
        }

        ptr = PutResourceRecordTTLWithLimit(&m->omsg, ptr, &m->omsg.h.mDNS_numUpdates, &rr->resrec, rr->resrec.rroriginalttl, limit);
        if (!ptr) goto exit;
    }

    return ptr;
exit:
    LogMsg("BuildUpdateMessage: Error formatting message for %s", ARDisplayString(m, rr));
    return mDNSNULL;
}

// Called with lock held
mDNSlocal void SendRecordRegistration(mDNS *const m, AuthRecord *rr)
{
    mDNSu8 *ptr = m->omsg.data;
    mStatus err = mStatus_UnknownErr;
    mDNSu8 *limit;
    DomainAuthInfo *AuthInfo;

    // For the ability to register large TXT records, we limit the single record registrations
    // to AbsoluteMaxDNSMessageData
    limit = ptr + AbsoluteMaxDNSMessageData;

    AuthInfo = GetAuthInfoForName_internal(m, rr->resrec.name);
    limit -= RRAdditionalSize(m, AuthInfo);

    mDNS_CheckLock(m);

    if (!rr->nta || mDNSIPv4AddressIsZero(rr->nta->Addr.ip.v4))
    {
        // We never call this function when there is no zone information . Log a message if it ever happens.
        LogMsg("SendRecordRegistration: No Zone information, should not happen %s", ARDisplayString(m, rr));
        return;
    }

    rr->updateid = mDNS_NewMessageID(m);
    InitializeDNSMessage(&m->omsg.h, rr->updateid, UpdateReqFlags);

    // set zone
    ptr = putZone(&m->omsg, ptr, limit, rr->zone, mDNSOpaque16fromIntVal(rr->resrec.rrclass));
    if (!ptr) goto exit;

    if (!(ptr = BuildUpdateMessage(m, ptr, rr, limit))) goto exit;

    if (rr->uselease)
    {
        ptr = putUpdateLeaseWithLimit(&m->omsg, ptr, DEFAULT_UPDATE_LEASE, limit);
        if (!ptr) goto exit;
    }
    if (rr->Private)
    {
        LogInfo("SendRecordRegistration TCP %p %s", rr->tcp, ARDisplayString(m, rr));
        if (rr->tcp) LogInfo("SendRecordRegistration: Disposing existing TCP connection for %s", ARDisplayString(m, rr));
        if (rr->tcp) { DisposeTCPConn(rr->tcp); rr->tcp = mDNSNULL; }
        if (!rr->nta) { LogMsg("SendRecordRegistration:Private:ERROR!! nta is NULL for %s", ARDisplayString(m, rr)); return; }
        rr->tcp = MakeTCPConn(m, &m->omsg, ptr, kTCPSocketFlags_UseTLS, &rr->nta->Addr, rr->nta->Port, &rr->nta->Host, mDNSNULL, rr);
    }
    else
    {
        LogInfo("SendRecordRegistration UDP %s", ARDisplayString(m, rr));
        if (!rr->nta) { LogMsg("SendRecordRegistration:ERROR!! nta is NULL for %s", ARDisplayString(m, rr)); return; }
        err = mDNSSendDNSMessage(m, &m->omsg, ptr, mDNSInterface_Any, mDNSNULL, &rr->nta->Addr, rr->nta->Port, mDNSNULL, GetAuthInfoForName_internal(m, rr->resrec.name), mDNSfalse);
        if (err) debugf("ERROR: SendRecordRegistration - mDNSSendDNSMessage - %d", err);
    }

    SetRecordRetry(m, rr, 0);
    return;
exit:
    LogMsg("SendRecordRegistration: Error formatting message for %s, disabling further updates", ARDisplayString(m, rr));
    // Disable this record from future updates
    rr->state = regState_NoTarget;
}

// Is the given record "rr" eligible for merging ?
mDNSlocal mDNSBool IsRecordMergeable(mDNS *const m, AuthRecord *rr, mDNSs32 time)
{
    DomainAuthInfo *info;
    (void) m; //unused
    // A record is eligible for merge, if the following properties are met.
    //
    // 1. uDNS Resource Record
    // 2. It is time to send them now
    // 3. It is in proper state
    // 4. Update zone has been resolved
    // 5. if DomainAuthInfo exists for the zone, it should not be soon deleted
    // 6. Zone information is present
    // 7. Update server is not zero
    // 8. It has a non-null zone
    // 9. It uses a lease option
    // 10. DontMerge is not set
    //
    // Following code is implemented as separate "if" statements instead of one "if" statement
    // is for better debugging purposes e.g., we know exactly what failed if debugging turned on.

    if (!AuthRecord_uDNS(rr)) return mDNSfalse;

    if (rr->LastAPTime + rr->ThisAPInterval - time > 0)
    { debugf("IsRecordMergeable: Time %d not reached for %s", rr->LastAPTime + rr->ThisAPInterval - m->timenow, ARDisplayString(m, rr)); return mDNSfalse; }

    if (!rr->zone) return mDNSfalse;

    info = GetAuthInfoForName_internal(m, rr->zone);

    if (info && info->deltime && m->timenow - info->deltime >= 0) {debugf("IsRecordMergeable: Domain %##s will be deleted soon", info->domain.c); return mDNSfalse;}

    if (rr->state != regState_DeregPending && rr->state != regState_Pending && rr->state != regState_Registered && rr->state != regState_Refresh && rr->state != regState_UpdatePending)
    { debugf("IsRecordMergeable: state %d not right  %s", rr->state, ARDisplayString(m, rr)); return mDNSfalse; }

    if (!rr->nta || mDNSIPv4AddressIsZero(rr->nta->Addr.ip.v4)) return mDNSfalse;

    if (!rr->uselease) return mDNSfalse;

    if (rr->mState == mergeState_DontMerge) {debugf("IsRecordMergeable Dontmerge true %s", ARDisplayString(m, rr)); return mDNSfalse;}
    debugf("IsRecordMergeable: Returning true for %s", ARDisplayString(m, rr));
    return mDNStrue;
}

// Is the resource record "rr" eligible to merge to with "currentRR" ?
mDNSlocal mDNSBool AreRecordsMergeable(mDNS *const m, AuthRecord *currentRR, AuthRecord *rr, mDNSs32 time)
{
    // A record is eligible to merge with another record as long it is eligible for merge in itself
    // and it has the same zone information as the other record
    if (!IsRecordMergeable(m, rr, time)) return mDNSfalse;

    if (!SameDomainName(currentRR->zone, rr->zone))
    { debugf("AreRecordMergeable zone mismatch current rr Zone %##s, rr zone  %##s", currentRR->zone->c, rr->zone->c); return mDNSfalse; }

    if (!mDNSSameIPv4Address(currentRR->nta->Addr.ip.v4, rr->nta->Addr.ip.v4)) return mDNSfalse;

    if (!mDNSSameIPPort(currentRR->nta->Port, rr->nta->Port)) return mDNSfalse;

    debugf("AreRecordsMergeable: Returning true for %s", ARDisplayString(m, rr));
    return mDNStrue;
}

// If we can't build the message successfully because of problems in pre-computing
// the space, we disable merging for all the current records
mDNSlocal void RRMergeFailure(mDNS *const m)
{
    AuthRecord *rr;
    for (rr = m->ResourceRecords; rr; rr = rr->next)
    {
        rr->mState = mergeState_DontMerge;
        rr->SendRNow = mDNSNULL;
        // Restarting the registration is much simpler than saving and restoring
        // the exact time
        ActivateUnicastRegistration(m, rr);
    }
}

mDNSlocal void SendGroupRRMessage(mDNS *const m, AuthRecord *anchorRR, mDNSu8 *ptr, DomainAuthInfo *info)
{
    mDNSu8 *limit;
    if (!anchorRR) {debugf("SendGroupRRMessage: Could not merge records"); return;}

    if (info && info->AutoTunnel) limit = m->omsg.data + AbsoluteMaxDNSMessageData;
    else limit = m->omsg.data + NormalMaxDNSMessageData;

    // This has to go in the additional section and hence need to be done last
    ptr = putUpdateLeaseWithLimit(&m->omsg, ptr, DEFAULT_UPDATE_LEASE, limit);
    if (!ptr)
    {
        LogMsg("SendGroupRRMessage: ERROR: Could not put lease option, failing the group registration");
        // if we can't put the lease, we need to undo the merge
        RRMergeFailure(m);
        return;
    }
    if (anchorRR->Private)
    {
        if (anchorRR->tcp) debugf("SendGroupRRMessage: Disposing existing TCP connection for %s", ARDisplayString(m, anchorRR));
        if (anchorRR->tcp) { DisposeTCPConn(anchorRR->tcp); anchorRR->tcp = mDNSNULL; }
        if (!anchorRR->nta) { LogMsg("SendGroupRRMessage:ERROR!! nta is NULL for %s", ARDisplayString(m, anchorRR)); return; }
        anchorRR->tcp = MakeTCPConn(m, &m->omsg, ptr, kTCPSocketFlags_UseTLS, &anchorRR->nta->Addr, anchorRR->nta->Port, &anchorRR->nta->Host, mDNSNULL, anchorRR);
        if (!anchorRR->tcp) LogInfo("SendGroupRRMessage: Cannot establish TCP connection for %s", ARDisplayString(m, anchorRR));
        else LogInfo("SendGroupRRMessage: Sent a group update ID: %d start %p, end %p, limit %p", mDNSVal16(m->omsg.h.id), m->omsg.data, ptr, limit);
    }
    else
    {
        mStatus err = mDNSSendDNSMessage(m, &m->omsg, ptr, mDNSInterface_Any, mDNSNULL, &anchorRR->nta->Addr, anchorRR->nta->Port, mDNSNULL, info, mDNSfalse);
        if (err) LogInfo("SendGroupRRMessage: Cannot send UDP message for %s", ARDisplayString(m, anchorRR));
        else LogInfo("SendGroupRRMessage: Sent a group UDP update ID: %d start %p, end %p, limit %p", mDNSVal16(m->omsg.h.id), m->omsg.data, ptr, limit);
    }
    return;
}

// As we always include the zone information and the resource records contain zone name
// at the end, it will get compressed. Hence, we subtract zoneSize and add two bytes for
// the compression pointer
mDNSlocal mDNSu32 RREstimatedSize(AuthRecord *rr, int zoneSize)
{
    int rdlength;

    // Note: Estimation of the record size has to mirror the logic in BuildUpdateMessage, otherwise estimation
    // would be wrong. Currently BuildUpdateMessage calls SetNewRData in UpdatePending case. Hence, we need
    // to account for that here. Otherwise, we might under estimate the size.
    if (rr->state == regState_UpdatePending)
        // old RData that will be deleted
        // new RData that will be added
        rdlength = rr->OrigRDLen + rr->InFlightRDLen;
    else
        rdlength = rr->resrec.rdestimate;

    if (rr->state == regState_DeregPending)
    {
        debugf("RREstimatedSize: ResourceRecord %##s (%s), DomainNameLength %d, zoneSize %d, rdestimate %d",
               rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), DomainNameLength(rr->resrec.name), zoneSize, rdlength);
        return DomainNameLength(rr->resrec.name) - zoneSize + 2 + 10 + rdlength;
    }

    // For SRV, TXT, AAAA etc. that are Unique/Verified, we also send a Deletion Record
    if (rr->resrec.RecordType == kDNSRecordTypeKnownUnique || rr->resrec.RecordType == kDNSRecordTypeVerified)
    {
        // Deletion Record: Resource Record Name + Base size (10) + 0
        // Record: Resource Record Name (Compressed = 2) + Base size (10) + rdestimate

        debugf("RREstimatedSize: ResourceRecord %##s (%s), DomainNameLength %d, zoneSize %d, rdestimate %d",
               rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), DomainNameLength(rr->resrec.name), zoneSize, rdlength);
        return DomainNameLength(rr->resrec.name) - zoneSize + 2 + 10 + 2 + 10 + rdlength;
    }
    else
    {
        return DomainNameLength(rr->resrec.name) - zoneSize + 2 + 10 + rdlength;
    }
}

mDNSlocal AuthRecord *MarkRRForSending(mDNS *const m)
{
    AuthRecord *rr;
    AuthRecord *firstRR = mDNSNULL;

    // Look for records that needs to be sent in the next two seconds (MERGE_DELAY_TIME is set to 1 second).
    // The logic is as follows.
    //
    // 1. Record 1 finishes getting zone data and its registration gets delayed by 1 second
    // 2. Record 2 comes 0.1 second later, finishes getting its zone data and its registration is also delayed by
    //    1 second which is now scheduled at 1.1 second
    //
    // By looking for 1 second into the future (m->timenow + MERGE_DELAY_TIME below does that) we have merged both
    // of the above records. Note that we can't look for records too much into the future as this will affect the
    // retry logic. The first retry is scheduled at 3 seconds. Hence, we should always look smaller than that.
    // Anything more than one second will affect the first retry to happen sooner.
    //
    // Note: As a side effect of looking one second into the future to facilitate merging, the retries happen
    // one second sooner.
    for (rr = m->ResourceRecords; rr; rr = rr->next)
    {
        if (!firstRR)
        {
            if (!IsRecordMergeable(m, rr, m->timenow + MERGE_DELAY_TIME)) continue;
            firstRR = rr;
        }
        else if (!AreRecordsMergeable(m, firstRR, rr, m->timenow + MERGE_DELAY_TIME)) continue;

        if (rr->SendRNow) LogMsg("MarkRRForSending: Resourcerecord %s already marked for sending", ARDisplayString(m, rr));
        rr->SendRNow = uDNSInterfaceMark;
    }

    // We parsed through all records and found something to send. The services/records might
    // get registered at different times but we want the refreshes to be all merged and sent
    // as one update. Hence, we accelerate some of the records so that they will sync up in
    // the future. Look at the records excluding the ones that we have already sent in the
    // previous pass. If it half way through its scheduled refresh/retransmit, merge them
    // into this packet.
    //
    // Note that we only look at Registered/Refresh state to keep it simple. As we don't know
    // whether the current update will fit into one or more packets, merging a resource record
    // (which is in a different state) that has been scheduled for retransmit would trigger
    // sending more packets.
    if (firstRR)
    {
        int acc = 0;
        for (rr = m->ResourceRecords; rr; rr = rr->next)
        {
            if ((rr->state != regState_Registered && rr->state != regState_Refresh) ||
                (rr->SendRNow == uDNSInterfaceMark) ||
                (!AreRecordsMergeable(m, firstRR, rr, m->timenow + rr->ThisAPInterval/2)))
                continue;
            rr->SendRNow = uDNSInterfaceMark;
            acc++;
        }
        if (acc) LogInfo("MarkRRForSending: Accelereated %d records", acc);
    }
    return firstRR;
}

mDNSlocal mDNSBool SendGroupUpdates(mDNS *const m)
{
    mDNSOpaque16 msgid;
    mDNSs32 spaceleft = 0;
    mDNSs32 zoneSize, rrSize;
    mDNSu8 *oldnext; // for debugging
    mDNSu8 *next = m->omsg.data;
    AuthRecord *rr;
    AuthRecord *anchorRR = mDNSNULL;
    int nrecords = 0;
    AuthRecord *startRR = m->ResourceRecords;
    mDNSu8 *limit = mDNSNULL;
    DomainAuthInfo *AuthInfo = mDNSNULL;
    mDNSBool sentallRecords = mDNStrue;


    // We try to fit as many ResourceRecords as possible in AbsoluteNormal/MaxDNSMessageData. Before we start
    // putting in resource records, we need to reserve space for a few things. Every group/packet should
    // have the following.
    //
    // 1) Needs space for the Zone information (which needs to be at the beginning)
    // 2) Additional section MUST have space for lease option, HINFO and TSIG option (which needs to
    //    to be at the end)
    //
    // In future we need to reserve space for the pre-requisites which also goes at the beginning.
    // To accomodate pre-requisites in the future, first we walk the whole list marking records
    // that can be sent in this packet and computing the space needed for these records.
    // For TXT and SRV records, we delete the previous record if any by sending the same
    // resource record with ANY RDATA and zero rdlen. Hence, we need to have space for both of them.

    while (startRR)
    {
        AuthInfo = mDNSNULL;
        anchorRR = mDNSNULL;
        nrecords = 0;
        zoneSize = 0;
        for (rr = startRR; rr; rr = rr->next)
        {
            if (rr->SendRNow != uDNSInterfaceMark) continue;

            rr->SendRNow = mDNSNULL;

            if (!anchorRR)
            {
                AuthInfo = GetAuthInfoForName_internal(m, rr->zone);

                // Though we allow single record registrations for UDP to be AbsoluteMaxDNSMessageData (See
                // SendRecordRegistration) to handle large TXT records, to avoid fragmentation we limit UDP
                // message to NormalMaxDNSMessageData
                if (AuthInfo && AuthInfo->AutoTunnel) spaceleft = AbsoluteMaxDNSMessageData;
                else spaceleft = NormalMaxDNSMessageData;

                next = m->omsg.data;
                spaceleft -= RRAdditionalSize(m, AuthInfo);
                if (spaceleft <= 0)
                {
                    LogMsg("SendGroupUpdates: ERROR!!: spaceleft is zero at the beginning");
                    RRMergeFailure(m);
                    return mDNSfalse;
                }
                limit = next + spaceleft;

                // Build the initial part of message before putting in the other records
                msgid = mDNS_NewMessageID(m);
                InitializeDNSMessage(&m->omsg.h, msgid, UpdateReqFlags);

                // We need zone information at the beginning of the packet. Length: ZNAME, ZTYPE(2), ZCLASS(2)
                // zone has to be non-NULL for a record to be mergeable, hence it is safe to set/ examine zone
                //without checking for NULL.
                zoneSize = DomainNameLength(rr->zone) + 4;
                spaceleft -= zoneSize;
                if (spaceleft <= 0)
                {
                    LogMsg("SendGroupUpdates: ERROR no space for zone information, disabling merge");
                    RRMergeFailure(m);
                    return mDNSfalse;
                }
                next = putZone(&m->omsg, next, limit, rr->zone, mDNSOpaque16fromIntVal(rr->resrec.rrclass));
                if (!next)
                {
                    LogMsg("SendGroupUpdates: ERROR! Cannot put zone, disabling merge");
                    RRMergeFailure(m);
                    return mDNSfalse;
                }
                anchorRR = rr;
            }

            rrSize = RREstimatedSize(rr, zoneSize - 4);

            if ((spaceleft - rrSize) < 0)
            {
                // If we can't fit even a single message, skip it, it will be sent separately
                // in CheckRecordUpdates
                if (!nrecords)
                {
                    LogInfo("SendGroupUpdates: Skipping message %s, spaceleft %d, rrSize %d", ARDisplayString(m, rr), spaceleft, rrSize);
                    // Mark this as not sent so that the caller knows about it
                    rr->SendRNow = uDNSInterfaceMark;
                    // We need to remove the merge delay so that we can send it immediately
                    rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
                    rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
                    rr = rr->next;
                    anchorRR = mDNSNULL;
                    sentallRecords = mDNSfalse;
                }
                else
                {
                    LogInfo("SendGroupUpdates:1: Parsed %d records and sending using %s, spaceleft %d, rrSize %d", nrecords, ARDisplayString(m, anchorRR), spaceleft, rrSize);
                    SendGroupRRMessage(m, anchorRR, next, AuthInfo);
                }
                break;      // breaks out of for loop
            }
            spaceleft -= rrSize;
            oldnext = next;
            LogInfo("SendGroupUpdates: Building a message with resource record %s, next %p, state %d, ttl %d", ARDisplayString(m, rr), next, rr->state, rr->resrec.rroriginalttl);
            if (!(next = BuildUpdateMessage(m, next, rr, limit)))
            {
                // We calculated the space and if we can't fit in, we had some bug in the calculation,
                // disable merge completely.
                LogMsg("SendGroupUpdates: ptr NULL while building message with %s", ARDisplayString(m, rr));
                RRMergeFailure(m);
                return mDNSfalse;
            }
            // If our estimate was higher, adjust to the actual size
            if ((next - oldnext) > rrSize)
                LogMsg("SendGroupUpdates: ERROR!! Record size estimation is wrong for %s, Estimate %d, Actual %d, state %d", ARDisplayString(m, rr), rrSize, next - oldnext, rr->state);
            else { spaceleft += rrSize; spaceleft -= (next - oldnext); }

            nrecords++;
            // We could have sent an update earlier with this "rr" as anchorRR for which we never got a response.
            // To preserve ordering, we blow away the previous connection before sending this.
            if (rr->tcp) { DisposeTCPConn(rr->tcp); rr->tcp = mDNSNULL;}
            rr->updateid = msgid;

            // By setting the retry time interval here, we will not be looking at these records
            // again when we return to CheckGroupRecordUpdates.
            SetRecordRetry(m, rr, 0);
        }
        // Either we have parsed all the records or stopped at "rr" above due to lack of space
        startRR = rr;
    }

    if (anchorRR)
    {
        LogInfo("SendGroupUpdates: Parsed %d records and sending using %s", nrecords, ARDisplayString(m, anchorRR));
        SendGroupRRMessage(m, anchorRR, next, AuthInfo);
    }
    return sentallRecords;
}

// Merge the record registrations and send them as a group only if they
// have same DomainAuthInfo and hence the same key to put the TSIG
mDNSlocal void CheckGroupRecordUpdates(mDNS *const m)
{
    AuthRecord *rr, *nextRR;
    // Keep sending as long as there is at least one record to be sent
    while (MarkRRForSending(m))
    {
        if (!SendGroupUpdates(m))
        {
            // if everything that was marked was not sent, send them out individually
            for (rr = m->ResourceRecords; rr; rr = nextRR)
            {
                // SendRecordRegistrtion might delete the rr from list, hence
                // dereference nextRR before calling the function
                nextRR = rr->next;
                if (rr->SendRNow == uDNSInterfaceMark)
                {
                    // Any records marked for sending should be eligible to be sent out
                    // immediately. Just being cautious
                    if (rr->LastAPTime + rr->ThisAPInterval - m->timenow > 0)
                    { LogMsg("CheckGroupRecordUpdates: ERROR!! Resourcerecord %s not ready", ARDisplayString(m, rr)); continue; }
                    rr->SendRNow = mDNSNULL;
                    SendRecordRegistration(m, rr);
                }
            }
        }
    }

    debugf("CheckGroupRecordUpdates: No work, returning");
    return;
}

mDNSlocal void hndlSRVChanged(mDNS *const m, AuthRecord *rr)
{
    // Reevaluate the target always as NAT/Target could have changed while
    // we were registering/deeregistering
    domainname *dt;
    const domainname *target = GetServiceTarget(m, rr);
    if (!target || target->c[0] == 0)
    {
        // we don't have a target, if we just derregistered, then we don't have to do anything
        if (rr->state == regState_DeregPending)
        {
            LogInfo("hndlSRVChanged: SRVChanged, No Target, SRV Deregistered for %##s, state %d", rr->resrec.name->c,
                    rr->state);
            rr->SRVChanged = mDNSfalse;
            dt = GetRRDomainNameTarget(&rr->resrec);
            if (dt) dt->c[0] = 0;
            rr->state = regState_NoTarget;  // Wait for the next target change
            rr->resrec.rdlength = rr->resrec.rdestimate = 0;
            return;
        }

        // we don't have a target, if we just registered, we need to deregister
        if (rr->state == regState_Pending)
        {
            LogInfo("hndlSRVChanged: SRVChanged, No Target, Deregistering again %##s, state %d", rr->resrec.name->c, rr->state);
            rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
            rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
            rr->state = regState_DeregPending;
            return;
        }
        LogInfo("hndlSRVChanged: Not in DeregPending or RegPending state %##s, state %d", rr->resrec.name->c, rr->state);
    }
    else
    {
        // If we were in registered state and SRV changed to NULL, we deregister and come back here
        // if we have a target, we need to register again.
        //
        // if we just registered check to see if it is same. If it is different just re-register the
        // SRV and its assoicated records
        //
        // UpdateOneSRVRecord takes care of re-registering all service records
        if ((rr->state == regState_DeregPending) ||
            (rr->state == regState_Pending && !SameDomainName(target, &rr->resrec.rdata->u.srv.target)))
        {
            dt = GetRRDomainNameTarget(&rr->resrec);
            if (dt) dt->c[0] = 0;
            rr->state = regState_NoTarget;  // NoTarget will allow us to pick up new target OR nat traversal state
            rr->resrec.rdlength = rr->resrec.rdestimate = 0;
            LogInfo("hndlSRVChanged: SRVChanged, Valid Target %##s, Registering all records for %##s, state %d",
                    target->c, rr->resrec.name->c, rr->state);
            rr->SRVChanged = mDNSfalse;
            UpdateOneSRVRecord(m, rr);
            return;
        }
        // Target did not change while this record was registering. Hence, we go to
        // Registered state - the state we started from.
        if (rr->state == regState_Pending) rr->state = regState_Registered;
    }

    rr->SRVChanged = mDNSfalse;
}

// Called with lock held
mDNSlocal void hndlRecordUpdateReply(mDNS *m, AuthRecord *rr, mStatus err, mDNSu32 random)
{
    mDNSBool InvokeCallback = mDNStrue;
    mDNSIPPort UpdatePort = zeroIPPort;

    mDNS_CheckLock(m);

    LogInfo("hndlRecordUpdateReply: err %d ID %d state %d %s(%p)", err, mDNSVal16(rr->updateid), rr->state, ARDisplayString(m, rr), rr);

    rr->updateError = err;
#if APPLE_OSX_mDNSResponder
    if (err == mStatus_BadSig || err == mStatus_BadKey || err == mStatus_BadTime) UpdateAutoTunnelDomainStatuses(m);
#endif

    SetRecordRetry(m, rr, random);

    rr->updateid = zeroID;  // Make sure that this is not considered as part of a group anymore
    // Later when need to send an update, we will get the zone data again. Thus we avoid
    // using stale information.
    //
    // Note: By clearing out the zone info here, it also helps better merging of records
    // in some cases. For example, when we get out regState_NoTarget state e.g., move out
    // of Double NAT, we want all the records to be in one update. Some BTMM records like
    // _autotunnel6 and host records are registered/deregistered when NAT state changes.
    // As they are re-registered the zone information is cleared out. To merge with other
    // records that might be possibly going out, clearing out the information here helps
    // as all of them try to get the zone data.
    if (rr->nta)
    {
        // We always expect the question to be stopped when we get a valid response from the server.
        // If the zone info tries to change during this time, updateid would be different and hence
        // this response should not have been accepted.
        if (rr->nta->question.ThisQInterval != -1)
            LogMsg("hndlRecordUpdateReply: ResourceRecord %s, zone info question %##s (%s) interval %d not -1",
                   ARDisplayString(m, rr), rr->nta->question.qname.c, DNSTypeName(rr->nta->question.qtype), rr->nta->question.ThisQInterval);
        UpdatePort = rr->nta->Port;
        CancelGetZoneData(m, rr->nta);
        rr->nta = mDNSNULL;
    }

    // If we are deregistering the record, then complete the deregistration. Ignore any NAT/SRV change
    // that could have happened during that time.
    if (rr->resrec.RecordType == kDNSRecordTypeDeregistering && rr->state == regState_DeregPending)
    {
        debugf("hndlRecordUpdateReply: Received reply for deregister record %##s type %d", rr->resrec.name->c, rr->resrec.rrtype);
        if (err) LogMsg("ERROR: Deregistration of record %##s type %d failed with error %d",
                        rr->resrec.name->c, rr->resrec.rrtype, err);
        rr->state = regState_Unregistered;
        CompleteDeregistration(m, rr);
        return;
    }

    // We are returning early without updating the state. When we come back from sleep we will re-register after
    // re-initializing all the state as though it is a first registration. If the record can't be registered e.g.,
    // no target, it will be deregistered. Hence, the updating to the right state should not matter when going
    // to sleep.
    if (m->SleepState)
    {
        // Need to set it to NoTarget state so that RecordReadyForSleep knows that
        // we are done
        if (rr->resrec.rrtype == kDNSType_SRV && rr->state == regState_DeregPending)
            rr->state = regState_NoTarget;
        return;
    }

    if (rr->state == regState_UpdatePending)
    {
        if (err) LogMsg("Update record failed for %##s (err %d)", rr->resrec.name->c, err);
        rr->state = regState_Registered;
        // deallocate old RData
        if (rr->UpdateCallback) rr->UpdateCallback(m, rr, rr->OrigRData, rr->OrigRDLen);
        SetNewRData(&rr->resrec, rr->InFlightRData, rr->InFlightRDLen);
        rr->OrigRData = mDNSNULL;
        rr->InFlightRData = mDNSNULL;
    }

    if (rr->SRVChanged)
    {
        if (rr->resrec.rrtype == kDNSType_SRV)
            hndlSRVChanged(m, rr);
        else
        {
            LogInfo("hndlRecordUpdateReply: Deregistered %##s (%s), state %d", rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype), rr->state);
            rr->SRVChanged = mDNSfalse;
            if (rr->state != regState_DeregPending) LogMsg("hndlRecordUpdateReply: ResourceRecord %s not in DeregPending state %d", ARDisplayString(m, rr), rr->state);
            rr->state = regState_NoTarget;  // Wait for the next target change
        }
        return;
    }

    if (rr->state == regState_Pending || rr->state == regState_Refresh)
    {
        if (!err)
        {
            if (rr->state == regState_Refresh) InvokeCallback = mDNSfalse;
            rr->state = regState_Registered;
        }
        else
        {
            // Retry without lease only for non-Private domains
            LogMsg("hndlRecordUpdateReply: Registration of record %##s type %d failed with error %d", rr->resrec.name->c, rr->resrec.rrtype, err);
            if (!rr->Private && rr->uselease && err == mStatus_UnknownErr && mDNSSameIPPort(UpdatePort, UnicastDNSPort))
            {
                LogMsg("hndlRecordUpdateReply: Will retry update of record %##s without lease option", rr->resrec.name->c);
                rr->uselease = mDNSfalse;
                rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
                rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
                SetNextuDNSEvent(m, rr);
                return;
            }
            // Communicate the error to the application in the callback below
        }
    }

    if (rr->QueuedRData && rr->state == regState_Registered)
    {
        rr->state = regState_UpdatePending;
        rr->InFlightRData = rr->QueuedRData;
        rr->InFlightRDLen = rr->QueuedRDLen;
        rr->OrigRData = rr->resrec.rdata;
        rr->OrigRDLen = rr->resrec.rdlength;
        rr->QueuedRData = mDNSNULL;
        rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
        rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
        SetNextuDNSEvent(m, rr);
        return;
    }

    // Don't invoke the callback on error as this may not be useful to the client.
    // The client may potentially delete the resource record on error which we normally
    // delete during deregistration
    if (!err && InvokeCallback && rr->RecordCallback)
    {
        LogInfo("hndlRecordUpdateReply: Calling record callback on %##s", rr->resrec.name->c);
        mDNS_DropLockBeforeCallback();
        rr->RecordCallback(m, rr, err);
        mDNS_ReclaimLockAfterCallback();
    }
    // CAUTION: MUST NOT do anything more with rr after calling rr->Callback(), because the client's callback function
    // is allowed to do anything, including starting/stopping queries, registering/deregistering records, etc.
}

mDNSlocal void uDNS_ReceiveNATPMPPacket(mDNS *m, const mDNSInterfaceID InterfaceID, mDNSu8 *pkt, mDNSu16 len)
{
    NATTraversalInfo *ptr;
    NATAddrReply     *AddrReply    = (NATAddrReply    *)pkt;
    NATPortMapReply  *PortMapReply = (NATPortMapReply *)pkt;
    mDNSu32 nat_elapsed, our_elapsed;

    // Minimum NAT-PMP packet is vers (1) opcode (1) + err (2) = 4 bytes
    if (len < 4) { LogMsg("NAT-PMP message too short (%d bytes)", len); return; }
    
    // Read multi-byte error value (field is identical in a NATPortMapReply)
    AddrReply->err = (mDNSu16) ((mDNSu16)pkt[2] << 8 | pkt[3]);
    
    if (AddrReply->err == NATErr_Vers)
    {
        NATTraversalInfo *n;        
        LogInfo("NAT-PMP version unsupported message received");
        for (n = m->NATTraversals; n; n=n->next)
        {
            // Send a NAT-PMP request for this operation as needed
            // and update the state variables
            uDNS_SendNATMsg(m, n, mDNSfalse);
        }
        
        m->NextScheduledNATOp = m->timenow;

        return;
    }

    // The minimum reasonable NAT-PMP packet length is vers (1) + opcode (1) + err (2) + upseconds (4) = 8 bytes
    // If it's not at least this long, bail before we byte-swap the upseconds field & overrun our buffer.
    // The retry timer will ensure we converge to correctness.
    if (len < 8)
    {
        LogMsg("NAT-PMP message too short (%d bytes) 0x%X 0x%X", len, AddrReply->opcode, AddrReply->err);
        return;
    }
    
    // Read multi-byte upseconds value (field is identical in a NATPortMapReply)
    AddrReply->upseconds = (mDNSs32) ((mDNSs32)pkt[4] << 24 | (mDNSs32)pkt[5] << 16 | (mDNSs32)pkt[6] << 8 | pkt[7]);

    nat_elapsed = AddrReply->upseconds - m->LastNATupseconds;
    our_elapsed = (m->timenow - m->LastNATReplyLocalTime) / mDNSPlatformOneSecond;
    debugf("uDNS_ReceiveNATPMPPacket %X upseconds %u nat_elapsed %d our_elapsed %d", AddrReply->opcode, AddrReply->upseconds, nat_elapsed, our_elapsed);

    // We compute a conservative estimate of how much the NAT gateways's clock should have advanced
    // 1. We subtract 12.5% from our own measured elapsed time, to allow for NAT gateways that have an inacurate clock that runs slowly
    // 2. We add a two-second safety margin to allow for rounding errors: e.g.
    //    -- if NAT gateway sends a packet at t=2.000 seconds, then one at t=7.999, that's approximately 6 real seconds,
    //       but based on the values in the packet (2,7) the apparent difference according to the packet is only 5 seconds
    //    -- if we're slow handling packets and/or we have coarse clock granularity,
    //       we could receive the t=2 packet at our t=1.999 seconds, which we round down to 1
    //       and the t=7.999 packet at our t=8.000 seconds, which we record as 8,
    //       giving an apparent local time difference of 7 seconds
    //    The two-second safety margin coves this possible calculation discrepancy
    if (AddrReply->upseconds < m->LastNATupseconds || nat_elapsed + 2 < our_elapsed - our_elapsed/8)
    { LogMsg("NAT-PMP epoch time check failed: assuming NAT gateway %#a rebooted", &m->Router); RecreateNATMappings(m, 0); }

    m->LastNATupseconds      = AddrReply->upseconds;
    m->LastNATReplyLocalTime = m->timenow;
#ifdef _LEGACY_NAT_TRAVERSAL_
    LNT_ClearState(m);
#endif // _LEGACY_NAT_TRAVERSAL_

    if (AddrReply->opcode == NATOp_AddrResponse)
    {
#if APPLE_OSX_mDNSResponder
        static char msgbuf[16];
        mDNS_snprintf(msgbuf, sizeof(msgbuf), "%d", AddrReply->err);
        mDNSASLLog((uuid_t *)&m->asl_uuid, "natt.natpmp.AddressRequest", AddrReply->err ? "failure" : "success", msgbuf, "");
#endif
        if (!AddrReply->err && len < sizeof(NATAddrReply)) { LogMsg("NAT-PMP AddrResponse message too short (%d bytes)", len); return; }
        natTraversalHandleAddressReply(m, AddrReply->err, AddrReply->ExtAddr);
    }
    else if (AddrReply->opcode == NATOp_MapUDPResponse || AddrReply->opcode == NATOp_MapTCPResponse)
    {
        mDNSu8 Protocol = AddrReply->opcode & 0x7F;
#if APPLE_OSX_mDNSResponder
        static char msgbuf[16];
        mDNS_snprintf(msgbuf, sizeof(msgbuf), "%s - %d", AddrReply->opcode == NATOp_MapUDPResponse ? "UDP" : "TCP", PortMapReply->err);
        mDNSASLLog((uuid_t *)&m->asl_uuid, "natt.natpmp.PortMapRequest", PortMapReply->err ? "failure" : "success", msgbuf, "");
#endif
        if (!PortMapReply->err)
        {
            if (len < sizeof(NATPortMapReply)) { LogMsg("NAT-PMP PortMapReply message too short (%d bytes)", len); return; }
            PortMapReply->NATRep_lease = (mDNSu32) ((mDNSu32)pkt[12] << 24 | (mDNSu32)pkt[13] << 16 | (mDNSu32)pkt[14] << 8 | pkt[15]);
        }

        // Since some NAT-PMP server implementations don't return the requested internal port in
        // the reply, we can't associate this reply with a particular NATTraversalInfo structure.
        // We globally keep track of the most recent error code for mappings.
        m->LastNATMapResultCode = PortMapReply->err;

        for (ptr = m->NATTraversals; ptr; ptr=ptr->next)
            if (ptr->Protocol == Protocol && mDNSSameIPPort(ptr->IntPort, PortMapReply->intport))
                natTraversalHandlePortMapReply(m, ptr, InterfaceID, PortMapReply->err, PortMapReply->extport, PortMapReply->NATRep_lease, NATTProtocolNATPMP);
    }
    else { LogMsg("Received NAT-PMP response with unknown opcode 0x%X", AddrReply->opcode); return; }

    // Don't need an SSDP socket if we get a NAT-PMP packet
    if (m->SSDPSocket) { debugf("uDNS_ReceiveNATPMPPacket destroying SSDPSocket %p", &m->SSDPSocket); mDNSPlatformUDPClose(m->SSDPSocket); m->SSDPSocket = mDNSNULL; }
}

mDNSlocal void uDNS_ReceivePCPPacket(mDNS *m, const mDNSInterfaceID InterfaceID, mDNSu8 *pkt, mDNSu16 len)
{
    NATTraversalInfo *ptr;
    PCPMapReply *reply = (PCPMapReply*)pkt;
    mDNSu32 client_delta, server_delta;
    mDNSBool checkEpochValidity = m->LastNATupseconds != 0;
    mDNSu8 strippedOpCode;
    mDNSv4Addr mappedAddress = zerov4Addr;
    mDNSu8 protocol = 0;
    mDNSIPPort intport = zeroIPPort;
    mDNSIPPort extport = zeroIPPort;
    
    // Minimum PCP packet is 24 bytes
    if (len < 24)
    {
        LogMsg("uDNS_ReceivePCPPacket: message too short (%d bytes)", len);
        return;
    }
    
    strippedOpCode = reply->opCode & 0x7f;
    
    if ((reply->opCode & 0x80) == 0x00 || (strippedOpCode != PCPOp_Announce && strippedOpCode != PCPOp_Map))
    {
        LogMsg("uDNS_ReceivePCPPacket: unhandled opCode %u", reply->opCode);
        return;
    }

    // Read multi-byte values
    reply->lifetime = (mDNSs32)((mDNSs32)pkt[4] << 24 | (mDNSs32)pkt[5] << 16 | (mDNSs32)pkt[ 6] << 8 | pkt[ 7]);
    reply->epoch    = (mDNSs32)((mDNSs32)pkt[8] << 24 | (mDNSs32)pkt[9] << 16 | (mDNSs32)pkt[10] << 8 | pkt[11]);

    client_delta = (m->timenow - m->LastNATReplyLocalTime) / mDNSPlatformOneSecond;
    server_delta = reply->epoch - m->LastNATupseconds;
    debugf("uDNS_ReceivePCPPacket: %X %X upseconds %u client_delta %d server_delta %d", reply->opCode, reply->result, reply->epoch, client_delta, server_delta);

    // If seconds since the epoch is 0, use 1 so we'll check epoch validity next time
    m->LastNATupseconds      = reply->epoch ? reply->epoch : 1;
    m->LastNATReplyLocalTime = m->timenow;

#ifdef _LEGACY_NAT_TRAVERSAL_
    LNT_ClearState(m);
#endif // _LEGACY_NAT_TRAVERSAL_

    // Don't need an SSDP socket if we get a PCP packet
    if (m->SSDPSocket) { debugf("uDNS_ReceivePCPPacket: destroying SSDPSocket %p", &m->SSDPSocket); mDNSPlatformUDPClose(m->SSDPSocket); m->SSDPSocket = mDNSNULL; }

    if (checkEpochValidity && (client_delta + 2 < server_delta - server_delta / 16 || server_delta + 2 < client_delta - client_delta / 16))
    {
        // If this is an ANNOUNCE packet, wait a random interval up to 5 seconds
        // otherwise, refresh immediately
        mDNSu32 waitTicks = strippedOpCode ? 0 : mDNSRandom(PCP_WAITSECS_AFTER_EPOCH_INVALID * mDNSPlatformOneSecond);
        LogMsg("uDNS_ReceivePCPPacket: Epoch invalid, %#a likely rebooted, waiting %u ticks", &m->Router, waitTicks);
        RecreateNATMappings(m, waitTicks);
        // we can ignore the rest of this packet, as new requests are about to go out
        return;
    }

    if (strippedOpCode == PCPOp_Announce)
        return;
    
    // We globally keep track of the most recent error code for mappings.
    // This seems bad to do with PCP, but best not change it now.
    m->LastNATMapResultCode = reply->result;
    
    if (!reply->result)
    {
        if (len < sizeof(PCPMapReply))
        {
            LogMsg("uDNS_ReceivePCPPacket: mapping response too short (%d bytes)", len);
            return;
        }
        
        // Check the nonce
        if (reply->nonce[0] != m->PCPNonce[0] || reply->nonce[1] != m->PCPNonce[1] || reply->nonce[2] != m->PCPNonce[2])
        {
            LogMsg("uDNS_ReceivePCPPacket: invalid nonce, ignoring. received { %x %x %x } expected { %x %x %x }",
                   reply->nonce[0], reply->nonce[1], reply->nonce[2],
                    m->PCPNonce[0],  m->PCPNonce[1],  m->PCPNonce[2]);
            return;
        }

        // Get the values
        protocol = reply->protocol;
        intport = reply->intPort;
        extport = reply->extPort;

        // Get the external address, which should be mapped, since we only support IPv4
        if (!mDNSAddrIPv4FromMappedIPv6(&reply->extAddress, &mappedAddress))
        {
            LogMsg("uDNS_ReceivePCPPacket: unexpected external address: %.16a", &reply->extAddress);
            reply->result = NATErr_NetFail;
            // fall through to report the error
        }
        else if (mDNSIPv4AddressIsZero(mappedAddress))
        {
            // If this is the deletion case, we will have sent the zero IPv4-mapped address
            // in our request, and the server should reflect it in the response, so we
            // should not log about receiving a zero address. And in this case, we no
            // longer have a NATTraversal to report errors back to, so it's ok to set the
            // result here.
            // In other cases, a zero address is an error, and we will have a NATTraversal
            // to report back to, so set an error and fall through to report it.
            // CheckNATMappings will log the error.
            reply->result = NATErr_NetFail;
        }
    }
    else
    {
        LogInfo("uDNS_ReceivePCPPacket: error received from server. opcode %X result %X lifetime %X epoch %X",
                reply->opCode, reply->result, reply->lifetime, reply->epoch);
        
        // If the packet is long enough, get the protocol & intport for matching to report
        // the error
        if (len >= sizeof(PCPMapReply))
        {
            protocol = reply->protocol;
            intport = reply->intPort;
        }
    }

    for (ptr = m->NATTraversals; ptr; ptr=ptr->next)
    {
        mDNSu8 ptrProtocol = ((ptr->Protocol & NATOp_MapTCP) == NATOp_MapTCP ? PCPProto_TCP : PCPProto_UDP);
        if ((protocol == ptrProtocol && mDNSSameIPPort(ptr->IntPort, intport)) ||
            (!ptr->Protocol && protocol == PCPProto_TCP && mDNSSameIPPort(DiscardPort, intport)))
        {
            natTraversalHandlePortMapReplyWithAddress(m, ptr, InterfaceID, reply->result ? NATErr_NetFail : NATErr_None, mappedAddress, extport, reply->lifetime, NATTProtocolPCP);
        }
    }
}

mDNSexport void uDNS_ReceiveNATPacket(mDNS *m, const mDNSInterfaceID InterfaceID, mDNSu8 *pkt, mDNSu16 len)
{
    if (len == 0)
        LogMsg("uDNS_ReceiveNATPacket: zero length packet");
    else if (pkt[0] == PCP_VERS)
        uDNS_ReceivePCPPacket(m, InterfaceID, pkt, len);
    else if (pkt[0] == NATMAP_VERS)
        uDNS_ReceiveNATPMPPacket(m, InterfaceID, pkt, len);
    else
        LogMsg("uDNS_ReceiveNATPacket: packet with version %u (expected %u or %u)", pkt[0], PCP_VERS, NATMAP_VERS);
}

// <rdar://problem/3925163> Shorten DNS-SD queries to avoid NAT bugs
// <rdar://problem/4288449> Add check to avoid crashing NAT gateways that have buggy DNS relay code
//
// We know of bugs in home NAT gateways that cause them to crash if they receive certain DNS queries.
// The DNS queries that make them crash are perfectly legal DNS queries, but even if they weren't,
// the gateway shouldn't crash -- in today's world of viruses and network attacks, software has to
// be written assuming that a malicious attacker could send them any packet, properly-formed or not.
// Still, we don't want to be crashing people's home gateways, so we go out of our way to avoid
// the queries that crash them.
//
// Some examples:
//
// 1. Any query where the name ends in ".in-addr.arpa." and the text before this is 32 or more bytes.
//    The query type does not need to be PTR -- the gateway will crash for any query type.
//    e.g. "ping long-name-crashes-the-buggy-router.in-addr.arpa" will crash one of these.
//
// 2. Any query that results in a large response with the TC bit set.
//
// 3. Any PTR query that doesn't begin with four decimal numbers.
//    These gateways appear to assume that the only possible PTR query is a reverse-mapping query
//    (e.g. "1.0.168.192.in-addr.arpa") and if they ever get a PTR query where the first four
//    labels are not all decimal numbers in the range 0-255, they handle that by crashing.
//    These gateways also ignore the remainder of the name following the four decimal numbers
//    -- whether or not it actually says in-addr.arpa, they just make up an answer anyway.
//
// The challenge therefore is to craft a query that will discern whether the DNS server
// is one of these buggy ones, without crashing it. Furthermore we don't want our test
// queries making it all the way to the root name servers, putting extra load on those
// name servers and giving Apple a bad reputation. To this end we send this query:
//     dig -t ptr 1.0.0.127.dnsbugtest.1.0.0.127.in-addr.arpa.
//
// The text preceding the ".in-addr.arpa." is under 32 bytes, so it won't cause crash (1).
// It will not yield a large response with the TC bit set, so it won't cause crash (2).
// It starts with four decimal numbers, so it won't cause crash (3).
// The name falls within the "1.0.0.127.in-addr.arpa." domain, the reverse-mapping name for the local
// loopback address, and therefore the query will black-hole at the first properly-configured DNS server
// it reaches, making it highly unlikely that this query will make it all the way to the root.
//
// Finally, the correct response to this query is NXDOMAIN or a similar error, but the
// gateways that ignore the remainder of the name following the four decimal numbers
// give themselves away by actually returning a result for this nonsense query.

mDNSlocal const domainname *DNSRelayTestQuestion = (const domainname*)
                                                   "\x1" "1" "\x1" "0" "\x1" "0" "\x3" "127" "\xa" "dnsbugtest"
                                                   "\x1" "1" "\x1" "0" "\x1" "0" "\x3" "127" "\x7" "in-addr" "\x4" "arpa";

// See comments above for DNSRelayTestQuestion
// If this is the kind of query that has the risk of crashing buggy DNS servers, we do a test question first
mDNSlocal mDNSBool NoTestQuery(DNSQuestion *q)
{
    int i;
    mDNSu8 *p = q->qname.c;
    if (q->AuthInfo) return(mDNStrue);      // Don't need a test query for private queries sent directly to authoritative server over TLS/TCP
    if (q->qtype != kDNSType_PTR) return(mDNStrue);     // Don't need a test query for any non-PTR queries
    for (i=0; i<4; i++)     // If qname does not begin with num.num.num.num, can't skip the test query
    {
        if (p[0] < 1 || p[0] > 3) return(mDNSfalse);
        if (              p[1] < '0' || p[1] > '9' ) return(mDNSfalse);
        if (p[0] >= 2 && (p[2] < '0' || p[2] > '9')) return(mDNSfalse);
        if (p[0] >= 3 && (p[3] < '0' || p[3] > '9')) return(mDNSfalse);
        p += 1 + p[0];
    }
    // If remainder of qname is ".in-addr.arpa.", this is a vanilla reverse-mapping query and
    // we can safely do it without needing a test query first, otherwise we need the test query.
    return(SameDomainName((domainname*)p, (const domainname*)"\x7" "in-addr" "\x4" "arpa"));
}

// Returns mDNStrue if response was handled
mDNSlocal mDNSBool uDNS_ReceiveTestQuestionResponse(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
                                                    const mDNSAddr *const srcaddr, const mDNSIPPort srcport)
{
    const mDNSu8 *ptr = msg->data;
    DNSQuestion pktq;
    DNSServer *s;
    mDNSu32 result = 0;

    // 1. Find out if this is an answer to one of our test questions
    if (msg->h.numQuestions != 1) return(mDNSfalse);
    ptr = getQuestion(msg, ptr, end, mDNSInterface_Any, &pktq);
    if (!ptr) return(mDNSfalse);
    if (pktq.qtype != kDNSType_PTR || pktq.qclass != kDNSClass_IN) return(mDNSfalse);
    if (!SameDomainName(&pktq.qname, DNSRelayTestQuestion)) return(mDNSfalse);

    // 2. If the DNS relay gave us a positive response, then it's got buggy firmware
    // else, if the DNS relay gave us an error or no-answer response, it passed our test
    if ((msg->h.flags.b[1] & kDNSFlag1_RC_Mask) == kDNSFlag1_RC_NoErr && msg->h.numAnswers > 0)
        result = DNSServer_Failed;
    else
        result = DNSServer_Passed;

    // 3. Find occurrences of this server in our list, and mark them appropriately
    for (s = m->DNSServers; s; s = s->next)
    {
        mDNSBool matchaddr = (s->teststate != result && mDNSSameAddress(srcaddr, &s->addr) && mDNSSameIPPort(srcport, s->port));
        mDNSBool matchid   = (s->teststate == DNSServer_Untested && mDNSSameOpaque16(msg->h.id, s->testid));
        if (matchaddr || matchid)
        {
            DNSQuestion *q;
            s->teststate = result;
            if (result == DNSServer_Passed)
            {
                LogInfo("DNS Server %#a:%d (%#a:%d) %d passed%s",
                        &s->addr, mDNSVal16(s->port), srcaddr, mDNSVal16(srcport), mDNSVal16(s->testid),
                        matchaddr ? "" : " NOTE: Reply did not come from address to which query was sent");
            }
            else
            {
                LogMsg("NOTE: Wide-Area Service Discovery disabled to avoid crashing defective DNS relay %#a:%d (%#a:%d) %d%s",
                       &s->addr, mDNSVal16(s->port), srcaddr, mDNSVal16(srcport), mDNSVal16(s->testid),
                       matchaddr ? "" : " NOTE: Reply did not come from address to which query was sent");
            }

            // If this server has just changed state from DNSServer_Untested to DNSServer_Passed, then retrigger any waiting questions.
            // We use the NoTestQuery() test so that we only retrigger questions that were actually blocked waiting for this test to complete.
            if (result == DNSServer_Passed)     // Unblock any questions that were waiting for this result
                for (q = m->Questions; q; q=q->next)
                    if (q->qDNSServer == s && !NoTestQuery(q))
                    {
                        q->ThisQInterval = INIT_UCAST_POLL_INTERVAL / QuestionIntervalStep;
                        q->unansweredQueries = 0;
                        q->LastQTime = m->timenow - q->ThisQInterval;
                        m->NextScheduledQuery = m->timenow;
                    }
        }
    }

    return(mDNStrue); // Return mDNStrue to tell uDNS_ReceiveMsg it doesn't need to process this packet further
}

// Called from mDNSCoreReceive with the lock held
mDNSexport void uDNS_ReceiveMsg(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *const srcaddr, const mDNSIPPort srcport)
{
    DNSQuestion *qptr;
    mStatus err = mStatus_NoError;

    mDNSu8 StdR    = kDNSFlag0_QR_Response | kDNSFlag0_OP_StdQuery;
    mDNSu8 UpdateR = kDNSFlag0_QR_Response | kDNSFlag0_OP_Update;
    mDNSu8 QR_OP   = (mDNSu8)(msg->h.flags.b[0] & kDNSFlag0_QROP_Mask);
    mDNSu8 rcode   = (mDNSu8)(msg->h.flags.b[1] & kDNSFlag1_RC_Mask);

    (void)srcport; // Unused

    debugf("uDNS_ReceiveMsg from %#-15a with "
           "%2d Question%s %2d Answer%s %2d Authorit%s %2d Additional%s %d bytes",
           srcaddr,
           msg->h.numQuestions,   msg->h.numQuestions   == 1 ? ", "   : "s,",
           msg->h.numAnswers,     msg->h.numAnswers     == 1 ? ", "   : "s,",
           msg->h.numAuthorities, msg->h.numAuthorities == 1 ? "y,  " : "ies,",
           msg->h.numAdditionals, msg->h.numAdditionals == 1 ? ""     : "s", end - msg->data);

    if (QR_OP == StdR)
    {
        //if (srcaddr && recvLLQResponse(m, msg, end, srcaddr, srcport)) return;
        if (uDNS_ReceiveTestQuestionResponse(m, msg, end, srcaddr, srcport)) return;
        for (qptr = m->Questions; qptr; qptr = qptr->next)
            if (msg->h.flags.b[0] & kDNSFlag0_TC && mDNSSameOpaque16(qptr->TargetQID, msg->h.id) && m->timenow - qptr->LastQTime < RESPONSE_WINDOW)
            {
                if (!srcaddr) LogMsg("uDNS_ReceiveMsg: TCP DNS response had TC bit set: ignoring");
                else
                {
                    // Don't reuse TCP connections. We might have failed over to a different DNS server
                    // while the first TCP connection is in progress. We need a new TCP connection to the
                    // new DNS server. So, always try to establish a new connection.
                    if (qptr->tcp) { DisposeTCPConn(qptr->tcp); qptr->tcp = mDNSNULL; }
                    qptr->tcp = MakeTCPConn(m, mDNSNULL, mDNSNULL, kTCPSocketFlags_Zero, srcaddr, srcport, mDNSNULL, qptr, mDNSNULL);
                }
            }
    }

    if (QR_OP == UpdateR)
    {
        mDNSu32 lease = GetPktLease(m, msg, end);
        mDNSs32 expire = m->timenow + (mDNSs32)lease * mDNSPlatformOneSecond;
        mDNSu32 random = mDNSRandom((mDNSs32)lease * mDNSPlatformOneSecond/10);

        //rcode = kDNSFlag1_RC_ServFail;	// Simulate server failure (rcode 2)

        // Walk through all the records that matches the messageID. There could be multiple
        // records if we had sent them in a group
        if (m->CurrentRecord)
            LogMsg("uDNS_ReceiveMsg ERROR m->CurrentRecord already set %s", ARDisplayString(m, m->CurrentRecord));
        m->CurrentRecord = m->ResourceRecords;
        while (m->CurrentRecord)
        {
            AuthRecord *rptr = m->CurrentRecord;
            m->CurrentRecord = m->CurrentRecord->next;
            if (AuthRecord_uDNS(rptr) && mDNSSameOpaque16(rptr->updateid, msg->h.id))
            {
                err = checkUpdateResult(m, rptr->resrec.name, rcode, msg, end);
                if (!err && rptr->uselease && lease)
                    if (rptr->expire - expire >= 0 || rptr->state != regState_UpdatePending)
                    {
                        rptr->expire = expire;
                        rptr->refreshCount = 0;
                    }
                // We pass the random value to make sure that if we update multiple
                // records, they all get the same random value
                hndlRecordUpdateReply(m, rptr, err, random);
            }
        }
    }
    debugf("Received unexpected response: ID %d matches no active records", mDNSVal16(msg->h.id));
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Query Routines
#endif

mDNSexport void sendLLQRefresh(mDNS *m, DNSQuestion *q)
{
    mDNSu8 *end;
    LLQOptData llq;
    mDNSu8 *limit = m->omsg.data + AbsoluteMaxDNSMessageData;

    if (q->ReqLease)
        if ((q->state == LLQ_Established && q->ntries >= kLLQ_MAX_TRIES) || q->expire - m->timenow < 0)
        {
            LogMsg("Unable to refresh LLQ %##s (%s) - will retry in %d seconds", q->qname.c, DNSTypeName(q->qtype), LLQ_POLL_INTERVAL / mDNSPlatformOneSecond);
            StartLLQPolling(m,q);
            return;
        }

    llq.vers     = kLLQ_Vers;
    llq.llqOp    = kLLQOp_Refresh;
    llq.err      = q->tcp ? GetLLQEventPort(m, &q->servAddr) : LLQErr_NoError;  // If using TCP tell server what UDP port to send notifications to
    llq.id       = q->id;
    llq.llqlease = q->ReqLease;

    InitializeDNSMessage(&m->omsg.h, q->TargetQID, uQueryFlags);
    end = putLLQ(&m->omsg, m->omsg.data, q, &llq);
    if (!end) { LogMsg("sendLLQRefresh: putLLQ failed %##s (%s)", q->qname.c, DNSTypeName(q->qtype)); return; }

    // Note that we (conditionally) add HINFO and TSIG here, since the question might be going away,
    // so we may not be able to reference it (most importantly it's AuthInfo) when we actually send the message
#ifndef REMOVE_TUNNELING
    end = putHINFO(m, &m->omsg, end, q->AuthInfo, limit);
    if (!end) { LogMsg("sendLLQRefresh: putHINFO failed %##s (%s)", q->qname.c, DNSTypeName(q->qtype)); return; }
#endif // #ifndef REMOVE_TUNNELING

    if (PrivateQuery(q))
    {
        DNSDigest_SignMessageHostByteOrder(&m->omsg, &end, q->AuthInfo);
        if (!end) { LogMsg("sendLLQRefresh: DNSDigest_SignMessage failed %##s (%s)", q->qname.c, DNSTypeName(q->qtype)); return; }
    }

    if (PrivateQuery(q) && !q->tcp)
    {
        LogInfo("sendLLQRefresh setting up new TLS session %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        if (!q->nta)
        {
            // Note: If a question is in LLQ_Established state, we never free the zone data for the
            // question (PrivateQuery). If we free, we reset the state to something other than LLQ_Established.
            // This function is called only if the query is in LLQ_Established state and hence nta should
            // never be NULL. In spite of that, we have seen q->nta being NULL in the field. Just refetch the
            // zone data in that case.
            q->nta = StartGetZoneData(m, &q->qname, ZoneServiceLLQ, LLQGotZoneData, q);
            return;
            // ThisQInterval is not adjusted when we return from here which means that we will get called back
            // again immediately. As q->servAddr and q->servPort are still valid and the nta->Host is initialized
            // without any additional discovery for PrivateQuery, things work. 
        }
        q->tcp = MakeTCPConn(m, &m->omsg, end, kTCPSocketFlags_UseTLS, &q->servAddr, q->servPort, &q->nta->Host, q, mDNSNULL);
    }
    else
    {
        mStatus err;

        // if AuthInfo and AuthInfo->AutoTunnel is set, we use the TCP socket but don't need to pass the AuthInfo as
        // we already protected the message above.
        LogInfo("sendLLQRefresh: using existing %s session %##s (%s)", PrivateQuery(q) ? "TLS" : "UDP",
                q->qname.c, DNSTypeName(q->qtype));

        err = mDNSSendDNSMessage(m, &m->omsg, end, mDNSInterface_Any, q->LocalSocket, &q->servAddr, q->servPort, q->tcp ? q->tcp->sock : mDNSNULL, mDNSNULL, mDNSfalse);
        if (err)
        {
            LogMsg("sendLLQRefresh: mDNSSendDNSMessage%s failed: %d", q->tcp ? " (TCP)" : "", err);
            if (q->tcp) { DisposeTCPConn(q->tcp); q->tcp = mDNSNULL; }
        }
    }

    q->ntries++;

    debugf("sendLLQRefresh ntries %d %##s (%s)", q->ntries, q->qname.c, DNSTypeName(q->qtype));

    q->LastQTime = m->timenow;
    SetNextQueryTime(m, q);
}

mDNSexport void LLQGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneInfo)
{
    DNSQuestion *q = (DNSQuestion *)zoneInfo->ZoneDataContext;

    mDNS_Lock(m);

    // If we get here it means that the GetZoneData operation has completed.
    // We hold on to the zone data if it is AutoTunnel as we use the hostname
    // in zoneInfo during the TLS connection setup.
    q->servAddr = zeroAddr;
    q->servPort = zeroIPPort;

    if (!err && zoneInfo && !mDNSIPPortIsZero(zoneInfo->Port) && !mDNSAddressIsZero(&zoneInfo->Addr) && zoneInfo->Host.c[0])
    {
        q->servAddr = zoneInfo->Addr;
        q->servPort = zoneInfo->Port;
        if (!PrivateQuery(q))
        {
            // We don't need the zone data as we use it only for the Host information which we
            // don't need if we are not going to use TLS connections.
            if (q->nta)
            {
                if (q->nta != zoneInfo) LogMsg("LLQGotZoneData: nta (%p) != zoneInfo (%p)  %##s (%s)", q->nta, zoneInfo, q->qname.c, DNSTypeName(q->qtype));
                CancelGetZoneData(m, q->nta);
                q->nta = mDNSNULL;
            }
        }
        q->ntries = 0;
        debugf("LLQGotZoneData %#a:%d", &q->servAddr, mDNSVal16(q->servPort));
        startLLQHandshake(m, q);
    }
    else
    {
        if (q->nta)
        {
            if (q->nta != zoneInfo) LogMsg("LLQGotZoneData: nta (%p) != zoneInfo (%p)  %##s (%s)", q->nta, zoneInfo, q->qname.c, DNSTypeName(q->qtype));
            CancelGetZoneData(m, q->nta);
            q->nta = mDNSNULL;
        }
        StartLLQPolling(m,q);
        if (err == mStatus_NoSuchNameErr)
        {
            // this actually failed, so mark it by setting address to all ones
            q->servAddr.type = mDNSAddrType_IPv4;
            q->servAddr.ip.v4 = onesIPv4Addr;
        }
    }

    mDNS_Unlock(m);
}

// Called in normal callback context (i.e. mDNS_busy and mDNS_reentrancy are both 1)
mDNSlocal void PrivateQueryGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneInfo)
{
    DNSQuestion *q = (DNSQuestion *) zoneInfo->ZoneDataContext;

    LogInfo("PrivateQueryGotZoneData %##s (%s) err %d Zone %##s Private %d", q->qname.c, DNSTypeName(q->qtype), err, zoneInfo->ZoneName.c, zoneInfo->ZonePrivate);

    if (q->nta != zoneInfo) LogMsg("PrivateQueryGotZoneData:ERROR!!: nta (%p) != zoneInfo (%p)  %##s (%s)", q->nta, zoneInfo, q->qname.c, DNSTypeName(q->qtype));

    if (err || !zoneInfo || mDNSAddressIsZero(&zoneInfo->Addr) || mDNSIPPortIsZero(zoneInfo->Port) || !zoneInfo->Host.c[0])
    {
        LogInfo("PrivateQueryGotZoneData: ERROR!! %##s (%s) invoked with error code %d %p %#a:%d",
                q->qname.c, DNSTypeName(q->qtype), err, zoneInfo,
                zoneInfo ? &zoneInfo->Addr : mDNSNULL,
                zoneInfo ? mDNSVal16(zoneInfo->Port) : 0);
        CancelGetZoneData(m, q->nta);
        q->nta = mDNSNULL;
        return;
    }

    if (!zoneInfo->ZonePrivate)
    {
        debugf("Private port lookup failed -- retrying without TLS -- %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
        q->AuthInfo      = mDNSNULL;        // Clear AuthInfo so we try again non-private
        q->ThisQInterval = InitialQuestionInterval;
        q->LastQTime     = m->timenow - q->ThisQInterval;
        CancelGetZoneData(m, q->nta);
        q->nta = mDNSNULL;
        mDNS_Lock(m);
        SetNextQueryTime(m, q);
        mDNS_Unlock(m);
        return;
        // Next call to uDNS_CheckCurrentQuestion() will do this as a non-private query
    }

    if (!PrivateQuery(q))
    {
        LogMsg("PrivateQueryGotZoneData: ERROR!! Not a private query %##s (%s) AuthInfo %p", q->qname.c, DNSTypeName(q->qtype), q->AuthInfo);
        CancelGetZoneData(m, q->nta);
        q->nta = mDNSNULL;
        return;
    }

    q->TargetQID = mDNS_NewMessageID(m);
    if (q->tcp) { DisposeTCPConn(q->tcp); q->tcp = mDNSNULL; }
    if (!q->nta) { LogMsg("PrivateQueryGotZoneData:ERROR!! nta is NULL for %##s (%s)", q->qname.c, DNSTypeName(q->qtype)); return; }
    q->tcp = MakeTCPConn(m, mDNSNULL, mDNSNULL, kTCPSocketFlags_UseTLS, &zoneInfo->Addr, zoneInfo->Port, &q->nta->Host, q, mDNSNULL);
    if (q->nta) { CancelGetZoneData(m, q->nta); q->nta = mDNSNULL; }
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Dynamic Updates
#endif

// Called in normal callback context (i.e. mDNS_busy and mDNS_reentrancy are both 1)
mDNSexport void RecordRegistrationGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneData)
{
    AuthRecord *newRR = (AuthRecord*)zoneData->ZoneDataContext;
    AuthRecord *ptr;
    int c1, c2;

    if (newRR->nta != zoneData)
        LogMsg("RecordRegistrationGotZoneData: nta (%p) != zoneData (%p)  %##s (%s)", newRR->nta, zoneData, newRR->resrec.name->c, DNSTypeName(newRR->resrec.rrtype));

    if (m->mDNS_busy != m->mDNS_reentrancy)
        LogMsg("RecordRegistrationGotZoneData: mDNS_busy (%ld) != mDNS_reentrancy (%ld)", m->mDNS_busy, m->mDNS_reentrancy);

    // make sure record is still in list (!!!)
    for (ptr = m->ResourceRecords; ptr; ptr = ptr->next) if (ptr == newRR) break;
    if (!ptr)
    {
        LogMsg("RecordRegistrationGotZoneData - RR no longer in list.  Discarding.");
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }

    // check error/result
    if (err)
    {
        if (err != mStatus_NoSuchNameErr) LogMsg("RecordRegistrationGotZoneData: error %d", err);
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }

    if (!zoneData) { LogMsg("ERROR: RecordRegistrationGotZoneData invoked with NULL result and no error"); return; }

    if (newRR->resrec.rrclass != zoneData->ZoneClass)
    {
        LogMsg("ERROR: New resource record's class (%d) does not match zone class (%d)", newRR->resrec.rrclass, zoneData->ZoneClass);
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }

    // Don't try to do updates to the root name server.
    // We might be tempted also to block updates to any single-label name server (e.g. com, edu, net, etc.) but some
    // organizations use their own private pseudo-TLD, like ".home", etc, and we don't want to block that.
    if (zoneData->ZoneName.c[0] == 0)
    {
        LogInfo("RecordRegistrationGotZoneData: No name server found claiming responsibility for \"%##s\"!", newRR->resrec.name->c);
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }

    // Store discovered zone data
    c1 = CountLabels(newRR->resrec.name);
    c2 = CountLabels(&zoneData->ZoneName);
    if (c2 > c1)
    {
        LogMsg("RecordRegistrationGotZoneData: Zone \"%##s\" is longer than \"%##s\"", zoneData->ZoneName.c, newRR->resrec.name->c);
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }
    newRR->zone = SkipLeadingLabels(newRR->resrec.name, c1-c2);
    if (!SameDomainName(newRR->zone, &zoneData->ZoneName))
    {
        LogMsg("RecordRegistrationGotZoneData: Zone \"%##s\" does not match \"%##s\" for \"%##s\"", newRR->zone->c, zoneData->ZoneName.c, newRR->resrec.name->c);
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }

    if (mDNSIPPortIsZero(zoneData->Port) || mDNSAddressIsZero(&zoneData->Addr) || !zoneData->Host.c[0])
    {
        LogInfo("RecordRegistrationGotZoneData: No _dns-update._udp service found for \"%##s\"!", newRR->resrec.name->c);
        CancelGetZoneData(m, newRR->nta);
        newRR->nta = mDNSNULL;
        return;
    }

    newRR->Private      = zoneData->ZonePrivate;
    debugf("RecordRegistrationGotZoneData: Set zone information for %##s %##s to %#a:%d",
           newRR->resrec.name->c, zoneData->ZoneName.c, &zoneData->Addr, mDNSVal16(zoneData->Port));

    // If we are deregistering, uDNS_DeregisterRecord will do that as it has the zone data now.
    if (newRR->state == regState_DeregPending)
    {
        mDNS_Lock(m);
        uDNS_DeregisterRecord(m, newRR);
        mDNS_Unlock(m);
        return;
    }

    if (newRR->resrec.rrtype == kDNSType_SRV)
    {
        const domainname *target;
        // Reevaluate the target always as NAT/Target could have changed while
        // we were fetching zone data.
        mDNS_Lock(m);
        target = GetServiceTarget(m, newRR);
        mDNS_Unlock(m);
        if (!target || target->c[0] == 0)
        {
            domainname *t = GetRRDomainNameTarget(&newRR->resrec);
            LogInfo("RecordRegistrationGotZoneData - no target for %##s", newRR->resrec.name->c);
            if (t) t->c[0] = 0;
            newRR->resrec.rdlength = newRR->resrec.rdestimate = 0;
            newRR->state = regState_NoTarget;
            CancelGetZoneData(m, newRR->nta);
            newRR->nta = mDNSNULL;
            return;
        }
    }
    // If we have non-zero service port (always?)
    // and a private address, and update server is non-private
    // and this service is AutoTarget
    // then initiate a NAT mapping request. On completion it will do SendRecordRegistration() for us
    if (newRR->resrec.rrtype == kDNSType_SRV && !mDNSIPPortIsZero(newRR->resrec.rdata->u.srv.port) &&
        mDNSv4AddrIsRFC1918(&m->AdvertisedV4.ip.v4) && newRR->nta && !mDNSAddrIsRFC1918(&newRR->nta->Addr) &&
        newRR->AutoTarget == Target_AutoHostAndNATMAP)
    {
        DomainAuthInfo *AuthInfo;
        AuthInfo = GetAuthInfoForName(m, newRR->resrec.name);
        if (AuthInfo && AuthInfo->AutoTunnel)
        {
            domainname *t = GetRRDomainNameTarget(&newRR->resrec);
            LogMsg("RecordRegistrationGotZoneData: ERROR!! AutoTunnel has Target_AutoHostAndNATMAP for %s", ARDisplayString(m, newRR));
            if (t) t->c[0] = 0;
            newRR->resrec.rdlength = newRR->resrec.rdestimate = 0;
            newRR->state = regState_NoTarget;
            CancelGetZoneData(m, newRR->nta);
            newRR->nta = mDNSNULL;
            return;
        }
        // During network transitions, we are called multiple times in different states. Setup NAT
        // state just once for this record.
        if (!newRR->NATinfo.clientContext)
        {
            LogInfo("RecordRegistrationGotZoneData StartRecordNatMap %s", ARDisplayString(m, newRR));
            newRR->state = regState_NATMap;
            StartRecordNatMap(m, newRR);
            return;
        }
        else LogInfo("RecordRegistrationGotZoneData: StartRecordNatMap for %s, state %d, context %p", ARDisplayString(m, newRR), newRR->state, newRR->NATinfo.clientContext);
    }
    mDNS_Lock(m);
    // We want IsRecordMergeable to check whether it is a record whose update can be
    // sent with others. We set the time before we call IsRecordMergeable, so that
    // it does not fail this record based on time. We are interested in other checks
    // at this time. If a previous update resulted in error, then don't reset the
    // interval. Preserve the back-off so that we don't keep retrying aggressively.
    if (newRR->updateError == mStatus_NoError)
    {
        newRR->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
        newRR->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
    }
    if (IsRecordMergeable(m, newRR, m->timenow + MERGE_DELAY_TIME))
    {
        // Delay the record registration by MERGE_DELAY_TIME so that we can merge them
        // into one update
        LogInfo("RecordRegistrationGotZoneData: Delayed registration for %s", ARDisplayString(m, newRR));
        newRR->LastAPTime += MERGE_DELAY_TIME;
    }
    mDNS_Unlock(m);
}

mDNSlocal void SendRecordDeregistration(mDNS *m, AuthRecord *rr)
{
    mDNSu8 *ptr = m->omsg.data;
    mDNSu8 *limit;
    DomainAuthInfo *AuthInfo;

    mDNS_CheckLock(m);

    if (!rr->nta || mDNSIPv4AddressIsZero(rr->nta->Addr.ip.v4))
    {
        LogMsg("SendRecordDeRegistration: No zone info for Resource record %s RecordType %d", ARDisplayString(m, rr), rr->resrec.RecordType);
        return;
    }

    limit = ptr + AbsoluteMaxDNSMessageData;
    AuthInfo = GetAuthInfoForName_internal(m, rr->resrec.name);
    limit -= RRAdditionalSize(m, AuthInfo);

    rr->updateid = mDNS_NewMessageID(m);
    InitializeDNSMessage(&m->omsg.h, rr->updateid, UpdateReqFlags);

    // set zone
    ptr = putZone(&m->omsg, ptr, limit, rr->zone, mDNSOpaque16fromIntVal(rr->resrec.rrclass));
    if (!ptr) goto exit;

    ptr = BuildUpdateMessage(m, ptr, rr, limit);

    if (!ptr) goto exit;

    if (rr->Private)
    {
        LogInfo("SendRecordDeregistration TCP %p %s", rr->tcp, ARDisplayString(m, rr));
        if (rr->tcp) LogInfo("SendRecordDeregistration: Disposing existing TCP connection for %s", ARDisplayString(m, rr));
        if (rr->tcp) { DisposeTCPConn(rr->tcp); rr->tcp = mDNSNULL; }
        if (!rr->nta) { LogMsg("SendRecordDeregistration:Private:ERROR!! nta is NULL for %s", ARDisplayString(m, rr)); return; }
        rr->tcp = MakeTCPConn(m, &m->omsg, ptr, kTCPSocketFlags_UseTLS, &rr->nta->Addr, rr->nta->Port, &rr->nta->Host, mDNSNULL, rr);
    }
    else
    {
        mStatus err;
        LogInfo("SendRecordDeregistration UDP %s", ARDisplayString(m, rr));
        if (!rr->nta) { LogMsg("SendRecordDeregistration:ERROR!! nta is NULL for %s", ARDisplayString(m, rr)); return; }
        err = mDNSSendDNSMessage(m, &m->omsg, ptr, mDNSInterface_Any, mDNSNULL, &rr->nta->Addr, rr->nta->Port, mDNSNULL, GetAuthInfoForName_internal(m, rr->resrec.name), mDNSfalse);
        if (err) debugf("ERROR: SendRecordDeregistration - mDNSSendDNSMessage - %d", err);
        //if (rr->state == regState_DeregPending) CompleteDeregistration(m, rr);		// Don't touch rr after this
    }
    SetRecordRetry(m, rr, 0);
    return;
exit:
    LogMsg("SendRecordDeregistration: Error formatting message for %s", ARDisplayString(m, rr));
}

mDNSexport mStatus uDNS_DeregisterRecord(mDNS *const m, AuthRecord *const rr)
{
    DomainAuthInfo *info;

    LogInfo("uDNS_DeregisterRecord: Resource Record %s, state %d", ARDisplayString(m, rr), rr->state);

    switch (rr->state)
    {
    case regState_Refresh:
    case regState_Pending:
    case regState_UpdatePending:
    case regState_Registered: break;
    case regState_DeregPending: break;

    case regState_NATError:
    case regState_NATMap:
    // A record could be in NoTarget to start with if the corresponding SRV record could not find a target.
    // It is also possible to reenter the NoTarget state when we move to a network with a NAT that has
    // no {PCP, NAT-PMP, UPnP/IGD} support. In that case before we entered NoTarget, we already deregistered with
    // the server.
    case regState_NoTarget:
    case regState_Unregistered:
    case regState_Zero:
    default:
        LogInfo("uDNS_DeregisterRecord: State %d for %##s type %s", rr->state, rr->resrec.name->c, DNSTypeName(rr->resrec.rrtype));
        // This function may be called during sleep when there are no sleep proxy servers
        if (rr->resrec.RecordType == kDNSRecordTypeDeregistering) CompleteDeregistration(m, rr);
        return mStatus_NoError;
    }

    // if unsent rdata is queued, free it.
    //
    // The data may be queued in QueuedRData or InFlightRData.
    //
    // 1) If the record is in Registered state, we store it in InFlightRData and copy the same in "rdata"
    //   *just* before sending the update to the server. Till we get the response, InFlightRData and "rdata"
    //   in the resource record are same. We don't want to free in that case. It will be freed when "rdata"
    //   is freed. If they are not same, the update has not been sent and we should free it here.
    //
    // 2) If the record is in UpdatePending state, we queue the update in QueuedRData. When the previous update
    //   comes back from the server, we copy it from QueuedRData to InFlightRData and repeat (1). This implies
    //   that QueuedRData can never be same as "rdata" in the resource record. As long as we have something
    //   left in QueuedRData, we should free it here.

    if (rr->InFlightRData && rr->UpdateCallback)
    {
        if (rr->InFlightRData != rr->resrec.rdata)
        {
            LogInfo("uDNS_DeregisterRecord: Freeing InFlightRData for %s", ARDisplayString(m, rr));
            rr->UpdateCallback(m, rr, rr->InFlightRData, rr->InFlightRDLen);
            rr->InFlightRData = mDNSNULL;
        }
        else
            LogInfo("uDNS_DeregisterRecord: InFlightRData same as rdata for %s", ARDisplayString(m, rr));
    }

    if (rr->QueuedRData && rr->UpdateCallback)
    {
        if (rr->QueuedRData == rr->resrec.rdata)
            LogMsg("uDNS_DeregisterRecord: ERROR!! QueuedRData same as rdata for %s", ARDisplayString(m, rr));
        else
        {
            LogInfo("uDNS_DeregisterRecord: Freeing QueuedRData for %s", ARDisplayString(m, rr));
            rr->UpdateCallback(m, rr, rr->QueuedRData, rr->QueuedRDLen);
            rr->QueuedRData = mDNSNULL;
        }
    }

    // If a current group registration is pending, we can't send this deregisration till that registration
    // has reached the server i.e., the ordering is important. Previously, if we did not send this
    // registration in a group, then the previous connection will be torn down as part of sending the
    // deregistration. If we send this in a group, we need to locate the resource record that was used
    // to send this registration and terminate that connection. This means all the updates on that might
    // be lost (assuming the response is not waiting for us at the socket) and the retry will send the
    // update again sometime in the near future.
    //
    // NOTE: SSL handshake failures normally free the TCP connection immediately. Hence, you may not
    // find the TCP below there. This case can happen only when tcp is trying to actively retransmit
    // the request or SSL negotiation taking time i.e resource record is actively trying to get the
    // message to the server. During that time a deregister has to happen.

    if (!mDNSOpaque16IsZero(rr->updateid))
    {
        AuthRecord *anchorRR;
        mDNSBool found = mDNSfalse;
        for (anchorRR = m->ResourceRecords; anchorRR; anchorRR = anchorRR->next)
        {
            if (AuthRecord_uDNS(rr) && mDNSSameOpaque16(anchorRR->updateid, rr->updateid) && anchorRR->tcp)
            {
                LogInfo("uDNS_DeregisterRecord: Found Anchor RR %s terminated", ARDisplayString(m, anchorRR));
                if (found)
                    LogMsg("uDNS_DeregisterRecord: ERROR: Another anchorRR %s found", ARDisplayString(m, anchorRR));
                DisposeTCPConn(anchorRR->tcp);
                anchorRR->tcp = mDNSNULL;
                found = mDNStrue;
            }
        }
        if (!found) LogInfo("uDNSDeregisterRecord: Cannot find the anchor Resource Record for %s, not an error", ARDisplayString(m, rr));
    }

    // Retry logic for deregistration should be no different from sending registration the first time.
    // Currently ThisAPInterval most likely is set to the refresh interval
    rr->state          = regState_DeregPending;
    rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
    rr->LastAPTime     = m->timenow - INIT_RECORD_REG_INTERVAL;
    info = GetAuthInfoForName_internal(m, rr->resrec.name);
    if (IsRecordMergeable(m, rr, m->timenow + MERGE_DELAY_TIME))
    {
        // Delay the record deregistration by MERGE_DELAY_TIME so that we can merge them
        // into one update. If the domain is being deleted, delay by 2 * MERGE_DELAY_TIME
        // so that we can merge all the AutoTunnel records and the service records in
        // one update (they get deregistered a little apart)
        if (info && info->deltime) rr->LastAPTime += (2 * MERGE_DELAY_TIME);
        else rr->LastAPTime += MERGE_DELAY_TIME;
    }
    // IsRecordMergeable could have returned false for several reasons e.g., DontMerge is set or
    // no zone information. Most likely it is the latter, CheckRecordUpdates will fetch the zone
    // data when it encounters this record.

    if (m->NextuDNSEvent - (rr->LastAPTime + rr->ThisAPInterval) >= 0)
        m->NextuDNSEvent = (rr->LastAPTime + rr->ThisAPInterval);

    return mStatus_NoError;
}

mDNSexport mStatus uDNS_UpdateRecord(mDNS *m, AuthRecord *rr)
{
    LogInfo("uDNS_UpdateRecord: Resource Record %##s, state %d", rr->resrec.name->c, rr->state);
    switch(rr->state)
    {
    case regState_DeregPending:
    case regState_Unregistered:
        // not actively registered
        goto unreg_error;

    case regState_NATMap:
    case regState_NoTarget:
        // change rdata directly since it hasn't been sent yet
        if (rr->UpdateCallback) rr->UpdateCallback(m, rr, rr->resrec.rdata, rr->resrec.rdlength);
        SetNewRData(&rr->resrec, rr->NewRData, rr->newrdlength);
        rr->NewRData = mDNSNULL;
        return mStatus_NoError;

    case regState_Pending:
    case regState_Refresh:
    case regState_UpdatePending:
        // registration in-flight. queue rdata and return
        if (rr->QueuedRData && rr->UpdateCallback)
            // if unsent rdata is already queued, free it before we replace it
            rr->UpdateCallback(m, rr, rr->QueuedRData, rr->QueuedRDLen);
        rr->QueuedRData = rr->NewRData;
        rr->QueuedRDLen = rr->newrdlength;
        rr->NewRData = mDNSNULL;
        return mStatus_NoError;

    case regState_Registered:
        rr->OrigRData = rr->resrec.rdata;
        rr->OrigRDLen = rr->resrec.rdlength;
        rr->InFlightRData = rr->NewRData;
        rr->InFlightRDLen = rr->newrdlength;
        rr->NewRData = mDNSNULL;
        rr->state = regState_UpdatePending;
        rr->ThisAPInterval = INIT_RECORD_REG_INTERVAL;
        rr->LastAPTime = m->timenow - INIT_RECORD_REG_INTERVAL;
        SetNextuDNSEvent(m, rr);
        return mStatus_NoError;

    case regState_NATError:
        LogMsg("ERROR: uDNS_UpdateRecord called for record %##s with bad state regState_NATError", rr->resrec.name->c);
        return mStatus_UnknownErr;      // states for service records only

    default: LogMsg("uDNS_UpdateRecord: Unknown state %d for %##s", rr->state, rr->resrec.name->c);
    }

unreg_error:
    LogMsg("uDNS_UpdateRecord: Requested update of record %##s type %d, in erroneous state %d",
           rr->resrec.name->c, rr->resrec.rrtype, rr->state);
    return mStatus_Invalid;
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Periodic Execution Routines
#endif

mDNSlocal void handle_unanswered_query(mDNS *const m)
{
    DNSQuestion *q = m->CurrentQuestion;

    if (q->unansweredQueries >= MAX_DNSSEC_UNANSWERED_QUERIES && DNSSECOptionalQuestion(q))
    {
        // If we are not receiving any responses for DNSSEC question, it could be due to
        // a broken middlebox or a DNS server that does not understand the EDNS0/DOK option that
        // silently drops the packets. Also as per RFC 5625 there are certain buggy DNS Proxies
        // that are known to drop these pkts. To handle this, we turn off sending the EDNS0/DOK
        // option if we have not received any responses indicating that the server or
        // the middlebox is DNSSEC aware. If we receive at least one response to a DNSSEC
        // question, we don't turn off validation. Also, we wait for MAX_DNSSEC_RETRANSMISSIONS
        // before turning off validation to accomodate packet loss.
        // 
        // Note: req_DO affects only DNSSEC_VALIDATION_SECURE_OPTIONAL questions;
        // DNSSEC_VALIDATION_SECURE questions ignores req_DO.

        if (q->qDNSServer && !q->qDNSServer->DNSSECAware && q->qDNSServer->req_DO)
        {
            q->qDNSServer->retransDO++;
            if (q->qDNSServer->retransDO == MAX_DNSSEC_RETRANSMISSIONS)
            {
                LogInfo("handle_unanswered_query: setting req_DO false for %#a", &q->qDNSServer->addr);
                q->qDNSServer->req_DO = mDNSfalse;
            }
        }

        if (!q->qDNSServer->req_DO)
        {
            q->ValidationState     = DNSSECValNotRequired; 
            q->ValidationRequired  = DNSSEC_VALIDATION_NONE;
        
            if (q->ProxyQuestion)
                q->ProxyDNSSECOK = mDNSfalse;
            LogInfo("handle_unanswered_query: unanswered query for %##s (%s), so turned off validation for %#a", 
                q->qname.c, DNSTypeName(q->qtype), &q->qDNSServer->addr);
        }
    }
}

// The question to be checked is not passed in as an explicit parameter;
// instead it is implicit that the question to be checked is m->CurrentQuestion.
mDNSexport void uDNS_CheckCurrentQuestion(mDNS *const m)
{
    DNSQuestion *q = m->CurrentQuestion;
    if (m->timenow - NextQSendTime(q) < 0) return;

    if (q->LongLived)
    {
        switch (q->state)
        {
        case LLQ_InitialRequest:   startLLQHandshake(m, q); break;
        case LLQ_SecondaryRequest:
            // For PrivateQueries, we need to start the handshake again as we don't do the Challenge/Response step
            if (PrivateQuery(q))
                startLLQHandshake(m, q);
            else
                sendChallengeResponse(m, q, mDNSNULL);
            break;
        case LLQ_Established:      sendLLQRefresh(m, q); break;
        case LLQ_Poll:             break;       // Do nothing (handled below)
        }
    }

    handle_unanswered_query(m);  
    // We repeat the check above (rather than just making this the "else" case) because startLLQHandshake can change q->state to LLQ_Poll
    if (!(q->LongLived && q->state != LLQ_Poll))
    {
        if (q->unansweredQueries >= MAX_UCAST_UNANSWERED_QUERIES)
        {
            DNSServer *orig = q->qDNSServer;
            if (orig)
                LogInfo("uDNS_CheckCurrentQuestion: Sent %d unanswered queries for %##s (%s) to %#a:%d (%##s)",
                        q->unansweredQueries, q->qname.c, DNSTypeName(q->qtype), &orig->addr, mDNSVal16(orig->port), orig->domain.c);

            PenalizeDNSServer(m, q, zeroID);
            q->noServerResponse = 1;
        }
        // There are two cases here.
        //
        // 1. We have only one DNS server for this question. It is not responding even after we sent MAX_UCAST_UNANSWERED_QUERIES.
        //    In that case, we need to keep retrying till we get a response. But we need to backoff as we retry. We set
        //    noServerResponse in the block above and below we do not touch the question interval. When we come here, we
        //    already waited for the response. We need to send another query right at this moment. We do that below by
        //    reinitializing dns servers and reissuing the query.
        //
        // 2. We have more than one DNS server. If at least one server did not respond, we would have set noServerResponse
        //    either now (the last server in the list) or before (non-last server in the list). In either case, if we have
        //    reached the end of DNS server list, we need to try again from the beginning. Ideally we should try just the
        //    servers that did not respond, but for simplicity we try all the servers. Once we reached the end of list, we
        //    set triedAllServersOnce so that we don't try all the servers aggressively. See PenalizeDNSServer.
        if (!q->qDNSServer && q->noServerResponse)
        {
            DNSServer *new;
            DNSQuestion *qptr;
            q->triedAllServersOnce = 1;
            // Re-initialize all DNS servers for this question. If we have a DNSServer, DNSServerChangeForQuestion will
            // handle all the work including setting the new DNS server.
            SetValidDNSServers(m, q);
            new = GetServerForQuestion(m, q);
            if (new)
            {
                LogInfo("uDNS_checkCurrentQuestion: Retrying question %p %##s (%s) DNS Server %#a:%d ThisQInterval %d",
                        q, q->qname.c, DNSTypeName(q->qtype), new ? &new->addr : mDNSNULL, mDNSVal16(new ? new->port : zeroIPPort), q->ThisQInterval);
                DNSServerChangeForQuestion(m, q, new);
            }
            for (qptr = q->next ; qptr; qptr = qptr->next)
                if (qptr->DuplicateOf == q) { qptr->validDNSServers = q->validDNSServers; qptr->qDNSServer = q->qDNSServer; }
        }
        if (q->qDNSServer && q->qDNSServer->teststate != DNSServer_Disabled)
        {
            mDNSu8 *end = m->omsg.data;
            mStatus err = mStatus_NoError;
            mDNSBool private = mDNSfalse;

            InitializeDNSMessage(&m->omsg.h, q->TargetQID, (DNSSECQuestion(q) ? DNSSecQFlags : uQueryFlags));

            if (q->qDNSServer->teststate != DNSServer_Untested || NoTestQuery(q))
            {
                end = putQuestion(&m->omsg, m->omsg.data, m->omsg.data + AbsoluteMaxDNSMessageData, &q->qname, q->qtype, q->qclass);
                if (DNSSECQuestion(q) && !q->qDNSServer->cellIntf)
                {
                    if (q->ProxyQuestion)
                        end = DNSProxySetAttributes(q, &m->omsg.h, &m->omsg, end, m->omsg.data + AbsoluteMaxDNSMessageData);
                    else
                        end = putDNSSECOption(&m->omsg, end, m->omsg.data + AbsoluteMaxDNSMessageData);
                }
                private = PrivateQuery(q);
            }
            else if (m->timenow - q->qDNSServer->lasttest >= INIT_UCAST_POLL_INTERVAL)  // Make sure at least three seconds has elapsed since last test query
            {
                LogInfo("Sending DNS test query to %#a:%d", &q->qDNSServer->addr, mDNSVal16(q->qDNSServer->port));
                q->ThisQInterval = INIT_UCAST_POLL_INTERVAL / QuestionIntervalStep;
                q->qDNSServer->lasttest = m->timenow;
                end = putQuestion(&m->omsg, m->omsg.data, m->omsg.data + AbsoluteMaxDNSMessageData, DNSRelayTestQuestion, kDNSType_PTR, kDNSClass_IN);
                q->qDNSServer->testid = m->omsg.h.id;
            }

            if (end > m->omsg.data && (q->qDNSServer->teststate != DNSServer_Failed || NoTestQuery(q)))
            {
                //LogMsg("uDNS_CheckCurrentQuestion %p %d %p %##s (%s)", q, NextQSendTime(q) - m->timenow, private, q->qname.c, DNSTypeName(q->qtype));
                if (private)
                {
                    if (q->nta) CancelGetZoneData(m, q->nta);
                    q->nta = StartGetZoneData(m, &q->qname, q->LongLived ? ZoneServiceLLQ : ZoneServiceQuery, PrivateQueryGotZoneData, q);
                    if (q->state == LLQ_Poll) q->ThisQInterval = (LLQ_POLL_INTERVAL + mDNSRandom(LLQ_POLL_INTERVAL/10)) / QuestionIntervalStep;
                }
                else
                {
                    debugf("uDNS_CheckCurrentQuestion sending %p %##s (%s) %#a:%d UnansweredQueries %d",
                           q, q->qname.c, DNSTypeName(q->qtype),
                           q->qDNSServer ? &q->qDNSServer->addr : mDNSNULL, mDNSVal16(q->qDNSServer ? q->qDNSServer->port : zeroIPPort), q->unansweredQueries);
                    if (!q->LocalSocket)
                    {
                        q->LocalSocket = mDNSPlatformUDPSocket(m, zeroIPPort);
                        if (q->LocalSocket)
                            mDNSPlatformSetDelegatePID(q->LocalSocket, &q->qDNSServer->addr, q);
                    }
                    if (!q->LocalSocket) err = mStatus_NoMemoryErr; // If failed to make socket (should be very rare), we'll try again next time
                    else err = mDNSSendDNSMessage(m, &m->omsg, end, q->qDNSServer->interface, q->LocalSocket, &q->qDNSServer->addr, q->qDNSServer->port, mDNSNULL, mDNSNULL, q->UseBackgroundTrafficClass);
                }
            }

            if (err != mStatus_TransientErr)   // if it is not a transient error backoff and DO NOT flood queries unnecessarily
            {
                q->ThisQInterval = q->ThisQInterval * QuestionIntervalStep; // Only increase interval if send succeeded
                q->unansweredQueries++;
                if (q->ThisQInterval > MAX_UCAST_POLL_INTERVAL)
                    q->ThisQInterval = MAX_UCAST_POLL_INTERVAL;
                if (private && q->state != LLQ_Poll)
                {
                    // We don't want to retransmit too soon. Hence, we always schedule our first
                    // retransmisson at 3 seconds rather than one second
                    if (q->ThisQInterval < (3 * mDNSPlatformOneSecond))
                        q->ThisQInterval = q->ThisQInterval * QuestionIntervalStep;
                    if (q->ThisQInterval > LLQ_POLL_INTERVAL)
                        q->ThisQInterval = LLQ_POLL_INTERVAL;
                    LogInfo("uDNS_CheckCurrentQuestion: private non polling question for %##s (%s) will be retried in %d ms", q->qname.c, DNSTypeName(q->qtype), q->ThisQInterval);
                }
                if (q->qDNSServer->cellIntf)
                {
                    // We don't want to retransmit too soon. Schedule our first retransmisson at
                    // MIN_UCAST_RETRANS_TIMEOUT seconds.
                    if (q->ThisQInterval < MIN_UCAST_RETRANS_TIMEOUT)
                        q->ThisQInterval = MIN_UCAST_RETRANS_TIMEOUT;
                }
                debugf("uDNS_CheckCurrentQuestion: Increased ThisQInterval to %d for %##s (%s), cell %d", q->ThisQInterval, q->qname.c, DNSTypeName(q->qtype), q->qDNSServer->cellIntf);
            }
            q->LastQTime = m->timenow;
            SetNextQueryTime(m, q);
        }
        else
        {
            // If we have no server for this query, or the only server is a disabled one, then we deliver
            // a transient failure indication to the client. This is important for things like iPhone
            // where we want to return timely feedback to the user when no network is available.
            // After calling MakeNegativeCacheRecord() we store the resulting record in the
            // cache so that it will be visible to other clients asking the same question.
            // (When we have a group of identical questions, only the active representative of the group gets
            // passed to uDNS_CheckCurrentQuestion -- we only want one set of query packets hitting the wire --
            // but we want *all* of the questions to get answer callbacks.)
            CacheRecord *rr;
            const mDNSu32 slot = HashSlot(&q->qname);
            CacheGroup *const cg = CacheGroupForName(m, slot, q->qnamehash, &q->qname);

            if (!q->qDNSServer)
            {
                if (!mDNSOpaque64IsZero(&q->validDNSServers))
                    LogMsg("uDNS_CheckCurrentQuestion: ERROR!!: valid DNSServer bits not zero 0x%x, 0x%x for question %##s (%s)",
                           q->validDNSServers.l[1], q->validDNSServers.l[0], q->qname.c, DNSTypeName(q->qtype));
                // If we reached the end of list while picking DNS servers, then we don't want to deactivate the
                // question. Try after 60 seconds. We find this by looking for valid DNSServers for this question,
                // if we find any, then we must have tried them before we came here. This avoids maintaining
                // another state variable to see if we had valid DNS servers for this question.
                SetValidDNSServers(m, q);
                if (mDNSOpaque64IsZero(&q->validDNSServers))
                {
                    LogInfo("uDNS_CheckCurrentQuestion: no DNS server for %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
                    q->ThisQInterval = 0;
                }
                else
                {
                    DNSQuestion *qptr;
                    // Pretend that we sent this question. As this is an ActiveQuestion, the NextScheduledQuery should
                    // be set properly. Also, we need to properly backoff in cases where we don't set the question to
                    // MaxQuestionInterval when we answer the question e.g., LongLived, we need to keep backing off
                    q->ThisQInterval = q->ThisQInterval * QuestionIntervalStep;
                    q->LastQTime = m->timenow;
                    SetNextQueryTime(m, q);
                    // Pick a new DNS server now. Otherwise, when the cache is 80% of its expiry, we will try
                    // to send a query and come back to the same place here and log the above message.
                    q->qDNSServer = GetServerForQuestion(m, q);
                    for (qptr = q->next ; qptr; qptr = qptr->next)
                        if (qptr->DuplicateOf == q) { qptr->validDNSServers = q->validDNSServers; qptr->qDNSServer = q->qDNSServer; }
                    LogInfo("uDNS_checkCurrentQuestion: Tried all DNS servers, retry question %p SuppressUnusable %d %##s (%s) with DNS Server %#a:%d after 60 seconds, ThisQInterval %d",
                            q, q->SuppressUnusable, q->qname.c, DNSTypeName(q->qtype),
                            q->qDNSServer ? &q->qDNSServer->addr : mDNSNULL, mDNSVal16(q->qDNSServer ? q->qDNSServer->port : zeroIPPort), q->ThisQInterval);
                }
            }
            else
            {
                q->ThisQInterval = 0;
                LogMsg("uDNS_CheckCurrentQuestion DNS server %#a:%d for %##s is disabled", &q->qDNSServer->addr, mDNSVal16(q->qDNSServer->port), q->qname.c);
            }

            if (cg)
            {
                for (rr = cg->members; rr; rr=rr->next)
                {
                    if (SameNameRecordAnswersQuestion(&rr->resrec, q))
                    {
                        LogInfo("uDNS_CheckCurrentQuestion: Purged resourcerecord %s", CRDisplayString(m, rr)); 
                        mDNS_PurgeCacheResourceRecord(m, rr);
                    }
                }
            }
            // For some of the WAB queries that we generate form within the mDNSResponder, most of the home routers
            // don't understand and return ServFail/NXDomain. In those cases, we don't want to try too often. We try
            // every fifteen minutes in that case
            MakeNegativeCacheRecord(m, &m->rec.r, &q->qname, q->qnamehash, q->qtype, q->qclass, (DomainEnumQuery(&q->qname) ? 60 * 15 : 60), mDNSInterface_Any, q->qDNSServer);
            q->unansweredQueries = 0;
            if (!mDNSOpaque16IsZero(q->responseFlags))
                m->rec.r.responseFlags = q->responseFlags;
            // We're already using the m->CurrentQuestion pointer, so CacheRecordAdd can't use it to walk the question list.
            // To solve this problem we set rr->DelayDelivery to a nonzero value (which happens to be 'now') so that we
            // momentarily defer generating answer callbacks until mDNS_Execute time.
            CreateNewCacheEntry(m, slot, cg, NonZeroTime(m->timenow), mDNStrue, mDNSNULL);
            ScheduleNextCacheCheckTime(m, slot, NonZeroTime(m->timenow));
            m->rec.r.responseFlags = zeroID;
            m->rec.r.resrec.RecordType = 0;     // Clear RecordType to show we're not still using it
            // MUST NOT touch m->CurrentQuestion (or q) after this -- client callback could have deleted it
        }
    }
}

mDNSexport void CheckNATMappings(mDNS *m)
{
    mStatus err = mStatus_NoError;
    mDNSBool rfc1918 = mDNSv4AddrIsRFC1918(&m->AdvertisedV4.ip.v4);
    mDNSBool HaveRoutable = !rfc1918 && !mDNSIPv4AddressIsZero(m->AdvertisedV4.ip.v4);
    m->NextScheduledNATOp = m->timenow + 0x3FFFFFFF;

    if (HaveRoutable) m->ExtAddress = m->AdvertisedV4.ip.v4;

    if (m->NATTraversals && rfc1918)            // Do we need to open a socket to receive multicast announcements from router?
    {
        if (m->NATMcastRecvskt == mDNSNULL)     // If we are behind a NAT and the socket hasn't been opened yet, open it
        {
            // we need to log a message if we can't get our socket, but only the first time (after success)
            static mDNSBool needLog = mDNStrue;
            m->NATMcastRecvskt = mDNSPlatformUDPSocket(m, NATPMPAnnouncementPort);
            if (!m->NATMcastRecvskt)
            {
                if (needLog)
                {
                    LogMsg("CheckNATMappings: Failed to allocate port 5350 UDP multicast socket for PCP & NAT-PMP announcements");
                    needLog = mDNSfalse;
                }
            }
            else
                needLog = mDNStrue;
        }
    }
    else                                        // else, we don't want to listen for announcements, so close them if they're open
    {
        if (m->NATMcastRecvskt) { mDNSPlatformUDPClose(m->NATMcastRecvskt); m->NATMcastRecvskt = mDNSNULL; }
        if (m->SSDPSocket)      { debugf("CheckNATMappings destroying SSDPSocket %p", &m->SSDPSocket); mDNSPlatformUDPClose(m->SSDPSocket); m->SSDPSocket = mDNSNULL; }
    }

    uDNS_RequestAddress(m);

    if (m->CurrentNATTraversal) LogMsg("WARNING m->CurrentNATTraversal already in use");
    m->CurrentNATTraversal = m->NATTraversals;

    while (m->CurrentNATTraversal)
    {
        NATTraversalInfo *cur = m->CurrentNATTraversal;
        mDNSv4Addr EffectiveAddress = HaveRoutable ? m->AdvertisedV4.ip.v4 : cur->NewAddress;
        m->CurrentNATTraversal = m->CurrentNATTraversal->next;

        if (HaveRoutable)       // If not RFC 1918 address, our own address and port are effectively our external address and port
        {
            cur->ExpiryTime = 0;
            cur->NewResult  = mStatus_NoError;
        }
        else // Check if it's time to send port mapping packet(s)
        {
            if (m->timenow - cur->retryPortMap >= 0) // Time to send a mapping request for this packet
            {
                if (cur->ExpiryTime && cur->ExpiryTime - m->timenow < 0)    // Mapping has expired
                {
                    cur->ExpiryTime    = 0;
                    cur->retryInterval = NATMAP_INIT_RETRY;
                }

                err = uDNS_SendNATMsg(m, cur, mDNStrue); // Will also do UPnP discovery for us, if necessary

                if (cur->ExpiryTime)                        // If have active mapping then set next renewal time halfway to expiry
                    NATSetNextRenewalTime(m, cur);
                else                                        // else no mapping; use exponential backoff sequence
                {
                    if      (cur->retryInterval < NATMAP_INIT_RETRY            ) cur->retryInterval = NATMAP_INIT_RETRY;
                    else if (cur->retryInterval < NATMAP_MAX_RETRY_INTERVAL / 2) cur->retryInterval *= 2;
                    else cur->retryInterval = NATMAP_MAX_RETRY_INTERVAL;
                    cur->retryPortMap = m->timenow + cur->retryInterval;
                }
            }

            if (m->NextScheduledNATOp - cur->retryPortMap > 0)
            {
                m->NextScheduledNATOp = cur->retryPortMap;
            }
        }

        // Notify the client if necessary. We invoke the callback if:
        // (1) We have an effective address,
        //     or we've tried and failed a couple of times to discover it
        // AND
        // (2) the client requested the address only,
        //     or the client won't need a mapping because we have a routable address,
        //     or the client has an expiry time and therefore a successful mapping,
        //     or we've tried and failed a couple of times (see "Time line" below)
        // AND
        // (3) we have new data to give the client that's changed since the last callback
        //
        // Time line is: Send, Wait 500ms, Send, Wait 1sec, Send, Wait 2sec, Send
        // At this point we've sent three requests without an answer, we've just sent our fourth request,
        // retryInterval is now 4 seconds, which is greater than NATMAP_INIT_RETRY * 8 (2 seconds),
        // so we return an error result to the caller.
        if (!mDNSIPv4AddressIsZero(EffectiveAddress) || cur->retryInterval > NATMAP_INIT_RETRY * 8)
        {
            const mStatus EffectiveResult = cur->NewResult ? cur->NewResult : mDNSv4AddrIsRFC1918(&EffectiveAddress) ? mStatus_DoubleNAT : mStatus_NoError;
            const mDNSIPPort ExternalPort = HaveRoutable ? cur->IntPort :
                                            !mDNSIPv4AddressIsZero(EffectiveAddress) && cur->ExpiryTime ? cur->RequestedPort : zeroIPPort;
                                            
            if (!cur->Protocol || HaveRoutable || cur->ExpiryTime || cur->retryInterval > NATMAP_INIT_RETRY * 8)
            {
                if (!mDNSSameIPv4Address(cur->ExternalAddress, EffectiveAddress) ||
                    !mDNSSameIPPort     (cur->ExternalPort,       ExternalPort)    ||
                    cur->Result != EffectiveResult)
                {
                    //LogMsg("NAT callback %d %d %d", cur->Protocol, cur->ExpiryTime, cur->retryInterval);
                    if (cur->Protocol && mDNSIPPortIsZero(ExternalPort) && !mDNSIPv4AddressIsZero(m->Router.ip.v4))
                    {
                        if (!EffectiveResult)
                            LogInfo("CheckNATMapping: Failed to obtain NAT port mapping %p from router %#a external address %.4a internal port %5d interval %d error %d",
                                    cur, &m->Router, &EffectiveAddress, mDNSVal16(cur->IntPort), cur->retryInterval, EffectiveResult);
                        else
                            LogMsg("CheckNATMapping: Failed to obtain NAT port mapping %p from router %#a external address %.4a internal port %5d interval %d error %d",
                                   cur, &m->Router, &EffectiveAddress, mDNSVal16(cur->IntPort), cur->retryInterval, EffectiveResult);
                    }

                    cur->ExternalAddress = EffectiveAddress;
                    cur->ExternalPort    = ExternalPort;
                    cur->Lifetime        = cur->ExpiryTime && !mDNSIPPortIsZero(ExternalPort) ?
                                           (cur->ExpiryTime - m->timenow + mDNSPlatformOneSecond/2) / mDNSPlatformOneSecond : 0;
                    cur->Result          = EffectiveResult;
                    mDNS_DropLockBeforeCallback();      // Allow client to legally make mDNS API calls from the callback
                    if (cur->clientCallback)
                        cur->clientCallback(m, cur);
                    mDNS_ReclaimLockAfterCallback();    // Decrement mDNS_reentrancy to block mDNS API calls again
                    // MUST NOT touch cur after invoking the callback
                }
            }
        }
    }
}

mDNSlocal mDNSs32 CheckRecordUpdates(mDNS *m)
{
    AuthRecord *rr;
    mDNSs32 nextevent = m->timenow + 0x3FFFFFFF;

    CheckGroupRecordUpdates(m);

    for (rr = m->ResourceRecords; rr; rr = rr->next)
    {
        if (!AuthRecord_uDNS(rr)) continue;
        if (rr->state == regState_NoTarget) {debugf("CheckRecordUpdates: Record %##s in NoTarget", rr->resrec.name->c); continue;}
        // While we are waiting for the port mapping, we have nothing to do. The port mapping callback
        // will take care of this
        if (rr->state == regState_NATMap) {debugf("CheckRecordUpdates: Record %##s in NATMap", rr->resrec.name->c); continue;}
        if (rr->state == regState_Pending || rr->state == regState_DeregPending || rr->state == regState_UpdatePending ||
            rr->state == regState_Refresh || rr->state == regState_Registered)
        {
            if (rr->LastAPTime + rr->ThisAPInterval - m->timenow <= 0)
            {
                if (rr->tcp) { DisposeTCPConn(rr->tcp); rr->tcp = mDNSNULL; }
                if (!rr->nta || mDNSIPv4AddressIsZero(rr->nta->Addr.ip.v4))
                {
                    // Zero out the updateid so that if we have a pending response from the server, it won't
                    // be accepted as a valid response. If we accept the response, we might free the new "nta"
                    if (rr->nta) { rr->updateid = zeroID; CancelGetZoneData(m, rr->nta); }
                    rr->nta = StartGetZoneData(m, rr->resrec.name, ZoneServiceUpdate, RecordRegistrationGotZoneData, rr);

                    // We have just started the GetZoneData. We need to wait for it to finish. SetRecordRetry here
                    // schedules the update timer to fire in the future.
                    //
                    // There are three cases.
                    //
                    // 1) When the updates are sent the first time, the first retry is intended to be at three seconds
                    //    in the future. But by calling SetRecordRetry here we set it to nine seconds. But it does not
                    //    matter because when the answer comes back, RecordRegistrationGotZoneData resets the interval
                    //    back to INIT_RECORD_REG_INTERVAL. This also gives enough time for the query.
                    //
                    // 2) In the case of update errors (updateError), this causes further backoff as
                    //    RecordRegistrationGotZoneData does not reset the timer. This is intentional as in the case of
                    //    errors, we don't want to update aggressively.
                    //
                    // 3) We might be refreshing the update. This is very similar to case (1). RecordRegistrationGotZoneData
                    //    resets it back to INIT_RECORD_REG_INTERVAL.
                    //
                    SetRecordRetry(m, rr, 0);
                }
                else if (rr->state == regState_DeregPending) SendRecordDeregistration(m, rr);
                else SendRecordRegistration(m, rr);
            }
        }
        if (nextevent - (rr->LastAPTime + rr->ThisAPInterval) > 0)
            nextevent = (rr->LastAPTime + rr->ThisAPInterval);
    }
    return nextevent;
}

mDNSexport void uDNS_Tasks(mDNS *const m)
{
    mDNSs32 nexte;
    DNSServer *d;

    m->NextuDNSEvent = m->timenow + 0x3FFFFFFF;

    nexte = CheckRecordUpdates(m);
    if (m->NextuDNSEvent - nexte > 0)
        m->NextuDNSEvent = nexte;

    for (d = m->DNSServers; d; d=d->next)
        if (d->penaltyTime)
        {
            if (m->timenow - d->penaltyTime >= 0)
            {
                LogInfo("DNS server %#a:%d out of penalty box", &d->addr, mDNSVal16(d->port));
                d->penaltyTime = 0;
            }
            else
            if (m->NextuDNSEvent - d->penaltyTime > 0)
                m->NextuDNSEvent = d->penaltyTime;
        }

    if (m->CurrentQuestion)
        LogMsg("uDNS_Tasks ERROR m->CurrentQuestion already set: %##s (%s)", m->CurrentQuestion->qname.c, DNSTypeName(m->CurrentQuestion->qtype));
    m->CurrentQuestion = m->Questions;
    while (m->CurrentQuestion && m->CurrentQuestion != m->NewQuestions)
    {
        DNSQuestion *const q = m->CurrentQuestion;
        if (ActiveQuestion(q) && !mDNSOpaque16IsZero(q->TargetQID))
        {
            uDNS_CheckCurrentQuestion(m);
            if (q == m->CurrentQuestion)
                if (m->NextuDNSEvent - NextQSendTime(q) > 0)
                    m->NextuDNSEvent = NextQSendTime(q);
        }
        // If m->CurrentQuestion wasn't modified out from under us, advance it now
        // We can't do this at the start of the loop because uDNS_CheckCurrentQuestion()
        // depends on having m->CurrentQuestion point to the right question
        if (m->CurrentQuestion == q)
            m->CurrentQuestion = q->next;
    }
    m->CurrentQuestion = mDNSNULL;
}

// ***************************************************************************
#if COMPILER_LIKES_PRAGMA_MARK
#pragma mark - Startup, Shutdown, and Sleep
#endif

mDNSexport void SleepRecordRegistrations(mDNS *m)
{
    AuthRecord *rr;
    for (rr = m->ResourceRecords; rr; rr=rr->next)
    {
        if (AuthRecord_uDNS(rr))
        {
            // Zero out the updateid so that if we have a pending response from the server, it won't
            // be accepted as a valid response.
            if (rr->nta) { rr->updateid = zeroID; CancelGetZoneData(m, rr->nta); rr->nta = mDNSNULL; }

            if (rr->NATinfo.clientContext)
            {
                mDNS_StopNATOperation_internal(m, &rr->NATinfo);
                rr->NATinfo.clientContext = mDNSNULL;
            }
            // We are waiting to update the resource record. The original data of the record is
            // in OrigRData and the updated value is in InFlightRData. Free the old and the new
            // one will be registered when we come back.
            if (rr->state == regState_UpdatePending)
            {
                // act as if the update succeeded, since we're about to delete the name anyway
                rr->state = regState_Registered;
                // deallocate old RData
                if (rr->UpdateCallback) rr->UpdateCallback(m, rr, rr->OrigRData, rr->OrigRDLen);
                SetNewRData(&rr->resrec, rr->InFlightRData, rr->InFlightRDLen);
                rr->OrigRData = mDNSNULL;
                rr->InFlightRData = mDNSNULL;
            }

            // If we have not begun the registration process i.e., never sent a registration packet,
            // then uDNS_DeregisterRecord will not send a deregistration
            uDNS_DeregisterRecord(m, rr);

            // When we wake, we call ActivateUnicastRegistration which starts at StartGetZoneData
        }
    }
}

mDNSexport void mDNS_AddSearchDomain(const domainname *const domain, mDNSInterfaceID InterfaceID)
{
    SearchListElem **p;
    SearchListElem *tmp = mDNSNULL;

    // Check to see if we already have this domain in our list
    for (p = &SearchList; *p; p = &(*p)->next)
        if (((*p)->InterfaceID == InterfaceID) && SameDomainName(&(*p)->domain, domain))
        {
            // If domain is already in list, and marked for deletion, unmark the delete
            // Be careful not to touch the other flags that may be present
            LogInfo("mDNS_AddSearchDomain already in list %##s", domain->c);
            if ((*p)->flag & SLE_DELETE) (*p)->flag &= ~SLE_DELETE;
            tmp = *p;
            *p = tmp->next;
            tmp->next = mDNSNULL;
            break;
        }


    // move to end of list so that we maintain the same order
    while (*p) p = &(*p)->next;

    if (tmp) *p = tmp;
    else
    {
        // if domain not in list, add to list, mark as add (1)
        *p = mDNSPlatformMemAllocate(sizeof(SearchListElem));
        if (!*p) { LogMsg("ERROR: mDNS_AddSearchDomain - malloc"); return; }
        mDNSPlatformMemZero(*p, sizeof(SearchListElem));
        AssignDomainName(&(*p)->domain, domain);
        (*p)->next = mDNSNULL;
        (*p)->InterfaceID = InterfaceID;
        LogInfo("mDNS_AddSearchDomain created new %##s, InterfaceID %p", domain->c, InterfaceID);
    }
}

mDNSlocal void FreeARElemCallback(mDNS *const m, AuthRecord *const rr, mStatus result)
{
    (void)m;    // unused
    if (result == mStatus_MemFree) mDNSPlatformMemFree(rr->RecordContext);
}

mDNSlocal void FoundDomain(mDNS *const m, DNSQuestion *question, const ResourceRecord *const answer, QC_result AddRecord)
{
    SearchListElem *slElem = question->QuestionContext;
    mStatus err;
    const char *name;

    if (answer->rrtype != kDNSType_PTR) return;
    if (answer->RecordType == kDNSRecordTypePacketNegative) return;
    if (answer->InterfaceID == mDNSInterface_LocalOnly) return;

    if      (question == &slElem->BrowseQ) name = mDNS_DomainTypeNames[mDNS_DomainTypeBrowse];
    else if (question == &slElem->DefBrowseQ) name = mDNS_DomainTypeNames[mDNS_DomainTypeBrowseDefault];
    else if (question == &slElem->AutomaticBrowseQ) name = mDNS_DomainTypeNames[mDNS_DomainTypeBrowseAutomatic];
    else if (question == &slElem->RegisterQ) name = mDNS_DomainTypeNames[mDNS_DomainTypeRegistration];
    else if (question == &slElem->DefRegisterQ) name = mDNS_DomainTypeNames[mDNS_DomainTypeRegistrationDefault];
    else { LogMsg("FoundDomain - unknown question"); return; }

    LogInfo("FoundDomain: %p %s %s Q %##s A %s", answer->InterfaceID, AddRecord ? "Add" : "Rmv", name, question->qname.c, RRDisplayString(m, answer));

    if (AddRecord)
    {
        ARListElem *arElem = mDNSPlatformMemAllocate(sizeof(ARListElem));
        if (!arElem) { LogMsg("ERROR: FoundDomain out of memory"); return; }
        mDNS_SetupResourceRecord(&arElem->ar, mDNSNULL, mDNSInterface_LocalOnly, kDNSType_PTR, 7200, kDNSRecordTypeShared, AuthRecordLocalOnly, FreeARElemCallback, arElem);
        MakeDomainNameFromDNSNameString(&arElem->ar.namestorage, name);
        AppendDNSNameString            (&arElem->ar.namestorage, "local");
        AssignDomainName(&arElem->ar.resrec.rdata->u.name, &answer->rdata->u.name);
        LogInfo("FoundDomain: Registering %s", ARDisplayString(m, &arElem->ar));
        err = mDNS_Register(m, &arElem->ar);
        if (err) { LogMsg("ERROR: FoundDomain - mDNS_Register returned %d", err); mDNSPlatformMemFree(arElem); return; }
        arElem->next = slElem->AuthRecs;
        slElem->AuthRecs = arElem;
    }
    else
    {
        ARListElem **ptr = &slElem->AuthRecs;
        while (*ptr)
        {
            if (SameDomainName(&(*ptr)->ar.resrec.rdata->u.name, &answer->rdata->u.name))
            {
                ARListElem *dereg = *ptr;
                *ptr = (*ptr)->next;
                LogInfo("FoundDomain: Deregistering %s", ARDisplayString(m, &dereg->ar));
                err = mDNS_Deregister(m, &dereg->ar);
                if (err) LogMsg("ERROR: FoundDomain - mDNS_Deregister returned %d", err);
                // Memory will be freed in the FreeARElemCallback
            }
            else
                ptr = &(*ptr)->next;
        }
    }
}

#if APPLE_OSX_mDNSResponder && MACOSX_MDNS_MALLOC_DEBUGGING
mDNSexport void udns_validatelists(void *const v)
{
    mDNS *const m = v;

    NATTraversalInfo *n;
    for (n = m->NATTraversals; n; n=n->next)
        if (n->next == (NATTraversalInfo *)~0 || n->clientCallback == (NATTraversalClientCallback) ~0)
            LogMemCorruption("m->NATTraversals: %p is garbage", n);

    DNSServer *d;
    for (d = m->DNSServers; d; d=d->next)
        if (d->next == (DNSServer *)~0 || d->teststate > DNSServer_Disabled)
            LogMemCorruption("m->DNSServers: %p is garbage (%d)", d, d->teststate);

    DomainAuthInfo *info;
    for (info = m->AuthInfoList; info; info = info->next)
        if (info->next == (DomainAuthInfo *)~0)
            LogMemCorruption("m->AuthInfoList: %p is garbage", info);

    HostnameInfo *hi;
    for (hi = m->Hostnames; hi; hi = hi->next)
        if (hi->next == (HostnameInfo *)~0 || hi->StatusCallback == (mDNSRecordCallback*)~0)
            LogMemCorruption("m->Hostnames: %p is garbage", n);

    SearchListElem *ptr;
    for (ptr = SearchList; ptr; ptr = ptr->next)
        if (ptr->next == (SearchListElem *)~0 || ptr->AuthRecs == (void*)~0)
            LogMemCorruption("SearchList: %p is garbage (%X)", ptr, ptr->AuthRecs);
}
#endif

// This should probably move to the UDS daemon -- the concept of legacy clients and automatic registration / automatic browsing
// is really a UDS API issue, not something intrinsic to uDNS
#ifndef REMOVE_BROWSE_REQUEST
mDNSlocal void uDNS_DeleteWABQueries(mDNS *const m, SearchListElem *ptr, int delete)
{
    const char *name1 = mDNSNULL;
    const char *name2 = mDNSNULL;
    ARListElem **arList = &ptr->AuthRecs;
    domainname namestorage1, namestorage2;
    mStatus err;

    // "delete" parameter indicates the type of query.
    switch (delete)
    {
    case UDNS_WAB_BROWSE_QUERY:
        mDNS_StopGetDomains(m, &ptr->BrowseQ);
        mDNS_StopGetDomains(m, &ptr->DefBrowseQ);
        name1 = mDNS_DomainTypeNames[mDNS_DomainTypeBrowse];
        name2 = mDNS_DomainTypeNames[mDNS_DomainTypeBrowseDefault];
        break;
    case UDNS_WAB_LBROWSE_QUERY:
        mDNS_StopGetDomains(m, &ptr->AutomaticBrowseQ);
        name1 = mDNS_DomainTypeNames[mDNS_DomainTypeBrowseAutomatic];
        break;
    case UDNS_WAB_REG_QUERY:
        mDNS_StopGetDomains(m, &ptr->RegisterQ);
        mDNS_StopGetDomains(m, &ptr->DefRegisterQ);
        name1 = mDNS_DomainTypeNames[mDNS_DomainTypeRegistration];
        name2 = mDNS_DomainTypeNames[mDNS_DomainTypeRegistrationDefault];
        break;
    default:
        LogMsg("uDNS_DeleteWABQueries: ERROR!! returning from default");
        return;
    }
    // When we get the results to the domain enumeration queries, we add a LocalOnly
    // entry. For example, if we issue a domain enumeration query for b._dns-sd._udp.xxxx.com,
    // and when we get a response, we add a LocalOnly entry b._dns-sd._udp.local whose RDATA
    // points to what we got in the response. Locate the appropriate LocalOnly entries and delete
    // them.
    if (name1)
    {
        MakeDomainNameFromDNSNameString(&namestorage1, name1);
        AppendDNSNameString(&namestorage1, "local");
    }
    if (name2)
    {
        MakeDomainNameFromDNSNameString(&namestorage2, name2);
        AppendDNSNameString(&namestorage2, "local");
    }
    while (*arList)
    {
        ARListElem *dereg = *arList;
        if ((name1 && SameDomainName(&dereg->ar.namestorage, &namestorage1)) ||
            (name2 && SameDomainName(&dereg->ar.namestorage, &namestorage2)))
        {
            LogInfo("uDNS_DeleteWABQueries: Deregistering PTR %##s -> %##s", dereg->ar.resrec.name->c, dereg->ar.resrec.rdata->u.name.c);
            *arList = dereg->next;
            err = mDNS_Deregister(m, &dereg->ar);
            if (err) LogMsg("uDNS_DeleteWABQueries:: ERROR!! mDNS_Deregister returned %d", err);
            // Memory will be freed in the FreeARElemCallback
        }
        else
        {
            LogInfo("uDNS_DeleteWABQueries: Skipping PTR %##s -> %##s", dereg->ar.resrec.name->c, dereg->ar.resrec.rdata->u.name.c);
            arList = &(*arList)->next;
        }
    }
}

mDNSexport void uDNS_SetupWABQueries(mDNS *const m)
{
    SearchListElem **p = &SearchList, *ptr;
    mStatus err;
    int action = 0;

    // step 1: mark each element for removal
    for (ptr = SearchList; ptr; ptr = ptr->next)
        ptr->flag |= SLE_DELETE;

    // Make sure we have the search domains from the platform layer so that if we start the WAB
    // queries below, we have the latest information.
    mDNS_Lock(m);
    if (!mDNSPlatformSetDNSConfig(m, mDNSfalse, mDNStrue, mDNSNULL, mDNSNULL, mDNSNULL, mDNSfalse))
    {
        // If the configuration did not change, clear the flag so that we don't free the searchlist.
        // We still have to start the domain enumeration queries as we may not have started them
        // before.
        for (ptr = SearchList; ptr; ptr = ptr->next)
            ptr->flag &= ~SLE_DELETE;
        LogInfo("uDNS_SetupWABQueries: No config change");
    }
    mDNS_Unlock(m);

    if (m->WABBrowseQueriesCount)
        action |= UDNS_WAB_BROWSE_QUERY;
    if (m->WABLBrowseQueriesCount)
        action |= UDNS_WAB_LBROWSE_QUERY;
    if (m->WABRegQueriesCount)
        action |= UDNS_WAB_REG_QUERY;


    // delete elems marked for removal, do queries for elems marked add
    while (*p)
    {
        ptr = *p;
        LogInfo("uDNS_SetupWABQueries:action 0x%x: Flags 0x%x,  AuthRecs %p, InterfaceID %p %##s", action, ptr->flag, ptr->AuthRecs, ptr->InterfaceID, ptr->domain.c);
        // If SLE_DELETE is set, stop all the queries, deregister all the records and free the memory.
        // Otherwise, check to see what the "action" requires. If a particular action bit is not set and
        // we have started the corresponding queries as indicated by the "flags", stop those queries and
        // deregister the records corresponding to them.
        if ((ptr->flag & SLE_DELETE) ||
            (!(action & UDNS_WAB_BROWSE_QUERY) && (ptr->flag & SLE_WAB_BROWSE_QUERY_STARTED)) ||
            (!(action & UDNS_WAB_LBROWSE_QUERY) && (ptr->flag & SLE_WAB_LBROWSE_QUERY_STARTED)) ||
            (!(action & UDNS_WAB_REG_QUERY) && (ptr->flag & SLE_WAB_REG_QUERY_STARTED)))
        {
            if (ptr->flag & SLE_DELETE)
            {
                ARListElem *arList = ptr->AuthRecs;
                ptr->AuthRecs = mDNSNULL;
                *p = ptr->next;

                // If the user has "local" in their DNS searchlist, we ignore that for the purposes of domain enumeration queries
                // We suppressed the domain enumeration for scoped search domains below. When we enable that
                // enable this.
                if ((ptr->flag & SLE_WAB_BROWSE_QUERY_STARTED) &&
                    !SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
                {
                    LogInfo("uDNS_SetupWABQueries: DELETE  Browse for domain  %##s", ptr->domain.c);
                    mDNS_StopGetDomains(m, &ptr->BrowseQ);
                    mDNS_StopGetDomains(m, &ptr->DefBrowseQ);
                }
                if ((ptr->flag & SLE_WAB_LBROWSE_QUERY_STARTED) &&
                    !SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
                {
                    LogInfo("uDNS_SetupWABQueries: DELETE  Legacy Browse for domain  %##s", ptr->domain.c);
                    mDNS_StopGetDomains(m, &ptr->AutomaticBrowseQ);
                }
                if ((ptr->flag & SLE_WAB_REG_QUERY_STARTED) &&
                    !SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
                {
                    LogInfo("uDNS_SetupWABQueries: DELETE  Registration for domain  %##s", ptr->domain.c);
                    mDNS_StopGetDomains(m, &ptr->RegisterQ);
                    mDNS_StopGetDomains(m, &ptr->DefRegisterQ);
                }

                mDNSPlatformMemFree(ptr);

                // deregister records generated from answers to the query
                while (arList)
                {
                    ARListElem *dereg = arList;
                    arList = arList->next;
                    LogInfo("uDNS_SetupWABQueries: DELETE Deregistering PTR %##s -> %##s", dereg->ar.resrec.name->c, dereg->ar.resrec.rdata->u.name.c);
                    err = mDNS_Deregister(m, &dereg->ar);
                    if (err) LogMsg("uDNS_SetupWABQueries:: ERROR!! mDNS_Deregister returned %d", err);
                    // Memory will be freed in the FreeARElemCallback
                }
                continue;
            }

            // If the user has "local" in their DNS searchlist, we ignore that for the purposes of domain enumeration queries
            // We suppressed the domain enumeration for scoped search domains below. When we enable that
            // enable this.
            if (!(action & UDNS_WAB_BROWSE_QUERY) && (ptr->flag & SLE_WAB_BROWSE_QUERY_STARTED) &&
                !SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
            {
                LogInfo("uDNS_SetupWABQueries: Deleting Browse for domain  %##s", ptr->domain.c);
                ptr->flag &= ~SLE_WAB_BROWSE_QUERY_STARTED;
                uDNS_DeleteWABQueries(m, ptr, UDNS_WAB_BROWSE_QUERY);
            }

            if (!(action & UDNS_WAB_LBROWSE_QUERY) && (ptr->flag & SLE_WAB_LBROWSE_QUERY_STARTED) &&
                !SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
            {
                LogInfo("uDNS_SetupWABQueries: Deleting Legacy Browse for domain  %##s", ptr->domain.c);
                ptr->flag &= ~SLE_WAB_LBROWSE_QUERY_STARTED;
                uDNS_DeleteWABQueries(m, ptr, UDNS_WAB_LBROWSE_QUERY);
            }

            if (!(action & UDNS_WAB_REG_QUERY) && (ptr->flag & SLE_WAB_REG_QUERY_STARTED) &&
                !SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
            {
                LogInfo("uDNS_SetupWABQueries: Deleting Registration for domain  %##s", ptr->domain.c);
                ptr->flag &= ~SLE_WAB_REG_QUERY_STARTED;
                uDNS_DeleteWABQueries(m, ptr, UDNS_WAB_REG_QUERY);
            }

            // Fall through to handle the ADDs
        }

        if ((action & UDNS_WAB_BROWSE_QUERY) && !(ptr->flag & SLE_WAB_BROWSE_QUERY_STARTED))
        {
            // If the user has "local" in their DNS searchlist, we ignore that for the purposes of domain enumeration queries.
            // Also, suppress the domain enumeration for scoped search domains for now until there is a need.
            if (!SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
            {
                mStatus err1, err2;
                err1 = mDNS_GetDomains(m, &ptr->BrowseQ,          mDNS_DomainTypeBrowse,              &ptr->domain, ptr->InterfaceID, FoundDomain, ptr);
                if (err1)
                {
                    LogMsg("uDNS_SetupWABQueries: GetDomains for domain %##s returned error(s):\n"
                           "%d (mDNS_DomainTypeBrowse)\n", ptr->domain.c, err1);
                }
                else
                {
                    LogInfo("uDNS_SetupWABQueries: Starting Browse for domain %##s", ptr->domain.c);
                }
                err2 = mDNS_GetDomains(m, &ptr->DefBrowseQ,       mDNS_DomainTypeBrowseDefault,       &ptr->domain, ptr->InterfaceID, FoundDomain, ptr);
                if (err2)
                {
                    LogMsg("uDNS_SetupWABQueries: GetDomains for domain %##s returned error(s):\n"
                           "%d (mDNS_DomainTypeBrowseDefault)\n", ptr->domain.c, err2);
                }
                else
                {
                    LogInfo("uDNS_SetupWABQueries: Starting Default Browse for domain %##s", ptr->domain.c);
                }
                // For simplicity, we mark a single bit for denoting that both the browse queries have started.
                // It is not clear as to why one would fail to start and the other would succeed in starting up.
                // If that happens, we will try to stop both the queries and one of them won't be in the list and
                // it is not a hard error.
                if (!err1 || !err2)
                {
                    ptr->flag |= SLE_WAB_BROWSE_QUERY_STARTED;
                }
            }
        }
        if ((action & UDNS_WAB_LBROWSE_QUERY) && !(ptr->flag & SLE_WAB_LBROWSE_QUERY_STARTED))
        {
            // If the user has "local" in their DNS searchlist, we ignore that for the purposes of domain enumeration queries.
            // Also, suppress the domain enumeration for scoped search domains for now until there is a need.
            if (!SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
            {
                mStatus err1;
                err1 = mDNS_GetDomains(m, &ptr->AutomaticBrowseQ, mDNS_DomainTypeBrowseAutomatic,     &ptr->domain, ptr->InterfaceID, FoundDomain, ptr);
                if (err1)
                {
                    LogMsg("uDNS_SetupWABQueries: GetDomains for domain %##s returned error(s):\n"
                           "%d (mDNS_DomainTypeBrowseAutomatic)\n",
                           ptr->domain.c, err1);
                }
                else
                {
                    ptr->flag |= SLE_WAB_LBROWSE_QUERY_STARTED;
                    LogInfo("uDNS_SetupWABQueries: Starting Legacy Browse for domain %##s", ptr->domain.c);
                }
            }
        }
        if ((action & UDNS_WAB_REG_QUERY) && !(ptr->flag & SLE_WAB_REG_QUERY_STARTED))
        {
            // If the user has "local" in their DNS searchlist, we ignore that for the purposes of domain enumeration queries.
            // Also, suppress the domain enumeration for scoped search domains for now until there is a need.
            if (!SameDomainName(&ptr->domain, &localdomain) && (ptr->InterfaceID == mDNSInterface_Any))
            {
                mStatus err1, err2;
                err1 = mDNS_GetDomains(m, &ptr->RegisterQ,        mDNS_DomainTypeRegistration,        &ptr->domain, ptr->InterfaceID, FoundDomain, ptr);
                if (err1)
                {
                    LogMsg("uDNS_SetupWABQueries: GetDomains for domain %##s returned error(s):\n"
                           "%d (mDNS_DomainTypeRegistration)\n", ptr->domain.c, err1);
                }
                else
                {
                    LogInfo("uDNS_SetupWABQueries: Starting Registration for domain %##s", ptr->domain.c);
                }
                err2 = mDNS_GetDomains(m, &ptr->DefRegisterQ,     mDNS_DomainTypeRegistrationDefault, &ptr->domain, ptr->InterfaceID, FoundDomain, ptr);
                if (err2)
                {
                    LogMsg("uDNS_SetupWABQueries: GetDomains for domain %##s returned error(s):\n"
                           "%d (mDNS_DomainTypeRegistrationDefault)", ptr->domain.c, err2);
                }
                else
                {
                    LogInfo("uDNS_SetupWABQueries: Starting Default Registration for domain %##s", ptr->domain.c);
                }
                if (!err1 || !err2)
                {
                    ptr->flag |= SLE_WAB_REG_QUERY_STARTED;
                }
            }
        }

        p = &ptr->next;
    }
}

// mDNS_StartWABQueries is called once per API invocation where normally
// one of the bits is set.
mDNSexport void uDNS_StartWABQueries(mDNS *const m, int queryType)
{
    if (queryType & UDNS_WAB_BROWSE_QUERY)
    {
        m->WABBrowseQueriesCount++;
        LogInfo("uDNS_StartWABQueries: Browse query count %d", m->WABBrowseQueriesCount);
    }
    if (queryType & UDNS_WAB_LBROWSE_QUERY)
    {
        m->WABLBrowseQueriesCount++;
        LogInfo("uDNS_StartWABQueries: Legacy Browse query count %d", m->WABLBrowseQueriesCount);
    }
    if (queryType & UDNS_WAB_REG_QUERY)
    {
        m->WABRegQueriesCount++;
        LogInfo("uDNS_StartWABQueries: Reg query count %d", m->WABRegQueriesCount);
    }
    uDNS_SetupWABQueries(m);
}

// mDNS_StopWABQueries is called once per API invocation where normally
// one of the bits is set.
mDNSexport void uDNS_StopWABQueries(mDNS *const m, int queryType)
{
    if (queryType & UDNS_WAB_BROWSE_QUERY)
    {
        m->WABBrowseQueriesCount--;
        LogInfo("uDNS_StopWABQueries: Browse query count %d", m->WABBrowseQueriesCount);
    }
    if (queryType & UDNS_WAB_LBROWSE_QUERY)
    {
        m->WABLBrowseQueriesCount--;
        LogInfo("uDNS_StopWABQueries: Legacy Browse query count %d", m->WABLBrowseQueriesCount);
    }
    if (queryType & UDNS_WAB_REG_QUERY)
    {
        m->WABRegQueriesCount--;
        LogInfo("uDNS_StopWABQueries: Reg query count %d", m->WABRegQueriesCount);
    }
    uDNS_SetupWABQueries(m);
}
#endif //#ifndef REMOVE_BROWSE_REQUEST
mDNSexport domainname  *uDNS_GetNextSearchDomain(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSs8 *searchIndex, mDNSBool ignoreDotLocal)
{
    SearchListElem *p = SearchList;
    int count = *searchIndex;
    (void) m; // unused

    if (count < 0) { LogMsg("uDNS_GetNextSearchDomain: count %d less than zero", count); return mDNSNULL; }

    // Skip the  domains that we already looked at before. Guard against "p"
    // being NULL. When search domains change we may not set the SearchListIndex
    // of the question to zero immediately e.g., domain enumeration query calls
    // uDNS_SetupWABQueries which reads in the new search domain but does not
    // restart the questions immediately. Questions are restarted as part of
    // network change and hence temporarily SearchListIndex may be out of range.

    for (; count && p; count--)
        p = p->next;

    while (p)
    {
        int labels = CountLabels(&p->domain);
        if (labels > 0)
        {
            const domainname *d = SkipLeadingLabels(&p->domain, labels - 1);
            if (SameDomainLabel(d->c, (const mDNSu8 *)"\x4" "arpa"))
            {
                LogInfo("uDNS_GetNextSearchDomain: skipping search domain %##s, InterfaceID %p", p->domain.c, p->InterfaceID);
                (*searchIndex)++;
                p = p->next;
                continue;
            }
            if (ignoreDotLocal && SameDomainLabel(d->c, (const mDNSu8 *)"\x5" "local"))
            {
                LogInfo("uDNS_GetNextSearchDomain: skipping local domain %##s, InterfaceID %p", p->domain.c, p->InterfaceID);
                (*searchIndex)++;
                p = p->next;
                continue;
            }
        }
        // Point to the next one in the list which we will look at next time.
        (*searchIndex)++;
        // When we are appending search domains in a ActiveDirectory domain, the question's InterfaceID
        // set to mDNSInterface_Unicast. Match the unscoped entries in that case.
        if (((InterfaceID == mDNSInterface_Unicast) && (p->InterfaceID == mDNSInterface_Any)) ||
            p->InterfaceID == InterfaceID)
        {
            LogInfo("uDNS_GetNextSearchDomain returning domain %##s, InterfaceID %p", p->domain.c, p->InterfaceID);
            return &p->domain;
        }
        LogInfo("uDNS_GetNextSearchDomain skipping domain %##s, InterfaceID %p", p->domain.c, p->InterfaceID);
        p = p->next;
    }
    return mDNSNULL;
}

mDNSlocal void FlushAddressCacheRecords(mDNS *const m)
{
    mDNSu32 slot;
    CacheGroup *cg;
    CacheRecord *cr;
    FORALL_CACHERECORDS(slot, cg, cr)
    {
        if (cr->resrec.InterfaceID) continue;

        // If a resource record can answer A or AAAA, they need to be flushed so that we will
        // deliver an ADD or RMV
        if (RRTypeAnswersQuestionType(&cr->resrec, kDNSType_A) ||
            RRTypeAnswersQuestionType(&cr->resrec, kDNSType_AAAA))
        {
            LogInfo("FlushAddressCacheRecords: Purging Resourcerecord %s", CRDisplayString(m, cr));
            mDNS_PurgeCacheResourceRecord(m, cr);
        }
    }
}

// Retry questions which has seach domains appended
mDNSexport void RetrySearchDomainQuestions(mDNS *const m)
{
    DNSQuestion *q;
    mDNSBool found = mDNSfalse;

    // Check to see if there are any questions which needs search domains to be applied.
    // If there is none, search domains can't possibly affect them.
    for (q = m->Questions; q; q = q->next)
    {
        if (q->AppendSearchDomains)
        {
            found = mDNStrue;
            break;
        }
    }
    if (!found)
    {
        LogInfo("RetrySearchDomainQuestions: Questions with AppendSearchDomain not found");
        return;
    }
    LogInfo("RetrySearchDomainQuestions: Question with AppendSearchDomain found %##s (%s)", q->qname.c, DNSTypeName(q->qtype));
    // Purge all the A/AAAA cache records and restart the queries. mDNSCoreRestartAddressQueries
    // does this. When we restart the question,  we first want to try the new search domains rather
    // than use the entries that is already in the cache. When we appended search domains, we might
    // have created cache entries which is no longer valid as there are new search domains now
    mDNSCoreRestartAddressQueries(m, mDNStrue, FlushAddressCacheRecords, mDNSNULL, mDNSNULL);
}

// Construction of Default Browse domain list (i.e. when clients pass NULL) is as follows:
// 1) query for b._dns-sd._udp.local on LocalOnly interface
//    (.local manually generated via explicit callback)
// 2) for each search domain (from prefs pane), query for b._dns-sd._udp.<searchdomain>.
// 3) for each result from (2), register LocalOnly PTR record b._dns-sd._udp.local. -> <result>
// 4) result above should generate a callback from question in (1).  result added to global list
// 5) global list delivered to client via GetSearchDomainList()
// 6) client calls to enumerate domains now go over LocalOnly interface
//    (!!!KRS may add outgoing interface in addition)

struct CompileTimeAssertionChecks_uDNS
{
    // Check our structures are reasonable sizes. Including overly-large buffers, or embedding
    // other overly-large structures instead of having a pointer to them, can inadvertently
    // cause structure sizes (and therefore memory usage) to balloon unreasonably.
    char sizecheck_tcpInfo_t     [(sizeof(tcpInfo_t)      <=  9056) ? 1 : -1];
    char sizecheck_SearchListElem[(sizeof(SearchListElem) <=  5000) ? 1 : -1];
};

#else // !UNICAST_DISABLED

mDNSexport const domainname *GetServiceTarget(mDNS *m, AuthRecord *const rr)
{
	(void) m;
	(void) rr;

	return mDNSNULL;
}

mDNSexport DomainAuthInfo *GetAuthInfoForName_internal(mDNS *m, const domainname *const name)
{
	(void) m;
	(void) name;

	return mDNSNULL;
}

mDNSexport DomainAuthInfo *GetAuthInfoForQuestion(mDNS *m, const DNSQuestion *const q)
{
	(void) m;
	(void) q;

	return mDNSNULL;
}

mDNSexport void startLLQHandshake(mDNS *m, DNSQuestion *q)
{
	(void) m;
	(void) q;
}

mDNSexport void DisposeTCPConn(struct tcpInfo_t *tcp)
{
	(void) tcp;
}

#ifndef REMOVE_NAT_FUCTIONALITY
mDNSexport mStatus mDNS_StartNATOperation_internal(mDNS *m, NATTraversalInfo *traversal)
{
	(void) m;
	(void) traversal;

	return mStatus_UnsupportedErr;
}

mDNSexport mStatus mDNS_StopNATOperation_internal(mDNS *m, NATTraversalInfo *traversal)
{
	(void) m;
	(void) traversal;

	return mStatus_UnsupportedErr;
}
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
mDNSexport void sendLLQRefresh(mDNS *m, DNSQuestion *q)
{
	(void) m;
	(void) q;
}

mDNSexport ZoneData *StartGetZoneData(mDNS *const m, const domainname *const name, const ZoneService target, ZoneDataCallback callback, void *ZoneDataContext)
{
	(void) m;
	(void) name;
	(void) target;
	(void) callback;
	(void) ZoneDataContext;

	return mDNSNULL;
}

mDNSexport void RecordRegistrationGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneData)
{
	(void) m;
	(void) err;
	(void) zoneData;
}

mDNSexport uDNS_LLQType uDNS_recvLLQResponse(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end,
                                             const mDNSAddr *const srcaddr, const mDNSIPPort srcport, DNSQuestion **matchQuestion)
{
	(void) m;
	(void) msg;
	(void) end;
	(void) srcaddr;
	(void) srcport;
	(void) matchQuestion;

	return uDNS_LLQ_Not;
}

mDNSexport void PenalizeDNSServer(mDNS *const m, DNSQuestion *q, mDNSOpaque16 responseFlags)
{
	(void) m;
	(void) q;
	(void) responseFlags;
}

mDNSexport void mDNS_AddSearchDomain(const domainname *const domain, mDNSInterfaceID InterfaceID)
{
    (void) domain;
    (void) InterfaceID;
}

mDNSexport void RetrySearchDomainQuestions(mDNS *const m)
{
    (void) m;
}

mDNSexport mStatus mDNS_SetSecretForDomain(mDNS *m, DomainAuthInfo *info, const domainname *domain, const domainname *keyname, const char *b64keydata, const domainname *hostname, mDNSIPPort *port, mDNSBool autoTunnel)
{
    (void) m;
    (void) info;
    (void) domain;
    (void) keyname;
    (void) b64keydata;
    (void) hostname;
    (void) port;
    (void) autoTunnel;
    
    return mStatus_UnsupportedErr;
}

mDNSexport domainname  *uDNS_GetNextSearchDomain(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSs8 *searchIndex, mDNSBool ignoreDotLocal)
{
    (void) m;
    (void) InterfaceID;
    (void) searchIndex;
    (void) ignoreDotLocal;
    
    return mDNSNULL;
}

mDNSexport DomainAuthInfo *GetAuthInfoForName(mDNS *m, const domainname *const name)
{
    (void) m;
    (void) name;
    
    return mDNSNULL;
}
#ifndef REMOVE_NAT_FUCTIONALITY
mDNSexport mStatus mDNS_StartNATOperation(mDNS *const m, NATTraversalInfo *traversal)
{
    (void) m;
    (void) traversal;
    
    return mStatus_UnsupportedErr;
}

mDNSexport mStatus mDNS_StopNATOperation(mDNS *const m, NATTraversalInfo *traversal)
{
    (void) m;
    (void) traversal;
    
    return mStatus_UnsupportedErr;
}
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
mDNSexport DNSServer *mDNS_AddDNSServer(mDNS *const m, const domainname *d, const mDNSInterfaceID interface, const mDNSs32 serviceID, const mDNSAddr *addr,
                                        const mDNSIPPort port, mDNSu32 scoped, mDNSu32 timeout, mDNSBool cellIntf, mDNSu16 resGroupID, mDNSBool reqA,
                                        mDNSBool reqAAAA, mDNSBool reqDO)
{
    (void) m;
    (void) d;
    (void) interface;
    (void) serviceID;
    (void) addr;
    (void) port;
    (void) scoped;
    (void) timeout;
    (void) cellIntf;
    (void) resGroupID;
    (void) reqA;
    (void) reqAAAA;
    (void) reqDO;
    
    return mDNSNULL;
}

mDNSexport void uDNS_SetupWABQueries(mDNS *const m)
{
    (void) m;
}

mDNSexport void uDNS_StartWABQueries(mDNS *const m, int queryType)
{
    (void) m;
    (void) queryType;
}

mDNSexport void uDNS_StopWABQueries(mDNS *const m, int queryType)
{
    (void) m;
    (void) queryType;
}

mDNSexport void mDNS_AddDynDNSHostName(mDNS *m, const domainname *fqdn, mDNSRecordCallback *StatusCallback, const void *StatusContext)
{
    (void) m;
    (void) fqdn;
    (void) StatusCallback;
    (void) StatusContext;
}
mDNSexport void mDNS_SetPrimaryInterfaceInfo(mDNS *m, const mDNSAddr *v4addr, const mDNSAddr *v6addr, const mDNSAddr *router)
{
    (void) m;
    (void) v4addr;
    (void) v6addr;
    (void) router;
}

mDNSexport void mDNS_RemoveDynDNSHostName(mDNS *m, const domainname *fqdn)
{
    (void) m;
    (void) fqdn;
}

mDNSexport void RecreateNATMappings(mDNS *const m, const mDNSu32 waitTicks)
{
    (void) m;
    (void) waitTicks;
}

mDNSexport mDNSBool IsGetZoneDataQuestion(DNSQuestion *q)
{
    (void)q;
    
    return mDNSfalse;
}

#endif // !UNICAST_DISABLED
