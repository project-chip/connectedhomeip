/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/CommandHandlerExchangeInterface.h>
#include <app/ConcreteCommandPath.h>
#include <app/data-model/EncodableToTLV.h>
#include <app/data-model/Encode.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/IntrusiveList.h>
#include <lib/support/logging/CHIPLogging.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {

/**
 *  A handler for incoming Invoke interactions.
 *
 *  Allows adding responses to be sent in an InvokeResponse: see the various
 *  "Add*" methods.
 *
 *  Allows adding the responses asynchronously when using `CommandHandler::Handle`
 *  (see documentation for `CommandHandler::Handle` for details)
 *
 *  Upgrading notes: this class has moved to an interface from a previous more complex
 *  implementation. If upgrading code between versions, please see docs/upgrading.md
 */
class CommandHandler
{
public:
    virtual ~CommandHandler() = default;

    /**
     * Class that allows asynchronous command processing before sending a
     * response.  When such processing is desired:
     *
     * 1) Create a Handle initialized with the CommandHandler that delivered the
     *    incoming command.
     * 2) Ensure the Handle, or some Handle it's moved into via the move
     *    constructor or move assignment operator, remains alive during the
     *    course of the asynchronous processing.
     * 3) Ensure that the ConcreteCommandPath involved will be known when
     *    sending the response.
     * 4) When ready to send the response:
     *    * Ensure that no other Matter tasks are running in parallel (e.g. by
     *      running on the Matter event loop or holding the Matter stack lock).
     *    * Call Get() to get the CommandHandler.
     *    * Check that Get() did not return null.
     *    * Add the response to the CommandHandler via one of the Add* methods.
     *    * Let the Handle get destroyed, or manually call Handle::Release() if
     *      destruction of the Handle is not desirable for some reason.
     *
     * The Invoke Response will not be sent until all outstanding Handles have
     * been destroyed or have had Release called.
     */
    class Handle : public IntrusiveListNodeBase<>
    {
    public:
        Handle() {}
        Handle(const Handle & handle) = delete;
        Handle(Handle && handle)
        {
            Init(handle.mpHandler);
            handle.Release();
        }
        Handle(decltype(nullptr)) {}
        Handle(CommandHandler * handler);
        ~Handle() { Release(); }

        Handle & operator=(Handle && handle)
        {
            Release();
            Init(handle.mpHandler);

            handle.Release();
            return *this;
        }

        Handle & operator=(decltype(nullptr))
        {
            Release();
            return *this;
        }

        /**
         * Get the CommandHandler object it holds. Get() may return a nullptr if the CommandHandler object it holds is no longer
         * valid.
         */
        CommandHandler * Get();

        void Release();

        void Invalidate() { mpHandler = nullptr; }

    private:
        void Init(CommandHandler * handler);

        CommandHandler * mpHandler = nullptr;
    };

