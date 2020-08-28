#include "server.h"

#include <string.h>

#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

#include "attribute-storage.h"
#include "gen/znet-bookkeeping.h"
#include "util.h"
#include <app/chip-zcl-zpro-codec.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

#ifndef EXAMPLE_SERVER_NODEID
// "nRF5"
#define EXAMPLE_SERVER_NODEID 12344321
#endif // EXAMPLE_SERVER_NODEID

// Transport Callbacks
namespace {

class ServerCallback : public SecureSessionMgrCallback
{
public:
    virtual void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state,
                                   System::PacketBuffer * buffer, SecureSessionMgrBase * mgr)
    {
        const size_t data_len = buffer->DataLength();
        char src_addr[PeerAddress::kMaxToStringSize];

        // as soon as a client connects, assume it is connected
        VerifyOrExit(buffer != NULL, ChipLogProgress(App, "Received data but couldn't process it..."));
        VerifyOrExit(header.GetSourceNodeId().HasValue(), ChipLogProgress(App, "Unknown source for received message"));

        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, ChipLogProgress(App, "Unknown source for received message"));

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

        ChipLogProgress(App, "Packet received from %s: %zu bytes", src_addr, static_cast<size_t>(data_len));

        HandleDataModelMessage(header, buffer, mgr);
        buffer = NULL;

    exit:
        // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
        if (buffer != NULL)
        {
            System::PacketBuffer::Free(buffer);
        }
    }

    virtual void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr)
    {
        ChipLogProgress(App, "Received a new connection.");
    }

private:
    /**
     * Handle a message that should be processed via our data model processing
     * codepath.
     *
     * @param [in] buffer The buffer holding the message.  This function guarantees
     *                    that it will free the buffer before returning.
     */
    void HandleDataModelMessage(const MessageHeader & header, System::PacketBuffer * buffer, SecureSessionMgrBase * mgr)
    {
        EmberApsFrame frame;
        bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) > 0;
        if (ok)
        {
            ChipLogProgress(App, "APS frame processing success!");
        }
        else
        {
            ChipLogProgress(App, "APS frame processing failure!");
            System::PacketBuffer::Free(buffer);
            return;
        }

        ChipResponseDestination responseDest(header.GetSourceNodeId().Value(), mgr);
        uint8_t * message;
        uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);
        ok                  = emberAfProcessMessage(&frame,
                                   0, // type
                                   message, messageLen,
                                   &responseDest, // source identifier
                                   NULL);

        System::PacketBuffer::Free(buffer);

        if (ok)
        {
            ChipLogProgress(App, "Data model processing success!");
        }
        else
        {
            ChipLogProgress(App, "Data model processing failure!");
        }
    }
};

ServerCallback gCallbacks;
SecurePairingUsingTestSecret gTestPairing;

} // namespace

// The echo server assumes the platform's networking has been setup already
void StartServer(DemoSessionManager * sessions)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    err = sessions->Init(EXAMPLE_SERVER_NODEID, &DeviceLayer::SystemLayer,
                         UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
    SuccessOrExit(err);

    err = sessions->NewPairing(Optional<NodeId>::Value(kUndefinedNodeId), peer, 0, 0, &gTestPairing);
    SuccessOrExit(err);

    sessions->SetDelegate(&gCallbacks);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(App, "ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        ChipLogProgress(App, "Lock Server Listening...");
    }
}
