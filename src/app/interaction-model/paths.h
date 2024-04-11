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
namespace InteractionModel {

/// Handles path attributes for interaction models.
///
/// It allows a user of the class to mark specific parths
/// as having changed. The intended use is for some listener to
/// perform operations as a result of something having changed,
/// usually by forwarding updates (e.g. in case of subscriptions
/// that cover that path).
///
/// Methods on this class MUCH be called from within the matter
/// main loop as they will likely trigger interaction model
/// internal updates and subscription event updates.
class Paths
{
public:
    virtual ~Paths() = 0;

    /// Mark some specific attributes dirty.
    /// Wildcards are supported.
    virtual void MarkDirty(const AttributePathParams & path) = 0;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
