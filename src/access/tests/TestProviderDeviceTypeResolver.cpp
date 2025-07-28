/*
 *   ProviderDeviceTypeResolver tests
 */

#include <access/ProviderDeviceTypeResolver.h>
#include <pw_unit_test/framework.h>

#include <lib/core/CHIPError.h>
#include <lib/support/Span.h>

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
    // Configure per‑endpoint lists from the test body.
    void SetDeviceTypes(EndpointId ep, chip::span<const DeviceTypeEntry> list)
    {
        VerifyOrDie(ep < kMaxEndpoints);
        mEntries[ep] = list;
    }

    // -------- Provider interface we actually care about ----------
    CHIP_ERROR DeviceTypes(EndpointId endpoint, ReadOnlyBufferBuilder<DeviceTypeEntry> & out) override
    {
        VerifyOrDie(endpoint < kMaxEndpoints);
        // Copy the span into the ReadOnlyBufferBuilder
        for (auto & e : mEntries[endpoint])
        {
            ReturnErrorOnFailure(out.PushBack(e));
        }
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR ProcessNullableField(app::ConcreteAttributePath const &) override { return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE; }

private:
    static constexpr size_t kMaxEndpoints = 10;
    // store spans owned by the test
    chip::span<const DeviceTypeEntry> mEntries[kMaxEndpoints]{};
};

/* Static getter that AccessControl will call */
static FakeProvider gProvider;
static Provider * GetProvider()
{
    return &gProvider;
}

/* ---------------------------------------------------------------------------
 *  Helper to make DeviceTypeEntry in readable form
 * --------------------------------------------------------------------------*/
static DeviceTypeEntry MakeEntry(DeviceTypeId id)
{
    DeviceTypeEntry e;
    e.deviceTypeId = id;
    e.version      = 1;
    return e;
}

/* ---------------------------------------------------------------------------
 *  The tests
 * --------------------------------------------------------------------------*/
class TestProviderDeviceTypeResolver : public ::testing::Test
{
protected:
    TestProviderDeviceTypeResolver() : mResolver(&GetProvider) {}

    void SetUp() override
    {
        // Clean provider state every test
        for (auto & span : mStorage)
            span = {};
        memset(mBuf, 0, sizeof(mBuf));
    }

    /* Provide a span backed by the fixed buffer so lifetime is OK */
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

    // one small buffer per endpoint – avoids dynamic allocation
    DeviceTypeEntry mBuf[5][4]; // 5 EPs × up to 4 types
    chip::Span<const DeviceTypeEntry> mStorage[5];
};

/* ---------- happy path: the type is present --------------------------------*/
TEST_F(TestProviderDeviceTypeResolver, ReturnsTrueWhenTypeFound)
{
    constexpr EndpointId kEp            = 1;
    constexpr DeviceTypeId kDim         = 0x0100; // Dimmer
    constexpr DeviceTypeId kOnOffSwitch = 0x0000;

    gProvider.SetDeviceTypes(kEp, MakeSpan({ kOnOffSwitch, kDim }, kEp));

    EXPECT_TRUE(mResolver.IsDeviceTypeOnEndpoint(kDim, kEp));
}

/* ---------- negative path: wrong endpoint ---------------------------------*/
TEST_F(TestProviderDeviceTypeResolver, ReturnsFalseWhenTypeOnDifferentEndpoint)
{
    constexpr DeviceTypeId kSensor = 0x0302;
    gProvider.SetDeviceTypes(/*ep=*/2, MakeSpan({ kSensor }, 2));

    EXPECT_FALSE(mResolver.IsDeviceTypeOnEndpoint(kSensor, /*query ep=*/1));
}

/* ---------- negative path: type absent ------------------------------------*/
TEST_F(TestProviderDeviceTypeResolver, ReturnsFalseWhenTypeNotPresent)
{
    constexpr EndpointId kEp     = 0;
    constexpr DeviceTypeId kLock = 0x000A;

    gProvider.SetDeviceTypes(kEp, MakeSpan({ /*empty list*/ }, kEp));

    EXPECT_FALSE(mResolver.IsDeviceTypeOnEndpoint(kLock, kEp));
}

} // namespace Test
} // namespace chip
