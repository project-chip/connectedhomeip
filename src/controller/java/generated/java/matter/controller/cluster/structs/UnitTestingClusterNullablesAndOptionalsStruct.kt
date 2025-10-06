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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class UnitTestingClusterNullablesAndOptionalsStruct(
  val nullableInt: UShort?,
  val optionalInt: Optional<UShort>,
  val nullableOptionalInt: Optional<UShort>?,
  val nullableString: String?,
  val optionalString: Optional<String>,
  val nullableOptionalString: Optional<String>?,
  val nullableStruct: UnitTestingClusterSimpleStruct?,
  val optionalStruct: Optional<UnitTestingClusterSimpleStruct>,
  val nullableOptionalStruct: Optional<UnitTestingClusterSimpleStruct>?,
  val nullableList: List<UByte>?,
  val optionalList: Optional<List<UByte>>,
  val nullableOptionalList: Optional<List<UByte>>?,
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterNullablesAndOptionalsStruct {\n")
    append("\tnullableInt : $nullableInt\n")
    append("\toptionalInt : $optionalInt\n")
    append("\tnullableOptionalInt : $nullableOptionalInt\n")
    append("\tnullableString : $nullableString\n")
    append("\toptionalString : $optionalString\n")
    append("\tnullableOptionalString : $nullableOptionalString\n")
    append("\tnullableStruct : $nullableStruct\n")
    append("\toptionalStruct : $optionalStruct\n")
    append("\tnullableOptionalStruct : $nullableOptionalStruct\n")
    append("\tnullableList : $nullableList\n")
    append("\toptionalList : $optionalList\n")
    append("\tnullableOptionalList : $nullableOptionalList\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (nullableInt != null) {
        put(ContextSpecificTag(TAG_NULLABLE_INT), nullableInt)
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_INT))
      }
      if (optionalInt.isPresent) {
        val optoptionalInt = optionalInt.get()
        put(ContextSpecificTag(TAG_OPTIONAL_INT), optoptionalInt)
      }
      if (nullableOptionalInt != null) {
        if (nullableOptionalInt.isPresent) {
          val optnullableOptionalInt = nullableOptionalInt.get()
          put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT), optnullableOptionalInt)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT))
      }
      if (nullableString != null) {
        put(ContextSpecificTag(TAG_NULLABLE_STRING), nullableString)
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_STRING))
      }
      if (optionalString.isPresent) {
        val optoptionalString = optionalString.get()
        put(ContextSpecificTag(TAG_OPTIONAL_STRING), optoptionalString)
      }
      if (nullableOptionalString != null) {
        if (nullableOptionalString.isPresent) {
          val optnullableOptionalString = nullableOptionalString.get()
          put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING), optnullableOptionalString)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING))
      }
      if (nullableStruct != null) {
        nullableStruct.toTlv(ContextSpecificTag(TAG_NULLABLE_STRUCT), this)
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_STRUCT))
      }
      if (optionalStruct.isPresent) {
        val optoptionalStruct = optionalStruct.get()
        optoptionalStruct.toTlv(ContextSpecificTag(TAG_OPTIONAL_STRUCT), this)
      }
      if (nullableOptionalStruct != null) {
        if (nullableOptionalStruct.isPresent) {
          val optnullableOptionalStruct = nullableOptionalStruct.get()
          optnullableOptionalStruct.toTlv(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT), this)
        }
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT))
      }
      if (nullableList != null) {
        startArray(ContextSpecificTag(TAG_NULLABLE_LIST))
        for (item in nullableList.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_LIST))
      }
      if (optionalList.isPresent) {
        val optoptionalList = optionalList.get()
        startArray(ContextSpecificTag(TAG_OPTIONAL_LIST))
        for (item in optoptionalList.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      }
      if (nullableOptionalList != null) {
        if (nullableOptionalList.isPresent) {
          val optnullableOptionalList = nullableOptionalList.get()
          startArray(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
          for (item in optnullableOptionalList.iterator()) {
            put(AnonymousTag, item)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
      }
      endStructure()
    }
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): UnitTestingClusterNullablesAndOptionalsStruct {
      tlvReader.enterStructure(tlvTag)
      val nullableInt =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_NULLABLE_INT))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_INT))
          null
        }
      val optionalInt =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_INT))) {
          Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_OPTIONAL_INT)))
        } else {
          Optional.empty()
        }
      val nullableOptionalInt =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT))) {
            Optional.of(tlvReader.getUShort(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT))
          null
        }
      val nullableString =
        if (!tlvReader.isNull()) {
          tlvReader.getString(ContextSpecificTag(TAG_NULLABLE_STRING))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_STRING))
          null
        }
      val optionalString =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_STRING))) {
          Optional.of(tlvReader.getString(ContextSpecificTag(TAG_OPTIONAL_STRING)))
        } else {
          Optional.empty()
        }
      val nullableOptionalString =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING))) {
            Optional.of(tlvReader.getString(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING))
          null
        }
      val nullableStruct =
        if (!tlvReader.isNull()) {
          UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_NULLABLE_STRUCT), tlvReader)
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_STRUCT))
          null
        }
      val optionalStruct =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_STRUCT))) {
          Optional.of(
            UnitTestingClusterSimpleStruct.fromTlv(
              ContextSpecificTag(TAG_OPTIONAL_STRUCT),
              tlvReader,
            )
          )
        } else {
          Optional.empty()
        }
      val nullableOptionalStruct =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT))) {
            Optional.of(
              UnitTestingClusterSimpleStruct.fromTlv(
                ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT),
                tlvReader,
              )
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT))
          null
        }
      val nullableList =
        if (!tlvReader.isNull()) {
          buildList<UByte> {
            tlvReader.enterArray(ContextSpecificTag(TAG_NULLABLE_LIST))
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getUByte(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_LIST))
          null
        }
      val optionalList =
        if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_LIST))) {
          Optional.of(
            buildList<UByte> {
              tlvReader.enterArray(ContextSpecificTag(TAG_OPTIONAL_LIST))
              while (!tlvReader.isEndOfContainer()) {
                add(tlvReader.getUByte(AnonymousTag))
              }
              tlvReader.exitContainer()
            }
          )
        } else {
          Optional.empty()
        }
      val nullableOptionalList =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))) {
            Optional.of(
              buildList<UByte> {
                tlvReader.enterArray(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST))
          null
        }

      tlvReader.exitContainer()

      return UnitTestingClusterNullablesAndOptionalsStruct(
        nullableInt,
        optionalInt,
        nullableOptionalInt,
        nullableString,
        optionalString,
        nullableOptionalString,
        nullableStruct,
        optionalStruct,
        nullableOptionalStruct,
        nullableList,
        optionalList,
        nullableOptionalList,
      )
    }
  }
}
