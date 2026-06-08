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

#include <app-common/zap-generated/cluster-objects.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {

/**
 * @brief Delegate interface for the Ambient Sensing Union cluster.
 *
 * Applications implement this to receive notifications about union state changes.
 */
class AmbientSensingUnionDelegate
{
public:
    virtual ~AmbientSensingUnionDelegate() = default;

    virtual void OnUnionNameChanged(const CharSpan & unionName) = 0;

    virtual void OnUnionHealthChanged(AmbientSensingUnion::UnionHealthEnum unionHealth) = 0;
};

} // namespace Clusters
} // namespace app
} // namespace chip

