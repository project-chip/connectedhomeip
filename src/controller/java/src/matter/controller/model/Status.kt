/*
 *   Copyright (c) 2023 Project CHIP Authors
 *   All rights reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 *
 */
package matter.controller.model

/**
 * Represents information about a node, including data on all available endpoints.
 *
 * @param endpoints A mapping of endpoint IDs with the associated cluster data.
 */
data class Status(val status: Int, val clusterStatus: Int?) {
  enum class Code(val id: Int) {
    SUCCESS(0X0),
    FAILURE(0X01),
    INVALID_SUSBSCRIPTION(0X7D),
    UNSUPPORTED_ACCESS(0X7E),
    UNSUPPORTED_ENDPOINT(0X7F),
    INVALID_ACTION(0X80),
    UNSUPPORTED_COMMAND(0X81),
    DEPRECATED82(0X82),
    DEPRECATED83(0X83),
    DEPRECATED84(0X84),
    INVALID_COMMAND(0X85),
    UNSUPPORTED_ATTRIBUTE(0X86),
    CONSTRAINT_ERROR(0X87),
    UNSUPPORTED_WRITE(0X88),
    RESOURCE_EXHAUSTED(0X89),
    DEPRECATED8A(0X8A),
    NOT_FOUND(0X8B),
    UNREPORTABLE_ATTRIBUTE(0X8C),
    INVALID_DATATYPE(0X8D),
    DEPRECATED8E(0X8E),
    UNSUPPORTED_READ(0X8F),
    DEPRECATED90(0X90),
    DEPRECATED91(0X91),
    DATA_VERSION_MISMATCH(0X92),
    DEPRECATED93(0X93),
    TIMEOUT(0X94),
    RESERVED95(0X95),
    RESERVED96(0X96),
    RESERVED97(0X97),
    RESERVED98(0X98),
    RESERVED99(0X99),
    RESERVED9A(0X9A),
    BUSY(0X9C),
    ACCESS_RESTRICTED(0x9D),
    DEPRECATEDC0(0XC0),
    DEPRECATEDC1(0XC1),
    DEPRECATEDC2(0XC2),
    UNSUPPORTED_CLUSTER(0XC3),
    DEPRECATEDC4(0XC4),
    NO_UPSTREAM_SUBSRICPTION(0XC5),
    NEEDS_TIMED_INTERACTION(0XC6),
    UNSUPPORTED_EVENT(0XC7),
    PATH_EXHAUSTED(0XC8),
    TIMED_REQUEST_MISMATCH(0XC9),
    FAILSAFE_REQUIRED(0XCA),
    INVALID_IN_STATE(0XCB),
    NO_COMMAND_RESPONSE(0XCC),
    WRITE_IGNORED(0XF0)
  }

  fun getCode(): Code? {
    for (code in Code.values()) {
      if (code.id == status) {
        return code
      }
    }
    return null
  }

  override fun toString(): String = "$status/$clusterStatus/"
}
