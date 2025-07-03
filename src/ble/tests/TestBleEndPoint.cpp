#include <cstdint>
#include <utility>

#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include <ble/BleApplicationDelegate.h>
#include <ble/BleLayer.h>
#include <ble/BleLayerDelegate.h>
#include <ble/BlePlatformDelegate.h>

namespace chip {
namespace Ble {

DLL_EXPORT BleLayerDelegate * mBleTransport = nullptr;

static unsigned int gConnCounter = 0;
static BLE_CONNECTION_OBJECT NextConnectionObject()
{
    uintptr_t raw = ++gConnCounter;
    return reinterpret_cast<BLE_CONNECTION_OBJECT>(raw);
}

/* Test fixture for testing BLEEndPoint behavior */
class TestBleEndPoint : public ::testing::Test, public BleApplicationDelegate, public BleLayerDelegate, public BlePlatformDelegate
{
public:
    // Set up the BLELayer and global transport delegate before each test
    static void SetUpTestSuite()
    {
        CHIP_ERROR err = Platform::MemoryInit();
        ASSERT_EQ(err, CHIP_NO_ERROR);
        err = DeviceLayer::SystemLayer().Init();
        ASSERT_EQ(err, CHIP_NO_ERROR);
    }
    static void TearDownTestSuite()
    {
        DeviceLayer::SystemLayer().Shutdown();
        Platform::MemoryShutdown();
    }
    void SetUp() override
    {
        CHIP_ERROR err = mBleLayer.Init(/* platformDelegate */ this, /* appDelegate */ this, &DeviceLayer::SystemLayer());
        ASSERT_EQ(err, CHIP_NO_ERROR);
        // Point global transport delegate to this test fixture so BLEEndPoint callbacks come here
        mBleLayer.mBleTransport = this;
        mBleTransport           = this;
        ResetCounters();
    }
    void TearDown() override
    {
        // Remove our global delegate pointer to avoid dangling references
        mBleTransport = nullptr;
        // Shutdown BleLayer to close/free any open BLEEndPoints and cancel timers
        mBleLayer.Shutdown();
    }

    // Utility to create a central-role BLEEndPoint.
    BLEEndPoint * CreateCentralEndPoint()
    {
        BLEEndPoint * ep              = nullptr;
        BLE_CONNECTION_OBJECT connObj = NextConnectionObject();
        CHIP_ERROR err                = mBleLayer.NewBleEndPoint(&ep, connObj, kBleRole_Central, /* autoClose */ true);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_NE(ep, nullptr);
        return ep;
    }

    // Drive the complete central-side handshake for the given endpoint.
    CHIP_ERROR CompleteCentralHandshake(BLEEndPoint * ep)
    {
        // 1. Start connection handshake (send Capabilities Request)
        ReturnErrorOnFailure(ep->StartConnect());
        // The BLE stack would confirm the write; simulate it:
        bool ok = mBleLayer.HandleWriteConfirmation(mPendingConnObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_1_UUID);
        VerifyOrReturnError(ok, CHIP_ERROR_INTERNAL);
        // 2. Simulate that subscription to CHAR_2 (indications) is completed
        ok = mBleLayer.HandleSubscribeComplete(mPendingConnObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_2_UUID);
        VerifyOrReturnError(ok, CHIP_ERROR_INTERNAL);
        // 3. Simulate the peripheral's Capabilities Response indication arriving
        BleTransportCapabilitiesResponseMessage resp;
        resp.mSelectedProtocolVersion  = CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION;
        resp.mFragmentSize             = 100;
        resp.mWindowSize               = BLE_MAX_RECEIVE_WINDOW_SIZE;
        System::PacketBufferHandle buf = System::PacketBufferHandle::New(16);
        ReturnErrorOnFailure(resp.Encode(buf));
        ReturnErrorOnFailure(ep->Receive(std::move(buf))); // Process the response
        return CHIP_NO_ERROR;
    }

