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

#include <algorithm>
#include <cstring>
#include <vector>

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

static_assert(chip::Transport::PeerAddress::kMaxToStringSize <= PYCHIP_CASE_TIMING_PEER_ADDR_LEN,
              "PychipCaseTimingRecord::peerAddress is too small to hold a rendered PeerAddress.");

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
constexpr size_t kStatusReportGeneralCodeOffset  = 0;
constexpr size_t kStatusReportProtocolIdOffset   = kStatusReportGeneralCodeOffset + sizeof(uint16_t);
constexpr size_t kStatusReportProtocolCodeOffset = kStatusReportProtocolIdOffset + sizeof(uint32_t);
constexpr size_t kStatusReportMinSize            = kStatusReportProtocolCodeOffset + sizeof(uint16_t);

bool ParseStatusCodes(const chip::ByteSpan & payload, uint16_t & generalCode, uint16_t & protocolCode)
{
    VerifyOrReturnValue(payload.size() >= kStatusReportMinSize, false);
    generalCode  = chip::Encoding::LittleEndian::Get16(payload.data() + kStatusReportGeneralCodeOffset);
    protocolCode = chip::Encoding::LittleEndian::Get16(payload.data() + kStatusReportProtocolCodeOffset);
    return true;
}

bool IsStatusSuccess(uint16_t generalCode, uint16_t protocolCode)
{
    return generalCode == chip::to_underlying(chip::Protocols::SecureChannel::GeneralStatusCode::kSuccess) &&
        protocolCode == chip::Protocols::SecureChannel::kProtocolCodeSuccess;
}

// Times the initiator side of CASE by observing the handshake messages as the transport
// layer sends and receives them. The tracing hooks in SessionManager are used rather than
// the received-message observer above because they report both directions: the handshake
// runs over an unauthenticated session, so every Sigma message reaches
// LogMessageSend/LogMessageReceived with its payload header intact.
class CaseTimingBackend : public chip::Tracing::Backend
{
public:
    void LogMessageSend(chip::Tracing::MessageSendInfo & info) override
    {
        VerifyOrReturn(info.payloadHeader != nullptr && info.packetHeader != nullptr);
        VerifyOrReturn(info.payloadHeader->HasProtocol(chip::Protocols::SecureChannel::Id));

        using chip::Protocols::SecureChannel::MsgType;
        const auto opcode  = static_cast<MsgType>(info.payloadHeader->GetMessageType());
        const uint64_t now = NowUs();

        // On the way out this node is the source; on the way in it is the destination.
        const uint16_t exchangeId = info.payloadHeader->GetExchangeID();
        const uint64_t localId    = NodeIdOrUndefined(info.packetHeader->GetSourceNodeId());

        if (opcode == MsgType::CASE_Sigma1)
        {
            // A Sigma1 leaving the node begins a new handshake.
            StartRecord(now, exchangeId, localId);
            return;
        }

        PychipCaseTimingRecord * record = FindRecord(exchangeId, localId);
        VerifyOrReturn(record != nullptr);

        if (opcode == MsgType::CASE_Sigma3)
        {
            Mark(*record, &PychipCaseTimingRecord::sigma3SentUs, PYCHIP_CASE_TIMING_MARK_SIGMA3_SENT, now);
        }
        else if (opcode == MsgType::StatusReport)
        {
            // A failure report leaving this node means this node rejected the peer, which is
            // the one error signal not visible on the inbound side. An outbound success
            // report is normal on the resumption path and is not an error.
            uint16_t generalCode  = 0;
            uint16_t protocolCode = 0;
            VerifyOrReturn(ParseStatusCodes(info.payload, generalCode, protocolCode));
            if (!IsStatusSuccess(generalCode, protocolCode))
            {
                record->marks = static_cast<uint8_t>(record->marks | PYCHIP_CASE_TIMING_MARK_LOCAL_FAILURE);
            }
        }
    }

