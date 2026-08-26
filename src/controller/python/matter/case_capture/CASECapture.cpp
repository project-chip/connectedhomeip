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

#include <controller/python/matter/case_capture/CASECapture.h>

#include <condition_variable>
#include <cstring>
#include <deque>
#include <mutex>

#include <controller/python/matter/native/ChipMainLoopWork.h>
#include <lib/address_resolve/TracingStructs.h>
#include <lib/core/CHIPConfig.h>
#include <lib/core/CHIPEncoding.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <protocols/secure_channel/StatusReport.h>
#include <system/SystemClock.h>
#include <system/SystemPacketBuffer.h>
#include <tracing/backend.h>
#include <tracing/registry.h>
#include <transport/TracingStructs.h>
#include <transport/raw/MessageHeader.h>

static_assert(CONFIG_BUILD_FOR_HOST_UNIT_TEST,
              "CASECapture.cpp requires CONFIG_BUILD_FOR_HOST_UNIT_TEST; this file is only valid for the Python test build.");

static_assert(chip::Transport::PeerAddress::kMaxToStringSize <= PYCHIP_CASE_HANDSHAKE_METRICS_PEER_ADDRESS_MAX_LENGTH,
              "PychipCASEHandshakeMetricsRecord::peerTransportAddress is too small to hold a rendered PeerAddress.");

namespace {

PychipCaseCapturedHeaders MakeHeaders(const chip::PacketHeader & packetHeader, const chip::PayloadHeader & payloadHeader)
{
    PychipCaseCapturedHeaders headers{};
    headers.sessionType = chip::to_underlying(packetHeader.GetSessionType());
    headers.sFlag       = packetHeader.HasSourceNodeId() ? 1u : 0u;
    headers.dsiz        = packetHeader.HasDestinationGroupId() ? 2u : packetHeader.HasDestinationNodeId() ? 1u : 0u;
    headers.protocolId  = payloadHeader.GetProtocolID().GetProtocolId();
    headers.opcode      = payloadHeader.GetMessageType();
    headers.isInitiator = payloadHeader.IsInitiator() ? 1u : 0u;
    headers.present     = 1u;
    return headers;
}

// Captures inbound Sigma2 / Sigma2_Resume / StatusReport for Python tests.
class CaseReceivedMessageObserver : public chip::Messaging::TestOnlyReceivedMessageObserver
{
public:
    void OnMessageReceived(const chip::PacketHeader & packetHeader, const chip::PayloadHeader & payloadHeader,
                           const chip::System::PacketBufferHandle & msgBuf) override
    {
        if (!payloadHeader.HasProtocol(chip::Protocols::SecureChannel::Id))
        {
            return;
        }

        using chip::Protocols::SecureChannel::MsgType;
        const auto opcode = static_cast<MsgType>(payloadHeader.GetMessageType());

        if (opcode == MsgType::CASE_Sigma2)
        {
            mSigma2 = MakeHeaders(packetHeader, payloadHeader);
            return;
        }

        if (opcode == MsgType::CASE_Sigma2Resume)
        {
            mSigma2Resume = MakeHeaders(packetHeader, payloadHeader);
            return;
        }

        if (opcode == MsgType::StatusReport)
        {
            mStatusReport             = MakeHeaders(packetHeader, payloadHeader);
            mStatusReportParsed       = 0;
            mStatusReportGeneralCode  = 0;
            mStatusReportProtocolId   = 0;
            mStatusReportProtocolCode = 0;
            if (!msgBuf.IsNull())
            {
                // Use Retain() to create a second handle because Parse() consumes the handle; the original must stay valid so
                // ExchangeManager can keep processing the message.
                chip::Protocols::SecureChannel::StatusReport report;
                if (report.Parse(msgBuf.Retain()) == CHIP_NO_ERROR)
                {
                    mStatusReportGeneralCode  = chip::to_underlying(report.GetGeneralCode());
                    mStatusReportProtocolId   = report.GetProtocolId().ToFullyQualifiedSpecForm();
                    mStatusReportProtocolCode = report.GetProtocolCode();
                    mStatusReportParsed       = 1;
                }
            }
        }
    }

    void ResetSlots()
    {
        mSigma2                   = PychipCaseCapturedHeaders{};
        mSigma2Resume             = PychipCaseCapturedHeaders{};
        mStatusReport             = PychipCaseCapturedHeaders{};
        mStatusReportParsed       = 0;
        mStatusReportGeneralCode  = 0;
        mStatusReportProtocolId   = 0;
        mStatusReportProtocolCode = 0;
    }

