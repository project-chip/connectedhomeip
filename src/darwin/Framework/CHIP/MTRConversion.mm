/**
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#import "MTRConversion.h"
#import "MTRLogging_Internal.h"

#include <lib/support/SafeInt.h>

CHIP_ERROR SetToCATValues(NSSet<NSNumber *> * catSet, chip::CATValues & values)
{
    values = chip::kUndefinedCATs;

    unsigned long long tagCount = catSet.count;
    if (tagCount > chip::kMaxSubjectCATAttributeCount) {
        MTR_LOG_ERROR("%llu CASE Authenticated Tags cannot be represented in a certificate.", tagCount);
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    size_t tagIndex = 0;
    for (NSNumber * boxedTag in [catSet.allObjects sortedArrayUsingSelector:@selector(compare:)]) {
        auto unboxedTag = boxedTag.unsignedLongLongValue;
        if (!chip::CanCastTo<chip::CASEAuthTag>(unboxedTag)) {
            MTR_LOG_ERROR("0x%llx is not a valid CASE Authenticated Tag value.", unboxedTag);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        auto tag = static_cast<chip::CASEAuthTag>(unboxedTag);
        if (!chip::IsValidCASEAuthTag(tag)) {
            MTR_LOG_ERROR("0x%" PRIx32 " is not a valid CASE Authenticated Tag value.", tag);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        values.values[tagIndex++] = tag;
    }

    return CHIP_NO_ERROR;
}

NSSet<NSNumber *> * CATValuesToSet(const chip::CATValues & values)
{
    auto * catSet = [[NSMutableSet alloc] initWithCapacity:values.GetNumTagsPresent()];
    for (auto & value : values.values) {
        if (value != chip::kUndefinedCAT) {
            [catSet addObject:@(value)];
        }
    }
    return [NSSet setWithSet:catSet];
}
