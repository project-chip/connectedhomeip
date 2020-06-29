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
#ifndef NETWORK_TEST_HELPERS_H_
#define NETWORK_TEST_HELPERS_H_

#include <functional>

#include <core/CHIPError.h>
#include <inet/InetLayer.h>
#include <system/SystemLayer.h>

#include <nlbyteorder.h>
#include <nlunit-test.h>

namespace chip {
namespace Test {

class IOContext
{
public:
    IOContext() {}

    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init(nlTestSuite * suite);

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    /// Perform a single short IO Loop
    void DriveIO();

    /// DriveIO until the specified number of milliseconds has passed or until
    /// completionFunction returns true
    void DriveIOUntil(unsigned maxWaitMs, std::function<bool(void)> completionFunction);

    nlTestSuite * GetTestSuite() { return mSuite; }
    System::Layer & GetSystemLayer() { return *mSystemLayer; }
    Inet::InetLayer & GetInetLayer() { return *mInetLayer; }

private:
    nlTestSuite * mSuite         = nullptr;
    System::Layer * mSystemLayer = nullptr;
    Inet::InetLayer * mInetLayer = nullptr;
};

} // namespace Test
} // namespace chip

#endif // NETWORK_TEST_HELPERS_H_
