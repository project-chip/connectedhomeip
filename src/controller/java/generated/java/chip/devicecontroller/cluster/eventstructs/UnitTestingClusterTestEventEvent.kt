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
package chip.devicecontroller.cluster.eventstructs

import chip.devicecontroller.cluster.*
import chip.tlv.Tag
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class UnitTestingClusterTestEventEvent (
    val arg1: Int,
    val arg2: Int,
    val arg3: Boolean,
    val arg4: chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct,
    val arg5: List<chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct>,
    val arg6: List<Int>) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("UnitTestingClusterTestEventEvent {\n")
    builder.append("\targ1 : $arg1\n")
    builder.append("\targ2 : $arg2\n")
    builder.append("\targ3 : $arg3\n")
    builder.append("\targ4 : $arg4\n")
    builder.append("\targ5 : $arg5\n")
    builder.append("\targ6 : $arg6\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(1), arg1)
    tlvWriter.put(ContextSpecificTag(2), arg2)
    tlvWriter.put(ContextSpecificTag(3), arg3)
    arg4.toTlv(ContextSpecificTag(4), tlvWriter)
    tlvWriter.startList(ContextSpecificTag(5))
      val iter_arg5 = arg5.iterator()
      while(iter_arg5.hasNext()) {
        val next = iter_arg5.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    tlvWriter.startList(ContextSpecificTag(6))
      val iter_arg6 = arg6.iterator()
      while(iter_arg6.hasNext()) {
        val next = iter_arg6.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : UnitTestingClusterTestEventEvent {
      tlvReader.enterStructure(tag)
      val arg1: Int = tlvReader.getInt(ContextSpecificTag(1))
      val arg2: Int = tlvReader.getInt(ContextSpecificTag(2))
      val arg3: Boolean = tlvReader.getBoolean(ContextSpecificTag(3))
      val arg4: chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct = chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(4), tlvReader)
      val arg5: List<chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct> = mutableListOf<chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(5))
      while(true) {
        try {
          this.add(chip.devicecontroller.cluster.structs.UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      val arg6: List<Int> = mutableListOf<Int>().apply {
      tlvReader.enterList(ContextSpecificTag(6))
      while(true) {
        try {
          this.add(tlvReader.getInt(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
      
      tlvReader.exitContainer()

      return UnitTestingClusterTestEventEvent(arg1, arg2, arg3, arg4, arg5, arg6)
    }
  }
}
