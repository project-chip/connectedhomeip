/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class BasicInformationClusterStartUpEvent(val softwareVersion: Long) {
  override fun toString(): String = buildString {
    append("BasicInformationClusterStartUpEvent {\n")
    append("\tsoftwareVersion : $softwareVersion\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_SOFTWARE_VERSION), softwareVersion)
      endStructure()
    }
  }

  companion object {
    private const val TAG_SOFTWARE_VERSION = 0

    fun fromTlv(tag: Tag, tlvReader: TlvReader): BasicInformationClusterStartUpEvent {
      tlvReader.enterStructure(tag)
      val softwareVersion = tlvReader.getLong(ContextSpecificTag(TAG_SOFTWARE_VERSION))

      tlvReader.exitContainer()

      return BasicInformationClusterStartUpEvent(softwareVersion)
    }
  }
}
