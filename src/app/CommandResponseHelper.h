/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <app/CommandHandler.h>
#include <app/ConcreteCommandPath.h>
#include <app/util/af.h>
#include <protocols/interaction_model/Constants.h>

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
    };

    CHIP_ERROR Success(ClusterStatus aClusterStatus)
    {
        CHIP_ERROR err = mCommandHandler->AddClusterSpecificSuccess(mCommandPath, aClusterStatus);
        if (err == CHIP_NO_ERROR)
        {
            mSentResponse = true;
        }
        return err;
    }

    CHIP_ERROR Failure(Protocols::InteractionModel::Status aStatus)
    {
        CHIP_ERROR err = mCommandHandler->FallibleAddStatus(mCommandPath, aStatus);
        if (err == CHIP_NO_ERROR)
        {
            mSentResponse = true;
        }
        return err;
    }

    CHIP_ERROR Failure(ClusterStatus aClusterStatus)
    {
        CHIP_ERROR err = mCommandHandler->AddClusterSpecificFailure(mCommandPath, aClusterStatus);
        if (err == CHIP_NO_ERROR)
        {
            mSentResponse = true;
        }
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
