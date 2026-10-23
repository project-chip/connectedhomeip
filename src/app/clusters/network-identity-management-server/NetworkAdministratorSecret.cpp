/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/network-identity-management-server/NetworkAdministratorSecret.h>
#include <lib/core/TLV.h>
#include <lib/support/CodeUtils.h>

namespace chip::Crypto {

using namespace TLV;

// network-administrator-shared-secret [anonymous] => STRUCTURE [tag-order]
enum
{
    kTag_Version   = 1, // version [1]    : UNSIGNED INTEGER [ range 8 bits ],
    kTag_Created   = 2, // created [2]    : UNSIGNED INTEGER [ range 32 bits ],
    kTag_RawSecret = 3, // raw-secret [3] : OCTET STRING [ length 32 ],
};

CHIP_ERROR EncodeNetworkAdministratorSecret(const NetworkAdministratorSecretData & secretData, MutableByteSpan & outEncoded)
{
    TLVWriter writer;
    TLVType outer;

    writer.Init(outEncoded);
    ReturnErrorOnFailure(writer.StartContainer(AnonymousTag(), kTLVType_Structure, outer));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_Version), static_cast<uint8_t>(0)));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_Created), secretData.created.count()));
    ReturnErrorOnFailure(writer.Put(ContextTag(kTag_RawSecret), ByteSpan(secretData.rawSecret.Span())));
    ReturnErrorOnFailure(writer.EndContainer(outer));
    ReturnErrorOnFailure(writer.Finalize());
    outEncoded.reduce_size(writer.GetLengthWritten());
    return CHIP_NO_ERROR;
}

CHIP_ERROR DecodeNetworkAdministratorSecret(const ByteSpan & encoded, NetworkAdministratorSecretData & outSecretData)
{
    TLVReader reader;
    TLVType outer;

    reader.Init(encoded);
    ReturnErrorOnFailure(reader.Next(kTLVType_Structure, AnonymousTag()));
    ReturnErrorOnFailure(reader.EnterContainer(outer));

    ReturnErrorOnFailure(reader.Next(ContextTag(kTag_Version)));
    uint8_t version;
    ReturnErrorOnFailure(reader.Get(version));
    VerifyOrReturnError(version == 0, CHIP_ERROR_INVALID_ARGUMENT);

    ReturnErrorOnFailure(reader.Next(ContextTag(kTag_Created)));
    uint32_t created;
    ReturnErrorOnFailure(reader.Get(created));
    outSecretData.created = System::Clock::Seconds32(created);

    ReturnErrorOnFailure(reader.Next(ContextTag(kTag_RawSecret)));
    ByteSpan rawSecretBytes;
    ReturnErrorOnFailure(reader.Get(rawSecretBytes));
    constexpr auto secretSize = decltype(outSecretData.rawSecret)::Length();
    VerifyOrReturnError(rawSecretBytes.size() == secretSize, CHIP_ERROR_INVALID_TLV_ELEMENT);
    memcpy(outSecretData.rawSecret.Bytes(), rawSecretBytes.data(), secretSize);

    // Reject unknown trailing fields since they won't survive a round-trip through encode.
    VerifyOrReturnError(reader.Next() == CHIP_END_OF_TLV, CHIP_ERROR_INVALID_TLV_ELEMENT);

    return reader.ExitContainer(outer);
}

} // namespace chip::Crypto
