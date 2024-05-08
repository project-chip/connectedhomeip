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

#include <app/reporting/Engine.h>

using namespace chip::app::reporting;

namespace chip {
namespace Test {

/**
 * @brief Class acts as an accessor to private methods of the Engine class without needing to give friend access to
 *        each individual test.
 *        This is not a Global API and should only be used for (Unit) Testing.
 */
class EngineTestAccess
{
public:
    EngineTestAccess(Engine * aEngine) : pEngine(aEngine) {}

    CHIP_ERROR BuildAndSendSingleReportData(chip::app::ReadHandler * apReadHandler)
    {
        return pEngine->BuildAndSendSingleReportData(apReadHandler);
    }

    Engine * GetEngine() { return pEngine; }

    ObjectPool<Engine::AttributePathParamsWithGeneration, CHIP_IM_SERVER_MAX_NUM_DIRTY_SET, ObjectPoolMem::kInline> &
    GetGlobalDirtySet()
    {
        return pEngine->mGlobalDirtySet;
    }

    bool MergeOverlappedAttributePath(const chip::app::AttributePathParams & aAttributePath)
    {
        return pEngine->MergeOverlappedAttributePath(aAttributePath);
    }

    inline void BumpDirtySetGeneration() { pEngine->BumpDirtySetGeneration(); }

    CHIP_ERROR InsertPathIntoDirtySet(const chip::app::AttributePathParams & aAttributePath)
    {
        return pEngine->InsertPathIntoDirtySet(aAttributePath);
    }

private:
    Engine * pEngine = nullptr;
};

} // namespace Test
} // namespace chip
