#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#include <limits>

using namespace chip;
using namespace chip::Protocols::UserDirectedCommissioning;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Internal;

struct TestUdcMessages : public ::testing::Test
{

    static void SetUpTestSuite() { ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR); }
    static void TearDownTestSuite() { chip::Platform::MemoryShutdown(); }
};

ByteSpan GetSpan(char * key)
{
    size_t len = strlen(key);
    // Stop the string from being null terminated to ensure the code makes no assumptions.
    key[len] = '1';
    return ByteSpan(Uint8::from_char(key), len);
}
class DLL_EXPORT TestCallback : public UserConfirmationProvider, public InstanceNameResolver
{
public:
    void OnUserDirectedCommissioningRequest(UDCClientState state)
    {
        mOnUserDirectedCommissioningRequestCalled = true;
        mState                                    = state;
    }

    void OnCancel(UDCClientState state) {}

    void OnCommissionerPasscodeReady(UDCClientState state) {}

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

TEST_F(TestUdcMessages, TestUDCServerClients)
{
    UserDirectedCommissioningServer udcServer;
    const char * instanceName1 = "servertest1";

    // test setting UDC Clients
    EXPECT_EQ(nullptr, udcServer.GetUDCClients().FindUDCClientState(instanceName1));
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    UDCClientState * state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UDCClientProcessingState::kUserDeclined, state->GetUDCClientProcessingState());
}

TEST_F(TestUdcMessages, TestUDCServerUserConfirmationProvider)
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

    Dnssd::DiscoveredNodeData discNodeData1;
    discNodeData1.Set<CommissionNodeData>();
    Dnssd::CommissionNodeData & nodeData1 = discNodeData1.Get<CommissionNodeData>();
    nodeData1.port                        = 5540;
    nodeData1.ipAddress[0]                = address;
    nodeData1.numIPs                      = 1;
    Platform::CopyString(nodeData1.instanceName, instanceName1);

    Dnssd::DiscoveredNodeData discNodeData2;
    discNodeData2.Set<CommissionNodeData>();
    Dnssd::CommissionNodeData & nodeData2 = discNodeData2.Get<CommissionNodeData>();
    nodeData2.port                        = 5540;
    nodeData2.ipAddress[0]                = address;
    nodeData2.numIPs                      = 1;
    nodeData2.longDiscriminator           = disc2;
    Platform::CopyString(nodeData2.instanceName, instanceName2);
    Platform::CopyString(nodeData2.deviceName, deviceName2);

    // test empty UserConfirmationProvider
    udcServer.OnCommissionableNodeFound(discNodeData2);
    udcServer.OnCommissionableNodeFound(discNodeData1);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UDCClientProcessingState::kUserDeclined, state->GetUDCClientProcessingState());
    // test other fields on UDCClientState
    EXPECT_STREQ(state->GetInstanceName(), instanceName1);
    // check that instance2 was found
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    EXPECT_EQ(nullptr, state);

    // test current state check
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    udcServer.SetUDCClientProcessingState((char *) instanceName2, UDCClientProcessingState::kDiscoveringNode);
    udcServer.OnCommissionableNodeFound(discNodeData2);
    udcServer.OnCommissionableNodeFound(discNodeData1);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UDCClientProcessingState::kUserDeclined, state->GetUDCClientProcessingState());
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UDCClientProcessingState::kPromptingUser, state->GetUDCClientProcessingState());
    // test other fields on UDCClientState
    EXPECT_STREQ(state->GetInstanceName(), instanceName2);
    EXPECT_STREQ(state->GetDeviceName(), deviceName2);
    EXPECT_EQ(state->GetLongDiscriminator(), disc2);

    // test non-empty UserConfirmationProvider
    udcServer.SetUserConfirmationProvider(&testCallback);
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);
    udcServer.SetUDCClientProcessingState((char *) instanceName2, UDCClientProcessingState::kDiscoveringNode);
    udcServer.OnCommissionableNodeFound(discNodeData1);
    EXPECT_FALSE(testCallback.mOnUserDirectedCommissioningRequestCalled);
    udcServer.OnCommissionableNodeFound(discNodeData2);
    EXPECT_TRUE(testCallback.mOnUserDirectedCommissioningRequestCalled);
    EXPECT_STREQ(testCallback.mState.GetInstanceName(), instanceName2);
}

