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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class CommodityPriceClusterCommodityPriceStruct(
  val periodStart: ULong,
  val periodEnd: ULong?,
  val price: CommodityPriceClusterPriceStruct,
  val description: Optional<String>,
  val components: Optional<List<CommodityPriceClusterCommodityPriceComponentStruct>>,
) {
  override fun toString(): String = buildString {
    append("CommodityPriceClusterCommodityPriceStruct {\n")
    append("\tperiodStart : $periodStart\n")
    append("\tperiodEnd : $periodEnd\n")
    append("\tprice : $price\n")
    append("\tdescription : $description\n")
    append("\tcomponents : $components\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PERIOD_START), periodStart)
      if (periodEnd != null) {
        put(ContextSpecificTag(TAG_PERIOD_END), periodEnd)
      } else {
        putNull(ContextSpecificTag(TAG_PERIOD_END))
      }
      price.toTlv(ContextSpecificTag(TAG_PRICE), this)
      if (description.isPresent) {
        val optdescription = description.get()
        put(ContextSpecificTag(TAG_DESCRIPTION), optdescription)
      }
      if (components.isPresent) {
        val optcomponents = components.get()
        startArray(ContextSpecificTag(TAG_COMPONENTS))
        for (item in optcomponents.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_PERIOD_START = 0
    private const val TAG_PERIOD_END = 1
    private const val TAG_PRICE = 2
    private const val TAG_DESCRIPTION = 3
    private const val TAG_COMPONENTS = 4

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): CommodityPriceClusterCommodityPriceStruct {
      tlvReader.enterStructure(tlvTag)
      val periodStart = tlvReader.getULong(ContextSpecificTag(TAG_PERIOD_START))
      val periodEnd =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_PERIOD_END))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_PERIOD_END))
          null
        }
      val price = CommodityPriceClusterPriceStruct.fromTlv(ContextSpecificTag(TAG_PRICE), tlvReader)
      val description =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_DESCRIPTION))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_DESCRIPTION)))
        } else {
          Optional.empty()
        }
      val components =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_COMPONENTS))) {
          Optional.of(
            buildList<CommodityPriceClusterCommodityPriceComponentStruct> {
              tlvReader.enterArray(ContextSpecificTag(TAG_COMPONENTS))
              while (!tlvReader.isEndOfContainer()) {
                add(
                  CommodityPriceClusterCommodityPriceComponentStruct.fromTlv(
                    AnonymousTag,
                    tlvReader,
                  )
                )
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return CommodityPriceClusterCommodityPriceStruct(
        periodStart,
        periodEnd,
        price,
        description,
        components,
      )
    }
  }
}
