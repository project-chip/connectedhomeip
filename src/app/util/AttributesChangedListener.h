/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/AttributePathParams.h>

namespace chip {
namespace app {

/// Notification object of a specific path being changed
class AttributesChangedListener
{
public:
    virtual ~AttributesChangedListener() = default;

    /// Called when the set of attributes identified by AttributePathParams (which may contain wildcards) is to be considered dirty.
    virtual void MarkDirty(const AttributePathParams & path) = 0;
};

} // namespace app
} // namespace chip
