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

#include <app/server/CommissioningWindowManager.h>
#include <transport/Session.h>

namespace chip {
namespace Test {
/**
 * @brief Class acts as an accessor to private methods of the CommissioningWindowManagerTestAccess class without needing to give
 * friend access to each individual test.
 */
class CommissioningWindowManagerTestAccess
{
public:
    CommissioningWindowManagerTestAccess() = delete;
    CommissioningWindowManagerTestAccess(CommissioningWindowManager * cmwinMgr) : mcmwinMgr(cmwinMgr) {}
    void SetPASESession(SessionHolderWithDelegate PASEsession)
    {
        auto optSession = PASEsession.Get();
        if (optSession.HasValue())
        {
            mcmwinMgr->mPASESession.ShiftToSession(optSession.Value());
        }
    }

private:
    CommissioningWindowManager * mcmwinMgr = nullptr;
};
} // namespace Test

} // namespace chip
