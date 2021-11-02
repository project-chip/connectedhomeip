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
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/MessageDef/AttributeDataElement.h>
#include <app/data-model/Decode.h>
#include <app/data-model/List.h> // So we can encode lists
#include <app/data-model/TagBoundEncoder.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPTLV.h>
#include <lib/core/Optional.h>

namespace chip {
namespace app {

/*
 * This interface permits applications to register a server-side command handler
 * at run-time for a given cluster. The handler can either be configured to handle all endpoints
 * for the given cluster or only handle a specific endpoint.
 *
 * If a command is not handled through this interface, it will default to invoking the generated DispatchSingleClusterCommand
 * instead.
 *
 */
class CommandHandlerInterface
{
public:
    struct HandlerContext
    {
    public:
        HandlerContext(CommandHandler & commandHandler, const ConcreteCommandPath & requestPath, TLV::TLVReader & aReader) :
            mCommandHandler(commandHandler), mRequestPath(requestPath), mPayload(aReader)
        {}

        void SetCommandHandled() { mCommandHandled = true; }

        CommandHandler & mCommandHandler;
        const ConcreteCommandPath & mRequestPath;
        TLV::TLVReader & mPayload;
        bool mCommandHandled = false;
    };

    /**
     * aEndpointId can be Missing to indicate that this object is meant to be
     * used with all endpoints.
     */
    CommandHandlerInterface(Optional<EndpointId> aEndpointId, ClusterId aClusterId) :
        mEndpointId(aEndpointId), mClusterId(aClusterId)
    {}

    virtual ~CommandHandlerInterface() {}

    /**
     * Callback that must be implemented to handle an invoke request.
     *
     * @param [in] handlerContext Context that encapsulates the current invoke request.
     *                            Handlers are responsible for correctly calling SetCommandHandled()
     *                            on the context if they did handle the command.
     *
     *                            This is not necessary if the HandleCommand() method below is invoked.
     */
    virtual void InvokeCommand(HandlerContext & handlerContext) = 0;

    /**
     * Mechanism for keeping track of a chain of CommandHandlerInterface.
     */
    void SetNext(CommandHandlerInterface * aNext) { mNext = aNext; }
    CommandHandlerInterface * GetNext() const { return mNext; }

    /**
     * Check whether a this CommandHandlerInterface is relevant for a
     * particular endpoint+cluster.  An CommandHandlerInterface will be used
     * for an invoke from a particular cluster only when this function returns
     * true.
     */
    bool Matches(EndpointId aEndpointId, ClusterId aClusterId) const
    {
        return (!mEndpointId.HasValue() || mEndpointId.Value() == aEndpointId) && mClusterId == aClusterId;
    }

    /**
     * Check whether an CommandHandlerInterface is relevant for a particular
     * specific endpoint.  This is used to clean up overrides registered for an
     * endpoint that becomes disabled.
     */
    bool MatchesExactly(EndpointId aEndpointId) const { return mEndpointId.HasValue() && mEndpointId.Value() == aEndpointId; }

    /**
     * Check whether another CommandHandlerInterface wants to handle the same set of
     * attributes as we do.
     */
    bool Matches(const CommandHandlerInterface & aOther) const
    {
        return mClusterId == aOther.mClusterId &&
            (!mEndpointId.HasValue() || !aOther.mEndpointId.HasValue() || mEndpointId.Value() == aOther.mEndpointId.Value());
    }

protected:
    /*
     * Helper function to automatically de-serialize the data payload into a cluster object
     * of type RequestT if the Cluster ID and Command ID in the context match. Upon successful
     * de-serialization, the provided lambda is invoked and passed in a reference to the cluster object.
     *
     * Provided lambda is expected to have the following signature:
     *  CHIP_ERROR Func(HandlerContext &handlerContext, RequestT &requestPayload);
     */
    template <typename RequestT, typename FuncT>
    CHIP_ERROR HandleCommand(HandlerContext & handlerContext, FuncT func)
    {
        //
        // If already handled, no more work to be done.
        //
        if (handlerContext.mCommandHandled)
        {
            return CHIP_NO_ERROR;
        }

        if (handlerContext.mRequestPath.mClusterId == RequestT::GetClusterId() &&
            handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId())
        {
            RequestT requestPayload;

            ReturnErrorOnFailure(DataModel::Decode(handlerContext.mPayload, requestPayload));
            ReturnErrorOnFailure(func(handlerContext, requestPayload));

            handlerContext.SetCommandHandled();
        }

        return CHIP_NO_ERROR;
    }

private:
    Optional<EndpointId> mEndpointId;
    ClusterId mClusterId;
    CommandHandlerInterface * mNext = nullptr;
};

} // namespace app
} // namespace chip
