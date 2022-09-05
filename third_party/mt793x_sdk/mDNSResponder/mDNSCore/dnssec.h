/* -*- Mode: C; tab-width: 4 -*-
 *
 * Copyright (c) 2011 Apple Computer, Inc. All rights reserved.
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
#ifndef __DNSSEC_H
#define __DNSSEC_H

#include "CryptoAlg.h"
#include "mDNSDebug.h"

typedef enum
{
    RRVS_rr, RRVS_rrsig, RRVS_key, RRVS_rrsig_key, RRVS_ds, RRVS_done,
} RRVerifierSet;

typedef struct RRVerifier_struct RRVerifier;
typedef struct DNSSECVerifier_struct DNSSECVerifier;
typedef struct AuthChain_struct AuthChain;
typedef struct InsecureContext_struct InsecureContext;

struct RRVerifier_struct
{
    RRVerifier *next;
    mDNSu16 rrtype;
    mDNSu16 rrclass;
    mDNSu32 rroriginalttl;
    mDNSu16 rdlength;
    mDNSu16 found;
    mDNSu32 namehash;
    mDNSu32 rdatahash;
    domainname name;
    mDNSu8  *rdata;
};

// Each AuthChain element has one rrset (with multiple resource records of same type), rrsig and key
// that validates the rrset.
struct AuthChain_struct
{
	AuthChain  *next;		// Next element in the chain
	RRVerifier *rrset;		// RRSET that is authenticated
	RRVerifier *rrsig;		// Signature for that RRSET
	RRVerifier *key;		// Public key for that RRSET
};

#define ResetAuthChain(dv) {    \
    (dv)->ac = mDNSNULL;        \
    (dv)->actail = &((dv)->ac); \
}

typedef void DNSSECVerifierCallback (mDNS *const m, DNSSECVerifier *dv, DNSSECStatus status);
//
// When we do a validation for a question, there might be additional validations that needs to be done e.g.,
// wildcard expanded answer. It is also possible that in the case of nsec we need to prove both that a wildcard
// does not apply and the closest encloser proves that name does not exist. We identify these with the following
// flags.
//
// Note: In the following, by "marking the validation", we mean that as part of validation we need to prove
// the ones that are marked with.
//
// A wildcard may be used to answer a question. In that case, we need to verify that the right wildcard was
// used in answering the question. This is done by marking the validation with WILDCARD_PROVES_ANSWER_EXPANDED.
//
// Sometimes we get a NXDOMAIN response. In this case, we may have a wildcard where we need to prove
// that the wildcard proves that the name does not exist. This is done by marking the validation with
// WILDCARD_PROVES_NONAME_EXISTS.
//
// In the case of NODATA error, sometimes the name may exist but the query type does not exist. This is done by
// marking the validation with NSEC_PROVES_NOTYPE_EXISTS.
//
// In both NXDOMAIN and NODATA proofs, we may have to prove that the NAME does not exist. This is done by marking
// the validation with NSEC_PROVES_NONAME_EXISTS.
//
#define WILDCARD_PROVES_ANSWER_EXPANDED 0x00000001
#define WILDCARD_PROVES_NONAME_EXISTS   0x00000002
#define NSEC_PROVES_NOTYPE_EXISTS       0x00000004
#define NSEC_PROVES_NONAME_EXISTS       0x00000008
#define NSEC3_OPT_OUT					0x00000010 // OptOut was set in NSEC3

struct DNSSECVerifier_struct
{
    domainname origName;            // Original question name that needs verification
    mDNSu16 origType;               // Original question type corresponding to origName
    mDNSu16 currQtype;              // Current question type that is being verified
    mDNSInterfaceID InterfaceID;    // InterfaceID of the question
    DNSQuestion q;
    mDNSu8 recursed;                // Number of times recursed during validation
    mDNSu8 ValidationRequired;      // Copy of the question's ValidationRequired status
    mDNSu8 InsecureProofDone;
    mDNSu8 NumPackets;              // Number of packets that we send on the wire for DNSSEC verification.
    mDNSs32 StartTime;              // Time the DNSSEC verification starts
    mDNSu32 flags;
    RRVerifierSet next;
    domainname *wildcardName;       // set if the answer is wildcard expanded
    RRVerifier *pendingNSEC;
    DNSSECVerifierCallback *DVCallback;
    DNSSECVerifier *parent;
    RRVerifier *rrset;              // rrset for which we have to verify
    RRVerifier *rrsig;              // RRSIG for rrset
    RRVerifier *key;                // DNSKEY for rrset
    RRVerifier *rrsigKey;           // RRSIG for DNSKEY
    RRVerifier *ds;                 // DS for DNSKEY set in parent zone
    AuthChain *saveac;
    AuthChain *ac;
    AuthChain **actail;
    AlgContext *ctx;
};


struct InsecureContext_struct
{
    DNSSECVerifier *dv;             // dv for which we are doing the insecure proof
    mDNSu8 skip;                    // labels to skip for forming the name from origName
    DNSSECStatus status;            // status to deliver when done
    mDNSu8 triggerLabelCount;       // Label count of the name that triggered the insecure proof
    DNSQuestion q;
};

#define LogDNSSEC LogOperation

#define DNS_SERIAL_GT(a, b) ((int)((a) - (b)) > 0)
#define DNS_SERIAL_LT(a, b) ((int)((a) - (b)) < 0)

extern void StartDNSSECVerification(mDNS *const m, void *context);
extern RRVerifier* AllocateRRVerifier(const ResourceRecord *const rr, mStatus *status);
extern mStatus AddRRSetToVerifier(DNSSECVerifier *dv, const ResourceRecord *const rr, RRVerifier *rv, RRVerifierSet set);
extern void VerifySignature(mDNS *const m, DNSSECVerifier *dv, DNSQuestion *q);
extern void FreeDNSSECVerifier(mDNS *const m, DNSSECVerifier *dv);
extern DNSSECVerifier *AllocateDNSSECVerifier(mDNS *const m, const domainname *name, mDNSu16 rrtype, mDNSInterfaceID InterfaceID,
                                              mDNSu8 ValidationRequired, DNSSECVerifierCallback dvcallback, mDNSQuestionCallback qcallback);
extern void InitializeQuestion(mDNS *const m, DNSQuestion *question, mDNSInterfaceID InterfaceID, const domainname *qname,
                               mDNSu16 qtype, mDNSQuestionCallback *callback, void *context);
extern void ValidateRRSIG(DNSSECVerifier *dv, RRVerifierSet type, const ResourceRecord *const rr);
extern void AuthChainLink(DNSSECVerifier *dv, AuthChain *ae);
extern mStatus DNSNameToLowerCase(domainname *d, domainname *result);
extern int DNSMemCmp(const mDNSu8 *const m1, const mDNSu8 *const m2, int len);
extern int DNSSECCanonicalOrder(const domainname *const d1, const domainname *const d2, int *subdomain);
extern void ProveInsecure(mDNS *const m, DNSSECVerifier *dv, InsecureContext *ic, domainname *trigger);
extern void BumpDNSSECStats(mDNS *const m, DNSSECStatsAction action, DNSSECStatsType type, mDNSu32 value);
extern char *DNSSECStatusName(DNSSECStatus status);

// DNSSECProbe belongs in DNSSECSupport.h but then we don't want to expose yet another plaform specific dnssec file
// to other platforms where dnssec is not supported.
extern void DNSSECProbe(mDNS *const m);

#endif // __DNSSEC_H
