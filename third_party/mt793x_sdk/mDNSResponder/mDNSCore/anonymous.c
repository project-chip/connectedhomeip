/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2012 Apple Computer, Inc. All rights reserved.
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

#include "anonymous.h"
#include "CryptoAlg.h"
#include "DNSCommon.h"
#include "mDNSEmbeddedAPI.h"

// Define ANONYMOUS_DISABLED to remove all the anonymous functionality
// and use the stub functions implemented later in this file.

#ifndef ANONYMOUS_DISABLED

#define ANON_NSEC3_ITERATIONS 1

mDNSlocal mDNSBool InitializeNSEC3Record(ResourceRecord * rr, const mDNSu8 * AnonData, int len, mDNSu32 salt)
{
    const mDNSu8 * ptr;
    rdataNSEC3 * nsec3 = (rdataNSEC3 *) rr->rdata->u.data;
    mDNSu8 *tmp, *nxt;
    unsigned short iter = ANON_NSEC3_ITERATIONS;
    int hlen;
    const mDNSu8 hashName[NSEC3_MAX_HASH_LEN];

    // Construct the RDATA first and construct the owner name based on that.
    ptr = (const mDNSu8 *) &salt;
    debugf("InitializeNSEC3Record: %x%x%x%x, name %##s", ptr[0], ptr[1], ptr[2], ptr[3], rr->name->c);

    // Set the RDATA
    nsec3->alg        = SHA1_DIGEST_TYPE;
    nsec3->flags      = 0;
    nsec3->iterations = swap16(iter);
    nsec3->saltLength = 4;
    tmp               = (mDNSu8 *) &nsec3->salt;
    *tmp++            = ptr[0];
    *tmp++            = ptr[1];
    *tmp++            = ptr[2];
    *tmp++            = ptr[3];

    // hashLength, nxt, bitmap
    *tmp++ = SHA1_HASH_LENGTH; // hash length
    nxt    = tmp;
    tmp += SHA1_HASH_LENGTH;
    *tmp++ = 0;                      // window number
    *tmp++ = NSEC_MCAST_WINDOW_SIZE; // window length
    mDNSPlatformMemZero(tmp, NSEC_MCAST_WINDOW_SIZE);
    tmp[kDNSType_PTR >> 3] |= 128 >> (kDNSType_PTR & 7);

    // Hash the base service name + salt + AnonData
    if (!NSEC3HashName(rr->name, nsec3, AnonData, len, hashName, &hlen))
    {
        LogMsg("InitializeNSEC3Record: NSEC3HashName failed for ##s", rr->name->c);
        return mDNSfalse;
    }
    if (hlen != SHA1_HASH_LENGTH)
    {
        LogMsg("InitializeNSEC3Record: hlen wrong %d", hlen);
        return mDNSfalse;
    }
    mDNSPlatformMemCopy(nxt, hashName, hlen);

    return mDNStrue;
}

mDNSlocal ResourceRecord * ConstructNSEC3Record(const domainname * service, const mDNSu8 * AnonData, int len, mDNSu32 salt)
{
    ResourceRecord * rr;
    int dlen;
    domainname * name;

    // We are just allocating an RData which has StandardAuthRDSize
    if (StandardAuthRDSize < MCAST_NSEC3_RDLENGTH)
    {
        LogMsg("ConstructNSEC3Record: StandardAuthRDSize %d smaller than MCAST_NSEC3_RDLENGTH %d", StandardAuthRDSize,
               MCAST_NSEC3_RDLENGTH);
        return mDNSNULL;
    }

    dlen = DomainNameLength(service);

    // Allocate space for the name and RData.
    rr = mDNSPlatformMemAllocate(sizeof(ResourceRecord) + dlen + sizeof(RData));
    if (!rr)
        return mDNSNULL;
    name              = (domainname *) ((mDNSu8 *) rr + sizeof(ResourceRecord));
    rr->RecordType    = kDNSRecordTypePacketAuth;
    rr->InterfaceID   = mDNSInterface_Any;
    rr->name          = (const domainname *) name;
    rr->rrtype        = kDNSType_NSEC3;
    rr->rrclass       = kDNSClass_IN;
    rr->rroriginalttl = kStandardTTL;
    rr->rDNSServer    = mDNSNULL;
    rr->rdlength      = MCAST_NSEC3_RDLENGTH;
    rr->rdestimate    = MCAST_NSEC3_RDLENGTH;
    rr->rdata         = (RData *) ((mDNSu8 *) rr->name + dlen);

    AssignDomainName(name, service);
    if (!InitializeNSEC3Record(rr, AnonData, len, salt))
    {
        mDNSPlatformMemFree(rr);
        return mDNSNULL;
    }
    return rr;
}

