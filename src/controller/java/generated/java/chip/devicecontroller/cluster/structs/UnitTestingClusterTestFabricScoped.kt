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
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
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
    tlvWriter.put(ContextSpecificTag(1), fabricSensitiveInt8u)
    if (optionalFabricSensitiveInt8u.isPresent) {
      val opt_optionalFabricSensitiveInt8u = optionalFabricSensitiveInt8u.get()
      tlvWriter.put(ContextSpecificTag(2), opt_optionalFabricSensitiveInt8u)
    }
    if (nullableFabricSensitiveInt8u == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), nullableFabricSensitiveInt8u)
    }
    if (nullableOptionalFabricSensitiveInt8u == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      if (nullableOptionalFabricSensitiveInt8u.isPresent) {
      val opt_nullableOptionalFabricSensitiveInt8u = nullableOptionalFabricSensitiveInt8u.get()
      tlvWriter.put(ContextSpecificTag(4), opt_nullableOptionalFabricSensitiveInt8u)
    }
    }
    tlvWriter.put(ContextSpecificTag(5), fabricSensitiveCharString)
    fabricSensitiveStruct.toTlv(ContextSpecificTag(6), tlvWriter)
    tlvWriter.startList(ContextSpecificTag(7))
      val iter_fabricSensitiveInt8uList = fabricSensitiveInt8uList.iterator()
      while(iter_fabricSensitiveInt8uList.hasNext()) {
        val next = iter_fabricSensitiveInt8uList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestFabricScoped {
      tlvReader.enterStructure(tag)
      val fabricSensitiveInt8u: Int = tlvReader.getInt(ContextSpecificTag(1))
      val optionalFabricSensitiveInt8u: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableFabricSensitiveInt8u: Int? = try {
      tlvReader.getInt(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
      val nullableOptionalFabricSensitiveInt8u: Optional<Int>? = try {
      try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
      val fabricSensitiveCharString: String = tlvReader.getString(ContextSpecificTag(5))
      val fabricSensitiveStruct: UnitTestingClusterSimpleStruct = UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(6), tlvReader)
      val fabricSensitiveInt8uList: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(7))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return UnitTestingClusterTestFabricScoped(fabricSensitiveInt8u, optionalFabricSensitiveInt8u, nullableFabricSensitiveInt8u, nullableOptionalFabricSensitiveInt8u, fabricSensitiveCharString, fabricSensitiveStruct, fabricSensitiveInt8uList, fabricIndex)
    }
  }
}
