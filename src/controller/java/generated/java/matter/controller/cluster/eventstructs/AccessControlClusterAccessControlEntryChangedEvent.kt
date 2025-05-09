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
package matter.controller.cluster.eventstructs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class AccessControlClusterAccessControlEntryChangedEvent(
  val adminNodeID: ULong?,
  val adminPasscodeID: UShort?,
  val changeType: UByte,
  val latestValue: matter.controller.cluster.structs.AccessControlClusterAccessControlEntryStruct?,
  val fabricIndex: UByte,
) {
  override fun toString(): String = buildString {
    append("AccessControlClusterAccessControlEntryChangedEvent {\n")
    append("\tadminNodeID : $adminNodeID\n")
    append("\tadminPasscodeID : $adminPasscodeID\n")
    append("\tchangeType : $changeType\n")
    append("\tlatestValue : $latestValue\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      if (adminNodeID != null) {
        put(ContextSpecificTag(TAG_ADMIN_NODE_ID), adminNodeID)
      } else {
        putNull(ContextSpecificTag(TAG_ADMIN_NODE_ID))
      }
      if (adminPasscodeID != null) {
        put(ContextSpecificTag(TAG_ADMIN_PASSCODE_ID), adminPasscodeID)
      } else {
        putNull(ContextSpecificTag(TAG_ADMIN_PASSCODE_ID))
      }
      put(ContextSpecificTag(TAG_CHANGE_TYPE), changeType)
      if (latestValue != null) {
        latestValue.toTlv(ContextSpecificTag(TAG_LATEST_VALUE), this)
      } else {
        putNull(ContextSpecificTag(TAG_LATEST_VALUE))
      }
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ADMIN_NODE_ID = 1
    private const val TAG_ADMIN_PASSCODE_ID = 2
    private const val TAG_CHANGE_TYPE = 3
    private const val TAG_LATEST_VALUE = 4
    private const val TAG_FABRIC_INDEX = 254

    fun fromTlv(
      tlvTag: Tag,
      tlvReader: TlvReader,
    ): AccessControlClusterAccessControlEntryChangedEvent {
      tlvReader.enterStructure(tlvTag)
      val adminNodeID =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_ADMIN_NODE_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ADMIN_NODE_ID))
          null
        }
      val adminPasscodeID =
        if (!tlvReader.isNull()) {
          tlvReader.getUShort(ContextSpecificTag(TAG_ADMIN_PASSCODE_ID))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ADMIN_PASSCODE_ID))
          null
        }
      val changeType = tlvReader.getUByte(ContextSpecificTag(TAG_CHANGE_TYPE))
      val latestValue =
        if (!tlvReader.isNull()) {
          matter.controller.cluster.structs.AccessControlClusterAccessControlEntryStruct.fromTlv(
            ContextSpecificTag(TAG_LATEST_VALUE),
            tlvReader,
          )
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_LATEST_VALUE))
          null
        }
      val fabricIndex = tlvReader.getUByte(ContextSpecificTag(TAG_FABRIC_INDEX))

      tlvReader.exitContainer()

      return AccessControlClusterAccessControlEntryChangedEvent(
        adminNodeID,
        adminPasscodeID,
        changeType,
        latestValue,
        fabricIndex,
      )
    }
  }
}
