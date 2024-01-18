#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>

#include <nlunit-test.h>

#include <lib/core/CHIPSafeCasts.h>
#include <lib/dnssd/TxtFields.h>
#include <lib/support/BufferWriter.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>
#include <transport/TransportMgr.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/UDP.h>

#include <limits>

using namespace chip;
using namespace chip::Protocols::UserDirectedCommissioning;
using namespace chip::Dnssd;
using namespace chip::Dnssd::Internal;

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
    NL_TEST_EXIT_ON_FAILED_ASSERT(inSuite, nullptr != state);
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
    nodeData1.resolutionData.port         = 5540;
    nodeData1.resolutionData.ipAddress[0] = address;
    nodeData1.resolutionData.numIPs       = 1;
    Platform::CopyString(nodeData1.commissionData.instanceName, instanceName1);

    Dnssd::DiscoveredNodeData nodeData2;
    nodeData2.resolutionData.port              = 5540;
    nodeData2.resolutionData.ipAddress[0]      = address;
    nodeData2.resolutionData.numIPs            = 1;
    nodeData2.commissionData.longDiscriminator = disc2;
    Platform::CopyString(nodeData2.commissionData.instanceName, instanceName2);
    Platform::CopyString(nodeData2.commissionData.deviceName, deviceName2);

    // test empty UserConfirmationProvider
    udcServer.OnCommissionableNodeFound(nodeData2);
    udcServer.OnCommissionableNodeFound(nodeData1);
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName1);
    NL_TEST_EXIT_ON_FAILED_ASSERT(inSuite, nullptr != state);
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
    NL_TEST_EXIT_ON_FAILED_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kUserDeclined == state->GetUDCClientProcessingState());
    state = udcServer.GetUDCClients().FindUDCClientState(instanceName2);
    NL_TEST_EXIT_ON_FAILED_ASSERT(inSuite, nullptr != state);
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
    NL_TEST_EXIT_ON_FAILED_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, UDCClientProcessingState::kDiscoveringNode == state->GetUDCClientProcessingState());

    // check if a callback happened
    NL_TEST_ASSERT(inSuite, testCallback.mFindCommissionableNodeCalled);

    // reset callback tracker so we can confirm that when the
    // same instance name is received, there is no callback
    testCallback.mFindCommissionableNodeCalled = false;

    payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));

    // reset the UDC message
    udcClient.EncodeUDCMessage(payloadBuf);

    // test OnMessageReceived again
    mUdcTransportMgr->HandleMessageReceived(peerAddress, std::move(payloadBuf));

    // verify it was not called
    NL_TEST_ASSERT(inSuite, !testCallback.mFindCommissionableNodeCalled);

    // next, reset the cache state and confirm the callback
    udcServer.ResetUDCClientProcessingStates();

    payloadBuf = MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));

    // reset the UDC message
    udcClient.EncodeUDCMessage(payloadBuf);

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
    CHIP_ERROR err = udcClient.EncodeUDCMessage(payloadBuf);

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

