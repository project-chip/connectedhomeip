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
            "  %s device-ip-address device-port echo|off|on|toggle\n",
            executable);
}

bool DetermineAddress(int argc, char * argv[], IPAddress * hostAddr, uint16_t * port)
{
    if (argc < 3)
    {
        return false;
    }

    if (!IPAddress::FromString(argv[1], *hostAddr))
    {
        fputs("Error: Invalid device IP address", stderr);
        return false;
    }

    std::string port_str(argv[2]);
    std::stringstream ss(port_str);
    ss >> *port;
    if (ss.fail() || !ss.eof())
    {
        fputs("Error: Invalid device port", stderr);
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
    if (argc < 4)
    {
        return false;
    }

    if (EqualsLiteral(argv[3], "off"))
    {
        *command = Command::Off;
        return true;
    }

    if (EqualsLiteral(argv[3], "on"))
    {
        *command = Command::On;
        return true;
    }

    if (EqualsLiteral(argv[3], "toggle"))
    {
        *command = Command::Toggle;
        return true;
    }

    if (EqualsLiteral(argv[3], "echo"))
    {
        *command = Command::Echo;
        return true;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[3]);
    return false;
}

// Handle the echo case, where we just send a string and expect to get it back.
void DoEcho(DeviceController::ChipDeviceController * controller, const IPAddress & host_addr, uint16_t port)
{
    size_t payload_len = strlen(PAYLOAD);

    auto * buffer = System::PacketBuffer::NewWithAvailableSize(payload_len);
    snprintf((char *) buffer->Start(), payload_len + 1, "%s", PAYLOAD);
    buffer->SetDataLength(payload_len);

    // Run the client
    char host_ip_str[40];
    host_addr.ToString(host_ip_str, sizeof(host_ip_str));
    while (1)
    {
        // Send calls release on this buffer, so bump up the ref because we want to reuse it
        buffer->AddRef();
        controller->SendMessage(NULL, buffer);
        printf("Msg sent to server at %s:%d\n", host_ip_str, port);

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

    const size_t data_len = chipZclBufferDataLength(zcl_buffer);

#ifdef DEBUG
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

int main(int argc, char * argv[])
{
    IPAddress host_addr;
    uint16_t port;
    Command command;
    if (!DetermineAddress(argc, argv, &host_addr, &port) || !DetermineCommand(argc, argv, &command))
    {
        ShowUsage(argv[0]);
        return -1;
    }

    auto * controller = new DeviceController::ChipDeviceController();
    controller->Init();

    controller->ConnectDevice(1, host_addr, NULL, EchoResponse, ReceiveError, port);

    if (command == Command::Echo)
    {
        DoEcho(controller, host_addr, port);
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
