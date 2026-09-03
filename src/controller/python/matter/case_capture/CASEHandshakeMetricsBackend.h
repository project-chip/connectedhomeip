/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
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

#pragma once

#include <controller/python/matter/case_capture/CASEHandshakeMetrics.h>
#include <controller/python/matter/case_capture/CompletedCASEHandshakeQueue.h>
#include <lib/address_resolve/TracingStructs.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/NodeId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemClock.h>
#include <tracing/backend.h>
#include <transport/TracingStructs.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {
namespace python {

// Times the initiator side of CASE by observing the handshake messages as the transport
// layer sends and receives them. The tracing hooks in SessionManager are used rather than
// the received-message observer above because they report both directions: the handshake
// runs over an unauthenticated session, so every Sigma message reaches
// LogMessageSend/LogMessageReceived with its payload header intact.
class CASEHandshakeMetricsBackend : public chip::Tracing::Backend
{
public:
    void LogMessageSend(chip::Tracing::MessageSendInfo & info) override
    {
        VerifyOrReturn(info.payloadHeader != nullptr && info.packetHeader != nullptr);
        VerifyOrReturn(info.payloadHeader->HasProtocol(chip::Protocols::SecureChannel::Id));

        using chip::Protocols::SecureChannel::MsgType;
        const auto opcode  = static_cast<MsgType>(info.payloadHeader->GetMessageType());
        const uint64_t now = CurrentTimestampUs();

        // On the way out this node is the source; on the way in it is the destination.
        const uint16_t exchangeId = info.payloadHeader->GetExchangeID();
        const uint64_t localId    = NodeIdOrUndefined(info.packetHeader->GetSourceNodeId());

        if (opcode == MsgType::CASE_Sigma1)
        {
            // A Sigma1 leaving the node begins a new handshake.
            BeginCASEHandshakeRecord(now, exchangeId, localId);
            return;
        }

        PychipCASEHandshakeMetricsRecord * record = FindCASEHandshakeRecord(exchangeId, localId);
        VerifyOrReturn(record != nullptr);

        if (opcode == MsgType::CASE_Sigma3)
        {
            RecordTimestamp(*record, &PychipCASEHandshakeMetricsRecord::sigma3SentTimestampUs,
                            CASEHandshakeRecordedField::kSigma3Sent, now);
        }
        else if (opcode == MsgType::StatusReport)
        {
            // A failure report leaving this node means this node rejected the peer, which is
            // the one error signal not visible on the inbound side. An outbound success
            // report is normal on the resumption path and is not an error.
            uint16_t generalCode  = 0;
            uint16_t protocolCode = 0;
            VerifyOrReturn(ParseStatusReportCodes(info.payload, generalCode, protocolCode));
            if (!IsStatusReportSuccess(generalCode, protocolCode))
            {
                const bool alreadyRejectedPeer = record->recordedFields.Has(CASEHandshakeRecordedField::kThisNodeRejectedPeer);
                record->recordedFields.Set(CASEHandshakeRecordedField::kThisNodeRejectedPeer);

                // Keep the codes, not just the fact of a rejection. They are already decoded
                // here, and without them the record carries LOCAL_FAILURE with both code
                // fields left at zero - which this struct documents as meaning success. The
                // inbound path owns these fields once it has decoded a report of its own, so
                // only fill them in while they are still unset.
                if (!record->recordedFields.Has(CASEHandshakeRecordedField::kStatusReportCodes))
                {
                    record->statusReportGeneralCode  = generalCode;
                    record->statusReportProtocolCode = protocolCode;
                    record->recordedFields.Set(CASEHandshakeRecordedField::kStatusReportCodes);
                }

                // Rejecting the peer ends the handshake from this side too.
                if (!alreadyRejectedPeer)
                {
                    ConcludeCASEHandshake(*record);
                }
            }
            else if (record->recordedFields.Has(CASEHandshakeRecordedField::kSigma2ResumeReceived))
            {
                // Resumption exchanges no Sigma3, and is closed out by this node acknowledging
                // Sigma2_Resume. That acknowledgement is only sent once the resumption MIC has been
                // validated, so concluding here rather than on Sigma2_Resume itself is what keeps a
                // resumption that failed validation from being reported as a success.
                ConcludeCASEHandshake(*record);
            }
        }
    }

