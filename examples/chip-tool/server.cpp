#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include <transport/SecureSessionMgr.h>
#include <transport/UDP.h>

#include <cassert>

extern "C" {
#include "chip-zcl/chip-zcl.h"
#include "gen/gen-cluster-id.h"
#include "gen/gen-command-id.h"
#include "gen/gen-types.h"
}

static const unsigned char local_private_key[] = { 0xc6, 0x1a, 0x2f, 0x89, 0x36, 0x67, 0x2b, 0x26, 0x12, 0x47, 0x4f,
                                                   0x11, 0x0e, 0x34, 0x15, 0x81, 0x81, 0x12, 0xfc, 0x36, 0xeb, 0x65,
                                                   0x61, 0x07, 0xaa, 0x63, 0xe8, 0xc5, 0x22, 0xac, 0x52, 0xa1 };

static const unsigned char remote_public_key[] = { 0x04, 0x30, 0x77, 0x2c, 0xe7, 0xd4, 0x0a, 0xf2, 0xf3, 0x19, 0xbd, 0xfb, 0x1f,
                                                   0xcc, 0x88, 0xd9, 0x83, 0x25, 0x89, 0xf2, 0x09, 0xf3, 0xab, 0xe4, 0x33, 0xb6,
                                                   0x7a, 0xff, 0x73, 0x3b, 0x01, 0x35, 0x34, 0x92, 0x73, 0x14, 0x59, 0x0b, 0xbd,
                                                   0x44, 0x72, 0x1b, 0xcd, 0xb9, 0x02, 0x53, 0xd9, 0xaf, 0xcc, 0x1a, 0xcd, 0xae,
                                                   0xe8, 0x87, 0x2e, 0x52, 0x3b, 0x98, 0xf0, 0xa1, 0x88, 0x4a, 0xe3, 0x03, 0x75 };

constexpr chip::NodeId kLocalNodeId = 12344321;

#define println(format, ...) printf(format "\n", ##__VA_ARGS__)

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;

namespace {
chip::SecureSessionMgr<chip::Transport::UDP> sessions;
}

extern "C" {
void chipZclPostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    (void) endpoint;
    (void) mask;
    (void) manufacturerCode;
    (void) type;
    (void) size;
    if (clusterId != CHIP_ZCL_CLUSTER_ON_OFF)
    {
        println("Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF)
    {
        println("Unknown attribute ID: %d", attributeId);
        return;
    }

    // At this point we can assume that value points to a boolean value.
    println("OnOff: %d", value[0]);
}
}

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
        VerifyOrExit(buffer != NULL, println("Received data but couldn't process it..."));
        VerifyOrExit(header.GetSourceNodeId().HasValue(), println("Unknown source for received message"));

        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, println("Unknown source for received message"));

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

        println("Packet received from %s: %zu bytes", src_addr, static_cast<size_t>(data_len));

        HandleDataModelMessage(buffer);
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
        CHIP_ERROR err;

        println("Received a new connection.");

        err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key,
                                                                   sizeof(local_private_key));
        VerifyOrExit(err == CHIP_NO_ERROR, println("Failed to setup encryption\n"));

    exit:
        return;
    }

private:
    /**
     * Handle a message that should be processed via our data model processing
     * codepath.
     *
     * @param [in] buffer The buffer holding the message.  This function guarantees
     *                    that it will free the buffer before returning.
     */
    void HandleDataModelMessage(System::PacketBuffer * buffer)
    {
        ChipZclStatus_t zclStatus = chipZclProcessIncoming((ChipZclBuffer_t *) buffer);
        if (zclStatus == CHIP_ZCL_STATUS_SUCCESS)
        {
            println("Data model processing success!");
        }
        else
        {
            println("Data model processing failure: %d", zclStatus);
        }
        System::PacketBuffer::Free(buffer);
    }
};

static ServerCallback gCallbacks;

} // namespace

void InitDataModelHandler()
{
    chipZclEndpointInit();
}

// The echo server assumes the platform's networking has been setup already
void StartServer(chip::SecureSessionMgr<chip::Transport::UDP> * sessions)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = sessions->Init(kLocalNodeId, &DeviceLayer::SystemLayer,
                         UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
    SuccessOrExit(err);

    sessions->SetDelegate(&gCallbacks);

exit:
    if (err != CHIP_NO_ERROR)
    {
        println("ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        println("Lock Server Listening...");
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
