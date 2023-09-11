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

class ApplicationBasicClusterApplicationStruct(
  val catalogVendorID: Int,
  val applicationID: String
) {
  override fun toString(): String = buildString {
    append("ApplicationBasicClusterApplicationStruct {\n")
    append("\tcatalogVendorID : $catalogVendorID\n")
    append("\tapplicationID : $applicationID\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_CATALOG_VENDOR_I_D), catalogVendorID)
      put(ContextSpecificTag(TAG_APPLICATION_I_D), applicationID)
      endStructure()
    }
  }

  companion object {
    private const val TAG_CATALOG_VENDOR_I_D = 0
    private const val TAG_APPLICATION_I_D = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ApplicationBasicClusterApplicationStruct {
      tlvReader.enterStructure(tag)
      val catalogVendorID = tlvReader.getInt(ContextSpecificTag(TAG_CATALOG_VENDOR_I_D))
      val applicationID = tlvReader.getString(ContextSpecificTag(TAG_APPLICATION_I_D))

      tlvReader.exitContainer()

      return ApplicationBasicClusterApplicationStruct(catalogVendorID, applicationID)
    }
  }
}