#if 0
// TODO Fix test below
// the following test case is not reliable (fails on mac, clang platforms for example)
TEST_F(TestUdcMessages, TestUDCServerInstanceNameResolver)
{
    UserDirectedCommissioningServer udcServer;
    UserDirectedCommissioningClient udcClient;
    TestCallback testCallback;
    UDCClientState * state;
    const char * instanceName1 = "servertest1";

    // setup for tests
    auto mUdcTransportMgr = chip::Platform::MakeUnique<DeviceTransportMgr>();
    mUdcTransportMgr->SetSessionManager(&udcServer);
    udcServer.SetInstanceNameResolver(&testCallback);

    // set state for instance1
    udcServer.SetUDCClientProcessingState((char *) instanceName1, UDCClientProcessingState::kUserDeclined);

    // encode our client message
    char nameBuffer[Dnssd::Commission::kInstanceNameMaxLength + 1] = "Chris";
    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    udcClient.EncodeUDCMessage(payloadBuf);

    // prepare peerAddress for handleMessage
    Inet::IPAddress commissioner;
    Inet::IPAddress::FromString("127.0.0.1", commissioner);
    uint16_t port                      = 11100;
    Transport::PeerAddress peerAddress = Transport::PeerAddress::UDP(commissioner, port);

    // test OnMessageReceived
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // check if the state is set for the instance name sent
    state = udcServer.GetUDCClients().FindUDCClientState(nameBuffer);
    ASSERT_NE(nullptr, state);
    EXPECT_EQ(UDCClientProcessingState::kDiscoveringNode, state->GetUDCClientProcessingState());

    // check if a callback happened
    EXPECT_TRUE(testCallback.mFindCommissionableNodeCalled);

    // reset callback tracker so we can confirm that when the
    // same instance name is received, there is no callback
    testCallback.mFindCommissionableNodeCalled = false;

    payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));

    // reset the UDC message
    udcClient.EncodeUDCMessage(payloadBuf);

    // test OnMessageReceived again
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // verify it was not called
    EXPECT_FALSE(testCallback.mFindCommissionableNodeCalled);

    // next, reset the cache state and confirm the callback
    udcServer.ResetUDCClientProcessingStates();

    payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));

    // reset the UDC message
    udcClient.EncodeUDCMessage(payloadBuf);

    // test OnMessageReceived again
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // verify it was called
    EXPECT_TRUE(testCallback.mFindCommissionableNodeCalled);
}

#endif

TEST_F(TestUdcMessages, TestUserDirectedCommissioningClientMessage)
{
    char nameBuffer[Dnssd::Commission::kInstanceNameMaxLength + 1] = "Chris";
    System::PacketBufferHandle payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    UserDirectedCommissioningClient udcClient;

    // obtain the UDC message
    CHIP_ERROR err = udcClient.EncodeUDCMessage(payloadBuf);

    // check the packet header fields
    PacketHeader packetHeader;
    packetHeader.DecodeAndConsume(payloadBuf);
    EXPECT_FALSE(packetHeader.IsEncrypted());

    // check the payload header fields
    PayloadHeader payloadHeader;
    payloadHeader.DecodeAndConsume(payloadBuf);
    EXPECT_EQ(payloadHeader.GetMessageType(), to_underlying(MsgType::IdentificationDeclaration));
    EXPECT_EQ(payloadHeader.GetProtocolID(), Protocols::UserDirectedCommissioning::Id);
    EXPECT_FALSE(payloadHeader.NeedsAck());
    EXPECT_TRUE(payloadHeader.IsInitiator());

    // check the payload
    char instanceName[Dnssd::Commission::kInstanceNameMaxLength + 1];
    size_t instanceNameLength = std::min<size_t>(payloadBuf->DataLength(), Dnssd::Commission::kInstanceNameMaxLength);
    payloadBuf->Read(Uint8::from_char(instanceName), instanceNameLength);
    instanceName[instanceNameLength] = '\0';
    ChipLogProgress(Inet, "UDC instance=%s", instanceName);
    EXPECT_STREQ(instanceName, nameBuffer);

    // verify no errors
    EXPECT_EQ(err, CHIP_NO_ERROR);
}

