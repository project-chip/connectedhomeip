/*
 *
 *    <COPYRIGHT>
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
#ifndef BLEUUID_H_
#define BLEUUID_H_

#include <stdint.h>

namespace chip {
namespace Ble {

// Type to represent 128-bit BLE UUIDs. 16-bit short UUIDs may be combined with
// the Bluetooth Base UUID to form full 128-bit UUIDs as described in the
// Service Discovery Protocol (SDP) definition, part of the Bluetooth Core
// Specification.
typedef struct
{
    uint8_t bytes[16];
} ChipBleUUID;

// UUID of CHIP BLE service. Exposed for use in scan filter.
extern const ChipBleUUID CHIP_BLE_SVC_ID;

bool UUIDsMatch(const ChipBleUUID * idOne, const ChipBleUUID * idTwo);

} /* namespace Ble */
} /* namespace chip */

#endif /* BLEUUID_H_ */
