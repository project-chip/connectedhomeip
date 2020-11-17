#include <nlunit-test.h>

extern void TestServicePool(nlTestSuite * suite, void * context);

static const nlTest sTests[] = { NL_TEST_DEF("TestServicePool", TestServicePool), NL_TEST_SENTINEL() };

int main()
{
    nlTestSuite theSuite = { "ServicePool", &sTests[0], nullptr, nullptr };
    nlTestRunner(&theSuite, nullptr);

    return nlTestRunnerStats(&theSuite);
}
