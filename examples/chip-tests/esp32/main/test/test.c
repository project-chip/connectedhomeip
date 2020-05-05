#include <stdio.h>
#include <string.h>
#include "unity.h"

extern "C" int test_main(void);

TEST_CASE("call test main", "[Main Test]")
{
    TEST_ASSERT_EQUAL(0, test_main());
}
