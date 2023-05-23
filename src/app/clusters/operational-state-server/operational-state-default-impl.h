/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Operational State Server Cluster
 ***************************************************************************/
#include <app/util/af.h>

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/error-mapping.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

using Status = Protocols::InteractionModel::Status;

/**
 * @brief Operational State Cluster Stop Command callback (from client)
 */
bool emberAfOperationalStateClusterPauseCallback(
    CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
    const Clusters::OperationalState::Commands::Pause::DecodableType & commandData)
{
    commandHandler->AddStatus(commandPath, Status::Success);
    return true;
}
/**
 * @brief Operational State Cluster Stop Command callback (from client)
 */
bool emberAfOperationalStateClusterStopCallback(
    CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
    const Clusters::OperationalState::Commands::Stop::DecodableType & commandData)
{
    commandHandler->AddStatus(commandPath, Status::Success);
    return true;
}
/**
 * @brief Operational State Cluster Start Command callback (from client)
 */
bool emberAfOperationalStateClusterStartCallback(
    CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
    const Clusters::OperationalState::Commands::Start::DecodableType & commandData)
{
    commandHandler->AddStatus(commandPath, Status::Success);
    return true;
}
/**
 * @brief Operational State Cluster Resume Command callback (from client)
 */
bool emberAfOperationalStateClusterResumeCallback(
    CommandHandler * commandHandler, const ConcreteCommandPath & commandPath,
    const Clusters::OperationalState::Commands::Resume::DecodableType & commandData)
{
    commandHandler->AddStatus(commandPath, Status::Success);
    return true;
}
