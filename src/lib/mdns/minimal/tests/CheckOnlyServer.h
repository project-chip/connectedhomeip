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
#include <mdns/minimal/ResponseSender.h>

#include <chrono>
#include <condition_variable>
#include <string>
#include <vector>

#include <mdns/minimal/RecordData.h>
#include <mdns/minimal/Server.h>
#include <mdns/minimal/records/Ptr.h>
#include <mdns/minimal/records/Srv.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemMutex.h>

#include <nlunit-test.h>

namespace mdns {
namespace Minimal {
namespace test {

class CheckOnlyServer : public ServerBase, public ParserDelegate
{
public:
    CheckOnlyServer(nlTestSuite * inSuite) : ServerBase(nullptr, 0), mInSuite(inSuite) { Reset(); }
    CheckOnlyServer() : ServerBase(nullptr, 0), mInSuite(nullptr) { Reset(); }
    ~CheckOnlyServer() {}

    void OnHeader(ConstHeaderRef & header) override
    {
        if (mInSuite != nullptr)
        {
            NL_TEST_ASSERT(mInSuite, header.GetFlags().IsResponse());
            NL_TEST_ASSERT(mInSuite, header.GetFlags().IsValidMdns());
            mTotalRecords += header.GetAnswerCount() + header.GetAdditionalCount();
        }
        if (!header.GetFlags().IsTruncated())
        {
            NL_TEST_ASSERT(mInSuite, mTotalRecords == GetNumExpectedRecords());
            mHeaderFound = true;
        }
    }

    void OnResource(ResourceType type, const ResourceData & data) override
    {
        bool recordIsExpected = false;
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (mExpectedRecord[i] == nullptr || mFoundRecord[i] == true)
            {
                continue;
            }
            // For now, types and names are sufficient for checking that the response sender is sending out the correct records.
            if (data.GetType() == mExpectedRecord[i]->GetType() && data.GetName() == mExpectedRecord[i]->GetName())
            {
                if (data.GetType() == QType::PTR)
                {
                    // Check that the internal values are the same
                    SerializedQNameIterator dataTarget;
                    ParsePtrRecord(data.GetData(), data.GetData(), &dataTarget);
                    const PtrResourceRecord * expectedPtr = static_cast<const PtrResourceRecord *>(mExpectedRecord[i]);
                    if (dataTarget == expectedPtr->GetPtr())
                    {
                        mFoundRecord[i]  = true;
                        recordIsExpected = true;
                        break;
                    }
                }
                else if (data.GetType() == QType::SRV)
                {
                    SrvRecord srv;
                    bool srvParseOk = srv.Parse(data.GetData(), data.GetData());
                    if (mInSuite != nullptr)
                    {
                        NL_TEST_ASSERT(mInSuite, srvParseOk == true);
                    }
                    const SrvResourceRecord * expectedSrv = static_cast<const SrvResourceRecord *>(mExpectedRecord[i]);
                    if (srvParseOk && srv.GetName() == expectedSrv->GetServerName() && srv.GetPort() == expectedSrv->GetPort())
                    {
                        mFoundRecord[i]  = true;
                        recordIsExpected = true;
                        break;
                    }
                }
                else
                {
                    mFoundRecord[i]  = true;
                    recordIsExpected = true;
                    break;
                }
            }
        }
        if (mInSuite != nullptr)
        {
            NL_TEST_ASSERT(mInSuite, recordIsExpected);
        }
    }

    void OnQuery(const QueryData & data) override {}

    CHIP_ERROR
    DirectSend(chip::System::PacketBufferHandle && data, const chip::Inet::IPAddress & addr, uint16_t port,
               chip::Inet::InterfaceId interface) override
    {
        ParsePacket(BytesRange(data->Start(), data->Start() + data->TotalLength()), this);
        if (mHeaderFound)
        {
            TestGotAllExpectedPackets();
        }
        mSendCalled = true;
        return CHIP_NO_ERROR;
    }

    void AddExpectedRecord(ResourceRecord * record)
    {
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (mExpectedRecord[i] == nullptr)
            {
                mExpectedRecord[i] = record;
                mFoundRecord[i]    = false;
                return;
            }
        }
    }
    bool GetSendCalled() { return mSendCalled; }
    bool GetHeaderFound() { return mHeaderFound; }
    void SetTestSuite(nlTestSuite * suite) { mInSuite = suite; }
    void Reset()
    {
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            mExpectedRecord[i] = nullptr;
            mFoundRecord[i]    = false;
        }
        mHeaderFound  = false;
        mSendCalled   = false;
        mTotalRecords = 0;
    }

private:
    nlTestSuite * mInSuite;
    static constexpr size_t kMaxExpectedRecords           = 10;
    ResourceRecord * mExpectedRecord[kMaxExpectedRecords] = {};
    bool mFoundRecord[kMaxExpectedRecords];
    bool mHeaderFound = false;
    bool mSendCalled  = false;
    int mTotalRecords = 0;

    int GetNumExpectedRecords()
    {
        int num = 0;
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (mExpectedRecord[i] != nullptr)
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
        for (size_t i = 0; i < kMaxExpectedRecords; ++i)
        {
            if (mExpectedRecord[i] != nullptr)
            {
                NL_TEST_ASSERT(mInSuite, mFoundRecord[i] == true);
            }
        }
    }
};

} // namespace test
} // namespace Minimal
} // namespace mdns