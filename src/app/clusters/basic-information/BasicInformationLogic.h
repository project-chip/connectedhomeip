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

#include <app/SpecificationDefinedRevisions.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/persistence/AttributePersistenceProvider.h>
#include <app/persistence/PascalString.h>
#include <clusters/BasicInformation/Structs.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <platform/ConfigurationManager.h>

namespace chip {
namespace app {
namespace Clusters {

/// Handles reading/writing of basic information data
class BasicInformationLogic
{
private:
    // singleton
    BasicInformationLogic() {}

public:
    static constexpr size_t kFixedLocationLength = 2;

    /// BasicInformationLogic is a SINGLETON according to the matter specification.
    static BasicInformationLogic & Instance();

    CHIP_ERROR Init(AttributePersistenceProvider & storage);

    bool GetReachable() const { return mReachable; }
    bool GetLocalConfigDisabled() const { return mLocalConfigDisabled; }
    CharSpan GetNodeLabel() const { return Storage::ShortPascalString::ContentOf(mNodeLabelBuffer); }

    // NOTE: these methods do NOT notify the cluster implementation of
    //       changes. Callers are responsible for that.
    DataModel::ActionReturnStatus SetLocalConfigDisabled(bool value, AttributePersistenceProvider & storage);
    DataModel::ActionReturnStatus SetNodeLabel(CharSpan label, AttributePersistenceProvider & storage);
    DataModel::ActionReturnStatus SetLocation(CharSpan location);

private:
    char mNodeLabelBuffer[32 + 1];
    bool mReachable           = true;
    bool mLocalConfigDisabled = false;
};

} // namespace Clusters
} // namespace app
} // namespace chip
