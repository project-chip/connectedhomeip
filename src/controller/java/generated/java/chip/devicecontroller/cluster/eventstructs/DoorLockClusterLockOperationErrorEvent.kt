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
import java.util.Optional
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class DoorLockClusterLockOperationErrorEvent(
  val lockOperationType: UInt,
  val operationSource: UInt,
  val operationError: UInt,
  val userIndex: UInt?,
  val fabricIndex: UInt?,
  val sourceNode: ULong?,
  val credentials:
    Optional<List<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct>>?,
) {
  override fun toString(): String = buildString {
    append("DoorLockClusterLockOperationErrorEvent {\n")
    append("\tlockOperationType : $lockOperationType\n")
    append("\toperationSource : $operationSource\n")
    append("\toperationError : $operationError\n")
    append("\tuserIndex : $userIndex\n")
    append("\tfabricIndex : $fabricIndex\n")
    append("\tsourceNode : $sourceNode\n")
    append("\tcredentials : $credentials\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_LOCK_OPERATION_TYPE), lockOperationType)
      put(ContextSpecificTag(TAG_OPERATION_SOURCE), operationSource)
      put(ContextSpecificTag(TAG_OPERATION_ERROR), operationError)
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
      if (credentials != null) {
        if (credentials.isPresent) {
          val optcredentials = credentials.get()
          startArray(ContextSpecificTag(TAG_CREDENTIALS))
          for (item in optcredentials.iterator()) {
            item.toTlv(AnonymousTag, this)
          }
          endArray()
        }
      } else {
        putNull(ContextSpecificTag(TAG_CREDENTIALS))
      }
      endStructure()
    }
  }

  companion object {
    private const val TAG_LOCK_OPERATION_TYPE = 0
    private const val TAG_OPERATION_SOURCE = 1
    private const val TAG_OPERATION_ERROR = 2
    private const val TAG_USER_INDEX = 3
    private const val TAG_FABRIC_INDEX = 4
    private const val TAG_SOURCE_NODE = 5
    private const val TAG_CREDENTIALS = 6

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): DoorLockClusterLockOperationErrorEvent {
      tlvReader.enterStructure(tlvTag)
      val lockOperationType = tlvReader.getUInt(ContextSpecificTag(TAG_LOCK_OPERATION_TYPE))
      val operationSource = tlvReader.getUInt(ContextSpecificTag(TAG_OPERATION_SOURCE))
      val operationError = tlvReader.getUInt(ContextSpecificTag(TAG_OPERATION_ERROR))
      val userIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_USER_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_USER_INDEX))
          null
        }
      val fabricIndex =
        if (!tlvReader.isNull()) {
          tlvReader.getUInt(ContextSpecificTag(TAG_FABRIC_INDEX))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_FABRIC_INDEX))
          null
        }
      val sourceNode =
        if (!tlvReader.isNull()) {
          tlvReader.getULong(ContextSpecificTag(TAG_SOURCE_NODE))
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_SOURCE_NODE))
          null
        }
      val credentials =
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_CREDENTIALS))) {
            Optional.of(
              buildList<chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct> {
                tlvReader.enterArray(ContextSpecificTag(TAG_CREDENTIALS))
                while (!tlvReader.isEndOfContainer()) {
                  this.add(
                    chip.devicecontroller.cluster.structs.DoorLockClusterCredentialStruct.fromTlv(
                      AnonymousTag,
                      tlvReader,
                    )
                  )
                }
                tlvReader.exitContainer()
              }
            )
          } else {
            Optional.empty()
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_CREDENTIALS))
          null
        }

      tlvReader.exitContainer()

      return DoorLockClusterLockOperationErrorEvent(
        lockOperationType,
        operationSource,
        operationError,
        userIndex,
        fabricIndex,
        sourceNode,
        credentials,
      )
    }
  }
}
