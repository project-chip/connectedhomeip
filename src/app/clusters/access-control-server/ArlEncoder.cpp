/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ArlEncoder.h"

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

using Entry                  = AccessRestrictionProvider::Entry;
using EntryListener          = AccessRestrictionProvider::Listener;
using StagingRestrictionType = Clusters::AccessControl::AccessRestrictionTypeEnum;
using StagingRestriction     = Clusters::AccessControl::Structs::AccessRestrictionStruct::Type;

namespace {

CHIP_ERROR StageEntryRestrictions(const std::vector<AccessRestrictionProvider::Restriction> & source,
                                  StagingRestriction destination[], size_t destinationCount)
{
    size_t count = source.size();
    if (count > 0 && count <= destinationCount)
    {
        for (size_t i = 0; i < count; i++)
        {
            const auto & restriction = source[i];
            ReturnErrorOnFailure(ArlEncoder::Convert(restriction.restrictionType, destination[i].type));

            if (restriction.id.HasValue())
            {
                destination[i].id.SetNonNull(restriction.id.Value());
            }
        }
    }
    else
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    return CHIP_NO_ERROR;
}

} // namespace

namespace chip {
namespace app {

CHIP_ERROR ArlEncoder::Convert(Clusters::AccessControl::AccessRestrictionTypeEnum from,
                               Access::AccessRestrictionProvider::Type & to)
{
    switch (from)
    {
    case StagingRestrictionType::kAttributeAccessForbidden:
        to = AccessRestrictionProvider::Type::kAttributeAccessForbidden;
        break;
    case StagingRestrictionType::kAttributeWriteForbidden:
        to = AccessRestrictionProvider::Type::kAttributeWriteForbidden;
        break;
    case StagingRestrictionType::kCommandForbidden:
        to = AccessRestrictionProvider::Type::kCommandForbidden;
        break;
    case StagingRestrictionType::kEventForbidden:
        to = AccessRestrictionProvider::Type::kEventForbidden;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlEncoder::Convert(Access::AccessRestrictionProvider::Type from,
                               Clusters::AccessControl::AccessRestrictionTypeEnum & to)
{
    switch (from)
    {
    case AccessRestrictionProvider::Type::kAttributeAccessForbidden:
        to = StagingRestrictionType::kAttributeAccessForbidden;
        break;
    case AccessRestrictionProvider::Type::kAttributeWriteForbidden:
        to = StagingRestrictionType::kAttributeWriteForbidden;
        break;
    case AccessRestrictionProvider::Type::kCommandForbidden:
        to = StagingRestrictionType::kCommandForbidden;
        break;
    case AccessRestrictionProvider::Type::kEventForbidden:
        to = StagingRestrictionType::kEventForbidden;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlEncoder::CommissioningEncodableEntry::Encode(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    ReturnErrorOnFailure(Stage());
    ReturnErrorOnFailure(mStagingEntry.Encode(writer, tag));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlEncoder::EncodableEntry::EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex fabric) const
{
    ReturnErrorOnFailure(Stage());
    ReturnErrorOnFailure(mStagingEntry.EncodeForRead(writer, tag, fabric));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlEncoder::CommissioningEncodableEntry::Stage() const
{
    mStagingEntry.endpoint = mEntry.endpointNumber;
    mStagingEntry.cluster  = mEntry.clusterId;
    ReturnErrorOnFailure(StageEntryRestrictions(mEntry.restrictions, mStagingRestrictions,
                                                sizeof(mStagingRestrictions) / sizeof(mStagingRestrictions[0])));
    mStagingEntry.restrictions = Span<StagingRestriction>(mStagingRestrictions, mEntry.restrictions.size());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlEncoder::EncodableEntry::Stage() const
{
    mStagingEntry.fabricIndex = mEntry.fabricIndex;
    mStagingEntry.endpoint    = mEntry.endpointNumber;
    mStagingEntry.cluster     = mEntry.clusterId;
    ReturnErrorOnFailure(StageEntryRestrictions(mEntry.restrictions, mStagingRestrictions,
                                                sizeof(mStagingRestrictions) / sizeof(mStagingRestrictions[0])));
    mStagingEntry.restrictions = Span<StagingRestriction>(mStagingRestrictions, mEntry.restrictions.size());

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
