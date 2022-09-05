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
 */

#ifndef __UDNS_H_
#define __UDNS_H_

#include "mDNSEmbeddedAPI.h"
#include "DNSCommon.h"

#ifdef  __cplusplus
extern "C" {
#endif

#define RESTART_GOODBYE_DELAY    (6 * mDNSPlatformOneSecond) // delay after restarting LLQ before nuking previous known answers (avoids flutter if we restart before we have networking up)
#define INIT_UCAST_POLL_INTERVAL (3 * mDNSPlatformOneSecond) // this interval is used after send failures on network transitions
                                                             // which typically heal quickly, so we start agressively and exponentially back off
#define MAX_UCAST_POLL_INTERVAL (60 * 60 * mDNSPlatformOneSecond)
//#define MAX_UCAST_POLL_INTERVAL (1 * 60 * mDNSPlatformOneSecond)
#define LLQ_POLL_INTERVAL       (15 * 60 * mDNSPlatformOneSecond) // Polling interval for zones w/ an advertised LLQ port (ie not static zones) if LLQ fails due to NAT, etc.
#define RESPONSE_WINDOW (60 * mDNSPlatformOneSecond)         // require server responses within one minute of request
#define MAX_DNSSEC_UNANSWERED_QUERIES 1                      // number of unanswered queries from any one uDNS server before turning off DNSSEC Validation
#define MAX_UCAST_UNANSWERED_QUERIES 2                       // number of unanswered queries from any one uDNS server before trying another server
#define DNSSERVER_PENALTY_TIME (60 * mDNSPlatformOneSecond)  // number of seconds for which new questions don't pick this server

// On some interfaces, we want to delay the first retransmission to a minimum of 2 seconds
// rather than the default (1 second).
#define MIN_UCAST_RETRANS_TIMEOUT (2 * mDNSPlatformOneSecond)

#define DEFAULT_UPDATE_LEASE 7200

#define QuestionIntervalStep 3
#define QuestionIntervalStep2 (QuestionIntervalStep*QuestionIntervalStep)
#define QuestionIntervalStep3 (QuestionIntervalStep*QuestionIntervalStep*QuestionIntervalStep)
#define InitialQuestionInterval ((mDNSPlatformOneSecond + QuestionIntervalStep-1) / QuestionIntervalStep)
#define MaxQuestionInterval         (3600 * mDNSPlatformOneSecond)

// just move to MaxQuestionInterval once over this threshold
#define QuestionIntervalThreshold   (QuestionIntervalStep3 * mDNSPlatformOneSecond)

// For Unicast record registrations, we initialize the interval to 1 second. When we send any query for
// the record registration e.g., GetZoneData, we always back off by QuestionIntervalStep
// so that the first retry does not happen until 3 seconds which should be enough for TCP/TLS to be done.
#define INIT_RECORD_REG_INTERVAL (1 * mDNSPlatformOneSecond)
#define MAX_RECORD_REG_INTERVAL (15 * 60 * mDNSPlatformOneSecond)
#define MERGE_DELAY_TIME    (1 * mDNSPlatformOneSecond)

// If we are refreshing, we do it at least 5 times with a min update frequency of
// 5 minutes
#define MAX_UPDATE_REFRESH_COUNT    5
#define MIN_UPDATE_REFRESH_TIME     (5 * 60 * mDNSPlatformOneSecond)

// For questions that use kDNSServiceFlagsTimeout and we don't have a matching resolver e.g., no dns servers,
// then use the default value of 30 seconds
#define DEFAULT_UDNS_TIMEOUT    30 // in seconds

// For questions that are validating responses (q->ValidatingResponse == 1), use 10 seconds
// which accomodates two DNS servers and two queries per DNS server.
#define DEFAULT_UDNSSEC_TIMEOUT    10 // in seconds

// If we are sending queries with EDNS0/DO option and we have no indications that the server
// is DNSSEC aware and we have already reached MAX_DNSSEC_RETRANSMISSIONS, we disable
// validation (for optional case only) for any questions that uses this server
#define MAX_DNSSEC_RETRANSMISSIONS 3

// Entry points into unicast-specific routines

extern void LLQGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneInfo);
extern void startLLQHandshake(mDNS *m, DNSQuestion *q);
extern void sendLLQRefresh(mDNS *m, DNSQuestion *q);

extern void SleepRecordRegistrations(mDNS *m);

// uDNS_UpdateRecord
// following fields must be set, and the update validated, upon entry.
// rr->NewRData
// rr->newrdlength
// rr->UpdateCallback