    void LogMessageReceived(chip::Tracing::MessageReceivedInfo & info) override
    {
        VerifyOrReturn(info.payloadHeader != nullptr && info.packetHeader != nullptr);
        VerifyOrReturn(info.payloadHeader->HasProtocol(chip::Protocols::SecureChannel::Id));

        using chip::Protocols::SecureChannel::MsgType;
        const auto opcode  = static_cast<MsgType>(info.payloadHeader->GetMessageType());
        const uint64_t now = CurrentTimestampUs();

        PychipCASEHandshakeMetricsRecord * record = FindCASEHandshakeRecord(
            info.payloadHeader->GetExchangeID(), NodeIdOrUndefined(info.packetHeader->GetDestinationNodeId()));
        VerifyOrReturn(record != nullptr);

        // The peer is not known when Sigma1 goes out, so identity is learned from the first
        // message it sends back. That address is also what ties the handshake to the discovery
        // that resolved it, which is the only exact correlation available: pairing them any
        // earlier could only guess, and would mis-assign spans when concurrent lookups finish
        // in a different order from the handshakes they belong to.
        if (record->peerTransportAddress[0] == kPeerAddressNotYetKnown && info.peerAddress != nullptr)
        {
            info.peerAddress->ToString(record->peerTransportAddress, sizeof(record->peerTransportAddress));
            RecordPeerIdentityAndDiscovery(*record, *info.peerAddress);
        }

        switch (opcode)
        {
        case MsgType::CASE_Sigma2:
            RecordTimestamp(*record, &PychipCASEHandshakeMetricsRecord::sigma2ReceivedTimestampUs,
                            CASEHandshakeRecordedField::kSigma2Received, now);
            break;
        case MsgType::CASE_Sigma2Resume:
            // Only timed here, not concluded. This hook runs before CASESession has validated the
            // resumption MIC, so the message arriving does not yet mean resumption succeeded. The
            // handshake is closed out by the StatusReport this node sends afterwards, which says
            // which way the validation went.
            RecordTimestamp(*record, &PychipCASEHandshakeMetricsRecord::sigma2ResumeReceivedTimestampUs,
                            CASEHandshakeRecordedField::kSigma2ResumeReceived, now);
            break;
        case MsgType::StatusReport:
            // A StatusReport on this exchange ends the handshake wherever it arrives, not only
            // after Sigma3. A responder that is already busy with another CASE answers Sigma1 with
            // BUSY, and that is a result the listener needs to hear about; waiting for Sigma3 would
            // leave the record in flight and the caller told nothing. Phases that never happened
            // keep their timestamps unset, so their durations come back as None.
            {
                const bool reachedConclusion =
                    RecordTimestamp(*record, &PychipCASEHandshakeMetricsRecord::statusReportReceivedTimestampUs,
                                    CASEHandshakeRecordedField::kStatusReportReceived, now);

                // The timestamp is kept whether the peer accepted or rejected Sigma3, since
                // the time to a rejection is still a real measurement. The codes are what
                // tell the two apart.
                uint16_t generalCode  = 0;
                uint16_t protocolCode = 0;
                if (!record->recordedFields.Has(CASEHandshakeRecordedField::kStatusReportCodes) &&
                    ParseStatusReportCodes(info.payload, generalCode, protocolCode))
                {
                    record->statusReportGeneralCode  = generalCode;
                    record->statusReportProtocolCode = protocolCode;
                    record->recordedFields.Set(CASEHandshakeRecordedField::kStatusReportCodes);
                }

                // The handshake is over, accepted or rejected either way. Publish only on the
                // transition so listeners are notified exactly once.
                if (reachedConclusion)
                {
                    ConcludeCASEHandshake(*record);
                }
            }
            break;
        default:
            break;
        }
    }

