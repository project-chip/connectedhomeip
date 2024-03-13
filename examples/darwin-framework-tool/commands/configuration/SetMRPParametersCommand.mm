/*
 *   Copyright (c) 2024 Project CHIP Authors
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
 */

#include "SetMRPParametersCommand.h"

using namespace chip;

namespace {

template <typename T>
NSNumber * _Nullable AsNumber(const Optional<T> & value)
{
    if (!value.HasValue()) {
        return nil;
    }

    return @(value.Value());
}

} // anonymous namespace

CHIP_ERROR SetMRPParametersCommand::RunCommand()
{
    if (!mIdleRetransmitMs.HasValue() && !mActiveRetransmitMs.HasValue() && !mActiveThresholdMs.HasValue() && !mAdditionalRetransmitDelayMs.HasValue()) {
        ChipLogError(chipTool, "set-mrp-parameters needs to have at least one argument provided");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    MTRSetMessageReliabilityParameters(AsNumber(mIdleRetransmitMs),
        AsNumber(mActiveRetransmitMs),
        AsNumber(mActiveThresholdMs),
        AsNumber(mAdditionalRetransmitDelayMs));
    return CHIP_NO_ERROR;
}
