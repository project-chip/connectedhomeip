#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

#include <nlunit-test.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#include <limits>

using namespace chip;
using namespace chip::Protocols::UserDirectedCommissioning;

class DLL_EXPORT TestCallback : public UserConfirmationProvider, public InstanceNameResolver
{
public:
    void OnUserDirectedCommissioningRequest(UDCClientState state)
    {
        mOnUserDirectedCommissioningRequestCalled = true;
        mState                                    = state;
    }

    void FindCommissionableNode(char * instanceName)
    {
        mFindCommissionableNodeCalled = true;
        mInstanceName                 = instanceName;
    }

    // virtual ~UserConfirmationProvider() = default;
    UDCClientState mState;
    char * mInstanceName;

    bool mOnUserDirectedCommissioningRequestCalled = false;
    bool mFindCommissionableNodeCalled             = false;
};

using DeviceTransportMgr = TransportMgr<Transport::UDP>;

void TestUDCServerClients(nlTestSuite * inSuite, void * inContext)
{
    UserDirectedCommissioningServer udcServer;
    const char * instanceName1 = "servertest1";

    // test setting UDC Clients
    NL_TEST_ASSERT(inSuite, nullptr == udcServer.GetUDCClients().FindUDCClientState(instanceName1));
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    UDCClientState * state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kUserDeclined == state->GetUDCClientProcessingState());
}

void TestUDCServerUserConfirmationProvider(nlTestSuite * inSuite, void * inContext)
{
    UserDirectedCommissioningServer udcServer;
    TestCallback testCallback;
    const char * instanceName1 = "servertest1";
    const char * instanceName2 = "servertest2";
    const char * deviceName2   = "device1";
    uint16_t disc2             = 1234;
    UDCClientState * state;

    chip::Inet::IPAddress address;
    chip::Inet::IPAddress::FromString("127.0.0.1", address); // need to populate with something

    // setup for tests
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);

    Dnssd::DiscoveredNodeData nodeData1;
    nodeData1.port         = 5540;
    nodeData1.ipAddress[0] = address;
    nodeData1.numIPs       = 1;
    strncpy((char *) nodeData1.instanceName, instanceName1, sizeof(nodeData1.instanceName));

    Dnssd::DiscoveredNodeData nodeData2;
    nodeData2.port              = 5540;
    nodeData2.ipAddress[0]      = address;
    nodeData2.numIPs            = 1;
    nodeData2.longDiscriminator = disc2;
    strncpy((char *) nodeData2.instanceName, instanceName2, sizeof(nodeData2.instanceName));
    strncpy((char *) nodeData2.deviceName, deviceName2, sizeof(nodeData2.deviceName));

    // test empty UserConfirmationProvider
    udcServer.OnCommissionableNodeFound(nodeData2);
    udcServer.OnCommissionableNodeFound(nodeData1);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kUserDeclined == state->GetUDCClientProcessingState());
    // test other fields on UDCClientState
    NL_TEST_ASSERT(inSuite, 0 == strcmp(state->GetInstanceName(), instanceName1));
    // check that instance2 was found
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    NL_TEST_ASSERT(inSuite, nullptr == state);

    // test current state check
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    udcServer.SetUDCClientProcessingState((char *) instanceName2, UDCClientProcessingState::kDiscoveringNode);
    udcServer.OnCommissionableNodeFound(nodeData2);
    udcServer.OnCommissionableNodeFound(nodeData1);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kUserDeclined == state->GetUDCClientProcessingState());
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kPromptingUser == state->GetUDCClientProcessingState());
    // test other fields on UDCClientState
    NL_TEST_ASSERT(inSuite, 0 == strcmp(state->GetInstanceName(), instanceName2));
    NL_TEST_ASSERT(inSuite, 0 == strcmp(state->GetDeviceName(), deviceName2));
    NL_TEST_ASSERT(inSuite, state->GetLongDiscriminator() == disc2);

    // test non-empty UserConfirmationProvider
    udcServer.SetUserConfirmationProvider(&testCallback);
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    udcServer.SetUDCClientProcessingState((char *) instanceName2, UDCClientProcessingState::kDiscoveringNode);
    udcServer.OnCommissionableNodeFound(nodeData1);
    NL_TEST_ASSERT(inSuite, !testCallback.mOnUserDirectedCommissioningRequestCalled);
    udcServer.OnCommissionableNodeFound(nodeData2);
    NL_TEST_ASSERT(inSuite, testCallback.mOnUserDirectedCommissioningRequestCalled);
    NL_TEST_ASSERT(inSuite, 0 == strcmp(testCallback.mState.GetInstanceName(), instanceName2));
}

