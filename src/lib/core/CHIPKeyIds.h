/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines constant enumerations for all CHIP key types,
 *      key flags, key ID fields, and helper API functions.
 *
 */

#pragma once

#include <limits.h>
#include <stdint.h>

namespace chip {

/**
 *  @class ChipKeyId
 *
 *  @brief
 *    The definition of the CHIP Key identifier. This class contains
 *    key types, key flags, key ID fields definition, and API functions.
 *
 */
class ChipKeyId
{
private:
    /**
     * @brief
     *   Private CHIP key ID fields, flags, and types.
     */
    enum
    {
        kMask_KeyFlags         = 0xF0000000, /**< CHIP key flag field mask. */
        kMask_KeyType          = 0x0FFFF000, /**< CHIP key type field mask. */
        kMask_KeyNumber        = 0x00000FFF, /**< CHIP key number field mask. */
        kMask_RootKeyNumber    = 0x00000C00, /**< Application group root key number field mask. */
        kMask_EpochKeyNumber   = 0x00000380, /**< Application group epoch key number field mask. */
        kMask_GroupLocalNumber = 0x0000007F, /**< Application group local number field mask. */

        kShift_RootKeyNumber    = 10, /**< Application group root key number field shift. */
        kShift_EpochKeyNumber   = 7,  /**< Application group epoch key number field shift. */
        kShift_GroupLocalNumber = 0,  /**< Application group local number field shift. */

        kFlag_UseCurrentEpochKey = 0x80000000, /**< Used to indicate that the key is of logical current type. */

        kTypeModifier_IncorporatesEpochKey = 0x00001000, /**< Used to indicate that the key incorporates group epoch key. */
    };

public:
    /**
     * @brief
     *   Public CHIP key ID fields, flags, and types.
     */
    enum
    {
        /**
         * @brief  CHIP key types used for CHIP message encryption.
         *
         * @note  16 (out of 32) most significant bits of the message encryption key
         *        type should be zero because only 16 least significant bits of the ID
         *        are encoded in the CHIP message.
         *  @{
         */
        kType_None         = 0x00000000, /**< CHIP message is unencrypted. */
        kType_General      = 0x00001000, /**< General key type. */
        kType_Session      = 0x00002000, /**< Session key type. */
        kType_AppStaticKey = 0x00004000, /**< Application static key type. */
        /** Application rotating key type. */
        kType_AppRotatingKey = kType_AppStaticKey | kTypeModifier_IncorporatesEpochKey,
        /** @} */

        /**
         * @brief  CHIP key types (other than CHIP message encryption types).
         *
         * @note  16 (out of 32) most significant bits of these types cannot be all zeros,
         *        because these values are reserved for the CHIP message encryption keys only.
         *  @{
         */

        /**
         * @brief  Constituent group key types.
         *  @{
         */
        /** Application group root key type. */
        kType_AppRootKey = 0x00010000,
        /** Application group epoch key type. */
        kType_AppEpochKey = 0x00020000 | kTypeModifier_IncorporatesEpochKey,
        /** Application group master key type. */
        kType_AppGroupMasterKey = 0x00030000,
        /** Application group intermediate key type. */
        kType_AppIntermediateKey = kType_AppRootKey | kTypeModifier_IncorporatesEpochKey,
        /** @} */

        /**
         * @brief  CHIP global key IDs.
         *  @{
         */
        /** Unspecified CHIP key ID. */
        kNone = kType_None | 0x0000,
        /** CHIP fabric secret ID. */
        kFabricSecret = kType_General | 0x0001,
        /** Fabric root key ID. */
        kFabricRootKey = kType_AppRootKey | (0 << kShift_RootKeyNumber),
        /** Client root key ID. */
        kClientRootKey = kType_AppRootKey | (1 << kShift_RootKeyNumber),
        /** Service root key ID. */
        kServiceRootKey = kType_AppRootKey | (2 << kShift_RootKeyNumber),
        /** @} */

