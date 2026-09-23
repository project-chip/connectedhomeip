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

#include <cstdint>

namespace chip::app::Clusters {

/// Provides the Color Control behavior that other clusters depend on.
///
/// This is separated out from the cluster itself, so that unit testing is
/// easier (easier mocking) and integration points are clearer. It also keeps
/// consumers from linking the Color Control implementation when no Color
/// Control cluster is instantiated.
class ColorControlIntegrationDelegate
{
public:
    virtual ~ColorControlIntegrationDelegate() = default;

    /// Couples the color temperature to a Level Control CurrentLevel value.
    ///
    /// Level Control 1.6.6.5 (CoupleColorTempToLevel Bit): "If this bit is set, changes to the
    /// CurrentLevel attribute SHALL be coupled with the color temperature set in the Color Control
    /// cluster." Testing that bit is the caller's responsibility; this call is unconditional.
    ///
    /// The mapping is one-way (level -> color temperature) and only takes effect while the active
    /// color mode is color temperature.
    virtual void CoupleColorTempToLevel(uint8_t currentLevel) = 0;
};

} // namespace chip::app::Clusters
