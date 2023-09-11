/**
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "TargetNavigatorManager.h"

using namespace std;
using namespace chip::app;
using namespace chip::app::Clusters::TargetNavigator;

TargetNavigatorManager::TargetNavigatorManager(std::list<std::string> targets, uint8_t currentTarget)
{
    mTargets       = targets;
    mCurrentTarget = currentTarget;
}

CHIP_ERROR TargetNavigatorManager::HandleGetTargetList(AttributeValueEncoder & aEncoder)
{
    // NOTE: the ids for each target start at 1 so that we can reserve 0 as "no current target"
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        int i = 0;
        for (std::string & entry : mTargets)
        {
            Structs::TargetInfoStruct::Type outputInfo;
            outputInfo.identifier = static_cast<uint8_t>(i + 1);
            outputInfo.name       = CharSpan::fromCharString(entry.c_str());
            ReturnErrorOnFailure(encoder.Encode(outputInfo));
            i++;
        }
        return CHIP_NO_ERROR;
    });
}

uint8_t TargetNavigatorManager::HandleGetCurrentTarget()
{
    return mCurrentTarget;
}

void TargetNavigatorManager::HandleNavigateTarget(CommandResponseHelper<NavigateTargetResponseType> & helper,
                                                  const uint64_t & target, const CharSpan & data)
{
    NavigateTargetResponseType response;
    if (target == kNoCurrentTarget || target > mTargets.size())
    {
        response.data   = chip::MakeOptional(CharSpan::fromCharString("error"));
        response.status = TargetNavigatorStatusEnum::kTargetNotFound;
        helper.Success(response);
        return;
    }
    mCurrentTarget = static_cast<uint8_t>(target);

    response.data   = chip::MakeOptional(CharSpan::fromCharString("data response"));
    response.status = TargetNavigatorStatusEnum::kSuccess;
    helper.Success(response);
}