void TestUDCClientState(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, rotatingIdLongLen > chip::Dnssd::kMaxRotatingIdLen);

    // test base case
    UDCClientState * state = mUdcClients.FindUDCClientState(instanceName1);
    NL_TEST_ASSERT(inSuite, state == nullptr);

    // add a default state
    NL_TEST_ASSERT(inSuite, CHIP_NO_ERROR == mUdcClients.CreateNewUDCClientState(instanceName1, &state));

    // get the state
    state = mUdcClients.FindUDCClientState(instanceName1);
    NL_TEST_EXIT_ON_FAILED_ASSERT(inSuite, nullptr != state);
    NL_TEST_ASSERT(inSuite, strcmp(state->GetInstanceName(), instanceName1) == 0);

    state->SetPeerAddress(chip::Transport::PeerAddress::UDP(address, port));
    NL_TEST_ASSERT(inSuite, port == state->GetPeerAddress().GetPort());

    state->SetDeviceName(deviceName);
    NL_TEST_ASSERT(inSuite, strcmp(state->GetDeviceName(), deviceName) == 0);

    state->SetLongDiscriminator(longDiscriminator);
    NL_TEST_ASSERT(inSuite, longDiscriminator == state->GetLongDiscriminator());

    state->SetVendorId(vendorId);
    NL_TEST_ASSERT(inSuite, vendorId == state->GetVendorId());

    state->SetProductId(productId);
    NL_TEST_ASSERT(inSuite, productId == state->GetProductId());

    state->SetRotatingId(rotatingId, rotatingIdLen);
    NL_TEST_ASSERT(inSuite, rotatingIdLen == state->GetRotatingIdLength());

    const uint8_t * testRotatingId = state->GetRotatingId();
    for (size_t i = 0; i < rotatingIdLen; i++)
    {
        NL_TEST_ASSERT(inSuite, testRotatingId[i] == rotatingId[i]);
    }

    state->SetRotatingId(rotatingIdLong, rotatingIdLongLen);

    NL_TEST_ASSERT(inSuite, chip::Dnssd::kMaxRotatingIdLen == state->GetRotatingIdLength());

    const uint8_t * testRotatingIdLong = state->GetRotatingId();
    for (size_t i = 0; i < chip::Dnssd::kMaxRotatingIdLen; i++)
    {
        NL_TEST_ASSERT(inSuite, testRotatingIdLong[i] == rotatingIdLong[i]);
    }
}

void TestUDCIdentificationDeclaration(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, id.HasDiscoveryInfo());
    NL_TEST_ASSERT(inSuite, strcmp(id.GetInstanceName(), instanceName) == 0);
    NL_TEST_ASSERT(inSuite, vendorId == id.GetVendorId());
    NL_TEST_ASSERT(inSuite, productId == id.GetProductId());
    NL_TEST_ASSERT(inSuite, port == id.GetCdPort());
    NL_TEST_ASSERT(inSuite, strcmp(id.GetDeviceName(), deviceName) == 0);
    NL_TEST_ASSERT(inSuite, rotatingIdLen == id.GetRotatingIdLength());
    NL_TEST_ASSERT(inSuite, memcmp(id.GetRotatingId(), rotatingId, rotatingIdLen) == 0);
    NL_TEST_ASSERT(inSuite, pairingHint == id.GetPairingHint());
    NL_TEST_ASSERT(inSuite, strcmp(id.GetPairingInst(), pairingInst) == 0);

    NL_TEST_ASSERT(inSuite, id.GetNumTargetAppInfos() == 3);
    NL_TEST_ASSERT(inSuite, id.GetTargetAppInfo(0, appInfo) && appInfo.vendorId == 1 && appInfo.productId == 9);
    NL_TEST_ASSERT(inSuite, id.GetTargetAppInfo(1, appInfo) && appInfo.vendorId == 2 && appInfo.productId == 8);
    NL_TEST_ASSERT(inSuite, id.GetTargetAppInfo(2, appInfo) && appInfo.vendorId == 3 && appInfo.productId == 7);
    NL_TEST_ASSERT(inSuite, id.GetNoPasscode() == true);
    NL_TEST_ASSERT(inSuite, id.GetCdUponPasscodeDialog() == true);
    NL_TEST_ASSERT(inSuite, id.GetCommissionerPasscode() == true);
    NL_TEST_ASSERT(inSuite, id.GetCommissionerPasscodeReady() == true);

    // TODO: add an ip

    uint8_t idBuffer[500];
    id.WritePayload(idBuffer, sizeof(idBuffer));

    // next, parse this object
    idOut.ReadPayload(idBuffer, sizeof(idBuffer));

    NL_TEST_ASSERT(inSuite, idOut.HasDiscoveryInfo());
    NL_TEST_ASSERT(inSuite, strcmp(idOut.GetInstanceName(), instanceName) == 0);
    NL_TEST_ASSERT(inSuite, vendorId == idOut.GetVendorId());
    NL_TEST_ASSERT(inSuite, productId == idOut.GetProductId());
    NL_TEST_ASSERT(inSuite, port == idOut.GetCdPort());
    NL_TEST_ASSERT(inSuite, strcmp(idOut.GetDeviceName(), deviceName) == 0);
    NL_TEST_ASSERT(inSuite, rotatingIdLen == idOut.GetRotatingIdLength());
    NL_TEST_ASSERT(inSuite, memcmp(idOut.GetRotatingId(), rotatingId, rotatingIdLen) == 0);
    NL_TEST_ASSERT(inSuite, strcmp(idOut.GetPairingInst(), pairingInst) == 0);
    NL_TEST_ASSERT(inSuite, pairingHint == idOut.GetPairingHint());

    NL_TEST_ASSERT(inSuite, id.GetNumTargetAppInfos() == idOut.GetNumTargetAppInfos());
    NL_TEST_ASSERT(inSuite, idOut.GetTargetAppInfo(0, appInfo) && appInfo.vendorId == 1 && appInfo.productId == 9);
    NL_TEST_ASSERT(inSuite, idOut.GetTargetAppInfo(1, appInfo) && appInfo.vendorId == 2 && appInfo.productId == 8);
    NL_TEST_ASSERT(inSuite, idOut.GetTargetAppInfo(2, appInfo) && appInfo.vendorId == 3 && appInfo.productId == 7);

    NL_TEST_ASSERT(inSuite, id.GetNoPasscode() == idOut.GetNoPasscode());
    NL_TEST_ASSERT(inSuite, id.GetCdUponPasscodeDialog() == idOut.GetCdUponPasscodeDialog());
    NL_TEST_ASSERT(inSuite, id.GetCommissionerPasscode() == idOut.GetCommissionerPasscode());
    NL_TEST_ASSERT(inSuite, id.GetCommissionerPasscodeReady() == idOut.GetCommissionerPasscodeReady());

    // TODO: remove following "force-fail" debug line
    // NL_TEST_ASSERT(inSuite, rotatingIdLen != id.GetRotatingIdLength());
}

