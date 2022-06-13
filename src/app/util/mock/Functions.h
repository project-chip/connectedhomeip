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

/**
 *    @file
 *     This file contains functions for the mocked attribute-storage.cpp
 */

#pragma once

#include <app/AttributeAccessInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/MessageDef/AttributeReportIB.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>

namespace chip {
namespace Test {
CHIP_ERROR ReadSingleMockClusterData(FabricIndex aAccessingFabricIndex, const app::ConcreteAttributePath & aPath,
                                     app::AttributeReportIBs::Builder & aAttributeReports,
                                     app::AttributeValueEncoder::AttributeEncodeState * apEncoderState);
void BumpVersion();
DataVersion GetVersion();
} // namespace Test
} // namespace chip
