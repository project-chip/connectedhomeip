#include "TestBDX.h"

#include <nlunit-test.h>

int main()
{
    nlTestSetOutputStyle(OUTPUT_CSV);

    return (TestBdxMessages());
}
