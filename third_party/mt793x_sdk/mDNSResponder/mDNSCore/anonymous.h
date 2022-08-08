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

#ifndef __ANONYMOUS_H_
#define __ANONYMOUS_H_
#include "mDNSEmbeddedAPI.h"

extern void ReInitAnonInfo(AnonymousInfo ** si, const domainname * name);
extern AnonymousInfo * AllocateAnonInfo(const domainname * service, const mDNSu8 * AnonData, int len, const ResourceRecord * rr);
extern void FreeAnonInfo(AnonymousInfo * ai);
extern void SetAnonData(DNSQuestion * q, ResourceRecord * rr, mDNSBool ForQuestion);
extern int AnonInfoAnswersQuestion(const ResourceRecord * const rr, const DNSQuestion * const q);
extern void InitializeAnonInfoForCR(mDNS * const m, CacheRecord ** McastNSEC3Records, CacheRecord * cr);
extern void InitializeAnonInfoForQuestion(mDNS * const m, CacheRecord ** McastNSEC3Records, DNSQuestion * q);
extern void CopyAnonInfoForCR(mDNS * const m, CacheRecord * crto, CacheRecord * crfrom);
extern mDNSBool IdenticalAnonInfo(AnonymousInfo * a1, AnonymousInfo * a2);

#endif
