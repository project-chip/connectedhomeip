#include <stdio.h>
#include <stdlib.h>

#include "test-unit.h"

int main(int argc, char ** argv)
{
    printf("%s: === start ===\n", argv[0]);
    testClusterServerBasic();
    testClusterServerIdentify();
    testClusterServerLevelControl();
    testClusterServerOnOff();
    testCodecSimple();
    testCoreDataModel();
    testCoreMessageDispatch();
    printf("%s: === end ===\n", argv[0]);
    return 0;
}