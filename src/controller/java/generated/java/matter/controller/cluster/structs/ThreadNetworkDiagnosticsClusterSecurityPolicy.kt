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
package matter.controller.cluster.structs

import matter.controller.cluster.*
import matter.tlv.ContextSpecificTag
import matter.tlv.Tag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class ThreadNetworkDiagnosticsClusterSecurityPolicy(val rotationTime: UShort, val flags: UShort) {
  override fun toString(): String = buildString {
    append("ThreadNetworkDiagnosticsClusterSecurityPolicy {\n")
    append("\trotationTime : $rotationTime\n")
    append("\tflags : $flags\n")
    append("}\n")
  }

  fun toTlv(tlvTag: Tag, tlvWriter: TlvWriter) {
    tlvWriter.apply {
      startStructure(tlvTag)
      put(ContextSpecificTag(TAG_ROTATION_TIME), rotationTime)
      put(ContextSpecificTag(TAG_FLAGS), flags)
      endStructure()
    }
  }

  companion object {
    private const val TAG_ROTATION_TIME = 0
    private const val TAG_FLAGS = 1

    fun fromTlv(tlvTag: Tag, tlvReader: TlvReader): ThreadNetworkDiagnosticsClusterSecurityPolicy {
      tlvReader.enterStructure(tlvTag)
      val rotationTime = tlvReader.getUShort(ContextSpecificTag(TAG_ROTATION_TIME))
      val flags = tlvReader.getUShort(ContextSpecificTag(TAG_FLAGS))

      tlvReader.exitContainer()

      return ThreadNetworkDiagnosticsClusterSecurityPolicy(rotationTime, flags)
    }
  }
}
