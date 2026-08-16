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
#include <cstdint>

namespace chip::app::Clusters {

/// Defines an interface for tracking a breadcrumb value.
///
/// The Breadcrumb attribute allows commissioners and administrators
/// to save a small payload that can be subsequently read, to keep
/// track of progress (see 11.10.6.1 of the Matter spec related
/// to Breadcrumb Attribute in the General Commissioning Cluster).
class BreadCrumbTracker
{
public:
    virtual ~BreadCrumbTracker()               = default;
    virtual void SetBreadCrumb(uint64_t value) = 0;
};

} // namespace chip::app::Clusters
