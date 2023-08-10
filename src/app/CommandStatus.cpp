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
#include "CommandStatus.h"

#include <lib/support/logging/CHIPLogging.h>

namespace chip {
namespace app {

using Protocols::InteractionModel::Status;

const CommandPathStatus & CommandPathStatus::LogIfFailure(const char * contextMessage) const
{
    if (mStatus == Status::Success)
    {
        return *this;
    }

    return LogStatus(contextMessage);
}

const CommandPathStatus & CommandPathStatus::LogStatus(const char * contextMessage) const
{
    if (contextMessage == nullptr)
    {
        contextMessage = "no additional context";
    }

    if (mStatus == Status::Success)
    {
        ChipLogProgress(DataManagement, "Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI " status SUCCESS (%s)",
                        mPath.mEndpointId, ChipLogValueMEI(mPath.mClusterId), ChipLogValueMEI(mPath.mCommandId), contextMessage);
    }
    else
    {
        ChipLogError(DataManagement,
                     "Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI " status " ChipLogFormatIMStatus " (%s)",
                     mPath.mEndpointId, ChipLogValueMEI(mPath.mClusterId), ChipLogValueMEI(mPath.mCommandId),
                     ChipLogValueIMStatus(mStatus), contextMessage);
    }

    return *this;
}

} // namespace app
} // namespace chip
