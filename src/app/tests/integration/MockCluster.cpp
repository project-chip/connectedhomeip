/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2017 Nest Labs, Inc.
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
 *      This file declares mock event generator and example
 *
 */

#include "MockCluster.h"
#include <platform/CHIPDeviceLayer.h>
#include <system/SystemPacketBuffer.h>

namespace TestCluster {
chip::app::ClusterSchemaEngine::AttributeInfo gSchemaMap[] = {
    /*  ParentHandle                ContextTag */
    { kAttributeHandle_Root, 1 },
    { kAttributeHandle_Root, 2 }
};

chip::app::ClusterSchemaEngine ClusterSchema = { {
    kClusterId,
    gSchemaMap,
    sizeof(gSchemaMap) / sizeof(gSchemaMap[0]),
    1,
} };
}; // namespace TestCluster

MockClusterDataSink::MockClusterDataSink(const chip::app::ClusterSchemaEngine * apClusterSchemaEngine) :
    ClusterDataSink(apClusterSchemaEngine)
{}

MockClusterDataSource::MockClusterDataSource(const chip::app::ClusterSchemaEngine * apClusterSchemaEngine) :
    ClusterDataSource(apClusterSchemaEngine)
{}

TestClusterDataSink::TestClusterDataSink(void) : MockClusterDataSink(&TestCluster::ClusterSchema) {}

CHIP_ERROR
TestClusterDataSink::SetLeafData(chip::app::AttributePathHandle aLeafHandle, chip::TLV::TLVReader & aReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aLeafHandle)
    {
    case TestCluster::kAttributeHandle_ValueA:
        err = aReader.Get(mValueA);
        SuccessOrExit(err);
        ChipLogDetail(DataManagement, "<<  TestCluster kAttributeHandle_ValueA is = %d", mValueA);
        break;
    case TestCluster::kAttributeHandle_ValueB:
        err = aReader.Get(mValueB);
        SuccessOrExit(err);
        ChipLogDetail(DataManagement, "<<  TestCluster kAttributeHandle_ValueB is = %d", mValueB);
        break;
    default:
        ChipLogDetail(DataManagement, "<<  UNKNOWN!");
        err = CHIP_ERROR_TLV_TAG_NOT_FOUND;
    }

exit:
    return err;
}

TestClusterDataSource::TestClusterDataSource(void) : MockClusterDataSource(&TestCluster::ClusterSchema) {}

CHIP_ERROR
TestClusterDataSource::GetLeafData(chip::app::AttributePathHandle aLeafHandle, uint64_t aTagToWrite, chip::TLV::TLVWriter & aWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aLeafHandle)
    {
    case TestCluster::kAttributeHandle_ValueA:
        err = aWriter.Put(aTagToWrite, mValueA);
        SuccessOrExit(err);

        ChipLogDetail(DataManagement, ">>  TestCluster kAttributeHandle_ValueA is = %d", mValueA);
        break;
    case TestCluster::kAttributeHandle_ValueB:
        err = aWriter.Put(aTagToWrite, mValueB);
        SuccessOrExit(err);

        ChipLogDetail(DataManagement, ">>  TestCluster kAttributeHandle_ValueB is = %d", mValueB);
        break;

    default:
        ChipLogDetail(DataManagement, ">>  UNKNOWN!");
        ExitNow(err = CHIP_ERROR_TLV_TAG_NOT_FOUND);
    }

exit:
    return err;
}