    // Utility to simulate a full peripheral-side handshake and return the new BLEEndPoint.
    BLEEndPoint * CompletePeripheralHandshake()
    {
        BLE_CONNECTION_OBJECT connObj = NextConnectionObject();
        // 1. Simulate central sending Capabilities Request via Write to CHAR_1
        constexpr uint8_t capReqData[]    = { 0x65, 0x6C, 0x54, 0x00, 0x00, 0x00, 0xC8, 0x00, 0x06 };
        System::PacketBufferHandle reqBuf = System::PacketBufferHandle::NewWithData(capReqData, sizeof(capReqData));
        bool ok = mBleLayer.HandleWriteReceived(connObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_1_UUID, std::move(reqBuf));
        EXPECT_TRUE(ok);
        // 2. Simulate central subscribing to CHAR_2 for indications
        ok = mBleLayer.HandleSubscribeReceived(connObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_2_UUID);
        EXPECT_TRUE(ok);
        // By now, our BleLayerDelegate::SetEndPoint should have been called, storing the endpoint in mLastEndPoint.
        EXPECT_NE(mLastEndPoint, nullptr);
        // The peripheral endpoint is now in Connected state (handshake response sent, waiting for ack)
        return mLastEndPoint;
    }

protected:
    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT) override {}
    void OnBleConnectionComplete(BLEEndPoint *) override {}
    void OnBleConnectionError(CHIP_ERROR) override {}
    void OnEndPointConnectComplete(BLEEndPoint * endPoint, CHIP_ERROR err) override
    {
        // Called when a handshake (as central) is completed successfully or aborted
        mLastEndPoint   = endPoint;
        mLastConnectErr = err;
        mConnectCompleteCalls++;
        EXPECT_EQ(err, CHIP_NO_ERROR);
    }
    void OnEndPointMessageReceived(BLEEndPoint * endPoint, System::PacketBufferHandle && msg) override {}
    void OnEndPointConnectionClosed(BLEEndPoint * endPoint, CHIP_ERROR err) override
    {
        mLastCloseErr = err;
        mConnectionClosedCalls++;
    }
    CHIP_ERROR SetEndPoint(BLEEndPoint * endPoint) override
    {
        // This is called when a peripheral-side handshake is completed (on receiving Subscribe request).
        // We save the endpoint so we can test it in the future
        mLastEndPoint = endPoint;
        return CHIP_NO_ERROR;
    }

    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return 247; }
    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT) override { return CHIP_NO_ERROR; }
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID *, const ChipBleUUID *,
                                System::PacketBufferHandle buf) override
    {
        // Capture the connection and data being written (central sending Capabilities Request)
        mPendingConnObj = connObj;
        mLastWriteBuf   = buf.Retain(); // keep a copy for verification
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID *, const ChipBleUUID *,
                              System::PacketBufferHandle buf) override
    {
        // Simulate sending an indication (peripheral sending Capabilities Response)
        mPendingConnObj = connObj;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR mLastConnectErr = CHIP_NO_ERROR;
    CHIP_ERROR mLastCloseErr   = CHIP_NO_ERROR;
    // Helper to reset counters and captured buffers
    void ResetCounters()
    {
        mConnectCompleteCalls  = 0;
        mConnectionClosedCalls = 0;
        mLastEndPoint          = nullptr;
        mLastWriteBuf          = nullptr; // free any retained buffer
        mPendingConnObj        = BLE_CONNECTION_UNINITIALIZED;
        mLastConnectErr        = CHIP_NO_ERROR;
        mLastCloseErr          = CHIP_NO_ERROR;
    }

    // Fixture state
    BleLayer mBleLayer;
    BLEEndPoint * mLastEndPoint = nullptr;                                // last endpoint delivered via delegate
    System::PacketBufferHandle mLastWriteBuf;                             // last buffer captured from a Write request
    BLE_CONNECTION_OBJECT mPendingConnObj = BLE_CONNECTION_UNINITIALIZED; // last connObj used in a platform callback
    int mConnectCompleteCalls             = 0;
    int mConnectionClosedCalls            = 0;
};

/* ==================== Test Cases ==================== */

TEST_F(TestBleEndPoint, StartConnectSendsCapabilitiesRequest)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);
    // Start the handshake
    EXPECT_EQ(ep->StartConnect(), CHIP_NO_ERROR);

    // Verify that a Capabilities Request was formed and captured
    ASSERT_FALSE(mLastWriteBuf.IsNull());
    BleTransportCapabilitiesRequestMessage req;
    // The buffer should decode into a valid request
    ASSERT_EQ(BleTransportCapabilitiesRequestMessage::Decode(mLastWriteBuf, req), CHIP_NO_ERROR);
    // Check some expected fields
    EXPECT_EQ(req.mWindowSize, BLE_MAX_RECEIVE_WINDOW_SIZE);
    EXPECT_EQ(req.mMtu, 247u); // Our GetMTU returned 247
    // The first supported protocol version nibble should be the max supported version
    uint8_t majorSupported = req.mSupportedProtocolVersions[0] & 0x0F; // lower 4 bits of first byte
    EXPECT_EQ(majorSupported, static_cast<uint8_t>(CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION));

    ep->Abort(); // Abort the connection to clean up (since we didn't finish the handshake here)
}

TEST_F(TestBleEndPoint, HandleConnectCompleteTriggersCallback)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);
    // Complete the handshake fully
    ASSERT_EQ(CompleteCentralHandshake(ep), CHIP_NO_ERROR);
    // The delegate callback OnEndPointConnectComplete should have been called once
    EXPECT_EQ(mConnectCompleteCalls, 1);
    EXPECT_EQ(mConnectionClosedCalls, 0);
    // The endpoint should now be in Connected state (and ready for data exchange or closure)
    EXPECT_EQ(ep->Send(System::PacketBufferHandle::New(0)), CHIP_NO_ERROR);
    ep->Abort();
}

TEST_F(TestBleEndPoint, CloseFiresConnectionClosedCallback)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);
    ASSERT_EQ(CompleteCentralHandshake(ep), CHIP_NO_ERROR);
    // Close the connection
    ep->Close();
    // This should trigger OnEndPointConnectionClosed exactly once
    EXPECT_EQ(mConnectionClosedCalls, 1);
    // (The actual endpoint will linger until unsubscribed; BleLayer.Shutdown will free it.)
}

} // namespace Ble
} // namespace chip
