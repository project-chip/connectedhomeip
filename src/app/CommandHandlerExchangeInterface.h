/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <access/SubjectDescriptor.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/GroupId.h>
#include <lib/core/Optional.h>
#include <messaging/ExchangeContext.h>
#include <protocols/interaction_model/StatusCode.h>
#include <system/SystemPacketBuffer.h>

namespace chip {
namespace app {

/**
 * Interface for sending InvokeResponseMessage(s).
 *
 * Provides information about the associated exchange context.
 *
 * Design Rationale: This interface enhances unit testability and allows applications to
 * customize InvokeResponse behavior. For example, a bridge application might locally execute
 * a command using cluster APIs without intending to sending a response on an exchange.
 * These cluster APIs require providing an instance of CommandHandler where a status response
 * is added (see https://github.com/project-chip/connectedhomeip/issues/32030).
 */
class CommandHandlerExchangeInterface
{
public:
    virtual ~CommandHandlerExchangeInterface() = default;

    /**
     * Get a non-owning pointer to the exchange context the InvokeRequestMessage was
     * delivered on.
     *
     * @return The exchange context. Might be nullptr if no exchange context has been
     *         assigned or the context has been released. For example, the exchange
     *         context might not be assigned in unit tests, or if an application wishes
     *         to locally execute cluster APIs and still receive response data without
     *         sending it on an exchange.
     */
    virtual Messaging::ExchangeContext * GetExchangeContext() const = 0;

    // TODO(#30453): Follow up refactor. It should be possible to remove
    // GetSubjectDescriptor and GetAccessingFabricIndex, as CommandHandler can get these
    // values from ExchangeContext.

    /**
     * Gets subject descriptor of the exchange.
     *
     * WARNING: This method should only be called when the caller is certain the
     * session has not been evicted.
     */
    virtual Access::SubjectDescriptor GetSubjectDescriptor() const = 0;

    /**
     * Gets accessing fabic index of the exchange.
     *
     * WARNING: This method should only be called when the caller is certain the
     * session has not been evicted.
     */
    virtual FabricIndex GetAccessingFabricIndex() const = 0;
    /**
     * If session for the exchange is a group session, returns its group ID. Otherwise,
     * returns a null optional.
     */
    virtual Optional<GroupId> GetGroupId() const = 0;

    /**
     * @brief Called to indicate a slow command is being processed.
     *
     * Enables the exchange to send whatever transport-level acks might be needed without waiting
     * for command processing to complete.
     */
    virtual void HandlingSlowCommand() = 0;

    /**
     * @brief Adds a completed InvokeResponseMessage to the queue for sending to requester.
     *
     * Called by CommandHandler.
     */
    virtual void AddInvokeResponseToSend(System::PacketBufferHandle && aPacket) = 0;

    /**
     * @brief Called to indicate that an InvokeResponse was dropped.
     *
     * Called by CommandHandler to relay this information to the requester.
     */
    virtual void ResponseDropped() = 0;

    /**
     * @brief Gets the maximum size of a packet buffer to encode a Command
     * Response message. This size depends on the underlying session used
     * by the exchange.
     *
     * The size returned here is the size not including the prepended headers.
     *
     * Called by CommandHandler when allocating buffer for encoding the Command
     * response.
     */
    virtual size_t GetCommandResponseMaxBufferSize() = 0;
};

} // namespace app
} // namespace chip
