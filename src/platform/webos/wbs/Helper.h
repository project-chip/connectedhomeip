#pragma once

#include <cstdint>
#include <string>

namespace chip {
namespace DeviceLayer {
namespace Internal {

struct WbsEndpoint
{
    // map device path to the connection
    GHashTable * mConnectionMap;
    bool mIsCentral;
};

struct WbsConnection
{
    char * mpPeerAddress;

    bool mIsNotify;
    uint16_t mMtu;
    std::string clientId;
    uint32_t ulMonitorToken;
    WbsEndpoint * mpEndpoint;
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

CHIP_ERROR ConnectDevice(std::string address, WbsEndpoint * apEndpoint);
void CancelConnect(WbsEndpoint * apEndpoint);

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
