/*
 *    Copyright (c) 2021 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "lib/support/CHIPMem.h"
#include "lib/support/CodeUtils.h"
#include "lib/support/ScopedBuffer.h"
#include <access/AuthMode.h>
#include <lib/support/Defer.h>
#include <transport/SecureSession.h>
#include <transport/SecureSessionTable.h>

namespace chip {
namespace Transport {

Optional<SessionHandle> SecureSessionTable::CreateNewSecureSessionForTest(SecureSession::Type secureSessionType,
                                                                          uint16_t localSessionId, NodeId localNodeId,
                                                                          NodeId peerNodeId, CATValues peerCATs,
                                                                          uint16_t peerSessionId, FabricIndex fabricIndex,
                                                                          const ReliableMessageProtocolConfig & config)
{
    if (secureSessionType == SecureSession::Type::kCASE)
    {
        if ((fabricIndex == kUndefinedFabricIndex) || (localNodeId == kUndefinedNodeId) || (peerNodeId == kUndefinedNodeId))
        {
            return Optional<SessionHandle>::Missing();
        }
    }
    else if (secureSessionType == SecureSession::Type::kPASE)
    {
        if ((fabricIndex != kUndefinedFabricIndex) || (localNodeId != kUndefinedNodeId) || (peerNodeId != kUndefinedNodeId))
        {
            // TODO: This secure session type is infeasible! We must fix the tests
            if (false)
            {
                return Optional<SessionHandle>::Missing();
            }

            (void) fabricIndex;
        }
    }

    SecureSession * result = mEntries.CreateObject(*this, secureSessionType, localSessionId, localNodeId, peerNodeId, peerCATs,
                                                   peerSessionId, fabricIndex, config);
    return result != nullptr ? MakeOptional<SessionHandle>(*result) : Optional<SessionHandle>::Missing();
}

Optional<SessionHandle> SecureSessionTable::CreateNewSecureSession(SecureSession::Type secureSessionType,
                                                                   ScopedNodeId sessionEvictionHint)
{
    Optional<SessionHandle> rv = Optional<SessionHandle>::Missing();
    SecureSession * allocated  = nullptr;

    auto sessionId = FindUnusedSessionId();
    VerifyOrReturnValue(sessionId.HasValue(), Optional<SessionHandle>::Missing());

    //
    // We allocate a new session out of the pool if we have space in it. If we don't, we need
    // to run the eviction algorithm to get a free slot. We shall ALWAYS be guaranteed to evict
    // an existing session in the table in normal operating circumstances.
    //
    if (mEntries.Allocated() < GetMaxSessionTableSize())
    {
        allocated = mEntries.CreateObject(*this, secureSessionType, sessionId.Value());
    }
    else
    {
        allocated = EvictAndAllocate(sessionId.Value(), secureSessionType, sessionEvictionHint);
    }

    VerifyOrReturnValue(allocated != nullptr, Optional<SessionHandle>::Missing());

    rv             = MakeOptional<SessionHandle>(*allocated);
    mNextSessionId = sessionId.Value() == kMaxSessionID ? static_cast<uint16_t>(kUnsecuredSessionId + 1)
                                                        : static_cast<uint16_t>(sessionId.Value() + 1);

    return rv;
}

SecureSession * SecureSessionTable::EvictAndAllocate(uint16_t localSessionId, SecureSession::Type secureSessionType,
                                                     const ScopedNodeId & sessionEvictionHint)
{
    VerifyOrDieWithMsg(!mRunningEvictionLogic, SecureChannel,
                       "EvictAndAllocate isn't re-entrant, yet someone called us while we're already running");

    mRunningEvictionLogic = true;

    auto cleanup = MakeDefer([this]() { mRunningEvictionLogic = false; });

    ChipLogProgress(SecureChannel, "Evicting a slot for session with LSID: %d, type: %u", localSessionId,
                    (uint8_t) secureSessionType);

    VerifyOrDie(mEntries.Allocated() <= GetMaxSessionTableSize());

    //
    // Create a temporary list of objects each of which points to a session in the existing
    // session table, but are swappable. This allows them to then be used with a sorting algorithm
    // without affecting the sessions in the table itself.
    //
    // The size of this shouldn't place significant demands on the stack if using the default
    // configuration for CHIP_CONFIG_SECURE_SESSION_POOL_SIZE (17). Each item is
    // 8 bytes in size (on a 32-bit platform), and 16 bytes in size (on a 64-bit platform,
    // including padding).
    //
    // Total size of this stack variable = 17 * 8 = 136bytes (32-bit platform), 272 bytes (64-bit platform).
    //
    // Even if the define is set to a large value, it's likely not so bad on the sort of platform setup
    // that would have that sort of pool size.
    //
    // We need to sort (as opposed to just a linear search for the smallest/largest item)
    // since it is possible that the candidate selected for eviction may not actually be
    // released once marked for expiration (see comments below for more details).
    //
    // Consequently, we may need to walk the candidate list till we find one that is.
    // Sorting provides a better overall performance model in this scheme.
    //
    // (#19967): Investigate doing linear search instead.
    //
    //
    SortableSession sortableSessions[CHIP_CONFIG_SECURE_SESSION_POOL_SIZE];

    unsigned int index = 0;

    //
    // Compute two key stats for each session - the number of other sessions that
    // match its fabric, as well as the number of other sessions that match its peer.
    //
    // This will be used by the session eviction algorithm later.
    //
    ForEachSession([&index, &sortableSessions, this](auto * session) {
        sortableSessions[index].mSession             = session;
        sortableSessions[index].mNumMatchingOnFabric = 0;
        sortableSessions[index].mNumMatchingOnPeer   = 0;

        ForEachSession([session, index, &sortableSessions](auto * otherSession) {
            if (session != otherSession)
            {
                if (session->GetFabricIndex() == otherSession->GetFabricIndex())
                {
                    sortableSessions[index].mNumMatchingOnFabric++;

                    if (session->GetPeerNodeId() == otherSession->GetPeerNodeId())
                    {
                        sortableSessions[index].mNumMatchingOnPeer++;
                    }
                }
            }

            return Loop::Continue;
        });

        index++;
        return Loop::Continue;
    });

    auto sortableSessionSpan = Span<SortableSession>(sortableSessions, mEntries.Allocated());
    EvictionPolicyContext policyContext(sortableSessionSpan, sessionEvictionHint);

    DefaultEvictionPolicy(policyContext);
    ChipLogProgress(SecureChannel, "Sorted sessions for eviction...");

    const auto numSessions = mEntries.Allocated();

#if CHIP_DETAIL_LOGGING
    ChipLogDetail(SecureChannel, "Sorted Eviction Candidates (ranked from best candidate to worst):");
    for (auto * session = sortableSessions; session != (sortableSessions + numSessions); session++)
    {
        ChipLogDetail(SecureChannel,
                      "\t%ld: [%p] -- Peer: [%u:" ChipLogFormatX64
                      "] State: '%s', NumMatchingOnFabric: %d NumMatchingOnPeer: %d ActivityTime: %lu",
                      static_cast<long int>(session - sortableSessions), session->mSession,
                      session->mSession->GetPeer().GetFabricIndex(), ChipLogValueX64(session->mSession->GetPeer().GetNodeId()),
                      session->mSession->GetStateStr(), session->mNumMatchingOnFabric, session->mNumMatchingOnPeer,
                      static_cast<unsigned long>(session->mSession->GetLastActivityTime().count()));
    }
#endif

    for (auto * session = sortableSessions; session != (sortableSessions + numSessions); session++)
    {
        if (session->mSession->IsPendingEviction())
        {
            continue;
        }

        ChipLogProgress(SecureChannel, "Candidate Session[%p] - Attempting to evict...", session->mSession);

        auto prevCount = mEntries.Allocated();

        //
        // SessionHolders act like weak-refs on a session, but since they do still add to the ref-count of a SecureSession, we
        // cannot actually tell whether there are truly any strong-refs (SessionHandles) on this session because if we did, we'd
        // avoid evicting it since it's pointless to do so.
        //
        // However, we don't actually have SessionHolders implemented correctly as weak-refs, requiring us to go ahead and 'try' to
        // evict it, and see if it still remains in the table. If it does, we have to try the next one. If it doesn't, we know we've
        // earned a free spot.
        //
        // See #19495.
        //
        session->mSession->MarkForEviction();

        auto newCount = mEntries.Allocated();

        if (newCount < prevCount)
        {
            ChipLogProgress(SecureChannel, "Successfully evicted a session!");
            auto * retSession = mEntries.CreateObject(*this, secureSessionType, localSessionId);
            VerifyOrDie(session != nullptr);
            return retSession;
        }
    }

    VerifyOrDieWithMsg(false, SecureChannel, "We couldn't find any session to evict at all, something's wrong!");
    return nullptr;
}

void SecureSessionTable::DefaultEvictionPolicy(EvictionPolicyContext & evictionContext)
{
    //
    // This implements a spec-compliant sorting policy that ensures both guarantees for sessions per-fabric as
    // mandated by the spec as well as fairness in terms of selecting the most appropriate session to evict
    // based on multiple criteria.
    //
    // See the description of this function in the header for more details on each sorting key below.
    //
    evictionContext.Sort([&evictionContext](const SortableSession & a, const SortableSession & b) -> bool {
        //
        // Sorting on Key1
        //
        if (a.mNumMatchingOnFabric != b.mNumMatchingOnFabric)
        {
            return a.mNumMatchingOnFabric > b.mNumMatchingOnFabric;
        }

        bool doesAMatchSessionHintFabric =
            a.mSession->GetPeer().GetFabricIndex() == evictionContext.GetSessionEvictionHint().GetFabricIndex();
        bool doesBMatchSessionHintFabric =
            b.mSession->GetPeer().GetFabricIndex() == evictionContext.GetSessionEvictionHint().GetFabricIndex();

        //
        // Sorting on Key2
        //
        if (doesAMatchSessionHintFabric != doesBMatchSessionHintFabric)
        {
            return doesAMatchSessionHintFabric > doesBMatchSessionHintFabric;
        }

        //
        // Sorting on Key3
        //
        if (a.mNumMatchingOnPeer != b.mNumMatchingOnPeer)
        {
            return a.mNumMatchingOnPeer > b.mNumMatchingOnPeer;
        }

        int doesAMatchSessionHint = a.mSession->GetPeer() == evictionContext.GetSessionEvictionHint();
        int doesBMatchSessionHint = b.mSession->GetPeer() == evictionContext.GetSessionEvictionHint();

        //
        // Sorting on Key4
        //
        if (doesAMatchSessionHint != doesBMatchSessionHint)
        {
            return doesAMatchSessionHint > doesBMatchSessionHint;
        }

        int aStateScore = 0, bStateScore = 0;
        auto assignStateScore = [](auto & score, const auto & session) {
            if (session.IsDefunct())
            {
                score = 2;
            }
            else if (session.IsActiveSession())
            {
                score = 1;
            }
            else
            {
                score = 0;
            }
        };

        assignStateScore(aStateScore, *a.mSession);
        assignStateScore(bStateScore, *b.mSession);

        //
        // Sorting on Key5
        //
        if (aStateScore != bStateScore)
        {
            return (aStateScore > bStateScore);
        }

        //
        // Sorting on Key6
        //
        return (a->GetLastActivityTime() < b->GetLastActivityTime());
    });
}

Optional<SessionHandle> SecureSessionTable::FindSecureSessionByLocalKey(uint16_t localSessionId)
{
    SecureSession * result = nullptr;
    mEntries.ForEachActiveObject([&](auto session) {
        if (session->GetLocalSessionId() == localSessionId)
        {
            result = session;
            return Loop::Break;
        }
        return Loop::Continue;
    });
    return result != nullptr ? MakeOptional<SessionHandle>(*result) : Optional<SessionHandle>::Missing();
}

Optional<uint16_t> SecureSessionTable::FindUnusedSessionId()
{
    uint16_t candidate_base = 0;
    uint64_t candidate_mask = 0;
    for (uint32_t i = 0; i <= kMaxSessionID; i += 64)
    {
        // candidate_base is the base session ID we are searching from.
        // We have a 64-bit mask anchored at this ID and iterate over the
        // whole session table, setting bits in the mask for in-use IDs.
        // If we can iterate through the entire session table and have
        // any bits clear in the mask, we have available session IDs.
        candidate_base = static_cast<uint16_t>(i + mNextSessionId);
        candidate_mask = 0;
        {
            uint16_t shift = static_cast<uint16_t>(kUnsecuredSessionId - candidate_base);
            if (shift <= 63)
            {
                candidate_mask |= (1ULL << shift); // kUnsecuredSessionId is never available
            }
        }
        mEntries.ForEachActiveObject([&](auto session) {
            uint16_t shift = static_cast<uint16_t>(session->GetLocalSessionId() - candidate_base);
            if (shift <= 63)
            {
                candidate_mask |= (1ULL << shift);
            }
            if (candidate_mask == UINT64_MAX)
            {
                return Loop::Break; // No bits clear means this bucket is full.
            }
            return Loop::Continue;
        });
        if (candidate_mask != UINT64_MAX)
        {
            break; // Any bit clear means we have an available ID in this bucket.
        }
    }
    if (candidate_mask != UINT64_MAX)
    {
        uint16_t offset = 0;
        while (candidate_mask & 1)
        {
            candidate_mask >>= 1;
            ++offset;
        }
        uint16_t available = static_cast<uint16_t>(candidate_base + offset);
        return MakeOptional<uint16_t>(available);
    }

    return NullOptional;
}

} // namespace Transport
} // namespace chip