TEST_F(TestUdcMessages, TestUDCClients)
{
    UDCClients<3> mUdcClients;
    const char * instanceName1 = "test1";
    const char * instanceName2 = "test2";
    const char * instanceName3 = "test3";
    const char * instanceName4 = "test4";

    // test base case
    UDCClientState * state = mUdcClients.FindUDCClientState(instanceName1);
    ASSERT_EQ(state, nullptr);

    // test max size
    EXPECT_EQ(CHIP_NO_ERROR, mUdcClients.CreateNewUDCClientState(instanceName1, &state));
    EXPECT_EQ(CHIP_NO_ERROR, mUdcClients.CreateNewUDCClientState(instanceName2, &state));
    EXPECT_EQ(CHIP_NO_ERROR, mUdcClients.CreateNewUDCClientState(instanceName3, &state));
    EXPECT_EQ(CHIP_ERROR_NO_MEMORY, mUdcClients.CreateNewUDCClientState(instanceName4, &state));

    // test reset
    mUdcClients.ResetUDCClientStates();
    EXPECT_EQ(CHIP_NO_ERROR, mUdcClients.CreateNewUDCClientState(instanceName4, &state));

    // test find
    EXPECT_EQ(nullptr, mUdcClients.FindUDCClientState(instanceName1));
    EXPECT_EQ(nullptr, mUdcClients.FindUDCClientState(instanceName2));
    EXPECT_EQ(nullptr, mUdcClients.FindUDCClientState(instanceName3));
    state = mUdcClients.FindUDCClientState(instanceName4);
    ASSERT_NE(nullptr, state);

    // test expiry
    state->Reset();
    EXPECT_EQ(nullptr, mUdcClients.FindUDCClientState(instanceName4));

    // test re-activation
    EXPECT_EQ(CHIP_NO_ERROR, mUdcClients.CreateNewUDCClientState(instanceName4, &state));
    System::Clock::Timestamp expirationTime = state->GetExpirationTime();
    state->SetExpirationTime(expirationTime - System::Clock::Milliseconds64(1));
    EXPECT_EQ((expirationTime - System::Clock::Milliseconds64(1)), state->GetExpirationTime());
    mUdcClients.MarkUDCClientActive(state);
    EXPECT_LT((expirationTime - System::Clock::Milliseconds64(1)), state->GetExpirationTime());
}

TEST_F(TestUdcMessages, TestUDCClientState)
{
    UDCClients<3> mUdcClients;
    const char * instanceName1 = "test1";
    Inet::IPAddress address;
    Inet::IPAddress::FromString("127.0.0.1", address);
    uint16_t port              = 333;
    uint16_t longDiscriminator = 1234;
    uint16_t vendorId          = 1111;
    uint16_t productId         = 2222;
    const char * deviceName    = "test name";

    // Rotating ID is given as up to 50 hex bytes
    char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1];
    uint8_t rotatingId[chip::Dnssd::kMaxRotatingIdLen];
    size_t rotatingIdLen;
    strcpy(rotatingIdString, "92873498273948734534");
    GetRotatingDeviceId(GetSpan(rotatingIdString), rotatingId, &rotatingIdLen);

    // create a Rotating ID longer than kMaxRotatingIdLen
    char rotatingIdLongString[chip::Dnssd::kMaxRotatingIdLen * 4 + 1];
    uint8_t rotatingIdLong[chip::Dnssd::kMaxRotatingIdLen * 2];
    size_t rotatingIdLongLen;
    strcpy(
        rotatingIdLongString,
        "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890");

    const ByteSpan & value = GetSpan(rotatingIdLongString);
    rotatingIdLongLen      = Encoding::HexToBytes(reinterpret_cast<const char *>(value.data()), value.size(), rotatingIdLong,
                                                  chip::Dnssd::kMaxRotatingIdLen * 2);

    EXPECT_GT(rotatingIdLongLen, chip::Dnssd::kMaxRotatingIdLen);

    // test base case
    UDCClientState * state = mUdcClients.FindUDCClientState(instanceName1);
    ASSERT_EQ(state, nullptr);

    // add a default state
    EXPECT_EQ(CHIP_NO_ERROR, mUdcClients.CreateNewUDCClientState(instanceName1, &state));

    // get the state
    state = mUdcClients.FindUDCClientState(instanceName1);
    ASSERT_NE(nullptr, state);
    EXPECT_STREQ(state->GetInstanceName(), instanceName1);

    state->SetPeerAddress(chip::Transport::PeerAddress::UDP(address, port));
    EXPECT_EQ(port, state->GetPeerAddress().GetPort());

    state->SetDeviceName(deviceName);
    EXPECT_STREQ(state->GetDeviceName(), deviceName);

    state->SetLongDiscriminator(longDiscriminator);
    EXPECT_EQ(longDiscriminator, state->GetLongDiscriminator());

    state->SetVendorId(vendorId);
    EXPECT_EQ(vendorId, state->GetVendorId());

    state->SetProductId(productId);
    EXPECT_EQ(productId, state->GetProductId());

    state->SetRotatingId(rotatingId, rotatingIdLen);
    EXPECT_EQ(rotatingIdLen, state->GetRotatingIdLength());

    const uint8_t * testRotatingId = state->GetRotatingId();
    for (size_t i = 0; i < rotatingIdLen; i++)
    {
        EXPECT_EQ(testRotatingId[i], rotatingId[i]);
    }

    state->SetRotatingId(rotatingIdLong, rotatingIdLongLen);

    EXPECT_EQ(chip::Dnssd::kMaxRotatingIdLen, state->GetRotatingIdLength());

    const uint8_t * testRotatingIdLong = state->GetRotatingId();
    for (size_t i = 0; i < chip::Dnssd::kMaxRotatingIdLen; i++)
    {
        EXPECT_EQ(testRotatingIdLong[i], rotatingIdLong[i]);
    }
}

