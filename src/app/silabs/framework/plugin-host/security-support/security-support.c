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
 * @brief Provides support features for security, such as random number generation.
 *******************************************************************************
   ******************************************************************************/

#include "app/framework/include/af.h"
#include "tool/random/random-number-generation.h"

EmberStatus emberAfGenerateRandomKey(EmberKeyData* result)
{
  if (platformRandomDataFunction(emberKeyContents(result),
                                 EMBER_ENCRYPTION_KEY_SIZE)) {
    return EMBER_INSUFFICIENT_RANDOM_DATA;
  }
  return EMBER_SUCCESS;
}

EmberStatus emberAfGenerateRandomData(uint8_t* result, uint8_t size)
{
  if (platformRandomDataFunction(result, size)) {
    return EMBER_INSUFFICIENT_RANDOM_DATA;
  }
  return EMBER_SUCCESS;
}
