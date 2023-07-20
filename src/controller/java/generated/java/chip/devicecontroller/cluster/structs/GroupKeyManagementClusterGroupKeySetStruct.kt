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

class GroupKeyManagementClusterGroupKeySetStruct (
    val groupKeySetID: Int,
    val groupKeySecurityPolicy: Int,
    val epochKey0: ByteArray?,
    val epochStartTime0: Long?,
    val epochKey1: ByteArray?,
    val epochStartTime1: Long?,
    val epochKey2: ByteArray?,
    val epochStartTime2: Long?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("GroupKeyManagementClusterGroupKeySetStruct {\n")
    builder.append("\tgroupKeySetID : $groupKeySetID\n")
    builder.append("\tgroupKeySecurityPolicy : $groupKeySecurityPolicy\n")
    builder.append("\tepochKey0 : $epochKey0\n")
    builder.append("\tepochStartTime0 : $epochStartTime0\n")
    builder.append("\tepochKey1 : $epochKey1\n")
    builder.append("\tepochStartTime1 : $epochStartTime1\n")
    builder.append("\tepochKey2 : $epochKey2\n")
    builder.append("\tepochStartTime2 : $epochStartTime2\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_KEY_SET_I_D), groupKeySetID)
    tlvWriter.put(ContextSpecificTag(TAG_GROUP_KEY_SECURITY_POLICY), groupKeySecurityPolicy)
    if (epochKey0 == null) { tlvWriter.putNull(ContextSpecificTag(TAG_EPOCH_KEY0)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_EPOCH_KEY0), epochKey0)
    }
    if (epochStartTime0 == null) { tlvWriter.putNull(ContextSpecificTag(TAG_EPOCH_START_TIME0)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_EPOCH_START_TIME0), epochStartTime0)
    }
    if (epochKey1 == null) { tlvWriter.putNull(ContextSpecificTag(TAG_EPOCH_KEY1)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_EPOCH_KEY1), epochKey1)
    }
    if (epochStartTime1 == null) { tlvWriter.putNull(ContextSpecificTag(TAG_EPOCH_START_TIME1)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_EPOCH_START_TIME1), epochStartTime1)
    }
    if (epochKey2 == null) { tlvWriter.putNull(ContextSpecificTag(TAG_EPOCH_KEY2)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_EPOCH_KEY2), epochKey2)
    }
    if (epochStartTime2 == null) { tlvWriter.putNull(ContextSpecificTag(TAG_EPOCH_START_TIME2)) }
    else {
      tlvWriter.put(ContextSpecificTag(TAG_EPOCH_START_TIME2), epochStartTime2)
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_GROUP_KEY_SET_I_D = 0
    private const val TAG_GROUP_KEY_SECURITY_POLICY = 1
    private const val TAG_EPOCH_KEY0 = 2
    private const val TAG_EPOCH_START_TIME0 = 3
    private const val TAG_EPOCH_KEY1 = 4
    private const val TAG_EPOCH_START_TIME1 = 5
    private const val TAG_EPOCH_KEY2 = 6
    private const val TAG_EPOCH_START_TIME2 = 7

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : GroupKeyManagementClusterGroupKeySetStruct {
      tlvReader.enterStructure(tag)
      val groupKeySetID: Int = tlvReader.getInt(ContextSpecificTag(TAG_GROUP_KEY_SET_I_D))
      val groupKeySecurityPolicy: Int = tlvReader.getInt(ContextSpecificTag(TAG_GROUP_KEY_SECURITY_POLICY))
      val epochKey0: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(TAG_EPOCH_KEY0))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_EPOCH_KEY0))
      null
    }
      val epochStartTime0: Long? = try {
      tlvReader.getLong(ContextSpecificTag(TAG_EPOCH_START_TIME0))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_EPOCH_START_TIME0))
      null
    }
      val epochKey1: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(TAG_EPOCH_KEY1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_EPOCH_KEY1))
      null
    }
      val epochStartTime1: Long? = try {
      tlvReader.getLong(ContextSpecificTag(TAG_EPOCH_START_TIME1))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_EPOCH_START_TIME1))
      null
    }
      val epochKey2: ByteArray? = try {
      tlvReader.getByteArray(ContextSpecificTag(TAG_EPOCH_KEY2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_EPOCH_KEY2))
      null
    }
      val epochStartTime2: Long? = try {
      tlvReader.getLong(ContextSpecificTag(TAG_EPOCH_START_TIME2))
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_EPOCH_START_TIME2))
      null
    }
      
      tlvReader.exitContainer()

      return GroupKeyManagementClusterGroupKeySetStruct(groupKeySetID, groupKeySecurityPolicy, epochKey0, epochStartTime0, epochKey1, epochStartTime1, epochKey2, epochStartTime2)
    }
  }
}
