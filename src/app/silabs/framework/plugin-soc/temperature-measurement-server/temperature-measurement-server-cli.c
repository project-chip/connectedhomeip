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
 * @brief CLI for the Temperature Measurement Server plugin.
 *******************************************************************************
   ******************************************************************************/

#ifdef EMBER_SCRIPTED_TEST
  #include "app/framework/plugin-soc/temperature-measurement-server/temperature-measurement-server-test.h"
#endif

#include "app/framework/include/af.h"
#include EMBER_AF_API_TEMPERATURE

//******************************************************************************
// CLI function to manually generate a temperature read
//******************************************************************************
void emAfTemperatureMeasurementServerReadCommand(void)
{
  halTemperatureStartRead();

  // No result will be printed here, as the callback that generates the value
  // is already implemented in temperature-measurement-server.c.  As such, the
  // callback will generate the diagnostic print.
}
