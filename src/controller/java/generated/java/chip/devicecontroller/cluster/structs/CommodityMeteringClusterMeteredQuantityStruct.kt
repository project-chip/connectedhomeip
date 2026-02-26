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
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class CommodityMeteringClusterMeteredQuantityStruct (
    val tariffComponentIDs: List<ULong>,
    val quantity: Long) {
  override fun toString(): String  = buildString {
    append("CommodityMeteringClusterMeteredQuantityStruct {\n")
    append("\ttariffComponentIDs : $tariffComponentIDs\n")
    append("\tquantity : $quantity\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      startArray(ContextSpecificTag(TAG_TARIFF_COMPONENT_I_DS))
      for (item in tariffComponentIDs.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      put(ContextSpecificTag(TAG_QUANTITY), quantity)
      endStructure()
    }
  }

  companion object {
    private const val TAG_TARIFF_COMPONENT_I_DS = 0
    private const val TAG_QUANTITY = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityMeteringClusterMeteredQuantityStruct {
      tlvReader.enterStructure(tlvTag)
      val tariffComponentIDs = buildList<ULong> {
      tlvReader.enterArray(ContextSpecificTag(TAG_TARIFF_COMPONENT_I_DS))
      while(!tlvReader.isEndOfContainer()) {
        add(tlvReader.getULong(AnonymousTag))
      }
      tlvReader.exitContainer()
    }
      val quantity = tlvReader.getLong(ContextSpecificTag(TAG_QUANTITY))
      
      tlvReader.exitContainer()

      return CommodityMeteringClusterMeteredQuantityStruct(tariffComponentIDs, quantity)
    }
  }
}
