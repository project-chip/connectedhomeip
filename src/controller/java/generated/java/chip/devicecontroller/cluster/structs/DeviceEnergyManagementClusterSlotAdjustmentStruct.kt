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
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DeviceEnergyManagementClusterSlotAdjustmentStruct(
  val slotIndex: UInt,
  val nominalPower: Long,
  val duration: ULong
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterSlotAdjustmentStruct {\n")
    append("\tslotIndex : $slotIndex\n")
    append("\tnominalPower : $nominalPower\n")
    append("\tduration : $duration\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_SLOT_INDEX), slotIndex)
      put(ContextSpecificTag(TAG_NOMINAL_POWER), nominalPower)
      put(ContextSpecificTag(TAG_DURATION), duration)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SLOT_INDEX = 0
    private const val TAG_NOMINAL_POWER = 1
    private const val TAG_DURATION = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader
    ): DeviceEnergyManagementClusterSlotAdjustmentStruct {
      tlvReader.enterStructure(tlvTag)
      val slotIndex = tlvReader.getUInt(ContextSpecificTag(TAG_SLOT_INDEX))
      val nominalPower = tlvReader.getLong(ContextSpecificTag(TAG_NOMINAL_POWER))
      val duration = tlvReader.getULong(ContextSpecificTag(TAG_DURATION))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterSlotAdjustmentStruct(slotIndex, nominalPower, duration)
    }
  }
}
