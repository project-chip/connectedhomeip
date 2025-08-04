/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
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

#include <lib/support/Span.h>
#include <protocols/interaction_model/StatusCode.h>

#include <variant>

namespace chip {
namespace Test {

/// specify what the next `emAfReadOrWriteAttribute` will contain
///
/// It may return a value with success or some error. The byte span WILL BE COPIED.
void SetEmberReadOutput(std::variant<chip::ByteSpan, chip::Protocols::InteractionModel::Status> what);

/// Grab the data currently in the buffer
chip::ByteSpan GetEmberBuffer();

} // namespace Test
} // namespace chip
