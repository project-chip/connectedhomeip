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
 * @brief Definitions for the Security Sensor plugin.
 *******************************************************************************
   ******************************************************************************/

#ifndef SILABS_SECURITY_SENSOR_H
#define SILABS_SECURITY_SENSOR_H

// Status bit definitions used when generating report to IAS Zone Server
#define EMBER_SECURITY_SENSOR_STATUS_ALARM     0x0001
#define EMBER_SECURITY_SENSOR_STATUS_NO_ALARM  0x0000
#define EMBER_SECURITY_SENSOR_STATUS_TAMPER    0x0004
#define EMBER_SECURITY_SENSOR_STATUS_NO_TAMPER 0x0000

#endif //SILABS_SECURITY_SENSOR_H
