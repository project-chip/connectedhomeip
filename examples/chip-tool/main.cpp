#include <assert.h>
#include <errno.h>
#include <iostream>
#include <new>
#include <sstream>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <core/CHIPError.h>
#include <inet/InetLayer.h>
#include <inet/UDPEndPoint.h>
#include <support/CHIPLogging.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

#include <controller/CHIPDeviceController.h>

extern "C" {
#include "chip-zcl/chip-zcl.h"
#include "gen/gen-cluster-id.h"
#include "gen/gen-command-id.h"
#include "gen/gen-types.h"
} // extern "C"

// Delay, in seconds, between sends for the echo case.
#define SEND_DELAY 5

using namespace ::chip;
using namespace ::chip::Inet;

static const char * PAYLOAD = "Message from Standalone CHIP echo client!";

// Device Manager Callbacks
static void EchoResponse(chip::DeviceController::ChipDeviceController * deviceController, void * appReqState,
                         System::PacketBuffer * buffer, const IPPacketInfo * packet_info)
{
    char src_addr[INET_ADDRSTRLEN];
    char dest_addr[INET_ADDRSTRLEN];
    size_t data_len = buffer->DataLength();

    packet_info->SrcAddress.ToString(src_addr, sizeof(src_addr));
    packet_info->DestAddress.ToString(dest_addr, sizeof(dest_addr));

    printf("UDP packet received from %s:%u to %s:%u (%zu bytes)\n", src_addr, packet_info->SrcPort, dest_addr,
           packet_info->DestPort, static_cast<size_t>(buffer->DataLength()));

    // attempt to print the incoming message
    char msg_buffer[data_len];
    msg_buffer[data_len] = 0; // Null-terminate whatever we received and treat like a string...
    memcpy(msg_buffer, buffer->Start(), data_len);
    int compare = strncmp(msg_buffer, PAYLOAD, data_len);
    if (compare == 0)
    {
        printf("Got expected Message...\n");
    }
    else
    {
        printf("Didn't get the expected Echo. Compare: %d\n", compare);
        printf("\nSend: %s \nRecv: %s\n", PAYLOAD, msg_buffer);
    }

    System::PacketBuffer::Free(buffer);
}

static void ReceiveError(chip::DeviceController::ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR error,
                         const IPPacketInfo * pi)
{
    printf("ERROR: %s\n Got UDP error\n", ErrorStr(error));
}

void ShowUsage(const char * executable)
{
    fprintf(stderr,
            "Usage: \n"
            "  %s device-ip-address echo|off|on|toggle\n",
            executable);
}

bool DetermineAddress(int argc, char * argv[], IPAddress * hostAddr)
{
    if (argc < 2)
    {
        return false;
    }

    if (!IPAddress::FromString(argv[1], *hostAddr))
    {
        fputs("Error: Invalid device IP address", stderr);
        return false;
    }

    return true;
}

enum class Command
{
    Off,
    On,
    Toggle,
    Echo,
};

template <int N>
bool EqualsLiteral(const char * str, const char (&literal)[N])
{
    return strncmp(str, literal, N) == 0;
}

bool DetermineCommand(int argc, char * argv[], Command * command)
{
    if (argc < 3)
    {
        return false;
    }

    if (EqualsLiteral(argv[2], "off"))
    {
        *command = Command::Off;
        return true;
    }

    if (EqualsLiteral(argv[2], "on"))
    {
        *command = Command::On;
        return true;
    }

    if (EqualsLiteral(argv[2], "toggle"))
    {
        *command = Command::Toggle;
        return true;
    }

    if (EqualsLiteral(argv[2], "echo"))
    {
        *command = Command::Echo;
        return true;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[2]);
    return false;
}