    /**
     * Adds the given command status and returns any failures in adding statuses (e.g. out
     * of buffer space) to the caller. `context` is an optional (if not nullptr)
     * debug string to include in logging.
     */
    virtual CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath,
                                         const Protocols::InteractionModel::ClusterStatusCode & aStatus,
                                         const char * context = nullptr) = 0;
    CHIP_ERROR FallibleAddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::Status aStatus,
                                 const char * context = nullptr)
    {
        return FallibleAddStatus(aRequestCommandPath, Protocols::InteractionModel::ClusterStatusCode{ aStatus }, context);
    }

    /**
     * Adds an IM global or Cluster status when the caller is unable to handle any failures. Logging is performed
     * and failure to register the status is checked with VerifyOrDie. `context` is an optional (if not nullptr)
     * debug string to include in logging.
     */
    virtual void AddStatus(const ConcreteCommandPath & aRequestCommandPath,
                           const Protocols::InteractionModel::ClusterStatusCode & aStatus, const char * context = nullptr) = 0;
    void AddStatus(const ConcreteCommandPath & aRequestCommandPath, const Protocols::InteractionModel::Status aStatus,
                   const char * context = nullptr)
    {
        AddStatus(aRequestCommandPath, Protocols::InteractionModel::ClusterStatusCode{ aStatus }, context);
    }

    /**
     * Sets the response to indicate Success with a cluster-specific status code `aClusterStatus` included.
     *
     * NOTE: For regular success, what you want is AddStatus/FailibleAddStatus(aRequestCommandPath,
     * InteractionModel::Status::Success).
     */
    virtual CHIP_ERROR AddClusterSpecificSuccess(const ConcreteCommandPath & aRequestCommandPath, ClusterStatus aClusterStatus)
    {
        return FallibleAddStatus(aRequestCommandPath,
                                 Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificSuccess(aClusterStatus));
    }

    /**
     * Sets the response to indicate Failure with a cluster-specific status code `aClusterStatus` included.
     */
    virtual CHIP_ERROR AddClusterSpecificFailure(const ConcreteCommandPath & aRequestCommandPath, ClusterStatus aClusterStatus)
    {
        return FallibleAddStatus(aRequestCommandPath,
                                 Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(aClusterStatus));
    }

    /**
     * GetAccessingFabricIndex() may only be called during synchronous command
     * processing.  Anything that runs async (while holding a
     * CommandHandler::Handle or equivalent) must not call this method, because
     * it will not work right if the session we're using was evicted.
     */
    virtual FabricIndex GetAccessingFabricIndex() const = 0;

    /**
     * API for adding a data response.  The `aEncodable` is generally expected to encode
     * a ClusterName::Commands::CommandName::Type struct, however any object should work.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     * @param [in] aResponseCommandId the command whose content is being encoded.
     * @param [in] aEncodable - an encodable that places the command data structure
     *             for `aResponseCommandId` into a TLV Writer.
     *
     * If you have no great way of handling the returned CHIP_ERROR, consider
     * using `AddResponse` which will automatically reply with `Failure` in
     * case AddResponseData fails.
     */
    virtual CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                                       const DataModel::EncodableToTLV & aEncodable) = 0;

    /**
     * Attempts to encode a response to a command.
     *
     * `aRequestCommandPath` represents the request path (endpoint/cluster/commandid) and the reply
     * will preserve the same path and switch the command id to aResponseCommandId.
     *
     * As this command does not return any error codes, it must try its best to encode the reply
     * and if it fails, it MUST encode a `Protocols::InteractionModel::Status::Failure` as a
     * reply (i.e. a reply is guaranteed to be sent).
     *
     * Above is the main difference from AddResponseData: AddResponse will auto-reply with failure while
     * AddResponseData allows the caller to try to deal with any CHIP_ERRORs.
     */
    virtual void AddResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommandId,
                             const DataModel::EncodableToTLV & aEncodable) = 0;

    /**
     * Check whether the InvokeRequest we are handling is a timed invoke.
     */
    virtual bool IsTimedInvoke() const = 0;

    /**
     * @brief Flush acks right away for a slow command
     *
     * Some commands that do heavy lifting of storage/crypto should
     * ack right away to improve reliability and reduce needless retries. This
     * method can be manually called in commands that are especially slow to
     * immediately schedule an acknowledgement (if needed) since the delayed
     * stand-alone ack timer may actually not hit soon enough due to blocking command
     * execution.
     *
     */
    virtual void FlushAcksRightAwayOnSlowCommand() = 0;

    virtual Access::SubjectDescriptor GetSubjectDescriptor() const = 0;

    /**
     * Gets the inner exchange context object, without ownership.
     *
     * WARNING: This is dangerous, since it is directly interacting with the
     *          exchange being managed automatically by mpResponder and
     *          if not done carefully, may end up with use-after-free errors.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    virtual Messaging::ExchangeContext * GetExchangeContext() const = 0;

    /**
     * API for adding a data response.  The template parameter T is generally
     * expected to be a ClusterName::Commands::CommandName::Type struct, but any
     * object that can be encoded using the DataModel::Encode machinery and
     * exposes the right command id will work.
     *
     * If you have no great way of handling the returned CHIP_ERROR, consider
     * using `AddResponse` which will automatically reply with `Failure` in
     * case AddResponseData fails.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     *
     *             The response path will be the same as the request, except the
     *             reply command ID used will be `CommandData::GetCommandId()` assumed
     *             to be a member of the templated type
     *
     * @param [in] aData the data for the response. It is expected to provide
     *             `GetCommandData` as a STATIC on its type as well as encode the
     *             correct data structure for building a reply.
     */
    template <typename CommandData>
    CHIP_ERROR AddResponseData(const ConcreteCommandPath & aRequestCommandPath, const CommandData & aData)
    {
        DataModel::EncodableType<CommandData> encoder(aData);
        return AddResponseData(aRequestCommandPath, CommandData::GetCommandId(), encoder);
    }

    /**
     * API for adding a response.  This will try to encode a data response (response command), and if that fails
     * it will encode a Protocols::InteractionModel::Status::Failure status response instead.
     *
     * Above is the main difference from AddResponseData: AddResponse will auto-reply with failure while
     * AddResponseData allows the caller to try to deal with any CHIP_ERRORs.
     *
     * The template parameter T is generally expected to be a ClusterName::Commands::CommandName::Type struct, but any object that
     * can be encoded using the DataModel::Encode machinery and exposes the right command id will work.
     *
     * Since the function will call AddStatus when it fails to encode the data, it cannot send any response when it fails to encode
     * a status code since another AddStatus call will also fail. The error from AddStatus will just be logged.
     *
     * @param [in] aRequestCommandPath the concrete path of the command we are
     *             responding to.
     * @param [in] aData the data for the response.
     */
    template <typename CommandData>
    void AddResponse(const ConcreteCommandPath & aRequestCommandPath, const CommandData & aData)
    {
        DataModel::EncodableType<CommandData> encodable(aData);
        AddResponse(aRequestCommandPath, CommandData::GetCommandId(), encodable);
    }

protected:
    /**
     * IncrementHoldOff will increase the inner refcount of the CommandHandler.
     *
     * Users should use CommandHandler::Handle for management the lifespan of the CommandHandler.
     * DefRef should be released in reasonable time, and Close() should only be called when the refcount reached 0.
     */
    virtual void IncrementHoldOff(Handle * apHandle) {}

    /**
     * DecrementHoldOff is used by CommandHandler::Handle for decreasing the refcount of the CommandHandler.
     * When refcount reached 0, CommandHandler will send the response to the peer and shutdown.
     */
    virtual void DecrementHoldOff(Handle * apHandle) {}
};

} // namespace app
} // namespace chip