mDNSlocal ResourceRecord * CopyNSEC3ResourceRecord(AnonymousInfo * si, const ResourceRecord * rr)
{
    int len;
    domainname * name;
    ResourceRecord * nsec3rr;

    if (rr->rdlength < MCAST_NSEC3_RDLENGTH)
    {
        LogMsg("CopyNSEC3ResourceRecord: rdlength %d smaller than MCAST_NSEC3_RDLENGTH %d", rr->rdlength, MCAST_NSEC3_RDLENGTH);
        return mDNSNULL;
    }
    // Allocate space for the name and the rdata along with the ResourceRecord
    len     = DomainNameLength(rr->name);
    nsec3rr = mDNSPlatformMemAllocate(sizeof(ResourceRecord) + len + sizeof(RData));
    if (!nsec3rr)
        return mDNSNULL;

    *nsec3rr      = *rr;
    name          = (domainname *) ((mDNSu8 *) nsec3rr + sizeof(ResourceRecord));
    nsec3rr->name = (const domainname *) name;
    AssignDomainName(name, rr->name);

    nsec3rr->rdata = (RData *) ((mDNSu8 *) nsec3rr->name + len);
    mDNSPlatformMemCopy(nsec3rr->rdata->u.data, rr->rdata->u.data, rr->rdlength);

    si->nsec3RR = nsec3rr;

    return nsec3rr;
}

// When a service is started or a browse is started with the Anonymous data, we allocate a new random
// number and based on that allocate a new NSEC3 resource record whose hash is a function of random number (salt) and
// the anonymous data.
//
// If we receive a packet with the NSEC3 option, we need to cache that along with the resource record so that we can
// check against the question to see whether it answers them or not. In that case, we pass the "rr" that we received.
mDNSexport AnonymousInfo * AllocateAnonInfo(const domainname * service, const mDNSu8 * data, int len, const ResourceRecord * rr)
{
    AnonymousInfo * ai;
    ai = (AnonymousInfo *) mDNSPlatformMemAllocate(sizeof(AnonymousInfo));
    if (!ai)
    {
        return mDNSNULL;
    }
    mDNSPlatformMemZero(ai, sizeof(AnonymousInfo));
    if (rr)
    {
        if (!CopyNSEC3ResourceRecord(ai, rr))
        {
            mDNSPlatformMemFree(ai);
            return mDNSNULL;
        }
        return ai;
    }
    ai->salt     = mDNSRandom(0xFFFFFFFF);
    ai->AnonData = mDNSPlatformMemAllocate(len);
    if (!ai->AnonData)
    {
        mDNSPlatformMemFree(ai);
        return mDNSNULL;
    }
    ai->AnonDataLen = len;
    mDNSPlatformMemCopy(ai->AnonData, data, len);
    ai->nsec3RR = ConstructNSEC3Record(service, data, len, ai->salt);
    if (!ai->nsec3RR)
    {
        mDNSPlatformMemFree(ai);
        return mDNSNULL;
    }
    return ai;
}

mDNSexport void FreeAnonInfo(AnonymousInfo * ai)
{
    if (ai->nsec3RR)
        mDNSPlatformMemFree(ai->nsec3RR);
    if (ai->AnonData)
        mDNSPlatformMemFree(ai->AnonData);
    mDNSPlatformMemFree(ai);
}

mDNSexport void ReInitAnonInfo(AnonymousInfo ** AnonInfo, const domainname * name)
{
    if (*AnonInfo)
    {
        AnonymousInfo * ai = *AnonInfo;
        *AnonInfo          = AllocateAnonInfo(name, ai->AnonData, ai->AnonDataLen, mDNSNULL);
        if (!(*AnonInfo))
            *AnonInfo = ai;
        else
            FreeAnonInfo(ai);
    }
}

