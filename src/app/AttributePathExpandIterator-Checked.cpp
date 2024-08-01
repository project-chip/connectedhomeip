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

#include "lib/support/logging/TextOnlyLogging.h"
#include <app/AttributePathExpandIterator-Checked.h>

namespace chip {
namespace app {
AttributePathExpandIteratorChecked::AttributePathExpandIteratorChecked(DataModel::Provider * dataModel,
                                                                       SingleLinkedListNode<AttributePathParams> * attributePath) :
    mDataModelIterator(dataModel, attributePath),
    mEmberIterator(dataModel, attributePath)
{
    CheckOutputsIdentical("Constructor");
}

bool AttributePathExpandIteratorChecked::Next()
{
    bool dmResult = mDataModelIterator.Next();
    bool emResult = mEmberIterator.Next();

    CheckOutputsIdentical("Next");

    VerifyOrDie(dmResult == emResult);

    return emResult;
}

bool AttributePathExpandIteratorChecked::Get(ConcreteAttributePath & aPath)
{
    CheckOutputsIdentical("Get");
    return mEmberIterator.Get(aPath);
}

void AttributePathExpandIteratorChecked::ResetCurrentCluster()
{
    mDataModelIterator.ResetCurrentCluster();
    mEmberIterator.ResetCurrentCluster();

    CheckOutputsIdentical("ResetCurrentCluster");
}

void AttributePathExpandIteratorChecked::ResetTo(SingleLinkedListNode<AttributePathParams> * paths)

{
    mDataModelIterator.ResetTo(paths);
    mEmberIterator.ResetTo(paths);
    CheckOutputsIdentical("ResetTo");
}

void AttributePathExpandIteratorChecked::CheckOutputsIdentical(const char * msg)
{
    ConcreteAttributePath dmPath;
    ConcreteAttributePath emPath;

    bool dmResult = mDataModelIterator.Get(dmPath);
    bool emResult = mEmberIterator.Get(emPath);

    if (dmResult == emResult)
    {
        // We check for:
        //    - either failed result (in which case path should not matter)
        //    - or exact match of paths on success
        //
        // NOTE: extra logic because mExpanded is NOT considered in operator== (ugly...)
        if ((dmResult == false) || ((dmPath == emPath) && (dmPath.mExpanded == emPath.mExpanded)))
        {
            // outputs are identical. All is good
            return;
        }
    }

    ChipLogProgress(Test, "Different paths in DM vs EMBER (%d and %d) in %s", dmResult, emResult, msg);
    ChipLogProgress(Test, "   DM PATH:    0x%X/" ChipLogFormatMEI "/" ChipLogFormatMEI " (%s)", dmPath.mEndpointId,
                    ChipLogValueMEI(dmPath.mClusterId), ChipLogValueMEI(dmPath.mAttributeId),
                    dmPath.mExpanded ? "EXPANDED" : "NOT expanded");
    ChipLogProgress(Test, "   EMBER PATH: 0x%X/" ChipLogFormatMEI "/" ChipLogFormatMEI " (%s)", emPath.mEndpointId,
                    ChipLogValueMEI(emPath.mClusterId), ChipLogValueMEI(emPath.mAttributeId),
                    emPath.mExpanded ? "EXPANDED" : "NOT expanded");

    chipDie();
}

} // namespace app
} // namespace chip
