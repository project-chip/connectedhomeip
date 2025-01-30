/*
 *    Copyright (c) 2022-2025 Project CHIP Authors
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
#include <app/GlobalAttributes.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <protocols/interaction_model/StatusCode.h>

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {

bool IsSupportedGlobalAttributeNotInMetadata(AttributeId attributeId)
{
    for (auto & attr : GlobalAttributesNotInMetadata)
    {
        if (attr == attributeId)
        {
            return true;
        }
    }

    return false;
}

DataModel::ActionReturnStatus ReadGlobalAttributeFromMetadata(DataModel::Provider * provider, const ConcreteAttributePath & path,
                                                              AttributeValueEncoder & encoder)
{
    CHIP_ERROR err;

    switch (path.mAttributeId)
    {
    case Clusters::Globals::Attributes::GeneratedCommandList::Id: {
        DataModel::ListBuilder<CommandId> builder;
        err = provider->GeneratedCommands(path, builder);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
        auto buffer = builder.TakeBuffer();

        return encoder.EncodeList([&buffer](const auto & encoder) {
            for (auto id : buffer)
            {
                ReturnErrorOnFailure(encoder.Encode(id));
            }
            return CHIP_NO_ERROR;
        });
    }
    case Clusters::Globals::Attributes::AcceptedCommandList::Id: {
        DataModel::ListBuilder<DataModel::AcceptedCommandEntry> builder;
        err = provider->AcceptedCommands(path, builder);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
        auto buffer = builder.TakeBuffer();

        return encoder.EncodeList([&buffer](const auto & encoder) {
            for (auto entry : buffer)
            {
                ReturnErrorOnFailure(encoder.Encode(entry.commandId));
            }
            return CHIP_NO_ERROR;
        });
    }
    case Clusters::Globals::Attributes::AttributeList::Id: {
        DataModel::ListBuilder<DataModel::AttributeEntry> builder;
        err = provider->Attributes(path, builder);
        if (err != CHIP_NO_ERROR)
        {
            break;
        }
        auto buffer = builder.TakeBuffer();

        return encoder.EncodeList([&buffer](const auto & encoder) {
            bool addedExtraGlobals      = false;
            constexpr auto lastGlobalId = GlobalAttributesNotInMetadata[ArraySize(GlobalAttributesNotInMetadata) - 1];
            // If EventList is not supported. The GlobalAttributesNotInMetadata is missing one id here.
            static_assert(lastGlobalId - GlobalAttributesNotInMetadata[0] == ArraySize(GlobalAttributesNotInMetadata),
                          "Ids in GlobalAttributesNotInMetadata not consecutive (except EventList)");

            for (auto entry : buffer)
            {
                if (!addedExtraGlobals && entry.attributeId > lastGlobalId)
                {
                    for (const auto & globalId : GlobalAttributesNotInMetadata)
                    {
                        ReturnErrorOnFailure(encoder.Encode(globalId));
                    }
                    addedExtraGlobals = true;
                }
                ReturnErrorOnFailure(encoder.Encode(entry.attributeId));
            }

            if (!addedExtraGlobals)
            {
                for (auto id : GlobalAttributesNotInMetadata)
                {
                    ReturnErrorOnFailure(encoder.Encode(id));
                }
            }
            return CHIP_NO_ERROR;
        });
    }
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    // if we get here, the path was NOT valid
    if (err == CHIP_ERROR_NOT_FOUND)
    {
        return DataModel::ValidateClusterPath(provider, path, Status::Failure);
    }
    return err;
}

} // namespace app
} // namespace chip