    void LogMessageReceived(chip::Tracing::MessageReceivedInfo & info) override
    {
        VerifyOrReturn(info.payloadHeader != nullptr && info.packetHeader != nullptr);
        VerifyOrReturn(info.payloadHeader->HasProtocol(chip::Protocols::SecureChannel::Id));

        using chip::Protocols::SecureChannel::MsgType;
        const auto opcode  = static_cast<MsgType>(info.payloadHeader->GetMessageType());
        const uint64_t now = NowUs();

        PychipCaseTimingRecord * record =
            FindRecord(info.payloadHeader->GetExchangeID(), NodeIdOrUndefined(info.packetHeader->GetDestinationNodeId()));
        VerifyOrReturn(record != nullptr);

        // The peer is not known when Sigma1 goes out, so identity is learned from the first
        // message it sends back.
        if (record->peerAddress[0] == kPeerAddressUnknown && info.peerAddress != nullptr)
        {
            info.peerAddress->ToString(record->peerAddress, sizeof(record->peerAddress));
            record->peerNodeId = NodeIdForAddress(*info.peerAddress);
        }

        switch (opcode)
        {
        case MsgType::CASE_Sigma2:
            Mark(*record, &PychipCaseTimingRecord::sigma2ReceivedUs, PYCHIP_CASE_TIMING_MARK_SIGMA2_RECEIVED, now);
            break;
        case MsgType::CASE_Sigma2Resume:
            Mark(*record, &PychipCaseTimingRecord::sigma2ResumeReceivedUs, PYCHIP_CASE_TIMING_MARK_SIGMA2_RESUME_RECEIVED, now);
            break;
        case MsgType::StatusReport:
            // Only the report closing out Sigma3 is of interest. A StatusReport arriving at
            // any other point belongs to an exchange this record is not timing, and is left
            // unmarked so the Python side sees an incomplete record.
            if ((record->marks & PYCHIP_CASE_TIMING_MARK_SIGMA3_SENT) != 0)
            {
                Mark(*record, &PychipCaseTimingRecord::statusReportReceivedUs, PYCHIP_CASE_TIMING_MARK_STATUS_REPORT_RECEIVED,
                     now);

                // The timestamp is kept whether the peer accepted or rejected Sigma3, since
                // the time to a rejection is still a real measurement. The codes are what
                // tell the two apart.
                uint16_t generalCode  = 0;
                uint16_t protocolCode = 0;
                if ((record->marks & PYCHIP_CASE_TIMING_MARK_STATUS_PARSED) == 0 &&
                    ParseStatusCodes(info.payload, generalCode, protocolCode))
                {
                    record->statusGeneralCode  = generalCode;
                    record->statusProtocolCode = protocolCode;
                    record->marks              = static_cast<uint8_t>(record->marks | PYCHIP_CASE_TIMING_MARK_STATUS_PARSED);
                }
            }
            break;
        default:
            break;
        }
    }

    // Operational discovery runs to completion before Sigma1 goes out, so there is no record
    // to mark yet. Spans are held here, keyed by peer so that concurrent lookups do not
    // overwrite each other, and attached by StartRecord.
    void LogNodeLookup(chip::Tracing::NodeLookupInfo & info) override
    {
        VerifyOrReturn(info.request != nullptr);
        PendingDiscovery * pending = FindOrAddPending(info.request->GetPeerId());
        VerifyOrReturn(pending != nullptr);
        pending->startUs = NowUs();
        pending->done    = false;
    }

    void LogNodeDiscovered(chip::Tracing::NodeDiscoveredInfo & info) override
    {
        // Intermediate results and retries also arrive here; only completion ends the span.
        VerifyOrReturn(info.type == chip::Tracing::DiscoveryInfoType::kResolutionDone);
        VerifyOrReturn(info.peerId != nullptr);
        PendingDiscovery * pending = FindPending(*info.peerId);
        VerifyOrReturn(pending != nullptr && pending->startUs != kPendingSlotFree);
        pending->doneUs = NowUs();
        pending->done   = true;
        // Remember which address belongs to which node, so a handshake can name its peer
        // from the address its replies arrive from.
        if (info.result != nullptr)
        {
            pending->address    = info.result->address;
            pending->hasAddress = true;
        }
    }

    void Reset(uint32_t maxRecords)
    {
        mRecords.assign(maxRecords, PychipCaseTimingRecord{});
        ClearCounters();
    }

