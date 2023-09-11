/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