// This function should be used only if you know that the question and
// the resource record belongs to the same set. The main usage is
// in ProcessQuery where we find the question to be part of the same
// set as the resource record, but it needs the AnonData to be
// initialized so that it can walk the cache records to see if they
// answer the question.
mDNSexport void SetAnonData(DNSQuestion * q, ResourceRecord * rr, mDNSBool ForQuestion)
{
    if (!q->AnonInfo || !rr->AnonInfo)
    {
        LogMsg("SetAnonData: question %##s(%p), rr %##s(%p), NULL", q->qname.c, q->AnonInfo, rr->name->c, rr->AnonInfo);
        return;
    }

    debugf("SetAnonData: question %##s(%p), rr %##s(%p)", q->qname.c, q->AnonInfo, rr->name->c, rr->AnonInfo);
    if (ForQuestion)
    {
        if (!q->AnonInfo->AnonData)
        {
            q->AnonInfo->AnonData = mDNSPlatformMemAllocate(rr->AnonInfo->AnonDataLen);
            if (!q->AnonInfo->AnonData)
                return;
        }
        mDNSPlatformMemCopy(q->AnonInfo->AnonData, rr->AnonInfo->AnonData, rr->AnonInfo->AnonDataLen);
        q->AnonInfo->AnonDataLen = rr->AnonInfo->AnonDataLen;
    }
    else
    {
        if (!rr->AnonInfo->AnonData)
        {
            rr->AnonInfo->AnonData = mDNSPlatformMemAllocate(q->AnonInfo->AnonDataLen);
            if (!rr->AnonInfo->AnonData)
                return;
        }
        mDNSPlatformMemCopy(rr->AnonInfo->AnonData, q->AnonInfo->AnonData, q->AnonInfo->AnonDataLen);
        rr->AnonInfo->AnonDataLen = q->AnonInfo->AnonDataLen;
    }
}

// returns -1 if the caller should ignore the result
// returns 1 if the record answers the question
// returns 0 if the record does not answer the question
mDNSexport int AnonInfoAnswersQuestion(const ResourceRecord * const rr, const DNSQuestion * const q)
{
    mDNSexport mDNS mDNSStorage;
    ResourceRecord * nsec3RR;
    int i;
    AnonymousInfo *qai, *rai;
    mDNSu8 * AnonData;
    int AnonDataLen;
    rdataNSEC3 * nsec3;
    int hlen;
    const mDNSu8 hashName[NSEC3_MAX_HASH_LEN];
    int nxtLength;
    mDNSu8 * nxtName;

    debugf("AnonInfoAnswersQuestion: question qname %##s", q->qname.c);

    // Currently only PTR records can have anonymous information
    if (q->qtype != kDNSType_PTR)
    {
        return -1;
    }

    // We allow anonymous questions to be answered by both normal services (without the
    // anonymous information) and anonymous services that are part of the same set. And
    // normal questions discover normal services and all anonymous services.
    //
    // The three cases have been enumerated clearly even though they all behave the
    // same way.
    if (!q->AnonInfo)
    {
        debugf("AnonInfoAnswersQuestion: not a anonymous type question");
        if (!rr->AnonInfo)
        {
            // case 1
            return -1;
        }
        else
        {
            // case 2
            debugf("AnonInfoAnswersQuestion: Question %##s not answered using anonymous record %##s", q->qname.c, rr->name->c);
            return -1;
        }
    }
    else
    {
        // case 3
        if (!rr->AnonInfo)
        {
            debugf("AnonInfoAnswersQuestion: not a anonymous type record");
            return -1;
        }
    }

    // case 4: We have the anonymous information both in the question and the record. We need
    // two sets of information to validate.
    //
    // 1) Anonymous data that identifies the set/group
    // 2) NSEC3 record that contains the hash and the salt
    //
    // If the question is a remote one, it does not have the anonymous information to validate (just
    // the NSEC3 record) and hence the anonymous data should come from the local resource record. If the
    // question is local, it can come from either of them and if there is a mismatch between the
    // question and record, it won't validate.

    qai = q->AnonInfo;
    rai = rr->AnonInfo;

    if (qai->AnonData && rai->AnonData)
    {
        // Before a cache record is created, if there is a matching question i.e., part
        // of the same set, then when the cache is created we also set the anonymous
        // information. Otherwise, the cache record contains just the NSEC3 record and we
        // won't be here for that case.
        //
        // It is also possible that a local question is matched against the local AuthRecord
        // as that is also the case for which the AnonData would be non-NULL for both.
        // We match questions against AuthRecords (rather than the cache) for LocalOnly case and
        // to see whether a .local query should be suppressed or not. The latter never happens
        // because PTR queries are never suppressed.

        // If they don't belong to the same anonymous set, then no point in validating.
        if ((qai->AnonDataLen != rai->AnonDataLen) || mDNSPlatformMemCmp(qai->AnonData, rai->AnonData, qai->AnonDataLen) != 0)
        {
            // debugf("AnonInfoAnswersQuestion: AnonData mis-match for record  %s question %##s ", RRDisplayString(&mDNSStorage,
            // rr),
            //        q->qname.c);
            return 0;
        }
        // AnonData matches i.e they belong to the same group and the same service.
        // LogInfo("AnonInfoAnswersQuestion: Answering qname %##s, rname %##s, without validation", q->qname.c, rr->name->c);
        return 1;
    }
    else
    {
        // debugf("AnonInfoAnswersQuestion: question %p, record %p", qai->AnonData, rai->AnonData);
    }

    if (qai->AnonData)
    {
        // If there is AnonData, then this is a local question. The
        // NSEC3 RR comes from the resource record which could be part
        // of the cache or local auth record. The cache entry could
        // be from a remote host or created when we heard our own
        // announcements. In any case, we use that to see if it matches
        // the question.
        AnonData    = qai->AnonData;
        AnonDataLen = qai->AnonDataLen;
        nsec3RR     = rai->nsec3RR;
    }
    else
    {
        // Remote question or hearing our own question back
        AnonData    = rai->AnonData;
        AnonDataLen = rai->AnonDataLen;
        nsec3RR     = qai->nsec3RR;
    }

    if (!AnonData || !nsec3RR)
    {
        // AnonData can be NULL for the cache entry and if we are hearing our own question back, AnonData is NULL for
        // that too and we can end up here for that case.
        // debugf("AnonInfoAnswersQuestion: AnonData %p or nsec3RR %p, NULL for question %##s, record %s", AnonData, nsec3RR,
        //        q->qname.c, RRDisplayString(&mDNSStorage, rr));
        return 0;
    }
    // debugf("AnonInfoAnswersQuestion: Validating question %##s, ResourceRecord %s", q->qname.c,
    //        RRDisplayString(&mDNSStorage, nsec3RR));

    nsec3 = (rdataNSEC3 *) nsec3RR->rdata->u.data;

    if (!NSEC3HashName(nsec3RR->name, nsec3, AnonData, AnonDataLen, hashName, &hlen))
    {
        LogMsg("AnonInfoAnswersQuestion: NSEC3HashName failed for ##s", nsec3RR->name->c);
        return mDNSfalse;
    }
    if (hlen != SHA1_HASH_LENGTH)
    {
        LogMsg("AnonInfoAnswersQuestion: hlen wrong %d", hlen);
        return mDNSfalse;
    }

    NSEC3Parse(nsec3RR, mDNSNULL, &nxtLength, &nxtName, mDNSNULL, mDNSNULL);

    if (hlen != nxtLength)
    {
        LogMsg("AnonInfoAnswersQuestion: ERROR!! hlen %d not same as nxtLength %d", hlen, nxtLength);
        return mDNSfalse;
    }

    for (i = 0; i < nxtLength; i++)
    {
        if (nxtName[i] != hashName[i])
        {
            // debugf("AnonInfoAnswersQuestion: mismatch output %x, digest %x, i %d", nxtName[i + 1], hashName[i], i);
            return 0;
        }
    }
    // LogInfo("AnonInfoAnswersQuestion: ResourceRecord %s matched question %##s (%s)", RRDisplayString(&mDNSStorage, nsec3RR),
    //         q->qname.c, DNSTypeName(q->qtype));
    return 1;
}