void TestUDCServerInstanceNameResolver(nlTestSuite * inSuite, void * inContext)
{
    UserDirectedCommissioningServer udcServer;
    UserDirectedCommissioningClient udcClient;
    TestCallback testCallback;
    UDCClientState * state;
    const char * instanceName1 = "servertest1";

    // setup for tests
    DeviceTransportMgr * mUdcTransportMgr = chip::Platform::New<DeviceTransportMgr>();
    mUdcTransportMgr->SetSessionManager(&udcServer);
    udcServer.SetInstanceNameResolver(&testCallback);

    // set state for instance1
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);

    // encode our client message
    char nameBuffer[Dnssd::Commission::kInstanceNameMaxLength + 1] = "Chris";
    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    udcClient.EncodeUDCMessage(std::move(payloadBuf));

    // prepare peerAddress for handleMessage
    Inet::IPAddress commissioner;
    Inet::IPAddress::FromString("127.0.0.1", commissioner);
    uint16_t port                      = 11100;
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(commissioner, port);

    // test OnMessageReceived
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // check if the state is set for the instance name sent
    state = udcServer.GetUDCClients().FindUDCClientState(nameBuffer);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kDiscoveringNode == state->GetUDCClientProcessingState());

    // check if a callback happened
    NL_TEST_ASSERT(inSuite, testCallback.mFindCommissionableNodeCalled);

    // reset callback tracker so we can confirm that when the
    // same instance name is received, there is no callback
    testCallback.mFindCommissionableNodeCalled = false;

    // reset the UDC message
    udcClient.EncodeUDCMessage(std::move(payloadBuf));

    // test OnMessageReceived again
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // verify it was not called
    NL_TEST_ASSERT(inSuite, !testCallback.mFindCommissionableNodeCalled);

    // next, reset the cache state and confirm the callback
    udcServer.ResetUDCClientProcessingStates();

    // reset the UDC message
    udcClient.EncodeUDCMessage(std::move(payloadBuf));

    // test OnMessageReceived again
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // verify it was called
    NL_TEST_ASSERT(inSuite, testCallback.mFindCommissionableNodeCalled);
}

void TestUserDirectedCommissioningClientMessage(nlTestSuite * inSuite, void * inContext)
{
    char nameBuffer[Dnssd::Commission::kInstanceNameMaxLength + 1] = "Chris";
    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    UserDirectedCommissioningClient udcClient;

    // obtain the UDC message
    CHIP_ERROR err = udcClient.EncodeUDCMessage(std::move(payloadBuf));

    // check the packet header fields
    PacketHeader packetHeader;
    packetHeader.DecodeAndConsume(payloadBuf);
    NL_TEST_ASSERT(inSuite, !packetHeader.IsEncrypted());

    // check the payload header fields
    PayloadHeader payloadHeader;
    payloadHeader.DecodeAndConsume(payloadBuf);
    NL_TEST_ASSERT(inSuite, payloadHeader.GetMessageType() == to_underlying(MsgType::IdentificationDeclaration));
    NL_TEST_ASSERT(inSuite, payloadHeader.GetProtocolID() == Protocols::UserDirectedCommissioning::Id);
    NL_TEST_ASSERT(inSuite, !payloadHeader.NeedsAck());
    NL_TEST_ASSERT(inSuite, payloadHeader.IsInitiator());

    // check the payload
    char instanceName[Dnssd::Commission::kInstanceNameMaxLength + 1];
    size_t instanceNameLength = std::min<size_t>(payloadBuf->DataLength(), Dnssd::Commission::kInstanceNameMaxLength);
    payloadBuf->Read(Uint8::from_char(instanceName), instanceNameLength);
    instanceName[instanceNameLength] = '\0';
    ChipLogProgress(Inet, "UDC instance=%s", instanceName);
    NL_TEST_ASSERT(inSuite, strcmp(instanceName, nameBuffer) == 0);

    // verify no errors
    NL_TEST_ASSERT(inSuite, err == CHIP_NO_ERROR);
}