    // Operational discovery runs to completion before Sigma1 goes out, so there is no record
    // to mark yet. Spans are held here, keyed by peer so that concurrent lookups do not
    // overwrite each other, and are attached by RecordPeerIdentityAndDiscovery once the peer's first
    // reply identifies which lookup the handshake came from.
    void LogNodeLookup(chip::Tracing::NodeLookupInfo & info) override
    {
        VerifyOrReturn(info.request != nullptr);
        PendingDeviceDiscovery * pending = FindOrCreateDiscoveryForPeer(info.request->GetPeerId());
        pending->startUs                 = CurrentTimestampUs();
        pending->done                    = false;
        // The slot is kept after a handshake claims it, so that its address still maps back to
        // a node id. A fresh lookup starts a new span in it, so clear the claim as well or
        // RecordPeerIdentityAndDiscovery would skip it and every later handshake for this peer would
        // report no discovery.
        pending->used = false;
    }

    void LogNodeDiscovered(chip::Tracing::NodeDiscoveredInfo & info) override
    {
        // Intermediate results and retries also arrive here; only completion ends the span.
        VerifyOrReturn(info.type == chip::Tracing::DiscoveryInfoType::kResolutionDone);
        VerifyOrReturn(info.peerId != nullptr);
        PendingDeviceDiscovery * pending = FindDiscoveryForPeer(*info.peerId);
        VerifyOrReturn(pending != nullptr && pending->inUse);
        // A completion carrying no address says nothing about what this lookup found, so it ends no
        // span. Leaving the slot unfinished is what keeps this lookup's duration off an address it
        // did not resolve, while any address the slot already held stays available for naming the
        // peer, which is all that address is there for.
        VerifyOrReturn(info.result != nullptr);
        pending->doneUs = CurrentTimestampUs();
        pending->done   = true;
        // Remember which address belongs to which node, so a handshake can name its peer
        // from the address its replies arrive from.
        pending->address    = info.result->address;
        pending->hasAddress = true;
    }

    void Reset() { ClearRecordsAndDiscoveries(); }

    uint32_t AbandonedCASEHandshakeCount()
    {
        // Swept here as well as when a handshake starts: a run that has stopped establishing would
        // otherwise keep reporting the figure from before its last handshake timed out.
        ExpireStaleCASEHandshakes(CurrentTimestampUs());
        return mAbandonedCASEHandshakeCount;
    }

private:
    void ClearRecordsAndDiscoveries()
    {
        for (auto & record : mInFlightCASEHandshakes)
        {
            record = PychipCASEHandshakeMetricsRecord{};
        }
        mAbandonedCASEHandshakeCount = 0;
        for (auto & pending : mPendingDiscoveries)
        {
            pending = PendingDeviceDiscovery{};
        }
    }

    // An operational discovery span waiting to be attached to the handshake that follows it.
    struct PendingDeviceDiscovery
    {
        chip::PeerId peer;
        chip::Transport::PeerAddress address;
        // Whether this slot is tracking a lookup at all. Held separately rather than inferred from
        // startUs, so no timestamp value has to be reserved to mean "empty".
        bool inUse       = false;
        uint64_t startUs = 0;
        uint64_t doneUs  = 0;
        bool done        = false;
        bool used        = false;
        bool hasAddress  = false;
    };

    // Discovery spans held while waiting for the handshakes they belong to. One slot per peer
    // being resolved concurrently, so this bounds how many parallel lookups can be attributed.
    // Exceeding it costs only the discovery figure: the handshake is still timed, and reports no
    // discovery rather than a wrong one.
    static constexpr size_t kMaxConcurrentDeviceDiscoveries = 8;

    // peerAddress holds a C string, so an empty first byte means the peer has not been seen yet.
    static constexpr char kPeerAddressNotYetKnown = '\0';

    // How long an in-flight handshake is kept before it is written off. CASESession allows a peer
    // 30 seconds of processing time while waiting for Sigma2, on top of the MRP round trip, and
    // fails the establishment once that expires. Twice that allowance is used here, so a handshake
    // is only written off well after the SDK itself has given up on it, while its slot is still
    // freed and counted within a bounded time.
    static constexpr uint64_t kCASEHandshakeProcessingAllowanceUs = 30 * 1000 * 1000ULL;
    static constexpr uint64_t kStaleCASEHandshakeAgeUs            = 2 * kCASEHandshakeProcessingAllowanceUs;

