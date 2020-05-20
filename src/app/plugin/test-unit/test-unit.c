#include "test-unit.h"

void main(int argc, char ** argv)
{
    testClusterServerBasic();
    testClusterServerIdentify();
    testClusterServerLevelControl();
    testClusterServerOnOff();
    testCodecSimple();
    testCoreDataModel();
    testCoreMessageDispatch();
}