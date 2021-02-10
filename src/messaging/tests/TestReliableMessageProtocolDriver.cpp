/* See Project CHIP LICENSE file for licensing information. */


/**
 *    @file
 *      This file implements a standalone/native program executable test driver
 *      for the CHIP core library CHIP ReliableMessageProtocol tests.
 *
 */

#include "TestMessagingLayer.h"

#include <nlunit-test.h>

int main()
{
    // Generate machine-readable, comma-separated value (CSV) output.
    nlTestSetOutputStyle(OUTPUT_CSV);

    return (TestReliableMessageProtocol());
}
