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

/**
 *    @file
 *      Stateful FuzzTest harness for the BDX TransferSession state machine.
 *
 *      The existing TestBdxTransferSession unit tests drive two cooperating
 *      sessions through the happy path (and a handful of hand-crafted
 *      Accept/Query messages). No fuzzer drives the state machine itself: the
 *      6 BdxMessages parsers were fuzzed in isolation, but the ordering /
 *      value-range space of HandleMessageReceived -> Handle{TransferInit,
 *      ReceiveAccept,SendAccept,Block,BlockEOF,BlockQuery,BlockQueryWithSkip,
 *      BlockAck,BlockAckEOF,StatusReport} across every TransferState was not.
 *
 *      This harness stands up ONE TransferSession, bootstrapped as
 *      either an initiator (StartTransfer) or a responder (WaitForTransfer),
 *      then feeds it a fuzzer-chosen sequence of steps. Each step either injects
 *      a wire message (built from the real BdxMessages structs, so it parses and
 *      reaches the handler bodies) or invokes a local producer API
 *      (AcceptTransfer / PrepareBlock / ...). Counter-bearing messages can carry
 *      either the session's currently-expected counter (mirrored from the public
 *      getters, so the mutator gets past the counter gate into the deep
 *      block-processing states) or a fuzzer-chosen counter (to exercise the
 *      reject paths). PollOutput is drained after every step.
 *
 *      Property: no ordering / value of BDX or StatusReport messages may crash
 *      the session (ASan/UBSan). The harness asserts nothing about the outputs.
 */

#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <protocols/Protocols.h>
#include <protocols/bdx/BdxMessages.h>
#include <protocols/bdx/BdxTransferSession.h>
#include <protocols/bdx/StatusCode.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemClock.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

namespace {

using namespace chip;
using namespace chip::bdx;
using namespace fuzztest;

using chip::System::PacketBufferHandle;

constexpr System::Clock::Timestamp kNoAdvanceTime = System::Clock::kZero;

// Upper bound on the number of steps per sequence (template arg to pw::fuzzer::VectorOf).
constexpr size_t kMaxSteps = 24;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        return true;
    }();
    (void) sInitialized;
}

// Serialize a BdxMessages struct into a PacketBuffer, matching the injection idiom
// used by TestBdxTransferSession. MessageSize() and WriteToBuffer() share a code
// path, so the allocated buffer is always exactly the needed size.
template <typename MsgType>
PacketBufferHandle EncodeBdxMessage(const MsgType & msg)
{
    const size_t msgSize = msg.MessageSize();
    Encoding::LittleEndian::PacketBufferWriter writer(PacketBufferHandle::New(msgSize), msgSize);
    if (writer.IsNull())
    {
        return PacketBufferHandle();
    }
    msg.WriteToBuffer(writer);
    return writer.Finalize();
}

// Wrap a payload in a PayloadHeader and feed it to the session, exactly as the
// exchange layer would once it has stripped the transport header.
void Inject(TransferSession & session, Protocols::Id protocol, uint8_t msgType, PacketBufferHandle buf)
{
    if (buf.IsNull())
    {
        return;
    }
    PayloadHeader header;
    header.SetMessageType(protocol, msgType);
    RETURN_SAFELY_IGNORED session.HandleMessageReceived(header, std::move(buf), kNoAdvanceTime);
}

// Build a PacketBuffer from raw bytes. An empty payload maps to an empty
// buffer via New(0) rather than NewWithData(payload.data(), 0), since an empty
// std::vector's data() may be null.
PacketBufferHandle MakeBuf(const std::vector<uint8_t> & payload)
{
    if (payload.empty())
    {
        return PacketBufferHandle::New(0);
    }
    return PacketBufferHandle::NewWithData(payload.data(), payload.size());
}

void DrainOutput(TransferSession & session)
{
    for (int i = 0; i < 8; ++i)
    {
        TransferSession::OutputEvent event;
        session.PollOutput(event, kNoAdvanceTime);
        if (event.EventType == TransferSession::OutputEventType::kNone)
        {
            break;
        }
    }
}

