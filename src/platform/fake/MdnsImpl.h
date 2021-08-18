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
#include "lib/mdns/platform/Mdns.h"
#include <support/CHIPMemString.h>

namespace chip {
namespace Mdns {

namespace test {
enum class CallType
{
    kUnknown,
    kStart,
    kStop,
};

struct ExpectedTxt
{
    static constexpr size_t kMaxExpectedKeySize = 3;
    static constexpr size_t kMaxExpectedValSize = 128;
    char key[kMaxExpectedKeySize + 1]           = "";
    char value[kMaxExpectedValSize + 1]         = "";
    bool operator==(const TextEntry & txt) const
    {
        if (strlen(txt.mKey) > kMaxExpectedKeySize || strcmp(txt.mKey, key) != 0)
        {
            return false;
        }
        if (strlen(value) != txt.mDataSize || memcmp(value, txt.mData, txt.mDataSize) != 0)
        {
            return false;
        }
        return true;
    }
};

struct ExpectedSubtype
{
    char name[kMaxSubtypeDescSize] = "";
    bool operator==(const char * subtype) const { return strcmp(name, subtype) == 0; }
};
struct ExpectedCall
{
    ExpectedCall & AddTxt(const char * key, const char * val)
    {
        if (numTxt < kMaxTxtRecords)
        {
            Platform::CopyString(txt[numTxt].key, key);
            Platform::CopyString(txt[numTxt].value, val);
            numTxt++;
        }
        return *this;
    }
    ExpectedCall & AddSubtype(const char * sub)
    {
        if (numSubtypes < kMaxSubtypes)
        {

            Platform::CopyString(subtype[numSubtypes++].name, sub);
        }
        return *this;
    }
    ExpectedCall & SetServiceName(const char * name)
    {
        Platform::CopyString(serviceName, name);
        return *this;
    }
    ExpectedCall & SetInstanceName(const char * name)
    {
        Platform::CopyString(instanceName, name);
        return *this;
    }
    ExpectedCall & SetHostName(const char * name)
    {
        Platform::CopyString(hostName, name);
        return *this;
    }
    ExpectedCall & SetProtocol(MdnsServiceProtocol prot)
    {
        protocol = prot;
        return *this;
    }
    bool TxtIsExpected(const TextEntry & entry) const
    {
        for (size_t i = 0; i < numTxt; ++i)
        {
            if (txt[i] == entry)
            {
                return true;
            }
        }
        return false;
    }
    bool SubtypeIsExpected(const char * sub) const
    {
        for (size_t i = 0; i < numSubtypes; ++i)
        {
            if (subtype[i] == sub)
            {
                return true;
            }
        }
        return false;
    }

    bool CheckMatch(CallType call, const MdnsService * service) const
    {
        bool callOk       = call == callType;
        bool instanceOk   = strcmp(instanceName, service->mName) == 0;
        bool hostOk       = strcmp(hostName, service->mHostName) == 0;
        bool serviceOk    = strcmp(serviceName, service->mType) == 0;
        bool subtypeNumOk = numSubtypes == service->mSubTypeSize;
        bool txtNumOk     = numTxt == service->mTextEntrySize;
        if (!callOk || !instanceOk || !hostOk || !serviceOk || !subtypeNumOk || !txtNumOk)
        {
            return false;
        }
        // There aren't a lot of these, so just double loop for now.
        // The orders of the txt and subtypes don't necessarily match.
        for (size_t i = 0; i < service->mSubTypeSize; ++i)
        {
            if (!SubtypeIsExpected(service->mSubTypes[i]))
            {
                return false;
            }
        }
        for (size_t i = 0; i < service->mTextEntrySize; ++i)
        {
            if (!TxtIsExpected(service->mTextEntries[i]))
            {
                return false;
            }
        }
        return true;
    }

    static constexpr size_t kMaxTxtRecords          = 10;
    static constexpr size_t kMaxSubtypes            = 10;
    CallType callType                               = CallType::kUnknown;
    MdnsServiceProtocol protocol                    = MdnsServiceProtocol::kMdnsProtocolUnknown;
    char instanceName[kMdnsInstanceNameMaxSize + 1] = "";
    char hostName[kMdnsHostNameMaxSize + 1]         = "";
    char serviceName[kMdnsTypeMaxSize + 1]          = "";
    ExpectedSubtype subtype[kMaxSubtypes];
    size_t numSubtypes = 0;
    ExpectedTxt txt[kMaxTxtRecords];
    size_t numTxt = 0;

    bool IsValid() const { return callType != CallType::kUnknown && protocol != MdnsServiceProtocol::kMdnsProtocolUnknown; }
};

CHIP_ERROR AddExpectedCall(const ExpectedCall & call);
void Reset();

} // namespace test

} // namespace Mdns
} // namespace chip
