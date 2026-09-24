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
 *      Seeded FuzzTest harness for the BleLayer multi-endpoint inbound-dispatch
 *      seam — the un-fuzzed territory that FuzzBleEndPointPW never reaches
 *      (it calls NewBleEndPoint(nullptr, ...) which BleLayer rejects, so no
 *      BLEEndPoint::Receive, no capabilities handshake, and no teardown ever
 *      runs there).
 *
 *      This harness drives external pre-auth inbound upcalls through the real
 *      entry points BleLayer::HandleWriteReceived / HandleIndicationReceived /
 *      HandleSubscribeReceived / HandleUnsubscribeReceived /
 *      HandleWriteConfirmation / HandleIndicationConfirmation /
 *      HandleConnectionError with NON-NULL connObj values, so that the pool
 *      Find/GetFree/NewBleEndPoint allocation path, BLEEndPoint::Receive, the
 *      capabilities handshake field validation, BtpEngine reassembly, and the
 *      DoClose/FinalizeClose/Free/Release teardown lifecycle all light up.
 *
 *      Faithfulness: ONE persistent BleLayer is built and Init'd ONCE behind a
 *      call_once singleton, together with the SystemLayer and all delegates. The
 *      endpoint pool storage is a process-global function-local static; a
 *      per-iteration BleLayer would leave pooled endpoints whose mBle dangles to
 *      a destroyed layer and produce a false cross-iteration ASan finding. The
 *      ONLY per-iteration teardown is CloseAllBleConnections(), the real
 *      production teardown path that Abort()/Free()s every live endpoint and
 *      restores every slot to mBle==nullptr.
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

// Sink delegates: swallow everything, never re-enter the layer. (Class A — no
// trust-boundary validation; the completed-message sink is downstream/out of
// scope, GetMTU returns a faithful >= 23 value, the handshake's own field
// validation runs unmodified.) Copied verbatim from FuzzBleEndPointPW.cpp.
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
    uint16_t GetMTU(BLE_CONNECTION_OBJECT) const override { return 247; }
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
using chip::Ble::ChipBleUUID;
using chip::Ble::FuzzBleApplicationDelegate;
using chip::Ble::FuzzBleLayerDelegate;
using chip::Ble::FuzzBlePlatformDelegate;
using chip::System::PacketBufferHandle;
using namespace fuzztest;

// ---------------------------------------------------------------------------
// Process-lifetime singletons (see the persistent-layer note in the file header).
//
// The BleLayer, its delegates and the SystemLayer are constructed and Init'd
// EXACTLY ONCE for the whole process. Never per-iteration. The endpoint pool
// is a process-global function-local static; tearing the layer down per
// iteration would dangle pooled endpoints' mBle to a freed layer and produce a
// FALSE cross-iteration UAF.
// ---------------------------------------------------------------------------
BleLayer & PersistentLayer()
{
    static FuzzBlePlatformDelegate sPlatformDelegate;
    static FuzzBleApplicationDelegate sAppDelegate;
    static FuzzBleLayerDelegate sLayerDelegate;
    static BleLayer * const sLayer = [&]() -> BleLayer * {
        VerifyOrDie(chip::Platform::MemoryInit() == CHIP_NO_ERROR);
        VerifyOrDie(chip::DeviceLayer::SystemLayer().Init() == CHIP_NO_ERROR);
        std::atexit([] { chip::DeviceLayer::SystemLayer().Shutdown(); });
        // Intentionally never deleted: a single long-lived BleLayer, exactly as
        // production has. The pool storage outlives any per-iteration state.
        auto * layer = new BleLayer();
        VerifyOrDie(layer->Init(&sPlatformDelegate, &sAppDelegate, &chip::DeviceLayer::SystemLayer()) == CHIP_NO_ERROR);
        layer->mBleTransport = &sLayerDelegate;
        return layer;
    }();
    return *sLayer;
}

// ---------------------------------------------------------------------------
// Dispatch axes.
// ---------------------------------------------------------------------------
enum OpType : uint8_t
{
    kWriteReceived          = 0, // CHAR_1 — new-connection / data path
    kIndicationReceived     = 1, // CHAR_2 — central handshake / data path
    kSubscribeReceived      = 2, // CHAR_2
    kUnsubscribeReceived    = 3, // CHAR_2
    kWriteConfirmation      = 4, // CHAR_1 — ack path (faithful, public upcall)
    kIndicationConfirmation = 5, // CHAR_2 — ack path (faithful, public upcall)
    kConnectionError        = 6, // teardown
    kNumOps                 = 7,
};

