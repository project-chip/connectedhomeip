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
 *
 *    Copyright (c) 2020 Silicon Labs
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
/***************************************************************************//**
 * @file
 * @brief A header for AFV2 scripted unit tests.
 *******************************************************************************
   ******************************************************************************/

#include PLATFORM_HEADER

#include "stack/core/ember-stack.h"
#include "hal/hal.h"
#include "stack/core/parcel.h"
#include "stack/core/scripted-stub.h"

#include "../test-framework-network.h"

// -----------------------------------------------------------------------------
// MAGIC.

#define ACTION(name, params, ...)          \
  static ActionType name ## ActionType = { \
    # name "ActionType",                   \
    # params,                              \
    name ## ActionPrinter,                 \
    name ## ActionPerformer,               \
  }

#define PRINTER(name)   static void name ## ActionPrinter(Action * action)
#define PERFORMER(name) static void name ## ActionPerformer(Action * action)

// -----------------------------------------------------------------------------
// CALLBACKS.

void scriptTickCallback(void);
