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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DeviceEnergyManagementClusterPowerAdjustStruct(
  val minPower: Long,
  val maxPower: Long,
  val minDuration: UInt,
  val maxDuration: UInt
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterPowerAdjustStruct {\n")
    append("\tminPower : $minPower\n")
    append("\tmaxPower : $maxPower\n")
    append("\tminDuration : $minDuration\n")
    append("\tmaxDuration : $maxDuration\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_MIN_POWER), minPower)
      put(ContextSpecificTag(TAG_MAX_POWER), maxPower)
      put(ContextSpecificTag(TAG_MIN_DURATION), minDuration)
      put(ContextSpecificTag(TAG_MAX_DURATION), maxDuration)
      endStructure()
    }
  }

  companion object {
    private const val TAG_MIN_POWER = 0
    private const val TAG_MAX_POWER = 1
    private const val TAG_MIN_DURATION = 2
    private const val TAG_MAX_DURATION = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DeviceEnergyManagementClusterPowerAdjustStruct {
      tlvReader.enterStructure(tlvTag)
      val minPower = tlvReader.getLong(ContextSpecificTag(TAG_MIN_POWER))
      val maxPower = tlvReader.getLong(ContextSpecificTag(TAG_MAX_POWER))
      val minDuration = tlvReader.getUInt(ContextSpecificTag(TAG_MIN_DURATION))
      val maxDuration = tlvReader.getUInt(ContextSpecificTag(TAG_MAX_DURATION))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterPowerAdjustStruct(
        minPower,
        maxPower,
        minDuration,
        maxDuration
      )
    }
  }
}
