/* See Project CHIP LICENSE file for licensing information. */

/**
 *    @file
 *      This file implements a standalone/native program executable
 *      test driver for the CHIP system layer library error string
 *      library unit tests.
 *
 */

#include "TestCryptoLayer.h"

#include <nlunit-test.h>

int main()
{
    // Generate machine-readable, comma-separated value (CSV) output.
    nlTestSetOutputStyle(OUTPUT_CSV);

    return (TestCHIPCryptoPAL());
}
