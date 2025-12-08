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
import java.util.Optional
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DeviceEnergyManagementClusterCostStruct(
  val costType: UInt,
  val value: Long,
  val decimalPoints: UInt,
  val currency: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("DeviceEnergyManagementClusterCostStruct {\n")
    append("\tcostType : $costType\n")
    append("\tvalue : $value\n")
    append("\tdecimalPoints : $decimalPoints\n")
    append("\tcurrency : $currency\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_COST_TYPE), costType)
      put(ContextSpecificTag(TAG_VALUE), value)
      put(ContextSpecificTag(TAG_DECIMAL_POINTS), decimalPoints)
      if (currency.isPresent) {
        val optcurrency = currency.get()
        put(ContextSpecificTag(TAG_CURRENCY), optcurrency)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_COST_TYPE = 0
    private const val TAG_VALUE = 1
    private const val TAG_DECIMAL_POINTS = 2
    private const val TAG_CURRENCY = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DeviceEnergyManagementClusterCostStruct {
      tlvReader.enterStructure(tlvTag)
      val costType = tlvReader.getUInt(ContextSpecificTag(TAG_COST_TYPE))
      val value = tlvReader.getLong(ContextSpecificTag(TAG_VALUE))
      val decimalPoints = tlvReader.getUInt(ContextSpecificTag(TAG_DECIMAL_POINTS))
      val currency =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CURRENCY))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_CURRENCY)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return DeviceEnergyManagementClusterCostStruct(costType, value, decimalPoints, currency)
    }
  }
}
