#include <cstdint>
#include <cstring>
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
#include <ble/BleError.h>
#include <ble/BleLayer.h>
#include <ble/BleLayerDelegate.h>
#include <ble/BlePlatformDelegate.h>

namespace { // === test‑only constants ===
constexpr uint16_t kBleTestMtu                     = 247;
constexpr uint16_t kBleTestFragmentSize            = 100;
constexpr uint16_t kBleTestScratchBufLen           = 16;
constexpr size_t kBtpStartFragmentWithAckHeaderLen = 5; // flags (1) + piggybacked ack (1) + seq (1) + length (2)
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

        return FinishCentralHandshakeAfterWriteConfirmation(ep);
    }

    CHIP_ERROR FinishCentralHandshakeAfterWriteConfirmation(BLEEndPoint * ep)
    {
        bool ok = mBleLayer.HandleSubscribeComplete(mPendingConnObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_2_UUID);
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
        EXPECT_EQ(err, mExpectedConnectErr);
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
        CHIP_ERROR err    = mNextSubscribeErr;
        mNextSubscribeErr = CHIP_NO_ERROR;
        mSubscribeCalls++;
        return err;
    }
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override
    {
        mUnsubscribeCalls++;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT) override
    {
        mCloseConnectionCalls++;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID *, const ChipBleUUID *,
                                System::PacketBufferHandle buf) override
    {
        mPendingConnObj      = connObj;
        CHIP_ERROR result    = mNextWriteRequestErr;
        mNextWriteRequestErr = CHIP_NO_ERROR;
        if (mConfirmNextWriteBeforeReturn)
        {
            // Deliver the GATT write confirmation before SendWriteRequest() returns, as happens on platforms that
            // release the stack lock while sending (e.g. Android BLEManagerImpl). StartConnect() moved its local
            // handle into SendWrite(), so `buf` becomes sole owner once HandleHandshakeConfirmationReceived()
            // frees the queued capabilities reference from mSendQueue and starts characteristic subscription.
            mConfirmNextWriteBeforeReturn = false;
            (void) mBleLayer.HandleWriteConfirmation(connObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_1_UUID);
            mWriteBufSoleOwnerAfterConfirm = buf.HasSoleOwnership();
            mSubscribeCallsDuringWrite     = mSubscribeCalls;
            if (mPinnedEndPointDuringWrite != nullptr)
            {
                BLEEndPoint * pinned                              = mPinnedEndPointDuringWrite;
                mPinnedEndPointDuringWrite                        = nullptr;
                BLEEndPoint * extras[BLE_LAYER_NUM_BLE_ENDPOINTS] = {};
                size_t allocated                                  = 0;
                while (allocated < BLE_LAYER_NUM_BLE_ENDPOINTS)
                {
                    BLEEndPoint * candidate = nullptr;
                    CHIP_ERROR allocErr     = mBleLayer.NewBleEndPoint(&candidate, NextConnectionObject(), kBleRole_Central,
                                                                       /* autoClose */ false);
                    if (allocErr == CHIP_ERROR_ENDPOINT_POOL_FULL)
                    {
                        break;
                    }
                    EXPECT_EQ(allocErr, CHIP_NO_ERROR);
                    EXPECT_NE(candidate, nullptr);
                    EXPECT_NE(candidate, pinned);
                    if (candidate == pinned)
                    {
                        mPinnedSlotReusedDuringWrite = true;
                    }
                    extras[allocated++] = candidate;
                }
                mPoolAllocationsDuringWrite   = allocated;
                mPoolAllocationRanDuringWrite = true;
                for (size_t i = 0; i < allocated; ++i)
                {
                    extras[i]->Abort();
                }
            }
        }
        mLastWriteBuf = buf.Retain();
        return result;
    }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID *, const ChipBleUUID *,
                              System::PacketBufferHandle) override
    {
        mPendingConnObj = connObj;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR mLastConnectErr      = CHIP_NO_ERROR;
    CHIP_ERROR mLastCloseErr        = CHIP_NO_ERROR;
    CHIP_ERROR mExpectedConnectErr  = CHIP_NO_ERROR;
    CHIP_ERROR mNextWriteRequestErr = CHIP_NO_ERROR;
    CHIP_ERROR mNextSubscribeErr    = CHIP_NO_ERROR;
    void ResetCounters()
    {
        mConnectCompleteCalls          = 0;
        mConnectionClosedCalls         = 0;
        mCloseConnectionCalls          = 0;
        mSubscribeCalls                = 0;
        mSubscribeCallsDuringWrite     = 0;
        mUnsubscribeCalls              = 0;
        mLastEndPoint                  = nullptr;
        mPinnedEndPointDuringWrite     = nullptr;
        mLastWriteBuf                  = nullptr;
        mPendingConnObj                = BLE_CONNECTION_UNINITIALIZED;
        mLastConnectErr                = CHIP_NO_ERROR;
        mLastCloseErr                  = CHIP_NO_ERROR;
        mExpectedConnectErr            = CHIP_NO_ERROR;
        mNextWriteRequestErr           = CHIP_NO_ERROR;
        mNextSubscribeErr              = CHIP_NO_ERROR;
        mConfirmNextWriteBeforeReturn  = false;
        mWriteBufSoleOwnerAfterConfirm = false;
        mPinnedSlotReusedDuringWrite   = false;
        mPoolAllocationRanDuringWrite  = false;
        mPoolAllocationsDuringWrite    = 0;
    }

    // Fixture state
    BleLayer mBleLayer;
    BLEEndPoint * mLastEndPoint              = nullptr;                   // last endpoint delivered via delegate
    BLEEndPoint * mPinnedEndPointDuringWrite = nullptr;                   // endpoint whose pool slot must stay pinned in SendWrite
    System::PacketBufferHandle mLastWriteBuf;                             // last buffer captured from a Write request
    BLE_CONNECTION_OBJECT mPendingConnObj = BLE_CONNECTION_UNINITIALIZED; // last connObj used in a platform callback
    int mConnectCompleteCalls             = 0;
    int mConnectionClosedCalls            = 0;
    int mCloseConnectionCalls             = 0;
    int mSubscribeCalls                   = 0;
    int mSubscribeCallsDuringWrite        = 0;
    int mUnsubscribeCalls                 = 0;
    bool mConfirmNextWriteBeforeReturn    = false; // confirm the next write from inside SendWriteRequest()
    bool mWriteBufSoleOwnerAfterConfirm   = false; // mSendQueue ref was freed inside HandleWriteConfirmation()
    bool mPinnedSlotReusedDuringWrite     = false; // true if NewBleEndPoint() handed out the pinned slot mid-write
    bool mPoolAllocationRanDuringWrite    = false; // true if pool allocation probe ran inside SendWriteRequest()
    size_t mPoolAllocationsDuringWrite    = 0;     // free pool slots allocated while endpoint remained pinned
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

// The capabilities request write may be confirmed before StartConnect() returns (Android releases the stack lock
// while sending the write). The handshake must free the capabilities request from mSendQueue inside SendWriteRequest()
// so the first post-connect Send() transmits the application payload rather than re-sending the capabilities request.
TEST_F(TestBleEndPoint, StartConnectHandlesWriteConfirmationBeforeSendWriteReturns)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);

    mConfirmNextWriteBeforeReturn = true;
    ASSERT_EQ(ep->StartConnect(), CHIP_NO_ERROR);
    EXPECT_TRUE(mWriteBufSoleOwnerAfterConfirm);
    EXPECT_EQ(mSubscribeCallsDuringWrite, 1);
    EXPECT_EQ(mSubscribeCalls, 1);

    ASSERT_EQ(FinishCentralHandshakeAfterWriteConfirmation(ep), CHIP_NO_ERROR);
    EXPECT_EQ(mConnectCompleteCalls, 1);
    EXPECT_EQ(mConnectionClosedCalls, 0);

    constexpr uint8_t kAppPayload[] = { 0xde, 0xad, 0xbe, 0xef };
    mLastWriteBuf                   = nullptr;
    ASSERT_EQ(ep->Send(System::PacketBufferHandle::NewWithData(kAppPayload, sizeof(kAppPayload))), CHIP_NO_ERROR);
    ASSERT_FALSE(mLastWriteBuf.IsNull());
    ASSERT_EQ(mLastWriteBuf->DataLength(), kBtpStartFragmentWithAckHeaderLen + sizeof(kAppPayload));
    EXPECT_EQ(std::memcmp(mLastWriteBuf->Start() + kBtpStartFragmentWithAckHeaderLen, kAppPayload, sizeof(kAppPayload)), 0);

    ep->Abort();
}