extern mStatus uDNS_UpdateRecord(mDNS *m, AuthRecord *rr);

extern void SetNextQueryTime(mDNS *const m, const DNSQuestion *const q);
extern mStatus mDNS_Register_internal(mDNS *const m, AuthRecord *const rr);
extern mStatus mDNS_Deregister_internal(mDNS *const m, AuthRecord *const rr, mDNS_Dereg_type drt);
extern mStatus mDNS_StartQuery_internal(mDNS *const m, DNSQuestion *const question);
extern mStatus mDNS_StopQuery_internal(mDNS *const m, DNSQuestion *const question);
#ifndef REMOVE_NAT_FUCTIONALITY
extern mStatus mDNS_StartNATOperation_internal(mDNS *const m, NATTraversalInfo *traversal);
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
extern void RecordRegistrationGotZoneData(mDNS *const m, mStatus err, const ZoneData *zoneData);
extern mStatus uDNS_DeregisterRecord(mDNS *const m, AuthRecord *const rr);
extern const domainname *GetServiceTarget(mDNS *m, AuthRecord *const rr);
extern void uDNS_CheckCurrentQuestion(mDNS *const m);

// integer fields of msg header must be in HOST byte order before calling this routine
extern void uDNS_ReceiveMsg(mDNS *const m, DNSMessage *const msg, const mDNSu8 *const end,
                            const mDNSAddr *const srcaddr, const mDNSIPPort srcport);

extern void uDNS_Tasks(mDNS *const m);
extern void UpdateAllSRVRecords(mDNS *m);
extern void CheckNATMappings(mDNS *m);

extern mStatus         uDNS_SetupDNSConfig(mDNS *const m);

// uDNS_SetupWABQueries reads search domains from the platform layer and starts the Wide Area Bonjour
// (WAB) domain enumeration queries if necessary.

#define UDNS_WAB_BROWSE_QUERY    0x00000001 // Browse queries (b, db)
#define UDNS_WAB_LBROWSE_QUERY   0x00000002 // Browse queries (lb)
#define UDNS_WAB_REG_QUERY       0x00000004 // Registration queries (r and dr)

extern void uDNS_SetupWABQueries(mDNS *const m);
extern void uDNS_StartWABQueries(mDNS *const m, int queryType);
extern void uDNS_StopWABQueries(mDNS *const m, int queryType);
extern domainname      *uDNS_GetNextSearchDomain(mDNS *const m, mDNSInterfaceID InterfaceID, mDNSs8 *searchIndex, mDNSBool ignoreDotLocal);

typedef enum
{
    uDNS_LLQ_Not = 0,   // Normal uDNS answer: Flush any stale records from cache, and respect record TTL
    uDNS_LLQ_Ignore,    // LLQ initial challenge packet: ignore -- has no useful records for us
    uDNS_LLQ_Entire,    // LLQ initial set of answers: Flush any stale records from cache, but assume TTL is 2 x LLQ refresh interval
    uDNS_LLQ_Events     // LLQ event packet: don't flush cache; assume TTL is 2 x LLQ refresh interval
} uDNS_LLQType;

extern uDNS_LLQType    uDNS_recvLLQResponse(mDNS *const m, const DNSMessage *const msg, const mDNSu8 *const end, const mDNSAddr *const srcaddr, const mDNSIPPort srcport, DNSQuestion **matchQuestion);
extern DomainAuthInfo *GetAuthInfoForName_internal(mDNS *m, const domainname *const name);
extern DomainAuthInfo *GetAuthInfoForQuestion(mDNS *m, const DNSQuestion *const q);
extern void DisposeTCPConn(struct tcpInfo_t *tcp);
#ifndef REMOVE_NAT_FUCTIONALITY
// NAT traversal
extern void uDNS_ReceiveNATPacket(mDNS *m, const mDNSInterfaceID InterfaceID, mDNSu8 *pkt, mDNSu16 len); // Called for each received PCP or NAT-PMP packet
extern void natTraversalHandleAddressReply(mDNS *const m, mDNSu16 err, mDNSv4Addr ExtAddr);
extern void natTraversalHandlePortMapReply(mDNS *const m, NATTraversalInfo *n, const mDNSInterfaceID InterfaceID, mDNSu16 err, mDNSIPPort extport, mDNSu32 lease, NATTProtocol protocol);
#endif //#ifndef REMOVE_NAT_FUCTIONALITY
#ifdef  __cplusplus
}
#endif

#endif // __UDNS_H_
