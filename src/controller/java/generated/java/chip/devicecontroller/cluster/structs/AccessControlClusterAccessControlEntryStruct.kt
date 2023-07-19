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

class AccessControlClusterAccessControlEntryStruct (
    val privilege: Int,
    val authMode: Int,
    val subjects: List<Long>?,
    val targets: List<AccessControlClusterAccessControlTargetStruct>?,
    val fabricIndex: Int) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("AccessControlClusterAccessControlEntryStruct {\n")
    builder.append("\tprivilege : $privilege\n")
    builder.append("\tauthMode : $authMode\n")
    builder.append("\tsubjects : $subjects\n")
    builder.append("\ttargets : $targets\n")
    builder.append("\tfabricIndex : $fabricIndex\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(1), privilege)
    tlvWriter.put(ContextSpecificTag(2), authMode)
    if (subjects == null) { tlvWriter.putNull(ContextSpecificTag(3)) }
    else {
      tlvWriter.startList(ContextSpecificTag(3))
      val itersubjects = subjects.iterator()
      while(itersubjects.hasNext()) {
        val next = itersubjects.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    if (targets == null) { tlvWriter.putNull(ContextSpecificTag(4)) }
    else {
      tlvWriter.startList(ContextSpecificTag(4))
      val itertargets = targets.iterator()
      while(itertargets.hasNext()) {
        val next = itertargets.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    tlvWriter.put(ContextSpecificTag(254), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlEntryStruct {
      tlvReader.enterStructure(tag)
      val privilege: Int = tlvReader.getInt(ContextSpecificTag(1))
      val authMode: Int = tlvReader.getInt(ContextSpecificTag(2))
      val subjects: List<Long>? = try {
      mutableListOf<Long>().apply {
      tlvReader.enterList(ContextSpecificTag(3))
      while(true) {
        try {
          this.add(tlvReader.getLong(AnonymousTag))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(3))
      null
    }
      val targets: List<AccessControlClusterAccessControlTargetStruct>? = try {
      mutableListOf<AccessControlClusterAccessControlTargetStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(4))
      while(true) {
        try {
          this.add(AccessControlClusterAccessControlTargetStruct.fromTlv(AnonymousTag, tlvReader))
        } catch (e: TlvParsingException) {
          break
        }
      }
      tlvReader.exitContainer()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(4))
      null
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(254))
      
      tlvReader.exitContainer()

      return AccessControlClusterAccessControlEntryStruct(privilege, authMode, subjects, targets, fabricIndex)
    }
  }
}
