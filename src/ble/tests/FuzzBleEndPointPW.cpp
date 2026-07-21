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

/**
 *    @file
 *      Seeded FuzzTest harness for BLEEndPoint::Receive. Drives the
 *      capabilities handshake with `ElementOf`-bounded windowSize / MTU values
 *      that target the field-validation logic, plus seeded BTP fragment slots
 *      for the post-Connected reorder queue.
 *
 *      WindowSize values cover: 0, 1, MIN-1, MIN, MIN+1, MAX, MAX+1, 0xFF.
 *      MTU values cover: spec floor (23), the underflow boundary (mtu - 3
 *      arithmetic), MTU = 0, MTU = 1, larger BLE 4.2 / 5.x sizes.
 */

#include <cstdint>
#include <cstdlib>
#include <tuple>
#include <utility>
#include <vector>

#include <pw_fuzzer/fuzztest.h>
#include <pw_unit_test/framework.h>

#include <ble/Ble.h>
#include <ble/BleConfig.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace Ble {

class FuzzBleLayerDelegate : public BleLayerDelegate
{
public:
    void OnBleConnectionComplete(BLEEndPoint *) override {}
    void OnBleConnectionError(CHIP_ERROR) override {}
    void OnEndPointConnectComplete(BLEEndPoint *, CHIP_ERROR) override {}
    void OnEndPointMessageReceived(BLEEndPoint *, System::PacketBufferHandle &&) override {}
    void OnEndPointConnectionClosed(BLEEndPoint *, CHIP_ERROR) override {}
    CHIP_ERROR SetEndPoint(BLEEndPoint *) override { return CHIP_NO_ERROR; }
};

class FuzzBlePlatformDelegate : public BlePlatformDelegate
{
public:
    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT) override { return CHIP_NO_ERROR; }
    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return 23; }
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *, System::PacketBufferHandle) override
    {
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT, const ChipBleUUID *, const ChipBleUUID *,
                                System::PacketBufferHandle) override
    {
        return CHIP_NO_ERROR;
    }
};

class FuzzBleApplicationDelegate : public BleApplicationDelegate
{
public:
    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT) override {}
};

} // namespace Ble
} // namespace chip

namespace {

using chip::Ble::BLEEndPoint;
using chip::Ble::BleLayer;
using chip::Ble::FuzzBleApplicationDelegate;
using chip::Ble::FuzzBleLayerDelegate;
using chip::Ble::FuzzBlePlatformDelegate;
using chip::Ble::kBleRole_Central;
using chip::System::PacketBufferHandle;
using namespace fuzztest;

void EnsureInitialized()
{
    static const bool sInitialized = [] {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        VerifyOrDie(chip::DeviceLayer::SystemLayer().Init() == CHIP_NO_ERROR);
        std::atexit([] { chip::DeviceLayer::SystemLayer().Shutdown(); });
        return true;
    }();
    (void) sInitialized;
}

// Boundary values for the window-size field — the interesting cases cluster at
// 0..effective-min and around MAX. Per BleConfig.h the effective minimum for
// stability is 3 (BLE_MAX_RECEIVE_WINDOW_SIZE must be > 2), so cover 0/1/2 plus
// the boundary around MAX.
auto AnyWindowSize()
{
    return ElementOf<uint8_t>({
        static_cast<uint8_t>(0),
        static_cast<uint8_t>(1),
        static_cast<uint8_t>(2),
        static_cast<uint8_t>(3),
        static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE - 1),
        static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE),
        static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE + 1),
        static_cast<uint8_t>(0xFE),
        static_cast<uint8_t>(0xFF),
    });
}

// MTU values that target the `mtu - 3` underflow path in the BTP fragment-
// length calculation.
auto AnyMtu()
{
    return ElementOf<uint16_t>({
        0,
        1,
        2,
        3,
        4,
        20,
        23, // BLE 4.0 ATT_MTU floor
        100,
        185, // BLE 4.2 default
        247, // BLE 5.x extended
        512,
        1024,
        0xFFFE,
        0xFFFF,
    });
}

// Structure-aware BTP data-phase fragments. Like the BtpEngine harness, the
// harness stamps the running sequence number (the BtpEngine inside the endpoint
// gates on seq == expected, so raw bytes bounce at the 2nd fragment). The ack
// bit is excluded (no TX window). The post-handshake seq base (0 or 1) depends
// on role / handshake direction, so it is a fuzzed bit (seqBaseOne) that
// coverage feedback resolves per role.
using EpFrag = std::tuple<uint8_t, uint16_t, std::vector<uint8_t>>; // (flags, declaredLen, payload)

