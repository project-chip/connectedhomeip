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

#include "thread-br-delegate.h"

#include <app-common/zap-generated/cluster-objects.h>
#include <app/AttributeAccessInterface.h>
#include <app/AttributeValueEncoder.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteCommandPath.h>
#include <inet/UDPEndPoint.h>
#include <lib/core/Optional.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ThreadBorderRouterManagement {

class ServerInstance : public CommandHandlerInterface, public AttributeAccessInterface, public Delegate::ActivateDatasetCallback
{
public:
    ServerInstance(EndpointId endpointId, Delegate * delegate) :
        CommandHandlerInterface(Optional<EndpointId>(endpointId), Id),
        AttributeAccessInterface(Optional<EndpointId>(endpointId), Id), mDelegate(delegate)
    {}
    virtual ~ServerInstance() = default;

    CHIP_ERROR Init();

    // CommandHanlerInterface
    void InvokeCommand(HandlerContext & ctx) override;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

    // ActivateDatasetCallbackInterface
    void OnActivateDatasetComplete(CHIP_ERROR error) override;

private:
    // Command Handlers
    void HandleGetActiveDatasetRequest(HandlerContext & ctx, const Commands::GetActiveDatasetRequest::DecodableType & req);
    void HandleGetPendingDatasetRequest(HandlerContext & ctx, const Commands::GetPendingDatasetRequest::DecodableType & req);
    void HandleSetActiveDatasetRequest(HandlerContext & ctx, const Commands::SetActiveDatasetRequest::DecodableType & req);
    void HandleSetPendingDatasetRequest(HandlerContext & ctx, const Commands::SetPendingDatasetRequest::DecodableType & req);

    // Attribute Read handler
    CHIP_ERROR ReadFeatureMap(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadBorderRouterName(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadBorderAgentID(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadThreadVersion(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadInterfaceEnabled(AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadActiveDatasetTimestamp(AttributeValueEncoder & aEncoder);

    static void OnPlatformEventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg);
    void OnFailSafeTimerExpired();
    void CommitSavedBreadcrumb();

    Delegate * mDelegate;
    app::CommandHandler::Handle mAsyncCommandHandle;
    ConcreteCommandPath mPath = ConcreteCommandPath(0, 0, 0);
    Optional<uint64_t> mBreadcrumb;
};

} // namespace ThreadBorderRouterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
