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

#include <app/data-model-interface/ActionContext.h>
#include <app/data-model-interface/DataModelChangeListener.h>
#include <app/data-model-interface/EventsGenerator.h>

namespace chip {
namespace app {
namespace InteractionModel {

/// Data provided to data models in order to interface with the interaction model environment.
///
/// Provides callback-style functionality to notify the interaction model of changes
/// (e.g. using paths to notify of attribute data changes or events to generate events)
/// as well as fetching current state (via actionContext)
struct InteractionModelContext
{
    EventsGenerator * eventsGenerator;
    DataModelChangeListener * dataModelChangeListener;
    ActionContext * actionContext;
};

} // namespace InteractionModel
} // namespace app
} // namespace chip