// Steps. The low bits of `opcode` select the action; the high bit selects whether
// counter-bearing messages carry the mirrored (expected) counter or the fuzzed one.
enum class Op : uint8_t
{
    kInjectInit = 0,
    kInjectReceiveAccept,
    kInjectSendAccept,
    kInjectBlockQuery,
    kInjectBlockQueryWithSkip,
    kInjectBlock,
    kInjectBlockEOF,
    kInjectBlockAck,
    kInjectBlockAckEOF,
    kInjectRawStatusReport,
    kInjectRawBdx,
    kLocalAccept,
    kLocalReject,
    kLocalPrepareBlockQuery,
    kLocalPrepareBlockQueryWithSkip,
    kLocalPrepareBlock,
    kLocalPrepareBlockAck,
    kLocalAbort,
    kDrain,
    kOpCount,
};

// Per-step fuzzed fields: (opcode, u16, u32, u64, payload).
using Step = std::tuple<uint8_t, uint16_t, uint32_t, uint64_t, std::vector<uint8_t>>;

const TransferControlFlags kDriveModes[] = { TransferControlFlags::kSenderDrive, TransferControlFlags::kReceiverDrive,
                                             TransferControlFlags::kAsync };

StatusCode PickStatusCode(uint32_t v)
{
    const StatusCode codes[] = { StatusCode::kLengthTooLarge,
                                 StatusCode::kLengthMismatch,
                                 StatusCode::kBadMessageContents,
                                 StatusCode::kBadBlockCounter,
                                 StatusCode::kUnexpectedMessage,
                                 StatusCode::kTransferFailedUnknownError,
                                 StatusCode::kTransferMethodNotSupported,
                                 StatusCode::kFileDesignatorUnknown,
                                 StatusCode::kStartOffsetNotSupported,
                                 StatusCode::kVersionNotSupported,
                                 StatusCode::kUnknown };
    return codes[v % (sizeof(codes) / sizeof(codes[0]))];
}

