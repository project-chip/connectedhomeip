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

#include <lib/core/CHIPError.h>

namespace chip::app::Clusters {

/// Represents a class that tracks a "configuration version" and accepts increases
/// to such a version.
///
/// For example BasicInformationCluster may increase a global configuration version and
/// will notify when such a configuration change occurs.
class ConfigurationVersionDelegate
{
public:
    virtual ~ConfigurationVersionDelegate() = default;

    // Bump the configuration version by 1
    virtual CHIP_ERROR IncreaseConfigurationVersion() = 0;
};

} // namespace chip::app::Clusters
