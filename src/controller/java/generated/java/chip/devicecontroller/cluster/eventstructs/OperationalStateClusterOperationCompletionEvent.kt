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
import chip.tlv.AnonymousTag
import chip.tlv.ContextSpecificTag
import chip.tlv.Tag
import chip.tlv.TlvParsingException
import chip.tlv.TlvReader
import chip.tlv.TlvWriter

import java.util.Optional

class OperationalStateClusterOperationCompletionEvent (
    val completionErrorCode: Int,
    val totalOperationalTime: Optional<Long>?,
    val pausedTime: Optional<Long>?) {
  override fun toString() : String {
    val builder: StringBuilder = StringBuilder()
    builder.append("OperationalStateClusterOperationCompletionEvent {\n")
    builder.append("\tcompletionErrorCode : $completionErrorCode\n")
    builder.append("\ttotalOperationalTime : $totalOperationalTime\n")
    builder.append("\tpausedTime : $pausedTime\n")
    builder.append("}\n")
    return builder.toString()
  }

  fun toTlv(tag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.startStructure(tag)
    tlvWriter.put(ContextSpecificTag(TAG_COMPLETION_ERROR_CODE), completionErrorCode)
    if (totalOperationalTime == null) { tlvWriter.putNull(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME)) }
    else {
      if (totalOperationalTime.isPresent) {
      val opttotalOperationalTime = totalOperationalTime.get()
      tlvWriter.put(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME), opttotalOperationalTime)
    }
    }
    if (pausedTime == null) { tlvWriter.putNull(ContextSpecificTag(TAG_PAUSED_TIME)) }
    else {
      if (pausedTime.isPresent) {
      val optpausedTime = pausedTime.get()
      tlvWriter.put(ContextSpecificTag(TAG_PAUSED_TIME), optpausedTime)
    }
    }
    tlvWriter.endStructure()
  }

  companion object {
    private const val TAG_COMPLETION_ERROR_CODE = 0
    private const val TAG_TOTAL_OPERATIONAL_TIME = 1
    private const val TAG_PAUSED_TIME = 2

    fun fromTlv(tag: Tag, tlvReader: TlvReader) : OperationalStateClusterOperationCompletionEvent {
      tlvReader.enterStructure(tag)
      val completionErrorCode: Int = tlvReader.getInt(ContextSpecificTag(TAG_COMPLETION_ERROR_CODE))
      val totalOperationalTime: Optional<Long>? = try {
      try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_TOTAL_OPERATIONAL_TIME))
      null
    }
      val pausedTime: Optional<Long>? = try {
      try {
      Optional.of(tlvReader.getLong(ContextSpecificTag(TAG_PAUSED_TIME)))
    } catch (e: TlvParsingException) {
      Optional.empty()
    }
    } catch (e: TlvParsingException) {
      tlvReader.getNull(ContextSpecificTag(TAG_PAUSED_TIME))
      null
    }
      
      tlvReader.exitContainer()

      return OperationalStateClusterOperationCompletionEvent(completionErrorCode, totalOperationalTime, pausedTime)
    }
  }
}
