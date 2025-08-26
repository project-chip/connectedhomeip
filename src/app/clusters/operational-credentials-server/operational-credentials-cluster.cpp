/*
 *
 *    Copyright (c) 2021-2025 Project CHIP Authors
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
#include <app/clusters/operational-credentials-server/operational-credentials-cluster.h>

/****************************************************************************
 * @file
 * @brief Implementation for the Operational Credentials Cluster
 ***************************************************************************/

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;

CHIP_ERROR OperationalCredentialsCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus OperationalCredentialsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request, AttributeValueEncoder & encoder)
{
    return Protocols::InteractionModel::Status::UnsupportedAttribute;
}

CHIP_ERROR OperationalCredentialsCluster::AcceptedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder) 
{
    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> OperationalCredentialsCluster::InvokeCommand(const DataModel::InvokeRequest & request, TLV::TLVReader & input_arguments, CommandHandler * handler) 
{
    return Protocols::InteractionModel::Status::UnsupportedCommand;
}