std::vector<std::vector<EpFrag>> EpFragSeeds()
{
    return {
        { EpFrag{ 0x05, 2, { 0xDE, 0xAD } } },
        { EpFrag{ 0x01, 4, { 0x01, 0x02 } }, EpFrag{ 0x04, 0, { 0x03, 0x04 } } },
    };
}

void BleEndpointReceiveDoesNotCrash(uint8_t windowSize, uint16_t fragmentSize, bool seqBaseOne, const std::vector<EpFrag> & frags)
{
    EnsureInitialized();

    BleLayer layer;
    FuzzBlePlatformDelegate platformDelegate;
    FuzzBleApplicationDelegate appDelegate;
    FuzzBleLayerDelegate layerDelegate;

    if (layer.Init(&platformDelegate, &appDelegate, &chip::DeviceLayer::SystemLayer()) != CHIP_NO_ERROR)
    {
        return;
    }
    layer.mBleTransport = &layerDelegate;

    // Single fixed non-null connObj for this central endpoint.
    BLE_CONNECTION_OBJECT connObj = reinterpret_cast<BLE_CONNECTION_OBJECT>(static_cast<uintptr_t>(0x1));

    BLEEndPoint * ep = nullptr;
    if (layer.NewBleEndPoint(&ep, connObj, kBleRole_Central, /* autoClose */ true) != CHIP_NO_ERROR || ep == nullptr)
    {
        layer.Shutdown();
        return;
    }

    // Drive the central all the way to Connected, mirroring
    // TestBleEndPoint::CompleteCentralHandshake. ONLY in the Connected state does
    // BLEEndPoint::Receive forward fragments to the BTP reassembler — without this
    // the data path is dead code (the bottleneck that capped this harness before).
    if (ep->StartConnect() == CHIP_NO_ERROR)
    {
        (void) layer.HandleWriteConfirmation(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_1_UUID);
        (void) layer.HandleSubscribeComplete(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_2_UUID);

        chip::Ble::BleTransportCapabilitiesResponseMessage resp;
        resp.mSelectedProtocolVersion = chip::Ble::kBleTransportProtocolVersion_V4; // negotiation OK
        resp.mFragmentSize            = fragmentSize;                               // fuzzed field
        resp.mWindowSize              = windowSize;                                 // fuzzed field

        PacketBufferHandle respBuf = PacketBufferHandle::New(chip::Ble::kCapabilitiesResponseLength);
        if (!respBuf.IsNull() && resp.Encode(respBuf) == CHIP_NO_ERROR)
        {
            RETURN_SAFELY_IGNORED ep->Receive(std::move(respBuf));
        }
    }

    // BTP data phase: stamp the running sequence number so fragments get past
    // the BtpEngine seq gate and exercise reassembly. The base is fuzzed.
    uint8_t seq = seqBaseOne ? 1 : 0; // chip::Ble::SequenceNumber_t is uint8_t
    for (const auto & f : frags)
    {
        const uint8_t flags                  = std::get<0>(f);
        const uint16_t declaredLen           = std::get<1>(f);
        const std::vector<uint8_t> & payload = std::get<2>(f);

        std::vector<uint8_t> frame;
        frame.push_back(flags);
        frame.push_back(seq);
        if (flags & 0x01) // kStartMessage carries a 16-bit total length
        {
            frame.push_back(static_cast<uint8_t>(declaredLen));
            frame.push_back(static_cast<uint8_t>(declaredLen >> 8));
        }
        frame.insert(frame.end(), payload.begin(), payload.end());

        auto buf = PacketBufferHandle::NewWithData(frame.data(), frame.size());
        if (buf.IsNull())
            continue;
        RETURN_SAFELY_IGNORED ep->Receive(std::move(buf));
        seq = static_cast<uint8_t>(seq + 1);
    }

    ep->Close();
    layer.Shutdown();
}

FUZZ_TEST(FuzzBleEndPointPW, BleEndpointReceiveDoesNotCrash)
    .WithDomains(AnyWindowSize(), /* fragmentSize */ AnyMtu(), /* seqBaseOne */ Arbitrary<bool>(),
                 VectorOf(TupleOf(ElementOf<uint8_t>({ 0x01, 0x02, 0x04, 0x05, 0x06, 0x03, 0x07, 0x00 }), Arbitrary<uint16_t>(),
                                  Arbitrary<std::vector<uint8_t>>()))
                     .WithMaxSize(8)
                     .WithSeeds(EpFragSeeds()));

} // namespace
