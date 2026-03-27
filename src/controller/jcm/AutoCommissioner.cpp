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

#include "AutoCommissioner.h"

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/InteractionModelEngine.h>
#include <controller/CommissioningDelegate.h>
#include <credentials/CHIPCert.h>
#include <lib/core/CHIPCore.h>
#include <lib/core/CHIPError.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace chip::app::Clusters;

namespace chip {
namespace Controller {
namespace JCM {

/*
 * AutoCommissioner override implementation
 */
CHIP_ERROR AutoCommissioner::SetCommissioningParameters(const CommissioningParameters & params)
{
    ReturnErrorOnFailure(chip::Controller::AutoCommissioner::SetCommissioningParameters(params));

#if CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC
    if (params.GetUseJCM().ValueOr(false))
    {
        auto extraReadPaths = params.GetExtraReadPaths();

        mTempReadPaths.clear();
        mTempReadPaths.reserve(extraReadPaths.size() + mExtraReadPaths.size());
        mTempReadPaths.insert(mTempReadPaths.end(), extraReadPaths.begin(), extraReadPaths.end());
        mTempReadPaths.insert(mTempReadPaths.end(), mExtraReadPaths.begin(), mExtraReadPaths.end());

        // Set the extra read paths for JCM
        mParams.SetExtraReadPaths(Span<app::AttributePathParams>(mTempReadPaths.data(), mTempReadPaths.size()));
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_JOINT_FABRIC

    return CHIP_NO_ERROR;
}

void AutoCommissioner::CleanupCommissioning()
{
    mTempReadPaths.clear();

    chip::Controller::AutoCommissioner::CleanupCommissioning();
}

} // namespace JCM
} // namespace Controller
} // namespace chip
