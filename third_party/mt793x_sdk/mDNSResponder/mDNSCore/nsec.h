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
#ifndef __NSEC_H
#define __NSEC_H

#include "dnssec.h"

extern mDNSBool AddNSECSForCacheRecord(mDNS *const m, CacheRecord *crlist, CacheRecord *negcr, mDNSu8 rcode);
extern void WildcardAnswerProof(mDNS *const m, DNSSECVerifier *dv);
extern void ValidateWithNSECS(mDNS *const m, DNSSECVerifier *dv, CacheRecord *rr);
extern mDNSBool NSECAnswersDS(mDNS *const m, ResourceRecord *rr, DNSQuestion *q);
extern int CountLabelsMatch(const domainname *const d1, const domainname *const d2);
extern void NameErrorNSECCallback(mDNS *const m, DNSSECVerifier *dv, DNSSECStatus status);
extern void VerifyNSEC(mDNS *const m, ResourceRecord *rr, RRVerifier *rv, DNSSECVerifier *pdv, CacheRecord *ncr,
	DNSSECVerifierCallback callback);
extern CacheRecord *NSECRecordIsDelegation(mDNS *const m, domainname *name, mDNSu16 qtype);
extern void NoDataNSECCallback(mDNS *const m, DNSSECVerifier *dv, DNSSECStatus status);

#endif // __NSEC_H