TEST_F(TestUdcMessages, TestUDCIdentificationDeclaration)
{
    IdentificationDeclaration id;
    IdentificationDeclaration idOut;

    const char * instanceName = "servertest1";
    uint16_t vendorId         = 1111;
    uint16_t productId        = 2222;
    uint16_t port             = 123;
    const char * deviceName   = "device1";
    uint16_t pairingHint      = 33;
    const char * pairingInst  = "Read 6 digit code from screen";

    TargetAppInfo appInfo;

    // Rotating ID is given as up to 50 hex bytes
    char rotatingIdString[chip::Dnssd::kMaxRotatingIdLen * 2 + 1];
    uint8_t rotatingId[chip::Dnssd::kMaxRotatingIdLen];
    size_t rotatingIdLen;
    strcpy(rotatingIdString, "92873498273948734534");
    GetRotatingDeviceId(GetSpan(rotatingIdString), rotatingId, &rotatingIdLen);

    id.SetInstanceName(instanceName);
    id.SetVendorId(vendorId);
    id.SetProductId(productId);
    id.SetDeviceName(deviceName);
    id.SetPairingInst(pairingInst);
    id.SetPairingHint(pairingHint);
    id.SetRotatingId(rotatingId, rotatingIdLen);
    id.SetCdPort(port);

    id.SetNoPasscode(true);

    appInfo.vendorId  = 1;
    appInfo.productId = 9;
    id.AddTargetAppInfo(appInfo);

    appInfo.vendorId  = 2;
    appInfo.productId = 8;
    id.AddTargetAppInfo(appInfo);

    appInfo.vendorId  = 3;
    appInfo.productId = 7;
    id.AddTargetAppInfo(appInfo);

    id.SetCdUponPasscodeDialog(true);
    id.SetCommissionerPasscode(true);
    id.SetCommissionerPasscodeReady(true);

    EXPECT_TRUE(id.HasDiscoveryInfo());
    EXPECT_STREQ(id.GetInstanceName(), instanceName);
    EXPECT_EQ(vendorId, id.GetVendorId());
    EXPECT_EQ(productId, id.GetProductId());
    EXPECT_EQ(port, id.GetCdPort());
    EXPECT_STREQ(id.GetDeviceName(), deviceName);
    EXPECT_EQ(rotatingIdLen, id.GetRotatingIdLength());
    EXPECT_EQ(memcmp(id.GetRotatingId(), rotatingId, rotatingIdLen), 0);
    EXPECT_EQ(pairingHint, id.GetPairingHint());
    EXPECT_STREQ(id.GetPairingInst(), pairingInst);

    EXPECT_EQ(id.GetNumTargetAppInfos(), 3);
    EXPECT_TRUE(id.GetTargetAppInfo(0, appInfo) && appInfo.vendorId == 1 && appInfo.productId == 9);
    EXPECT_TRUE(id.GetTargetAppInfo(1, appInfo) && appInfo.vendorId == 2 && appInfo.productId == 8);
    EXPECT_TRUE(id.GetTargetAppInfo(2, appInfo) && appInfo.vendorId == 3 && appInfo.productId == 7);
    EXPECT_EQ(id.GetNoPasscode(), true);
    EXPECT_EQ(id.GetCdUponPasscodeDialog(), true);
    EXPECT_EQ(id.GetCommissionerPasscode(), true);
    EXPECT_EQ(id.GetCommissionerPasscodeReady(), true);

    // TODO: add an ip

    uint8_t idBuffer[500];
    id.WritePayload(idBuffer, sizeof(idBuffer));

    // next, parse this object
    idOut.ReadPayload(idBuffer, sizeof(idBuffer));

    EXPECT_TRUE(idOut.HasDiscoveryInfo());
    EXPECT_STREQ(idOut.GetInstanceName(), instanceName);
    EXPECT_EQ(vendorId, idOut.GetVendorId());
    EXPECT_EQ(productId, idOut.GetProductId());
    EXPECT_EQ(port, idOut.GetCdPort());
    EXPECT_STREQ(idOut.GetDeviceName(), deviceName);
    EXPECT_EQ(rotatingIdLen, idOut.GetRotatingIdLength());
    EXPECT_EQ(memcmp(idOut.GetRotatingId(), rotatingId, rotatingIdLen), 0);
    EXPECT_STREQ(idOut.GetPairingInst(), pairingInst);
    EXPECT_EQ(pairingHint, idOut.GetPairingHint());

    EXPECT_EQ(id.GetNumTargetAppInfos(), idOut.GetNumTargetAppInfos());
    EXPECT_TRUE(idOut.GetTargetAppInfo(0, appInfo) && appInfo.vendorId == 1 && appInfo.productId == 9);
    EXPECT_TRUE(idOut.GetTargetAppInfo(1, appInfo) && appInfo.vendorId == 2 && appInfo.productId == 8);
    EXPECT_TRUE(idOut.GetTargetAppInfo(2, appInfo) && appInfo.vendorId == 3 && appInfo.productId == 7);

    EXPECT_EQ(id.GetNoPasscode(), idOut.GetNoPasscode());
    EXPECT_EQ(id.GetCdUponPasscodeDialog(), idOut.GetCdUponPasscodeDialog());
    EXPECT_EQ(id.GetCommissionerPasscode(), idOut.GetCommissionerPasscode());
    EXPECT_EQ(id.GetCommissionerPasscodeReady(), idOut.GetCommissionerPasscodeReady());

    // TODO: remove following "force-fail" debug line
    // NL_TEST_ASSERT(inSuite, rotatingIdLen != id.GetRotatingIdLength());
}

