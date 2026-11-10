/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include "ClosureController.h"

#include <clusters/ClosureControl/Commands.h>
#include <clusters/ClosureControl/Ids.h>
#include <controller/InvokeInteraction.h>
#include <devices/Types.h>
#include <lib/core/Optional.h>
#include <lib/support/CHIPMem.h>

#include <variant>

using namespace chip::app::Clusters;

namespace chip::app {

namespace {
const ClusterId kClientClusters[] = { ClosureControl::Id };

constexpr uint16_t kTimedInvokeTimeoutMs = 10000;

void OnSuccess(const ConcreteCommandPath & path, const StatusIB & status, const DataModel::NullObjectType &)
{
    ChipLogProgress(AppServer, "Closure Control command 0x%" PRIx32 " succeeded on endpoint %u", path.mCommandId, path.mEndpointId);
}

void OnFailure(CHIP_ERROR error)
{
    ChipLogError(AppServer, "Closure Control command failed: %" CHIP_ERROR_FORMAT, error.Format());
}

class MoveToBindingCommand : public BindingCommand
{
public:
    explicit MoveToBindingCommand(const ClosureControl::Commands::MoveTo::Type & command) : mCommand(command) {}

    ClusterId GetClusterId() const override { return ClosureControl::Id; }

    void Dispatch(const BindingTarget & target) override
    {
        const auto * unicast = std::get_if<UnicastBindingTarget>(&target);
        VerifyOrReturn(unicast != nullptr, ChipLogError(AppServer, "MoveTo requires a timed invoke; group binding skipped"));

        LogErrorOnFailure(Controller::InvokeCommandRequest(&unicast->exchangeManager, unicast->session, unicast->endpoint, mCommand,
                                                           OnSuccess, OnFailure, kTimedInvokeTimeoutMs));
    }

private:
    ClosureControl::Commands::MoveTo::Type mCommand;
};

class CalibrateBindingCommand : public BindingCommand
{
public:
    explicit CalibrateBindingCommand(const ClosureControl::Commands::Calibrate::Type & command) : mCommand(command) {}

    ClusterId GetClusterId() const override { return ClosureControl::Id; }

    void Dispatch(const BindingTarget & target) override
    {
        const auto * unicast = std::get_if<UnicastBindingTarget>(&target);
        VerifyOrReturn(unicast != nullptr, ChipLogError(AppServer, "Calibrate requires a timed invoke; group binding skipped"));

        LogErrorOnFailure(Controller::InvokeCommandRequest(&unicast->exchangeManager, unicast->session, unicast->endpoint, mCommand,
                                                           OnSuccess, OnFailure, kTimedInvokeTimeoutMs));
    }

private:
    ClosureControl::Commands::Calibrate::Type mCommand;
};
} // namespace

ClosureController::ClosureController(const Config & config) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kClosureController, 1)), mConfig(config)
{}

CHIP_ERROR ClosureController::Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    mBindingCluster.Create(
        BindingCluster::Context{
            .bindingTable    = mConfig.bindingTable,
            .bindingManager  = mConfig.bindingManager,
            .platformManager = mConfig.platformManager,
        },
        endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mBindingCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void ClosureController::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);
    if (mBindingCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mBindingCluster.Cluster()));
        mBindingCluster.Destroy();
    }
}

CHIP_ERROR ClosureController::ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const
{
    return out.ReferenceExisting(Span<const ClusterId>(kClientClusters));
}

CHIP_ERROR ClosureController::SendMoveTo(const MoveToArgs & args)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    Platform::UniquePtr<BindingCommand> command(Platform::New<MoveToBindingCommand>(ClosureControl::Commands::MoveTo::Type{
        .position = FromStdOptional(args.position),
        .latch    = FromStdOptional(args.latch),
        .speed    = FromStdOptional(args.speed),
    }));
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
    return mConfig.bindingHandler.Dispatch(mEndpointId, std::move(command));
}

CHIP_ERROR ClosureController::SendGroupedMoveTo(const MoveToArgs & args)
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    return mConfig.bindingHandler.DispatchSimple(mEndpointId,
                                                 ClosureControl::Commands::GroupedMoveTo::Type{
                                                     .position = FromStdOptional(args.position),
                                                     .latch    = FromStdOptional(args.latch),
                                                     .speed    = FromStdOptional(args.speed),
                                                 });
}

CHIP_ERROR ClosureController::SendStop()
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    return mConfig.bindingHandler.DispatchSimple(mEndpointId, ClosureControl::Commands::Stop::Type{});
}

CHIP_ERROR ClosureController::SendCalibrate()
{
    VerifyOrReturnError(mEndpointId != kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    Platform::UniquePtr<BindingCommand> command(
        Platform::New<CalibrateBindingCommand>(ClosureControl::Commands::Calibrate::Type{}));
    VerifyOrReturnError(command != nullptr, CHIP_ERROR_NO_MEMORY);
    return mConfig.bindingHandler.Dispatch(mEndpointId, std::move(command));
}

} // namespace chip::app