    static uint64_t CurrentTimestampUs() { return chip::System::SystemClock().GetMonotonicMicroseconds64().count(); }

    // A CASE packet header carries a node id in only one direction, so the absent case is
    // mapped to kUndefinedNodeId and takes part in record matching like any other value.
    static chip::NodeId NodeIdOrUndefined(const chip::Optional<chip::NodeId> & nodeId)
    {
        return nodeId.HasValue() ? nodeId.Value() : chip::kUndefinedNodeId;
    }

    // Locates the handshake a message belongs to. Searching newest first means a reused
    // exchange id resolves to the current handshake rather than a finished one.
    // A slot holding nothing is free. A handshake occupies one from Sigma1 until it concludes.
    static bool IsSlotFree(const PychipCASEHandshakeMetricsRecord & record) { return !record.recordedFields.HasAny(); }

    // Locates the in-flight handshake a message belongs to, which is how concurrent handshakes
    // are kept apart.
    PychipCASEHandshakeMetricsRecord * FindCASEHandshakeRecord(uint16_t exchangeId, chip::NodeId localEphemeralNodeId)
    {
        for (auto & candidate : mInFlightCASEHandshakes)
        {
            if (!IsSlotFree(candidate) && candidate.exchangeId == exchangeId &&
                candidate.localEphemeralNodeId == localEphemeralNodeId)
            {
                return &candidate;
            }
        }
        return nullptr;
    }

    // Hands a concluded handshake to its listeners and releases its slot.
    void ConcludeCASEHandshake(PychipCASEHandshakeMetricsRecord & record)
    {
        CompletedCASEHandshakes().Publish(record);
        record = PychipCASEHandshakeMetricsRecord{};
    }

    PendingDeviceDiscovery * FindDiscoveryForPeer(const chip::PeerId & peer)
    {
        for (auto & pending : mPendingDiscoveries)
        {
            if (pending.inUse && pending.peer == peer)
            {
                return &pending;
            }
        }
        return nullptr;
    }

    // Whether a resolved address and the address a message arrived from name the same peer.
    //
    // Deliberately not PeerAddress::operator==, which for UDP also requires the interface to match.
    // Address resolution clears the interface for anything other than an IPv6 link-local address, so
    // that sending follows the routing table, while an inbound packet always carries the interface it
    // was received on. Comparing the two whole addresses would therefore never match for a peer whose
    // resolved address is a unique-local or global one, which is every device reached through a
    // router, and the handshake would report neither its peer nor its discovery. An interface left
    // unset on either side is read as "not stated" rather than as a difference; the address and port
    // already say which peer this is.
    static bool IsSamePeerEndpoint(const chip::Transport::PeerAddress & resolved, const chip::Transport::PeerAddress & observed)
    {
        if (resolved.GetTransportType() != observed.GetTransportType() || !(resolved.GetIPAddress() == observed.GetIPAddress()) ||
            resolved.GetPort() != observed.GetPort())
        {
            return false;
        }
        if (resolved.GetInterface().IsPresent() && observed.GetInterface().IsPresent())
        {
            return resolved.GetInterface() == observed.GetInterface();
        }
        return true;
    }

    // Ranks two lookups that resolved the same address, so the one this handshake followed wins.
    // A span no handshake has taken yet beats one already taken, and between two of equal standing
    // the lookup that started later is the newer one.
    static bool IsCloserDiscoveryMatch(const PendingDeviceDiscovery & candidate, const PendingDeviceDiscovery & incumbent)
    {
        const bool candidateUnclaimed = candidate.done && !candidate.used;
        const bool incumbentUnclaimed = incumbent.done && !incumbent.used;
        if (candidateUnclaimed != incumbentUnclaimed)
        {
            return candidateUnclaimed;
        }
        return candidate.startUs > incumbent.startUs;
    }

