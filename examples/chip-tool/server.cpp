#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

#include <cassert>

#include "attribute-storage.h"
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include "gen/znet-bookkeeping.h"
#include "util.h"
#include <app/chip-zcl-zpro-codec.h>

constexpr chip::NodeId kLocalNodeId = 12344321;

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {
chip::SecureSessionMgr<chip::Transport::UDP> sessions;
}

extern "C" {
void emberAfPostAttributeChangeCallback(uint8_t endpoint, EmberAfClusterId clusterId, EmberAfAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    (void) endpoint;
    (void) mask;
    (void) manufacturerCode;
    (void) type;
    (void) size;
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        printf("Unknown cluster ID: %d\n", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        printf("Unknown attribute ID: %d\n", attributeId);
        return;
    }

    // At this point we can assume that value points to a boolean value.
    printf("OnOff: %d\n", value[0]);
}
}

namespace {
class ServerCallback : public SecureSessionMgrCallback
{
public:
    void OnMessageReceived(const MessageHeader & header, Transport::PeerConnectionState * state, System::PacketBuffer * buffer,
                           SecureSessionMgrBase * mgr) override
    {
        const size_t data_len = buffer->DataLength();
        char src_addr[PeerAddress::kMaxToStringSize];

        // as soon as a client connects, assume it is connected
        VerifyOrExit(buffer != NULL, printf("Received data but couldn't process it...\n"));
        VerifyOrExit(header.GetSourceNodeId().HasValue(), printf("Unknown source for received message\n"));

        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, printf("Unknown source for received message\n"));

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

        printf("Packet received from %s: %zu bytes\n", src_addr, static_cast<size_t>(data_len));

        HandleDataModelMessage(header, buffer, mgr);
        buffer = NULL;

    exit:
        // SendTo calls Free on the buffer without an AddRef, if SendTo was not called, free the buffer.
        if (buffer != NULL)
        {
            System::PacketBuffer::Free(buffer);
        }
    }

    void OnNewConnection(Transport::PeerConnectionState * state, SecureSessionMgrBase * mgr) override
    {
        printf("Received a new connection.\n");
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
        bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame);
        if (ok)
        {
            printf("APS frame processing success!\n");
        }
        else
        {
            printf("APS frame processing failure\n");
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
            printf("Data model processing success!\n");
        }
        else
        {
            printf("Data model processing failure\n");
        }
    }
};

ServerCallback gCallbacks;
static SecurePairingUsingTestSecret gTestPairing;

} // namespace

void InitDataModelHandler()
{
    emberAfEndpointConfigure();
    emAfInit();
}

// The echo server assumes the platform's networking has been setup already
void StartServer(chip::SecureSessionMgr<chip::Transport::UDP> * sessions)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    err = sessions->Init(kLocalNodeId, &DeviceLayer::SystemLayer,
                         UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
    SuccessOrExit(err);

    err = sessions->NewPairing(Optional<NodeId>::Value(kUndefinedNodeId), peer, 0, 0, &gTestPairing);
    SuccessOrExit(err);

    sessions->SetDelegate(&gCallbacks);

exit:
    if (err != CHIP_NO_ERROR)
    {
        printf("ERROR setting up transport: %s\n", ErrorStr(err));
    }
    else
    {
        printf("Lock Server Listening...\n");
    }
}

int main()
{
    chip::DeviceLayer::PlatformMgr().InitChipStack();

    // Init ZCL Data Model
    InitDataModelHandler();
    StartServer(&sessions);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
