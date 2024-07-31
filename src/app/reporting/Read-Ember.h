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
#include <app/AttributeEncodeState.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/data-model-interface/DataModel.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace reporting {
namespace EmberImpl {

CHIP_ERROR RetrieveClusterData(InteractionModel::DataModel * dataModel, const Access::SubjectDescriptor & subjectDescriptor,
                               bool isFabricFiltered, AttributeReportIBs::Builder & reportBuilder,
                               const ConcreteReadAttributePath & path, AttributeEncodeState * encoderState);

} // namespace EmberImpl
} // namespace reporting
} // namespace app
} // namespace chip