    // Clears the captured records but keeps whatever capacity the last start asked for.
    void ResetKeepingCapacity()
    {
        std::fill(mRecords.begin(), mRecords.end(), PychipCaseTimingRecord{});
        ClearCounters();
    }

    // Copies only the records actually captured, so a large capacity costs nothing to read.
    void CopyRecords(PychipCaseTimingRecord * out, uint32_t capacity, uint32_t & written, uint32_t & available,
                     uint32_t & dropped) const
    {
        available = mCount;
        dropped   = mDropped;
        written   = std::min(capacity, mCount);
        if (out != nullptr && written > 0)
        {
            std::memcpy(out, mRecords.data(), written * sizeof(PychipCaseTimingRecord));
        }
    }

private:
    void ClearCounters()
    {
        mCount   = 0;
        mDropped = 0;
        for (auto & pending : mPending)
        {
            pending = PendingDiscovery{};
        }
    }

    // An operational discovery span waiting to be attached to the handshake that follows it.
    struct PendingDiscovery
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
    static constexpr size_t kPendingDiscoveryMax = 8;

    // A slot is free when no lookup has stamped a start time into it.
    static constexpr uint64_t kPendingSlotFree = 0;

    // peerAddress holds a C string, so an empty first byte means the peer has not been seen yet.
    static constexpr char kPeerAddressUnknown = '\0';

    static uint64_t NowUs() { return chip::System::SystemClock().GetMonotonicMicroseconds64().count(); }

    // A CASE packet header carries a node id in only one direction, so the absent case is
    // mapped to kUndefinedNodeId and takes part in record matching like any other value.
    static chip::NodeId NodeIdOrUndefined(const chip::Optional<chip::NodeId> & nodeId)
    {
        return nodeId.HasValue() ? nodeId.Value() : chip::kUndefinedNodeId;
    }

    // Locates the handshake a message belongs to. Searching newest first means a reused
    // exchange id resolves to the current handshake rather than a finished one.
    PychipCaseTimingRecord * FindRecord(uint16_t exchangeId, chip::NodeId localNodeId)
    {
        for (uint32_t i = mCount; i > 0; i--)
        {
            PychipCaseTimingRecord & record = mRecords[i - 1];
            if (record.exchangeId == exchangeId && record.localNodeId == localNodeId)
            {
                return &record;
            }
        }
        return nullptr;
    }

    PendingDiscovery * FindPending(const chip::PeerId & peer)
    {
        for (auto & pending : mPending)
        {
            if (pending.startUs != kPendingSlotFree && pending.peer == peer)
            {
                return &pending;
            }
        }
        return nullptr;
    }

    // Maps a peer's transport address back to the node id that discovery resolved it from.
    // kUndefinedNodeId when the address was never discovered in this capture, e.g. it was cached.
    chip::NodeId NodeIdForAddress(const chip::Transport::PeerAddress & address) const
    {
        for (const auto & pending : mPending)
        {
            if (pending.hasAddress && pending.address == address)
            {
                return pending.peer.GetNodeId();
            }
        }
        return chip::kUndefinedNodeId;
    }

    PendingDiscovery * FindOrAddPending(const chip::PeerId & peer)
    {
        if (PendingDiscovery * existing = FindPending(peer))
        {
            return existing;
        }
        for (auto & pending : mPending)
        {
            if (pending.startUs == kPendingSlotFree)
            {
                pending      = PendingDiscovery{};
                pending.peer = peer;
                return &pending;
            }
        }
        // All slots taken. Recycle the one already claimed by a handshake, since its span
        // has been copied into that record; its address mapping is the only loss.
        for (auto & pending : mPending)
        {
            if (pending.used)
            {
                pending      = PendingDiscovery{};
                pending.peer = peer;
                return &pending;
            }
        }
        return nullptr; // Every slot is an in-flight lookup; this one goes unrecorded.
    }

    // Claims the oldest finished discovery. The peer a handshake is for is not known when
    // Sigma1 goes out, so under concurrency this pairs by completion order rather than by
    // peer. Exact for one handshake at a time.
    PendingDiscovery * ClaimOldestDiscovery()
    {
        PendingDiscovery * oldest = nullptr;
        for (auto & pending : mPending)
        {
            if (pending.done && !pending.used && (oldest == nullptr || pending.doneUs < oldest->doneUs))
            {
                oldest = &pending;
            }
        }
        return oldest;
    }

