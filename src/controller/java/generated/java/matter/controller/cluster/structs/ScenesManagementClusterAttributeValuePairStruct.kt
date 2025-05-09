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

class ScenesManagementClusterAttributeValuePairStruct(
  val attributeID: UInt,
  val valueUnsigned8: Optional<UByte>,
  val valueSigned8: Optional<Byte>,
  val valueUnsigned16: Optional<UShort>,
  val valueSigned16: Optional<Short>,
  val valueUnsigned32: Optional<UInt>,
  val valueSigned32: Optional<Int>,
  val valueUnsigned64: Optional<ULong>,
  val valueSigned64: Optional<Long>,
) {
  override fun toString(): String = buildString {
    append("ScenesManagementClusterAttributeValuePairStruct {\n")
    append("\tattributeID : $attributeID\n")
    append("\tvalueUnsigned8 : $valueUnsigned8\n")
    append("\tvalueSigned8 : $valueSigned8\n")
    append("\tvalueUnsigned16 : $valueUnsigned16\n")
    append("\tvalueSigned16 : $valueSigned16\n")
    append("\tvalueUnsigned32 : $valueUnsigned32\n")
    append("\tvalueSigned32 : $valueSigned32\n")
    append("\tvalueUnsigned64 : $valueUnsigned64\n")
    append("\tvalueSigned64 : $valueSigned64\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ATTRIBUTE_ID), attributeID)
      if (valueUnsigned8.isPresent) {
        val optvalueUnsigned8 = valueUnsigned8.get()
        put(ContextSpecificTag(TAG_VALUE_UNSIGNED8), optvalueUnsigned8)
      }
      if (valueSigned8.isPresent) {
        val optvalueSigned8 = valueSigned8.get()
        put(ContextSpecificTag(TAG_VALUE_SIGNED8), optvalueSigned8)
      }
      if (valueUnsigned16.isPresent) {
        val optvalueUnsigned16 = valueUnsigned16.get()
        put(ContextSpecificTag(TAG_VALUE_UNSIGNED16), optvalueUnsigned16)
      }
      if (valueSigned16.isPresent) {
        val optvalueSigned16 = valueSigned16.get()
        put(ContextSpecificTag(TAG_VALUE_SIGNED16), optvalueSigned16)
      }
      if (valueUnsigned32.isPresent) {
        val optvalueUnsigned32 = valueUnsigned32.get()
        put(ContextSpecificTag(TAG_VALUE_UNSIGNED32), optvalueUnsigned32)
      }
      if (valueSigned32.isPresent) {
        val optvalueSigned32 = valueSigned32.get()
        put(ContextSpecificTag(TAG_VALUE_SIGNED32), optvalueSigned32)
      }
      if (valueUnsigned64.isPresent) {
        val optvalueUnsigned64 = valueUnsigned64.get()
        put(ContextSpecificTag(TAG_VALUE_UNSIGNED64), optvalueUnsigned64)
      }
      if (valueSigned64.isPresent) {
        val optvalueSigned64 = valueSigned64.get()
        put(ContextSpecificTag(TAG_VALUE_SIGNED64), optvalueSigned64)
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_ATTRIBUTE_ID = 0
    private const val TAG_VALUE_UNSIGNED8 = 1
    private const val TAG_VALUE_SIGNED8 = 2
    private const val TAG_VALUE_UNSIGNED16 = 3
    private const val TAG_VALUE_SIGNED16 = 4
    private const val TAG_VALUE_UNSIGNED32 = 5
    private const val TAG_VALUE_SIGNED32 = 6
    private const val TAG_VALUE_UNSIGNED64 = 7
    private const val TAG_VALUE_SIGNED64 = 8

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): ScenesManagementClusterAttributeValuePairStruct {
      tlvReader.enterStructure(tlvTag)
      val attributeID = tlvReader.getUInt(ContextSpecificTag(TAG_ATTRIBUTE_ID))
      val valueUnsigned8 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_UNSIGNED8))) {
          Optional.of(tlvReader.getUByte(ContextSpecificTag(TAG_VALUE_UNSIGNED8)))
        } else {
          Optional.empty()
        }
      val valueSigned8 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_SIGNED8))) {
          Optional.of(tlvReader.getByte(ContextSpecificTag(TAG_VALUE_SIGNED8)))
        } else {
          Optional.empty()
        }
      val valueUnsigned16 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_UNSIGNED16))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_VALUE_UNSIGNED16)))
        } else {
          Optional.empty()
        }
      val valueSigned16 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_SIGNED16))) {
          Optional.of(tlvReader.getShort(ContextSpecificTag(TAG_VALUE_SIGNED16)))
        } else {
          Optional.empty()
        }
      val valueUnsigned32 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_UNSIGNED32))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_VALUE_UNSIGNED32)))
        } else {
          Optional.empty()
        }
      val valueSigned32 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_SIGNED32))) {
          Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_VALUE_SIGNED32)))
        } else {
          Optional.empty()
        }
      val valueUnsigned64 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_UNSIGNED64))) {
          Optional.of(tlvReader.getULong(ContextSpecificTag(TAG_VALUE_UNSIGNED64)))
        } else {
          Optional.empty()
        }
      val valueSigned64 =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE_SIGNED64))) {
          Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_VALUE_SIGNED64)))
        } else {
          Optional.empty()
        }

      tlvReader.exitContainer()

      return ScenesManagementClusterAttributeValuePairStruct(
        attributeID,
        valueUnsigned8,
        valueSigned8,
        valueUnsigned16,
        valueSigned16,
        valueUnsigned32,
        valueSigned32,
        valueUnsigned64,
        valueSigned64,
      )
    }
  }
}
