/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

/**
 *    @file
 *      This file implements CHIP key types helper functions.
 *
 */
#include "CHIPKeyIds.h"

#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>

namespace chip {

/**
 *  Determine whether the specified key ID belongs to one of the application
 *  group key types (static or rotating).
 *
 *  @param[in]   keyId     CHIP key identifier.
 *  @return      true      if the keyId is of rotating or static key type.
 *
 */
bool ChipKeyId::IsAppGroupKey(uint32_t keyId)
{
    return IsAppStaticKey(keyId) || IsAppRotatingKey(keyId);
}

/**
 *  Determine whether the specified application group key ID uses "current" epoch key.
 *
 *  @param[in]   keyId     CHIP application group key identifier.
 *  @return      true      if the keyId indicates usage of the current epoch key.
 *
 */
bool ChipKeyId::UsesCurrentEpochKey(uint32_t keyId)
{
    return IncorporatesEpochKey(keyId) && ((keyId & kFlag_UseCurrentEpochKey) != 0);
}

/**
 *  Determine whether the specified application group key ID incorporates root key.
 *
 *  @param[in]   keyId     CHIP application group key identifier.
 *  @return      true      if the keyId incorporates root key.
 *
 */
bool ChipKeyId::IncorporatesRootKey(uint32_t keyId)
{
    uint32_t keyType = GetType(keyId);
    return keyType == kType_AppStaticKey || keyType == kType_AppRotatingKey || keyType == kType_AppRootKey ||
        keyType == kType_AppIntermediateKey;
}

/**
 *  Determine whether the specified application group key ID incorporates group master key.
 *
 *  @param[in]   keyId     CHIP application group key identifier.
 *  @return      true      if the keyId incorporates group master key.
 *
 */
bool ChipKeyId::IncorporatesAppGroupMasterKey(uint32_t keyId)
{
    uint32_t keyType = GetType(keyId);
    return keyType == kType_AppStaticKey || keyType == kType_AppRotatingKey || keyType == kType_AppGroupMasterKey;
}

/**
 *  Construct application group key ID given constituent key IDs and other information.
 *
 *  @param[in]   keyType               Derived application group key type.
 *  @param[in]   rootKeyId             Root key ID used to derive application group key.
 *  @param[in]   epochKeyId            Epoch key ID used to derive application group key.
 *  @param[in]   appGroupMasterKeyId   Application group master key ID used to derive
 *                                     application group key.
 *  @param[in]   useCurrentEpochKey    A boolean flag that indicates if key should be derived
 *                                     using "current" epoch key.
 *  @return      application group key ID.
 *
 */
uint32_t ChipKeyId::MakeAppKeyId(uint32_t keyType, uint32_t rootKeyId, uint32_t epochKeyId, uint32_t appGroupMasterKeyId,
                                 bool useCurrentEpochKey)
{
    return (keyType | (rootKeyId & kMask_RootKeyNumber) | (appGroupMasterKeyId & kMask_GroupLocalNumber) |
            (useCurrentEpochKey ? static_cast<uint32_t>(kFlag_UseCurrentEpochKey) : (epochKeyId & kMask_EpochKeyNumber)));
}

/**
 *  Construct application intermediate key ID given constituent key IDs.
 *
 *  @param[in]   rootKeyId             Root key ID used to derive application intermediate key.
 *  @param[in]   epochKeyId            Epoch key ID used to derive application intermediate key.
 *  @param[in]   useCurrentEpochKey    A boolean flag that indicates if key should be derived
 *                                     using "current" epoch key.
 *  @return      application intermediate key ID.
 *
 */
uint32_t ChipKeyId::MakeAppIntermediateKeyId(uint32_t rootKeyId, uint32_t epochKeyId, bool useCurrentEpochKey)
{
    return MakeAppKeyId(kType_AppIntermediateKey, rootKeyId, epochKeyId, kNone, useCurrentEpochKey);
}

/**
 *  Construct application rotating key ID given constituent key IDs and other information.
 *
 *  @param[in]   rootKeyId             Root key ID used to derive application rotating key.
 *  @param[in]   epochKeyId            Epoch key ID used to derive application rotating key.
 *  @param[in]   appGroupMasterKeyId   Application group master key ID used to derive
 *                                     application rotating key.
 *  @param[in]   useCurrentEpochKey    A boolean flag that indicates if key should be derived
 *                                     using "current" epoch key.
 *  @return      application rotating key ID.
 *
 */
uint32_t ChipKeyId::MakeAppRotatingKeyId(uint32_t rootKeyId, uint32_t epochKeyId, uint32_t appGroupMasterKeyId,
                                         bool useCurrentEpochKey)
{
    return MakeAppKeyId(kType_AppRotatingKey, rootKeyId, epochKeyId, appGroupMasterKeyId, useCurrentEpochKey);
}

/**
 *  Construct application static key ID given constituent key IDs.
 *
 *  @param[in]   rootKeyId             Root key ID used to derive application static key.
 *  @param[in]   appGroupMasterKeyId   Application group master key ID used to derive
 *                                     application static key.
 *  @return      application static key ID.
 *
 */
uint32_t ChipKeyId::MakeAppStaticKeyId(uint32_t rootKeyId, uint32_t appGroupMasterKeyId)
{
    return MakeAppKeyId(kType_AppStaticKey, rootKeyId, kNone, appGroupMasterKeyId, false);
}

/**
 *  Convert application key ID to application static key ID.
 *
 *  @param[in]   keyId                 Application key ID.
 *  @return      application static key ID.
 *
 */
uint32_t ChipKeyId::ConvertToStaticAppKeyId(uint32_t keyId)
{
    return MakeAppStaticKeyId(GetRootKeyId(keyId), GetAppGroupMasterKeyId(keyId));
}

/**
 *  Update application group key ID with new epoch key number.
 *
 *  @param[in]   keyId                 Application key ID.
 *  @param[in]   epochKeyId            Epoch key ID, which will be used in construction
 *                                     of the updated application key ID.
 *  @return      application key ID.
 *
 */
uint32_t ChipKeyId::UpdateEpochKeyId(uint32_t keyId, uint32_t epochKeyId)
{
    return (keyId & ~(kFlag_UseCurrentEpochKey | kMask_EpochKeyNumber)) | (epochKeyId & kMask_EpochKeyNumber);
}

/**
 *  Determine whether key identifier has valid (legal) value.
 *
 *  @param[in]   keyId                 CHIP key ID.
 *  @return      true                  if key ID value is valid.
 *
 */
bool ChipKeyId::IsValidKeyId(uint32_t keyId)
{
    unsigned int usedBits = kMask_KeyType;

    switch (GetType(keyId))
    {
    case kType_None:
        return false;
    case kType_General:
    case kType_Session:
        usedBits |= kMask_KeyNumber;
        break;
    case kType_AppStaticKey:
        usedBits |= kMask_RootKeyNumber | kMask_GroupLocalNumber;
        break;
    case kType_AppRotatingKey:
        usedBits |= kFlag_UseCurrentEpochKey | kMask_RootKeyNumber | kMask_GroupLocalNumber;
        if (!UsesCurrentEpochKey(keyId))
        {
            usedBits |= kMask_EpochKeyNumber;
        }
        break;
    case kType_AppRootKey:
        usedBits |= kMask_RootKeyNumber;
        break;
    case kType_AppIntermediateKey:
        usedBits |= kFlag_UseCurrentEpochKey | kMask_RootKeyNumber;
        if (!UsesCurrentEpochKey(keyId))
        {
            usedBits |= kMask_EpochKeyNumber;
        }
        break;
    case kType_AppEpochKey:
        usedBits |= kFlag_UseCurrentEpochKey;
        if (!UsesCurrentEpochKey(keyId))
        {
            usedBits |= kMask_EpochKeyNumber;
        }
        break;
    case kType_AppGroupMasterKey:
        usedBits |= kMask_GroupLocalNumber;
        break;
    default:
        return false;
    }

    if (IncorporatesRootKey(keyId))
    {
        uint32_t rootKeyId = GetRootKeyId(keyId);
        VerifyOrReturnError(rootKeyId == kFabricRootKey || rootKeyId == kClientRootKey || rootKeyId == kServiceRootKey, false);
    }

    return (keyId & ~usedBits) == 0;
}

/**
 *  Determine whether a given key ID identifies a key that is suitable for CHIP message encryption.
 *
 *  @param[in]   keyId                 CHIP key ID.
 *  @param[in]   allowLogicalKeys      Specifies whether logical keys IDs (such as the "current" rotating key)
 *                                     should be considered suitable for message encryption.
 *  @return      true                  If the identified key can be used to encrypt CHIP messages.
 *
 */
bool ChipKeyId::IsMessageEncryptionKeyId(uint32_t keyId, bool allowLogicalKeys)
{
    switch (GetType(keyId))
    {
    case kType_Session:
    case kType_AppStaticKey:
        return true;
    case kType_AppRotatingKey:
        return allowLogicalKeys || !UsesCurrentEpochKey(keyId);
    default:
        return false;
    }
}

/**
 * Determines whether two key IDs identify the same key, or in the case of rotating keys, the same
 * group of keys independent of any particular epoch.
 *
 *  @param[in]   keyId1                The first key ID to test.
 *  @param[in]   keyId2                The second key ID to test.
 *
 *  @return                            True if the keys IDs represent the same key.
 */
bool ChipKeyId::IsSameKeyOrGroup(uint32_t keyId1, uint32_t keyId2)
{
    enum
    {
        kIgnoreEpochMask = ~(kMask_EpochKeyNumber | kFlag_UseCurrentEpochKey)
    };

    // If the key ids are identical then they represent the same key.
    if (keyId1 == keyId2)
        return true;

    // For rotating keys, treat the key ids as the same if they differ only in their choice of epoch
    // key number.
    if (IncorporatesEpochKey(keyId1) && !IsAppEpochKey(keyId1) && (keyId1 & kIgnoreEpochMask) == (keyId2 & kIgnoreEpochMask))
        return true;

    // Otherwise the key ids identify different keys.
    return false;
}

/**
 *  Decode a CHIP key identifier with a descriptive string.
 *
 *  @param[in]   keyId     CHIP key ID to decode and for which to return
 *                         a descriptive string.
 *
 *  @return  A pointer to a NULL-terminated string describing the specified key ID.
 *
 */
const char * ChipKeyId::DescribeKey(uint32_t keyId)
{
    const char * retval;

    switch (GetType(keyId))
    {
    case kType_None:
        retval = "No Key";
        break;
    case kType_General:
        if (keyId == kFabricSecret)
        {
            retval = "Fabric Secret";
        }
        else
        {
            retval = "Other General Key";
        }
        break;
    case kType_Session:
        retval = "Session Key";
        break;
    case kType_AppStaticKey:
        retval = "Application Static Key";
        break;
    case kType_AppRotatingKey:
        retval = "Application Rotating Key";
        break;
    case kType_AppRootKey:
        if (keyId == kFabricRootKey)
        {
            retval = "Fabric Root Key";
        }
        else if (keyId == kClientRootKey)
        {
            retval = "Client Root Key";
        }
        else if (keyId == kServiceRootKey)
        {
            retval = "Service Root Key";
        }
        else
        {
            retval = "Other Root Key";
        }
        break;
    case kType_AppIntermediateKey:
        retval = "Application Intermediate Key";
        break;
    case kType_AppEpochKey:
        retval = "Application Epoch Key";
        break;
    case kType_AppGroupMasterKey:
        retval = "Application Group Master Key";
        break;
    default:
        retval = "Unknown Key Type";
    }

    return retval;
}

} // namespace chip
