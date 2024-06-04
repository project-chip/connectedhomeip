/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#pragma once

#include <app/reporting/SynchronizedReportSchedulerImpl.h>

namespace chip {
namespace Test {

/**
 * @brief Class acts as an accessor to private methods of the SynchronizedReportSchedulerImpl class without needing to give
 * friend access to each individual test. This is not a Global API and should only be used for (Unit) Testing.
 */
class SynchronizedReportSchedulerImplTestAccess
{

public:
    SynchronizedReportSchedulerImplTestAccess(app::reporting::SynchronizedReportSchedulerImpl * apSynchronizedReportSchedulerImpl) :
        mpSynchronizedReportSchedulerImpl(apSynchronizedReportSchedulerImpl)
    {}

    System::Clock::Timestamp & GetNextMinTimestamp() { return mpSynchronizedReportSchedulerImpl->mNextMinTimestamp; }
    System::Clock::Timestamp & GetNextMaxTimestamp() { return mpSynchronizedReportSchedulerImpl->mNextMaxTimestamp; }
    System::Clock::Timestamp & GetNextReportTimestamp() { return mpSynchronizedReportSchedulerImpl->mNextReportTimestamp; }

    void UnregisterAllHandlers() { mpSynchronizedReportSchedulerImpl->UnregisterAllHandlers(); }

    app::reporting::ReadHandlerNode * FindReadHandlerNode(const app::ReadHandler * aReadHandler)
    {
        return mpSynchronizedReportSchedulerImpl->FindReadHandlerNode(aReadHandler);
    }

private:
    app::reporting::SynchronizedReportSchedulerImpl * mpSynchronizedReportSchedulerImpl = nullptr;
};

} // namespace Test
} // namespace chip
