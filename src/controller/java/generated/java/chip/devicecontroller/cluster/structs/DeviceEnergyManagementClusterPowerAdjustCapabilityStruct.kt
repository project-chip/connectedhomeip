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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DeviceEnergyManagementClusterPowerAdjustCapabilityStruct(
  val powerAdjustCapability: List<DeviceEnergyManagementClusterPowerAdjustStruct>?,
  val cause: UInt,
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterPowerAdjustCapabilityStruct {\n")
    append("\tpowerAdjustCapability : $powerAdjustCapability\n")
    append("\tcause : $cause\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (powerAdjustCapability != null) {
        startArray(ContextSpecificTag(TAG_POWER_ADJUST_CAPABILITY))
        for (item in powerAdjustCapability.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      } else {
        putNull(ContextSpecificTag(TAG_POWER_ADJUST_CAPABILITY))
      }
      put(ContextSpecificTag(TAG_CAUSE), cause)
      endStructure()
    }
  }

  companion object {
    private const val TAG_POWER_ADJUST_CAPABILITY = 0
    private const val TAG_CAUSE = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): DeviceEnergyManagementClusterPowerAdjustCapabilityStruct {
      tlvReader.enterStructure(tlvTag)
      val powerAdjustCapability =
        if (!tlvReader.isNull()) {
          buildList<DeviceEnergyManagementClusterPowerAdjustStruct> {
            tlvReader.enterArray(ContextSpecificTag(TAG_POWER_ADJUST_CAPABILITY))
            while (!tlvReader.isEndOfContainer()) {
              add(DeviceEnergyManagementClusterPowerAdjustStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_POWER_ADJUST_CAPABILITY))
          null
        }
      val cause = tlvReader.getUInt(ContextSpecificTag(TAG_CAUSE))

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterPowerAdjustCapabilityStruct(powerAdjustCapability, cause)
    }
  }
}
