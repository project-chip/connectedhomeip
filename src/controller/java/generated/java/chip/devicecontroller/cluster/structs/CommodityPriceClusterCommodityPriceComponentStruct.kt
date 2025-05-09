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

class CommodityPriceClusterCommodityPriceComponentStruct (
    val price: Long,
    val source: UInt,
    val description: Optional<String>,
    val tariffComponentID: Optional<ULong>) {
  override fun toString(): String  = buildString {
    append("CommodityPriceClusterCommodityPriceComponentStruct {\n")
    append("\tprice : $price\n")
    append("\tsource : $source\n")
    append("\tdescription : $description\n")
    append("\ttariffComponentID : $tariffComponentID\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PRICE), price)
      put(ContextSpecificTag(TAG_SOURCE), source)
      if (description.isPresent) {
      val optdescription = description.get()
      put(ContextSpecificTag(TAG_DESCRIPTION), optdescription)
    }
      if (tariffComponentID.isPresent) {
      val opttariffComponentID = tariffComponentID.get()
      put(ContextSpecificTag(TAG_TARIFF_COMPONENT_ID), opttariffComponentID)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRICE = 0
    private const val TAG_SOURCE = 1
    private const val TAG_DESCRIPTION = 2
    private const val TAG_TARIFF_COMPONENT_ID = 3

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityPriceClusterCommodityPriceComponentStruct {
      tlvReader.enterStructure(tlvTag)
      val price = tlvReader.getLong(ContextSpecificTag(TAG_PRICE))
      val source = tlvReader.getUInt(ContextSpecificTag(TAG_SOURCE))
      val description = if (tlvReader.isNextTag(ContextSpecificTag(TAG_DESCRIPTION))) {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DESCRIPTION)))
    } else {
      Optional.empty()
    }
      val tariffComponentID = if (tlvReader.isNextTag(ContextSpecificTag(TAG_TARIFF_COMPONENT_ID))) {
      Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_TARIFF_COMPONENT_ID)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return CommodityPriceClusterCommodityPriceComponentStruct(price, source, description, tariffComponentID)
    }
  }
}
