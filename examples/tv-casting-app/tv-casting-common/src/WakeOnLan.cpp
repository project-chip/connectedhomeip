/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "WakeOnLan.h"

constexpr int kBroadcastOption    = 1;
constexpr int kWoLMagicPacketSize = 102;

CHIP_ERROR SendWakeOnLanPacket(chip::CharSpan * MACAddress)
{
    ChipLogProgress(AppServer, "SendWakeOnLanPacket called");
    VerifyOrReturnError(MACAddress != nullptr && MACAddress->size() > 0, CHIP_ERROR_INVALID_ARGUMENT);
    const int kMACLength = chip::DeviceLayer::ConfigurationManager::kPrimaryMACAddressLength;

    // Create a socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0)
    {
        ChipLogError(AppServer, "socket(): Could not create socket");
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Enable broadcast option
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &kBroadcastOption, sizeof(kBroadcastOption)) < 0)
    {
        ChipLogError(AppServer, "setsockopt(): Could not enable broadcast option on socket");
        close(sockfd);
        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Convert MAC Address to bytes
    uint8_t MACBytes[kMACLength];
    for (int i = 0; i < 2 * kMACLength; i += 2)
    {
        char byteString[3];
        byteString[0]   = MACAddress->data()[i];
        byteString[1]   = MACAddress->data()[i + 1];
        byteString[2]   = '\0';
        MACBytes[i / 2] = static_cast<uint8_t>(std::strtol(byteString, nullptr, 16));
    }

    // Create the Wake On LAN "magic" packet
    char magicPacket[kWoLMagicPacketSize];
    std::memset(magicPacket, 0xFF, kMACLength);
    for (int i = kMACLength; i < kWoLMagicPacketSize; i += kMACLength)
    {
        std::memcpy(magicPacket + i, MACBytes, kMACLength);
    }

    // Set up the broadcast address
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(9);
    addr.sin_addr.s_addr = INADDR_BROADCAST;

    // Send the Wake On LAN packet
    ssize_t bytesSent = sendto(sockfd, magicPacket, kWoLMagicPacketSize, 0, (struct sockaddr *) &addr, sizeof(addr));
    if (bytesSent < 0)
    {
        ChipLogError(AppServer, "sendto(): Could not send WoL magic packet");
        close(sockfd);
        return CHIP_ERROR_INCORRECT_STATE;
    }
    ChipLogProgress(AppServer, "Broadcasted WoL magic packet with MACAddress %.*s", 2 * kMACLength, MACAddress->data());

    close(sockfd);
    return CHIP_NO_ERROR;
}