// A single fuzzer-chosen operation: (opType, connId, payload).
using Op  = std::tuple<uint8_t, uint8_t, std::vector<uint8_t>>;
using Ops = std::vector<Op>;

// connId -> distinct non-null BLE_CONNECTION_OBJECT. Find/GetFree only compare
// pointer identity and test mBle!=nullptr; they never dereference connObj. The
// +1 guarantees non-null (BLE_CONNECTION_UNINITIALIZED == nullptr is rejected).
BLE_CONNECTION_OBJECT ConnObj(uint8_t connId)
{
    return reinterpret_cast<BLE_CONNECTION_OBJECT>(static_cast<uintptr_t>(connId % 4) + 1);
}

// Small set of teardown error codes to drive.
CHIP_ERROR PickError(const std::vector<uint8_t> & payload)
{
    const CHIP_ERROR errs[] = {
        BLE_ERROR_REMOTE_DEVICE_DISCONNECTED,
        BLE_ERROR_APP_CLOSED_CONNECTION,
        CHIP_ERROR_TIMEOUT,
    };
    const uint8_t sel = payload.empty() ? 0 : static_cast<uint8_t>(payload[0] % 3);
    return errs[sel];
}

PacketBufferHandle MakeBuf(const std::vector<uint8_t> & payload)
{
    return PacketBufferHandle::NewWithData(payload.data(), payload.size());
}

