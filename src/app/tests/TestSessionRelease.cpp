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

/*
 * Regression tests for the family of crashes that occur when an
 * ExchangeContext outlives the SecureSession it was created on (for example,
 * when a TCP connection is torn down while a higher layer like the
 * Interaction Model is still synchronously processing an inbound message on
 * the exchange). In that situation:
 *
 *   - SessionHolderWithDelegate::SessionReleased clears the holder before
 *     invoking the delegate callback, so ExchangeContext::mSession becomes
 *     empty.
 *   - ExchangeContext::OnSessionReleased takes the IsSendExpected() path
 *     and calls DoClose(true) instead of Abort(), so the EC stays alive
 *     (ExchangeHolder keeps its raw pointer when IsSendExpected() is true
 *     at the time OnExchangeClosing fires).
 *   - Subsequent send-side helpers that assume mSession is populated
 *     (UseSuggestedResponseTimeout via mSession->ComputeRoundTripTimeout,
 *     and GetSessionHandle's VerifyOrDieWithObject) crash the process.
 *
 * These tests pin down the contract that the patched call sites must
 * tolerate a released session by returning CHIP_ERROR_MISSING_SECURE_SESSION
 * instead of crashing.
 *
 * Crash-site coverage (one test per patched guard):
 *   StatusResponse::Send               - StatusResponseSendAfterSessionRelease*
 *   ReadHandler::SendStatusReport      - ReadHandlerSendStatusReportAfterSessionReleaseDoesNotCrash
 *   ReadHandler::SendReportData        - ReadHandlerSendReportDataAfterSessionReleaseDoesNotCrash
 *   WriteHandler::SendWriteResponse    - WriteHandlerSendWriteResponseAfterSessionReleaseDoesNotCrash
 *   CommandResponseSender::SendCommandResponse - CommandResponseSenderSendCommandResponseAfterSessionReleaseDoesNotCrash
 */

#include <pw_unit_test/framework.h>

#include <app/CommandHandlerImpl.h>
#include <app/CommandResponseSender.h>
#include <app/InteractionModelEngine.h>
#include <app/InteractionModelTimeout.h>
#include <app/ReadHandler.h>
#include <app/StatusResponse.h>
#include <app/WriteHandler.h>
#include <app/reporting/tests/MockReportScheduler.h>
#include <app/tests/AppTestContext.h>
#include <lib/core/CHIPError.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/tests/ExtraPwTestMacros.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeDelegate.h>
#include <messaging/ExchangeMgr.h>
#include <protocols/Protocols.h>
#include <protocols/secure_channel/Constants.h>
#include <system/SystemPacketBuffer.h>

// ---------------------------------------------------------------------------
// File-internal helpers (anonymous namespace)
// ---------------------------------------------------------------------------
namespace {

using namespace chip;
using namespace chip::app;
using namespace chip::Messaging;

// Minimal ExchangeDelegate that does nothing useful but lets us hold an
// ExchangeContext alive past a session release by setting WillSendMessage
// on it before triggering session expiry.
class DoNothingExchangeDelegate : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(ExchangeContext *, const PayloadHeader &, System::PacketBufferHandle &&) override
    {
        return CHIP_NO_ERROR;
    }
    void OnResponseTimeout(ExchangeContext *) override {}
};

// Minimal ReadHandler::ManagementCallback (mirrors NullReadHandlerCallback in
// TestReadInteraction.cpp).
class NullReadHandlerCallback : public ReadHandler::ManagementCallback
{
public:
    void OnDone(ReadHandler &) override {}
    ReadHandler::ApplicationCallback * GetAppCallback() override { return nullptr; }
    InteractionModelEngine * GetInteractionModelEngine() override { return InteractionModelEngine::GetInstance(); }
};

// Minimal CommandResponseSender::Callback.
class NullCommandResponseSenderCallback : public CommandResponseSender::Callback
{
public:
    void OnDone(CommandResponseSender &) override {}
};

// Minimal CommandHandlerImpl::Callback (required to construct CommandResponseSender).
class NullCommandHandlerImplCallback : public CommandHandlerImpl::Callback
{
public:
    void OnDone(CommandHandlerImpl &) override {}
    Protocols::InteractionModel::Status ValidateCommandCanBeDispatched(const DataModel::InvokeRequest &) override
    {
        return Protocols::InteractionModel::Status::Success;
    }
    void DispatchCommand(CommandHandlerImpl &, const ConcreteCommandPath &, TLV::TLVReader &) override {}
};

} // namespace

