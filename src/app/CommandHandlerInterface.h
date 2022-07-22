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
#include <app/ConcreteClusterPath.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/Decode.h>
#include <app/data-model/List.h> // So we can encode lists
#include <lib/core/DataModelTypes.h>
#include <lib/support/Iterators.h>

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
        void SetCommandNotHandled() { mCommandHandled = false; }

        /*
         * Returns a TLVReader positioned at the TLV struct that contains the payload of the command.
         *
         * If the reader is requested from the context, then we can assume there is an intention
         * to access the payload of this command and consequently, to handle this command.
         *
         * If this is not true, the application should call SetCommandNotHandled().
         *
         */
        TLV::TLVReader & GetReader()
        {
            SetCommandHandled();
            return mPayload;
        }

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
     * The callee is required to handle *all* errors that may occur during the handling of this command,
     * including errors like those encountered during decode and encode of the payloads as
     * well as application-specific errors. As part of handling the error, the callee is required
     * to handle generation of an appropriate status response.
     *
     * The only exception to this rule is if the HandleCommand helper method is used below - it will
     * help handle some of these cases (see below).
     *
     * @param [in] handlerContext Context that encapsulates the current invoke request.
     *                            Handlers are responsible for correctly calling SetCommandHandled()
     *                            on the context if they did handle the command.
     *
     *                            This is not necessary if the HandleCommand() method below is invoked.
     */
    virtual void InvokeCommand(HandlerContext & handlerContext) = 0;

    typedef Loop (*CommandIdCallback)(CommandId id, void * context);

    /**
     * Function that may be implemented to enumerate accepted (client-to-server)
     * commands for the given cluster.
     *
     * If this function returns CHIP_ERROR_NOT_IMPLEMENTED, the list of accepted
     * commands will come from the endpoint metadata for the cluster.
     *
     * If this function returns any other error, that will be treated as an
     * error condition by the caller, and handling will depend on the caller.
     *
     * Otherwise the list of accepted commands will be the list of values passed
     * to the provided callback.
     *
     * The implementation _must_ pass the provided context to the callback.
     *
     * If the callback returns Loop::Break, there must be no more calls to it.
     * This is used by callbacks that just look for a particular value in the
     * list.
     */
    virtual CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    /**
     * Function that may be implemented to enumerate generated (response)
     * commands for the given cluster.
     *
     * If this function returns CHIP_ERROR_NOT_IMPLEMENTED, the list of
     * generated commands will come from the endpoint metadata for the cluster.
     *
     * If this function returns any other error, that will be treated as an
     * error condition by the caller, and handling will depend on the caller.
     *
     * Otherwise the list of generated commands will be the list of values
     * passed to the provided callback.
     *
     * The implementation _must_ pass the provided context to the callback.
     *
     * If the callback returns Loop::Break, there must be no more calls to it.
     * This is used by callbacks that just look for a particular value in the
     * list.
     */
    virtual CHIP_ERROR EnumerateGeneratedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

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
    bool MatchesEndpoint(EndpointId aEndpointId) const { return mEndpointId.HasValue() && mEndpointId.Value() == aEndpointId; }

    /**
     * Check whether another CommandHandlerInterface wants to handle the same set of
     * commands as we do.
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
     * de-serialization, the provided function is invoked and passed in a reference to the cluster object.
     *
     * Any errors encountered in this function prior to calling func result in the automatic generation of a status response.
     * If `func` is called, the responsibility for doing so shifts to the callee to handle any further errors that are encountered.
     *
     * The provided function is expected to have the following signature:
     *  void Func(HandlerContext &handlerContext, const RequestT &requestPayload);
     */
    template <typename RequestT, typename FuncT>
    void HandleCommand(HandlerContext & handlerContext, FuncT func)
    {
        if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mClusterId == RequestT::GetClusterId()) &&
            (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
        {
            RequestT requestPayload;

            //
            // If the command matches what the caller is looking for, let's mark this as being handled
            // even if errors happen after this. This ensures that we don't execute any fall-back strategies
            // to handle this command since at this point, the caller is taking responsibility for handling
            // the command in its entirety, warts and all.
            //
            handlerContext.SetCommandHandled();

            if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
            {
                handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                         Protocols::InteractionModel::Status::InvalidCommand);
                return;
            }

            func(handlerContext, requestPayload);
        }
    }

private:
    Optional<EndpointId> mEndpointId;
    ClusterId mClusterId;
    CommandHandlerInterface * mNext = nullptr;
};

} // namespace app
} // namespace chip