void DispatchOne(BleLayer & layer, const Op & op)
{
    const uint8_t opType                 = static_cast<uint8_t>(std::get<0>(op) % kNumOps);
    BLE_CONNECTION_OBJECT connObj        = ConnObj(std::get<1>(op));
    const std::vector<uint8_t> & payload = std::get<2>(op);

    switch (opType)
    {
    case kWriteReceived: {
        auto buf = MakeBuf(payload);
        if (!buf.IsNull())
        {
            (void) layer.HandleWriteReceived(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_1_UUID,
                                             std::move(buf));
        }
        break;
    }
    case kIndicationReceived: {
        auto buf = MakeBuf(payload);
        if (!buf.IsNull())
        {
            (void) layer.HandleIndicationReceived(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_2_UUID,
                                                  std::move(buf));
        }
        break;
    }
    case kSubscribeReceived:
        (void) layer.HandleSubscribeReceived(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_2_UUID);
        break;
    case kUnsubscribeReceived:
        (void) layer.HandleUnsubscribeReceived(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_2_UUID);
        break;
    case kWriteConfirmation:
        // Faithful ack path: BleLayer::HandleWriteConfirmation -> HandleAckReceived.
        (void) layer.HandleWriteConfirmation(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_1_UUID);
        break;
    case kIndicationConfirmation:
        // Faithful ack path: BleLayer::HandleIndicationConfirmation -> HandleAckReceived.
        (void) layer.HandleIndicationConfirmation(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_2_UUID);
        break;
    case kConnectionError:
        layer.HandleConnectionError(connObj, PickError(payload));
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// The property: dispatch a bounded sequence of inbound upcalls, then reset the
// pool via the real teardown path. No crash / no sanitizer error must occur.
// ---------------------------------------------------------------------------
// Establish a Connected central endpoint via the proven CompleteCentralHandshake
// recipe (see TestBleEndPoint.cpp / FuzzBleEndPointPW): StartConnect ->
// WriteConfirmation -> SubscribeComplete -> Receive(Encode'd V4 response). Without
// this the dispatch only ever sees freshly-allocated / Connecting endpoints; with
// it the fuzzed ops exercise Connected-state data delivery and teardown.
void PreconnectCentral(BleLayer & layer, BLE_CONNECTION_OBJECT connObj)
{
    BLEEndPoint * ep = nullptr;
    if (layer.NewBleEndPoint(&ep, connObj, chip::Ble::kBleRole_Central, /* autoClose */ true) != CHIP_NO_ERROR || ep == nullptr)
        return;
    if (ep->StartConnect() != CHIP_NO_ERROR)
        return;
    (void) layer.HandleWriteConfirmation(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_1_UUID);
    (void) layer.HandleSubscribeComplete(connObj, &chip::Ble::CHIP_BLE_SVC_ID, &chip::Ble::CHIP_BLE_CHAR_2_UUID);

    chip::Ble::BleTransportCapabilitiesResponseMessage resp;
    resp.mSelectedProtocolVersion = chip::Ble::kBleTransportProtocolVersion_V4;
    resp.mFragmentSize            = 247;
    resp.mWindowSize              = 4;
    PacketBufferHandle buf        = PacketBufferHandle::New(chip::Ble::kCapabilitiesResponseLength);
    if (!buf.IsNull() && resp.Encode(buf) == CHIP_NO_ERROR)
        (void) ep->Receive(std::move(buf));
}

void BleLayerDispatchDoesNotCrash(const Ops & ops)
{
    BleLayer & layer = PersistentLayer();

    // Pre-establish Connected central endpoints for the connObjs the ops address,
    // so the fuzzed dispatch exercises Connected-state paths (data delivery,
    // connected-state teardown, multi-endpoint interaction), not just allocation.
    //
    // NOTE: the default BLE_LAYER_NUM_BLE_ENDPOINTS is 1, so only one of these
    // connects and the rest hit CHIP_ERROR_ENDPOINT_POOL_FULL. To exercise the
    // multi-endpoint pool/dispatch/teardown paths, build with the pool raised,
    // e.g. -DBLE_LAYER_NUM_BLE_ENDPOINTS=4.
    for (uint8_t cid = 0; cid < 4; ++cid)
    {
        PreconnectCentral(layer, ConnObj(cid));
    }

    for (const auto & op : ops)
    {
        DispatchOne(layer, op);
    }

    // Faithful per-iteration reset: Abort()/Free() every live endpoint through
    // the production teardown path so every pool slot returns to mBle==nullptr.
    // This is NOT a layer teardown (the layer is persistent — see the file header).
    layer.CloseAllBleConnections();
}

// ---------------------------------------------------------------------------
// Seed corpus — one representative per dispatch arm, plus lifetime/teardown
// interleavings, so the mutator can build from each (must pass UUID/null gates).
// ---------------------------------------------------------------------------
std::vector<uint8_t> CapReq(uint8_t windowSize, uint16_t mtu)
{
    // Wire bytes of a BTP capabilities request.
    // Built directly so seed construction (which runs at static-init time via
    // .WithSeeds(SeedSequences())) never calls chip::Platform::MemoryAlloc before
    // Platform::MemoryInit() — that ordering aborts in VerifyInitialized().
    //
    // Wire layout (kCapabilitiesRequestLength=9): magic(2) + supportedVersions(4)
    // + mtu(2 LE) + windowSize(1). The 4 version bytes encode V4 in the low nibble
    // of byte 0. (Earlier 3-version layout misaligned mtu/windowSize -> window 0.)
    return { 0x65, 0x6C, 0x04, 0x00, 0x00, 0x00, static_cast<uint8_t>(mtu), static_cast<uint8_t>(mtu >> 8), windowSize };
}
std::vector<uint8_t> CapResp(uint8_t windowSize, uint16_t fragmentSize)
{
    // Wire bytes of a BTP capabilities response.
    return { 0x65, 0x6C, 0x04, static_cast<uint8_t>(fragmentSize), static_cast<uint8_t>(fragmentSize >> 8), windowSize };
}

Op MkOp(uint8_t opType, uint8_t connId, std::vector<uint8_t> payload)
{
    return std::make_tuple(opType, connId, std::move(payload));
}

std::vector<Ops> SeedSequences()
{
    // BTP data fragment shapes (from FuzzBleEndPointPW BtpDataFragmentSeeds).
    const std::vector<uint8_t> startEnd  = { 0x05, 0x01, 0x01, 0x00, 0x00 };
    const std::vector<uint8_t> startOnly = { 0x01, 0x00, 0x01, 0x00, 0x00 };
    const std::vector<uint8_t> endOnly   = { 0x04, 0x01, 0x02, 0x00 };
    const std::vector<uint8_t> pureAck   = { 0x08 };

    std::vector<Ops> seeds;

    // Peripheral handshake -> Connected -> BTP data path (conn0).
    seeds.push_back({
        MkOp(kWriteReceived, 0, CapReq(6, 247)),
        MkOp(kSubscribeReceived, 0, {}),
        MkOp(kWriteReceived, 0, startEnd),
    });

    // Central handshake (conn0) via indication + capabilities response.
    seeds.push_back({
        MkOp(kIndicationReceived, 0, CapResp(6, 200)),
        MkOp(kIndicationReceived, 0, startEnd),
    });

    // Handshake field-validation boundaries (windowSize / mtu sweeps).
    const uint8_t windowSizes[] = { 0,
                                    1,
                                    2,
                                    3,
                                    static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE - 1),
                                    static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE),
                                    static_cast<uint8_t>(BLE_MAX_RECEIVE_WINDOW_SIZE + 1),
                                    0xFE,
                                    0xFF };
    const uint16_t mtus[]       = { 0, 1, 2, 3, 4, 20, 23, 100, 185, 247, 512, 1024, 0xFFFE, 0xFFFF };
    for (uint8_t ws : windowSizes)
    {
        for (uint16_t mtu : mtus)
        {
            seeds.push_back({ MkOp(kWriteReceived, 0, CapReq(ws, mtu)), MkOp(kSubscribeReceived, 0, {}) });
            seeds.push_back({ MkOp(kIndicationReceived, 0, CapResp(ws, mtu)) });
        }
    }

    // Disconnect during an in-progress reassembly.
    seeds.push_back({
        MkOp(kWriteReceived, 0, CapReq(6, 247)), MkOp(kSubscribeReceived, 0, {}), MkOp(kWriteReceived, 0, startOnly),
        MkOp(kConnectionError, 0, { 0 }), // BLE_ERROR_REMOTE_DEVICE_DISCONNECTED
    });

    // Unsubscribe followed by connection-error on the same slot.
    seeds.push_back({
        MkOp(kWriteReceived, 0, CapReq(6, 247)),
        MkOp(kSubscribeReceived, 0, {}),
        MkOp(kUnsubscribeReceived, 0, {}),
        MkOp(kConnectionError, 0, { 1 }),
    });

    // Multiple connections contending for the endpoint pool (pool-full path at NUM==1).
    seeds.push_back({
        MkOp(kWriteReceived, 0, CapReq(6, 247)),
        MkOp(kWriteReceived, 1, CapReq(6, 247)),
        MkOp(kSubscribeReceived, 0, {}),
        MkOp(kSubscribeReceived, 1, {}),
        MkOp(kConnectionError, 0, { 0 }),
        MkOp(kWriteReceived, 2, CapReq(6, 247)),
    });

    // Stray confirmations / data against never-opened or freed connObjs.
    seeds.push_back({
        MkOp(kWriteConfirmation, 3, {}),
        MkOp(kIndicationConfirmation, 3, {}),
        MkOp(kWriteReceived, 3, endOnly),
        MkOp(kIndicationReceived, 3, pureAck),
    });

    return seeds;
}

