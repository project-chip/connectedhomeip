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

// The lifecycle of a NetworkIdentityManagementRegistrar: which calls it accepts, how it reports the
// outcomes it owes its caller, the two ways it can be shut down, and when it counts as idle. Its
// operations get as far as asking the controller for a session here and no further, since a stub
// controller has none to give; what the cluster commands look like on the wire needs to be covered
// by integration tests.

#include <pw_unit_test/framework.h>

#include <controller/CHIPDeviceController.h>
#include <controller/NetworkIdentityManagementRegistrar.h>
#include <credentials/CHIPCert.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/support/Span.h>
#include <lib/support/tests/ExtraPwTestMacros.h>

#include <memory>
#include <optional>

using namespace chip;
using namespace chip::Controller;

namespace {

constexpr NodeId kNimNodeId       = 0x1234;
constexpr EndpointId kNimEndpoint = 1;

// A 20-byte key identifier, which is the only shape RemoveClient accepts.
constexpr uint8_t kClientIdentifierBytes[Credentials::kKeyIdentifierLength] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                                                                0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
                                                                                0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13 };
constexpr Credentials::CertificateKeyId kClientIdentifier{ kClientIdentifierBytes };

// RegisterClient() only checks that an identity is non-empty and fits, and nothing here gets as far
// as putting one on the wire, so the contents are arbitrary.
constexpr uint8_t kClientIdentityBytes[] = { 0x15, 0x18 };
constexpr ByteSpan kClientIdentity{ kClientIdentityBytes };

/**
 * A controller that establishes no sessions of its own: it holds on to the callbacks it is given so
 * that a test decides if and when a connection attempt resolves, which is what lets an operation sit
 * in flight. Refusing outright is the third option, mirroring a controller that is not initialized.
 */
class StubDeviceController : public DeviceController
{
public:
    void RefuseConnections() { mAcceptConnections = false; }
    bool ConnectionPending() const { return mOnFailure != nullptr; }

    CHIP_ERROR GetConnectedDevice(NodeId peerNodeId, Callback::Callback<OnDeviceConnected> * onConnection,
                                  Callback::Callback<OnDeviceConnectionFailure> * onFailure,
                                  TransportPayloadCapability transportPayloadCapability) override
    {
        VerifyOrReturnError(mAcceptConnections, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrDie(!ConnectionPending());
        mOnConnection = onConnection;
        mOnFailure    = onFailure;
        return CHIP_NO_ERROR;
    }

    // Fails the connection attempt in flight, which is as far as an operation gets here: without a
    // real session there is no way to let one reach the invoke.
    void FailPendingConnection(CHIP_ERROR error)
    {
        VerifyOrDie(ConnectionPending());
        auto * onFailure = mOnFailure;
        mOnConnection    = nullptr;
        mOnFailure       = nullptr;
        onFailure->mCall(onFailure->mContext, ScopedNodeId(kNimNodeId, kUndefinedFabricIndex), error);
    }

private:
    bool mAcceptConnections                                    = true;
    Callback::Callback<OnDeviceConnected> * mOnConnection      = nullptr;
    Callback::Callback<OnDeviceConnectionFailure> * mOnFailure = nullptr;
};

class TestNetworkIdentityManagementRegistrar : public ::testing::Test
{
protected:
    // Records a status the registrar reported, so a test can tell "not yet" from "reported".
    struct StatusRecorder
    {
        bool Called() const { return status.has_value(); }
        CHIP_ERROR Status() const
        {
            VerifyOrDie(status.has_value());
            return status.value();
        }

        std::optional<CHIP_ERROR> status;
        Callback::Callback<OnClientUnregisteredFunct> callback{
            [](void * context, CHIP_ERROR aStatus) { static_cast<StatusRecorder *>(context)->status = aStatus; }, this
        };
    };

    // Records the outcome of a registration, which unlike the other operations also says whether its
    // status is determinate, i.e. whether the AddClient it stands for definitely had no effect.
    struct RegistrationRecorder
    {
        bool Called() const { return status.has_value(); }
        CHIP_ERROR Status() const
        {
            VerifyOrDie(status.has_value());
            return status.value();
        }

        std::optional<CHIP_ERROR> status;
        bool determinate = false;
        Callback::Callback<OnClientRegisteredFunct> callback{ [](void * context, CHIP_ERROR aStatus, bool aDeterminate) {
                                                                 auto * self       = static_cast<RegistrationRecorder *>(context);
                                                                 self->status      = aStatus;
                                                                 self->determinate = aDeterminate;
                                                             },
                                                              this };
    };

