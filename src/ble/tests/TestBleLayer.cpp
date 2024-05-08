/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <array>
#include <cstdint>
#include <memory>
#include <numeric>
#include <type_traits>

#include <gtest/gtest.h>

#include <lib/support/CHIPMem.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

#define _CHIP_BLE_BLE_H
#include <ble/BleApplicationDelegate.h>
#include <ble/BleLayer.h>
#include <ble/BleLayerDelegate.h>
#include <ble/BlePlatformDelegate.h>

namespace chip {
namespace Ble {

namespace {

constexpr ChipBleUUID uuidZero{};
constexpr ChipBleUUID uuidSvc   = { { 0x00, 0x00, 0xFF, 0xF6, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34,
                                      0xFB } };
constexpr ChipBleUUID uuidChar1 = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D,
                                      0x11 } };
constexpr ChipBleUUID uuidChar2 = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D,
                                      0x12 } };
constexpr ChipBleUUID uuidChar3 = { { 0x64, 0x63, 0x02, 0x38, 0x87, 0x72, 0x45, 0xF2, 0xB8, 0x7D, 0x74, 0x8A, 0x83, 0x21, 0x8F,
                                      0x04 } };

}; // namespace

class TestBleLayer : public BleLayer,
                     private BleApplicationDelegate,
                     private BleLayerDelegate,
                     private BlePlatformDelegate,
                     public ::testing::Test
{
public:
    static void SetUpTestSuite()
    {
        ASSERT_EQ(chip::Platform::MemoryInit(), CHIP_NO_ERROR);
        ASSERT_EQ(DeviceLayer::SystemLayer().Init(), CHIP_NO_ERROR);
    }

    static void TearDownTestSuite()
    {
        DeviceLayer::SystemLayer().Shutdown();
        chip::Platform::MemoryShutdown();
    }

    void SetUp()
    {
        ASSERT_EQ(Init(this, this, &DeviceLayer::SystemLayer()), CHIP_NO_ERROR);
        mBleTransport = this;
    }

    void TearDown()
    {
        mBleTransport = nullptr;
        Shutdown();
    }

    // Return unique BLE connection object for each call.
    template <typename T = BLE_CONNECTION_OBJECT>
    BLE_CONNECTION_OBJECT GetConnectionObject()
    {
        T conn = BLE_CONNECTION_UNINITIALIZED;

        if constexpr (std::is_pointer_v<T>)
        {
            conn = reinterpret_cast<T>(&mNumConnection + mNumConnection);
        }
        else
        {
            conn = static_cast<T>(mNumConnection);
        }

        mNumConnection++;
        return conn;
    }

    // Passing capabilities request message to HandleWriteReceived should create
    // new BLE endpoint which later can be used to receive more data.
    bool HandleWriteReceivedCapabilitiesRequest(BLE_CONNECTION_OBJECT connObj)
    {
        constexpr uint8_t capReq[] = { 0x65, 0x6c, 0x54, 0x00, 0x00, 0x00, 0xc8, 0x00, 0x06 };
        auto buf                   = System::PacketBufferHandle::NewWithData(capReq, sizeof(capReq));
        return HandleWriteReceived(connObj, &uuidSvc, &uuidChar1, std::move(buf));
    }

    // Processing subscription request after capabilities request should finalize
    // connection establishment.
    bool HandleSubscribeReceivedOnChar2(BLE_CONNECTION_OBJECT connObj)
    {
        return HandleSubscribeReceived(connObj, &uuidSvc, &uuidChar2);
    }

    ///
    // Implementation of BleApplicationDelegate

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj) override {}

    ///
    // Implementation of BleLayerDelegate

    void OnBleConnectionComplete(BLEEndPoint * endpoint) override {}
    void OnBleConnectionError(CHIP_ERROR err) override {}
    void OnEndPointConnectComplete(BLEEndPoint * endPoint, CHIP_ERROR err) override {}
    void OnEndPointMessageReceived(BLEEndPoint * endPoint, System::PacketBufferHandle && msg) override {}
    void OnEndPointConnectionClosed(BLEEndPoint * endPoint, CHIP_ERROR err) override {}
    CHIP_ERROR SetEndPoint(BLEEndPoint * endPoint) override { return CHIP_NO_ERROR; }

    ///
    // Implementation of BlePlatformDelegate

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override { return true; }
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override { return true; }
    bool CloseConnection(BLE_CONNECTION_OBJECT connObj) override { return true; }
    uint16_t GetMTU(BLE_CONNECTION_OBJECT connObj) const override { return 0; }
    bool SendIndication(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                        PacketBufferHandle pBuf) override
    {
        return true;
    }
    bool SendWriteRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                          PacketBufferHandle pBuf) override
    {
        return true;
    }
    bool SendReadRequest(BLE_CONNECTION_OBJECT connObj, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                         PacketBufferHandle pBuf) override
    {
        return true;
    }
    bool SendReadResponse(BLE_CONNECTION_OBJECT connObj, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                          const ChipBleUUID * charId) override
    {
        return true;
    }

