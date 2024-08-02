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

class UnitTestingClusterTestGlobalStruct(
  val name: String,
  val myBitmap: ULong?,
  val myEnum: Optional<UInt>?,
) {
  override fun toString(): String = buildString {
    append("UnitTestingClusterTestGlobalStruct {\n")
    append("\tname : $name\n")
    append("\tmyBitmap : $myBitmap\n")
    append("\tmyEnum : $myEnum\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_NAME), name)
      if (myBitmap != null) {
        put(ContextSpecificTag(TAG_MY_BITMAP), myBitmap)
      } else {
        putNull(ContextSpecificTag(TAG_MY_BITMAP))
      }
      if (myEnum != null) {
        if (myEnum.isPresent) {
          val optmyEnum = myEnum.get()
          put(ContextSpecificTag(TAG_MY_ENUM), optmyEnum)
        }
      } else {
        putNull(ContextSpecificTag(TAG_MY_ENUM))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_NAME = 0
    private const val TAG_MY_BITMAP = 1
    private const val TAG_MY_ENUM = 2

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): UnitTestingClusterTestGlobalStruct {
      tlvReader.enterStructure(tlvTag)
      val name = tlvReader.getString(ContextSpecificTag(TAG_NAME))
      val myBitmap =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_MY_BITMAP))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MY_BITMAP))
          null
        }
      val myEnum =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_MY_ENUM))) {
            Optional.of(tlvReader.getUInt(ContextSpecificTag(TAG_MY_ENUM)))
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_MY_ENUM))
          null
        }

      tlvReader.exitContainer()

      return UnitTestingClusterTestGlobalStruct(name, myBitmap, myEnum)
    }
  }
}