    // Names the handshake's peer and, when the same lookup produced that peer's address,
    // hands the discovery span to this record. Both stay unset when the address was never
    // resolved while listening, e.g. it was already cached: no duration is better than one
    // belonging to a different handshake.
    void RecordPeerIdentityAndDiscovery(PychipCASEHandshakeMetricsRecord & record, const chip::Transport::PeerAddress & address)
    {
        // More than one lookup can hold the same address. A device commissioned again under a new
        // node id keeps the address it had, so an earlier one's span sits in the table beside this
        // one's. Taking whichever is found first would name this handshake's peer with the earlier
        // node id and, that span having been claimed already, report no discovery at all, so choose
        // between them rather than stopping at the first.
        PendingDeviceDiscovery * closest = nullptr;
        for (auto & pending : mPendingDiscoveries)
        {
            if (!pending.hasAddress || !IsSamePeerEndpoint(pending.address, address))
            {
                continue;
            }
            if (closest == nullptr || IsCloserDiscoveryMatch(pending, *closest))
            {
                closest = &pending;
            }
        }
        VerifyOrReturn(closest != nullptr);

        record.peerNodeId = closest->peer.GetNodeId();
        // Claimed so a later handshake to the same peer waits for its own lookup rather
        // than reusing this span.
        if (closest->done && !closest->used)
        {
            closest->used                        = true;
            record.discoveryStartedTimestampUs   = closest->startUs;
            record.discoveryCompletedTimestampUs = closest->doneUs;
            record.recordedFields.Set(CASEHandshakeRecordedField::kDeviceDiscovery);
        }
    }

    // Returns the slot tracking this peer, allocating or evicting one as needed. Never null,
    // so a lookup is always recorded.
    PendingDeviceDiscovery * FindOrCreateDiscoveryForPeer(const chip::PeerId & peer)
    {
        if (PendingDeviceDiscovery * existing = FindDiscoveryForPeer(peer))
        {
            return existing;
        }
        for (auto & pending : mPendingDiscoveries)
        {
            if (!pending.inUse)
            {
                pending       = PendingDeviceDiscovery{};
                pending.inUse = true;
                pending.peer  = peer;
                return &pending;
            }
        }

        // All slots taken, so one has to go. Prefer a slot a handshake already took its span
        // from, since only the address mapping is lost.
        PendingDeviceDiscovery * victim = nullptr;
        for (auto & pending : mPendingDiscoveries)
        {
            if (pending.used)
            {
                victim = &pending;
                break;
            }
        }

        // Otherwise evict the slot that has been waiting longest. A lookup that never resolved
        // holds its slot forever, so without this a run of unreachable peers would starve every
        // later handshake of its discovery time and its peer id.
        if (victim == nullptr)
        {
            for (auto & pending : mPendingDiscoveries)
            {
                if (victim == nullptr || pending.startUs < victim->startUs)
                {
                    victim = &pending;
                }
            }
        }

        *victim       = PendingDeviceDiscovery{};
        victim->inUse = true;
        victim->peer  = peer;
        return victim;
    }

    // Writes off handshakes that have been in flight too long to still be live, freeing their
    // slots and counting them. Without this a handshake that never got a reply would hold its slot
    // for the rest of the run and never be counted, which is the opposite of what the abandoned
    // count is documented to mean.
    void ExpireStaleCASEHandshakes(uint64_t now)
    {
        for (auto & candidate : mInFlightCASEHandshakes)
        {
            if (IsSlotFree(candidate) || now - candidate.sigma1SentTimestampUs < kStaleCASEHandshakeAgeUs)
            {
                continue;
            }
            candidate = PychipCASEHandshakeMetricsRecord{};
            mAbandonedCASEHandshakeCount++;
        }
    }

