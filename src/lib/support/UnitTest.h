#pragma once

/*
 * Run fixture's class function as a test.
 */
#define TEST_F_FROM_FIXTURE(test_fixture, test_name)                                                                               \
    TEST_F(test_fixture, test_name)                                                                                                \
    {                                                                                                                              \
        test_name();                                                                                                               \
    }                                                                                                                              \
    void test_fixture::test_name()

namespace chip {
namespace test {

int RunAllTests();

} // namespace test
} // namespace chip
