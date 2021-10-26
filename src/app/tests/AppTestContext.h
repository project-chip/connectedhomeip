/*
 *    Copyright (c) 2021 Project CHIP Authors
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
#pragma once

#include <messaging/tests/MessagingContext.h>
#include <transport/raw/tests/NetworkTestHelpers.h>

#include <nlunit-test.h>

namespace chip {
namespace Test {

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class AppContext : public MessagingContext
{
public:
    /// Initialize the underlying layers and test suite pointer
    CHIP_ERROR Init();

    // Shutdown all layers, finalize operations
    CHIP_ERROR Shutdown();

    static int Initialize(void * context)
    {
        auto * ctx = static_cast<AppContext *>(context);
        return ctx->Init() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

    static int Finalize(void * context)
    {
        auto * ctx = static_cast<AppContext *>(context);
        return ctx->Shutdown() == CHIP_NO_ERROR ? SUCCESS : FAILURE;
    }

private:
    chip::TransportMgr<chip::Test::LoopbackTransport> mTransportManager;
    chip::Test::IOContext mIOContext;
};

} // namespace Test
} // namespace chip