    void FillSnapshot(PychipCaseCaptureSnapshot & out) const
    {
        out.sigma2                   = mSigma2;
        out.sigma2Resume             = mSigma2Resume;
        out.statusReport             = mStatusReport;
        out.statusReportParsed       = mStatusReportParsed;
        out.statusReportGeneralCode  = mStatusReportGeneralCode;
        out.statusReportProtocolId   = mStatusReportProtocolId;
        out.statusReportProtocolCode = mStatusReportProtocolCode;
    }

private:
    PychipCaseCapturedHeaders mSigma2{};
    PychipCaseCapturedHeaders mSigma2Resume{};
    PychipCaseCapturedHeaders mStatusReport{};
    uint8_t mStatusReportParsed        = 0;
    uint16_t mStatusReportGeneralCode  = 0;
    uint32_t mStatusReportProtocolId   = 0;
    uint16_t mStatusReportProtocolCode = 0;
};

CaseReceivedMessageObserver gCaseObserver;

// A StatusReport body is generalCode (uint16), protocolId (uint32), protocolCode (uint16), all
// little endian, per StatusReport::Parse. The tracing hooks hand over the payload with the
// packet and payload headers already settled, so it starts at generalCode. Offsets are derived
// from the field widths rather than written out, so they cannot drift apart.
constexpr size_t kStatusReportGeneralCodeByteOffset  = 0;
constexpr size_t kStatusReportProtocolIdByteOffset   = kStatusReportGeneralCodeByteOffset + sizeof(uint16_t);
constexpr size_t kStatusReportProtocolCodeByteOffset = kStatusReportProtocolIdByteOffset + sizeof(uint32_t);
constexpr size_t kStatusReportMinimumSizeBytes       = kStatusReportProtocolCodeByteOffset + sizeof(uint16_t);

bool ParseStatusReportCodes(const chip::ByteSpan & payload, uint16_t & generalCode, uint16_t & protocolCode)
{
    VerifyOrReturnValue(payload.size() >= kStatusReportMinimumSizeBytes, false);
    generalCode  = chip::Encoding::LittleEndian::Get16(payload.data() + kStatusReportGeneralCodeByteOffset);
    protocolCode = chip::Encoding::LittleEndian::Get16(payload.data() + kStatusReportProtocolCodeByteOffset);
    return true;
}

bool IsStatusReportSuccess(uint16_t generalCode, uint16_t protocolCode)
{
    return generalCode == chip::to_underlying(chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess) &&
        protocolCode == chip::Protocols::SecureChannel::kProtocolCodeSuccess;
}

// Holds handshakes that have reached a conclusion until a consumer thread collects them.
//
// The producer is the CHIP event loop inside the tracing hooks, so Publish does the least work
// that correctness allows: take a short lock, copy, signal, return. It never enters Python and
// never waits for a consumer, which is what keeps a slow or absent listener from showing up in
// the timings being measured.
class CompletedCASEHandshakeQueue
{
public:
    void Open(uint32_t depth)
    {
        const std::lock_guard<std::mutex> lock(mMutex);
        mDepth = depth;
        mQueue.clear();
        mDroppedCount = 0;
        mIsOpen       = true;
    }

    void Close()
    {
        {
            const std::lock_guard<std::mutex> lock(mMutex);
            mIsOpen = false;
            mQueue.clear();
        }
        // Wake every waiter so each can see notifications were stopped and unwind.
        mRecordAvailable.notify_all();
    }

    // Called on the CHIP event loop thread, from inside the measured path.
    void Publish(const PychipCASEHandshakeMetricsRecord & record)
    {
        {
            const std::lock_guard<std::mutex> lock(mMutex);
            if (!mIsOpen)
            {
                return;
            }
            if (mQueue.size() >= mDepth)
            {
                // Drop the oldest rather than block the handshake or grow without bound. The
                // count is what makes a consumer that cannot keep up visible to the caller.
                mQueue.pop_front();
                mDroppedCount++;
            }
            mQueue.push_back(record);
        }
        mRecordAvailable.notify_one();
    }

