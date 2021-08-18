/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <app/CommandHandler.h>
#include <app/common/gen/att-storage.h>
#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/attributes/Accessors.h>
#include <app/common/gen/cluster-id.h>
#include <app/common/gen/command-id.h>
#include <app/util/af.h>
#include <map>
#include <set>
#include <string.h>
#include <tuple>
#include <utility>

constexpr size_t GROUP_KEY_ROOT_SIZE  = 16;
constexpr size_t GROUP_KEY_EPOCH_SIZE = 8;
constexpr uint8_t GROUP_KEY_SET_SIZE  = 3;

using namespace chip;

struct RootKey
{
    bool active                             = false;
    uint8_t value[GROUP_KEY_ROOT_SIZE]      = { 0 };
    uint8_t startTime[GROUP_KEY_EPOCH_SIZE] = { 0 };
    uint8_t policy                          = 0;
};

struct KeySet
{
    uint8_t current = 0;
    RootKey epoch[GROUP_KEY_SET_SIZE];
};

static std::map<std::tuple<uint16_t, uint16_t>, KeySet *> sGroupKeys;
static std::set<std::tuple<uint16_t, GroupId, uint16_t>> sGroupStates;

//------------------------------------------------------------------------------
// Local functions
//------------------------------------------------------------------------------

static EmberAfStatus revokeKeys(uint16_t fabricIndex, uint16_t keySetIndex);
static EmberAfStatus revokeKeys(uint16_t fabricIndex);

static uint8_t nextEpoch(uint8_t epoch)
{
    return epoch < GROUP_KEY_SET_SIZE - 1 ? epoch + 1 : 0;
}

static uint8_t previousEpoch(uint8_t epoch)
{
    return epoch > 0 ? epoch - 1 : GROUP_KEY_SET_SIZE - 1;
}

