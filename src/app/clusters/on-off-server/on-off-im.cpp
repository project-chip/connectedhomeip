/**
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

/**
 * @file
 * @brief Placeholder routines for the On-Off plugin in interaction model.
 */

#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/MessageDef/MessageDef.h>
#include <app/im-encoder.h>
#include <app/util/basic-types.h>

#include <support/logging/CHIPLogging.h>

namespace chip {
namespace app {
namespace cluster {
namespace OnOff {

// TODO: Call handler in on-off.cpp here.
void HandleOffCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    // Simplily does nothing, should be replaced by actual logic.
}

void HandleOnCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    // Simplily does nothing, should be replaced by actual logic.
}

void HandleToggleCommandReceived(chip::TLV::TLVReader & aReader, chip::app::Command * apCommandObj)
{
    // Simplily does nothing, should be replaced by actual logic.
}

} // namespace OnOff
} // namespace cluster
} // namespace app
} // namespace chip
