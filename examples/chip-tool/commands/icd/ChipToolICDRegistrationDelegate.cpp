/*
 *   Copyright (c) 2023 Project CHIP Authors
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

#include "ChipToolICDRegistrationDelegate.h"

#include <crypto/CHIPCryptoPAL.h>
#include <lib/support/BytesToHex.h>

#include <cstdint>

void ChipToolICDRegistrationDelegate::SetCheckInNodeId(chip::NodeId checkInNodeId)
{
    mCheckInNodeId = checkInNodeId;
}

void ChipToolICDRegistrationDelegate::GenerateSymmetricKey(
    chip::NodeId device, chip::Callback::Callback<OnSymmetricKeyGenerationCompleted> * onCompletion)
{
    chip::DeviceLayer::SystemLayer().ScheduleLambda([this, device, onCompletion] {
        // TODO: We need to persist the generated key.
        uint32_t mockMonitoredSubject = 1;
        uint8_t mockIcdKey[kKeySize];
        char mockIcdKeyHex[kKeySize * 2 + 1];
        chip::Crypto::DRBG_get_bytes(mockIcdKey, sizeof(mockIcdKey));
        onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, mCheckInNodeId, mockMonitoredSubject, ICDKey(mockIcdKey));
        chip::Encoding::BytesToHex(mockIcdKey, sizeof(mockIcdKey), mockIcdKeyHex, sizeof(mockIcdKeyHex),
                                   chip::Encoding::HexFlags::kNullTerminate);
        ChipLogProgress(chipTool, "Generated Symmetric Key for Node ID " ChipLogFormatX64 ": %s", ChipLogValueX64(device),
                        mockIcdKeyHex);
    });
}

void ChipToolICDRegistrationDelegate::ICDRegistrationComplete(chip::NodeId device, uint32_t icdCounter)
{
    // TODO: We may want to trigger real subscription in this function.
    ChipLogProgress(chipTool, "Node ID " ChipLogFormatX64 " completes ICD Registration with start ICDCounter = %" PRIu32,
                    ChipLogValueX64(device), icdCounter);
}
