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

class CommodityTariffClusterCurrencyStruct(val currency: UShort, val decimalPoints: UByte) {
  override fun toString(): String = buildString {
    append("CommodityTariffClusterCurrencyStruct {\n")
    append("\tcurrency : $currency\n")
    append("\tdecimalPoints : $decimalPoints\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CURRENCY), currency)
      put(ContextSpecificTag(TAG_DECIMAL_POINTS), decimalPoints)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CURRENCY = 0
    private const val TAG_DECIMAL_POINTS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): CommodityTariffClusterCurrencyStruct {
      tlvReader.enterStructure(tlvTag)
      val currency = tlvReader.getUShort(ContextSpecificTag(TAG_CURRENCY))
      val decimalPoints = tlvReader.getUByte(ContextSpecificTag(TAG_DECIMAL_POINTS))

      tlvReader.exitContainer()

      return CommodityTariffClusterCurrencyStruct(currency, decimalPoints)
    }
  }
}
