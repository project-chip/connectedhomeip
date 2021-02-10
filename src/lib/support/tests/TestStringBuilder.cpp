/* See Project CHIP LICENSE file for licensing information. */

#include <support/StringBuilder.h>
#include <support/UnitTestRegistration.h>

#include <nlunit-test.h>

namespace {

using namespace chip;

void TestStringBuilder(nlTestSuite * inSuite, void * inContext)
{

    StringBuilder<64> builder;

    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "") == 0);

    builder.Add("foo");
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foo") == 0);

    builder.Add("bar");
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foobar") == 0);
}

void TestIntegerAppend(nlTestSuite * inSuite, void * inContext)
{

    StringBuilder<64> builder;

    builder.Add("nr: ").Add(1234);
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "nr: 1234") == 0);

    builder.Add(", ").Add(-22);
    NL_TEST_ASSERT(inSuite, builder.Fit());
    NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "nr: 1234, -22") == 0);
}

void TestOverflow(nlTestSuite * inSuite, void * inContext)
{

    {
        StringBuilder<4> builder;

        builder.Add("foo");
        NL_TEST_ASSERT(inSuite, builder.Fit());
        NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foo") == 0);

        builder.Add("bar");
        NL_TEST_ASSERT(inSuite, !builder.Fit());
        NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "foo") == 0);
    }

    {
        StringBuilder<7> builder;

        builder.Add("x: ").Add(12345);
        NL_TEST_ASSERT(inSuite, !builder.Fit());
        NL_TEST_ASSERT(inSuite, strcmp(builder.c_str(), "x: 123") == 0);
    }
}

const nlTest sTests[] = {
    NL_TEST_DEF("TestStringBuilder", TestStringBuilder), //
    NL_TEST_DEF("TestIntegerAppend", TestIntegerAppend), //
    NL_TEST_DEF("TestOverflow", TestOverflow),           //
    NL_TEST_SENTINEL()                                   //
};

} // namespace

int TestStringBuilder(void)
{
    nlTestSuite theSuite = { "StringBuilder", sTests, nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);
    return nlTestRunnerStats(&theSuite);
}

CHIP_REGISTER_TEST_SUITE(TestStringBuilder)
