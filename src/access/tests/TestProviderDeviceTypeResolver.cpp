/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <access/ProviderDeviceTypeResolver.h>
#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>
#include <pw_unit_test/framework.h>

namespace chip {
namespace Test {

using chip::Access::ProviderDeviceTypeResolver;
using chip::app::DataModel::DeviceTypeEntry;
using chip::app::DataModel::Provider;

/* ---------------------------------------------------------------------------
 * Fake Data‑Model provider
 * --------------------------------------------------------------------------*/
class FakeProvider : public Provider
{
public:
    void SetDeviceTypes(EndpointId ep, chip::span<const DeviceTypeEntry> list) { mEntries[ep] = list; }

    CHIP_ERROR DeviceTypes(EndpointId endpoint, ReadOnlyBufferBuilder<DeviceTypeEntry> & out) override
    {
        for (auto & e : mEntries[endpoint])
        {
            out.PushBack(e);
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ProcessNullableField(app::ConcreteAttributePath const &) override { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

private:
    chip::span<const DeviceTypeEntry> mEntries[5]{};
};

static FakeProvider gProvider;
static Provider * GetProvider()
{
    return &gProvider;
}

static DeviceTypeEntry MakeEntry(DeviceTypeId id)
{
    DeviceTypeEntry e;
    e.deviceTypeId = id;
    e.version      = 1;
    return e;
}

class ProviderDeviceTypeResolverTest : public ::pw::unit_test::Test
{
protected:
    ProviderDeviceTypeResolverTest() : mResolver(&GetProvider) {}

    void SetUp() override
    {
        for (auto & span : mStorage)
            span = {};
        memset(mBuf, 0, sizeof(mBuf));
    }

    chip::span<const DeviceTypeEntry> MakeSpan(std::initializer_list<DeviceTypeId> ids, size_t ep)
    {
        DeviceTypeEntry * dst = &mBuf[ep][0];
        size_t idx            = 0;
        for (auto id : ids)
        {
            dst[idx++] = MakeEntry(id);
        }
        mStorage[ep] = chip::Span<const DeviceTypeEntry>(dst, idx);
        return mStorage[ep];
    }

    ProviderDeviceTypeResolver mResolver;
    DeviceTypeEntry mBuf[5][4];
    chip::Span<const DeviceTypeEntry> mStorage[5];
};

TEST_F(ProviderDeviceTypeResolverTest, ReturnsTrueWhenTypeFound)
{
    constexpr EndpointId kEp            = 1;
    constexpr DeviceTypeId kDim         = 0x0100; // Dimmer
    constexpr DeviceTypeId kOnOffSwitch = 0x0000;

    gProvider.SetDeviceTypes(kEp, MakeSpan({ kOnOffSwitch, kDim }, kEp));
    EXPECT_TRUE(mResolver.IsDeviceTypeOnEndpoint(kDim, kEp));
}

TEST_F(ProviderDeviceTypeResolverTest, ReturnsFalseWhenTypeOnDifferentEndpoint)
{
    constexpr DeviceTypeId kSensor = 0x0302;
    gProvider.SetDeviceTypes(/*ep=*/2, MakeSpan({ kSensor }, 2));
    EXPECT_FALSE(mResolver.IsDeviceTypeOnEndpoint(kSensor, /*query ep=*/1));
}

TEST_F(ProviderDeviceTypeResolverTest, ReturnsFalseWhenTypeNotPresent)
{
    constexpr EndpointId kEp     = 0;
    constexpr DeviceTypeId kLock = 0x000A;
    gProvider.SetDeviceTypes(kEp, MakeSpan({ /*empty list*/ }, kEp));
    EXPECT_FALSE(mResolver.IsDeviceTypeOnEndpoint(kLock, kEp));
}

} // namespace Test
} // namespace chip
