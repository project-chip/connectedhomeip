#include "group_translator.hpp"

// Matter components
#include <lib/support/UnitTestContext.h>
#include <lib/support/UnitTestRegistration.h>

// Third party library
#include <nlunit-test.h>
// mocks
#include "MockMatterDataStorage.hpp"

using namespace unify::matter_bridge;
#define TEST_LOG_TAG "TestGroupTranslator"

void TestGroupTranslator(nlTestSuite * inSuite, void * aContext)
{
    Test::MockMatterDataStorage mock_data_storage;
    group_translator test_group_translator(mock_data_storage);
    group_translator::matter_group test_matter_group = { 1, 1 };

    NL_TEST_ASSERT(inSuite, test_group_translator.add_matter_group(test_matter_group));
    NL_TEST_ASSERT(inSuite, 1 == test_group_translator.get_unify_group(test_matter_group));
}

void TestGroupTranslatorUnifyGroupAdd(nlTestSuite * inSuite, void * aContext)
{
    Test::MockMatterDataStorage mock_data_storage;
    group_translator test_group_translator(mock_data_storage);
    // Assigned unify group 
    test_group_translator.register_unify_group(1);
    // Assign matter group
    group_translator::matter_group test_matter_group = { 1, 1 };
    NL_TEST_ASSERT(inSuite, test_group_translator.add_matter_group(test_matter_group));
    // Check the unify group incremented 
    NL_TEST_ASSERT(inSuite, 2 == test_group_translator.get_unify_group(test_matter_group));
}

class TestContext
{
public:
    nlTestSuite * mTestSuite;
};

/**
 *   Test Suite. It lists all the test functions.
 */
static const nlTest sTests[] = { NL_TEST_DEF("TestGroupTranslator", TestGroupTranslator),
                                 NL_TEST_DEF("TestGroupTranslatorUnifyGroupAdd", TestGroupTranslatorUnifyGroupAdd),
                                 NL_TEST_SENTINEL() };

static nlTestSuite kTheSuite = { "TestGroupTranslatorSuite", &sTests[0], nullptr, nullptr };

int TestGroupTranslatorSuite(void)
{
    return chip::ExecuteTestsWithContext<TestContext>(&kTheSuite);
}

CHIP_REGISTER_TEST_SUITE(TestGroupTranslatorSuite)
