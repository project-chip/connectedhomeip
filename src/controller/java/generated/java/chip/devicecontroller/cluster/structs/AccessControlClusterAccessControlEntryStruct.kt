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
    tlvWriter.put(ContextSpecificTag(TAG_PRIVILEGE), privilege)
    tlvWriter.put(ContextSpecificTag(TAG_AUTH_MODE), authMode)
    if (subjects == null) { tlvWriter.putNull(ContextSpecificTag(TAG_SUBJECTS)) }
    else {
      tlvWriter.startList(ContextSpecificTag(TAG_SUBJECTS))
      val itersubjects = subjects.iterator()
      while(itersubjects.hasNext()) {
        val next = itersubjects.next()
        tlvWriter.put(AnonymousTag, next)
      }
      tlvWriter.endList()
    }
    if (targets == null) { tlvWriter.putNull(ContextSpecificTag(TAG_TARGETS)) }
    else {
      tlvWriter.startList(ContextSpecificTag(TAG_TARGETS))
      val itertargets = targets.iterator()
      while(itertargets.hasNext()) {
        val next = itertargets.next()
        next.toTlv(AnonymousTag, tlvWriter)
      }
      tlvWriter.endList()
    }
    tlvWriter.put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_PRIVILEGE = 1
    private const val TAG_AUTH_MODE = 2
    private const val TAG_SUBJECTS = 3
    private const val TAG_TARGETS = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : AccessControlClusterAccessControlEntryStruct {
      tlvReader.enterStructure(tag)
      val privilege: Int = tlvReader.getInt(ContextSpecificTag(TAG_PRIVILEGE))
      val authMode: Int = tlvReader.getInt(ContextSpecificTag(TAG_AUTH_MODE))
      val subjects: List<Long>? = try {
      mutableListOf<Long>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_SUBJECTS))
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
      tlvReader.getNull(ContextSpecificTag(TAG_SUBJECTS))
      null
    }
      val targets: List<AccessControlClusterAccessControlTargetStruct>? = try {
      mutableListOf<AccessControlClusterAccessControlTargetStruct>().apply {
      tlvReader.enterList(ContextSpecificTag(TAG_TARGETS))
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
      tlvReader.getNull(ContextSpecificTag(TAG_TARGETS))
      null
    }
      val fabricIndex: Int = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))
      
      tlvReader.exitContainer()

      return AccessControlClusterAccessControlEntryStruct(privilege, authMode, subjects, targets, fabricIndex)
    }
  }
}