// Find a matching NSEC3 record for the name. We parse the questions and the records in the packet in order.
// Similarly we also parse the NSEC3 records in order and this mapping to the questions and records
// respectively.
mDNSlocal CacheRecord * FindMatchingNSEC3ForName(mDNS * const m, CacheRecord ** nsec3, const domainname * name)
{
    CacheRecord * cr;
    CacheRecord ** prev = nsec3;

    (void) m;

    for (cr = *nsec3; cr; cr = cr->next)
    {
        if (SameDomainName(cr->resrec.name, name))
        {
            debugf("FindMatchingNSEC3ForName: NSEC3 record %s matched %##s", CRDisplayString(m, cr), name->c);
            *prev    = cr->next;
            cr->next = mDNSNULL;
            return cr;
        }
        prev = &cr->next;
    }
    return mDNSNULL;
}

mDNSexport void InitializeAnonInfoForQuestion(mDNS * const m, CacheRecord ** McastNSEC3Records, DNSQuestion * q)
{
    CacheRecord * nsec3CR;

    if (q->qtype != kDNSType_PTR)
        return;

    nsec3CR = FindMatchingNSEC3ForName(m, McastNSEC3Records, &q->qname);
    if (nsec3CR)
    {
        q->AnonInfo = AllocateAnonInfo(mDNSNULL, mDNSNULL, 0, &nsec3CR->resrec);
        if (q->AnonInfo)
        {
            debugf("InitializeAnonInfoForQuestion: Found a matching NSEC3 record %s, for %##s (%s)",
                   RRDisplayString(m, q->AnonInfo->nsec3RR), q->qname.c, DNSTypeName(q->qtype));
        }
        ReleaseCacheRecord(m, nsec3CR);
    }
}

