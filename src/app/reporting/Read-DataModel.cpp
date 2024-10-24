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
#include <app/reporting/Read-DataModel.h>

#include <app/AppConfig.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/util/MatterCallbacks.h>
#include <optional>

namespace chip {
namespace app {
namespace reporting {
namespace DataModelImpl {

DataModel::ActionReturnStatus RetrieveClusterData(DataModel::Provider * dataModel,
                                                  const Access::SubjectDescriptor & subjectDescriptor, bool isFabricFiltered,
                                                  AttributeReportIBs::Builder & reportBuilder,
                                                  const ConcreteReadAttributePath & path, AttributeEncodeState * encoderState)
{
    // Odd ifdef is to only do this if the `Read-Check` does not do it already.
#if !CHIP_CONFIG_USE_EMBER_DATA_MODEL
    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", path.mClusterId,
                  path.mAttributeId);
    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Pre, path);
#endif // !CHIP_CONFIG_USE_EMBER_DATA_MODEL

    DataModel::ReadAttributeRequest readRequest;

    if (isFabricFiltered)
    {
        readRequest.readFlags.Set(DataModel::ReadFlags::kFabricFiltered);
    }
    readRequest.subjectDescriptor = subjectDescriptor;
    readRequest.path              = path;

    DataVersion version = 0;
    if (std::optional<DataModel::ClusterInfo> clusterInfo = dataModel->GetClusterInfo(path); clusterInfo.has_value())
    {
        version = clusterInfo->dataVersion;
    }
    else
    {
        ChipLogError(DataManagement, "Read request on unknown cluster - no data version available");
    }

    TLV::TLVWriter checkpoint;
    reportBuilder.Checkpoint(checkpoint);

    AttributeValueEncoder attributeValueEncoder(reportBuilder, subjectDescriptor, path, version, isFabricFiltered, encoderState);

    DataModel::ActionReturnStatus status = dataModel->ReadAttribute(readRequest, attributeValueEncoder);

    if (status.IsSuccess())
    {
        // Odd ifdef is to only do this if the `Read-Check` does not do it already.
#if !CHIP_CONFIG_USE_EMBER_DATA_MODEL
        // TODO: this callback being only executed on success is awkward. The Write callback is always done
        //       for both read and write.
        //
        //       For now this preserves existing/previous code logic, however we should consider to ALWAYS
        //       call this.
        DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                              DataModelCallbacks::OperationOrder::Post, path);
#endif // !CHIP_CONFIG_USE_EMBER_DATA_MODEL
        return status;
    }

    // Encoder state is relevant for errors in case they are retryable.
    //
    // Generally only out of space encoding errors would be retryable, however we save the state
    // for all errors in case this is information that is useful (retry or error position).
    if (encoderState != nullptr)
    {
        *encoderState = attributeValueEncoder.GetState();
    }

    // Out of space errors may be chunked data, reporting those cases would be very confusing
    // as they are not fully errors. Report only others (which presumably are not recoverable
    // and will be sent to the client as well).
    if (!status.IsOutOfSpaceEncodingResponse())
    {
        DataModel::ActionReturnStatus::StringStorage storage;
        ChipLogError(DataManagement, "Failed to read attribute: %s", status.c_str(storage));
    }
    return status;
}

bool IsClusterDataVersionEqualTo(DataModel::Provider * dataModel, const ConcreteClusterPath & path, DataVersion dataVersion)
{
    std::optional<DataModel::ClusterInfo> info = dataModel->GetClusterInfo(path);
    if (!info.has_value())
    {
        return false;
    }

    return (info->dataVersion == dataVersion);
}

} // namespace DataModelImpl
} // namespace reporting
} // namespace app
} // namespace chip
