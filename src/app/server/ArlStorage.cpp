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

#include <app/server/ArlStorage.h>

#include <lib/support/DefaultStorageKeyAllocator.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Access;

using Entry                  = AccessRestriction::Entry;
using EntryListener          = AccessRestriction::EntryListener;
using StagingRestrictionType = Clusters::AccessControl::AccessRestrictionTypeEnum;
using StagingRestriction     = Clusters::AccessControl::Structs::AccessRestrictionStruct::Type;

namespace {

CHIP_ERROR Convert(StagingRestrictionType from, AccessRestriction::Type & to)
{
    switch (from)
    {
    case StagingRestrictionType::kAttributeAccessForbidden:
        to = AccessRestriction::Type::kAttributeAccessForbidden;
        break;
    case StagingRestrictionType::kAttributeWriteForbidden:
        to = AccessRestriction::Type::kAttributeWriteForbidden;
        break;
    case StagingRestrictionType::kCommandForbidden:
        to = AccessRestriction::Type::kCommandForbidden;
        break;
    case StagingRestrictionType::kEventForbidden:
        to = AccessRestriction::Type::kEventForbidden;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Convert(AccessRestriction::Type from, StagingRestrictionType & to)
{
    switch (from)
    {
    case AccessRestriction::Type::kAttributeAccessForbidden:
        to = StagingRestrictionType::kAttributeAccessForbidden;
        break;
    case AccessRestriction::Type::kAttributeWriteForbidden:
        to = StagingRestrictionType::kAttributeWriteForbidden;
        break;
    case AccessRestriction::Type::kCommandForbidden:
        to = StagingRestrictionType::kCommandForbidden;
        break;
    case AccessRestriction::Type::kEventForbidden:
        to = StagingRestrictionType::kEventForbidden;
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    return CHIP_NO_ERROR;
}

} // namespace

namespace chip {
namespace app {

CHIP_ERROR ArlStorage::DecodableEntry::Decode(TLV::TLVReader & reader)
{
    ReturnErrorOnFailure(mStagingEntry.Decode(reader));

    mEntry.fabricIndex    = mStagingEntry.GetFabricIndex();
    mEntry.endpointNumber = mStagingEntry.endpoint;
    mEntry.clusterId      = mStagingEntry.cluster;

    auto iterator = mStagingEntry.restrictions.begin();
    while (iterator.Next())
    {
        auto & tmp = iterator.GetValue();
        AccessRestriction::Restriction restriction;
        ReturnErrorOnFailure(Convert(tmp.type, restriction.restrictionType));

        if (!tmp.id.IsNull())
        {
            restriction.id.SetValue(tmp.id.Value());
        }

        mEntry.restrictions.push_back(restriction);
    }
    ReturnErrorOnFailure(iterator.GetStatus());

    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlStorage::EncodableEntry::EncodeForRead(TLV::TLVWriter & writer, TLV::Tag tag, FabricIndex fabric) const
{
    ReturnErrorOnFailure(Stage());
    ReturnErrorOnFailure(mStagingEntry.EncodeForRead(writer, tag, fabric));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlStorage::EncodableEntry::EncodeForWrite(TLV::TLVWriter & writer, TLV::Tag tag) const
{
    ReturnErrorOnFailure(Stage());
    ReturnErrorOnFailure(mStagingEntry.EncodeForWrite(writer, tag));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ArlStorage::EncodableEntry::Stage() const
{
    mStagingEntry.fabricIndex = mEntry->fabricIndex;
    mStagingEntry.endpoint    = mEntry->endpointNumber;
    mStagingEntry.cluster     = mEntry->clusterId;

    {
        size_t count = mEntry->restrictions.size();
        if (count > 0 && count <= CHIP_CONFIG_ACCESS_RESTRICTION_MAX_RESTRICTIONS_PER_ENTRY)
        {
            for (size_t i = 0; i < count; i++)
            {
                auto restriction = mEntry->restrictions[i];
                StagingRestriction tmp;
                ReturnErrorOnFailure(Convert(restriction.restrictionType, tmp.type));

                if (restriction.id.HasValue())
                {
                    tmp.id.SetNonNull(restriction.id.Value());
                }

                mStagingRestrictions[i] = tmp;
            }
            mStagingEntry.restrictions = Span<StagingRestriction>(mStagingRestrictions, count);
        }
        else
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