mDNSexport void InitializeAnonInfoForCR(mDNS * const m, CacheRecord ** McastNSEC3Records, CacheRecord * cr)
{
    CacheRecord * nsec3CR;

    if (!(*McastNSEC3Records))
        return;

    // If already initialized or not a PTR type, we don't have to do anything
    if (cr->resrec.AnonInfo || cr->resrec.rrtype != kDNSType_PTR)
        return;

    nsec3CR = FindMatchingNSEC3ForName(m, McastNSEC3Records, cr->resrec.name);
    if (nsec3CR)
    {
        cr->resrec.AnonInfo = AllocateAnonInfo(mDNSNULL, mDNSNULL, 0, &nsec3CR->resrec);
        if (cr->resrec.AnonInfo)
        {
            debugf("InitializeAnonInfoForCR: Found a matching NSEC3 record %s, for %##s (%s)",
                   RRDisplayString(m, cr->resrec.AnonInfo->nsec3RR), cr->resrec.name->c, DNSTypeName(cr->resrec.rrtype));
        }
        ReleaseCacheRecord(m, nsec3CR);
    }
}

mDNSexport mDNSBool IdenticalAnonInfo(AnonymousInfo * a1, AnonymousInfo * a2)
{
    // if a1 is NULL and a2 is not NULL AND vice-versa
    // return false as there is a change.
    if ((a1 != mDNSNULL) != (a2 != mDNSNULL))
        return mDNSfalse;

    // Both could be NULL or non-NULL
    if (a1 && a2)
    {
        // The caller already verified that the owner name is the same.
        // Check whether the RData is same.
        if (!IdenticalSameNameRecord(a1->nsec3RR, a2->nsec3RR))
        {
            debugf("IdenticalAnonInfo: nsec3RR mismatch");
            return mDNSfalse;
        }
    }
    return mDNStrue;
}

mDNSexport void CopyAnonInfoForCR(mDNS * const m, CacheRecord * crto, CacheRecord * crfrom)
{
    AnonymousInfo * aifrom = crfrom->resrec.AnonInfo;
    AnonymousInfo * aito   = crto->resrec.AnonInfo;

    (void) m;

    if (!aifrom)
        return;

    if (aito)
    {
        crto->resrec.AnonInfo = aifrom;
        FreeAnonInfo(aito);
        crfrom->resrec.AnonInfo = mDNSNULL;
    }
    else
    {
        FreeAnonInfo(aifrom);
        crfrom->resrec.AnonInfo = mDNSNULL;
    }
}

#else // !ANONYMOUS_DISABLED

mDNSexport void ReInitAnonInfo(AnonymousInfo ** si, const domainname * name)
{
    (void) si;
    (void) name;
}

mDNSexport AnonymousInfo * AllocateAnonInfo(const domainname * service, const mDNSu8 * AnonData, int len, const ResourceRecord * rr)
{
    (void) service;
    (void) AnonData;
    (void) len;
    (void) rr;

    return mDNSNULL;
}

mDNSexport void FreeAnonInfo(AnonymousInfo * ai)
{
    (void) ai;
}

mDNSexport void SetAnonData(DNSQuestion * q, ResourceRecord * rr, mDNSBool ForQuestion)
{
    (void) q;
    (void) rr;
    (void) ForQuestion;
}

mDNSexport int AnonInfoAnswersQuestion(const ResourceRecord * const rr, const DNSQuestion * const q)
{
    (void) rr;
    (void) q;

    return mDNSfalse;
}

mDNSexport void InitializeAnonInfoForQuestion(mDNS * const m, CacheRecord ** McastNSEC3Records, DNSQuestion * q)
{
    (void) m;
    (void) McastNSEC3Records;
    (void) q;
}

mDNSexport void InitializeAnonInfoForCR(mDNS * const m, CacheRecord ** McastNSEC3Records, CacheRecord * cr)
{
    (void) m;
    (void) McastNSEC3Records;
    (void) cr;
}

mDNSexport void CopyAnonInfoForCR(mDNS * const m, CacheRecord * crto, CacheRecord * crfrom)
{
    (void) m;
    (void) crto;
    (void) crfrom;
}

mDNSexport mDNSBool IdenticalAnonInfo(AnonymousInfo * a1, AnonymousInfo * a2)
{
    (void) a1;
    (void) a2;

    return mDNStrue;
}

#endif // !ANONYMOUS_DISABLED
