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

class DeviceEnergyManagementClusterPowerRangeAdjustStartEvent(
  val adjustment:
    chip.devicecontroller.cluster.structs.DeviceEnergyManagementClusterPowerRangeAdjustStruct,
  val duration: ULong,
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterPowerRangeAdjustStartEvent {\n")
    append("\tadjustment : $adjustment\n")
    append("\tduration : $duration\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      adjustment.toTlv(ContextSpecificTag(TAG_ADJUSTMENT), this)
      put(ContextSpecificTag(TAG_DURATION), duration)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ADJUSTMENT = 0
    private const val TAG_DURATION = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): DeviceEnergyManagementClusterPowerRangeAdjustStartEvent {
      tlvReader.enterStructure(tlvTag)
      val adjustment =
        chip.devicecontroller.cluster.structs.DeviceEnergyManagementClusterPowerRangeAdjustStruct
          .fromTlv(ContextSpecificTag(TAG_ADJUSTMENT), tlvReader)
      val duration = tlvReader.getULong(ContextSpecificTag(TAG_DURATION))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterPowerRangeAdjustStartEvent(adjustment, duration)
    }
  }
}