    // Records that the registrar reported itself idle.
    struct IdleRecorder
    {
        bool called = false;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{
            [](void * context) { static_cast<IdleRecorder *>(context)->called = true; }, this
        };
    };

    StubDeviceController mController;
    NetworkIdentityManagementRegistrar mRegistrar{ mController, kNimNodeId, kNimEndpoint };
};

TEST_F(TestNetworkIdentityManagementRegistrar, StartsIdle)
{
    EXPECT_TRUE(mRegistrar.IsIdle());
}

TEST_F(TestNetworkIdentityManagementRegistrar, IsBusyWhileARevocationIsInFlight)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    EXPECT_FALSE(mRegistrar.IsIdle());
    EXPECT_FALSE(revocation.Called());

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(mRegistrar.IsIdle());
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_TIMEOUT);
}

// A controller that cannot even attempt the connection is reported like any other outcome. The
// registrar has taken the callback on by then, so unlike a refused call this one is in flight, however
// briefly, and leaves the registrar idle again once it completes.
TEST_F(TestNetworkIdentityManagementRegistrar, AControllerRefusalIsReportedThroughTheCallback)
{
    mController.RefuseConnections();

    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mRegistrar.IsIdle());
}

TEST_F(TestNetworkIdentityManagementRegistrar, RefusesAnOverlappingRevocation)
{
    StatusRecorder first, second;
    mRegistrar.UnregisterClient(kClientIdentifier, &first.callback);

    // A refusal is delivered through the callback, synchronously, and leaves the request that is
    // already in flight alone.
    mRegistrar.UnregisterClient(kClientIdentifier, &second.callback);
    ASSERT_TRUE(second.Called());
    EXPECT_EQ(second.Status(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_FALSE(first.Called());

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
}

TEST_F(TestNetworkIdentityManagementRegistrar, ShutdownReportsAnOperationInFlightAsCancelled)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    mRegistrar.Shutdown();
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_CANCELLED);
    EXPECT_TRUE(mRegistrar.IsIdle());
}

TEST_F(TestNetworkIdentityManagementRegistrar, RefusesCallsAfterShutdown)
{
    mRegistrar.Shutdown();

    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_INCORRECT_STATE);
}

// A refused call and a session that never materialises both leave the NIM untouched, and the
// registrar saying so is what spares its caller a RemoveClient for access that was never granted.
TEST_F(TestNetworkIdentityManagementRegistrar, ARefusedRegistrationIsADeterminateFailure)
{
    mRegistrar.Shutdown();

    RegistrationRecorder registration;
    mRegistrar.RegisterClient(kClientIdentity, &registration.callback);
    ASSERT_TRUE(registration.Called());
    EXPECT_EQ(registration.Status(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(registration.determinate);
}

TEST_F(TestNetworkIdentityManagementRegistrar, ARegistrationThatNeverGetsASessionIsADeterminateFailure)
{
    RegistrationRecorder registration;
    mRegistrar.RegisterClient(kClientIdentity, &registration.callback);
    ASSERT_FALSE(registration.Called());

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    ASSERT_TRUE(registration.Called());
    EXPECT_EQ(registration.Status(), CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(registration.determinate) << "the AddClient cannot have been sent without a session";
}

// Abandoning a registration is the one case whose determinacy depends on how far it got. Still
// waiting for a session, as here, it cannot have been acted on. Once the command is in flight it can,
// so the caller is left to revoke an identity that may or may not be on the network. That is not
// reachable with a mock controller that never produces a session, and is left to integration testing.
TEST_F(TestNetworkIdentityManagementRegistrar, ARegistrationAbandonedWhileConnectingIsADeterminateFailure)
{
    RegistrationRecorder registration;
    mRegistrar.RegisterClient(kClientIdentity, &registration.callback);

    mRegistrar.Shutdown();
    ASSERT_TRUE(registration.Called());
    EXPECT_EQ(registration.Status(), CHIP_ERROR_CANCELLED);
    EXPECT_TRUE(registration.determinate);
}

// Revoking from a failed registration's completion is what a commissioner does with an indeterminate
// failure, and with a synchronous one it means UnregisterClient() is called while RegisterClient() is
// still on the stack. The failure is forced here by refusing both requests outright, which makes for
// the tightest version of that nesting: what matters is that the registrar comes out of it idle with
// both callers answered, not what determinacy it reported on the way.
TEST_F(TestNetworkIdentityManagementRegistrar, ARevocationCanStartFromAFailedRegistration)
{
    mController.RefuseConnections();

    StatusRecorder revocation;
    struct Registration
    {
        NetworkIdentityManagementRegistrar & registrar;
        StatusRecorder & revocation;
        int completions = 0;

        Callback::Callback<OnClientRegisteredFunct> callback{ [](void * context, CHIP_ERROR, bool) {
                                                                 auto * self = static_cast<Registration *>(context);
                                                                 self->completions++;
                                                                 self->registrar.UnregisterClient(kClientIdentifier,
                                                                                                  &self->revocation.callback);
                                                             },
                                                              this };
    };
    Registration registration{ mRegistrar, revocation };

    mRegistrar.RegisterClient(kClientIdentity, &registration.callback);
    EXPECT_EQ(registration.completions, 1);
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_INCORRECT_STATE);
    EXPECT_TRUE(mRegistrar.IsIdle());
}

TEST_F(TestNetworkIdentityManagementRegistrar, WaitForIdleCompletesImmediatelyWhenThereIsNothingToWaitFor)
{
    IdleRecorder idle;
    mRegistrar.WaitForIdle(&idle.callback);
    EXPECT_TRUE(idle.called);
}

// The graceful half of a shutdown: nothing new gets in, but what is in flight is left alone.
TEST_F(TestNetworkIdentityManagementRegistrar, StopAcceptingRequestsLeavesAnOperationInFlight)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    mRegistrar.StopAcceptingRequests();
    EXPECT_FALSE(revocation.Called());
    EXPECT_FALSE(mRegistrar.IsIdle());

    StatusRecorder refused;
    mRegistrar.UnregisterClient(kClientIdentifier, &refused.callback);
    ASSERT_TRUE(refused.Called());
    EXPECT_EQ(refused.Status(), CHIP_ERROR_INCORRECT_STATE);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(revocation.Called());
}

// The point of the whole thing: an owner that is about to go away can let an in-flight revocation
// reach the network first.
TEST_F(TestNetworkIdentityManagementRegistrar, WaitForIdleWaitsForAnOperationInFlight)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    IdleRecorder idle;
    mRegistrar.WaitForIdle(&idle.callback);
    EXPECT_FALSE(idle.called);
    EXPECT_FALSE(revocation.Called());

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(idle.called);
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(mRegistrar.IsIdle());
}

