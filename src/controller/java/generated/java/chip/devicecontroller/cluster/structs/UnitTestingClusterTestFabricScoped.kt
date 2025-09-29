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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class UnitTestingClusterTestFabricScoped(
  val fabricSensitiveInt8u: UInt,
  val optionalFabricSensitiveInt8u: Optional<UInt>,
  val nullableFabricSensitiveInt8u: UInt?,
  val nullableOptionalFabricSensitiveInt8u: Optional<UInt>?,
  val fabricSensitiveCharString: String,
  val fabricSensitiveStruct: UnitTestingClusterSimpleStruct,
  val fabricSensitiveInt8uList: List<UInt>,
  val fabricIndex: UInt
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterTestFabricScoped {\n")
    append("\tfabricSensitiveInt8u : $fabricSensitiveInt8u\n")
    append("\toptionalFabricSensitiveInt8u : $optionalFabricSensitiveInt8u\n")
    append("\tnullableFabricSensitiveInt8u : $nullableFabricSensitiveInt8u\n")
    append("\tnullableOptionalFabricSensitiveInt8u : $nullableOptionalFabricSensitiveInt8u\n")
    append("\tfabricSensitiveCharString : $fabricSensitiveCharString\n")
    append("\tfabricSensitiveStruct : $fabricSensitiveStruct\n")
    append("\tfabricSensitiveInt8uList : $fabricSensitiveInt8uList\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U), fabricSensitiveInt8u)
      if (optionalFabricSensitiveInt8u.isPresent) {
        val optoptionalFabricSensitiveInt8u = optionalFabricSensitiveInt8u.get()
        put(
          ContextSpecificTag(TAG_OPTIONAL_FABRIC_SENSITIVE_INT8U),
          optoptionalFabricSensitiveInt8u
        )
      }
      if (nullableFabricSensitiveInt8u != null) {
        put(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U), nullableFabricSensitiveInt8u)
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U))
      }
      if (nullableOptionalFabricSensitiveInt8u != null) {
        if (nullableOptionalFabricSensitiveInt8u.isPresent) {
          val optnullableOptionalFabricSensitiveInt8u = nullableOptionalFabricSensitiveInt8u.get()
          put(
            ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U),
            optnullableOptionalFabricSensitiveInt8u
          )
        }
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U))
      }
      put(ContextSpecificTag(TAG_FABRIC_SENSITIVE_CHAR_STRING), fabricSensitiveCharString)
      fabricSensitiveStruct.toTlv(ContextSpecificTag(TAG_FABRIC_SENSITIVE_STRUCT), this)
      startArray(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U_LIST))
      for (item in fabricSensitiveInt8uList.iterator()) {
        put(AnonymousTag, item)
      }
      endArray()
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_FABRIC_SENSITIVE_INT8U = 1
    private const val TAG_OPTIONAL_FABRIC_SENSITIVE_INT8U = 2
    private const val TAG_NULLABLE_FABRIC_SENSITIVE_INT8U = 3
    private const val TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U = 4
    private const val TAG_FABRIC_SENSITIVE_CHAR_STRING = 5
    private const val TAG_FABRIC_SENSITIVE_STRUCT = 6
    private const val TAG_FABRIC_SENSITIVE_INT8U_LIST = 7
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): UnitTestingClusterTestFabricScoped {
      tlvReader.enterStructure(tlvTag)
      val fabricSensitiveInt8u = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U))
      val optionalFabricSensitiveInt8u =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_FABRIC_SENSITIVE_INT8U))) {
          Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_OPTIONAL_FABRIC_SENSITIVE_INT8U)))
        } else {
          Optional.empty()
        }
      val nullableFabricSensitiveInt8u =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U))
          null
        }
      val nullableOptionalFabricSensitiveInt8u =
        if (!tlvReader.isNull()) {
          if (
            tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U))
          ) {
            Optional.of(
              tlvReader.getUInt(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U))
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U))
          null
        }
      val fabricSensitiveCharString =
        tlvReader.getString(ContextSpecificTag(TAG_FABRIC_SENSITIVE_CHAR_STRING))
      val fabricSensitiveStruct =
        UnitTestingClusterSimpleStruct.fromTlv(
          ContextSpecificTag(TAG_FABRIC_SENSITIVE_STRUCT),
          tlvReader
        )
      val fabricSensitiveInt8uList =
        buildList<UInt> {
          tlvReader.enterArray(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U_LIST))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUInt(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return UnitTestingClusterTestFabricScoped(
        fabricSensitiveInt8u,
        optionalFabricSensitiveInt8u,
        nullableFabricSensitiveInt8u,
        nullableOptionalFabricSensitiveInt8u,
        fabricSensitiveCharString,
        fabricSensitiveStruct,
        fabricSensitiveInt8uList,
        fabricIndex
      )
    }
  }
}
