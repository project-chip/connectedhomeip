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
#include <chrono>
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

#include <app/chip-zcl-zpro-codec.h>

// Delay, in seconds, between sends for the echo case.
#define SEND_DELAY 5

// Limits on endpoint values.  Could be wrong, if we start using endpoint 0 for
// something.
#define CHIP_ZCL_ENDPOINT_MIN 0x01
#define CHIP_ZCL_ENDPOINT_MAX 0xF0

using namespace ::chip;
using namespace ::chip::Inet;

// NOTE: Remote device ID is in sync with the echo server device id
//       At some point, we may want to add an option to connect to a device without
//       knowing its id, because the ID can be learned on the first response that is received.
constexpr NodeId kLocalDeviceId  = 112233;
constexpr NodeId kRemoteDeviceId = 12344321;
constexpr std::chrono::seconds kWaitingForResponseTimeout(1);

static const char * PAYLOAD    = "Message from Standalone CHIP echo client!";
bool isDeviceConnected         = false;
static bool waitingForResponse = true;

// Device Manager Callbacks
static void OnConnect(DeviceController::ChipDeviceController * controller, Transport::PeerConnectionState * state,
                      void * appReqState)
{
    isDeviceConnected = true;
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
    if (extractApsFrame(buffer->Start(), buffer->DataLength(), &frame) == 0)
    {
        printf("APS frame processing failure!\n");
        System::PacketBuffer::Free(buffer);
        return;
    }

    printf("APS frame processing success!\n");
    uint8_t * message;
    uint16_t messageLen = extractMessage(buffer->Start(), buffer->DataLength(), &message);

    VerifyOrExit(messageLen >= 3, printf("Unexpected response length: %d\n", messageLen));
    // Bit 3 of the frame control byte set means direction is server to client.
    // We expect no other bits to be set.
    VerifyOrExit(message[0] == 8, printf("Unexpected frame control byte: 0x%02x\n", message[0]));
    VerifyOrExit(message[1] == 1, printf("Unexpected sequence number: %d\n", message[1]));

    // message[2] is the command id.
    switch (message[2])
    {
    case 0x0b: {
        // Default Response command.  Remaining bytes are the command id of the
        // command that's being responded to and a status code.
        VerifyOrExit(messageLen == 5, printf("Unexpected response length: %d\n", messageLen));
        printf("Got default response to command '0x%02x' for cluster '0x%02x'.  Status is '0x%02x'.\n", message[3], frame.clusterId,
               message[4]);
        break;
    }
    case 0x01: {
        // Read Attributes Response command.  Remaining bytes are a list of
        // (attr id, 0, attr type, attr value) or (attr id, failure status)
        // tuples.
        //
        // But for now we only support one attribute value, and that value is a
        // boolean.
        VerifyOrExit(messageLen >= 6, printf("Unexpected response length for Read Attributes command: %d\n", messageLen));
        uint16_t attr_id;
        memcpy(&attr_id, message + 3, sizeof(attr_id));
        if (message[5] == 0)
        {
            // FIXME: Should we have a mapping of type ids to types, based on
            // table 2.6.2.2 in Rev 8 of the ZCL spec?  0x10 is "Boolean".
            VerifyOrExit(messageLen == 8,
                         printf("Unexpected response length for successful Read Attributes command: %d\n", messageLen));
            printf("Read attribute '0x%04x' for cluster '0x%02x'.  Type is '0x%02x', value is '0x%02x'.\n", attr_id,
                   frame.clusterId, message[6], message[7]);
        }
        else
        {
            VerifyOrExit(messageLen == 6,
                         printf("Unexpected response length for failed Read Attributes command: %d\n", messageLen));
            printf("Reading attribute '0x%04x' for cluster '0x%02x' failed with status '0x%02x'.\n", attr_id, frame.clusterId,
                   message[5]);
        }
        break;
    }
    default: {
        printf("Unexpected command '0x%02x'.\n", message[2]);
        break;
    }
    }

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
            "    toggle device-ip-address device-port endpoint-id\n"
            "    read device-ip-address device-port endpoint-id attr-name\n"
            "  Supported attribute names for the 'read' command:\n"
            "    onoff -- OnOff attribute from the On/Off cluster\n",
            executable);
}

enum class Command
{
    Off,
    On,
    Toggle,
    Read,
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

    if (EqualsLiteral(argv[1], "read"))
    {
        *command = Command::Read;
        return argc == 6;
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

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return false;
}

struct CommandArgs
{
    IPAddress hostAddr;
    uint16_t port;
    uint16_t discriminator;
    uint32_t setupPINCode;
    uint8_t endpointId;
    // attrName is only used for Read commands.
    const char * attrName;
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
    case Command::Read: {
        if (!DetermineArgsOnOff(argv, commandArgs))
        {
            return false;
        }
        if (command == Command::Read)
        {
            commandArgs->attrName = argv[5];
        }
        return true;
    }
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
    char name[6];
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
void DoOnOff(DeviceController::ChipDeviceController * controller, Command command, const CommandArgs & commandArgs)
{
    const uint8_t endpoint = commandArgs.endpointId;

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
    case Command::Read:
        if (!EqualsLiteral(commandArgs.attrName, "onoff"))
        {
            fprintf(stderr, "Don't know how to read '%s' attribute\n", commandArgs.attrName);
            return;
        }
        dataLength = encodeReadOnOffCommand(buffer->Start(), bufferSize, endpoint);
        break;
    default:
        fprintf(stderr, "Unknown command: %d\n", int(command));
        return;
    }
    ChipLogProgress(Zcl, "Encoded data of length %d", dataLength);
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
    auto start = std::chrono::system_clock::now();
    while (waitingForResponse &&
           std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - start) < kWaitingForResponseTimeout)
    {
        // Just poll for the response.
        sleep(1);
    }

    if (waitingForResponse)
    {
        fprintf(stderr, "No response from device.");
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
        err = controller->ConnectDeviceWithoutSecurePairing(kRemoteDeviceId, commandArgs.hostAddr, NULL, OnConnect, OnMessage,
                                                            OnError, commandArgs.port);
        VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to connect to the device"));
        DoEchoIP(controller, commandArgs.hostAddr, commandArgs.port);
        break;

    default:
        err = controller->ConnectDeviceWithoutSecurePairing(kRemoteDeviceId, commandArgs.hostAddr, NULL, OnConnect, OnMessage,
                                                            OnError, commandArgs.port);
        VerifyOrExit(err == CHIP_NO_ERROR, fprintf(stderr, "Failed to connect to the device"));
        DoOnOff(controller, command, commandArgs);
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
