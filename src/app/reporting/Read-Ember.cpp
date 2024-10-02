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
#include <app/reporting/Read-Ember.h>

#include <app/AppConfig.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/util/MatterCallbacks.h>
#include <app/util/ember-compatibility-functions.h>

namespace chip {
namespace app {
namespace reporting {
namespace EmberImpl {

DataModel::ActionReturnStatus RetrieveClusterData(DataModel::Provider * dataModel,
                                                  const Access::SubjectDescriptor & subjectDescriptor, bool isFabricFiltered,
                                                  AttributeReportIBs::Builder & reportBuilder,
                                                  const ConcreteReadAttributePath & path, AttributeEncodeState * encoderState)
{
    // Odd ifdef is to only do this if the `Read-Check` does not do it already.
#if !CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
    ChipLogDetail(DataManagement, "<RE:Run> Cluster %" PRIx32 ", Attribute %" PRIx32 " is dirty", path.mClusterId,
                  path.mAttributeId);

    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Pre, path);
#endif // !CHIP_CONFIG_USE_DATA_MODEL_INTERFACE

    ReturnErrorOnFailure(ReadSingleClusterData(subjectDescriptor, isFabricFiltered, path, reportBuilder, encoderState));

    // Odd ifdef is to only do this if the `Read-Check` does not do it already.
#if !CHIP_CONFIG_USE_DATA_MODEL_INTERFACE
    DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Read,
                                                          DataModelCallbacks::OperationOrder::Post, path);
#endif // !CHIP_CONFIG_USE_DATA_MODEL_INTERFACE

    return CHIP_NO_ERROR;
}

bool IsClusterDataVersionEqualTo(DataModel::Provider * dataModel, const ConcreteClusterPath & path, DataVersion dataVersion)
{
    return IsClusterDataVersionEqual(path, dataVersion);
}

} // namespace EmberImpl
} // namespace reporting
} // namespace app
} // namespace chip
