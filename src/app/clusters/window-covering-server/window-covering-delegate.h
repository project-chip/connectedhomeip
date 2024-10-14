/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include <app/CommandResponseHelper.h>

namespace chip {
namespace app {
namespace Clusters {
namespace WindowCovering {

// Window Covering Type
enum class WindowCoveringType : uint8_t
{
    Lift     = 0x00, // window coverings that lift up and down or slide left to right
    Tilt     = 0x01, // window coverings with vertical or horizontal strips
    Reserved = 0x02, // dont use
};

/** @brief
 *    Defines methods for implementing application-specific logic for the WindowCovering Cluster.
 */
class Delegate
{
public:
    /**
     * @brief
     *   This method adjusts window covering position so the physical lift/slide and tilt is at the target
     *   open/up position set before calling this method. This will happen as fast as possible.
     *
     *   @param[in]  type            window covering type.
     *
     *   @return CHIP_NO_ERROR On success.
     *   @return Other Value indicating it failed to adjust window covering position.
     */
    virtual CHIP_ERROR HandleMovement(WindowCoveringType type) = 0;

    /**
     * @brief
     *   This method stops any adjusting to the physical tilt and lift/slide that is currently occurring.
     *
     *   @return CHIP_NO_ERROR On success.
     *   @return Other Value indicating it failed to stop any adjusting to the physical tilt and lift/slide that is currently
     * occurring..
     */
    virtual CHIP_ERROR HandleStopMotion() = 0;

    virtual ~Delegate() = default;

    void SetEndpoint(chip::EndpointId endpoint) { mEndpoint = endpoint; }

protected:
    EndpointId mEndpoint = 0;
};

} // namespace WindowCovering
} // namespace Clusters
} // namespace app
} // namespace chip