namespace chip {
namespace app {

class TestSessionRelease : public chip::Testing::AppContext
{
public:
    // Declarations for tests that access private members of ReadHandler,
    // WriteHandler, and CommandResponseSender.  The bodies are defined via
    // TEST_F_FROM_FIXTURE below so that the code runs in the context of
    // this class (the declared friend) rather than in the generated subclass.
    void ReadHandlerSendStatusReportAfterSessionReleaseDoesNotCrash();
    void ReadHandlerSendReportDataAfterSessionReleaseDoesNotCrash();
    void WriteHandlerSendWriteResponseAfterSessionReleaseDoesNotCrash();
    void CommandResponseSenderSendCommandResponseAfterSessionReleaseDoesNotCrash();
};

// ---------------------------------------------------------------------------
// Helper: create an exchange that will hit the IsSendExpected() branch in
// ExchangeContext::OnSessionReleased.
//
// Calling WillSendMessage() before the session is expired ensures that when
// OnSessionReleased fires it reaches the IsSendExpected() branch, which calls
// DoClose(true).  kFlagWillSendMessage is NOT cleared, so ExchangeHolder sees
// IsSendExpected()==true in OnExchangeClosing and keeps its mpExchangeCtx raw
// pointer.  The EC remains alive (refcount unchanged by DoClose); callers must
// check HasSessionHandle() before attempting any send.
// ---------------------------------------------------------------------------
static ExchangeContext * MakeRawExchangeWillSend(chip::Testing::AppContext & ctx, DoNothingExchangeDelegate & delegate)
{
    ExchangeContext * ec = ctx.NewExchangeToBob(&delegate);
    VerifyOrDieWithMsg(ec != nullptr, Test, "NewExchangeToBob returned null");
    ec->WillSendMessage();
    return ec;
}

// ---------------------------------------------------------------------------
// StatusResponse::Send tests (cover the guard in StatusResponse.cpp)
//
// These do not access any private members, so plain TEST_F is fine.
// ---------------------------------------------------------------------------

// StatusResponse::Send on an exchange whose session has been released.
// Without the guard, this crashes inside Optional::Value() via
// ExchangeContext::UseSuggestedResponseTimeout. With the fix,
// StatusResponse::Send returns CHIP_ERROR_MISSING_SECURE_SESSION.
TEST_F(TestSessionRelease, StatusResponseSendAfterSessionReleaseDoesNotCrash)
{
    DoNothingExchangeDelegate delegate;
    ExchangeHandle exchange = [&] {
        ExchangeContext * ec = NewExchangeToBob(&delegate);
        VerifyOrDieWithMsg(ec != nullptr, Test, "NewExchangeToBob returned null");
        ec->WillSendMessage();
        return ExchangeHandle(*ec);
    }();
    ASSERT_TRUE(exchange->HasSessionHandle());

    ExpireSessionAliceToBob();

    EXPECT_FALSE(exchange->HasSessionHandle());
    EXPECT_EQ(StatusResponse::Send(Protocols::InteractionModel::Status::Success, &exchange.Get(),
                                   /* aExpectResponse = */ false),
              CHIP_ERROR_MISSING_SECURE_SESSION);

    // OnSessionReleased called DoClose(true), which did NOT drop the initial
    // EC ref.  Call Close() explicitly here to release it (DoClose is
    // idempotent so the call body is a no-op, but the Release() in Close()
    // drops refcount from 2 to 1).  The ExchangeHandle destructor then
    // drops the final ref, freeing the EC cleanly.
    exchange->Close();
}

// Same as above but exercises the aExpectResponse=true branch, which is
// what ReadHandler::SendStatusReport ultimately reaches.
TEST_F(TestSessionRelease, StatusResponseSendExpectingResponseAfterSessionReleaseDoesNotCrash)
{
    DoNothingExchangeDelegate delegate;
    ExchangeHandle exchange = [&] {
        ExchangeContext * ec = NewExchangeToBob(&delegate);
        VerifyOrDieWithMsg(ec != nullptr, Test, "NewExchangeToBob returned null");
        ec->WillSendMessage();
        return ExchangeHandle(*ec);
    }();
    ASSERT_TRUE(exchange->HasSessionHandle());

    ExpireSessionAliceToBob();

    EXPECT_FALSE(exchange->HasSessionHandle());
    EXPECT_EQ(StatusResponse::Send(Protocols::InteractionModel::Status::Busy, &exchange.Get(),
                                   /* aExpectResponse = */ true),
              CHIP_ERROR_MISSING_SECURE_SESSION);

    // Same as above: DoClose(true) kept the EC alive. Close() releases the
    // initial ref (refcount 2→1), then ~ExchangeHandle releases the last ref.
    exchange->Close();
}

// ---------------------------------------------------------------------------
// ReadHandler::SendStatusReport test (covers the GetSessionHandle guard in
// ReadHandler.cpp SendStatusReport when IsPriming()==true)
//
// Uses TEST_F_FROM_FIXTURE so the body runs as TestSessionRelease::
// ReadHandlerSendStatusReportAfterSessionReleaseDoesNotCrash(), which is the
// declared friend of ReadHandler.
// ---------------------------------------------------------------------------
TEST_F_FROM_FIXTURE(TestSessionRelease, ReadHandlerSendStatusReportAfterSessionReleaseDoesNotCrash)
{
    // ReadHandler constructor calls
    //   GetInteractionModelEngine()->GetReportingEngine().GetDirtySetGeneration()
    // so the engine must be initialised first.
    auto * engine                              = InteractionModelEngine::GetInstance();
    reporting::ReportSchedulerImpl * scheduler = reporting::GetDefaultReportScheduler();
    ASSERT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), scheduler), CHIP_NO_ERROR);

    DoNothingExchangeDelegate ecDelegate;
    ExchangeContext * ec = MakeRawExchangeWillSend(*this, ecDelegate);

    NullReadHandlerCallback nullCallback;
    // ReadHandler::ReadHandler grabs ec into mExchangeCtx (ExchangeHolder).
    // The constructor sets PrimingReports flag and grabs the session handle.
    ReadHandler rh(nullCallback, ec, ReadHandler::InteractionType::Read, scheduler);

    // IsPriming() is set by the constructor (ReadHandler.cpp:72).
    ASSERT_TRUE(rh.IsPriming());

    // Put the handler into CanStartReporting state (normally done via
    // ProcessReadRequest; accessed here via the TestSessionRelease
    // friend declaration in ReadHandler.h).
    rh.mState = ReadHandler::HandlerState::CanStartReporting;

    ExpireSessionAliceToBob();

    // After OnSessionReleased, ExchangeContext::OnSessionReleased calls
    // DoClose(true).  kFlagWillSendMessage is NOT cleared, so ExchangeHolder
    // keeps its raw pointer (IsSendExpected()==true in OnExchangeClosing).
    // mExchangeCtx.Get() is non-null but HasSessionHandle() is false.
    ASSERT_NE(rh.mExchangeCtx.Get(), nullptr);
    ASSERT_FALSE(rh.mExchangeCtx.Get()->HasSessionHandle());

    // Without the ReadHandler.cpp guard: crashes (null pointer dereference on
    // the session). With the guard: returns CHIP_ERROR_MISSING_SECURE_SESSION.
    EXPECT_EQ(rh.SendStatusReport(Protocols::InteractionModel::Status::Success), CHIP_ERROR_MISSING_SECURE_SESSION);

    engine->Shutdown();
    // ReadHandler destructor calls ExchangeHolder::Release(). mExchangeCtx is
    // non-null and IsSendExpected()==true, so Release() calls Abort() on the
    // still-alive EC: DoClose(no-op) + Release() drops refcount to 0 → freed.
}