TEST_F(TestNetworkIdentityManagementRegistrar, ReleasesEveryWaitingCaller)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    IdleRecorder first, second;
    mRegistrar.WaitForIdle(&first.callback);
    mRegistrar.WaitForIdle(&second.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(first.called);
    EXPECT_TRUE(second.called);
}

// Idle means the caller has its answer, not just that the network is done with us: a waiter that
// tears things down must not do so while a completion is still on its way to whoever asked for it.
TEST_F(TestNetworkIdentityManagementRegistrar, TheIdleNotificationFollowsTheCompletion)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    struct OrderObserver
    {
        StatusRecorder & revocation;
        bool sawCompletion = false;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{ [](void * context) {
                                                                             auto * self = static_cast<OrderObserver *>(context);
                                                                             self->sawCompletion = self->revocation.Called();
                                                                         },
                                                                          this };
    };
    OrderObserver observer{ revocation };
    mRegistrar.WaitForIdle(&observer.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(observer.sawCompletion);
}

// And the reason the ordering matters: a completion that starts the next request leaves the
// registrar busy, which is only visible because the idle check happens after the completion.
TEST_F(TestNetworkIdentityManagementRegistrar, ARequestStartedFromACompletionSuppressesTheIdleNotification)
{
    // Revokes a second time from the completion of the first revocation, standing in for a caller
    // that drives the registrar through a sequence of requests.
    struct ChainedRequest
    {
        NetworkIdentityManagementRegistrar & registrar;
        int completions = 0;

        Callback::Callback<OnClientUnregisteredFunct> callback{ [](void * context, CHIP_ERROR) {
                                                                   auto * self = static_cast<ChainedRequest *>(context);
                                                                   if (++self->completions == 1)
                                                                   {
                                                                       self->registrar.UnregisterClient(kClientIdentifier,
                                                                                                        &self->callback);
                                                                   }
                                                               },
                                                                this };
    };
    ChainedRequest chained{ mRegistrar };
    mRegistrar.UnregisterClient(kClientIdentifier, &chained.callback);

    IdleRecorder idle;
    mRegistrar.WaitForIdle(&idle.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    ASSERT_EQ(chained.completions, 1);
    EXPECT_FALSE(mRegistrar.IsIdle()); // the second revocation was accepted
    EXPECT_FALSE(idle.called);

    // The second revocation finishing is what finally releases the waiter.
    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_EQ(chained.completions, 2);
    EXPECT_TRUE(idle.called);
}

// Escalating a graceful shutdown to an abrupt one, which is what an owner that has run out of
// patience (or out of time) does.
TEST_F(TestNetworkIdentityManagementRegistrar, ShutdownReleasesAWaitingCaller)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    IdleRecorder idle;
    mRegistrar.WaitForIdle(&idle.callback);
    ASSERT_FALSE(idle.called);

    mRegistrar.Shutdown();
    EXPECT_TRUE(idle.called);
    ASSERT_TRUE(revocation.Called());
    EXPECT_EQ(revocation.Status(), CHIP_ERROR_CANCELLED);
}

