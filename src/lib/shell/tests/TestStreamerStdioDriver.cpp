/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file implements a standalone/native program executable
 *      test driver for the CHIP system layer library timer unit
 *      tests.
 *
 */

#include "TestShell.h"

int main(int argc, char * argv[])
{
    // Generate machine-readable, comma-separated value (CSV) output.
    nlTestSetOutputStyle(OUTPUT_CSV);

    return (TestStreamerStdio());
}
