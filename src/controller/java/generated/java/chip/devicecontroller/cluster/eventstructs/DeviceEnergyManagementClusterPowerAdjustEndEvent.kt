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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DeviceEnergyManagementClusterPowerAdjustEndEvent(
  val cause: UInt,
  val duration: ULong,
  val energyUse: Long,
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterPowerAdjustEndEvent {\n")
    append("\tcause : $cause\n")
    append("\tduration : $duration\n")
    append("\tenergyUse : $energyUse\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CAUSE), cause)
      put(ContextSpecificTag(TAG_DURATION), duration)
      put(ContextSpecificTag(TAG_ENERGY_USE), energyUse)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CAUSE = 0
    private const val TAG_DURATION = 1
    private const val TAG_ENERGY_USE = 2

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): DeviceEnergyManagementClusterPowerAdjustEndEvent {
      tlvReader.enterStructure(tlvTag)
      val cause = tlvReader.getUInt(ContextSpecificTag(TAG_CAUSE))
      val duration = tlvReader.getULong(ContextSpecificTag(TAG_DURATION))
      val energyUse = tlvReader.getLong(ContextSpecificTag(TAG_ENERGY_USE))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterPowerAdjustEndEvent(cause, duration, energyUse)
    }
  }
}