TEST_F(TestNetworkIdentityManagementRegistrar, DestructionReleasesAWaitingCaller)
{
    auto owner = std::make_unique<NetworkIdentityManagementRegistrar>(mController, kNimNodeId, kNimEndpoint);

    StatusRecorder revocation;
    owner->UnregisterClient(kClientIdentifier, &revocation.callback);

    IdleRecorder idle;
    owner->WaitForIdle(&idle.callback);
    ASSERT_FALSE(idle.called);

    owner.reset();
    EXPECT_TRUE(idle.called);
    EXPECT_TRUE(revocation.Called());
}

// The idle notification is an ordinary cancelable callback, so an owner that goes away can withdraw
// it instead of being called back into freed memory.
TEST_F(TestNetworkIdentityManagementRegistrar, CancellingTheIdleCallbackSuppressesIt)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    IdleRecorder idle;
    mRegistrar.WaitForIdle(&idle.callback);
    idle.callback.Cancel();

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_FALSE(idle.called);
    EXPECT_TRUE(revocation.Called());
}

// Withdrawing a callback the registrar is in the middle of a round of notifications for. This is a
// stand-in for one waiter destroying another as they unwind together.
TEST_F(TestNetworkIdentityManagementRegistrar, AWaiterCanCancelAnotherFromItsCallback)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    IdleRecorder second;
    struct Canceller
    {
        IdleRecorder & other;
        bool called = false;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{ [](void * context) {
                                                                             auto * self  = static_cast<Canceller *>(context);
                                                                             self->called = true;
                                                                             self->other.callback.Cancel();
                                                                         },
                                                                          this };
    };
    Canceller first{ second };

    mRegistrar.WaitForIdle(&first.callback);
    mRegistrar.WaitForIdle(&second.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(first.called);
    EXPECT_FALSE(second.called);
}