void TestUDCClients(nlTestSuite * inSuite, void * inContext)
{
    UDCClients<3> mUdcClients;
    const char * instanceName1 = "test1";
    const char * instanceName2 = "test2";
    const char * instanceName3 = "test3";
    const char * instanceName4 = "test4";

    // test base case
    UDCClientState * state = mUdcClients.FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, state == nullptr);

    // test max size
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == mUdcClients.CreateNewUDCClientState(instanceName1, &state));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == mUdcClients.CreateNewUDCClientState(instanceName2, &state));
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == mUdcClients.CreateNewUDCClientState(instanceName3, &state));
    NL_TEST_ASSERT(inSuite, CHIP_ERROR_NO_MEMORY == mUdcClients.CreateNewUDCClientState(instanceName4, &state));

    // test reset
    mUdcClients.ResetUDCClientStates();
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == mUdcClients.CreateNewUDCClientState(instanceName4, &state));

    // test find
    NL_TEST_ASSERT(inSuite, nullptr == mUdcClients.FindUDCClientState(instanceName1));
    NL_TEST_ASSERT(inSuite, nullptr == mUdcClients.FindUDCClientState(instanceName2));
    NL_TEST_ASSERT(inSuite, nullptr == mUdcClients.FindUDCClientState(instanceName3));
    state = mUdcClients.FindUDCClientState(instanceName4);
    NL_TEST_ASSERT(inSuite, nullptr != state);

    // test expiry
    state->Reset();
    NL_TEST_ASSERT(inSuite, nullptr == mUdcClients.FindUDCClientState(instanceName4));

    // test re-activation
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == mUdcClients.CreateNewUDCClientState(instanceName4, &state));
    System::Clock::Timestamp expirationTime = state->GetExpirationTime();
    state->SetExpirationTime(expirationTime - System::Clock::Milliseconds64(1));
    NL_TEST_ASSERT(inSuite, (expirationTime - System::Clock::Milliseconds64(1)) == state->GetExpirationTime());
    mUdcClients.MarkUDCClientActive(state);
    NL_TEST_ASSERT(inSuite, (expirationTime - System::Clock::Milliseconds64(1)) < state->GetExpirationTime());
}

// Test Suite

/**
 *  Test Suite that lists all the test functions.
 */
// clang-format off
static const nlTest sTests[] =
{
    NL_TEST_DEF("TestUDCServerClients", TestUDCServerClients),
    NL_TEST_DEF("TestUDCServerUserConfirmationProvider", TestUDCServerUserConfirmationProvider),
    NL_TEST_DEF("TestUDCServerInstanceNameResolver", TestUDCServerInstanceNameResolver),
    NL_TEST_DEF("TestUserDirectedCommissioningClientMessage", TestUserDirectedCommissioningClientMessage),
    NL_TEST_DEF("TestUDCClients", TestUDCClients),

    NL_TEST_SENTINEL()
};
// clang-format on

/**
 *  Set up the test suite.
 */
static int TestSetup(void * inContext)
{
    CHIP_ERROR error = chip::Platform::MemoryInit();
    if (error != CHIP_NO_ERROR)
        return FAILURE;
    return SUCCESS;
}

/**
 *  Tear down the test suite.
 */
static int TestTeardown(void * inContext)
{
    chip::Platform::MemoryShutdown();
    return SUCCESS;
}

// clang-format off
static nlTestSuite sSuite =
{
    "Test-CHIP-UdcMessages",
    &sTests[0],
    TestSetup,
    TestTeardown,
};
// clang-format on

/**
 *  Main
 */
int TestUdcMessages()
{
    // Run test suit against one context
    nlTestRunner(&sSuite, nullptr);

    return (nlTestRunnerStats(&sSuite));
}

CHIP_REGISTER_TEST_SUITE(TestUdcMessages)
