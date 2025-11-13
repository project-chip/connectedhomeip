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

class PushAvStreamTransportClusterContainerOptionsStruct(
  val containerType: UInt,
  val CMAFContainerOptions: Optional<PushAvStreamTransportClusterCMAFContainerOptionsStruct>,
) {
  override fun toString(): String = buildString {
    append("PushAvStreamTransportClusterContainerOptionsStruct {\n")
    append("\tcontainerType : $containerType\n")
    append("\tCMAFContainerOptions : $CMAFContainerOptions\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_CONTAINER_TYPE), containerType)
      if (CMAFContainerOptions.isPresent) {
        val optCMAFContainerOptions = CMAFContainerOptions.get()
        optCMAFContainerOptions.toTlv(ContextSpecificTag(TAG_CMAF_CONTAINER_OPTIONS), this)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_CONTAINER_TYPE = 0
    private const val TAG_CMAF_CONTAINER_OPTIONS = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): PushAvStreamTransportClusterContainerOptionsStruct {
      tlvReader.enterStructure(tlvTag)
      val containerType = tlvReader.getUInt(ContextSpecificTag(TAG_CONTAINER_TYPE))
      val CMAFContainerOptions =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_CMAF_CONTAINER_OPTIONS))) {
          Optional.of(
            PushAvStreamTransportClusterCMAFContainerOptionsStruct.fromTlv(
              ContextSpecificTag(TAG_CMAF_CONTAINER_OPTIONS),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return PushAvStreamTransportClusterContainerOptionsStruct(containerType, CMAFContainerOptions)
    }
  }
}
