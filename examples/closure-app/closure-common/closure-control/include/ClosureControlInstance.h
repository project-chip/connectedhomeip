/*
 *
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

#include <ClosureControlDelegate.h>
#include <app/clusters/closure-control-server/closure-control-server.h>
#include <app/util/af-types.h>
#include <lib/core/CHIPError.h>

namespace chip {
namespace app {
namespace Clusters {
namespace ClosureControl {
using namespace chip::app::Clusters::ClosureControl;

class ClosureControlInstance : public Instance
{
public:
    ClosureControlInstance(EndpointId aEndpointId, ClosureControlDelegate & aDelegate, Feature aFeatures,
                           OptionalAttribute aOptionalAttribute) :
        ClosureControl::Instance(aEndpointId, aDelegate, aFeatures, aOptionalAttribute)
    {
        mDelegate = &aDelegate;
    }

    // Delete copy constructor and assignment operator.
    ClosureControlInstance(const ClosureControlInstance &)             = delete;
    ClosureControlInstance(const ClosureControlInstance &&)            = delete;
    ClosureControlInstance & operator=(const ClosureControlInstance &) = delete;

    CHIP_ERROR Init();
    void Shutdown();

    ClosureControlDelegate * GetDelegate() { return mDelegate; };

private:
    ClosureControlDelegate * mDelegate;
};

} // namespace ClosureControl
} // namespace Clusters
} // namespace app
} // namespace chip