private:
    unsigned int mNumConnection = 0;
};

TEST_F(TestBleLayer, CheckBleTransportCapabilitiesRequestMessage)
{
    auto buf = System::PacketBufferHandle::New(100);
    ASSERT_FALSE(buf.IsNull());

    BleTransportCapabilitiesRequestMessage msg{};
    msg.SetSupportedProtocolVersion(0, CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION);
    msg.SetSupportedProtocolVersion(1, CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION + 1);
    msg.mMtu        = 200;
    msg.mWindowSize = BLE_MAX_RECEIVE_WINDOW_SIZE;

    ASSERT_EQ(msg.Encode(buf), CHIP_NO_ERROR);
    ChipLogByteSpan(Test, ByteSpan(buf->Start(), buf->DataLength()));

    BleTransportCapabilitiesRequestMessage msgVerify;
    ASSERT_EQ(BleTransportCapabilitiesRequestMessage::Decode(buf, msgVerify), CHIP_NO_ERROR);
    EXPECT_EQ(memcmp(msg.mSupportedProtocolVersions, msgVerify.mSupportedProtocolVersions, sizeof(msg.mSupportedProtocolVersions)),
              0);
    EXPECT_EQ(msg.mMtu, msgVerify.mMtu);
    EXPECT_EQ(msg.mWindowSize, msgVerify.mWindowSize);
}

TEST_F(TestBleLayer, CheckBleTransportCapabilitiesResponseMessage)
{
    auto buf = System::PacketBufferHandle::New(100);
    ASSERT_FALSE(buf.IsNull());

    BleTransportCapabilitiesResponseMessage msg{};
    msg.mSelectedProtocolVersion = CHIP_BLE_TRANSPORT_PROTOCOL_MIN_SUPPORTED_VERSION;
    msg.mFragmentSize            = 200;
    msg.mWindowSize              = BLE_MAX_RECEIVE_WINDOW_SIZE;

    EXPECT_EQ(msg.Encode(buf), CHIP_NO_ERROR);
    ChipLogByteSpan(Test, ByteSpan(buf->Start(), buf->DataLength()));

    BleTransportCapabilitiesResponseMessage msgVerify;
    ASSERT_EQ(BleTransportCapabilitiesResponseMessage::Decode(buf, msgVerify), CHIP_NO_ERROR);
    EXPECT_EQ(msg.mSelectedProtocolVersion, msgVerify.mSelectedProtocolVersion);
    EXPECT_EQ(msg.mFragmentSize, msgVerify.mFragmentSize);
    EXPECT_EQ(msg.mWindowSize, msgVerify.mWindowSize);
}

TEST_F(TestBleLayer, HandleWriteReceivedCapabilitiesRequest)
{
    auto connObj = GetConnectionObject();
    EXPECT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
}

TEST_F(TestBleLayer, HandleSubscribeReceivedInvalidUUID)
{
    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleSubscribeReceived(connObj, &uuidZero, &uuidZero));
    EXPECT_FALSE(HandleSubscribeReceived(connObj, &uuidSvc, &uuidChar1));
}

TEST_F(TestBleLayer, HandleSubscribeReceived)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
    EXPECT_TRUE(HandleSubscribeReceivedOnChar2(connObj));
}

TEST_F(TestBleLayer, HandleSubscribeCompleteInvalidUUID)
{
    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleSubscribeComplete(connObj, &uuidZero, &uuidZero));
    EXPECT_FALSE(HandleSubscribeComplete(connObj, &uuidSvc, &uuidChar1));
}

TEST_F(TestBleLayer, HandleSubscribeComplete)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
    ASSERT_TRUE(HandleSubscribeReceivedOnChar2(connObj));

    EXPECT_TRUE(HandleSubscribeComplete(connObj, &uuidSvc, &uuidChar2));
}

TEST_F(TestBleLayer, HandleUnsubscribeReceivedInvalidUUID)
{
    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleUnsubscribeReceived(connObj, &uuidZero, &uuidZero));
    EXPECT_FALSE(HandleUnsubscribeReceived(connObj, &uuidSvc, &uuidChar1));
}

TEST_F(TestBleLayer, HandleUnsubscribeReceived)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
    ASSERT_TRUE(HandleSubscribeReceivedOnChar2(connObj));

    EXPECT_TRUE(HandleUnsubscribeReceived(connObj, &uuidSvc, &uuidChar2));
}

TEST_F(TestBleLayer, HandleUnsubscribeCompleteInvalidUUID)
{
    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleUnsubscribeComplete(connObj, &uuidZero, &uuidZero));
    EXPECT_FALSE(HandleUnsubscribeComplete(connObj, &uuidSvc, &uuidChar1));
}

