/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <lib/dnssd/minimal_mdns/ResponseSender.h>

#include <chrono>
#include <condition_variable>
#include <string>
#include <vector>

#include <pw_unit_test/framework.h>

#include <lib/core/StringBuilderAdapters.h>
#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/wire/RecordData.h>
#include <lib/dnssd/wire/records/Ptr.h>
#include <lib/dnssd/wire/records/Srv.h>
#include <lib/dnssd/wire/records/Txt.h>
#include <lib/support/CHIPMemString.h>
#include <system/SystemMutex.h>

namespace mdns {
namespace Minimal {
namespace test {

inline constexpr chip::Dnssd::QNamePart kIgnoreQNameParts[] = { "IGNORE", "THIS" };
namespace {
bool StringMatches(const chip::Dnssd::BytesRange & br, const char * str)
{
    return br.Size() == strlen(str) && memcmp(str, br.Start(), br.Size()) == 0;
}

template <size_t N>
void MakePrintableName(char (&location)[N], chip::Dnssd::SerializedQNameIterator name)
{
    auto buf = chip::Encoding::BigEndian::BufferWriter(reinterpret_cast<uint8_t *>(&location[0]), N);
    while (name.Next())
    {
        buf.Put(name.Value());
        buf.Put(".");
    }
    buf.Put('\0');
}

template <size_t N>
void MakePrintableName(char (&location)[N], chip::Dnssd::FullQName name)
{
    auto buf = chip::Encoding::BigEndian::BufferWriter(reinterpret_cast<uint8_t *>(&location[0]), N);
    for (size_t i = 0; i < name.nameCount; ++i)
    {
        buf.Put(name.names[i]);
        buf.Put(".");
    }
    buf.Put('\0');
}

} // namespace

class CheckOnlyServer : private chip::PoolImpl<ServerBase::EndpointInfo, 0, chip::ObjectPoolMem::kInline,
                                               ServerBase::EndpointInfoPoolType::Interface>,
                        public ServerBase,
                        public chip::Dnssd::ParserDelegate,
                        public chip::Dnssd::TxtRecordDelegate
{
public:
    CheckOnlyServer() : ServerBase(*static_cast<ServerBase::EndpointInfoPoolType *>(this)) { Reset(); }
    ~CheckOnlyServer() {}

    // Parser delegates
    void OnHeader(chip::Dnssd::ConstHeaderRef & header) override
    {
        EXPECT_TRUE(header.GetFlags().IsResponse());
        EXPECT_TRUE(header.GetFlags().IsValidMdns());
        mTotalRecords += header.GetAnswerCount() + header.GetAdditionalCount();

        if (!header.GetFlags().IsTruncated())
        {
            EXPECT_EQ(mTotalRecords, GetNumExpectedRecords());
            if (mTotalRecords != GetNumExpectedRecords())
            {
                ChipLogError(Discovery, "Received %d records, expected %d", mTotalRecords, GetNumExpectedRecords());
            }
            mHeaderFound = true;
        }
    }

    void OnResource(chip::Dnssd::ResourceType type, const chip::Dnssd::ResourceData & data) override
    {
        chip::Dnssd::SerializedQNameIterator target;
        switch (data.GetType())
        {
        case chip::Dnssd::QType::PTR:
            chip::Dnssd::ParsePtrRecord(data.GetData(), mPacketData, &target);
            break;
        case chip::Dnssd::QType::SRV: {
            chip::Dnssd::SrvRecord srv;
            bool srvParseOk = srv.Parse(data.GetData(), mPacketData);
            EXPECT_TRUE(srvParseOk);
            if (!srvParseOk)
            {
                return;
            }
            target = srv.GetName();
            break;
        }
        default:
            break;
        }

        bool recordIsExpected = false;
        for (auto & info : mExpectedRecordInfo)
        {
            if (info.record == nullptr || info.found)
            {
                continue;
            }

            if (data.GetType() == info.record->GetType() &&
                (info.record->GetName() == kIgnoreQname || data.GetName() == info.record->GetName()) &&
                (info.target == kIgnoreQname || target == info.target))
            {
                if (data.GetType() == chip::Dnssd::QType::TXT)
                {
                    // First parse out the expected record to see what keys/values we have.
                    ClearTxtRecords();
                    const chip::Dnssd::TxtResourceRecord * expectedTxt =
                        static_cast<const chip::Dnssd::TxtResourceRecord *>(info.record);
                    for (size_t t = 0; t < expectedTxt->GetNumEntries(); ++t)
                    {
                        bool ok = AddExpectedTxtRecord(expectedTxt->GetEntries()[t]);
                        EXPECT_TRUE(ok);
                    }
                    chip::Dnssd::ParseTxtRecord(data.GetData(), this);
                    if (CheckTxtRecordMatches())
                    {
                        info.found       = true;
                        recordIsExpected = true;
                        break;
                    }
                }
                else
                {
                    info.found       = true;
                    recordIsExpected = true;
                    break;
                }
            }
        }
        EXPECT_TRUE(recordIsExpected);
        if (!recordIsExpected)
        {
            char nameStr[64];
            char targetStr[64];
            chip::Dnssd::SerializedQNameIterator dataTarget;
            chip::Dnssd::SerializedQNameIterator it = data.GetName();
            MakePrintableName(nameStr, it);
            switch (data.GetType())
            {
            case chip::Dnssd::QType::PTR:
                chip::Dnssd::ParsePtrRecord(data.GetData(), data.GetData(), &dataTarget);
                break;
            case chip::Dnssd::QType::SRV: {
                chip::Dnssd::SrvRecord srv;
                if (srv.Parse(data.GetData(), data.GetData()))
                {
                    dataTarget = srv.GetName();
                }
                break;
            }
            default:
                break;
            }
            MakePrintableName(targetStr, dataTarget);
            ChipLogError(Discovery, "Received unexpected record of type %u: %s %s", static_cast<uint16_t>(data.GetType()), nameStr,
                         targetStr);
        }
    }

    void OnQuery(const chip::Dnssd::QueryData & data) override {}

    // chip::Dnssd::TxtRecordDelegate
    void OnRecord(const chip::Dnssd::BytesRange & name, const chip::Dnssd::BytesRange & value) override
    {
        for (size_t i = 0; i < mNumExpectedTxtRecords; ++i)
        {
            if (StringMatches(name, mExpectedTxt[i].key) && StringMatches(value, mExpectedTxt[i].val))
            {
                mExpectedTxt[i].found = true;
                break;
            }
        }
        mNumReceivedTxtRecords++;
    }
    bool CheckTxtRecordMatches()
    {
        if (mNumReceivedTxtRecords != mNumExpectedTxtRecords)
        {
            return false;
        }
        for (size_t i = 0; i < mNumExpectedTxtRecords; ++i)
        {
            if (!mExpectedTxt[i].found)
            {
                return false;
            }
        }
        return true;
    }

    // ServerBase overrides
    CHIP_ERROR
    DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
               chip::Inet::InterfaceId interface) override
    {
        mPacketData = chip::Dnssd::BytesRange(data->Start(), data->Start() + data->TotalLength());
        chip::Dnssd::ParseMdnsPacket(mPacketData, this);
        if (mHeaderFound)
        {
            TestGotAllExpectedPackets();
        }
        mSendCalled = true;
        return CHIP_NO_ERROR;
    }