    // Called on a consumer thread with the interpreter lock released.
    bool Wait(PychipCASEHandshakeMetricsRecord & out, uint32_t timeoutMs, uint32_t & droppedCount)
    {
        std::unique_lock<std::mutex> lock(mMutex);
        mRecordAvailable.wait_for(lock, std::chrono::milliseconds(timeoutMs), [this] { return !mQueue.empty() || !mIsOpen; });

        droppedCount = mDroppedCount;
        if (mQueue.empty())
        {
            return false; // Timed out, or closed while waiting.
        }
        out = mQueue.front();
        mQueue.pop_front();
        return true;
    }

private:
    std::mutex mMutex;
    std::condition_variable mRecordAvailable;
    std::deque<PychipCASEHandshakeMetricsRecord> mQueue;
    size_t mDepth          = PYCHIP_CASE_HANDSHAKE_METRICS_NOTIFICATION_QUEUE_DEFAULT_DEPTH;
    uint32_t mDroppedCount = 0;
    bool mIsOpen           = false;
};

CompletedCASEHandshakeQueue gCompletedCASEHandshakeQueue;

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
                            PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA3_SENT, now);
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
                const bool alreadyRejectedPeer =
                    (record->recordedFields & PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_THIS_NODE_REJECTED_PEER) != 0;
                record->recordedFields =
                    static_cast<uint8_t>(record->recordedFields | PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_THIS_NODE_REJECTED_PEER);

                // Keep the codes, not just the fact of a rejection. They are already decoded
                // here, and without them the record carries LOCAL_FAILURE with both code
                // fields left at zero - which this struct documents as meaning success. The
                // inbound path owns these fields once it has decoded a report of its own, so
                // only fill them in while they are still unset.
                if ((record->recordedFields & PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_CODES) == 0)
                {
                    record->statusReportGeneralCode  = generalCode;
                    record->statusReportProtocolCode = protocolCode;
                    record->recordedFields =
                        static_cast<uint8_t>(record->recordedFields | PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_CODES);
                }

                // Rejecting the peer ends the handshake from this side too.
                if (!alreadyRejectedPeer)
                {
                    ConcludeCASEHandshake(*record);
                }
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
                            PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA2_RECEIVED, now);
            break;
        case MsgType::CASE_Sigma2Resume:
            // Resumption exchanges no Sigma2 or Sigma3, so this is where a resumed handshake
            // concludes for the initiator.
            if (RecordTimestamp(*record, &PychipCASEHandshakeMetricsRecord::sigma2ResumeReceivedTimestampUs,
                                PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA2_RESUME_RECEIVED, now))
            {
                ConcludeCASEHandshake(*record);
            }
            break;
        case MsgType::StatusReport:
            // Only the report closing out Sigma3 is of interest. A StatusReport arriving at
            // any other point belongs to an exchange this record is not timing, and is left
            // unmarked so the Python side sees an incomplete record.
            if ((record->recordedFields & PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA3_SENT) != 0)
            {
                const bool reachedConclusion =
                    RecordTimestamp(*record, &PychipCASEHandshakeMetricsRecord::statusReportReceivedTimestampUs,
                                    PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_RECEIVED, now);

                // The timestamp is kept whether the peer accepted or rejected Sigma3, since
                // the time to a rejection is still a real measurement. The codes are what
                // tell the two apart.
                uint16_t generalCode  = 0;
                uint16_t protocolCode = 0;
                if ((record->recordedFields & PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_CODES) == 0 &&
                    ParseStatusReportCodes(info.payload, generalCode, protocolCode))
                {
                    record->statusReportGeneralCode  = generalCode;
                    record->statusReportProtocolCode = protocolCode;
                    record->recordedFields =
                        static_cast<uint8_t>(record->recordedFields | PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_STATUS_REPORT_CODES);
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
        VerifyOrReturn(pending != nullptr && pending->startUs != kDiscoverySlotUnused);
        pending->doneUs = CurrentTimestampUs();
        pending->done   = true;
        // Remember which address belongs to which node, so a handshake can name its peer
        // from the address its replies arrive from.
        if (info.result != nullptr)
        {
            pending->address    = info.result->address;
            pending->hasAddress = true;
        }
    }

    void Reset() { ClearRecordsAndDiscoveries(); }

    uint32_t AbandonedCASEHandshakeCount() const { return mAbandonedCASEHandshakeCount; }

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

    // A slot is free when no lookup has stamped a start time into it.
    static constexpr uint64_t kDiscoverySlotUnused = 0;

    // peerAddress holds a C string, so an empty first byte means the peer has not been seen yet.
    static constexpr char kPeerAddressNotYetKnown = '\0';

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
    static bool IsSlotFree(const PychipCASEHandshakeMetricsRecord & record) { return record.recordedFields == 0; }

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
        gCompletedCASEHandshakeQueue.Publish(record);
        record = PychipCASEHandshakeMetricsRecord{};
    }

    PendingDeviceDiscovery * FindDiscoveryForPeer(const chip::PeerId & peer)
    {
        for (auto & pending : mPendingDiscoveries)
        {
            if (pending.startUs != kDiscoverySlotUnused && pending.peer == peer)
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
            record.recordedFields =
                static_cast<uint8_t>(record.recordedFields | PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_DEVICE_DISCOVERY);
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
            if (pending.startUs == kDiscoverySlotUnused)
            {
                pending      = PendingDeviceDiscovery{};
                pending.peer = peer;
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

        *victim      = PendingDeviceDiscovery{};
        victim->peer = peer;
        return victim;
    }

    void BeginCASEHandshakeRecord(uint64_t now, uint16_t exchangeId, chip::NodeId localEphemeralNodeId)
    {
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
        slot->recordedFields        = PYCHIP_CASE_HANDSHAKE_METRICS_RECORDED_SIGMA1_SENT;

        // Discovery is not attached here: which peer this handshake is for is still unknown. It is
        // attached once the peer replies and its address identifies the lookup.
    }

    // Ignored if the field was already set: an MRP retransmission or a duplicate delivery
    // must not move the timestamp of the first occurrence. Returns true only on the transition,
    // so a caller can act once on a handshake reaching a given state.
    static bool RecordTimestamp(PychipCASEHandshakeMetricsRecord & record, uint64_t PychipCASEHandshakeMetricsRecord::*field,
                                uint8_t bit, uint64_t now)
    {
        VerifyOrReturnValue((record.recordedFields & bit) == 0, false);
        record.*field         = now;
        record.recordedFields = static_cast<uint8_t>(record.recordedFields | bit);
        return true;
    }

    // Scratch space for handshakes in progress. Fixed and small, because it holds only what is
    // currently overlapping rather than a history of everything that has been run.
    PychipCASEHandshakeMetricsRecord mInFlightCASEHandshakes[PYCHIP_CASE_HANDSHAKE_METRICS_MAX_IN_FLIGHT]{};
    uint32_t mAbandonedCASEHandshakeCount = 0;
    PendingDeviceDiscovery mPendingDiscoveries[kMaxConcurrentDeviceDiscoveries]{};
};

CASEHandshakeMetricsBackend gCASEHandshakeMetricsBackend;
bool gCASEHandshakeMetricsBackendRegistered = false;

} // namespace