// ---------------------------------------------------------------------------
// ReadHandler::SendReportData test (covers the GetSessionHandle guard in
// ReadHandler.cpp SendReportData when IsPriming()==true)
// ---------------------------------------------------------------------------
TEST_F_FROM_FIXTURE(TestSessionRelease, ReadHandlerSendReportDataAfterSessionReleaseDoesNotCrash)
{
    auto * engine                              = InteractionModelEngine::GetInstance();
    reporting::ReportSchedulerImpl * scheduler = reporting::GetDefaultReportScheduler();
    ASSERT_EQ(engine->Init(&GetExchangeManager(), &GetFabricTable(), scheduler), CHIP_NO_ERROR);

    DoNothingExchangeDelegate ecDelegate;
    ExchangeContext * ec = MakeRawExchangeWillSend(*this, ecDelegate);

    NullReadHandlerCallback nullCallback;
    ReadHandler rh(nullCallback, ec, ReadHandler::InteractionType::Read, scheduler);

    ASSERT_TRUE(rh.IsPriming());
    rh.mState = ReadHandler::HandlerState::CanStartReporting;

    ExpireSessionAliceToBob();

    ASSERT_NE(rh.mExchangeCtx.Get(), nullptr);
    ASSERT_FALSE(rh.mExchangeCtx.Get()->HasSessionHandle());

    System::PacketBufferHandle payload = System::PacketBufferHandle::New(64);
    ASSERT_FALSE(payload.IsNull());

    // Without the ReadHandler.cpp guard: crashes (null pointer dereference).
    // With the guard: returns CHIP_ERROR_MISSING_SECURE_SESSION.
    EXPECT_EQ(rh.SendReportData(std::move(payload), /* aMoreChunks = */ false), CHIP_ERROR_MISSING_SECURE_SESSION);

    engine->Shutdown();
}

