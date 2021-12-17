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

#include <lib/dnssd/MinimalMdnsServer.h>
#include <lib/dnssd/minimal_mdns/RecordData.h>
#include <lib/dnssd/minimal_mdns/Server.h>
#include <lib/dnssd/minimal_mdns/records/Ptr.h>
#include <lib/dnssd/minimal_mdns/records/Srv.h>
#include <lib/dnssd/minimal_mdns/records/Txt.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/UnitTestRegistration.h>
#include <system/SystemMutex.h>

#include <nlunit-test.h>

namespace mdns {
namespace Minimal {
namespace test {

constexpr QNamePart kIgnoreQNameParts[] = { "IGNORE", "THIS" };
namespace {
bool StringMatches(const BytesRange & br, const char * str)
{
    return br.Size() == strlen(str) && memcmp(str, br.Start(), br.Size()) == 0;
}

template <size_t N>
void MakePrintableName(char (&location)[N], SerializedQNameIterator name)
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
void MakePrintableName(char (&location)[N], FullQName name)
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
                        public ParserDelegate,
                        public TxtRecordDelegate
{
public:
    CheckOnlyServer(nlTestSuite * inSuite) : ServerBase(*static_cast<ServerBase::EndpointInfoPoolType *>(this)), mInSuite(inSuite)
    {
        Reset();
    }
    CheckOnlyServer() : ServerBase(*static_cast<ServerBase::EndpointInfoPoolType *>(this)), mInSuite(nullptr) { Reset(); }
    ~CheckOnlyServer() {}

    // Parser delegates
    void OnHeader(ConstHeaderRef & header) override
    {
        NL_TEST_ASSERT(mInSuite, header.GetFlags().IsResponse());
        NL_TEST_ASSERT(mInSuite, header.GetFlags().IsValidMdns());
        mTotalRecords += header.GetAnswerCount() + header.GetAdditionalCount();

        if (!header.GetFlags().IsTruncated())
        {
            NL_TEST_ASSERT(mInSuite, mTotalRecords == GetNumExpectedRecords());
            if (mTotalRecords != GetNumExpectedRecords())
            {
                ChipLogError(Discovery, "Received %d records, expected %d", mTotalRecords, GetNumExpectedRecords());
            }
            mHeaderFound = true;
        }
    }

    void OnResource(ResourceType type, const ResourceData & data) override
    {
        SerializedQNameIterator target;
        switch (data.GetType())
        {
        case QType::PTR:
            ParsePtrRecord(data.GetData(), mPacketData, &target);
            break;
        case QType::SRV: {
            SrvRecord srv;
            bool srvParseOk = srv.Parse(data.GetData(), mPacketData);
            NL_TEST_ASSERT(mInSuite, srvParseOk);
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
                if (data.GetType() == QType::TXT)
                {
                    // First parse out the expected record to see what keys/values we have.
                    ClearTxtRecords();
                    const TxtResourceRecord * expectedTxt = static_cast<const TxtResourceRecord *>(info.record);
                    for (size_t t = 0; t < expectedTxt->GetNumEntries(); ++t)
                    {
                        bool ok = AddExpectedTxtRecord(expectedTxt->GetEntries()[t]);
                        NL_TEST_ASSERT(mInSuite, ok);
                    }
                    ParseTxtRecord(data.GetData(), this);
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
        NL_TEST_ASSERT(mInSuite, recordIsExpected);
        if (!recordIsExpected)
        {
            char nameStr[64];
            char targetStr[64];
            SerializedQNameIterator dataTarget;
            SerializedQNameIterator it = data.GetName();
            MakePrintableName(nameStr, it);
            switch (data.GetType())
            {
            case QType::PTR:
                ParsePtrRecord(data.GetData(), data.GetData(), &dataTarget);
                break;
            case QType::SRV: {
                SrvRecord srv;
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

    void OnQuery(const QueryData & data) override {}

    // TxtRecordDelegate
    void OnRecord(const BytesRange & name, const BytesRange & value) override
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
        mPacketData = BytesRange(data->Start(), data->Start() + data->TotalLength());
        ParsePacket(mPacketData, this);
        if (mHeaderFound)
        {
            TestGotAllExpectedPackets();
        }
        mSendCalled = true;
        return CHIP_NO_ERROR;
    }

    // Functions used for controlling testing.
    void AddExpectedRecord(PtrResourceRecord * ptr)
    {
        RecordInfo * info = AddExpectedRecordBase(ptr);
        if (info == nullptr)
        {
            return;
        }
        info->target = ptr->GetPtr();
    }
    void AddExpectedRecord(SrvResourceRecord * srv)
    {
        RecordInfo * info = AddExpectedRecordBase(srv);
        NL_TEST_ASSERT(mInSuite, info != nullptr);
        if (info == nullptr)
        {
            return;
        }
        info->target = srv->GetServerName();
    }
    void AddExpectedRecord(TxtResourceRecord * txt)
    {
        RecordInfo * info = AddExpectedRecordBase(txt);
        NL_TEST_ASSERT(mInSuite, info != nullptr);
        if (info == nullptr)
        {
            return;
        }
        info->target = kIgnoreQname;
    }
    bool GetSendCalled() { return mSendCalled; }
    bool GetHeaderFound() { return mHeaderFound; }
    void SetTestSuite(nlTestSuite * suite) { mInSuite = suite; }
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
    nlTestSuite * mInSuite;
    static constexpr size_t kMaxExpectedRecords = 10;
    struct RecordInfo
    {
        ResourceRecord * record;
        bool found = false;
        FullQName target;
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
    static constexpr size_t kMaxExpectedTxt = 11;
    KV mExpectedTxt[kMaxExpectedTxt];
    size_t mNumExpectedTxtRecords = 0;
    size_t mNumReceivedTxtRecords = 0;
    bool mHeaderFound             = false;
    bool mSendCalled              = false;
    int mTotalRecords             = 0;
    FullQName kIgnoreQname        = FullQName(kIgnoreQNameParts);
    BytesRange mPacketData;

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
        if (mInSuite == nullptr)
        {
            return;
        }
        for (auto & info : mExpectedRecordInfo)
        {
            if (info.record == nullptr)
            {
                continue;
            }
            NL_TEST_ASSERT(mInSuite, info.found == true);
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

    RecordInfo * AddExpectedRecordBase(ResourceRecord * record)
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
