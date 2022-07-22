/*
 *
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

/**
 *    @file
 *      This file defines and implements some utlities for generating persistent storage keys
 *
 */

#pragma once

#include <lib/support/CodeUtils.h>

namespace chip {

// This macro generates a key for storage using a node ID and a key prefix, and performs the given action
// on that key.
#define PERSISTENT_KEY_OP(node, keyPrefix, key, action)                                                                            \
    do                                                                                                                             \
    {                                                                                                                              \
        constexpr size_t len = std::extent<decltype(keyPrefix)>::value;                                                            \
        nlSTATIC_ASSERT_PRINT(len > 0, "keyPrefix length must be known at compile time");                                          \
        /* 2 * sizeof(chip::NodeId) to accommodate 2 character for each byte in Node Id */                                         \
        char key[len + 2 * sizeof(chip::NodeId) + 1];                                                                              \
        nlSTATIC_ASSERT_PRINT(sizeof(node) <= sizeof(uint64_t), "Node ID size is greater than expected");                          \
        snprintf(key, sizeof(key), "%s%" PRIx64, keyPrefix, node);                                                                 \
        action;                                                                                                                    \
    } while (0)

} // namespace chip