// If SendWrite() fails synchronously after QueueTx() stashed the capabilities request in mSendQueue, StartConnect()
// must close the endpoint (clearing mSendQueue), notify connect failure once, close the BLE connection, and release
// the endpoint pool slot.
TEST_F(TestBleEndPoint, StartConnectHandlesSynchronousSendWriteFailure)
{
    BLEEndPoint * ep = CreateCentralEndPoint();
    ASSERT_NE(ep, nullptr);

    mNextWriteRequestErr = BLE_ERROR_GATT_WRITE_FAILED;
    mExpectedConnectErr  = BLE_ERROR_GATT_WRITE_FAILED;
    EXPECT_EQ(ep->StartConnect(), BLE_ERROR_GATT_WRITE_FAILED);
    // DoClose() -> FinalizeClose() freed the mSendQueue reference, leaving mLastWriteBuf as sole owner.
    ASSERT_FALSE(mLastWriteBuf.IsNull());
    EXPECT_TRUE(mLastWriteBuf.HasSoleOwnership());
    EXPECT_EQ(mConnectCompleteCalls, 1);
    EXPECT_EQ(mLastConnectErr, BLE_ERROR_GATT_WRITE_FAILED);
    EXPECT_EQ(mConnectionClosedCalls, 0);
    EXPECT_EQ(mCloseConnectionCalls, 1);
    EXPECT_EQ(mSubscribeCalls, 0);

    // Confirm the failed endpoint slot was returned to the pool by exhausting all pool slots cleanly.
    BLEEndPoint * poolEndpoints[BLE_LAYER_NUM_BLE_ENDPOINTS] = {};
    for (auto & slot : poolEndpoints)
    {
        slot = CreateCentralEndPoint();
        ASSERT_NE(slot, nullptr);
    }
    for (auto * slot : poolEndpoints)
    {
        slot->Abort();
    }
}

