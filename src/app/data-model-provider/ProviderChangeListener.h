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

#include <app/AttributePathParams.h>

namespace chip {
namespace app {
namespace DataModel {

/// Notification listener for changes of the underlying data in a
/// data model.
///
/// Used to notify that a specific attribute path (or several attributes
/// via wildcards) have changed their underlying content.
///
/// Methods on this class MUST be called from within the matter
/// main loop as they will likely trigger interaction model
/// internal updates and subscription data reporting.
class ProviderChangeListener
{
public:
    virtual ~ProviderChangeListener() = default;

    /// Mark all attributes matching the given path (which may be a wildcard) dirty.
    ///
    /// Wildcards are supported.
    virtual void MarkDirty(const AttributePathParams & path) = 0;
};

} // namespace DataModel
} // namespace app
} // namespace chip