    // Functions used for controlling testing.
    void AddExpectedRecord(chip::Dnssd::PtrResourceRecord * ptr)
    {
        RecordInfo * info = AddExpectedRecordBase(ptr);
        if (info == nullptr)
        {
            return;
        }
        info->target = ptr->GetPtr();
    }
    void AddExpectedRecord(chip::Dnssd::SrvResourceRecord * srv)
    {
        RecordInfo * info = AddExpectedRecordBase(srv);
        ASSERT_NE(info, nullptr);
        if (info == nullptr)
        {
            return;
        }
        info->target = srv->GetServerName();
    }
    void AddExpectedRecord(chip::Dnssd::TxtResourceRecord * txt)
    {
        RecordInfo * info = AddExpectedRecordBase(txt);
        ASSERT_NE(info, nullptr);
        if (info == nullptr)
        {
            return;
        }
        info->target = kIgnoreQname;
    }
    bool GetSendCalled() { return mSendCalled; }
    bool GetHeaderFound() { return mHeaderFound; }
    void Reset()
    {
        for (auto & info : mExpectedRecordInfo)
        {
            info.record = nullptr;
            info.found  = false;
        }
        mHeaderFound  = false;
        mSendCalled   = false;
        mTotalRecords = 0;
        ClearTxtRecords();
    }

private:
    static constexpr size_t kMaxExpectedRecords = 10;
    struct RecordInfo
    {
        chip::Dnssd::ResourceRecord * record;
        bool found = false;
        chip::Dnssd::FullQName target;
    };
    RecordInfo mExpectedRecordInfo[kMaxExpectedRecords];
    struct KV
    {
        static constexpr size_t kMaxKey = 10;
        static constexpr size_t kMaxVal = 128; // max pairing instruction len + 1
        char key[kMaxKey + 1]           = "";
        char val[kMaxVal + 1]           = "";
        bool found                      = false;
        bool operator==(const KV & rhs) const { return strcmp(key, rhs.key) == 0 && strcmp(val, rhs.val) == 0; }
        void Clear()
        {
            memset(key, 0, sizeof(key));
            memset(val, 0, sizeof(val));
            found = false;
        }
    };
    static constexpr size_t kMaxExpectedTxt = 13;
    KV mExpectedTxt[kMaxExpectedTxt];
    size_t mNumExpectedTxtRecords       = 0;
    size_t mNumReceivedTxtRecords       = 0;
    bool mHeaderFound                   = false;
    bool mSendCalled                    = false;
    int mTotalRecords                   = 0;
    chip::Dnssd::FullQName kIgnoreQname = chip::Dnssd::FullQName(kIgnoreQNameParts);
    chip::Dnssd::BytesRange mPacketData;

