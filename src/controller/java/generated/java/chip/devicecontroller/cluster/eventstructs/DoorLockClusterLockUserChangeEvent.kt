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
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvParsingException
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

import java.util.Optional

class DoorLockClusterLockUserChangeEvent (
    val lockDataType: UInt,
    val dataOperationType: UInt,
    val operationSource: UInt,
    val userIndex: UInt?,
    val fabricIndex: UInt?,
    val sourceNode: ULong?,
    val dataIndex: UInt?) {
  override fun toString(): String  = buildString {
    append("DoorLockClusterLockUserChangeEvent {\n")
    append("\tlockDataType : $lockDataType\n")
    append("\tdataOperationType : $dataOperationType\n")
    append("\toperationSource : $operationSource\n")
    append("\tuserIndex : $userIndex\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("\tsourceNode : $sourceNode\n")
    append("\tdataIndex : $dataIndex\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LOCK_DATA_TYPE), lockDataType)
      put(ContextSpecificTag(TAG_DATA_OPERATION_TYPE), dataOperationType)
      put(ContextSpecificTag(TAG_OPERATION_SOURCE), operationSource)
      if (userIndex != null) {
      put(ContextSpecificTag(TAG_USER_INDEX), userIndex)
    } else {
      putNull(ContextSpecificTag(TAG_USER_INDEX))
    }
      if (fabricIndex != null) {
      put(ContextSpecificTag(TAG_FABRIC_INDEX), fabricIndex)
    } else {
      putNull(ContextSpecificTag(TAG_FABRIC_INDEX))
    }
      if (sourceNode != null) {
      put(ContextSpecificTag(TAG_SOURCE_NODE), sourceNode)
    } else {
      putNull(ContextSpecificTag(TAG_SOURCE_NODE))
    }
      if (dataIndex != null) {
      put(ContextSpecificTag(TAG_DATA_INDEX), dataIndex)
    } else {
      putNull(ContextSpecificTag(TAG_DATA_INDEX))
    }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LOCK_DATA_TYPE = 0
    private const val TAG_DATA_OPERATION_TYPE = 1
    private const val TAG_OPERATION_SOURCE = 2
    private const val TAG_USER_INDEX = 3
    private const val TAG_FABRIC_INDEX = 4
    private const val TAG_SOURCE_NODE = 5
    private const val TAG_DATA_INDEX = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader) : DoorLockClusterLockUserChangeEvent {
      tlvReader.enterStructure(tlvTag)
      val lockDataType = tlvReader.getUInt(ContextSpecificTag(TAG_LOCK_DATA_TYPE))
      val dataOperationType = tlvReader.getUInt(ContextSpecificTag(TAG_DATA_OPERATION_TYPE))
      val operationSource = tlvReader.getUInt(ContextSpecificTag(TAG_OPERATION_SOURCE))
      val userIndex = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_USER_INDEX))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_USER_INDEX))
      null
    }
      val fabricIndex = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_FABRIC_INDEX))
      null
    }
      val sourceNode = if (!tlvReader.isNull()) {
      tlvReader.getULong(ContextSpecificTag(TAG_SOURCE_NODE))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_SOURCE_NODE))
      null
    }
      val dataIndex = if (!tlvReader.isNull()) {
      tlvReader.getUInt(ContextSpecificTag(TAG_DATA_INDEX))
    } else {
      tlvReader.getNull(ContextSpecificTag(TAG_DATA_INDEX))
      null
    }
      
      tlvReader.exitContainer()

      return DoorLockClusterLockUserChangeEvent(lockDataType, dataOperationType, operationSource, userIndex, fabricIndex, sourceNode, dataIndex)
    }
  }
}
