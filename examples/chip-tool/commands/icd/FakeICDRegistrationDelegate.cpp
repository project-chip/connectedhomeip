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

#include "FakeICDRegistrationDelegate.h"

#include <cstdint>

void FakeIcdRegistrationDelegate::SetControllerNodeId(chip::NodeId controllerNodeId)
{
    mControllerNodeId = controllerNodeId;
}

void FakeIcdRegistrationDelegate::GenerateSymmetricKey(chip::NodeId device,
                                                       chip::Callback::Callback<OnSymmetricKeyGenerationCompleted> * onCompletion)
{
    ChipLogProgress(chipTool, "Generate Mock Symmetric Key for Node ID " ChipLogFormatX64, ChipLogValueX64(device));

    chip::DeviceLayer::SystemLayer().ScheduleLambda([this, onCompletion] {
        uint8_t mockIcdKey[]          = { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
        uint8_t mockVerificationKey[] = { 0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0 };
        onCompletion->mCall(onCompletion->mContext, CHIP_NO_ERROR, mControllerNodeId, 1, ICDKey(mockIcdKey),
                            chip::MakeOptional(ICDKey(mockVerificationKey)));
    });
}

void FakeIcdRegistrationDelegate::ReadyForSubscription(chip::NodeId device, uint32_t icdCounter)
{
    ChipLogProgress(chipTool, "Node ID " ChipLogFormatX64 " Is ready for subscription with ICD Counter %" PRIu32,
                    ChipLogValueX64(device), icdCounter);
}
