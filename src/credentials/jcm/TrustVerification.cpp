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

#include "TrustVerification.h"

#include <controller/InvokeInteraction.h>
#include <credentials/CHIPCert.h>
#include <credentials/FabricTable.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Crypto;
using namespace ::chip::Controller;
using namespace chip::app::Clusters;

namespace chip {
namespace Credentials {
namespace JCM {

void TrustVerificationStateMachine::RegisterTrustVerificationDelegate(TrustVerificationDelegate * trustVerificationDelegate)
{
    ChipLogProgress(Controller, "JCM: Setting trust verification delegate");
    mTrustVerificationDelegate = trustVerificationDelegate;
}

void TrustVerificationStateMachine::StartTrustVerification()
{
    PerformTrustVerificationStage(GetNextTrustVerificationStage(kIdle));
}

void TrustVerificationStateMachine::TrustVerificationStageFinished(const TrustVerificationStage & completedStage,
                                                                   const TrustVerificationError & error)
{
    ChipLogProgress(Controller, "JCM: Trust Verification Stage Finished: %s", EnumToString(completedStage).c_str());

    if (mTrustVerificationDelegate != nullptr)
    {
        mTrustVerificationDelegate->OnProgressUpdate(*this, completedStage, mInfo, error);
    }

    if (error != TrustVerificationError::kSuccess)
    {
        OnTrustVerificationComplete(error);
        return;
    }

    if (completedStage == TrustVerificationStage::kComplete || completedStage == TrustVerificationStage::kError)
    {
        ChipLogProgress(Controller, "JCM: Trust Verification already complete or error");
        OnTrustVerificationComplete(error);
        return;
    }

    auto nextStage = GetNextTrustVerificationStage(completedStage);
    if (nextStage == TrustVerificationStage::kError)
    {
        OnTrustVerificationComplete(TrustVerificationError::kInternalError);
        return;
    }

    PerformTrustVerificationStage(nextStage);
}

} // namespace JCM
} // namespace Credentials
} // namespace chip
