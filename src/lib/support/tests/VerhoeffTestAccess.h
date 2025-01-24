/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/support/verhoeff/Verhoeff.h>

/**
 * @brief Class acts as an accessor to private methods of the Verhoeff class without needing to give friend access to
 *        each individual test.
 */
class VerhoeffTestAccess
{
public:
    VerhoeffTestAccess() = delete;

    static const uint8_t * GetPermuteTable() { return Verhoeff10::sPermTable; }

    static const uint8_t * GetMultiplyTable() { return Verhoeff10::sMultiplyTable; }
};
