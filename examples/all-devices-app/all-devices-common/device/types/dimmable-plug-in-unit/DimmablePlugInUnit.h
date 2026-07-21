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

#include <device/capabilities/dimmable-load/impl/LoggingDimmableLoad.h>

namespace chip::app {

class DimmablePlugInUnit : public LoggingDimmableLoad
{
public:
    DimmablePlugInUnit(const Context & context, const Config & config = {});
    ~DimmablePlugInUnit() override = default;
};

} // namespace chip::app
