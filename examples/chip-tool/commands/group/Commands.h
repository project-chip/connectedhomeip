/*
 *   Copyright (c) 2022 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */

#pragma once

#include <lib/support/Span.h>

#include "commands/common/CHIPCommand.h"
#include "commands/common/Command.h"
#include "commands/common/Commands.h"

class ShowControllerGroups : public CHIPCommand
{
public:
    ShowControllerGroups(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("show-groups", credsIssuerConfig) {}
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    bool FindKeySetId(chip::FabricIndex fabricIndex, chip::GroupId groupId, chip::KeysetId & keysetId)
    {
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        auto iter                                                = groupDataProvider->IterateGroupKeys(fabricIndex);
        chip::Credentials::GroupDataProvider::GroupKey groupKey;
        while (iter->Next(groupKey))
        {
            if (groupKey.group_id == groupId)
            {
                keysetId = groupKey.keyset_id;
                iter->Release();
                return true;
            }
        }
        iter->Release();
        return false;
    }

    CHIP_ERROR RunCommand() override
    {
        fprintf(stderr, "\n");
        fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
        fprintf(stderr, "  | Available Groups :                                                                  |\n");
        fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
        fprintf(stderr, "  | Group Id   |  KeySet Id     |   Group Name                                          |\n");
        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        auto it                                                  = groupDataProvider->IterateGroupInfo(fabricIndex);
        chip::Credentials::GroupDataProvider::GroupInfo group;
        if (it)
        {
            while (it->Next(group))
            {
                chip::KeysetId keysetId;
                if (FindKeySetId(fabricIndex, group.group_id, keysetId))
                {
                    fprintf(stderr, "  | 0x%-12x  0x%-13x  %-50s |\n", group.group_id, keysetId, group.name);
                }
                else
                {
                    fprintf(stderr, "  | 0x%-12x  %-15s  %-50s |\n", group.group_id, "None", group.name);
                }
            }
            it->Release();
        }
        fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }
};

class AddGroup : public CHIPCommand
{
public:
    AddGroup(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("add-group", credsIssuerConfig)
    {
        AddArgument("groupName", &groupName);
        AddArgument("groupId", chip::kUndefinedGroupId, UINT16_MAX, &groupId);
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(20); }

    CHIP_ERROR RunCommand() override
    {
        if (strlen(groupName) > CHIP_CONFIG_MAX_GROUP_NAME_LENGTH || groupId == chip::kUndefinedGroupId)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        chip::Credentials::GroupDataProvider::GroupInfo group;

        group.SetName(groupName);
        group.group_id = groupId;
        ReturnErrorOnFailure(groupDataProvider->SetGroupInfo(fabricIndex, group));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

private:
    char * groupName;
    chip::GroupId groupId;
};

class RemoveGroup : public CHIPCommand
{
public:
    RemoveGroup(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("remove-group", credsIssuerConfig)
    {
        AddArgument("groupId", chip::kUndefinedGroupId, UINT16_MAX, &groupId);
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    CHIP_ERROR RunCommand() override
    {
        if (groupId == chip::kUndefinedGroupId)
        {
            ChipLogError(chipTool, "Invalid group Id : 0x%x", groupId);
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        ReturnErrorOnFailure(groupDataProvider->RemoveGroupInfo(fabricIndex, groupId));
        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

private:
    chip::GroupId groupId;
};

class ShowKeySets : public CHIPCommand
{
public:
    ShowKeySets(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("show-keysets", credsIssuerConfig) {}
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    CHIP_ERROR RunCommand() override
    {
        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        chip::Credentials::GroupDataProvider::KeySet keySet;

        fprintf(stderr, "\n");
        fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
        fprintf(stderr, "  | Available KeySets :                                                                 |\n");
        fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
        fprintf(stderr, "  | KeySet Id   |   Key Policy                                                          |\n");

        auto it = groupDataProvider->IterateKeySets(fabricIndex);
        if (it)
        {
            while (it->Next(keySet))
            {
                fprintf(stderr, "  | 0x%-12x  %-66s  |\n", keySet.keyset_id,
                        (keySet.policy == chip::Credentials::GroupDataProvider::SecurityPolicy::kCacheAndSync) ? "Cache and Sync"
                                                                                                               : "Trust First");
            }
            it->Release();
        }
        fprintf(stderr, "  +-------------------------------------------------------------------------------------+\n");
        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }
};

class BindKeySet : public CHIPCommand
{
public:
    BindKeySet(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("bind-keyset", credsIssuerConfig)
    {
        AddArgument("groupId", chip::kUndefinedGroupId, UINT16_MAX, &groupId);
        AddArgument("keysetId", 0, UINT16_MAX, &keysetId);
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    CHIP_ERROR RunCommand() override
    {
        size_t current_count                                     = 0;
        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();

        auto iter     = groupDataProvider->IterateGroupKeys(fabricIndex);
        current_count = iter->Count();
        iter->Release();

        ReturnErrorOnFailure(groupDataProvider->SetGroupKeyAt(fabricIndex, current_count,
                                                              chip::Credentials::GroupDataProvider::GroupKey(groupId, keysetId)));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

private:
    chip::GroupId groupId;
    chip::KeysetId keysetId;
};

class UnbindKeySet : public CHIPCommand
{
public:
    UnbindKeySet(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("unbind-keyset", credsIssuerConfig)
    {
        AddArgument("groupId", chip::kUndefinedGroupId, UINT16_MAX, &groupId);
        AddArgument("keysetId", 0, UINT16_MAX, &keysetId);
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    CHIP_ERROR RunCommand() override
    {
        size_t index                                             = 0;
        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        auto iter                                                = groupDataProvider->IterateGroupKeys(fabricIndex);
        size_t maxCount                                          = iter->Count();
        chip::Credentials::GroupDataProvider::GroupKey groupKey;
        while (iter->Next(groupKey))
        {
            if (groupKey.group_id == groupId && groupKey.keyset_id == keysetId)
            {
                break;
            }
            index++;
        }
        iter->Release();
        if (index >= maxCount)
        {
            return CHIP_ERROR_INTERNAL;
        }

        ReturnErrorOnFailure(groupDataProvider->RemoveGroupKeyAt(fabricIndex, index));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

private:
    chip::GroupId groupId;
    chip::KeysetId keysetId;
};

class AddKeySet : public CHIPCommand
{
public:
    AddKeySet(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("add-keysets", credsIssuerConfig)
    {
        AddArgument("keysetId", 0, UINT16_MAX, &keysetId);
        AddArgument("keyPolicy", 0, UINT16_MAX, &keyPolicy);
        AddArgument("validityTime", 0, UINT64_MAX, &validityTime);
        AddArgument("EpochKey", &epochKey);
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    CHIP_ERROR RunCommand() override
    {
        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();
        uint8_t compressed_fabric_id[sizeof(uint64_t)];
        chip::MutableByteSpan compressed_fabric_id_span(compressed_fabric_id);
        ReturnLogErrorOnFailure(CurrentCommissioner().GetCompressedFabricIdBytes(compressed_fabric_id_span));

        if ((keyPolicy != chip::Credentials::GroupDataProvider::SecurityPolicy::kCacheAndSync &&
             keyPolicy != chip::Credentials::GroupDataProvider::SecurityPolicy::kTrustFirst) ||
            (epochKey.size()) != chip::Credentials::GroupDataProvider::EpochKey::kLengthBytes)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }

        chip::Credentials::GroupDataProvider::KeySet keySet(keysetId, keyPolicy, 1);
        chip::Credentials::GroupDataProvider::EpochKey epoch_key;
        epoch_key.start_time = validityTime;
        memcpy(epoch_key.key, epochKey.data(), chip::Credentials::GroupDataProvider::EpochKey::kLengthBytes);

        memcpy(keySet.epoch_keys, &epoch_key, sizeof(chip::Credentials::GroupDataProvider::EpochKey));
        ReturnErrorOnFailure(groupDataProvider->SetKeySet(fabricIndex, compressed_fabric_id_span, keySet));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

private:
    chip::KeysetId keysetId;
    chip::Credentials::GroupDataProvider::SecurityPolicy keyPolicy;
    uint64_t validityTime;
    chip::ByteSpan epochKey;
};

class RemoveKeySet : public CHIPCommand
{
public:
    RemoveKeySet(CredentialIssuerCommands * credsIssuerConfig) : CHIPCommand("remove-keyset", credsIssuerConfig)
    {
        AddArgument("keysetId", 0, UINT16_MAX, &keysetId);
    }
    chip::System::Clock::Timeout GetWaitDuration() const override { return chip::System::Clock::Seconds16(5); }

    CHIP_ERROR RunCommand() override
    {
        CHIP_ERROR err                                           = CHIP_NO_ERROR;
        chip::FabricIndex fabricIndex                            = CurrentCommissioner().GetFabricIndex();
        chip::Credentials::GroupDataProvider * groupDataProvider = chip::Credentials::GetGroupDataProvider();

        // Unbind all group
        size_t index = 0;
        auto iter    = groupDataProvider->IterateGroupKeys(fabricIndex);
        chip::Credentials::GroupDataProvider::GroupKey groupKey;
        while (iter->Next(groupKey))
        {
            if (groupKey.keyset_id == keysetId)
            {
                err = groupDataProvider->RemoveGroupKeyAt(fabricIndex, index);
                if (err != CHIP_NO_ERROR)
                {
                    break;
                }
            }
            index++;
        }
        iter->Release();

        ReturnErrorOnFailure(err);
        ReturnErrorOnFailure(groupDataProvider->RemoveKeySet(fabricIndex, keysetId));

        SetCommandExitStatus(CHIP_NO_ERROR);
        return CHIP_NO_ERROR;
    }

private:
    chip::KeysetId keysetId;
};

void registerCommandsGroup(Commands & commands, CredentialIssuerCommands * credsIssuerConfig)
{
    const char * clusterName = "GroupSettings";

    commands_list clusterCommands = {
        make_unique<ShowControllerGroups>(credsIssuerConfig),
        make_unique<AddGroup>(credsIssuerConfig),
        make_unique<RemoveGroup>(credsIssuerConfig),
        make_unique<ShowKeySets>(credsIssuerConfig),
        make_unique<BindKeySet>(credsIssuerConfig),
        make_unique<UnbindKeySet>(credsIssuerConfig),
        make_unique<AddKeySet>(credsIssuerConfig),
        make_unique<RemoveKeySet>(credsIssuerConfig),
    };

    commands.RegisterCommandSet(clusterName, clusterCommands,
                                "Commands for manipulating group keys and memberships for chip-tool itself.");
}
