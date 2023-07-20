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
    if (nullableInt == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_INT)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_INT), nullableInt)
    }
    if (optionalInt.isPresent) {
      val optoptionalInt = optionalInt.get()
      tlvWriter.put(ContextSpecificTag(TAG_OPTIONAL_INT), optoptionalInt)
    }
    if (nullableOptionalInt == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT)) }
    else {
      if (nullableOptionalInt.isPresent) {
      val optnullableOptionalInt = nullableOptionalInt.get()
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT), optnullableOptionalInt)
    }
    }
    if (nullableString == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_STRING)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_STRING), nullableString)
    }
    if (optionalString.isPresent) {
      val optoptionalString = optionalString.get()
      tlvWriter.put(ContextSpecificTag(TAG_OPTIONAL_STRING), optoptionalString)
    }
    if (nullableOptionalString == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING)) }
    else {
      if (nullableOptionalString.isPresent) {
      val optnullableOptionalString = nullableOptionalString.get()
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING), optnullableOptionalString)
    }
    }
    if (nullableStruct == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_STRUCT)) }
    else {
      nullableStruct.toTlv(ContextSpecificTag(TAG_NULLABLE_STRUCT), tlvWriter)
    }
    if (optionalStruct.isPresent) {
      val optoptionalStruct = optionalStruct.get()
      optoptionalStruct.toTlv(ContextSpecificTag(TAG_OPTIONAL_STRUCT), tlvWriter)
    }
    if (nullableOptionalStruct == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT)) }
    else {
      if (nullableOptionalStruct.isPresent) {
      val optnullableOptionalStruct = nullableOptionalStruct.get()
      optnullableOptionalStruct.toTlv(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT), tlvWriter)
    }
    }
    if (nullableList == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_LIST)) }
    else {
      tlvWriter.startList(ContextSpecificTag(TAG_NULLABLE_LIST))
      val iternullableList = nullableList.iterator()
      while(iternullableList.hasNext()) {
        val next = iternullableList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    if (optionalList.isPresent) {
      val optoptionalList = optionalList.get()
      tlvWriter.startList(ContextSpecificTag(TAG_OPTIONAL_LIST))
      val iteroptoptionalList = optoptionalList.iterator()
      while(iteroptoptionalList.hasNext()) {
        val next = iteroptoptionalList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    if (nullableOptionalList == null) { tlvWriter.putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST)) }
    else {
      if (nullableOptionalList.isPresent) {
      val optnullableOptionalList = nullableOptionalList.get()
      tlvWriter.startList(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
      val iteroptnullableOptionalList = optnullableOptionalList.iterator()
      while(iteroptnullableOptionalList.hasNext()) {
        val next = iteroptnullableOptionalList.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_NULLABLE_INT = 0
    private const val TAG_OPTIONAL_INT = 1
    private const val TAG_NULLABLE_OPTIONAL_INT = 2
    private const val TAG_NULLABLE_STRING = 3
    private const val TAG_OPTIONAL_STRING = 4
    private const val TAG_NULLABLE_OPTIONAL_STRING = 5
    private const val TAG_NULLABLE_STRUCT = 6
    private const val TAG_OPTIONAL_STRUCT = 7
    private const val TAG_NULLABLE_OPTIONAL_STRUCT = 8
    private const val TAG_NULLABLE_LIST = 9
    private const val TAG_OPTIONAL_LIST = 10
    private const val TAG_NULLABLE_OPTIONAL_LIST = 11

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterNullablesAndOptionalsStruct {
      tlvReader.enterStructure(tag)
      val nullableInt: Int? = try {
      tlvReader.getInt(ContextSpecificTag(TAG_NULLABLE_INT))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_INT))
      null
    }
      val optionalInt: Optional<Int> = try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_OPTIONAL_INT)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalInt: Optional<Int>? = try {
      try {
      Optional.of(tlvReader.getInt(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT))
      null
    }
      val nullableString: String? = try {
      tlvReader.getString(ContextSpecificTag(TAG_NULLABLE_STRING))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_STRING))
      null
    }
      val optionalString: Optional<String> = try {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_OPTIONAL_STRING)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalString: Optional<String>? = try {
      try {
      Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING))
      null
    }
      val nullableStruct: UnitTestingClusterSimpleStruct? = try {
      UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_NULLABLE_STRUCT), tlvReader)
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_STRUCT))
      null
    }
      val optionalStruct: Optional<UnitTestingClusterSimpleStruct> = try {
      Optional.of(UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_OPTIONAL_STRUCT), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
      val nullableOptionalStruct: Optional<UnitTestingClusterSimpleStruct>? = try {
      try {
      Optional.of(UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT), tlvReader))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT))
      null
    }
      val nullableList: List<Int>? = try {
      mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_NULLABLE_LIST))
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
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_LIST))
      null
    }
      val optionalList: Optional<List<Int>> = try {
      Optional.of(mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_OPTIONAL_LIST))
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
      tlvReader.enterList(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
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
      tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
      null
    }
      
      tlvReader.exitContainer()

      return UnitTestingClusterNullablesAndOptionalsStruct(nullableInt, optionalInt, nullableOptionalInt, nullableString, optionalString, nullableOptionalString, nullableStruct, optionalStruct, nullableOptionalStruct, nullableList, optionalList, nullableOptionalList)
    }
  }
}