extern "C" {

PyChipError pychip_case_capture_set_observer(chip::Controller::DeviceCommissioner * devCtrl)
{
    VerifyOrReturnError(devCtrl != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    chip::MainLoopWork::ExecuteInMainLoop([devCtrl] {
        if (devCtrl->ExchangeMgr()->GetTestOnlyReceivedMessageObserver() != &gCaseObserver)
        {
            devCtrl->ExchangeMgr()->SetTestOnlyReceivedMessageObserver(&gCaseObserver);
        }
    });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_capture_reset(void)
{
    chip::MainLoopWork::ExecuteInMainLoop([] { gCaseObserver.ResetSlots(); });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_capture_get_snapshot(PychipCaseCaptureSnapshot * out)
{
    VerifyOrReturnError(out != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    std::memset(out, 0, sizeof(*out));
    chip::MainLoopWork::ExecuteInMainLoop([out] { gCaseObserver.FillSnapshot(*out); });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_start_notifications(uint32_t depth)
{
    VerifyOrReturnError(depth <= PYCHIP_CASE_HANDSHAKE_METRICS_NOTIFICATION_QUEUE_MAX_DEPTH,
                        ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    // The notification queue carries its own lock, so it is opened on the calling thread. Registering the
    // backend does go through the event loop, because that is where the tracing registry lives.
    gCompletedCASEHandshakeQueue.Open(depth == 0 ? PYCHIP_CASE_HANDSHAKE_METRICS_NOTIFICATION_QUEUE_DEFAULT_DEPTH : depth);
    chip::MainLoopWork::ExecuteInMainLoop([] {
        gCASEHandshakeMetricsBackend.Reset();
        if (!gCASEHandshakeMetricsBackendRegistered)
        {
            chip::Tracing::Register(gCASEHandshakeMetricsBackend);
            gCASEHandshakeMetricsBackendRegistered = true;
        }
    });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_stop_notifications(void)
{
    gCompletedCASEHandshakeQueue.Close();
    chip::MainLoopWork::ExecuteInMainLoop([] {
        if (gCASEHandshakeMetricsBackendRegistered)
        {
            chip::Tracing::Unregister(gCASEHandshakeMetricsBackend);
            gCASEHandshakeMetricsBackendRegistered = false;
        }
    });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_wait_for_notification(PychipCASEHandshakeMetricsRecord * out, uint32_t timeoutMs,
                                                                uint8_t * received, uint32_t * dropped)
{
    VerifyOrReturnError(out != nullptr && received != nullptr && dropped != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    std::memset(out, 0, sizeof(*out));
    uint32_t droppedCount = 0;
    const bool gotRecord  = gCompletedCASEHandshakeQueue.Wait(*out, timeoutMs, droppedCount);

    *received = gotRecord ? 1u : 0u;
    *dropped  = droppedCount;
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_handshake_metrics_get_abandoned_count(uint32_t * abandoned)
{
    VerifyOrReturnError(abandoned != nullptr, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    uint32_t count = 0;
    chip::MainLoopWork::ExecuteInMainLoop([&] { count = gCASEHandshakeMetricsBackend.AbandonedCASEHandshakeCount(); });
    *abandoned = count;
    return ToPyChipError(CHIP_NO_ERROR);
}

} // extern "C"
