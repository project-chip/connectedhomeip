/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/closure/impl/LoggingClosure.h>

#include <vector>

namespace chip::app {

/// Feature selection and panel composition of a closure, as handed to LoggingClosure.
struct SampleClosure
{
    Closure::Config closure;
    std::vector<PanelList> panels;
};

/// Builds the closure exposed by `--device closure`: a door that positions, latches and moves at a
/// selectable speed, composed of a translating lift panel, a flow-modulating panel and a latching
/// rotating panel. The semantic tags it references have static storage duration.
SampleClosure MakeSampleClosure();

} // namespace chip::app
