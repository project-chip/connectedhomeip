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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AccessControlClusterAccessControlEntryStruct(
  val privilege: UInt,
  val authMode: UInt,
  val subjects: List<ULong>?,
  val targets: List<AccessControlClusterAccessControlTargetStruct>?,
  val fabricIndex: UInt,
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

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_PRIVILEGE), privilege)
      put(ContextSpecificTag(TAG_AUTH_MODE), authMode)
      if (subjects != null) {
        startArray(ContextSpecificTag(TAG_SUBJECTS))
        for (item in subjects.iterator()) {
          put(AnonymousTag, item)
        }
        endArray()
      } else {
        putNull(ContextSpecificTag(TAG_SUBJECTS))
      }
      if (targets != null) {
        startArray(ContextSpecificTag(TAG_TARGETS))
        for (item in targets.iterator()) {
          item.toTlv(AnonymousTag, this)
        }
        endArray()
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

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): AccessControlClusterAccessControlEntryStruct {
      tlvReader.enterStructure(tlvTag)
      val privilege = tlvReader.getUInt(ContextSpecificTag(TAG_PRIVILEGE))
      val authMode = tlvReader.getUInt(ContextSpecificTag(TAG_AUTH_MODE))
      val subjects =
        if (!tlvReader.isNull()) {
          buildList<ULong> {
            tlvReader.enterArray(ContextSpecificTag(TAG_SUBJECTS))
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getULong(AnonymousTag))
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
            tlvReader.enterArray(ContextSpecificTag(TAG_TARGETS))
            while (!tlvReader.isEndOfContainer()) {
              add(AccessControlClusterAccessControlTargetStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_TARGETS))
          null
        }
      val fabricIndex = tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AccessControlClusterAccessControlEntryStruct(
        privilege,
        authMode,
        subjects,
        targets,
        fabricIndex,
      )
    }
  }
}
