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

class UnitTestingClusterNullablesAndOptionalsStruct (
    val nullableInt: Int?,
    val optionalInt: Optional<Int>,
    val nullableOptionalInt: Optional<Int>?,
    val nullableString: String?,
    val optionalString: Optional<String>,
    val nullableOptionalString: Optional<String>?,
    val nullableStruct: UnitTestingClusterSimpleStruct?,
    val optionalStruct: Optional<UnitTestingClusterSimpleStruct>,
    val nullableOptionalStruct: Optional<UnitTestingClusterSimpleStruct>?,
    val nullableList: List<Int>?,
    val optionalList: Optional<List<Int>>,
    val nullableOptionalList: Optional<List<Int>>?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("UnitTestingClusterNullablesAndOptionalsStruct {\n")
    builder.append("\tnullableInt : $nullableInt\n")
    builder.append("\toptionalInt : $optionalInt\n")
    builder.append("\tnullableOptionalInt : $nullableOptionalInt\n")
    builder.append("\tnullableString : $nullableString\n")
    builder.append("\toptionalString : $optionalString\n")
    builder.append("\tnullableOptionalString : $nullableOptionalString\n")
    builder.append("\tnullableStruct : $nullableStruct\n")
    builder.append("\toptionalStruct : $optionalStruct\n")
    builder.append("\tnullableOptionalStruct : $nullableOptionalStruct\n")
    builder.append("\tnullableList : $nullableList\n")
    builder.append("\toptionalList : $optionalList\n")
    builder.append("\tnullableOptionalList : $nullableOptionalList\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    if (nullableInt == null) { tlvWriter.putNull(ContextSpecificTag(0)) }
    else {
      tlvWriter.put(ContextSpecificTag(0), nullableInt)
    }
    if (optionalInt.isPresent) {
      val opt_optionalInt = optionalInt.get()
      tlvWriter.put(ContextSpecificTag(1), opt_optionalInt)
    }
    if (nullableOptionalInt == null) { tlvWriter.putNull(ContextSpecificTag(2)) }
    else {
      if (nullableOptionalInt.isPresent) {
      val opt_nullableOptionalInt = nullableOptionalInt.get()
      tlvWriter.put(ContextSpecificTag(2), opt_nullableOptionalInt)
    }
    }
    if (nullableString == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.put(ContextSpecificTag(3), nullableString)
    }
    if (optionalString.isPresent) {
      val opt_optionalString = optionalString.get()
      tlvWriter.put(ContextSpecificTag(4), opt_optionalString)
    }
    if (nullableOptionalString == null) { tlvWriter.putNull(ContextSpecificTag(5)) }
    else {
      if (nullableOptionalString.isPresent) {
      val opt_nullableOptionalString = nullableOptionalString.get()
      tlvWriter.put(ContextSpecificTag(5), opt_nullableOptionalString)
    }
    }
    if (nullableStruct == null) { tlvWriter.putNull(ContextSpecificTag(6)) }
    else {
      nullableStruct.toTlv(ContextSpecificTag(6), tlvWriter)
    }
    if (optionalStruct.isPresent) {
      val opt_optionalStruct = optionalStruct.get()
      opt_optionalStruct.toTlv(ContextSpecificTag(7), tlvWriter)
    }
    if (nullableOptionalStruct == null) { tlvWriter.putNull(ContextSpecificTag(8)) }
    else {
      if (nullableOptionalStruct.isPresent) {
      val opt_nullableOptionalStruct = nullableOptionalStruct.get()
      opt_nullableOptionalStruct.toTlv(ContextSpecificTag(8), tlvWriter)
    }
    }
    if (nullableList == null) { tlvWriter.putNull(ContextSpecificTag(9)) }
    else {
      tlvWriter.startList(ContextSpecificTag(9))
      val iter_nullableList = nullableList.iterator()
      while(iter_nullableList.hasNext()) {
        val next = iter_nullableList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    if (optionalList.isPresent) {
      val opt_optionalList = optionalList.get()
      tlvWriter.startList(ContextSpecificTag(10))
      val iter_opt_optionalList = opt_optionalList.iterator()
      while(iter_opt_optionalList.hasNext()) {
        val next = iter_opt_optionalList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    if (nullableOptionalList == null) { tlvWriter.putNull(ContextSpecificTag(11)) }
    else {
      if (nullableOptionalList.isPresent) {
      val opt_nullableOptionalList = nullableOptionalList.get()
      tlvWriter.startList(ContextSpecificTag(11))
      val iter_opt_nullableOptionalList = opt_nullableOptionalList.iterator()
      while(iter_opt_nullableOptionalList.hasNext()) {
        val next = iter_opt_nullableOptionalList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    }
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterNullablesAndOptionalsStruct {
      tlvReader.enterStructure(tag)
      val nullableInt: Int? = try {
      tlvReader.getInt(ContextSpecificTag(0))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(0))
      null
    }
      val optionalInt: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(1)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalInt: Optional<Int>? = try {
      try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(2)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(2))
      null
    }
      val nullableString: String? = try {
      tlvReader.getString(ContextSpecificTag(3))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
      val optionalString: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(4)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalString: Optional<String>? = try {
      try {
      Optional.of(tlvReader.getString(ContextSpecificTag(5)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(5))
      null
    }
      val nullableStruct: UnitTestingClusterSimpleStruct? = try {
      UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(6), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(6))
      null
    }
      val optionalStruct: Optional<UnitTestingClusterSimpleStruct> = try {
      Optional.of(UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(7), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalStruct: Optional<UnitTestingClusterSimpleStruct>? = try {
      try {
      Optional.of(UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(8), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(8))
      null
    }
      val nullableList: List<Int>? = try {
      mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(9))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(9))
      null
    }
      val optionalList: Optional<List<Int>> = try {
      Optional.of(mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(10))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalList: Optional<List<Int>>? = try {
      try {
      Optional.of(mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(11))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    })
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(11))
      null
    }
      
      tlvReader.exitContainer()

      return UnitTestingClusterNullablesAndOptionalsStruct(nullableInt, optionalInt, nullableOptionalInt, nullableString, optionalString, nullableOptionalString, nullableStruct, optionalStruct, nullableOptionalStruct, nullableList, optionalList, nullableOptionalList)
    }
  }
}
