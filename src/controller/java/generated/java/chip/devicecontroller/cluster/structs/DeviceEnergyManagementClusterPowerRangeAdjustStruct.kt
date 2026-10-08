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

class DeviceEnergyManagementClusterPowerRangeAdjustStruct(
  val minPower: Long?,
  val maxPower: Long?,
  val cause: UInt,
  val endTime: ULong,
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterPowerRangeAdjustStruct {\n")
    append("\tminPower : $minPower\n")
    append("\tmaxPower : $maxPower\n")
    append("\tcause : $cause\n")
    append("\tendTime : $endTime\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (minPower != null) {
        put(ContextSpecificTag(TAG_MIN_POWER), minPower)
      } else {
        putNull(ContextSpecificTag(TAG_MIN_POWER))
      }
      if (maxPower != null) {
        put(ContextSpecificTag(TAG_MAX_POWER), maxPower)
      } else {
        putNull(ContextSpecificTag(TAG_MAX_POWER))
      }
      put(ContextSpecificTag(TAG_CAUSE), cause)
      put(ContextSpecificTag(TAG_END_TIME), endTime)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MIN_POWER = 0
    private const val TAG_MAX_POWER = 1
    private const val TAG_CAUSE = 2
    private const val TAG_END_TIME = 3

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): DeviceEnergyManagementClusterPowerRangeAdjustStruct {
      tlvReader.enterStructure(tlvTag)
      val minPower =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_MIN_POWER))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MIN_POWER))
          null
        }
      val maxPower =
        if (!tlvReader.isNull()) {
          tlvReader.getLong(ContextSpecificTag(TAG_MAX_POWER))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MAX_POWER))
          null
        }
      val cause = tlvReader.getUInt(ContextSpecificTag(TAG_CAUSE))
      val endTime = tlvReader.getULong(ContextSpecificTag(TAG_END_TIME))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterPowerRangeAdjustStruct(minPower, maxPower, cause, endTime)
    }
  }
}
