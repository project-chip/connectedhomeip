/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <cstddef>
#include <cstdint>
#include <lib/support/Span.h>
#include <vector>

namespace chip {
namespace app {
namespace Clusters {
namespace WebRTCTransportProvider {

/**
 * @brief Owned storage backing an SFrameStruct::Type.
 *
 * Globals::Structs::SFrameStruct::Type holds only non-owning spans: the sender
 * and receiver key material points into the decoded command payload, which is
 * no longer valid once the command handler returns. This type deep-copies that
 * key material so a configuration retained by a delegate (e.g. for later frame
 * encryption) stays valid for the lifetime of this object.
 */
class SFrameConfigStorage : public Globals::Structs::SFrameStruct::Type
{
public:
    SFrameConfigStorage() = default;
    SFrameConfigStorage(const SFrameConfigStorage & other) { *this = other; }

    SFrameConfigStorage & operator=(const SFrameConfigStorage & other)
    {
        if (this == &other)
        {
            return *this;
        }

        audioCipherSuite = other.audioCipherSuite;
        videoCipherSuite = other.videoCipherSuite;
        ratchetBits      = other.ratchetBits;
        ratchetTime      = other.ratchetTime;

        CopyKey(other.senderKey, mSenderKeyKid, mSenderKeyBaseKey, senderKey);

        const size_t receiveKeyCount = other.receiveKeys.size();
        mReceiveKeys.resize(receiveKeyCount);
        mReceiveKeysKid.resize(receiveKeyCount);
        mReceiveKeysBaseKey.resize(receiveKeyCount);
        for (size_t i = 0; i < receiveKeyCount; i++)
        {
            CopyKey(other.receiveKeys[i], mReceiveKeysKid[i], mReceiveKeysBaseKey[i], mReceiveKeys[i]);
        }
        receiveKeys = DataModel::List<const Globals::Structs::SFrameKeyStruct::Type>(mReceiveKeys.data(), mReceiveKeys.size());

        return *this;
    }

    /**
     * @brief Builds an owned configuration from a decoded command payload.
     */
    static SFrameConfigStorage FromDecodable(const Globals::Structs::SFrameStruct::DecodableType & in)
    {
        SFrameConfigStorage out;
        out.audioCipherSuite = in.audioCipherSuite;
        out.videoCipherSuite = in.videoCipherSuite;
        out.ratchetBits      = in.ratchetBits;
        out.ratchetTime      = in.ratchetTime;

        CopyKey(in.senderKey, out.mSenderKeyKid, out.mSenderKeyBaseKey, out.senderKey);

        auto iter = in.receiveKeys.begin();
        while (iter.Next())
        {
            out.mReceiveKeysKid.emplace_back();
            out.mReceiveKeysBaseKey.emplace_back();
            out.mReceiveKeys.emplace_back();
            CopyKey(iter.GetValue(), out.mReceiveKeysKid.back(), out.mReceiveKeysBaseKey.back(), out.mReceiveKeys.back());
        }
        out.receiveKeys =
            DataModel::List<const Globals::Structs::SFrameKeyStruct::Type>(out.mReceiveKeys.data(), out.mReceiveKeys.size());

        return out;
    }

private:
    // Copies one key's byte material into owned buffers and rebinds `dst` to them.
    template <typename KeyStruct>
    static void CopyKey(const KeyStruct & src, std::vector<uint8_t> & kidStorage, std::vector<uint8_t> & baseKeyStorage,
                        Globals::Structs::SFrameKeyStruct::Type & dst)
    {
        CopyBytes(src.kid, kidStorage);
        CopyBytes(src.baseKey, baseKeyStorage);
        dst.kid     = ByteSpan(kidStorage.data(), kidStorage.size());
        dst.baseKey = ByteSpan(baseKeyStorage.data(), baseKeyStorage.size());
    }

    static void CopyBytes(const ByteSpan & src, std::vector<uint8_t> & dst)
    {
        if (src.empty())
        {
            dst.clear();
            return;
        }
        dst.assign(src.data(), src.data() + src.size());
    }

    std::vector<uint8_t> mSenderKeyKid;
    std::vector<uint8_t> mSenderKeyBaseKey;
    std::vector<Globals::Structs::SFrameKeyStruct::Type> mReceiveKeys;
    std::vector<std::vector<uint8_t>> mReceiveKeysKid;
    std::vector<std::vector<uint8_t>> mReceiveKeysBaseKey;
};

} // namespace WebRTCTransportProvider
} // namespace Clusters
} // namespace app
} // namespace chip