    void StartRecord(uint64_t now, uint16_t exchangeId, chip::NodeId localNodeId)
    {
        if (mCount >= mRecords.size())
        {
            mDropped++;
            return;
        }
        PychipCaseTimingRecord & record = mRecords[mCount];
        record                          = PychipCaseTimingRecord{};
        record.sigma1SentUs             = now;
        record.exchangeId               = exchangeId;
        record.localNodeId              = localNodeId;
        record.marks                    = PYCHIP_CASE_TIMING_MARK_SIGMA1_SENT;

        // Attach the discovery that preceded this handshake, if there was one. Consumed
        // rather than copied, so a later handshake that reused a cached address reports no
        // discovery instead of repeating these timestamps.
        if (PendingDiscovery * pending = ClaimOldestDiscovery())
        {
            pending->used           = true;
            record.discoveryStartUs = pending->startUs;
            record.discoveryDoneUs  = pending->doneUs;
            record.marks            = static_cast<uint8_t>(record.marks | PYCHIP_CASE_TIMING_MARK_DISCOVERY);
        }

        mCount++;
    }

    // Ignored if the field was already set: an MRP retransmission or a duplicate delivery
    // must not move the timestamp of the first occurrence.
    static void Mark(PychipCaseTimingRecord & record, uint64_t PychipCaseTimingRecord::* field, uint8_t bit, uint64_t now)
    {
        VerifyOrReturn((record.marks & bit) == 0);
        record.*field = now;
        record.marks  = static_cast<uint8_t>(record.marks | bit);
    }

    // Heap-allocated so the capture size is the caller's choice rather than a compile-time
    // constant. Acceptable here because this file is host-test-only, per the static_assert
    // above; it is not part of any device build.
    std::vector<PychipCaseTimingRecord> mRecords;
    uint32_t mCount   = 0;
    uint32_t mDropped = 0;
    PendingDiscovery mPending[kPendingDiscoveryMax]{};
};

CaseTimingBackend gTimingBackend;
bool gTimingRegistered = false;

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

PyChipError pychip_case_timing_start(uint32_t maxRecords)
{
    const uint32_t capacity = (maxRecords == 0) ? PYCHIP_CASE_TIMING_DEFAULT_MAX_RECORDS : maxRecords;
    chip::MainLoopWork::ExecuteInMainLoop([capacity] {
        gTimingBackend.Reset(capacity);
        if (!gTimingRegistered)
        {
            chip::Tracing::Register(gTimingBackend);
            gTimingRegistered = true;
        }
    });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_timing_stop(void)
{
    chip::MainLoopWork::ExecuteInMainLoop([] {
        if (gTimingRegistered)
        {
            chip::Tracing::Unregister(gTimingBackend);
            gTimingRegistered = false;
        }
    });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_timing_reset(void)
{
    chip::MainLoopWork::ExecuteInMainLoop([] { gTimingBackend.ResetKeepingCapacity(); });
    return ToPyChipError(CHIP_NO_ERROR);
}

PyChipError pychip_case_timing_get_records(PychipCaseTimingRecord * out, uint32_t capacity, uint32_t * written,
                                           uint32_t * available, uint32_t * dropped)
{
    VerifyOrReturnError(written != nullptr && available != nullptr && dropped != nullptr,
                        ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));
    VerifyOrReturnError(out != nullptr || capacity == 0, ToPyChipError(CHIP_ERROR_INVALID_ARGUMENT));

    uint32_t writtenCount   = 0;
    uint32_t availableCount = 0;
    uint32_t droppedCount   = 0;
    chip::MainLoopWork::ExecuteInMainLoop(
        [&] { gTimingBackend.CopyRecords(out, capacity, writtenCount, availableCount, droppedCount); });

    *written   = writtenCount;
    *available = availableCount;
    *dropped   = droppedCount;
    return ToPyChipError(CHIP_NO_ERROR);
}

} // extern "C"
