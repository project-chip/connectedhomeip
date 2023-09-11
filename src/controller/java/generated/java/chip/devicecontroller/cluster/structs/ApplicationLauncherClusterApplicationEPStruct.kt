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
import java.util.Optional

class ApplicationLauncherClusterApplicationEPStruct(
  val application: ApplicationLauncherClusterApplicationStruct,
  val endpoint: Optional<Int>
) {
  override fun toString(): String = buildString {
    append("ApplicationLauncherClusterApplicationEPStruct {\n")
    append("\tapplication : $application\n")
    append("\tendpoint : $endpoint\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      application.toTlv(ContextSpecificTag(TAG_APPLICATION), this)
      if (endpoint.isPresent) {
        val optendpoint = endpoint.get()
        put(ContextSpecificTag(TAG_ENDPOINT), optendpoint)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_APPLICATION = 0
    private const val TAG_ENDPOINT = 1

    fun fromTlv(tag: Tag, tlvReader: TlvReader): ApplicationLauncherClusterApplicationEPStruct {
      tlvReader.enterStructure(tag)
      val application =
        ApplicationLauncherClusterApplicationStruct.fromTlv(
          ContextSpecificTag(TAG_APPLICATION),
          tlvReader
        )
      val endpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_ENDPOINT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ApplicationLauncherClusterApplicationEPStruct(application, endpoint)
    }
  }
}
