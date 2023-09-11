/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.structs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class ElectrostaticFilterMonitoringClusterReplacementProductStruct(
  val productIdentifierType: Int,
  val productIdentifierValue: String
) {
  override fun toString(): String = buildString {
    append("ElectrostaticFilterMonitoringClusterReplacementProductStruct {\n")
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
    ): ElectrostaticFilterMonitoringClusterReplacementProductStruct {
      tlvReader.enterStructure(tag)
      val productIdentifierType = tlvReader.getInt(ContextSpecificTag(TAG_PRODUCT_IDENTIFIER_TYPE))
      val productIdentifierValue =
        tlvReader.getString(ContextSpecificTag(TAG_PRODUCT_IDENTIFIER_VALUE))

      tlvReader.exitContainer()

      return ElectrostaticFilterMonitoringClusterReplacementProductStruct(
        productIdentifierType,
        productIdentifierValue
      )
    }
  }
}
