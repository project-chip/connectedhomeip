#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

#include <nlunit-test.h>

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
    void OnUserDirectedCommissioningRequest(const Mdns::DiscoveredNodeData & nodeData)
    {
        mOnUserDirectedCommissioningRequestCalled = true;
        mNodeData                                 = nodeData;
    }

    void FindCommissionableNode(char * instanceName)
    {
        mFindCommissionableNodeCalled = true;
        mInstanceName                 = instanceName;
    }

    // virtual ~UserConfirmationProvider() = default;
    Mdns::DiscoveredNodeData mNodeData;
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
    UDCClientState * state;

    // setup for tests
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);

    // test empty UserConfirmationProvider
    Mdns::DiscoveredNodeData nodeData;
    strncpy((char *) nodeData.instanceName, instanceName2, sizeof(nodeData.instanceName));
    udcServer.OnCommissionableNodeFound(nodeData);
    strncpy((char *) nodeData.instanceName, instanceName1, sizeof(nodeData.instanceName));
    udcServer.OnCommissionableNodeFound(nodeData);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kUserDeclined == state->GetUDCClientProcessingState());
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    NL_TEST_ASSERT(inSuite, nullptr == state);

    // test current state check
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    udcServer.SetUDCClientProcessingState((char *) instanceName2, UDCClientProcessingState::kDiscoveringNode);
    strncpy((char *) nodeData.instanceName, instanceName2, sizeof(nodeData.instanceName));
    udcServer.OnCommissionableNodeFound(nodeData);
    strncpy((char *) nodeData.instanceName, instanceName1, sizeof(nodeData.instanceName));
    udcServer.OnCommissionableNodeFound(nodeData);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kUserDeclined == state->GetUDCClientProcessingState());
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    NL_TEST_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kPromptingUser == state->GetUDCClientProcessingState());

    // test non-empty UserConfirmationProvider
    udcServer.SetUserConfirmationProvider(&testCallback);
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    udcServer.SetUDCClientProcessingState((char *) instanceName2, UDCClientProcessingState::kDiscoveringNode);
    strncpy((char *) nodeData.instanceName, instanceName1, sizeof(nodeData.instanceName));
    udcServer.OnCommissionableNodeFound(nodeData);
    NL_TEST_ASSERT(inSuite, !testCallback.mOnUserDirectedCommissioningRequestCalled);
    strncpy((char *) nodeData.instanceName, instanceName2, sizeof(nodeData.instanceName));
    udcServer.OnCommissionableNodeFound(nodeData);
    NL_TEST_ASSERT(inSuite, testCallback.mOnUserDirectedCommissioningRequestCalled);
    NL_TEST_ASSERT(inSuite, 0 == strcmp(testCallback.mNodeData.instanceName, instanceName2));
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
    mUdcTransportMgr->SetSecureSessionMgr(&udcServer);
    udcServer.SetInstanceNameResolver(&testCallback);

    // set state for instance1
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);

    // encode our client message
    char nameBuffer[Mdns::kMaxInstanceNameSize + 1] = "Chris";
    System::PacketBufferHandle payloadBuf           = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
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
    char nameBuffer[Mdns::kMaxInstanceNameSize + 1] = "Chris";
    System::PacketBufferHandle payloadBuf           = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    UserDirectedCommissioningClient udcClient;

    // obtain the UDC message
    CHIP_ERROR err = udcClient.EncodeUDCMessage(std::move(payloadBuf));

    // check the packet header fields
    PacketHeader packetHeader;
    packetHeader.DecodeAndConsume(payloadBuf);
    NL_TEST_ASSERT(inSuite, !packetHeader.GetFlags().Has(Header::FlagValues::kEncryptedMessage));

    // check the payload header fields
    PayloadHeader payloadHeader;
    payloadHeader.DecodeAndConsume(payloadBuf);
    NL_TEST_ASSERT(inSuite, payloadHeader.GetMessageType() == to_underlying(MsgType::IdentificationDeclaration));
    NL_TEST_ASSERT(inSuite, payloadHeader.GetProtocolID() == Protocols::UserDirectedCommissioning::Id);
    NL_TEST_ASSERT(inSuite, !payloadHeader.NeedsAck());
    NL_TEST_ASSERT(inSuite, payloadHeader.IsInitiator());

    // check the payload
    char instanceName[chip::Mdns::kMaxInstanceNameSize + 1];
    size_t instanceNameLength = (payloadBuf->DataLength() > (chip::Mdns::kMaxInstanceNameSize)) ? chip::Mdns::kMaxInstanceNameSize
                                                                                                : payloadBuf->DataLength();
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
    uint64_t expirationTime = state->GetExpirationTimeMs();
    state->SetExpirationTimeMs(expirationTime - 1);
    NL_TEST_ASSERT(inSuite, (expirationTime - 1) == state->GetExpirationTimeMs());
    mUdcClients.MarkUDCClientActive(state);
    NL_TEST_ASSERT(inSuite, (expirationTime - 1) < state->GetExpirationTimeMs());
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
