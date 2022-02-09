/**
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "TargetNavigatorManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::TargetNavigator;

CHIP_ERROR TargetNavigatorManager::HandleGetTargetList(AttributeValueEncoder & aEncoder)
{
    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    return aEncoder.EncodeList([](const auto & encoder) -> CHIP_ERROR {
        int maximumVectorSize = 2;
        for (int i = 0; i < maximumVectorSize; ++i)
        {
            Structs::TargetInfo::Type outputInfo;
            outputInfo.identifier = static_cast<uint8_t>(i + 1);
            outputInfo.name       = chip::CharSpan::fromCharString("exampleName");
            ReturnErrorOnFailure(encoder.Encode(outputInfo));
        }
        return CHIP_NO_ERROR;
    });
}

uint8_t TargetNavigatorManager::HandleGetCurrentTarget()
{
    return 0;
}

void TargetNavigatorManager::HandleNavigateTarget(CommandResponseHelper<NavigateTargetResponseType> & helper,
                                                  const uint64_t & target, const CharSpan & data)
{
    // TODO: Insert code here
    Commands::NavigateTargetResponse::Type response;
    response.data   = chip::Optional<CharSpan>(chip::CharSpan::fromCharString("data response"));
    response.status = chip::app::Clusters::TargetNavigator::StatusEnum::kSuccess;
    helper.Success(response);
}