TEST_F(TestUdcMessages, TestUDCCommissionerDeclaration)
{
    CommissionerDeclaration id;
    CommissionerDeclaration idOut;

    CommissionerDeclaration::CdError errorCode = CommissionerDeclaration::CdError::kCaseConnectionFailed;

    id.SetErrorCode(errorCode);
    id.SetNeedsPasscode(true);
    id.SetNoAppsFound(true);
    id.SetPasscodeDialogDisplayed(true);
    id.SetCommissionerPasscode(true);
    id.SetQRCodeDisplayed(true);

    EXPECT_EQ(errorCode, id.GetErrorCode());
    EXPECT_EQ(id.GetNeedsPasscode(), true);
    EXPECT_EQ(id.GetNoAppsFound(), true);
    EXPECT_EQ(id.GetPasscodeDialogDisplayed(), true);
    EXPECT_EQ(id.GetCommissionerPasscode(), true);
    EXPECT_EQ(id.GetQRCodeDisplayed(), true);

    uint8_t idBuffer[500];
    id.WritePayload(idBuffer, sizeof(idBuffer));

    // next, parse this object
    idOut.ReadPayload(idBuffer, sizeof(idBuffer));

    EXPECT_EQ(errorCode, idOut.GetErrorCode());
    EXPECT_EQ(id.GetNeedsPasscode(), idOut.GetNeedsPasscode());
    EXPECT_EQ(id.GetNoAppsFound(), idOut.GetNoAppsFound());
    EXPECT_EQ(id.GetPasscodeDialogDisplayed(), idOut.GetPasscodeDialogDisplayed());
    EXPECT_EQ(id.GetCommissionerPasscode(), idOut.GetCommissionerPasscode());
    EXPECT_EQ(id.GetQRCodeDisplayed(), idOut.GetQRCodeDisplayed());
}
