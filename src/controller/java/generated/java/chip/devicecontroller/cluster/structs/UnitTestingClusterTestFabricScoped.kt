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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class UnitTestingClusterTestFabricScoped (
    val fabricSensitiveInt8u: Int,
    val optionalFabricSensitiveInt8u: Optional<Int>,
    val nullableFabricSensitiveInt8u: Int?,
    val nullableOptionalFabricSensitiveInt8u: Optional<Int>?,
    val fabricSensitiveCharString: String,
    val fabricSensitiveStruct: UnitTestingClusterSimpleStruct,
    val fabricSensitiveInt8uList: List<Int>,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("UnitTestingClusterTestFabricScoped {\n")
    builder.append("\tfabricSensitiveInt8u : $fabricSensitiveInt8u\n")
    builder.append("\toptionalFabricSensitiveInt8u : $optionalFabricSensitiveInt8u\n")
    builder.append("\tnullableFabricSensitiveInt8u : $nullableFabricSensitiveInt8u\n")
    builder.append("\tnullableOptionalFabricSensitiveInt8u : $nullableOptionalFabricSensitiveInt8u\n")
    builder.append("\tfabricSensitiveCharString : $fabricSensitiveCharString\n")
    builder.append("\tfabricSensitiveStruct : $fabricSensitiveStruct\n")
    builder.append("\tfabricSensitiveInt8uList : $fabricSensitiveInt8uList\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U), fabricSensitiveInt8u)
    if (optionalFabricSensitiveInt8u.isPresent) {
      val optoptionalFabricSensitiveInt8u = optionalFabricSensitiveInt8u.get()
      tlvWriter.put(ContextSpecificTag(TAG_OPTIONAL_FABRIC_SENSITIVE_INT8U), optoptionalFabricSensitiveInt8u)
    }
    if (nullableFabricSensitiveInt8u == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U), nullableFabricSensitiveInt8u)
    }
    if (nullableOptionalFabricSensitiveInt8u == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U)) }
    else {
      if (nullableOptionalFabricSensitiveInt8u.isPresent) {
      val optnullableOptionalFabricSensitiveInt8u = nullableOptionalFabricSensitiveInt8u.get()
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U), optnullableOptionalFabricSensitiveInt8u)
    }
    }
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_SENSITIVE_CHAR_STRING), fabricSensitiveCharString)
    fabricSensitiveStruct.toTlv(ContextSpecificTag(TAG_FABRIC_SENSITIVE_STRUCT), tlvWriter)
    tlvWriter.startList(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U_LIST))
      val iterfabricSensitiveInt8uList = fabricSensitiveInt8uList.iterator()
      while(iterfabricSensitiveInt8uList.hasNext()) {
        val next = iterfabricSensitiveInt8uList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
    tlvWriter.endStructure()
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

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestFabricScoped {
      tlvReader.enterStructure(tag)
      val fabricSensitiveInt8u: Int = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U))
      val optionalFabricSensitiveInt8u: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_OPTIONAL_FABRIC_SENSITIVE_INT8U)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableFabricSensitiveInt8u: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_FABRIC_SENSITIVE_INT8U))
      null
    }
      val nullableOptionalFabricSensitiveInt8u: Optional<Int>? = try {
      try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_FABRIC_SENSITIVE_INT8U))
      null
    }
      val fabricSensitiveCharString: String = tlvReader.getString(ContextSpecificTag(TAG_FABRIC_SENSITIVE_CHAR_STRING))
      val fabricSensitiveStruct: UnitTestingClusterSimpleStruct = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_FABRIC_SENSITIVE_STRUCT), tlvReader)
      val fabricSensitiveInt8uList: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_FABRIC_SENSITIVE_INT8U_LIST))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return UnitTestingClusterTestFabricScoped(fabricSensitiveInt8u, optionalFabricSensitiveInt8u, nullableFabricSensitiveInt8u, nullableOptionalFabricSensitiveInt8u, fabricSensitiveCharString, fabricSensitiveStruct, fabricSensitiveInt8uList, fabricIndex)
    }
  }
}
