/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <app/AttributeAccessInterface.h>
#include <app/util/af.h>
#include <list>

namespace chip {
namespace app {
namespace Clusters {
namespace AudioOutput {

/** @brief
 *    Defines methods for implementing application-specific logic for the Audio Output Cluster.
 */
class Delegate
{
public:
    virtual uint8_t HandleGetCurrentOutput()                                            = 0;
    virtual CHIP_ERROR HandleGetOutputList(app::AttributeValueEncoder & aEncoder)       = 0;
    virtual bool HandleRenameOutput(const uint8_t & index, const chip::CharSpan & name) = 0;
    virtual bool HandleSelectOutput(const uint8_t & index)                              = 0;

    virtual ~Delegate() = default;
};

} // namespace AudioOutput
} // namespace Clusters
} // namespace app
} // namespace chip
