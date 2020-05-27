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
            "  %s device-ip-address device-port\n",
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

// ================================================================================
// Main Code
// ================================================================================

int main(int argc, char * argv[])
{
    IPAddress host_addr;
    uint16_t port;
    if (!DetermineAddress(argc, argv, &host_addr, &port))
    {
        ShowUsage(argv[0]);
        return -1;
    }

    size_t payload_len = strlen(PAYLOAD);

    chip::System::PacketBuffer * buffer = chip::System::PacketBuffer::NewWithAvailableSize(payload_len);
    snprintf((char *) buffer->Start(), payload_len + 1, "%s", PAYLOAD);
    buffer->SetDataLength(payload_len);

    chip::DeviceController::ChipDeviceController * controller = new chip::DeviceController::ChipDeviceController();
    controller->Init();

    controller->ConnectDevice(1, host_addr, NULL, EchoResponse, ReceiveError, port);

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

    return 0;
}
