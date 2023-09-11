/*
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <messaging/tests/MessagingContext.h>

namespace chip {
namespace Test {

/**
 * @brief The context of test cases for messaging layer. It wil initialize network layer and system layer, and create
 *        two secure sessions, connected with each other. Exchanges can be created for each secure session.
 */
class AppContext : public LoopbackMessagingContext
{
    typedef LoopbackMessagingContext Super;

public:
    /// Initialize the underlying layers.
    CHIP_ERROR Init() override;

    // Shutdown all layers, finalize operations
    void Shutdown() override;
};

} // namespace Test
} // namespace chip