// Handle the echo case, where we just send a string and expect to get it back.
void DoEcho(DeviceController::ChipDeviceController * controller, const IPAddress & host_addr)
{
    size_t payload_len = strlen(PAYLOAD) + 1;

    // Run the client
    char host_ip_str[40];
    host_addr.ToString(host_ip_str, sizeof(host_ip_str));
    while (1)
    {
        // Reallocate buffer on each run, as the secure transport encrypts and
        // overwrites the buffer from previous iteration.
        auto * buffer = System::PacketBuffer::NewWithAvailableSize(payload_len);
        snprintf((char *) buffer->Start(), payload_len, "%s", PAYLOAD);
        buffer->SetDataLength(payload_len);

        controller->SendMessage(NULL, buffer);
        printf("Msg sent to server at %s:%d\n", host_ip_str, CHIP_PORT);

        controller->ServiceEvents();

        sleep(SEND_DELAY);
    }
}

// Handle the on/off/toggle case, where we are sending a ZCL command and not
// expecting a response at all.
void DoOnOff(DeviceController::ChipDeviceController * controller, Command command)
{
    ChipZclCommandId_t zclCommand;
    switch (command)
    {
    case Command::Off:
        zclCommand = CHIP_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_OFF;
        break;
    case Command::On:
        zclCommand = CHIP_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_ON;
        break;
    case Command::Toggle:
        zclCommand = CHIP_ZCL_CLUSTER_ON_OFF_SERVER_COMMAND_TOGGLE;
        break;
    default:
        fprintf(stderr, "Unknown command: %d\n", command);
        return;
    }

    // Make sure our buffer is big enough, but this will need a better setup!
    static const size_t bufferSize = 1024;
    auto * buffer                  = System::PacketBuffer::NewWithAvailableSize(bufferSize);

    ChipZclBuffer_t * zcl_buffer = (ChipZclBuffer_t *) buffer;
    ChipZclCommandContext_t ctx  = {
        1,                              // endpointId
        CHIP_ZCL_CLUSTER_ON_OFF,        // clusterId
        true,                           // clusterSpecific
        false,                          // mfgSpecific
        0,                              // mfgCode
        zclCommand,                     // commandId
        ZCL_DIRECTION_CLIENT_TO_SERVER, // direction
        0,                              // payloadStartIndex
        nullptr,                        // request
        nullptr                         // response
    };
    chipZclEncodeZclHeader(zcl_buffer, &ctx);

#ifdef DEBUG
    const size_t data_len = chipZclBufferDataLength(zcl_buffer);

    fprintf(stderr, "SENDING: %zu ", data_len);
    for (size_t i = 0; i < data_len; ++i)
    {
        fprintf(stderr, "%d ", chipZclBufferPointer(zcl_buffer)[i]);
    }
    fprintf(stderr, "\n");
#endif

    controller->SendMessage(NULL, buffer);
    controller->ServiceEvents();
}

// ================================================================================
// Main Code
// ================================================================================
static const unsigned char local_private_key[] = { 0x00, 0xd1, 0x90, 0xd9, 0xb3, 0x95, 0x1c, 0x5f, 0xa4, 0xe7, 0x47,
                                                   0x92, 0x5b, 0x0a, 0xa9, 0xa7, 0xc1, 0x1c, 0xe7, 0x06, 0x10, 0xe2,
                                                   0xdd, 0x16, 0x41, 0x52, 0x55, 0xb7, 0xb8, 0x80, 0x8d, 0x87, 0xa1 };

static const unsigned char remote_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                                   0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                                   0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                                   0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                                   0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

int main(int argc, char * argv[])
{
    IPAddress host_addr;
    Command command;
    if (!DetermineAddress(argc, argv, &host_addr) || !DetermineCommand(argc, argv, &command))
    {
        ShowUsage(argv[0]);
        return -1;
    }

    auto * controller = new DeviceController::ChipDeviceController();
    controller->Init();

    controller->ConnectDevice(1, host_addr, NULL, EchoResponse, ReceiveError);
    controller->ManualKeyExchange(remote_public_key, sizeof(remote_public_key), local_private_key, sizeof(local_private_key));

    if (command == Command::Echo)
    {
        DoEcho(controller, host_addr);
    }
    else
    {
        DoOnOff(controller, command);
    }

    controller->Shutdown();
    delete controller;

    return 0;
}

extern "C" {
// We have to have this empty callback, because the ZCL code links against it.
void chipZclPostAttributeChangeCallback(uint8_t endpoint, ChipZclClusterId clusterId, ChipZclAttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{}
} // extern "C"
