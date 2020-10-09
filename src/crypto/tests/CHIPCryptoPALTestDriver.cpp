/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
