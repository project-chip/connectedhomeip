/*
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/data-model-provider/ProviderChangeListener.h>

#include <vector>

namespace chip {
namespace Test {

/// A provider change listener that stores all dirty reports
/// into an internal vector
class TestProviderChangeListener : public app::DataModel::ProviderChangeListener
{
public:
    void MarkDirty(const app::AttributePathParams & path) override { mDirtyList.push_back(path); }

    std::vector<app::AttributePathParams> & DirtyList() { return mDirtyList; }
    const std::vector<app::AttributePathParams> & DirtyList() const { return mDirtyList; }

private:
    std::vector<app::AttributePathParams> mDirtyList;
};

} // namespace Test
} // namespace chip
