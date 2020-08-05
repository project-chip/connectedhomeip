/*
 *   Copyright (c) 2020 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

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
#include <platform/ConnectivityManager.h>
#include <support/CHIPLogging.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>

#include <controller/CHIPDeviceController.h>

extern "C" {
#include "chip-zcl/chip-zcl.h"
} // extern "C"

#include "chip-zcl/chip-zcl-zpro-codec.h"

// Delay, in seconds, between sends for the echo case.
#define SEND_DELAY 5

using namespace ::chip;
using namespace ::chip::Inet;

// NOTE: Remote device ID is in sync with the echo server device id
//       At some point, we may want to add an option to connect to a device without
//       knowing its id, because the ID can be learned on the first response that is received.
constexpr NodeId kLocalDeviceId  = 112233;
constexpr NodeId kRemoteDeviceId = 12344321;

static const unsigned char local_private_key[] = { 0x00, 0xd1, 0x90, 0xd9, 0xb3, 0x95, 0x1c, 0x5f, 0xa4, 0xe7, 0x47,
                                                   0x92, 0x5b, 0x0a, 0xa9, 0xa7, 0xc1, 0x1c, 0xe7, 0x06, 0x10, 0xe2,
                                                   0xdd, 0x16, 0x41, 0x52, 0x55, 0xb7, 0xb8, 0x80, 0x8d, 0x87, 0xa1 };

static const unsigned char remote_public_key[] = { 0x04, 0xe2, 0x07, 0x64, 0xff, 0x6f, 0x6a, 0x91, 0xd9, 0xc2, 0xc3, 0x0a, 0xc4,
                                                   0x3c, 0x56, 0x4b, 0x42, 0x8a, 0xf3, 0xb4, 0x49, 0x29, 0x39, 0x95, 0xa2, 0xf7,
                                                   0x02, 0x8c, 0xa5, 0xce, 0xf3, 0xc9, 0xca, 0x24, 0xc5, 0xd4, 0x5c, 0x60, 0x79,
                                                   0x48, 0x30, 0x3c, 0x53, 0x86, 0xd9, 0x23, 0xe6, 0x61, 0x1f, 0x5a, 0x3d, 0xdf,
                                                   0x9f, 0xdc, 0x35, 0xea, 0xd0, 0xde, 0x16, 0x7e, 0x64, 0xde, 0x7f, 0x3c, 0xa6 };

static const char * PAYLOAD    = "Message from Standalone CHIP echo client!";
bool isDeviceConnected         = false;
static bool waitingForResponse = true;

// Device Manager Callbacks
static void OnConnect(DeviceController::ChipDeviceController * controller, Transport::PeerConnectionState * state,
                      void * appReqState)
{
    isDeviceConnected = true;

    if (state != NULL)
    {
        CHIP_ERROR err = controller->ManualKeyExchange(state, remote_public_key, sizeof(remote_public_key), local_private_key,
                                                       sizeof(local_private_key));

        if (err != CHIP_NO_ERROR)
        {
            fprintf(stderr, "Failed to exchange keys\n");
        }
    }
}

static bool ContentMayBeADataModelMessage(System::PacketBuffer * buffer)
{
    // A data model message has a first byte whose value is always one of  0x00,
    // 0x01, 0x02, 0x03.
    return buffer->DataLength() > 0 && buffer->Start()[0] < 0x04;
}

// This function consumes (i.e. frees) the buffer.
static void HandleDataModelMessage(System::PacketBuffer * buffer)
{
    EmberApsFrame frame;
    bool ok = extractApsFrame(buffer->Start(), buffer->DataLength(), &frame);
    if (ok)
    {
        printf("APS frame processing success!\n");
    }
    else
    {
        printf("APS frame processing failure!\n");
        System::PacketBuffer::Free(buffer);
        return;
    }

    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);

    VerifyOrExit(messageLen == 5, printf("Unexpected response length: %d\n", messageLen));
    // Bit 3 of the frame control byte set means direction is server to client.
    // We expect no other bits to be set.
    VerifyOrExit(message[0] == 8, printf("Unexpected frame control byte: 0x%02x\n", message[0]));
    VerifyOrExit(message[1] == 1, printf("Unexpected sequence number: %d\n", message[1]));
    VerifyOrExit(message[2] == 0x0b, printf("Unexpected command 0x%02x; expected Default Response", message[2]));
    printf("Got default response to command '0x%02x' for cluster '0x%02x'.  Status is '0x%02x'.\n", message[3], frame.clusterId,
           message[4]);
exit:
    System::PacketBuffer::Free(buffer);
}

static void OnMessage(DeviceController::ChipDeviceController * deviceController, void * appReqState, System::PacketBuffer * buffer)
{
    size_t data_len    = buffer->DataLength();
    waitingForResponse = false;

    printf("Message received: %zu bytes\n", data_len);

    if (ContentMayBeADataModelMessage(buffer))
    {
        HandleDataModelMessage(buffer);
        return;
    }

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

static void OnError(DeviceController::ChipDeviceController * deviceController, void * appReqState, CHIP_ERROR error,
                    const IPPacketInfo * pi)
{
    waitingForResponse = false;
    printf("ERROR: %s\n Got error\n", ErrorStr(error));
}

void ShowUsage(const char * executable)
{
    fprintf(stderr,
            "Usage: \n"
            "  %s command [params]\n"
            "  Supported commands and their parameters:\n"
            "    echo-ble discriminator setupPINCode\n"
            "    echo device-ip-address device-port\n"
            "    off device-ip-address device-port endpoint-id\n"
            "    on device-ip-address device-port endpoint-id\n"
            "    toggle device-ip-address device-port endpoint-id\n",
            executable);
}

enum class Command
{
    Off,
    On,
    Toggle,
    Echo,
    EchoBle,
};

template <int N>
bool EqualsLiteral(const char * str, const char (&literal)[N])
{
    return strncmp(str, literal, N) == 0;
}

bool DetermineCommand(int argc, char * argv[], Command * command)
{
    if (argc <= 1)
    {
        return false;
    }

    if (EqualsLiteral(argv[1], "off"))
    {
        *command = Command::Off;
        return argc == 5;
    }

    if (EqualsLiteral(argv[1], "on"))
    {
        *command = Command::On;
        return argc == 5;
    }

    if (EqualsLiteral(argv[1], "toggle"))
    {
        *command = Command::Toggle;
        return argc == 5;
    }

    if (EqualsLiteral(argv[1], "echo"))
    {
        *command = Command::Echo;
        return argc == 4;
    }

    if (EqualsLiteral(argv[1], "echo-ble"))
    {
        *command = Command::EchoBle;
        return argc == 4;
    }

    fprintf(stderr, "Unknown command: %s\n", argv[3]);
    return false;
}

struct CommandArgs
{
    IPAddress hostAddr;
    uint16_t port;
    uint16_t discriminator;
    uint32_t setupPINCode;
    uint8_t endpointId;
};

bool DetermineArgsBle(char * argv[], CommandArgs * commandArgs)
{
    std::string discriminator_str(argv[2]);
    commandArgs->discriminator = std::stoi(discriminator_str);

    std::string setup_pin_code_str(argv[3]);
    commandArgs->setupPINCode = std::stoi(setup_pin_code_str);
    return true;
}

bool DetermineArgsEcho(char * argv[], CommandArgs * commandArgs)
{
    if (!IPAddress::FromString(argv[2], commandArgs->hostAddr))
    {
        fputs("Error: Invalid device IP address", stderr);
        return false;
    }

    std::string port_str(argv[3]);
    std::stringstream ss(port_str);
    ss >> commandArgs->port;
    if (ss.fail() || !ss.eof())
    {
        fputs("Error: Invalid device port", stderr);
        return false;
    }

    return true;
}

bool DetermineArgsOnOff(char * argv[], CommandArgs * commandArgs)
{
    if (!DetermineArgsEcho(argv, commandArgs))
    {
        return false;
    }

    std::string endpoint_str(argv[4]);
    std::stringstream ss(endpoint_str);
    // stringstream treats uint8_t as char, which is not what we want here.
    uint16_t endpoint;
    ss >> endpoint;
    if (ss.fail() || !ss.eof() || endpoint < CHIP_ZCL_ENDPOINT_MIN || endpoint > CHIP_ZCL_ENDPOINT_MAX)
    {
        fprintf(stderr, "Error: Invalid endpoint id '%s'\n", argv[4]);
        return false;
    }
    commandArgs->endpointId = endpoint;

    return true;
}

bool DetermineCommandArgs(char * argv[], Command command, CommandArgs * commandArgs)
{
    switch (command)
    {
    case Command::EchoBle:
        return DetermineArgsBle(argv, commandArgs);

    case Command::Echo:
        return DetermineArgsEcho(argv, commandArgs);

    case Command::On:
    case Command::Off:
    case Command::Toggle:
        return DetermineArgsOnOff(argv, commandArgs);
    }

    fprintf(stderr, "Need to define arg handling for command '%d'\n", int(command));
    return false;
}

// Handle the echo case, where we just send a string and expect to get it back.
void DoEcho(DeviceController::ChipDeviceController * controller, const char * identifier)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    size_t payload_len = strlen(PAYLOAD);

    // Run the client
    while (1)
    {
        if (isDeviceConnected)
        {
            // Reallocate buffer on each run, as the secure transport encrypts and
            // overwrites the buffer from previous iteration.
            auto * buffer = System::PacketBuffer::NewWithAvailableSize(payload_len);
            memcpy(buffer->Start(), PAYLOAD, payload_len);
            buffer->SetDataLength(payload_len);

            err = controller->SendMessage(NULL, buffer);
            printf("Msg sent to server %s\n", err != CHIP_NO_ERROR ? ErrorStr(err) : identifier);
        }

        sleep(SEND_DELAY);
    }
}

void DoEchoBle(DeviceController::ChipDeviceController * controller, const uint16_t discriminator)
{
    char name[4];
    snprintf(name, sizeof(name), "%u", discriminator);
    DoEcho(controller, "");
}

void DoEchoIP(DeviceController::ChipDeviceController * controller, const IPAddress & hostAddr, uint16_t port)
{
    char name[46];
    char hostIpStr[40];
    hostAddr.ToString(hostIpStr, sizeof(hostIpStr));
    snprintf(name, sizeof(name), "%s:%d", hostIpStr, port);

    DoEcho(controller, name);
}

// Handle the on/off/toggle case, where we are sending a ZCL command and not
// expecting a response at all.
void DoOnOff(DeviceController::ChipDeviceController * controller, Command command, uint8_t endpoint)
{
    // Make sure our buffer is big enough, but this will need a better setup!
    static const size_t bufferSize = 1024;
    auto * buffer                  = System::PacketBuffer::NewWithAvailableSize(bufferSize);

    uint16_t dataLength = 0;
    switch (command)
    {
    case Command::Off:
        dataLength = encodeOffCommand(buffer->Start(), bufferSize, endpoint);
        break;
    case Command::On:
        dataLength = encodeOnCommand(buffer->Start(), bufferSize, endpoint);
        break;
    case Command::Toggle:
        dataLength = encodeToggleCommand(buffer->Start(), bufferSize, endpoint);
        break;
    default:
        fprintf(stderr, "Unknown command: %d\n", int(command));
        return;
    }
    buffer->SetDataLength(dataLength);

#ifdef DEBUG
    const size_t data_len = buffer->DataLength();

    fprintf(stderr, "SENDING: %zu ", data_len);
    for (size_t i = 0; i < data_len; ++i)
    {
        fprintf(stderr, "%d ", buffer->Start()[i]);
    }
    fprintf(stderr, "\n");
#endif

    controller->SendMessage(NULL, buffer);
    // FIXME: waitingForResponse is being written on other threads, presumably.
    // We probably need some more synchronization here.
    while (waitingForResponse)
    {
        // Just poll for the response.
        sleep(0);
    }
}

CHIP_ERROR ExecuteCommand(DeviceController::ChipDeviceController * controller, Command command, CommandArgs & commandArgs)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (command)
    {
    case Command::EchoBle:
        err = controller->ConnectDevice(kRemoteDeviceId, commandArgs.discriminator, commandArgs.setupPINCode, NULL, OnConnect,
                                        OnMessage, OnError);
        VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to connect to the device"));
        DoEchoBle(controller, commandArgs.discriminator);
        break;

    case Command::Echo:
        err =
            controller->ConnectDevice(kRemoteDeviceId, commandArgs.hostAddr, NULL, OnConnect, OnMessage, OnError, commandArgs.port);
        VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to connect to the device"));
        DoEchoIP(controller, commandArgs.hostAddr, commandArgs.port);
        break;

    default:
        err =
            controller->ConnectDevice(kRemoteDeviceId, commandArgs.hostAddr, NULL, OnConnect, OnMessage, OnError, commandArgs.port);
        VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to connect to the device"));
        DoOnOff(controller, command, commandArgs.endpointId);
        controller->ServiceEventSignal();
        break;
    }

exit:
    return err;
}

// ================================================================================
// Main Code
// ================================================================================
int main(int argc, char * argv[])
{
    Command command;
    CommandArgs commandArgs;

    if (!DetermineCommand(argc, argv, &command) || !DetermineCommandArgs(argv, command, &commandArgs))
    {
        ShowUsage(argv[0]);
        return EXIT_FAILURE;
    }

    auto * controller = new DeviceController::ChipDeviceController();
    CHIP_ERROR err    = controller->Init(kLocalDeviceId);
    VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to initialize the device controller"));

    err = controller->ServiceEvents();
    VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to initialize the run loop"));

    err = ExecuteCommand(controller, command, commandArgs);
    VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to send command"));
exit:
    if (err != CHIP_NO_ERROR)
    {
        fprintf(stderr, "ERROR: %s\n", ErrorStr(err));
    }
    controller->Shutdown();
    delete controller;

    return (err == CHIP_NO_ERROR) ? EXIT_SUCCESS : EXIT_FAILURE;
}
