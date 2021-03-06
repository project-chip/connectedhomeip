/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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
#pragma once

#include <app/ClusterCatalog.h>
#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>

namespace TestCluster {
extern chip::app::ClusterSchemaEngine ClusterSchema;

enum
{
    kClusterId = 0x00000014
};

enum
{
    kAttributeHandle_Root   = 1,
    kAttributeHandle_ValueA = 2,
    kAttributeHandle_ValueB = 3,
};
}; // namespace TestCluster

class MockClusterDataSink : public chip::app::ClusterDataSink
{
public:
    MockClusterDataSink(const chip::app::ClusterSchemaEngine * apClusterSchemaEngine);
    void ResetDataSink(void) { ClearVersion(); };
};

class MockClusterDataSource : public chip::app::ClusterDataSource
{
public:
    MockClusterDataSource(const chip::app::ClusterSchemaEngine * apClusterSchemaEngine);
};

class TestClusterDataSink : public MockClusterDataSink
{
public:
    TestClusterDataSink();
    int mValueA = 1;
    int mValueB = 2;

private:
    CHIP_ERROR SetLeafData(chip::app::AttributePathHandle aLeafHandle, chip::TLV::TLVReader & aReader) __OVERRIDE;
};

class TestClusterDataSource : public MockClusterDataSource
{
public:
    TestClusterDataSource();
    int mValueA = 1;
    int mValueB = 2;

private:
    CHIP_ERROR GetLeafData(chip::app::AttributePathHandle aLeafHandle, uint64_t aTagToWrite,
                           chip::TLV::TLVWriter & aWriter) __OVERRIDE;
};
