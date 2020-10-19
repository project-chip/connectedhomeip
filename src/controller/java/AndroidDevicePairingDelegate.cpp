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

#include "AndroidDevicePairingDelegate.h"

#include <transport/RendezvousSessionDelegate.h>

void AndroidDevicePairingDelegate::OnNetworkCredentialsRequested(chip::RendezvousDeviceCredentialsDelegate * callback)
{
    using namespace chip::DeviceLayer::Internal;

    // This is a dummy implementation of Thread provisioning which allows to test Rendezvous over BLE with
    // Thread-enabled devices by sending OpenThread Border Router default credentials.
    //
    // TODO:
    // 1. Figure out whether WiFi or Thread provisioning should be performed
    // 2. Call Java code to prompt a user for credentials or use the commissioner component of the app

    constexpr uint8_t XPAN_ID[kThreadExtendedPANIdLength]  = { 0x11, 0x11, 0x11, 0x11, 0x22, 0x22, 0x22, 0x22 };
    constexpr uint8_t MESH_PREFIX[kThreadMeshPrefixLength] = { 0xFD, 0x11, 0x11, 0x11, 0x11, 0x22, 0x00, 0x00 };
    constexpr uint8_t NETWORK_KEY[kThreadMasterKeyLength]  = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                              0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF };

    DeviceNetworkInfo threadData = {};
    memcpy(threadData.ThreadExtendedPANId, XPAN_ID, sizeof(XPAN_ID));
    memcpy(threadData.ThreadMeshPrefix, MESH_PREFIX, sizeof(MESH_PREFIX));
    memcpy(threadData.ThreadMasterKey, NETWORK_KEY, sizeof(NETWORK_KEY));
    threadData.ThreadPANId                      = 0x1234;
    threadData.ThreadChannel                    = 15;
    threadData.FieldPresent.ThreadExtendedPANId = 1;
    threadData.FieldPresent.ThreadMeshPrefix    = 1;

    callback->SendThreadCredentials(threadData);
}

void AndroidDevicePairingDelegate::OnOperationalCredentialsRequested(const char * csr, size_t csr_length,
                                                                     chip::RendezvousDeviceCredentialsDelegate * callback)
{}
