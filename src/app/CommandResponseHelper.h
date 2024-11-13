/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#pragma once

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <protocols/interaction_model/Constants.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

template <typename CommandData>
class CommandResponseHelper
{
public:
    CommandResponseHelper(app::CommandHandler * command, const app::ConcreteCommandPath & commandPath) :
        mCommandHandler(command), mCommandPath(commandPath), mSentResponse(false)
    {}

    CHIP_ERROR Success(const CommandData & aResponse)
    {
        mCommandHandler->AddResponse(mCommandPath, aResponse);
        mSentResponse = true;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR Success()
    {
        CHIP_ERROR err = mCommandHandler->FallibleAddStatus(mCommandPath, Protocols::InteractionModel::Status::Success);
        mSentResponse  = (err == CHIP_NO_ERROR);
        return err;
    }

    CHIP_ERROR Failure(Protocols::InteractionModel::Status aStatus)
    {
        CHIP_ERROR err = mCommandHandler->FallibleAddStatus(mCommandPath, aStatus);
        mSentResponse  = (err == CHIP_NO_ERROR);
        return err;
    }

    CHIP_ERROR Failure(ClusterStatus aClusterStatus)
    {
        CHIP_ERROR err = mCommandHandler->FallibleAddStatus(
            mCommandPath, Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(aClusterStatus));
        mSentResponse = (err == CHIP_NO_ERROR);
        return err;
    }

    bool HasSentResponse() const { return mSentResponse; }

private:
    app::CommandHandler * mCommandHandler;
    app::ConcreteCommandPath mCommandPath;
    bool mSentResponse;
};

} // namespace app
} // namespace chip