        /**
         * @brief  Maximum values for key ID subfields.
         *  @{
         */
        kKeyNumber_Max        = kMask_KeyNumber,
        kRootKeyNumber_Max    = (kMask_RootKeyNumber >> kShift_RootKeyNumber),
        kEpochKeyNumber_Max   = (kMask_EpochKeyNumber >> kShift_EpochKeyNumber),
        kGroupLocalNumber_Max = (kMask_GroupLocalNumber >> kShift_GroupLocalNumber),
        /** @} */
    };

    /**
     *  Get CHIP key type of the specified key ID.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return                type of the key ID.
     *
     */
    static uint32_t GetType(uint32_t keyId) { return keyId & kMask_KeyType; }

    /**
     *  Determine whether the specified key ID is of a general type.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return      true      if the keyId has General type.
     *
     */
    static bool IsGeneralKey(uint32_t keyId) { return GetType(keyId) == kType_General; }

    /**
     *  Determine whether the specified key ID is of a session type.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return      true      if the keyId of a session type.
     *
     */
    static bool IsSessionKey(uint32_t keyId) { return GetType(keyId) == kType_Session; }

    /**
     *  Determine whether the specified key ID is of an application static type.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return      true      if the keyId of an application static type.
     *
     */
    static bool IsAppStaticKey(uint32_t keyId) { return GetType(keyId) == kType_AppStaticKey; }

    /**
     *  Determine whether the specified key ID is of an application rotating type.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return      true      if the keyId of an application rotating type.
     *
     */
    static bool IsAppRotatingKey(uint32_t keyId) { return GetType(keyId) == kType_AppRotatingKey; }

    static bool IsAppGroupKey(uint32_t keyId);

    /**
     *  Determine whether the specified key ID is of an application root key type.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return      true      if the keyId of an application root key type.
     *
     */
    static bool IsAppRootKey(uint32_t keyId) { return GetType(keyId) == kType_AppRootKey; }

    /**
     *  Determine whether the specified key ID is of an application epoch key type.
     *
     *  @param[in]   keyId     CHIP key identifier.
     *  @return      true      if the keyId of an application epoch key type.
     *
     */
    static bool IsAppEpochKey(uint32_t keyId) { return GetType(keyId) == kType_AppEpochKey; }

    /**
     *  Determine whether the specified key ID is of an application group master key type.
     *
     *  @param[in]       keyId     CHIP key identifier.
     *  @return  true      if the keyId of an application group master key type.
     *
     */
    static bool IsAppGroupMasterKey(uint32_t keyId) { return GetType(keyId) == kType_AppGroupMasterKey; }

    /**
     *  Construct session key ID given session key number.
     *
     *  @param[in]   sessionKeyNumber      Session key number.
     *  @return      session key ID.
     *
     */
    static uint16_t MakeSessionKeyId(uint16_t sessionKeyNumber)
    {
        static_assert(kType_Session <= UINT16_MAX, "We'll overflow");
        return static_cast<uint16_t>(kType_Session | (sessionKeyNumber & kMask_KeyNumber));
    }

    /**
     *  Construct general key ID given general key number.
     *
     *  @param[in]   generalKeyNumber       General key number.
     *  @return      general key ID.
     *
     */
    static uint16_t MakeGeneralKeyId(uint16_t generalKeyNumber)
    {
        static_assert(kType_General <= UINT16_MAX, "We'll overflow");
        return static_cast<uint16_t>(kType_General | (generalKeyNumber & kMask_KeyNumber));
    }

    /**
     *  Get application group root key ID that was used to derive specified application key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      root key ID.
     *
     */
    static uint32_t GetRootKeyId(uint32_t keyId) { return kType_AppRootKey | (keyId & kMask_RootKeyNumber); }

    /**
     *  Get application group epoch key ID that was used to derive specified application key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      epoch key ID.
     *
     */
    static uint32_t GetEpochKeyId(uint32_t keyId) { return kType_AppEpochKey | (keyId & kMask_EpochKeyNumber); }

    /**
     *  Get application group master key ID that was used to derive specified application key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      application group master key ID.
     *
     */
    static uint32_t GetAppGroupMasterKeyId(uint32_t keyId) { return kType_AppGroupMasterKey | (keyId & kMask_GroupLocalNumber); }