TEST_F(TestBleLayer, HandleUnsubscribeComplete)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
    ASSERT_TRUE(HandleSubscribeReceivedOnChar2(connObj));

    EXPECT_TRUE(HandleUnsubscribeComplete(connObj, &uuidSvc, &uuidChar2));
}

TEST_F(TestBleLayer, HandleWriteReceivedInvalidUUID)
{
    auto connObj = GetConnectionObject();
    auto buf     = System::PacketBufferHandle::New(0);
    ASSERT_FALSE(buf.IsNull());

    EXPECT_FALSE(HandleWriteReceived(connObj, &uuidZero, &uuidZero, buf.Retain()));
    EXPECT_FALSE(HandleWriteReceived(connObj, &uuidSvc, &uuidChar3, std::move(buf)));
}

TEST_F(TestBleLayer, HandleWriteReceived)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
    ASSERT_TRUE(HandleSubscribeReceivedOnChar2(connObj));

    constexpr uint8_t data[] = { to_underlying(BtpEngine::HeaderFlags::kStartMessage) |
                                     to_underlying(BtpEngine::HeaderFlags::kEndMessage),
                                 0x00, 0x01, 0x00, 0xff };
    auto buf                 = System::PacketBufferHandle::NewWithData(data, sizeof(data));
    ASSERT_FALSE(buf.IsNull());

    EXPECT_TRUE(HandleWriteReceived(connObj, &uuidSvc, &uuidChar1, std::move(buf)));
}

TEST_F(TestBleLayer, HandleWriteConfirmationInvalidUUID)
{
    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleWriteConfirmation(connObj, &uuidZero, &uuidZero));
    EXPECT_FALSE(HandleWriteConfirmation(connObj, &uuidSvc, &uuidChar2));
}

TEST_F(TestBleLayer, HandleWriteConfirmationUninitialized)
{
    ASSERT_FALSE(HandleWriteReceivedCapabilitiesRequest(BLE_CONNECTION_UNINITIALIZED));
}

TEST_F(TestBleLayer, HandleWriteConfirmation)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));

    EXPECT_TRUE(HandleWriteConfirmation(connObj, &uuidSvc, &uuidChar1));
}

TEST_F(TestBleLayer, HandleIndicationReceivedInvalidUUID)
{
    auto connObj = GetConnectionObject();
    auto buf     = System::PacketBufferHandle::New(0);
    ASSERT_FALSE(buf.IsNull());

    EXPECT_FALSE(HandleIndicationReceived(connObj, &uuidZero, &uuidZero, buf.Retain()));
    EXPECT_FALSE(HandleIndicationReceived(connObj, &uuidSvc, &uuidChar1, std::move(buf)));
}

TEST_F(TestBleLayer, HandleIndicationReceived)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));
    ASSERT_TRUE(HandleSubscribeReceivedOnChar2(connObj));

    constexpr uint8_t data[] = { to_underlying(BtpEngine::HeaderFlags::kStartMessage) |
                                     to_underlying(BtpEngine::HeaderFlags::kEndMessage),
                                 0x00, 0x01, 0x00, 0xff };
    auto buf                 = System::PacketBufferHandle::NewWithData(data, sizeof(data));
    ASSERT_FALSE(buf.IsNull());

    EXPECT_TRUE(HandleIndicationReceived(connObj, &uuidSvc, &uuidChar2, std::move(buf)));
}

TEST_F(TestBleLayer, HandleIndicationConfirmationInvalidUUID)
{
    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleIndicationConfirmation(connObj, &uuidZero, &uuidZero));
    EXPECT_FALSE(HandleIndicationConfirmation(connObj, &uuidSvc, &uuidChar1));
}

TEST_F(TestBleLayer, HandleIndicationConfirmation)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));

    EXPECT_TRUE(HandleIndicationConfirmation(connObj, &uuidSvc, &uuidChar2));
}

TEST_F(TestBleLayer, HandleConnectionError)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));

    HandleConnectionError(connObj, CHIP_ERROR_ACCESS_DENIED);
}

TEST_F(TestBleLayer, CloseBleConnectionUninitialized)
{
    CloseBleConnection(BLE_CONNECTION_UNINITIALIZED);
}

TEST_F(TestBleLayer, CloseBleConnection)
{
    auto connObj = GetConnectionObject();
    ASSERT_TRUE(HandleWriteReceivedCapabilitiesRequest(connObj));

    CloseBleConnection(connObj);
}

TEST_F(TestBleLayer, ExceedBleConnectionEndPointLimit)
{
    for (size_t i = 0; i < BLE_LAYER_NUM_BLE_ENDPOINTS; i++)
    {
        // Saturate BLE end-point pool
        EXPECT_TRUE(HandleWriteReceivedCapabilitiesRequest(GetConnectionObject()));
    }

    auto connObj = GetConnectionObject();
    EXPECT_FALSE(HandleWriteReceivedCapabilitiesRequest(connObj));
}

}; // namespace Ble
}; // namespace chip
