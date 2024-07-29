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
package matter.controller.cluster.structs

import java.util.Optional
import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThermostatClusterAtomicAttributeStatusStruct(
  val attributeID: Optional<UInt>,
  val statusCode: Optional<UByte>,
) {
  override fun toString(): String = buildString {
    append("ThermostatClusterAtomicAttributeStatusStruct {\n")
    append("\tattributeID : $attributeID\n")
    append("\tstatusCode : $statusCode\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (attributeID.isPresent) {
        val optattributeID = attributeID.get()
        put(ContextSpecificTag(TAG_ATTRIBUTE_I_D), optattributeID)
      }
      if (statusCode.isPresent) {
        val optstatusCode = statusCode.get()
        put(ContextSpecificTag(TAG_STATUS_CODE), optstatusCode)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ATTRIBUTE_I_D = 0
    private const val TAG_STATUS_CODE = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThermostatClusterAtomicAttributeStatusStruct {
      tlvReader.enterStructure(tlvTag)
      val attributeID =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_ATTRIBUTE_I_D))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_ATTRIBUTE_I_D)))
        } else {
          Optional.empty()
        }
      val statusCode =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_STATUS_CODE))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_STATUS_CODE)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ThermostatClusterAtomicAttributeStatusStruct(attributeID, statusCode)
    }
  }
}
