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

#include <app/InteractionModelEngine.h>

namespace chip {
namespace Test {

using namespace chip::app;
/**
 * @brief Class acts as an accessor to private methods of the InteractionModelEngine class without needing to give friend access to
 *        each individual test.
 *        This is not a Global API and should only be used for (Unit) Testing.
 */
class InteractionModelEngineTestAccess
{

public:
    InteractionModelEngineTestAccess(InteractionModelEngine * apInteractionModelEngine) :
        mpInteractionModelEngine(apInteractionModelEngine)
    {}

    InteractionModelEngine * GetInstance() { return InteractionModelEngine::GetInstance(); }

    uint32_t ComputeTimeSecondsTillNextSubscriptionResumption()
    {
        return mpInteractionModelEngine->ComputeTimeSecondsTillNextSubscriptionResumption();
    }

    void SetNumSubscriptionResumptionRetries(uint32_t aNumSubscriptionResumptionRetries)
    {
        mpInteractionModelEngine->mNumSubscriptionResumptionRetries = aNumSubscriptionResumptionRetries;
    }
    uint32_t GetNumSubscriptionResumptionRetries() { return mpInteractionModelEngine->mNumSubscriptionResumptionRetries; }

    void SetNumOfSubscriptionsToResume(int8_t aNumOfSubscriptionsToResume)
    {
        mpInteractionModelEngine->mNumOfSubscriptionsToResume = aNumOfSubscriptionsToResume;
    }
    uint32_t GetNumOfSubscriptionsToResume() { return mpInteractionModelEngine->mNumOfSubscriptionsToResume; }

private:
    InteractionModelEngine * mpInteractionModelEngine = nullptr;
};

} // namespace Test
} // namespace chip
