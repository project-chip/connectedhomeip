#include <cstdint>
#include <type_traits>
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

namespace { // === test‑only constants ===
constexpr uint16_t kBleTestMtu           = 247;
constexpr uint16_t kBleTestFragmentSize  = 100;
constexpr uint16_t kBleTestScratchBufLen = 16;
} // namespace

namespace chip {
namespace Ble {

DLL_EXPORT BleLayerDelegate * mBleTransport = nullptr;

static unsigned int gConnCounter = 0;
template <typename T>
static inline typename std::enable_if<std::is_integral<T>::value, T>::type MakeConnObj(unsigned int n)
{
    return static_cast<T>(n);
}
template <typename T>
static inline typename std::enable_if<std::is_pointer<T>::value, T>::type MakeConnObj(unsigned int n)
{
    return reinterpret_cast<T>(static_cast<uintptr_t>(n));
}

static BLE_CONNECTION_OBJECT NextConnectionObject()
{
    return MakeConnObj<BLE_CONNECTION_OBJECT>(++gConnCounter);
}

/* Test fixture for testing BLEEndPoint behavior */
class TestBleEndPoint : public ::testing::Test, public BleApplicationDelegate, public BleLayerDelegate, public BlePlatformDelegate
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::SystemLayer().Init(), CHIP_NO_ERROR);
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
        mBleLayer.mBleTransport = this;
        mBleTransport           = this;
        ResetCounters();
    }
    void TearDown() override
    {
        mBleTransport = nullptr;
        mBleLayer.Shutdown();
    }

    BLEEndPoint * CreateCentralEndPoint()
    {
        BLEEndPoint * ep              = nullptr;
        BLE_CONNECTION_OBJECT connObj = NextConnectionObject();
        CHIP_ERROR err                = mBleLayer.NewBleEndPoint(&ep, connObj, kBleRole_Central, /* autoClose */ true);
        EXPECT_EQ(err, CHIP_NO_ERROR);
        EXPECT_NE(ep, nullptr);
        return ep;
    }

    CHIP_ERROR CompleteCentralHandshake(BLEEndPoint * ep)
    {
        ReturnErrorOnFailure(ep->StartConnect());

        bool ok = mBleLayer.HandleWriteConfirmation(mPendingConnObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_1_UUID);
        VerifyOrReturnError(ok, CHIP_ERROR_INTERNAL);

        ok = mBleLayer.HandleSubscribeComplete(mPendingConnObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_2_UUID);
        VerifyOrReturnError(ok, CHIP_ERROR_INTERNAL);

        BleTransportCapabilitiesResponseMessage resp;
        resp.mSelectedProtocolVersion = CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION;
        resp.mFragmentSize            = kBleTestFragmentSize;
        resp.mWindowSize              = BLE_MAX_RECEIVE_WINDOW_SIZE;

        System::PacketBufferHandle buf = System::PacketBufferHandle::New(kBleTestScratchBufLen);
        ReturnErrorOnFailure(resp.Encode(buf));
        ReturnErrorOnFailure(ep->Receive(std::move(buf)));
        return CHIP_NO_ERROR;
    }

protected:
    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT) override {}
    void OnBleConnectionComplete(BLEEndPoint *) override {}
    void OnBleConnectionError(CHIP_ERROR) override {}
    void OnEndPointConnectComplete(BLEEndPoint * endPoint, CHIP_ERROR err) override
    {
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

    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return kBleTestMtu; }
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
        mPendingConnObj = connObj;
        mLastWriteBuf   = buf.Retain();
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID *, const ChipBleUUID *,
                              System::PacketBufferHandle) override
    {
        mPendingConnObj = connObj;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR mLastConnectErr = CHIP_NO_ERROR;
    CHIP_ERROR mLastCloseErr   = CHIP_NO_ERROR;
    void ResetCounters()
    {
        mConnectCompleteCalls  = 0;
        mConnectionClosedCalls = 0;
        mLastEndPoint          = nullptr;
        mLastWriteBuf          = nullptr;
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

// Verify capability request is formed correctly on connect
TEST_F(TestBleEndPoint, StartConnectSendsCapabilitiesRequest)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);
    EXPECT_EQ(ep->StartConnect(), CHIP_NO_ERROR);
    ASSERT_FALSE(mLastWriteBuf.IsNull());
    BleTransportCapabilitiesRequestMessage req;
    ASSERT_EQ(BleTransportCapabilitiesRequestMessage::Decode(mLastWriteBuf, req), CHIP_NO_ERROR);
    EXPECT_EQ(req.mWindowSize, BLE_MAX_RECEIVE_WINDOW_SIZE);
    EXPECT_EQ(req.mMtu, static_cast<uint16_t>(kBleTestMtu));
    uint8_t majorSupported = req.mSupportedProtocolVersions[0] & 0x0F; // lower 4 bits of first byte
    EXPECT_EQ(majorSupported, static_cast<uint8_t>(CHIP_BLE_TRANSPORT_PROTOCOL_MAX_SUPPORTED_VERSION));

    ep->Abort(); // Abort the connection to clean up (since we didn't finish the handshake here)
}

// Confirm connect completion triggers delegate callback
TEST_F(TestBleEndPoint, HandleConnectCompleteTriggersCallback)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);
    ASSERT_EQ(CompleteCentralHandshake(ep), CHIP_NO_ERROR);
    EXPECT_EQ(mConnectCompleteCalls, 1);
    EXPECT_EQ(mConnectionClosedCalls, 0);
    EXPECT_EQ(ep->Send(System::PacketBufferHandle::New(0)), CHIP_NO_ERROR);
    ep->Abort();
}

// Ensure connection closure invokes proper cleanup callback
TEST_F(TestBleEndPoint, CloseFiresConnectionClosedCallback)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);
    ASSERT_EQ(CompleteCentralHandshake(ep), CHIP_NO_ERROR);
    ep->Close();
    EXPECT_EQ(mConnectionClosedCalls, 1);
}

} // namespace Ble
} // namespace chip