    /**
     *  Get application group root key number that was used to derive specified application key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      root key number.
     *
     */
    static uint8_t GetRootKeyNumber(uint32_t keyId)
    {
        return static_cast<uint8_t>((keyId & kMask_RootKeyNumber) >> kShift_RootKeyNumber);
    }

    /**
     *  Get application group epoch key number that was used to derive specified application key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      epoch key number.
     *
     */
    static uint8_t GetEpochKeyNumber(uint32_t keyId)
    {
        return static_cast<uint8_t>((keyId & kMask_EpochKeyNumber) >> kShift_EpochKeyNumber);
    }

    /**
     *  Get application group local number that was used to derive specified application key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      application group local number.
     *
     */
    static uint8_t GetAppGroupLocalNumber(uint32_t keyId)
    {
        return static_cast<uint8_t>((keyId & kMask_GroupLocalNumber) >> kShift_GroupLocalNumber);
    }

    /**
     *  Construct application group root key ID given root key number.
     *
     *  @param[in]   rootKeyNumber         Root key number.
     *  @return      root key ID.
     *
     */
    static uint32_t MakeRootKeyId(uint8_t rootKeyNumber)
    {
        return static_cast<uint32_t>(kType_AppRootKey | (rootKeyNumber << kShift_RootKeyNumber));
    }

    /**
     *  Construct application group root key ID given epoch key number.
     *
     *  @param[in]   epochKeyNumber        Epoch key number.
     *  @return      epoch key ID.
     *
     */
    static uint32_t MakeEpochKeyId(uint8_t epochKeyNumber)
    {
        return static_cast<uint32_t>(kType_AppEpochKey | (epochKeyNumber << kShift_EpochKeyNumber));
    }

    /**
     *  Construct application group master key ID given application group local number.
     *
     *  @param[in]   appGroupLocalNumber   Application group local number.
     *  @return      application group master key ID.
     *
     */
    static uint32_t MakeAppGroupMasterKeyId(uint8_t appGroupLocalNumber)
    {
        return static_cast<uint32_t>(kType_AppGroupMasterKey | (appGroupLocalNumber << kShift_GroupLocalNumber));
    }

    /**
     *  Convert application group key ID to application current key ID.
     *
     *  @param[in]   keyId                 Application key ID.
     *  @return      application current key ID.
     *
     */
    static uint32_t ConvertToCurrentAppKeyId(uint32_t keyId) { return (keyId & ~kMask_EpochKeyNumber) | kFlag_UseCurrentEpochKey; }

    /**
     *  Determine whether the specified application group key ID incorporates epoch key.
     *
     *  @param[in]   keyId     CHIP application group key identifier.
     *  @return      true      if the keyId incorporates epoch key.
     *
     */
    static bool IncorporatesEpochKey(uint32_t keyId) { return (keyId & kTypeModifier_IncorporatesEpochKey) != 0; }

    static bool UsesCurrentEpochKey(uint32_t keyId);
    static bool IncorporatesRootKey(uint32_t keyId);
    static bool IncorporatesAppGroupMasterKey(uint32_t keyId);

    static uint32_t MakeAppKeyId(uint32_t keyType, uint32_t rootKeyId, uint32_t epochKeyId, uint32_t appGroupMasterKeyId,
                                 bool useCurrentEpochKey);
    static uint32_t MakeAppIntermediateKeyId(uint32_t rootKeyId, uint32_t epochKeyId, bool useCurrentEpochKey);
    static uint32_t MakeAppRotatingKeyId(uint32_t rootKeyId, uint32_t epochKeyId, uint32_t appGroupMasterKeyId,
                                         bool useCurrentEpochKey);
    static uint32_t MakeAppStaticKeyId(uint32_t rootKeyId, uint32_t appGroupMasterKeyId);
    static uint32_t ConvertToStaticAppKeyId(uint32_t keyId);
    static uint32_t UpdateEpochKeyId(uint32_t keyId, uint32_t epochKeyId);

    static bool IsValidKeyId(uint32_t keyId);
    static bool IsMessageSessionId(uint32_t keyId, bool allowLogicalKeys = true);
    static bool IsSameKeyOrGroup(uint32_t keyId1, uint32_t keyId2);
    static const char * DescribeKey(uint32_t keyId);
};

} // namespace chip