// ---------------------------------------------------------------------------
// WriteHandler::SendWriteResponse test (covers the UseSuggestedResponseTimeout
// guard in WriteHandler.cpp)
// ---------------------------------------------------------------------------
TEST_F_FROM_FIXTURE(TestSessionRelease, WriteHandlerSendWriteResponseAfterSessionReleaseDoesNotCrash)
{
    DoNothingExchangeDelegate ecDelegate;
    ExchangeContext * ec = MakeRawExchangeWillSend(*this, ecDelegate);

    WriteHandler wh;

    // Initialise the TLV write-response builder that FinalizeMessage requires.
    // This mirrors what HandleWriteRequestMessage does before calling
    // SendWriteResponse.
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
    ASSERT_FALSE(packet.IsNull());
    System::PacketBufferTLVWriter messageWriter;
    messageWriter.Init(std::move(packet));
    // Accessed via the TestSessionRelease friend in WriteHandler.h.
    ASSERT_EQ(wh.mWriteResponseBuilder.Init(&messageWriter), CHIP_NO_ERROR);
    wh.mWriteResponseBuilder.CreateWriteResponses();
    ASSERT_EQ(wh.mWriteResponseBuilder.GetError(), CHIP_NO_ERROR);

    // Set state to AddStatus (normally reached after ProcessWriteRequest).
    wh.mState = WriteHandler::State::AddStatus;

    // Grab the exchange into the WriteHandler's ExchangeHolder.
    wh.mExchangeCtx.Grab(ec);

    ExpireSessionAliceToBob();

    // ExchangeHolder keeps its pointer: IsSendExpected()==true in
    // OnExchangeClosing (kFlagWillSendMessage not cleared by DoClose).
    ASSERT_NE(wh.mExchangeCtx.Get(), nullptr);
    ASSERT_FALSE(wh.mExchangeCtx.Get()->HasSessionHandle());

    // WriteHandler::SendWriteResponse has a pre-existing null-exchange guard
    // (VerifyOrExit(mExchangeCtx, ...)) that passes (non-null), followed by
    // the HasSessionHandle() guard (VerifyOrExit(mExchangeCtx->HasSessionHandle(), ...))
    // which fires, returning CHIP_ERROR_MISSING_SECURE_SESSION.
    EXPECT_EQ(wh.SendWriteResponse(std::move(messageWriter)), CHIP_ERROR_MISSING_SECURE_SESSION);

    // WriteHandler destructor calls ExchangeHolder::Release(). mExchangeCtx is
    // non-null and IsSendExpected()==true, so Release() calls Abort() on the
    // still-alive EC: DoClose(no-op) + Release() drops refcount to 0 → freed.
}

// ---------------------------------------------------------------------------
// CommandResponseSender::SendCommandResponse test (covers the
// UseSuggestedResponseTimeout guard in CommandResponseSender.cpp, which is
// only reached when HasMoreToSend() is true after popping the first chunk)
// ---------------------------------------------------------------------------
TEST_F_FROM_FIXTURE(TestSessionRelease, CommandResponseSenderSendCommandResponseAfterSessionReleaseDoesNotCrash)
{
    DoNothingExchangeDelegate ecDelegate;
    ExchangeContext * ec = MakeRawExchangeWillSend(*this, ecDelegate);

    NullCommandResponseSenderCallback crsCallback;
    NullCommandHandlerImplCallback cmdCallback;
    CommandResponseSender crs(&crsCallback, &cmdCallback);

    // Add two chunks so that after PopHead() HasMoreToSend() is still true,
    // which is the only code path that reaches UseSuggestedResponseTimeout.
    // AddInvokeResponseToSend is public (CommandHandlerExchangeInterface).
    System::PacketBufferHandle chunk1 = System::PacketBufferHandle::New(64);
    System::PacketBufferHandle chunk2 = System::PacketBufferHandle::New(64);
    ASSERT_FALSE(chunk1.IsNull());
    ASSERT_FALSE(chunk2.IsNull());
    crs.AddInvokeResponseToSend(std::move(chunk1));
    crs.AddInvokeResponseToSend(std::move(chunk2));

    // Grab the exchange (mExchangeCtx is private, accessed via friend).
    crs.mExchangeCtx.Grab(ec);

    ExpireSessionAliceToBob();

    // ExchangeHolder keeps its pointer (IsSendExpected()==true in
    // OnExchangeClosing; kFlagWillSendMessage not cleared by DoClose).
    ASSERT_NE(crs.mExchangeCtx.Get(), nullptr);
    ASSERT_FALSE(crs.mExchangeCtx.Get()->HasSessionHandle());

    // Without the CommandResponseSender.cpp guard: crashes (null pointer
    // dereference on the session). With the guard: CHIP_ERROR_MISSING_SECURE_SESSION.
    EXPECT_EQ(crs.SendCommandResponse(), CHIP_ERROR_MISSING_SECURE_SESSION);

    // CommandResponseSender destructor calls ExchangeHolder::Release().
    // mExchangeCtx is non-null and IsSendExpected()==true, so Release() calls
    // Abort() on the still-alive EC: DoClose(no-op) + Release() → freed.
}

} // namespace app
} // namespace chip
