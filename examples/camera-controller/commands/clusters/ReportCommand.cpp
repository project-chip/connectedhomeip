/*
 *   Copyright (c) 2025 Project CHIP Authors
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

#include "ReportCommand.h"

#include <app/InteractionModelEngine.h>
#include <inttypes.h>

using namespace ::chip;

void ReportCommand::OnAttributeData(const app::ConcreteDataAttributePath & path, TLV::TLVReader * data,
                                    const app::StatusIB & status)
{
    CHIP_ERROR error = status.ToChipError();
    if (CHIP_NO_ERROR != error)
    {
        LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(path, status));

        ChipLogError(NotSpecified, "Response Failure: %s", ErrorStr(error));
        mError = error;
        return;
    }

    if (data == nullptr)
    {
        ChipLogError(NotSpecified, "Response Failure: No Data");
        mError = CHIP_ERROR_INTERNAL;
        return;
    }

    LogErrorOnFailure(RemoteDataModelLogger::LogAttributeAsJSON(path, data));
}

void ReportCommand::OnEventData(const app::EventHeader & eventHeader, TLV::TLVReader * data, const app::StatusIB * status)
{
    if (status != nullptr)
    {
        CHIP_ERROR error = status->ToChipError();
        if (CHIP_NO_ERROR != error)
        {
            LogErrorOnFailure(RemoteDataModelLogger::LogErrorAsJSON(eventHeader, *status));

            ChipLogError(NotSpecified, "Response Failure: %s", ErrorStr(error));
            mError = error;
            return;
        }
    }

    if (data == nullptr)
    {
        ChipLogError(NotSpecified, "Response Failure: No Data");
        mError = CHIP_ERROR_INTERNAL;
        return;
    }

    LogErrorOnFailure(RemoteDataModelLogger::LogEventAsJSON(eventHeader, data));
}