static EmberAfStatus printKeys()
{
    emberAfGroupKeyManagementClusterPrint("GROUP KEY SETS(%u)", sGroupKeys.size());

    std::map<std::tuple<uint16_t, uint16_t>, KeySet *>::iterator it;
    uint16_t fabricIndex = 0;
    uint16_t keySet      = 0;

    for (it = sGroupKeys.begin(); it != sGroupKeys.end(); ++it)
    {
        fabricIndex = std::get<0>(it->first);
        keySet      = std::get<1>(it->first);

        emberAfGroupKeyManagementClusterPrint("  fabric:%u, set:%u", fabricIndex, keySet);
        uint8_t index     = it->second->current < GROUP_KEY_SET_SIZE ? it->second->current : 0;
        RootKey & current = it->second->epoch[index];
        if (current.active)
        {
            emberAfGroupKeyManagementClusterPrint("    current: key(%u), start(%u), policy:%u", sizeof(current.value),
                                                  sizeof(current.startTime), current.policy);

            index              = previousEpoch(index);
            RootKey & previous = it->second->epoch[index];
            if (previous.active)
            {
                emberAfGroupKeyManagementClusterPrint("    previous: key(%u), start(%u), policy:%u", sizeof(previous.value),
                                                      sizeof(previous.startTime), previous.policy);

                index         = previousEpoch(index);
                RootKey & old = it->second->epoch[index];
                if (old.active)
                {
                    emberAfGroupKeyManagementClusterPrint("    old: key(%u), start(%u), policy:%u", sizeof(old.value),
                                                          sizeof(old.startTime), old.policy);
                }
            }
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

static KeySet * getKeySet(uint16_t fabricIndex, uint16_t keySetIndex)
{
    std::tuple<uint16_t, uint16_t> index(fabricIndex, keySetIndex);
    std::map<std::tuple<uint16_t, uint16_t>, KeySet *>::iterator it = sGroupKeys.find(index);
    return it == sGroupKeys.end() ? nullptr : it->second;
}

static EmberAfStatus setKey(uint16_t fabricIndex, uint16_t keySetIndex, chip::ByteSpan & keyRoot, chip::ByteSpan & epochStartTime,
                            uint8_t securityPolicy)
{
    if (keyRoot.size() != GROUP_KEY_ROOT_SIZE || epochStartTime.size() != GROUP_KEY_EPOCH_SIZE)
    {
        return EMBER_ZCL_STATUS_INVALID_VALUE;
    }
    else
    {
        KeySet * set = getKeySet(fabricIndex, keySetIndex);
        if (set)
        {
            set->current = nextEpoch(set->current);
        }
        else
        {
            set                                                                  = new KeySet();
            sGroupKeys[std::tuple<uint16_t, uint16_t>(fabricIndex, keySetIndex)] = set;
        }
        RootKey & key = set->epoch[set->current];
        key.active    = true;
        memcpy(key.value, keyRoot.data(), GROUP_KEY_ROOT_SIZE);
        memcpy(key.startTime, epochStartTime.data(), GROUP_KEY_EPOCH_SIZE);
        printKeys();
        return EMBER_ZCL_STATUS_SUCCESS;
    }
}

static EmberAfStatus removeKey(uint16_t fabricIndex, uint16_t keySetIndex)
{
    emberAfGroupKeyManagementClusterPrint("removeKey(%u, %u)", fabricIndex, keySetIndex);

    std::tuple<uint16_t, uint16_t> index(fabricIndex, keySetIndex);
    std::map<std::tuple<uint16_t, uint16_t>, KeySet *>::iterator it = sGroupKeys.find(index);
    EmberAfStatus status                                            = EMBER_ZCL_STATUS_SUCCESS;

    if (it == sGroupKeys.end())
    {
        // No key found
        status = EMBER_ZCL_STATUS_NOT_FOUND;
    }
    else
    {
        // Existing key
        delete it->second;
        sGroupKeys.erase(it);
    }

    printKeys();
    revokeKeys(fabricIndex, keySetIndex);
    return status;
}

static EmberAfStatus removeAllKeys(uint16_t fabricIndex)
{
    emberAfGroupKeyManagementClusterPrint("removeAllKeys(%u)", fabricIndex);

    std::map<std::tuple<uint16_t, uint16_t>, KeySet *>::iterator it = sGroupKeys.begin();
    while (it != sGroupKeys.end())
    {
        auto curr = it++;
        if (std::get<0>(curr->first) == fabricIndex)
        {
            delete curr->second;
            sGroupKeys.erase(curr);
        }
    }
    printKeys();
    revokeKeys(fabricIndex);
    return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus printStatus()
{
    emberAfGroupKeyManagementClusterPrint("GROUP STATES(%u)", sGroupStates.size());

    std::set<std::tuple<uint16_t, GroupId, uint16_t>>::iterator it;
    uint16_t fabricIndex = 0;
    uint16_t groupId     = 0;
    uint16_t keySet      = 0;

    for (it = sGroupStates.begin(); it != sGroupStates.end(); ++it)
    {
        fabricIndex = std::get<0>(*it);
        groupId     = std::get<1>(*it);
        keySet      = std::get<2>(*it);
        emberAfGroupKeyManagementClusterPrint("  fabric:%u, group:%u, set:%u", fabricIndex, groupId, keySet);
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus assignKey(uint16_t fabricIndex, GroupId groupId, uint16_t keySetIndex)
{
    KeySet * set = getKeySet(fabricIndex, keySetIndex);
    if (set)
    {
        sGroupStates.insert(std::tuple<uint16_t, GroupId, uint16_t>(fabricIndex, groupId, keySetIndex));
        printStatus();
        return EMBER_ZCL_STATUS_SUCCESS;
    }
    else
    {
        return EMBER_ZCL_STATUS_NOT_FOUND;
    }
}

static EmberAfStatus revokeKey(uint16_t fabricIndex, GroupId groupId, uint16_t keySetIndex)
{
    sGroupStates.erase(std::tuple<uint16_t, GroupId, uint16_t>(fabricIndex, groupId, keySetIndex));
    printStatus();
    return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus revokeKeys(uint16_t fabricIndex, uint16_t keySetIndex)
{
    std::set<std::tuple<uint16_t, GroupId, uint16_t>>::iterator it = sGroupStates.begin();
    while (it != sGroupStates.end())
    {
        auto curr = it++;
        if (std::get<0>(*curr) == fabricIndex && std::get<2>(*curr) == keySetIndex)
        {
            sGroupStates.erase(curr);
        }
    }
    printStatus();
    return EMBER_ZCL_STATUS_SUCCESS;
}

static EmberAfStatus revokeKeys(uint16_t fabricIndex)
{
    std::set<std::tuple<uint16_t, GroupId, uint16_t>>::iterator it = sGroupStates.begin();
    while (it != sGroupStates.end())
    {
        auto curr = it++;
        if (std::get<0>(*curr) == fabricIndex)
        {
            sGroupStates.erase(curr);
        }
    }
    printStatus();
    return EMBER_ZCL_STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Group Key Management Cluster server callbacks
//------------------------------------------------------------------------------

/** @brief Group Key Management Cluster Server Init
 *
 * Server Init
 *
 * @param endpoint    Endpoint that is being initialized
 */
void emberAfGroupKeyManagementClusterServerInitCallback(chip::EndpointId endpoint) {}

/**
 * @brief  Cluster AddKey Command callback (from client)
 */
bool emberAfGroupKeyManagementClusterSetKeyCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                    uint16_t fabricIndex, uint16_t keySetIndex, chip::ByteSpan keyRoot,
                                                    chip::ByteSpan epochStartTime, uint8_t securityPolicy)
{
    EmberAfStatus result = EMBER_ZCL_STATUS_FAILURE;

    if (0 == endpoint)
    {
        result = setKey(fabricIndex, keySetIndex, keyRoot, epochStartTime, securityPolicy);
    }
    else
    {
        result = EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    EmberStatus status = emberAfSendImmediateDefaultResponse(result);
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Group Key Management: failed to send %s: 0x%x", "default_response", status);
    }
    return true;
}

/**
 * @brief  Cluster RemoveKey Command callback (from client)
 */
bool emberAfGroupKeyManagementClusterRemoveKeyCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t fabricIndex, uint16_t keySetIndex)
{
    EmberAfStatus result = EMBER_ZCL_STATUS_FAILURE;

    if (0 == endpoint)
    {
        result = removeKey(fabricIndex, keySetIndex);
    }
    else
    {
        result = EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    EmberStatus status = emberAfSendImmediateDefaultResponse(result);
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Group Key Management: failed to send %s: 0x%x", "default_response", status);
    }
    return true;
}
/**
 * @brief  Cluster RemoveAllKeys Command callback (from client)
 */
bool emberAfGroupKeyManagementClusterRemoveAllKeysCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                           uint16_t fabricIndex)
{
    EmberAfStatus result = EMBER_ZCL_STATUS_FAILURE;

    if (0 == endpoint)
    {
        result = removeAllKeys(fabricIndex);
    }
    else
    {
        result = EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    EmberStatus status = emberAfSendImmediateDefaultResponse(result);
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Group Key Management: failed to send %s: 0x%x", "default_response", status);
    }
    return true;
}
/**
 * @brief  Cluster AssignKey Command callback (from client)
 */
bool emberAfGroupKeyManagementClusterAssignKeyCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t fabricIndex, uint16_t groupId, uint16_t keySetIndex)
{
    EmberAfStatus result = EMBER_ZCL_STATUS_FAILURE;

    if (0 == endpoint)
    {
        result = assignKey(fabricIndex, groupId, keySetIndex);
    }
    else
    {
        result = EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    EmberStatus status = emberAfSendImmediateDefaultResponse(result);
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Group Key Management: failed to send %s: 0x%x", "default_response", status);
    }
    return true;
}
/**
 * @brief  Cluster RevokeKey Command callback (from client)
 */
bool emberAfGroupKeyManagementClusterRevokeKeyCallback(chip::EndpointId endpoint, chip::app::CommandHandler * commandObj,
                                                       uint16_t fabricIndex, uint16_t groupId, uint16_t keySetIndex)
{
    EmberAfStatus result = EMBER_ZCL_STATUS_FAILURE;

    if (0 == endpoint)
    {
        result = revokeKey(fabricIndex, groupId, keySetIndex);
    }
    else
    {
        result = EMBER_ZCL_STATUS_INVALID_VALUE;
    }

    EmberStatus status = emberAfSendImmediateDefaultResponse(result);
    if (EMBER_SUCCESS != status)
    {
        emberAfGroupsClusterPrintln("Group Key Management: failed to send %s: 0x%x", "default_response", status);
    }
    return true;
}
