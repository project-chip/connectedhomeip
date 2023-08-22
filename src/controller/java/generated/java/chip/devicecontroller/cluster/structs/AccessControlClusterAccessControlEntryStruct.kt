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
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

class AccessControlClusterAccessControlEntryStruct(
  val privilege: Int,
  val authMode: Int,
  val subjects: List<Long>?,
  val targets: List<AccessControlClusterAccessControlTargetStruct>?,
  val fabricIndex: Int
) {
  override fun toString(): String = buildString {
    append("AccessControlClusterAccessControlEntryStruct {\n")
    append("\tprivilege : $privilege\n")
    append("\tauthMode : $authMode\n")
    append("\tsubjects : $subjects\n")
    append("\ttargets : $targets\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tag)
      put(ContextSpecificTag(TAG_PRIVILEGE), privilege)
      put(ContextSpecificTag(TAG_AUTH_MODE), authMode)
      if (subjects != null) {
        startList(ContextSpecificTag(TAG_SUBJECTS))
        for (item in subjects.iterator()) {
          put(AnonymousTag, item)
        }
        endList()
      } else {
        putNull(ContextSpecificTag(TAG_SUBJECTS))
      }
      if (targets != null) {
        startList(ContextSpecificTag(TAG_TARGETS))
        for (item in targets.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endList()
      } else {
        putNull(ContextSpecificTag(TAG_TARGETS))
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_PRIVILEGE = 1
    private const val TAG_AUTH_MODE = 2
    private const val TAG_SUBJECTS = 3
    private const val TAG_TARGETS = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(tag: Tag, tlvReader: TlvReader): AccessControlClusterAccessControlEntryStruct {
      tlvReader.enterStructure(tag)
      val privilege = tlvReader.getInt(ContextSpecificTag(TAG_PRIVILEGE))
      val authMode = tlvReader.getInt(ContextSpecificTag(TAG_AUTH_MODE))
      val subjects =
        if (!tlvReader.isNull()) {
          buildList<Long> {
            tlvReader.enterList(ContextSpecificTag(TAG_SUBJECTS))
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getLong(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SUBJECTS))
          null
        }
      val targets =
        if (!tlvReader.isNull()) {
          buildList<AccessControlClusterAccessControlTargetStruct> {
            tlvReader.enterList(ContextSpecificTag(TAG_TARGETS))
            while (!tlvReader.isEndOfContainer()) {
              add(AccessControlClusterAccessControlTargetStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TARGETS))
          null
        }
      val fabricIndex = tlvReader.getInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AccessControlClusterAccessControlEntryStruct(
        privilege,
        authMode,
        subjects,
        targets,
        fabricIndex
      )
    }
  }
}