// Even if a platform delivers an early write confirmation (including one whose subscribe step fails and closes the
// endpoint re-entrantly) before SendWriteRequest() returns, StartConnect() must keep the pool slot pinned until it
// unwinds, avoid double-closing, and return the slot cleanly afterwards.
TEST_F(TestBleEndPoint, StartConnectHandlesWriteConfirmationThenSendWriteFailure)
{
    {
        BLEEndPoint * ep = CreateCentralEndPoint();
        ASSERT_NE(ep, nullptr);

        mConfirmNextWriteBeforeReturn = true;
        mNextWriteRequestErr          = BLE_ERROR_GATT_WRITE_FAILED;
        mExpectedConnectErr           = BLE_ERROR_GATT_WRITE_FAILED;
        EXPECT_EQ(ep->StartConnect(), BLE_ERROR_GATT_WRITE_FAILED);
        EXPECT_TRUE(mWriteBufSoleOwnerAfterConfirm);
        EXPECT_EQ(mSubscribeCallsDuringWrite, 1);
        EXPECT_EQ(mSubscribeCalls, 1);
        EXPECT_EQ(mConnectCompleteCalls, 1);
        EXPECT_EQ(mLastConnectErr, BLE_ERROR_GATT_WRITE_FAILED);
        EXPECT_EQ(mUnsubscribeCalls, 1);
        ASSERT_TRUE(mBleLayer.HandleUnsubscribeComplete(mPendingConnObj, &CHIP_BLE_SVC_ID, &CHIP_BLE_CHAR_2_UUID));
        EXPECT_EQ(mCloseConnectionCalls, 1);
    }

    ResetCounters();
    {
        BLEEndPoint * ep = CreateCentralEndPoint();
        ASSERT_NE(ep, nullptr);

        mConfirmNextWriteBeforeReturn = true;
        mPinnedEndPointDuringWrite    = ep;
        mNextSubscribeErr             = BLE_ERROR_GATT_SUBSCRIBE_FAILED;
        mNextWriteRequestErr          = BLE_ERROR_GATT_WRITE_FAILED;
        mExpectedConnectErr           = BLE_ERROR_GATT_SUBSCRIBE_FAILED;
        EXPECT_EQ(ep->StartConnect(), BLE_ERROR_GATT_WRITE_FAILED);
        EXPECT_TRUE(mWriteBufSoleOwnerAfterConfirm);
        EXPECT_TRUE(mPoolAllocationRanDuringWrite);
        EXPECT_FALSE(mPinnedSlotReusedDuringWrite);
        EXPECT_EQ(mPoolAllocationsDuringWrite, static_cast<size_t>(BLE_LAYER_NUM_BLE_ENDPOINTS - 1));
        EXPECT_EQ(mSubscribeCallsDuringWrite, 1);
        EXPECT_EQ(mSubscribeCalls, 1);
        EXPECT_EQ(mUnsubscribeCalls, 0);
        EXPECT_EQ(mConnectCompleteCalls, 1);
        EXPECT_EQ(mLastConnectErr, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
        EXPECT_EQ(mCloseConnectionCalls, 1);
    }

    ResetCounters();
    {
        // When SendWriteRequest() itself succeeds (CHIP_NO_ERROR) after a re-entrant callback closed and freed the
        // endpoint (as on Android when the GATT write was submitted before the stack lock was re-acquired),
        // StartConnect() returns CHIP_NO_ERROR for the synchronous write submission while OnEndPointConnectComplete
        // has already reported the connect failure once and the slot stays pinned until StartConnect() returns.
        BLEEndPoint * ep = CreateCentralEndPoint();
        ASSERT_NE(ep, nullptr);

        mConfirmNextWriteBeforeReturn = true;
        mPinnedEndPointDuringWrite    = ep;
        mNextSubscribeErr             = BLE_ERROR_GATT_SUBSCRIBE_FAILED;
        mNextWriteRequestErr          = CHIP_NO_ERROR;
        mExpectedConnectErr           = BLE_ERROR_GATT_SUBSCRIBE_FAILED;
        EXPECT_EQ(ep->StartConnect(), CHIP_NO_ERROR);
        EXPECT_TRUE(mWriteBufSoleOwnerAfterConfirm);
        EXPECT_TRUE(mPoolAllocationRanDuringWrite);
        EXPECT_FALSE(mPinnedSlotReusedDuringWrite);
        EXPECT_EQ(mPoolAllocationsDuringWrite, static_cast<size_t>(BLE_LAYER_NUM_BLE_ENDPOINTS - 1));
        EXPECT_EQ(mSubscribeCallsDuringWrite, 1);
        EXPECT_EQ(mSubscribeCalls, 1);
        EXPECT_EQ(mUnsubscribeCalls, 0);
        EXPECT_EQ(mConnectCompleteCalls, 1);
        EXPECT_EQ(mLastConnectErr, BLE_ERROR_GATT_SUBSCRIBE_FAILED);
        EXPECT_EQ(mCloseConnectionCalls, 1);
    }

    BLEEndPoint * poolEndpoints[BLE_LAYER_NUM_BLE_ENDPOINTS] = {};
    for (auto & slot : poolEndpoints)
    {
        slot = CreateCentralEndPoint();
        ASSERT_NE(slot, nullptr);
    }
    for (auto * slot : poolEndpoints)
    {
        slot->Abort();
    }
}

} // namespace Ble
} // namespace chip