    void BeginCASEHandshakeRecord(uint64_t now, uint16_t exchangeId, chip::NodeId localEphemeralNodeId)
    {
        // Reclaim anything that has timed out, before deciding a slot has to be taken from a
        // handshake that may still be live.
        ExpireStaleCASEHandshakes(now);

        PychipCASEHandshakeMetricsRecord * slot = nullptr;
        for (auto & candidate : mInFlightCASEHandshakes)
        {
            if (IsSlotFree(candidate))
            {
                slot = &candidate;
                break;
            }
        }

        if (slot == nullptr)
        {
            // Every slot is occupied. A handshake that never concludes, typically one that timed
            // out with no reply, would otherwise hold its slot for the rest of the run, so give up
            // whichever has been waiting longest and count it as abandoned.
            for (auto & candidate : mInFlightCASEHandshakes)
            {
                if (slot == nullptr || candidate.sigma1SentTimestampUs < slot->sigma1SentTimestampUs)
                {
                    slot = &candidate;
                }
            }
            mAbandonedCASEHandshakeCount++;
        }

        *slot                       = PychipCASEHandshakeMetricsRecord{};
        slot->sigma1SentTimestampUs = now;
        slot->exchangeId            = exchangeId;
        slot->localEphemeralNodeId  = localEphemeralNodeId;
        slot->recordedFields.Set(CASEHandshakeRecordedField::kSigma1Sent);

        // Discovery is not attached here: which peer this handshake is for is still unknown. It is
        // attached once the peer replies and its address identifies the lookup.
    }

    // Ignored if the field was already set: an MRP retransmission or a duplicate delivery
    // must not move the timestamp of the first occurrence. Returns true only on the transition,
    // so a caller can act once on a handshake reaching a given state.
    static bool RecordTimestamp(PychipCASEHandshakeMetricsRecord & record, uint64_t PychipCASEHandshakeMetricsRecord::*field,
                                CASEHandshakeRecordedField bit, uint64_t now)
    {
        VerifyOrReturnValue(!record.recordedFields.Has(bit), false);
        record.*field = now;
        record.recordedFields.Set(bit);
        return true;
    }

    // A StatusReport body is generalCode (uint16), protocolId (uint32), protocolCode (uint16), all
    // little endian, per StatusReport::Parse. The tracing hooks hand over the payload with the
    // packet and payload headers already settled, so it starts at generalCode. Offsets are derived
    // from the field widths rather than written out, so they cannot drift apart.
    static constexpr size_t kStatusReportGeneralCodeByteOffset  = 0;
    static constexpr size_t kStatusReportProtocolIdByteOffset   = kStatusReportGeneralCodeByteOffset + sizeof(uint16_t);
    static constexpr size_t kStatusReportProtocolCodeByteOffset = kStatusReportProtocolIdByteOffset + sizeof(uint32_t);
    static constexpr size_t kStatusReportMinimumSizeBytes       = kStatusReportProtocolCodeByteOffset + sizeof(uint16_t);

    static bool ParseStatusReportCodes(const chip::ByteSpan & payload, uint16_t & generalCode, uint16_t & protocolCode)
    {
        VerifyOrReturnValue(payload.size() >= kStatusReportMinimumSizeBytes, false);
        generalCode  = chip::Encoding::LittleEndian::Get16(payload.data() + kStatusReportGeneralCodeByteOffset);
        protocolCode = chip::Encoding::LittleEndian::Get16(payload.data() + kStatusReportProtocolCodeByteOffset);
        return true;
    }

    static bool IsStatusReportSuccess(uint16_t generalCode, uint16_t protocolCode)
    {
        return generalCode == chip::to_underlying(chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess) &&
            protocolCode == chip::Protocols::SecureChannel::kProtocolCodeSuccess;
    }

    // Scratch space for handshakes in progress. Fixed and small, because it holds only what is
    // currently overlapping rather than a history of everything that has been run.
    PychipCASEHandshakeMetricsRecord mInFlightCASEHandshakes[kCASEHandshakeMetricsMaxInFlight]{};
    uint32_t mAbandonedCASEHandshakeCount = 0;
    PendingDeviceDiscovery mPendingDiscoveries[kMaxConcurrentDeviceDiscoveries]{};
};

// Registering the backend is what starts handshakes being timed. Clears anything left from a
// previous run, so a fresh start never reports a stale handshake.
void StartCASEHandshakeMetricsBackend();

// Stops timing. Handshakes still in flight are simply forgotten.
void StopCASEHandshakeMetricsBackend();

// Handshakes that began but never reached a conclusion, so no listener heard about them.
uint32_t AbandonedCASEHandshakeCount();

} // namespace python
} // namespace chip
