#pragma once
#include <stdint.h>

#include "LsRequester.h"
#include <cstdint>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <lib/core/CHIPError.h>
#include <luna-service2++/handle.hpp>
#include <platform/GLibTypeDeleter.h>
#include <string>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {
class WbsConnection;
struct WbsEndpoint
{
    // map device path to the connection
    GHashTable * mConnectionMap;
    bool mIsCentral;
    // Need To implement ShotDown() function
};

struct ConnectParams
{
    ConnectParams(char * remoteAddress, WbsEndpoint * endpoint) : mAddress(remoteAddress), mEndpoint(endpoint), mNumRetries(0) {}
    char * mAddress;
    WbsEndpoint * mEndpoint;
    uint16_t mNumRetries;
};

struct ConnectionDataBundle
{
    WbsConnection * mConn;
    chip::System::PacketBufferHandle buf;
};

class WbsConnection
{
public:
    WbsConnection();
    ~WbsConnection() = default;
    CHIP_ERROR InitConnectionData(bool aIsCentral, WbsEndpoint *& apEndpoint);
    CHIP_ERROR ShutdownWbsLayer(WbsEndpoint * apEndpoint);

    const char * GetPeerAddress() const { return mPeerAddress; };

    uint16_t GetMTU() const { return mMtu; };
    void SetMTU(uint16_t aMtu) { mMtu = aMtu; };

    bool IsNotifyAcquired() const { return mNotifyAcquired; }
    void SetNotifyAcquired(bool aNotifyAcquired) { mNotifyAcquired = aNotifyAcquired; }

    /// 1. Send indication to the CHIP RX characteristic on the remote peripheral device
    CHIP_ERROR SendIndication(chip::System::PacketBufferHandle apBuf);
    /// 2. Write to the CHIP RX characteristic on the remote peripheral device
    CHIP_ERROR SendWriteRequest(chip::System::PacketBufferHandle apBuf);
    /// 3. subscribe from the CHIP TX characteristic on the remote peripheral device
    CHIP_ERROR SubscribeCharacteristic();
    /// 4. Unsubscribe from the CHIP TX characteristic on the remote peripheral device
    CHIP_ERROR UnsubscribeCharacteristic();
    CHIP_ERROR CloseConnection();

    static bool GattGetStatus(std::string address);
    static bool GattGetServices(std::string address);

    static bool gattMonitorCharateristicsCb(LSHandle * sh, LSMessage * message, void * userData);
    static ConnectionDataBundle * MakeConnectionDataBundle(WbsConnection * apConn, chip::System::PacketBufferHandle apBuf);

    void EndpointCleanup(WbsEndpoint * apEndpoint);
    CHIP_ERROR ConnectDevice(std::string address, WbsEndpoint * aEndpoint);
    void CancelConnect(); // NEED TO IMPLETEMENT

private:
    struct IOChannel
    {
        IOChannel() = default;
        ~IOChannel();

        GAutoPtr<GIOChannel> mChannel;
        GAutoPtr<GSource> mWatchSource;
    };

    // Static Function
    static CHIP_ERROR ConnectDeviceImpl(ConnectParams * apParams);
    static CHIP_ERROR SendIndicationImpl(ConnectionDataBundle * data);
    static CHIP_ERROR SendWriteRequestImpl(ConnectionDataBundle * data);
    static CHIP_ERROR SubscribeCharacteristicImpl(BLE_CONNECTION_OBJECT apConn);
    static CHIP_ERROR UnsubscribeCharacteristicImpl(BLE_CONNECTION_OBJECT connection);
    static CHIP_ERROR CloseConnectionImpl(WbsConnection * conn);
    static void WbsOTConnectionDestroy(WbsConnection * aConn);
    static void UpdateConnectionTable(std::string remoteAddr, std::string clientId, WbsEndpoint & aEndpoint);

    char * mPeerAddress;
    bool mNotifyAcquired = false;
    uint16_t mMtu        = 0;
    std::string mClientId;
    uint32_t mMonitorToken;
    WbsEndpoint * mEndpoint;
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
