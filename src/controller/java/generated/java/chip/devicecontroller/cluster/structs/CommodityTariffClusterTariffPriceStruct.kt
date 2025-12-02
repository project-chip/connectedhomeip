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

class CommodityTariffClusterTariffPriceStruct (
    val priceType: UInt,
    val price: Optional<Long>,
    val priceLevel: Optional<Int>) {
  override fun toString(): String  = buildString {
    append("CommodityTariffClusterTariffPriceStruct {\n")
    append("\tpriceType : $priceType\n")
    append("\tprice : $price\n")
    append("\tpriceLevel : $priceLevel\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PRICE_TYPE), priceType)
      if (price.isPresent) {
      val optprice = price.get()
      put(ContextSpecificTag(TAG_PRICE), optprice)
    }
      if (priceLevel.isPresent) {
      val optpriceLevel = priceLevel.get()
      put(ContextSpecificTag(TAG_PRICE_LEVEL), optpriceLevel)
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRICE_TYPE = 0
    private const val TAG_PRICE = 1
    private const val TAG_PRICE_LEVEL = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : CommodityTariffClusterTariffPriceStruct {
      tlvReader.enterStructure(tlvTag)
      val priceType = tlvReader.getUInt(ContextSpecificTag(TAG_PRICE_TYPE))
      val price = if (tlvReader.isNextTag(ContextSpecificTag(TAG_PRICE))) {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_PRICE)))
    } else {
      Optional.empty()
    }
      val priceLevel = if (tlvReader.isNextTag(ContextSpecificTag(TAG_PRICE_LEVEL))) {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_PRICE_LEVEL)))
    } else {
      Optional.empty()
    }
      
      tlvReader.exitContainer()

      return CommodityTariffClusterTariffPriceStruct(priceType, price, priceLevel)
    }
  }
}
