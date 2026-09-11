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

#include "NamedPipeTranslators.h"
#include <device/types/robotic-vacuum-cleaner/RoboticVacuumCleaner.h>
#include <device/types/robotic-vacuum-cleaner/impl/SimulatedRoboticVacuumCleaner.h>
#include <posix/named_pipe/translators/RvcTranslator.h>

namespace chip::app {

void RegisterNamedPipeTranslators(RoboticVacuumCleaner & device, NamedPipe::Dispatcher & dispatcher)
{
    LogErrorOnFailure(dispatcher.EnsureTranslatorRegistered<NamedPipe::RvcTranslator>());
}

void RegisterNamedPipeTranslators(SimulatedRoboticVacuumCleaner & device, NamedPipe::Dispatcher & dispatcher)
{
    LogErrorOnFailure(dispatcher.EnsureTranslatorRegistered<NamedPipe::RvcTranslator>());
}

} // namespace chip::app