void TestUDCCommissionerDeclaration(nlTestSuite * inSuite, void * inContext)
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

    NL_TEST_ASSERT(inSuite, errorCode == id.GetErrorCode());
    NL_TEST_ASSERT(inSuite, id.GetNeedsPasscode() == true);
    NL_TEST_ASSERT(inSuite, id.GetNoAppsFound() == true);
    NL_TEST_ASSERT(inSuite, id.GetPasscodeDialogDisplayed() == true);
    NL_TEST_ASSERT(inSuite, id.GetCommissionerPasscode() == true);
    NL_TEST_ASSERT(inSuite, id.GetQRCodeDisplayed() == true);

    uint8_t idBuffer[500];
    id.WritePayload(idBuffer, sizeof(idBuffer));

    // next, parse this object
    idOut.ReadPayload(idBuffer, sizeof(idBuffer));

    NL_TEST_ASSERT(inSuite, errorCode == idOut.GetErrorCode());
    NL_TEST_ASSERT(inSuite, id.GetNeedsPasscode() == idOut.GetNeedsPasscode());
    NL_TEST_ASSERT(inSuite, id.GetNoAppsFound() == idOut.GetNoAppsFound());
    NL_TEST_ASSERT(inSuite, id.GetPasscodeDialogDisplayed() == idOut.GetPasscodeDialogDisplayed());
    NL_TEST_ASSERT(inSuite, id.GetCommissionerPasscode() == idOut.GetCommissionerPasscode());
    NL_TEST_ASSERT(inSuite, id.GetQRCodeDisplayed() == idOut.GetQRCodeDisplayed());
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
    // the following test case is not reliable (fails on mac, clang platforms for example)
    // NL_TEST_DEF("TestUDCServerInstanceNameResolver", TestUDCServerInstanceNameResolver),
    NL_TEST_DEF("TestUserDirectedCommissioningClientMessage", TestUserDirectedCommissioningClientMessage),
    NL_TEST_DEF("TestUDCClients", TestUDCClients),
    NL_TEST_DEF("TestUDCClientState", TestUDCClientState),
    NL_TEST_DEF("TestUDCIdentificationDeclaration", TestUDCIdentificationDeclaration),
    NL_TEST_DEF("TestUDCCommissionerDeclaration", TestUDCCommissionerDeclaration),

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
