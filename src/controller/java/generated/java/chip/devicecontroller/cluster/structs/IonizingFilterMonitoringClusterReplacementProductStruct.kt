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
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class IonizingFilterMonitoringClusterReplacementProductStruct(
  val productIdentifierType: Int,
  val productIdentifierValue: String
) {
  override fun toString(): String = buildString {
    append("IonizingFilterMonitoringClusterReplacementProductStruct {\n")
    append("\tproductIdentifierType : $productIdentifierType\n")
    append("\tproductIdentifierValue : $productIdentifierValue\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_PRODUCT_IDENTIFIER_TYPE), productIdentifierType)
      put(ContextSpecificTag(TAG_PRODUCT_IDENTIFIER_VALUE), productIdentifierValue)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRODUCT_IDENTIFIER_TYPE = 0
    private const val TAG_PRODUCT_IDENTIFIER_VALUE = 1

    fun fromTlv(
      tag: Tag,
      tlvReader: TlvReader
    ): IonizingFilterMonitoringClusterReplacementProductStruct {
      tlvReader.enterStructure(tag)
      val productIdentifierType = tlvReader.getInt(ContextSpecificTag(TAG_PRODUCT_IDENTIFIER_TYPE))
      val productIdentifierValue =
        tlvReader.getString(ContextSpecificTag(TAG_PRODUCT_IDENTIFIER_VALUE))

      tlvReader.exitContainer()

      return IonizingFilterMonitoringClusterReplacementProductStruct(
        productIdentifierType,
        productIdentifierValue
      )
    }
  }
}
