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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ApplicationLauncherClusterApplicationEPStruct(
  val application: ApplicationLauncherClusterApplicationStruct,
  val endpoint: Optional<UInt>,
) {
  override fun toString(): String = buildString {
    append("ApplicationLauncherClusterApplicationEPStruct {\n")
    append("\tapplication : $application\n")
    append("\tendpoint : $endpoint\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ApplicationLauncherClusterApplicationEPStruct {
      tlvReader.enterStructure(tlvTag)
      val application =
        ApplicationLauncherClusterApplicationStruct.fromTlv(
          ContextSpecificTag(TAG_APPLICATION),
          tlvReader,
        )
      val endpoint =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ENDPOINT))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ENDPOINT)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ApplicationLauncherClusterApplicationEPStruct(application, endpoint)
    }
  }
}