// What WaitForIdle() is for: an owner reclaims the registrar as soon as it is safe to. Nothing may
// touch the registrar after the callback returns, and a waiter queued behind the one that destroyed
// it is still owed its callback, which the destructor delivers on the way out.
//
// The registrar is on the heap here rather than the fixture's member so that the read of a destroyed
// registrar this guards against lands in freed memory, where ASAN can see it.
TEST_F(TestNetworkIdentityManagementRegistrar, AWaiterCanDestroyTheRegistrar)
{
    auto owner = std::make_unique<NetworkIdentityManagementRegistrar>(mController, kNimNodeId, kNimEndpoint);

    StatusRecorder revocation;
    owner->UnregisterClient(kClientIdentifier, &revocation.callback);

    struct Destroyer
    {
        std::unique_ptr<NetworkIdentityManagementRegistrar> & owner;
        bool called = false;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{ [](void * context) {
                                                                             auto * self  = static_cast<Destroyer *>(context);
                                                                             self->called = true;
                                                                             self->owner.reset();
                                                                         },
                                                                          this };
    };
    Destroyer destroyer{ owner };
    IdleRecorder behind;

    owner->WaitForIdle(&destroyer.callback);
    owner->WaitForIdle(&behind.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(destroyer.called);
    EXPECT_TRUE(behind.called);
    EXPECT_FALSE(owner);
}

// A waiter that starts a request of its own is not the registrar's cue to keep telling the others it
// is idle, since it no longer is. They stay queued for the next time it actually is.
TEST_F(TestNetworkIdentityManagementRegistrar, AWaiterThatStartsARequestKeepsTheOthersWaiting)
{
    StatusRecorder first;
    mRegistrar.UnregisterClient(kClientIdentifier, &first.callback);

    struct Restarter
    {
        NetworkIdentityManagementRegistrar & registrar;
        StatusRecorder & second;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{ [](void * context) {
                                                                             auto * self = static_cast<Restarter *>(context);
                                                                             self->registrar.UnregisterClient(
                                                                                 kClientIdentifier, &self->second.callback);
                                                                         },
                                                                          this };
    };
    StatusRecorder second;
    Restarter restarter{ mRegistrar, second };
    IdleRecorder behind;

    mRegistrar.WaitForIdle(&restarter.callback);
    mRegistrar.WaitForIdle(&behind.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    ASSERT_FALSE(second.Called()); // the request the waiter started is in flight
    EXPECT_FALSE(mRegistrar.IsIdle());
    EXPECT_FALSE(behind.called);

    // The request the waiter started finishing is what makes the registrar idle again.
    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(second.Called());
    EXPECT_TRUE(behind.called);
}

// Escalating to an abrupt shutdown from a waiter's callback, the other half of what an owner might do
// with the registrar once it hears it is idle.
TEST_F(TestNetworkIdentityManagementRegistrar, AWaiterCanShutDownTheRegistrar)
{
    StatusRecorder revocation;
    mRegistrar.UnregisterClient(kClientIdentifier, &revocation.callback);

    struct Shutter
    {
        NetworkIdentityManagementRegistrar & registrar;
        bool called = false;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{ [](void * context) {
                                                                             auto * self  = static_cast<Shutter *>(context);
                                                                             self->called = true;
                                                                             self->registrar.Shutdown();
                                                                         },
                                                                          this };
    };
    Shutter shutter{ mRegistrar };
    IdleRecorder behind;

    mRegistrar.WaitForIdle(&shutter.callback);
    mRegistrar.WaitForIdle(&behind.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_TRUE(shutter.called);
    EXPECT_TRUE(behind.called);
    EXPECT_TRUE(mRegistrar.IsIdle());
}

// The awkward combination: a waiter starts a request and then destroys the registrar anyway. What
// makes this safe is the destructor's abort, which both reports the abandoned request and leaves the
// registrar idle, so the frame still walking the waiters is told to stop looking at it.
TEST_F(TestNetworkIdentityManagementRegistrar, AWaiterCanDestroyTheRegistrarAfterStartingARequest)
{
    auto owner = std::make_unique<NetworkIdentityManagementRegistrar>(mController, kNimNodeId, kNimEndpoint);

    StatusRecorder first;
    owner->UnregisterClient(kClientIdentifier, &first.callback);

    struct RestarterAndDestroyer
    {
        std::unique_ptr<NetworkIdentityManagementRegistrar> & owner;
        StatusRecorder & second;

        Callback::Callback<OnNetworkIdentityRegistrarIdleFunct> callback{ [](void * context) {
                                                                             auto * self =
                                                                                 static_cast<RestarterAndDestroyer *>(context);
                                                                             self->owner->UnregisterClient(kClientIdentifier,
                                                                                                           &self->second.callback);
                                                                             self->owner.reset();
                                                                         },
                                                                          this };
    };
    StatusRecorder second;
    RestarterAndDestroyer destroyer{ owner, second };
    IdleRecorder behind;

    owner->WaitForIdle(&destroyer.callback);
    owner->WaitForIdle(&behind.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    EXPECT_FALSE(owner);

    // The request the waiter started never reached the network, and the destructor said so.
    ASSERT_TRUE(second.Called());
    EXPECT_EQ(second.Status(), CHIP_ERROR_CANCELLED);
    EXPECT_TRUE(behind.called);
}

// An owner that never waits for idle can reclaim the registrar from the completion of its request
// instead, which is only safe because a registrar with no waiters has nothing left to do once the
// completion has been delivered. Heap-allocated so ASAN sees a read of the freed registrar.
TEST_F(TestNetworkIdentityManagementRegistrar, ACompletionCanDestroyARegistrarNobodyIsWaitingOn)
{
    auto owner = std::make_unique<NetworkIdentityManagementRegistrar>(mController, kNimNodeId, kNimEndpoint);

    struct Destroyer
    {
        std::unique_ptr<NetworkIdentityManagementRegistrar> & owner;
        std::optional<CHIP_ERROR> status;

        Callback::Callback<OnClientUnregisteredFunct> callback{ [](void * context, CHIP_ERROR aStatus) {
                                                                   auto * self  = static_cast<Destroyer *>(context);
                                                                   self->status = aStatus;
                                                                   self->owner.reset();
                                                               },
                                                                this };
    };
    Destroyer destroyer{ owner };
    owner->UnregisterClient(kClientIdentifier, &destroyer.callback);

    mController.FailPendingConnection(CHIP_ERROR_TIMEOUT);
    ASSERT_TRUE(destroyer.status.has_value());
    EXPECT_EQ(destroyer.status.value(), CHIP_ERROR_TIMEOUT);
    EXPECT_FALSE(owner);
}

} // namespace
