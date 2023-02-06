/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <credentials/GroupDataProvider.h>
#include <crypto/CHIPCryptoPAL.h>

namespace chip {

namespace GroupTesting {

class DefaultIpkValue
{
public:
    DefaultIpkValue() {}

    static ByteSpan GetDefaultIpk()
    {
        static const uint8_t mDefaultIpk[Crypto::CHIP_CRYPTO_SYMMETRIC_KEY_LENGTH_BYTES] = {
            't', 'e', 'm', 'p', 'o', 'r', 'a', 'r', 'y', ' ', 'i', 'p', 'k', ' ', '0', '1'
        };
        return ByteSpan(mDefaultIpk);
    }
};

inline CHIP_ERROR InitData(chip::Credentials::GroupDataProvider * provider, chip::FabricIndex fabric_index,
                           const ByteSpan & compressed_fabric_id)
{
    static const chip::GroupId kGroup1   = 0x0101;
    static const chip::GroupId kGroup2   = 0x0102;
    static const chip::GroupId kGroup3   = 0x0103;
    static const chip::KeysetId kKeySet1 = 0x01a1;
    static const chip::KeysetId kKeySet2 = 0x01a2;
    static const chip::KeysetId kKeySet3 = 0x01a3;

    // Groups

    const chip::Credentials::GroupDataProvider::GroupInfo group1(kGroup1, "Group #1");
    ReturnErrorOnFailure(provider->SetGroupInfo(fabric_index, group1));
    ReturnErrorOnFailure(provider->AddEndpoint(fabric_index, group1.group_id, 1));

    const chip::Credentials::GroupDataProvider::GroupInfo group2(kGroup2, "Group #2");
    ReturnErrorOnFailure(provider->SetGroupInfo(fabric_index, group2));
    ReturnErrorOnFailure(provider->AddEndpoint(fabric_index, group2.group_id, 0));

    const chip::Credentials::GroupDataProvider::GroupInfo group3(kGroup3, "Group #3");
    ReturnErrorOnFailure(provider->SetGroupInfo(fabric_index, group3));
    ReturnErrorOnFailure(provider->AddEndpoint(fabric_index, group3.group_id, 0));

    // Key Sets

    chip::Credentials::GroupDataProvider::KeySet keyset1(kKeySet1,
                                                         chip::Credentials::GroupDataProvider::SecurityPolicy::kCacheAndSync, 3);
    const chip::Credentials::GroupDataProvider::EpochKey epoch_keys1[] = {
        { 1110000, { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf } },
        { 1110001, { 0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf } },
        { 1110002, { 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf } },
    };
    memcpy(keyset1.epoch_keys, epoch_keys1, sizeof(epoch_keys1));
    CHIP_ERROR err = provider->SetKeySet(fabric_index, compressed_fabric_id, keyset1);
    ReturnErrorOnFailure(err);

    chip::Credentials::GroupDataProvider::KeySet keyset2(kKeySet2,
                                                         chip::Credentials::GroupDataProvider::SecurityPolicy::kCacheAndSync, 3);
    const chip::Credentials::GroupDataProvider::EpochKey epoch_keys2[] = {
        { 2220000, { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf } },
        { 2220001, { 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef } },
        { 2220002, { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff } },
    };
    memcpy(keyset2.epoch_keys, epoch_keys2, sizeof(epoch_keys2));
    err = provider->SetKeySet(fabric_index, compressed_fabric_id, keyset2);
    ReturnErrorOnFailure(err);

    chip::Credentials::GroupDataProvider::KeySet keyset3(kKeySet3,
                                                         chip::Credentials::GroupDataProvider::SecurityPolicy::kTrustFirst, 3);
    const chip::Credentials::GroupDataProvider::EpochKey epoch_keys3[] = {
        { 2220000, { 0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf } },
        { 2220001, { 0xd1, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf } },
        { 2220002, { 0xd2, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf } },
    };
    memcpy(keyset3.epoch_keys, epoch_keys3, sizeof(epoch_keys3));
    err = provider->SetKeySet(fabric_index, compressed_fabric_id, keyset3);
    ReturnErrorOnFailure(err);

    provider->SetGroupKeyAt(fabric_index, 0, chip::Credentials::GroupDataProvider::GroupKey(kGroup1, kKeySet1));
    provider->SetGroupKeyAt(fabric_index, 1, chip::Credentials::GroupDataProvider::GroupKey(kGroup2, kKeySet2));
    provider->SetGroupKeyAt(fabric_index, 2, chip::Credentials::GroupDataProvider::GroupKey(kGroup3, kKeySet3));

    return CHIP_NO_ERROR;
}

} // namespace GroupTesting

} // namespace chip
