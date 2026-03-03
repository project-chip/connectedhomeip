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
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AmbientContextSensingClusterObjectCountConfigStruct(
  val countingObject: AmbientContextSensingClusterSemanticTagStruct,
  val objectCountThreshold: UInt,
) {
  override fun toString(): String = buildString {
    append("AmbientContextSensingClusterObjectCountConfigStruct {\n")
    append("\tcountingObject : $countingObject\n")
    append("\tobjectCountThreshold : $objectCountThreshold\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      countingObject.toTlv(ContextSpecificTag(TAG_COUNTING_OBJECT), this)
      put(ContextSpecificTag(TAG_OBJECT_COUNT_THRESHOLD), objectCountThreshold)
      endStructure()
    }
  }

  companion object {
    private const val TAG_COUNTING_OBJECT = 0
    private const val TAG_OBJECT_COUNT_THRESHOLD = 1

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AmbientContextSensingClusterObjectCountConfigStruct {
      tlvReader.enterStructure(tlvTag)
      val countingObject =
        AmbientContextSensingClusterSemanticTagStruct.fromTlv(
          ContextSpecificTag(TAG_COUNTING_OBJECT),
          tlvReader,
        )
      val objectCountThreshold = tlvReader.getUInt(ContextSpecificTag(TAG_OBJECT_COUNT_THRESHOLD))

      tlvReader.exitContainer()

      return AmbientContextSensingClusterObjectCountConfigStruct(
        countingObject,
        objectCountThreshold,
      )
    }
  }
}
