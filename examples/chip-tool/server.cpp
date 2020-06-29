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
        printf("Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != CHIP_ZCL_CLUSTER_ON_OFF_SERVER_ATTRIBUTE_ON_OFF)
    {
        printf("Unknown attribute ID: %d", attributeId);
        return;
    }

    // At this point we can assume that value points to a boolean value.
    printf("OnOff: %d\n", value[0]);
}
}

void OnReceiveError(CHIP_ERROR error, const chip::Inet::IPPacketInfo & info)
{
    (void) info;
    printf("ReceiveError: %s\n", chip::ErrorStr(error));
}

void OnNewConnection(chip::Transport::PeerConnectionState * state, chip::SecureSessionMgr * transport)
{
    CHIP_ERROR err;
    (void) transport;

    printf("Received a new connection.\n");

    err = state->GetSecureSession().TemporaryManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key,
                                                               sizeof(local_private_key));
    assert(err == CHIP_NO_ERROR);

    return;
}

void OnMessageReceived(const chip::MessageHeader & header, chip::Transport::PeerConnectionState * state,
                       chip::System::PacketBuffer * buffer, chip::SecureSessionMgr * transport)
{
    (void) header;
    (void) transport;
    if (buffer != nullptr)
    {
        printf("Got message from %lx", state->GetPeerNodeId());
        chipZclProcessIncoming(reinterpret_cast<ChipZclBuffer_t *>(buffer));
        chip::System::PacketBuffer::Free(buffer);
    }
}

void SetupTransport(chip::SecureSessionMgr & transport, chip::Inet::IPAddressType type)
{
    chip::Transport::UdpListenParameters p;
    p.SetAddressType(type).SetListenPort(11095);
    assert(transport.Init(kLocalNodeId, &chip::DeviceLayer::InetLayer, p) == CHIP_NO_ERROR);
    transport.SetMessageReceiveHandler(OnMessageReceived, &transport);
    transport.SetReceiveErrorHandler(OnReceiveError);
    transport.SetNewConnectionHandler(OnNewConnection, &transport);
}

int main()
{
    chip::DeviceLayer::PlatformMgr().InitChipStack();
    chipZclEndpointInit();

    chip::SecureSessionMgr transport_ipv6;
    chip::SecureSessionMgr transport_ipv4;
    SetupTransport(transport_ipv4, chip::Inet::kIPAddressType_IPv4);
    SetupTransport(transport_ipv6, chip::Inet::kIPAddressType_IPv6);

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