// Payload domain biased toward valid BTP capabilities-handshake frames so
// endpoints reach Connected (exercising the multi-endpoint pool's connected-
// state + teardown paths), while still covering arbitrary/malformed bytes. The
// capabilities handshake is not sequence-gated, so a single valid frame connects
// an endpoint without per-endpoint seq tracking (the BTP data path's seq gate is
// covered deeply by the dedicated FuzzBtpEnginePW / FuzzBleEndPointPW harnesses).
auto AnyBlePayload()
{
    return OneOf(
        // Capabilities REQUEST (peripheral, CHAR_1 write): magic(2) versions(4) mtu(2 LE) ws(1)
        Map(
            [](uint16_t mtu, uint8_t ws) {
                return std::vector<uint8_t>{
                    0x65, 0x6C, 0x04, 0x00, 0x00, 0x00, static_cast<uint8_t>(mtu), static_cast<uint8_t>(mtu >> 8), ws
                };
            },
            Arbitrary<uint16_t>(), Arbitrary<uint8_t>()),
        // Capabilities RESPONSE (central, CHAR_2 indication): 65 6C 04 fragLo fragHi ws
        Map(
            [](uint16_t frag, uint8_t ws) {
                return std::vector<uint8_t>{ 0x65, 0x6C, 0x04, static_cast<uint8_t>(frag), static_cast<uint8_t>(frag >> 8), ws };
            },
            Arbitrary<uint16_t>(), Arbitrary<uint8_t>()),
        // Arbitrary / malformed bytes.
        Arbitrary<std::vector<uint8_t>>());
}

} // namespace

FUZZ_TEST(FuzzBleLayerPW, BleLayerDispatchDoesNotCrash)
    .WithDomains(VectorOf(TupleOf(/* opType  */ ElementOf<uint8_t>({ kWriteReceived, kIndicationReceived, kSubscribeReceived,
                                                                     kUnsubscribeReceived, kWriteConfirmation,
                                                                     kIndicationConfirmation, kConnectionError }),
                                  /* connId   */ ElementOf<uint8_t>({ 0, 1, 2, 3 }),
                                  /* payload  */ AnyBlePayload()))
                     .WithMaxSize(24)
                     .WithSeeds(SeedSequences()));