void HandleStep(TransferSession & session, const Step & step)
{
    const uint8_t rawOp                  = std::get<0>(step);
    const uint16_t u16                   = std::get<1>(step);
    const uint32_t u32                   = std::get<2>(step);
    const uint64_t u64                   = std::get<3>(step);
    const std::vector<uint8_t> & payload = std::get<4>(step);

    const Op op = static_cast<Op>(rawOp % static_cast<uint8_t>(Op::kOpCount));
    // Derive the mirror flag from a value bit (NOT from rawOp: `rawOp % kOpCount`
    // consumes the whole byte, so a high bit there would perturb the opcode).
    const bool useMirror = (u32 & 0x1u) != 0u;

    switch (op)
    {
    case Op::kInjectInit: {
        TransferInit msg;
        msg.TransferCtlOptions.Set(kDriveModes[u16 % 3]);
        msg.Version        = 0;
        msg.MaxBlockSize   = static_cast<uint16_t>(u32);
        msg.StartOffset    = u64;
        msg.MaxLength      = (static_cast<uint64_t>(u32) << 8) ^ u64;
        msg.FileDesLength  = static_cast<uint16_t>(payload.size() > kMaxFileDesignatorLen ? kMaxFileDesignatorLen : payload.size());
        msg.FileDesignator = payload.empty() ? nullptr : payload.data();
        const MessageType type = (u32 & 0x1u) ? MessageType::SendInit : MessageType::ReceiveInit;
        Inject(session, Protocols::BDX::Id, to_underlying(type), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectReceiveAccept: {
        ReceiveAccept msg;
        msg.TransferCtlFlags.Set(kDriveModes[u16 % 3]);
        msg.Version      = 0;
        msg.MaxBlockSize = static_cast<uint16_t>(u16);
        msg.StartOffset  = static_cast<uint64_t>(u32);
        msg.Length       = u64;
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::ReceiveAccept), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectSendAccept: {
        SendAccept msg;
        msg.TransferCtlFlags.Set(kDriveModes[u16 % 3]);
        msg.Version      = 0;
        msg.MaxBlockSize = static_cast<uint16_t>(u16);
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::SendAccept), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectBlockQuery: {
        BlockQuery msg;
        msg.BlockCounter = useMirror ? session.GetNextQueryNum() : u32;
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::BlockQuery), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectBlockQueryWithSkip: {
        BlockQueryWithSkip msg;
        msg.BlockCounter = useMirror ? session.GetNextQueryNum() : u32;
        msg.BytesToSkip  = u64;
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::BlockQueryWithSkip), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectBlock: {
        Block msg;
        msg.BlockCounter = useMirror ? session.GetNextBlockNum() : u32;
        msg.Data         = payload.empty() ? nullptr : payload.data();
        msg.DataLength   = payload.size();
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::Block), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectBlockEOF: {
        BlockEOF msg;
        msg.BlockCounter = useMirror ? session.GetNextBlockNum() : u32;
        msg.Data         = payload.empty() ? nullptr : payload.data();
        msg.DataLength   = payload.size();
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::BlockEOF), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectBlockAck: {
        BlockAck msg;
        msg.BlockCounter = useMirror ? session.GetNextBlockNum() : u32;
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::BlockAck), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectBlockAckEOF: {
        BlockAckEOF msg;
        msg.BlockCounter = useMirror ? session.GetNextBlockNum() : u32;
        Inject(session, Protocols::BDX::Id, to_underlying(MessageType::BlockAckEOF), EncodeBdxMessage(msg));
        break;
    }
    case Op::kInjectRawStatusReport: {
        // Raw SecureChannel StatusReport bytes -> HandleStatusReportMessage / StatusReport::Parse.
        Inject(session, Protocols::SecureChannel::Id, to_underlying(Protocols::SecureChannel::MsgType::StatusReport),
               MakeBuf(payload));
        break;
    }
    case Op::kInjectRawBdx: {
        // Arbitrary bytes under a fuzzer-chosen BDX message type -> the Handle*/Parse dispatch
        // with garbage, in whatever state the session currently holds.
        Inject(session, Protocols::BDX::Id, static_cast<uint8_t>(u16), MakeBuf(payload));
        break;
    }
    case Op::kLocalAccept: {
        TransferSession::TransferAcceptData acceptData;
        acceptData.ControlMode  = kDriveModes[u16 % 3];
        acceptData.MaxBlockSize = static_cast<uint16_t>(u16);
        acceptData.StartOffset  = static_cast<uint64_t>(u32);
        acceptData.Length       = u64;
        RETURN_SAFELY_IGNORED session.AcceptTransfer(acceptData);
        break;
    }
    case Op::kLocalReject: {
        RETURN_SAFELY_IGNORED session.RejectTransfer(PickStatusCode(u32));
        break;
    }
    case Op::kLocalPrepareBlockQuery: {
        RETURN_SAFELY_IGNORED session.PrepareBlockQuery();
        break;
    }
    case Op::kLocalPrepareBlockQueryWithSkip: {
        RETURN_SAFELY_IGNORED session.PrepareBlockQueryWithSkip(u64);
        break;
    }
    case Op::kLocalPrepareBlock: {
        TransferSession::BlockData blockData;
        blockData.Data   = payload.empty() ? nullptr : payload.data();
        blockData.Length = payload.size();
        blockData.IsEof  = (u32 & 0x1u) != 0u;
        RETURN_SAFELY_IGNORED session.PrepareBlock(blockData);
        break;
    }
    case Op::kLocalPrepareBlockAck: {
        RETURN_SAFELY_IGNORED session.PrepareBlockAck();
        break;
    }
    case Op::kLocalAbort: {
        RETURN_SAFELY_IGNORED session.AbortTransfer(PickStatusCode(u32));
        break;
    }
    case Op::kDrain:
    case Op::kOpCount:
        break;
    }

    DrainOutput(session);
}

Step Mk(uint8_t op, uint16_t a, uint32_t b, uint64_t c, std::vector<uint8_t> payload = {})
{
    return std::make_tuple(op, a, b, c, std::move(payload));
}

// Seed step-sequences that walk the session through the deep accept -> query ->
// block -> blockEOF flow. Random sequences almost never reach it (any malformed
// step trips the session into an error state, wasting the remaining steps), so
// these seed the corpus with the multi-block accumulation path the mutator then
// explores. Op values follow the Op enum; a `u32` with bit0 set requests the
// mirrored (currently-expected) counter. Setup args are drawn randomly, so each
// seed only reaches depth when the random setup role is compatible -- which is
// enough to seed the corpus with the deep code paths.
std::vector<std::vector<Step>> BdxSeedStepSequences()
{
    const std::vector<uint8_t> block(64, 0xAB);
    const std::vector<uint8_t> fileDes = { 't', 'e', 's', 't', '.', 'b', 'i', 'n' };
    return {
        // initiating receiver: ReceiveAccept(64,len=100) -> query -> Block -> query -> BlockEOF
        { Mk(1, 64, 0, 100, {}), Mk(13, 0, 0, 0), Mk(5, 0, 1, 0, block), Mk(13, 0, 0, 0), Mk(6, 0, 1, 0, block) },
        // initiating sender: SendAccept(64) -> BlockQuery(mirror) -> PrepareBlock
        { Mk(2, 64, 0, 0, {}), Mk(3, 0, 1, 0), Mk(15, 0, 0, 0, block) },
        // responder: Init(64,"test.bin") -> local Accept(64) -> BlockQuery(mirror) -> PrepareBlock
        { Mk(0, 64, 0, 0, fileDes), Mk(11, 64, 0, 0), Mk(3, 0, 1, 0), Mk(15, 0, 0, 0, block) },
    };
}

void BdxSessionSequenceDoesNotCrash(bool weInitiate, uint8_t roleByte, uint8_t initFlags, uint16_t initMaxBlock,
                                    uint64_t initLength, uint64_t initOffset, const std::vector<Step> & steps)
{
    EnsureInitialized();

    TransferSession session;
    const System::Clock::Timeout timeout = System::Clock::Seconds16(24);
    const TransferRole role              = (roleByte & 0x1u) ? TransferRole::kSender : TransferRole::kReceiver;
    const uint16_t setupBlock            = (initMaxBlock == 0) ? 64 : initMaxBlock;

    if (weInitiate)
    {
        TransferSession::TransferInitData initData;
        initData.TransferCtlFlags       = kDriveModes[initFlags % 3];
        initData.MaxBlockSize           = setupBlock;
        initData.StartOffset            = initOffset;
        initData.Length                 = initLength;
        static const uint8_t kFileDes[] = { 't', 'e', 's', 't', '.', 'b', 'i', 'n' };
        initData.FileDesignator         = kFileDes;
        initData.FileDesLength          = static_cast<uint16_t>(sizeof(kFileDes));
        if (session.StartTransfer(role, initData, timeout) != CHIP_NO_ERROR)
        {
            return;
        }
    }
    else
    {
        BitFlags<TransferControlFlags> opts;
        opts.Set(TransferControlFlags::kSenderDrive).Set(TransferControlFlags::kReceiverDrive);
        if (session.WaitForTransfer(role, opts, setupBlock, timeout) != CHIP_NO_ERROR)
        {
            return;
        }
    }
    DrainOutput(session);

    for (const Step & step : steps)
    {
        HandleStep(session, step);
    }
}

FUZZ_TEST(FuzzBdxTransferSessionPW, BdxSessionSequenceDoesNotCrash)
    .WithDomains(Arbitrary<bool>(),     // weInitiate: StartTransfer vs WaitForTransfer
                 Arbitrary<uint8_t>(),  // role selector
                 Arbitrary<uint8_t>(),  // drive-mode selector for setup
                 Arbitrary<uint16_t>(), // proposed MaxBlockSize
                 Arbitrary<uint64_t>(), // proposed transfer length
                 Arbitrary<uint64_t>(), // proposed start offset
                 VectorOf(TupleOf(Arbitrary<uint8_t>(), Arbitrary<uint16_t>(), Arbitrary<uint32_t>(), Arbitrary<uint64_t>(),
                                  Arbitrary<std::vector<uint8_t>>()))
                     .WithMaxSize(kMaxSteps)
                     .WithSeeds(BdxSeedStepSequences()));

} // namespace