    int GetNumExpectedRecords() const
    {
        int num = 0;
        for (auto & info : mExpectedRecordInfo)
        {
            if (info.record != nullptr)
            {
                ++num;
            }
        }
        return num;
    }
    void TestGotAllExpectedPackets()
    {
        for (auto & info : mExpectedRecordInfo)
        {
            if (info.record == nullptr)
            {
                continue;
            }
            EXPECT_TRUE(info.found);
            if (!info.found)
            {
                char name[64];
                char target[64];
                MakePrintableName(name, info.record->GetName());
                MakePrintableName(target, info.target);
                ChipLogError(Discovery, "Did not receive expected record of type %u : %s %s",
                             static_cast<uint16_t>(info.record->GetType()), name, target);
            }
        }
    }
    void ClearTxtRecords()
    {
        for (auto & kv : mExpectedTxt)
        {
            kv.Clear();
        }
        mNumExpectedTxtRecords = 0;
        mNumReceivedTxtRecords = 0;
    }
    bool AddExpectedTxtRecord(const char * const entry)
    {
        if (mNumExpectedTxtRecords == kMaxExpectedTxt)
        {
            return false;
        }
        size_t lenKey      = strlen(entry);
        const char * equal = strchr(entry, '=');
        if (equal != nullptr)
        {
            chip::Platform::CopyString(mExpectedTxt[mNumExpectedTxtRecords].val, (equal + 1));
            lenKey = static_cast<size_t>(equal - entry);
        }
        chip::ByteSpan key = chip::ByteSpan(reinterpret_cast<const uint8_t *>(entry), lenKey);
        chip::Platform::CopyString(mExpectedTxt[mNumExpectedTxtRecords++].key, key);

        return true;
    }

    RecordInfo * AddExpectedRecordBase(chip::Dnssd::ResourceRecord * record)
    {
        for (auto & info : mExpectedRecordInfo)
        {
            if (info.record == nullptr)
            {
                info.record = record;
                info.found  = false;
                return &info;
            }
        }
        return nullptr;
    }
};

struct ServerSwapper
{
    ServerSwapper(CheckOnlyServer * server)
    {
        chip::Dnssd::GlobalMinimalMdnsServer::Instance().Server().Shutdown();
        chip::Dnssd::GlobalMinimalMdnsServer::Instance().SetReplacementServer(server);
    }
    ~ServerSwapper() { chip::Dnssd::GlobalMinimalMdnsServer::Instance().SetReplacementServer(nullptr); }
};

} // namespace test
} // namespace Minimal
} // namespace mdns
