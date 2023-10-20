/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

package matter.onboardingpayload

/**
 * Enum values for possible flows for out-of-box commissioning that a Matter device manufacturer may
 * select for a given product.
 */
enum class CommissioningFlow(val value: Int) {
  STANDARD(0), // Device automatically enters pairing mode upon power-up
  USER_ACTION_REQUIRED(1), // Device requires a user interaction to enter pairing mode
  CUSTOM(2) // Commissioning steps should be retrieved from the distributed compliance ledger
}
