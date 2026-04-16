/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/ConcreteAttributePath.h>
#include <app/data-model-provider/AttributeChangeListener.h>

#include <algorithm>
#include <vector>

namespace chip {
namespace Testing {

/// A provider change listener that stores all dirty reports
/// into an internal vector
class TestAttributeChangeListener : public app::DataModel::AttributeChangeListener
{
public:
    void OnAttributeChanged(const app::ConcreteAttributePath & path, app::DataModel::AttributeChangeType type) override
    {
        mChangedList.push_back(path);
        if (type == app::DataModel::AttributeChangeType::kReportable)
        {
            mDirtyList.push_back(path);
        }
    }

    void OnEndpointChanged(EndpointId endpointId, app::DataModel::EndpointChangeType type) override
    {
        mChangedEndpoints.push_back(endpointId);
    }

    std::vector<app::ConcreteAttributePath> & DirtyList() { return mDirtyList; }
    const std::vector<app::ConcreteAttributePath> & DirtyList() const { return mDirtyList; }

    std::vector<app::ConcreteAttributePath> & ChangedList() { return mChangedList; }
    const std::vector<app::ConcreteAttributePath> & ChangedList() const { return mChangedList; }

    std::vector<EndpointId> & ChangedEndpoints() { return mChangedEndpoints; }
    const std::vector<EndpointId> & ChangedEndpoints() const { return mChangedEndpoints; }

    /**
     * @brief Check if a specific attribute path is dirty.
     *
     * @param path The concrete attribute path to check
     * @return true if the path is dirty, false otherwise
     */
    bool IsDirty(const app::ConcreteAttributePath & path) const
    {
        return std::find(mDirtyList.cbegin(), mDirtyList.cend(), path) != mDirtyList.cend();
    }

    bool IsChanged(const app::ConcreteAttributePath & path) const
    {
        return std::find(mChangedList.cbegin(), mChangedList.cend(), path) != mChangedList.cend();
    }

    bool IsChangedEndpoint(EndpointId endpoint) const
    {
        return std::find(mChangedEndpoints.cbegin(), mChangedEndpoints.cend(), endpoint) != mChangedEndpoints.cend();
    }

private:
    std::vector<app::ConcreteAttributePath> mDirtyList;
    std::vector<app::ConcreteAttributePath> mChangedList;
    std::vector<EndpointId> mChangedEndpoints;
};

} // namespace Testing
} // namespace chip
