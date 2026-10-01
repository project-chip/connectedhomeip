#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include <glib.h>
#include <luna-service2/lunaservice.h>

#include <ble/Ble.h>
#include <lib/core/CHIPError.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct WbsEndpoint
{
    // map device path to the connection
    GHashTable * mConnectionMap;
    bool mIsCentral;
};

// Allocated with chip::Platform::New/Delete (has non-trivial members).
struct WbsConnection
{
    char * mpPeerAddress = nullptr;

    bool mIsNotify = false;
    uint16_t mMtu  = 0;
    std::string clientId;
    LSMessageToken ulMonitorToken = LSMESSAGE_TOKEN_INVALID;
    WbsEndpoint * mpEndpoint      = nullptr;

    // [LGE_MATTER_COMPAT_PATCH]
    LSMessageToken mMtuStatusToken = LSMESSAGE_TOKEN_INVALID;
    std::atomic<uint16_t> mNegotiatedMtu{ 0 };
};

CHIP_ERROR InitConnectionData(bool aIsCentral, WbsEndpoint *& apEndpoint);
CHIP_ERROR ShutdownWbsLayer(WbsEndpoint * apEndpoint);
CHIP_ERROR CloseWbsConnection(BLE_CONNECTION_OBJECT apConn);

/// Write to the CHIP RX characteristic on the remote peripheral device
CHIP_ERROR WbsSendWriteRequest(BLE_CONNECTION_OBJECT apConn, chip::System::PacketBufferHandle apBuf);
/// Subscribe to the CHIP TX characteristic on the remote peripheral device
CHIP_ERROR WbsSubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn);
/// Unsubscribe from the CHIP TX characteristic on the remote peripheral device
CHIP_ERROR WbsUnsubscribeCharacteristic(BLE_CONNECTION_OBJECT apConn);

// [LGE_MATTER_COMPAT_PATCH]
/// BLE write pacing control for whitelist-based delay
void SetActivePacingDelay(uint16_t vendorId, uint16_t productId);
void ClearActivePacingDelay();

/// Record BLE pacing TIMEOUT failure (stage 10-18) for auto-detection of devices needing pacing
/// Consecutive TIMEOUT failures will ADD the device to whitelist
void RecordPacingFailure(uint16_t vendorId, uint16_t productId, uint8_t stageFailed);

/// Record ANY commissioning failure for devices already in whitelist
/// Consecutive failures (any reason) will REMOVE the device from whitelist
/// This helps recover when pacing causes worse problems than it solves
void RecordCommissioningFailure(uint16_t vendorId, uint16_t productId);

/// Clear failure record on successful commissioning
void ClearPacingFailureRecord(uint16_t vendorId, uint16_t productId);

/// Blocking connect (gatt/connect + service discovery + MTU exchange). Must be called on the
/// Matter GLib context or a thread that may block; never on the LsRequester (lsTask) thread.
CHIP_ERROR ConnectDevice(std::string address, WbsEndpoint * apEndpoint);
void CancelConnect(WbsEndpoint * apEndpoint);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
