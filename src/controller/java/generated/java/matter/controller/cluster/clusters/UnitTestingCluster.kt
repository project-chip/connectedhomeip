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

package matter.controller.cluster.clusters

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import matter.controller.InvokeRequest
import matter.controller.InvokeResponse
import matter.controller.MatterController
import matter.controller.ReadData
import matter.controller.ReadRequest
import matter.controller.WriteRequest
import matter.controller.WriteRequests
import matter.controller.WriteResponse
import matter.controller.cluster.structs.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.tlv.AnonymousTag
import matter.tlv.ContextSpecificTag
import matter.tlv.TlvReader
import matter.tlv.TlvWriter

class UnitTestingCluster(private val controller: MatterController, private val endpointId: UShort) {
  class TestSpecificResponse(val returnValue: UByte)

  class TestAddArgumentsResponse(val returnValue: UByte)

  class TestSimpleArgumentResponse(val returnValue: Boolean)

  class TestStructArrayArgumentResponse(
    val arg1: List<UnitTestingClusterNestedStructList>,
    val arg2: List<UnitTestingClusterSimpleStruct>,
    val arg3: List<UByte>,
    val arg4: List<Boolean>,
    val arg5: UByte,
    val arg6: Boolean
  )

  class BooleanResponse(val value: Boolean)

  class TestListInt8UReverseResponse(val arg1: List<UByte>)

  class TestEnumsResponse(val arg1: UShort, val arg2: UByte)

  class TestNullableOptionalResponse(
    val wasPresent: Boolean,
    val wasNull: Boolean?,
    val value: UByte?,
    val originalValue: UByte?
  )

  class TestComplexNullableOptionalResponse(
    val nullableIntWasNull: Boolean,
    val nullableIntValue: UShort?,
    val optionalIntWasPresent: Boolean,
    val optionalIntValue: UShort?,
    val nullableOptionalIntWasPresent: Boolean,
    val nullableOptionalIntWasNull: Boolean?,
    val nullableOptionalIntValue: UShort?,
    val nullableStringWasNull: Boolean,
    val nullableStringValue: String?,
    val optionalStringWasPresent: Boolean,
    val optionalStringValue: String?,
    val nullableOptionalStringWasPresent: Boolean,
    val nullableOptionalStringWasNull: Boolean?,
    val nullableOptionalStringValue: String?,
    val nullableStructWasNull: Boolean,
    val nullableStructValue: UnitTestingClusterSimpleStruct?,
    val optionalStructWasPresent: Boolean,
    val optionalStructValue: UnitTestingClusterSimpleStruct?,
    val nullableOptionalStructWasPresent: Boolean,
    val nullableOptionalStructWasNull: Boolean?,
    val nullableOptionalStructValue: UnitTestingClusterSimpleStruct?,
    val nullableListWasNull: Boolean,
    val nullableListValue: List<UByte>?,
    val optionalListWasPresent: Boolean,
    val optionalListValue: List<UByte>?,
    val nullableOptionalListWasPresent: Boolean,
    val nullableOptionalListWasNull: Boolean?,
    val nullableOptionalListValue: List<UByte>?
  )

  class SimpleStructResponse(val arg1: UnitTestingClusterSimpleStruct)

  class TestEmitTestEventResponse(val value: ULong)

  class TestEmitTestFabricScopedEventResponse(val value: ULong)

  class ListInt8uAttribute(val value: List<UByte>)

  class ListOctetStringAttribute(val value: List<ByteArray>)

  class ListStructOctetStringAttribute(val value: List<UnitTestingClusterTestListStructOctet>)

  class ListNullablesAndOptionalsStructAttribute(
    val value: List<UnitTestingClusterNullablesAndOptionalsStruct>
  )

  class StructAttrAttribute(val value: UnitTestingClusterSimpleStruct)

  class ListLongOctetStringAttribute(val value: List<ByteArray>)

  class ListFabricScopedAttribute(val value: List<UnitTestingClusterTestFabricScoped>)

  class NullableBooleanAttribute(val value: Boolean?)

  class NullableBitmap8Attribute(val value: UByte?)

  class NullableBitmap16Attribute(val value: UShort?)

  class NullableBitmap32Attribute(val value: UInt?)

  class NullableBitmap64Attribute(val value: ULong?)

  class NullableInt8uAttribute(val value: UByte?)

  class NullableInt16uAttribute(val value: UShort?)

  class NullableInt24uAttribute(val value: UInt?)

  class NullableInt32uAttribute(val value: UInt?)

  class NullableInt40uAttribute(val value: ULong?)

  class NullableInt48uAttribute(val value: ULong?)

  class NullableInt56uAttribute(val value: ULong?)

  class NullableInt64uAttribute(val value: ULong?)

  class NullableInt8sAttribute(val value: Byte?)

  class NullableInt16sAttribute(val value: Short?)

  class NullableInt24sAttribute(val value: Int?)

  class NullableInt32sAttribute(val value: Int?)

  class NullableInt40sAttribute(val value: Long?)

  class NullableInt48sAttribute(val value: Long?)

  class NullableInt56sAttribute(val value: Long?)

  class NullableInt64sAttribute(val value: Long?)

  class NullableEnum8Attribute(val value: UByte?)

  class NullableEnum16Attribute(val value: UShort?)

  class NullableFloatSingleAttribute(val value: Float?)

  class NullableFloatDoubleAttribute(val value: Double?)

  class NullableOctetStringAttribute(val value: ByteArray?)

  class NullableCharStringAttribute(val value: String?)

  class NullableEnumAttrAttribute(val value: UByte?)

  class NullableStructAttribute(val value: UnitTestingClusterSimpleStruct?)

  class NullableRangeRestrictedInt8uAttribute(val value: UByte?)

  class NullableRangeRestrictedInt8sAttribute(val value: Byte?)

  class NullableRangeRestrictedInt16uAttribute(val value: UShort?)

  class NullableRangeRestrictedInt16sAttribute(val value: Short?)

  class GeneratedCommandListAttribute(val value: List<UInt>)

  class AcceptedCommandListAttribute(val value: List<UInt>)

  class EventListAttribute(val value: List<UInt>)

  class AttributeListAttribute(val value: List<UInt>)

  suspend fun test(timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun testNotHandled(timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun testSpecific(timedInvokeTimeout: Duration? = null): TestSpecificResponse {
    val commandId: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_RETURN_VALUE: Int = 0
    var returnValue_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_RETURN_VALUE)) {
        returnValue_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (returnValue_decoded == null) {
      throw IllegalStateException("returnValue not found in TLV")
    }

    tlvReader.exitContainer()

    return TestSpecificResponse(returnValue_decoded)
  }

  suspend fun testUnknownCommand(timedInvokeTimeout: Duration? = null) {
    val commandId: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun testAddArguments(
    arg1: UByte,
    arg2: UByte,
    timedInvokeTimeout: Duration? = null
  ): TestAddArgumentsResponse {
    val commandId: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)

    val TAG_ARG2_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ARG2_REQ), arg2)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_RETURN_VALUE: Int = 0
    var returnValue_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_RETURN_VALUE)) {
        returnValue_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (returnValue_decoded == null) {
      throw IllegalStateException("returnValue not found in TLV")
    }

    tlvReader.exitContainer()

    return TestAddArgumentsResponse(returnValue_decoded)
  }

  suspend fun testSimpleArgumentRequest(
    arg1: Boolean,
    timedInvokeTimeout: Duration? = null
  ): TestSimpleArgumentResponse {
    val commandId: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_RETURN_VALUE: Int = 0
    var returnValue_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_RETURN_VALUE)) {
        returnValue_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (returnValue_decoded == null) {
      throw IllegalStateException("returnValue not found in TLV")
    }

    tlvReader.exitContainer()

    return TestSimpleArgumentResponse(returnValue_decoded)
  }

  suspend fun testStructArrayArgumentRequest(
    arg1: List<UnitTestingClusterNestedStructList>,
    arg2: List<UnitTestingClusterSimpleStruct>,
    arg3: List<UByte>,
    arg4: List<Boolean>,
    arg5: UByte,
    arg6: Boolean,
    timedInvokeTimeout: Duration? = null
  ): TestStructArrayArgumentResponse {
    val commandId: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG1_REQ))
    for (item in arg1.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val TAG_ARG2_REQ: Int = 1
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG2_REQ))
    for (item in arg2.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val TAG_ARG3_REQ: Int = 2
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG3_REQ))
    for (item in arg3.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()

    val TAG_ARG4_REQ: Int = 3
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG4_REQ))
    for (item in arg4.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()

    val TAG_ARG5_REQ: Int = 4
    tlvWriter.put(ContextSpecificTag(TAG_ARG5_REQ), arg5)

    val TAG_ARG6_REQ: Int = 5
    tlvWriter.put(ContextSpecificTag(TAG_ARG6_REQ), arg6)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    var arg1_decoded: List<UnitTestingClusterNestedStructList>? = null

    val TAG_ARG2: Int = 1
    var arg2_decoded: List<UnitTestingClusterSimpleStruct>? = null

    val TAG_ARG3: Int = 2
    var arg3_decoded: List<UByte>? = null

    val TAG_ARG4: Int = 3
    var arg4_decoded: List<Boolean>? = null

    val TAG_ARG5: Int = 4
    var arg5_decoded: UByte? = null

    val TAG_ARG6: Int = 5
    var arg6_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_ARG1)) {
        arg1_decoded =
          buildList<UnitTestingClusterNestedStructList> {
            tlvReader.enterArray(tag)
            while (!tlvReader.isEndOfContainer()) {
              add(UnitTestingClusterNestedStructList.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
      }

      if (tag == ContextSpecificTag(TAG_ARG2)) {
        arg2_decoded =
          buildList<UnitTestingClusterSimpleStruct> {
            tlvReader.enterArray(tag)
            while (!tlvReader.isEndOfContainer()) {
              add(UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
            }
            tlvReader.exitContainer()
          }
      }

      if (tag == ContextSpecificTag(TAG_ARG3)) {
        arg3_decoded =
          buildList<UByte> {
            tlvReader.enterArray(tag)
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getUByte(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
      }

      if (tag == ContextSpecificTag(TAG_ARG4)) {
        arg4_decoded =
          buildList<Boolean> {
            tlvReader.enterArray(tag)
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getBoolean(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
      }

      if (tag == ContextSpecificTag(TAG_ARG5)) {
        arg5_decoded = tlvReader.getUByte(tag)
      }

      if (tag == ContextSpecificTag(TAG_ARG6)) {
        arg6_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (arg1_decoded == null) {
      throw IllegalStateException("arg1 not found in TLV")
    }

    if (arg2_decoded == null) {
      throw IllegalStateException("arg2 not found in TLV")
    }

    if (arg3_decoded == null) {
      throw IllegalStateException("arg3 not found in TLV")
    }

    if (arg4_decoded == null) {
      throw IllegalStateException("arg4 not found in TLV")
    }

    if (arg5_decoded == null) {
      throw IllegalStateException("arg5 not found in TLV")
    }

    if (arg6_decoded == null) {
      throw IllegalStateException("arg6 not found in TLV")
    }

    tlvReader.exitContainer()

    return TestStructArrayArgumentResponse(
      arg1_decoded,
      arg2_decoded,
      arg3_decoded,
      arg4_decoded,
      arg5_decoded,
      arg6_decoded
    )
  }

  suspend fun testStructArgumentRequest(
    arg1: UnitTestingClusterSimpleStruct,
    timedInvokeTimeout: Duration? = null
  ): BooleanResponse {
    val commandId: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testNestedStructArgumentRequest(
    arg1: UnitTestingClusterNestedStruct,
    timedInvokeTimeout: Duration? = null
  ): BooleanResponse {
    val commandId: UInt = 8u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListStructArgumentRequest(
    arg1: List<UnitTestingClusterSimpleStruct>,
    timedInvokeTimeout: Duration? = null
  ): BooleanResponse {
    val commandId: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG1_REQ))
    for (item in arg1.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListInt8UArgumentRequest(
    arg1: List<UByte>,
    timedInvokeTimeout: Duration? = null
  ): BooleanResponse {
    val commandId: UInt = 10u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG1_REQ))
    for (item in arg1.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testNestedStructListArgumentRequest(
    arg1: UnitTestingClusterNestedStructList,
    timedInvokeTimeout: Duration? = null
  ): BooleanResponse {
    val commandId: UInt = 11u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListNestedStructListArgumentRequest(
    arg1: List<UnitTestingClusterNestedStructList>,
    timedInvokeTimeout: Duration? = null
  ): BooleanResponse {
    val commandId: UInt = 12u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG1_REQ))
    for (item in arg1.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: Boolean? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getBoolean(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListInt8UReverseRequest(
    arg1: List<UByte>,
    timedInvokeTimeout: Duration? = null
  ): TestListInt8UReverseResponse {
    val commandId: UInt = 13u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.startArray(ContextSpecificTag(TAG_ARG1_REQ))
    for (item in arg1.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    var arg1_decoded: List<UByte>? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_ARG1)) {
        arg1_decoded =
          buildList<UByte> {
            tlvReader.enterArray(tag)
            while (!tlvReader.isEndOfContainer()) {
              add(tlvReader.getUByte(AnonymousTag))
            }
            tlvReader.exitContainer()
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (arg1_decoded == null) {
      throw IllegalStateException("arg1 not found in TLV")
    }

    tlvReader.exitContainer()

    return TestListInt8UReverseResponse(arg1_decoded)
  }

  suspend fun testEnumsRequest(
    arg1: UShort,
    arg2: UByte,
    timedInvokeTimeout: Duration? = null
  ): TestEnumsResponse {
    val commandId: UInt = 14u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)

    val TAG_ARG2_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ARG2_REQ), arg2)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    var arg1_decoded: UShort? = null

    val TAG_ARG2: Int = 1
    var arg2_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_ARG1)) {
        arg1_decoded = tlvReader.getUShort(tag)
      }

      if (tag == ContextSpecificTag(TAG_ARG2)) {
        arg2_decoded = tlvReader.getUByte(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (arg1_decoded == null) {
      throw IllegalStateException("arg1 not found in TLV")
    }

    if (arg2_decoded == null) {
      throw IllegalStateException("arg2 not found in TLV")
    }

    tlvReader.exitContainer()

    return TestEnumsResponse(arg1_decoded, arg2_decoded)
  }

  suspend fun testNullableOptionalRequest(
    arg1: UByte?,
    timedInvokeTimeout: Duration? = null
  ): TestNullableOptionalResponse {
    val commandId: UInt = 15u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1?.let { tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_WAS_PRESENT: Int = 0
    var wasPresent_decoded: Boolean? = null

    val TAG_WAS_NULL: Int = 1
    var wasNull_decoded: Boolean? = null

    val TAG_VALUE: Int = 2
    var value_decoded: UByte? = null

    val TAG_ORIGINAL_VALUE: Int = 3
    var originalValue_decoded: UByte? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_WAS_PRESENT)) {
        wasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_WAS_NULL)) {
        wasNull_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getBoolean(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUByte(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_ORIGINAL_VALUE)) {
        originalValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (!tlvReader.isNull()) {
              if (tlvReader.isNextTag(tag)) {
                tlvReader.getUByte(tag)
              } else {
                null
              }
            } else {
              tlvReader.getNull(tag)
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (wasPresent_decoded == null) {
      throw IllegalStateException("wasPresent not found in TLV")
    }

    tlvReader.exitContainer()

    return TestNullableOptionalResponse(
      wasPresent_decoded,
      wasNull_decoded,
      value_decoded,
      originalValue_decoded
    )
  }

  suspend fun testComplexNullableOptionalRequest(
    nullableInt: UShort?,
    optionalInt: UShort?,
    nullableOptionalInt: UShort?,
    nullableString: String?,
    optionalString: String?,
    nullableOptionalString: String?,
    nullableStruct: UnitTestingClusterSimpleStruct?,
    optionalStruct: UnitTestingClusterSimpleStruct?,
    nullableOptionalStruct: UnitTestingClusterSimpleStruct?,
    nullableList: List<UByte>?,
    optionalList: List<UByte>?,
    nullableOptionalList: List<UByte>?,
    timedInvokeTimeout: Duration? = null
  ): TestComplexNullableOptionalResponse {
    val commandId: UInt = 16u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_NULLABLE_INT_REQ: Int = 0
    nullableInt?.let { tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_INT_REQ), nullableInt) }

    val TAG_OPTIONAL_INT_REQ: Int = 1
    optionalInt?.let { tlvWriter.put(ContextSpecificTag(TAG_OPTIONAL_INT_REQ), optionalInt) }

    val TAG_NULLABLE_OPTIONAL_INT_REQ: Int = 2
    nullableOptionalInt?.let {
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_REQ), nullableOptionalInt)
    }

    val TAG_NULLABLE_STRING_REQ: Int = 3
    nullableString?.let {
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_STRING_REQ), nullableString)
    }

    val TAG_OPTIONAL_STRING_REQ: Int = 4
    optionalString?.let {
      tlvWriter.put(ContextSpecificTag(TAG_OPTIONAL_STRING_REQ), optionalString)
    }

    val TAG_NULLABLE_OPTIONAL_STRING_REQ: Int = 5
    nullableOptionalString?.let {
      tlvWriter.put(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_REQ), nullableOptionalString)
    }

    val TAG_NULLABLE_STRUCT_REQ: Int = 6
    nullableStruct?.let {
      nullableStruct.toTlv(ContextSpecificTag(TAG_NULLABLE_STRUCT_REQ), tlvWriter)
    }

    val TAG_OPTIONAL_STRUCT_REQ: Int = 7
    optionalStruct?.let {
      optionalStruct.toTlv(ContextSpecificTag(TAG_OPTIONAL_STRUCT_REQ), tlvWriter)
    }

    val TAG_NULLABLE_OPTIONAL_STRUCT_REQ: Int = 8
    nullableOptionalStruct?.let {
      nullableOptionalStruct.toTlv(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_REQ), tlvWriter)
    }

    val TAG_NULLABLE_LIST_REQ: Int = 9
    nullableList?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_NULLABLE_LIST_REQ))
      for (item in nullableList.iterator()) {
        tlvWriter.put(AnonymousTag, item)
      }
      tlvWriter.endArray()
    }

    val TAG_OPTIONAL_LIST_REQ: Int = 10
    optionalList?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_OPTIONAL_LIST_REQ))
      for (item in optionalList.iterator()) {
        tlvWriter.put(AnonymousTag, item)
      }
      tlvWriter.endArray()
    }

    val TAG_NULLABLE_OPTIONAL_LIST_REQ: Int = 11
    nullableOptionalList?.let {
      tlvWriter.startArray(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_REQ))
      for (item in nullableOptionalList.iterator()) {
        tlvWriter.put(AnonymousTag, item)
      }
      tlvWriter.endArray()
    }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NULLABLE_INT_WAS_NULL: Int = 0
    var nullableIntWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_INT_VALUE: Int = 1
    var nullableIntValue_decoded: UShort? = null

    val TAG_OPTIONAL_INT_WAS_PRESENT: Int = 2
    var optionalIntWasPresent_decoded: Boolean? = null

    val TAG_OPTIONAL_INT_VALUE: Int = 3
    var optionalIntValue_decoded: UShort? = null

    val TAG_NULLABLE_OPTIONAL_INT_WAS_PRESENT: Int = 4
    var nullableOptionalIntWasPresent_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_INT_WAS_NULL: Int = 5
    var nullableOptionalIntWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_INT_VALUE: Int = 6
    var nullableOptionalIntValue_decoded: UShort? = null

    val TAG_NULLABLE_STRING_WAS_NULL: Int = 7
    var nullableStringWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_STRING_VALUE: Int = 8
    var nullableStringValue_decoded: String? = null

    val TAG_OPTIONAL_STRING_WAS_PRESENT: Int = 9
    var optionalStringWasPresent_decoded: Boolean? = null

    val TAG_OPTIONAL_STRING_VALUE: Int = 10
    var optionalStringValue_decoded: String? = null

    val TAG_NULLABLE_OPTIONAL_STRING_WAS_PRESENT: Int = 11
    var nullableOptionalStringWasPresent_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_STRING_WAS_NULL: Int = 12
    var nullableOptionalStringWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_STRING_VALUE: Int = 13
    var nullableOptionalStringValue_decoded: String? = null

    val TAG_NULLABLE_STRUCT_WAS_NULL: Int = 14
    var nullableStructWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_STRUCT_VALUE: Int = 15
    var nullableStructValue_decoded: UnitTestingClusterSimpleStruct? = null

    val TAG_OPTIONAL_STRUCT_WAS_PRESENT: Int = 16
    var optionalStructWasPresent_decoded: Boolean? = null

    val TAG_OPTIONAL_STRUCT_VALUE: Int = 17
    var optionalStructValue_decoded: UnitTestingClusterSimpleStruct? = null

    val TAG_NULLABLE_OPTIONAL_STRUCT_WAS_PRESENT: Int = 18
    var nullableOptionalStructWasPresent_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_STRUCT_WAS_NULL: Int = 19
    var nullableOptionalStructWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_STRUCT_VALUE: Int = 20
    var nullableOptionalStructValue_decoded: UnitTestingClusterSimpleStruct? = null

    val TAG_NULLABLE_LIST_WAS_NULL: Int = 21
    var nullableListWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_LIST_VALUE: Int = 22
    var nullableListValue_decoded: List<UByte>? = null

    val TAG_OPTIONAL_LIST_WAS_PRESENT: Int = 23
    var optionalListWasPresent_decoded: Boolean? = null

    val TAG_OPTIONAL_LIST_VALUE: Int = 24
    var optionalListValue_decoded: List<UByte>? = null

    val TAG_NULLABLE_OPTIONAL_LIST_WAS_PRESENT: Int = 25
    var nullableOptionalListWasPresent_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_LIST_WAS_NULL: Int = 26
    var nullableOptionalListWasNull_decoded: Boolean? = null

    val TAG_NULLABLE_OPTIONAL_LIST_VALUE: Int = 27
    var nullableOptionalListValue_decoded: List<UByte>? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_NULLABLE_INT_WAS_NULL)) {
        nullableIntWasNull_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_INT_VALUE)) {
        nullableIntValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUShort(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_INT_WAS_PRESENT)) {
        optionalIntWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_INT_VALUE)) {
        optionalIntValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUShort(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_WAS_PRESENT)) {
        nullableOptionalIntWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_WAS_NULL)) {
        nullableOptionalIntWasNull_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getBoolean(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_VALUE)) {
        nullableOptionalIntValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getUShort(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_STRING_WAS_NULL)) {
        nullableStringWasNull_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_STRING_VALUE)) {
        nullableStringValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_STRING_WAS_PRESENT)) {
        optionalStringWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_STRING_VALUE)) {
        optionalStringValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_WAS_PRESENT)) {
        nullableOptionalStringWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_WAS_NULL)) {
        nullableOptionalStringWasNull_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getBoolean(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_VALUE)) {
        nullableOptionalStringValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getString(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_STRUCT_WAS_NULL)) {
        nullableStructWasNull_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_STRUCT_VALUE)) {
        nullableStructValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              UnitTestingClusterSimpleStruct.fromTlv(tag, tlvReader)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_STRUCT_WAS_PRESENT)) {
        optionalStructWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_STRUCT_VALUE)) {
        optionalStructValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              UnitTestingClusterSimpleStruct.fromTlv(tag, tlvReader)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_WAS_PRESENT)) {
        nullableOptionalStructWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_WAS_NULL)) {
        nullableOptionalStructWasNull_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getBoolean(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_VALUE)) {
        nullableOptionalStructValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              UnitTestingClusterSimpleStruct.fromTlv(tag, tlvReader)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_LIST_WAS_NULL)) {
        nullableListWasNull_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_LIST_VALUE)) {
        nullableListValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<UByte> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_LIST_WAS_PRESENT)) {
        optionalListWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_OPTIONAL_LIST_VALUE)) {
        optionalListValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<UByte> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_WAS_PRESENT)) {
        nullableOptionalListWasPresent_decoded = tlvReader.getBoolean(tag)
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_WAS_NULL)) {
        nullableOptionalListWasNull_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              tlvReader.getBoolean(tag)
            } else {
              null
            }
          }
      }

      if (tag == ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_VALUE)) {
        nullableOptionalListValue_decoded =
          if (tlvReader.isNull()) {
            tlvReader.getNull(tag)
            null
          } else {
            if (tlvReader.isNextTag(tag)) {
              buildList<UByte> {
                tlvReader.enterArray(tag)
                while (!tlvReader.isEndOfContainer()) {
                  add(tlvReader.getUByte(AnonymousTag))
                }
                tlvReader.exitContainer()
              }
            } else {
              null
            }
          }
      } else {
        tlvReader.skipElement()
      }
    }

    if (nullableIntWasNull_decoded == null) {
      throw IllegalStateException("nullableIntWasNull not found in TLV")
    }

    if (optionalIntWasPresent_decoded == null) {
      throw IllegalStateException("optionalIntWasPresent not found in TLV")
    }

    if (nullableOptionalIntWasPresent_decoded == null) {
      throw IllegalStateException("nullableOptionalIntWasPresent not found in TLV")
    }

    if (nullableStringWasNull_decoded == null) {
      throw IllegalStateException("nullableStringWasNull not found in TLV")
    }

    if (optionalStringWasPresent_decoded == null) {
      throw IllegalStateException("optionalStringWasPresent not found in TLV")
    }

    if (nullableOptionalStringWasPresent_decoded == null) {
      throw IllegalStateException("nullableOptionalStringWasPresent not found in TLV")
    }

    if (nullableStructWasNull_decoded == null) {
      throw IllegalStateException("nullableStructWasNull not found in TLV")
    }

    if (optionalStructWasPresent_decoded == null) {
      throw IllegalStateException("optionalStructWasPresent not found in TLV")
    }

    if (nullableOptionalStructWasPresent_decoded == null) {
      throw IllegalStateException("nullableOptionalStructWasPresent not found in TLV")
    }

    if (nullableListWasNull_decoded == null) {
      throw IllegalStateException("nullableListWasNull not found in TLV")
    }

    if (optionalListWasPresent_decoded == null) {
      throw IllegalStateException("optionalListWasPresent not found in TLV")
    }

    if (nullableOptionalListWasPresent_decoded == null) {
      throw IllegalStateException("nullableOptionalListWasPresent not found in TLV")
    }

    tlvReader.exitContainer()

    return TestComplexNullableOptionalResponse(
      nullableIntWasNull_decoded,
      nullableIntValue_decoded,
      optionalIntWasPresent_decoded,
      optionalIntValue_decoded,
      nullableOptionalIntWasPresent_decoded,
      nullableOptionalIntWasNull_decoded,
      nullableOptionalIntValue_decoded,
      nullableStringWasNull_decoded,
      nullableStringValue_decoded,
      optionalStringWasPresent_decoded,
      optionalStringValue_decoded,
      nullableOptionalStringWasPresent_decoded,
      nullableOptionalStringWasNull_decoded,
      nullableOptionalStringValue_decoded,
      nullableStructWasNull_decoded,
      nullableStructValue_decoded,
      optionalStructWasPresent_decoded,
      optionalStructValue_decoded,
      nullableOptionalStructWasPresent_decoded,
      nullableOptionalStructWasNull_decoded,
      nullableOptionalStructValue_decoded,
      nullableListWasNull_decoded,
      nullableListValue_decoded,
      optionalListWasPresent_decoded,
      optionalListValue_decoded,
      nullableOptionalListWasPresent_decoded,
      nullableOptionalListWasNull_decoded,
      nullableOptionalListValue_decoded
    )
  }

  suspend fun simpleStructEchoRequest(
    arg1: UnitTestingClusterSimpleStruct,
    timedInvokeTimeout: Duration? = null
  ): SimpleStructResponse {
    val commandId: UInt = 17u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    var arg1_decoded: UnitTestingClusterSimpleStruct? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_ARG1)) {
        arg1_decoded = UnitTestingClusterSimpleStruct.fromTlv(tag, tlvReader)
      } else {
        tlvReader.skipElement()
      }
    }

    if (arg1_decoded == null) {
      throw IllegalStateException("arg1 not found in TLV")
    }

    tlvReader.exitContainer()

    return SimpleStructResponse(arg1_decoded)
  }

  suspend fun timedInvokeRequest(timedInvokeTimeout: Duration) {
    val commandId: UInt = 18u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun testSimpleOptionalArgumentRequest(
    arg1: Boolean?,
    timedInvokeTimeout: Duration? = null
  ) {
    val commandId: UInt = 19u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1?.let { tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")
  }

  suspend fun testEmitTestEventRequest(
    arg1: UByte,
    arg2: UByte,
    arg3: Boolean,
    timedInvokeTimeout: Duration? = null
  ): TestEmitTestEventResponse {
    val commandId: UInt = 20u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)

    val TAG_ARG2_REQ: Int = 1
    tlvWriter.put(ContextSpecificTag(TAG_ARG2_REQ), arg2)

    val TAG_ARG3_REQ: Int = 2
    tlvWriter.put(ContextSpecificTag(TAG_ARG3_REQ), arg3)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: ULong? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getULong(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return TestEmitTestEventResponse(value_decoded)
  }

  suspend fun testEmitTestFabricScopedEventRequest(
    arg1: UByte,
    timedInvokeTimeout: Duration? = null
  ): TestEmitTestFabricScopedEventResponse {
    val commandId: UInt = 21u

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timedInvokeTimeout
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.FINE, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    var value_decoded: ULong? = null

    while (!tlvReader.isEndOfContainer()) {
      val tag = tlvReader.peekElement().tag

      if (tag == ContextSpecificTag(TAG_VALUE)) {
        value_decoded = tlvReader.getULong(tag)
      } else {
        tlvReader.skipElement()
      }
    }

    if (value_decoded == null) {
      throw IllegalStateException("value not found in TLV")
    }

    tlvReader.exitContainer()

    return TestEmitTestFabricScopedEventResponse(value_decoded)
  }

  suspend fun readBooleanAttribute(): Boolean {
    val ATTRIBUTE_ID: UInt = 0u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Boolean attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun writeBooleanAttribute(value: Boolean, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 0u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readBitmap8Attribute(): UByte {
    val ATTRIBUTE_ID: UInt = 1u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Bitmap8 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeBitmap8Attribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 1u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readBitmap16Attribute(): UShort {
    val ATTRIBUTE_ID: UInt = 2u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Bitmap16 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeBitmap16Attribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 2u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readBitmap32Attribute(): UInt {
    val ATTRIBUTE_ID: UInt = 3u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Bitmap32 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeBitmap32Attribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 3u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readBitmap64Attribute(): ULong {
    val ATTRIBUTE_ID: UInt = 4u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Bitmap64 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeBitmap64Attribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 4u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt8uAttribute(): UByte {
    val ATTRIBUTE_ID: UInt = 5u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int8u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt8uAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 5u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt16uAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 6u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int16u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt16uAttribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 6u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt24uAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 7u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int24u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt24uAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 7u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt32uAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 8u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int32u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt32uAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 8u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt40uAttribute(): ULong {
    val ATTRIBUTE_ID: UInt = 9u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int40u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt40uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 9u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt48uAttribute(): ULong {
    val ATTRIBUTE_ID: UInt = 10u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int48u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt48uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 10u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt56uAttribute(): ULong {
    val ATTRIBUTE_ID: UInt = 11u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int56u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt56uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 11u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt64uAttribute(): ULong {
    val ATTRIBUTE_ID: UInt = 12u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int64u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt64uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 12u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt8sAttribute(): Byte {
    val ATTRIBUTE_ID: UInt = 13u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int8s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Byte = tlvReader.getByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt8sAttribute(value: Byte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 13u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt16sAttribute(): Short {
    val ATTRIBUTE_ID: UInt = 14u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int16s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Short = tlvReader.getShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt16sAttribute(value: Short, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 14u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt24sAttribute(): Int {
    val ATTRIBUTE_ID: UInt = 15u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int24s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Int = tlvReader.getInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt24sAttribute(value: Int, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 15u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt32sAttribute(): Int {
    val ATTRIBUTE_ID: UInt = 16u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int32s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Int = tlvReader.getInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt32sAttribute(value: Int, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt40sAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 17u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int40s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt40sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 17u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt48sAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 18u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int48s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt48sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 18u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt56sAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 19u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int56s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt56sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 19u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readInt64sAttribute(): Long {
    val ATTRIBUTE_ID: UInt = 20u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Int64s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long = tlvReader.getLong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeInt64sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 20u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readEnum8Attribute(): UByte {
    val ATTRIBUTE_ID: UInt = 21u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Enum8 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeEnum8Attribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 21u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readEnum16Attribute(): UShort {
    val ATTRIBUTE_ID: UInt = 22u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Enum16 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeEnum16Attribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 22u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readFloatSingleAttribute(): Float {
    val ATTRIBUTE_ID: UInt = 23u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Floatsingle attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Float = tlvReader.getFloat(AnonymousTag)

    return decodedValue
  }

  suspend fun writeFloatSingleAttribute(value: Float, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 23u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readFloatDoubleAttribute(): Double {
    val ATTRIBUTE_ID: UInt = 24u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Floatdouble attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Double = tlvReader.getDouble(AnonymousTag)

    return decodedValue
  }

  suspend fun writeFloatDoubleAttribute(value: Double, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 24u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readOctetStringAttribute(): ByteArray {
    val ATTRIBUTE_ID: UInt = 25u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Octetstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray = tlvReader.getByteArray(AnonymousTag)

    return decodedValue
  }

  suspend fun writeOctetStringAttribute(value: ByteArray, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 25u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readListInt8uAttribute(): ListInt8uAttribute {
    val ATTRIBUTE_ID: UInt = 26u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Listint8u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UByte> =
      buildList<UByte> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUByte(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return ListInt8uAttribute(decodedValue)
  }

  suspend fun writeListInt8uAttribute(value: List<UByte>, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 26u

    val tlvWriter = TlvWriter()
    tlvWriter.startArray(AnonymousTag)
    for (item in value.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readListOctetStringAttribute(): ListOctetStringAttribute {
    val ATTRIBUTE_ID: UInt = 27u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Listoctetstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ByteArray> =
      buildList<ByteArray> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getByteArray(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return ListOctetStringAttribute(decodedValue)
  }

  suspend fun writeListOctetStringAttribute(
    value: List<ByteArray>,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 27u

    val tlvWriter = TlvWriter()
    tlvWriter.startArray(AnonymousTag)
    for (item in value.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readListStructOctetStringAttribute(): ListStructOctetStringAttribute {
    val ATTRIBUTE_ID: UInt = 28u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Liststructoctetstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UnitTestingClusterTestListStructOctet> =
      buildList<UnitTestingClusterTestListStructOctet> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(UnitTestingClusterTestListStructOctet.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return ListStructOctetStringAttribute(decodedValue)
  }

  suspend fun writeListStructOctetStringAttribute(
    value: List<UnitTestingClusterTestListStructOctet>,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 28u

    val tlvWriter = TlvWriter()
    tlvWriter.startArray(AnonymousTag)
    for (item in value.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readLongOctetStringAttribute(): ByteArray {
    val ATTRIBUTE_ID: UInt = 29u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Longoctetstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray = tlvReader.getByteArray(AnonymousTag)

    return decodedValue
  }

  suspend fun writeLongOctetStringAttribute(value: ByteArray, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 29u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readCharStringAttribute(): String {
    val ATTRIBUTE_ID: UInt = 30u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Charstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String = tlvReader.getString(AnonymousTag)

    return decodedValue
  }

  suspend fun writeCharStringAttribute(value: String, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 30u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readLongCharStringAttribute(): String {
    val ATTRIBUTE_ID: UInt = 31u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Longcharstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String = tlvReader.getString(AnonymousTag)

    return decodedValue
  }

  suspend fun writeLongCharStringAttribute(value: String, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 31u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readEpochUsAttribute(): ULong {
    val ATTRIBUTE_ID: UInt = 32u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Epochus attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

    return decodedValue
  }

  suspend fun writeEpochUsAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 32u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readEpochSAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 33u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Epochs attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun writeEpochSAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 33u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readVendorIdAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 34u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Vendorid attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeVendorIdAttribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 34u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readListNullablesAndOptionalsStructAttribute():
    ListNullablesAndOptionalsStructAttribute {
    val ATTRIBUTE_ID: UInt = 35u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) {
      "Listnullablesandoptionalsstruct attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UnitTestingClusterNullablesAndOptionalsStruct> =
      buildList<UnitTestingClusterNullablesAndOptionalsStruct> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(UnitTestingClusterNullablesAndOptionalsStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return ListNullablesAndOptionalsStructAttribute(decodedValue)
  }

  suspend fun writeListNullablesAndOptionalsStructAttribute(
    value: List<UnitTestingClusterNullablesAndOptionalsStruct>,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 35u

    val tlvWriter = TlvWriter()
    tlvWriter.startArray(AnonymousTag)
    for (item in value.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readEnumAttrAttribute(): UByte {
    val ATTRIBUTE_ID: UInt = 36u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Enumattr attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeEnumAttrAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 36u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readStructAttrAttribute(): StructAttrAttribute {
    val ATTRIBUTE_ID: UInt = 37u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Structattr attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UnitTestingClusterSimpleStruct =
      UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader)

    return StructAttrAttribute(decodedValue)
  }

  suspend fun writeStructAttrAttribute(
    value: UnitTestingClusterSimpleStruct,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 37u

    val tlvWriter = TlvWriter()
    value.toTlv(AnonymousTag, tlvWriter)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readRangeRestrictedInt8uAttribute(): UByte {
    val ATTRIBUTE_ID: UInt = 38u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Rangerestrictedint8u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeRangeRestrictedInt8uAttribute(
    value: UByte,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 38u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readRangeRestrictedInt8sAttribute(): Byte {
    val ATTRIBUTE_ID: UInt = 39u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Rangerestrictedint8s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Byte = tlvReader.getByte(AnonymousTag)

    return decodedValue
  }

  suspend fun writeRangeRestrictedInt8sAttribute(value: Byte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 39u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readRangeRestrictedInt16uAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 40u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Rangerestrictedint16u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeRangeRestrictedInt16uAttribute(
    value: UShort,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 40u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readRangeRestrictedInt16sAttribute(): Short {
    val ATTRIBUTE_ID: UInt = 41u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Rangerestrictedint16s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Short = tlvReader.getShort(AnonymousTag)

    return decodedValue
  }

  suspend fun writeRangeRestrictedInt16sAttribute(
    value: Short,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 41u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readListLongOctetStringAttribute(): ListLongOctetStringAttribute {
    val ATTRIBUTE_ID: UInt = 42u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Listlongoctetstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<ByteArray> =
      buildList<ByteArray> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getByteArray(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return ListLongOctetStringAttribute(decodedValue)
  }

  suspend fun writeListLongOctetStringAttribute(
    value: List<ByteArray>,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 42u

    val tlvWriter = TlvWriter()
    tlvWriter.startArray(AnonymousTag)
    for (item in value.iterator()) {
      tlvWriter.put(AnonymousTag, item)
    }
    tlvWriter.endArray()

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readListFabricScopedAttribute(): ListFabricScopedAttribute {
    val ATTRIBUTE_ID: UInt = 43u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Listfabricscoped attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UnitTestingClusterTestFabricScoped> =
      buildList<UnitTestingClusterTestFabricScoped> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(UnitTestingClusterTestFabricScoped.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    return ListFabricScopedAttribute(decodedValue)
  }

  suspend fun writeListFabricScopedAttribute(
    value: List<UnitTestingClusterTestFabricScoped>,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 43u

    val tlvWriter = TlvWriter()
    tlvWriter.startArray(AnonymousTag)
    for (item in value.iterator()) {
      item.toTlv(AnonymousTag, tlvWriter)
    }
    tlvWriter.endArray()

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readTimedWriteBooleanAttribute(): Boolean {
    val ATTRIBUTE_ID: UInt = 48u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Timedwriteboolean attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun writeTimedWriteBooleanAttribute(value: Boolean, timedWriteTimeout: Duration) {
    val ATTRIBUTE_ID: UInt = 48u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readGeneralErrorBooleanAttribute(): Boolean {
    val ATTRIBUTE_ID: UInt = 49u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Generalerrorboolean attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun writeGeneralErrorBooleanAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 49u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readClusterErrorBooleanAttribute(): Boolean {
    val ATTRIBUTE_ID: UInt = 50u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Clustererrorboolean attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

    return decodedValue
  }

  suspend fun writeClusterErrorBooleanAttribute(
    value: Boolean,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 50u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readUnsupportedAttribute(): Boolean? {
    val ATTRIBUTE_ID: UInt = 255u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Unsupported attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeUnsupportedAttribute(value: Boolean, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 255u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableBooleanAttribute(): NullableBooleanAttribute {
    val ATTRIBUTE_ID: UInt = 16384u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableboolean attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Boolean? =
      if (!tlvReader.isNull()) {
        tlvReader.getBoolean(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableBooleanAttribute(decodedValue)
  }

  suspend fun writeNullableBooleanAttribute(value: Boolean, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16384u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableBitmap8Attribute(): NullableBitmap8Attribute {
    val ATTRIBUTE_ID: UInt = 16385u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablebitmap8 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableBitmap8Attribute(decodedValue)
  }

  suspend fun writeNullableBitmap8Attribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16385u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableBitmap16Attribute(): NullableBitmap16Attribute {
    val ATTRIBUTE_ID: UInt = 16386u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablebitmap16 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (!tlvReader.isNull()) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableBitmap16Attribute(decodedValue)
  }

  suspend fun writeNullableBitmap16Attribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16386u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableBitmap32Attribute(): NullableBitmap32Attribute {
    val ATTRIBUTE_ID: UInt = 16387u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablebitmap32 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableBitmap32Attribute(decodedValue)
  }

  suspend fun writeNullableBitmap32Attribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16387u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableBitmap64Attribute(): NullableBitmap64Attribute {
    val ATTRIBUTE_ID: UInt = 16388u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablebitmap64 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong? =
      if (!tlvReader.isNull()) {
        tlvReader.getULong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableBitmap64Attribute(decodedValue)
  }

  suspend fun writeNullableBitmap64Attribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16388u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt8uAttribute(): NullableInt8uAttribute {
    val ATTRIBUTE_ID: UInt = 16389u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint8u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt8uAttribute(decodedValue)
  }

  suspend fun writeNullableInt8uAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16389u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt16uAttribute(): NullableInt16uAttribute {
    val ATTRIBUTE_ID: UInt = 16390u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint16u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (!tlvReader.isNull()) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt16uAttribute(decodedValue)
  }

  suspend fun writeNullableInt16uAttribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16390u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt24uAttribute(): NullableInt24uAttribute {
    val ATTRIBUTE_ID: UInt = 16391u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint24u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt24uAttribute(decodedValue)
  }

  suspend fun writeNullableInt24uAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16391u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt32uAttribute(): NullableInt32uAttribute {
    val ATTRIBUTE_ID: UInt = 16392u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint32u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt? =
      if (!tlvReader.isNull()) {
        tlvReader.getUInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt32uAttribute(decodedValue)
  }

  suspend fun writeNullableInt32uAttribute(value: UInt, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16392u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt40uAttribute(): NullableInt40uAttribute {
    val ATTRIBUTE_ID: UInt = 16393u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint40u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong? =
      if (!tlvReader.isNull()) {
        tlvReader.getULong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt40uAttribute(decodedValue)
  }

  suspend fun writeNullableInt40uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16393u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt48uAttribute(): NullableInt48uAttribute {
    val ATTRIBUTE_ID: UInt = 16394u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint48u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong? =
      if (!tlvReader.isNull()) {
        tlvReader.getULong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt48uAttribute(decodedValue)
  }

  suspend fun writeNullableInt48uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16394u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt56uAttribute(): NullableInt56uAttribute {
    val ATTRIBUTE_ID: UInt = 16395u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint56u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong? =
      if (!tlvReader.isNull()) {
        tlvReader.getULong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt56uAttribute(decodedValue)
  }

  suspend fun writeNullableInt56uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16395u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt64uAttribute(): NullableInt64uAttribute {
    val ATTRIBUTE_ID: UInt = 16396u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint64u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ULong? =
      if (!tlvReader.isNull()) {
        tlvReader.getULong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt64uAttribute(decodedValue)
  }

  suspend fun writeNullableInt64uAttribute(value: ULong, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16396u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt8sAttribute(): NullableInt8sAttribute {
    val ATTRIBUTE_ID: UInt = 16397u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint8s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Byte? =
      if (!tlvReader.isNull()) {
        tlvReader.getByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt8sAttribute(decodedValue)
  }

  suspend fun writeNullableInt8sAttribute(value: Byte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16397u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt16sAttribute(): NullableInt16sAttribute {
    val ATTRIBUTE_ID: UInt = 16398u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint16s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Short? =
      if (!tlvReader.isNull()) {
        tlvReader.getShort(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt16sAttribute(decodedValue)
  }

  suspend fun writeNullableInt16sAttribute(value: Short, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16398u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt24sAttribute(): NullableInt24sAttribute {
    val ATTRIBUTE_ID: UInt = 16399u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint24s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Int? =
      if (!tlvReader.isNull()) {
        tlvReader.getInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt24sAttribute(decodedValue)
  }

  suspend fun writeNullableInt24sAttribute(value: Int, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16399u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt32sAttribute(): NullableInt32sAttribute {
    val ATTRIBUTE_ID: UInt = 16400u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint32s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Int? =
      if (!tlvReader.isNull()) {
        tlvReader.getInt(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt32sAttribute(decodedValue)
  }

  suspend fun writeNullableInt32sAttribute(value: Int, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16400u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt40sAttribute(): NullableInt40sAttribute {
    val ATTRIBUTE_ID: UInt = 16401u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint40s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        tlvReader.getLong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt40sAttribute(decodedValue)
  }

  suspend fun writeNullableInt40sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16401u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt48sAttribute(): NullableInt48sAttribute {
    val ATTRIBUTE_ID: UInt = 16402u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint48s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        tlvReader.getLong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt48sAttribute(decodedValue)
  }

  suspend fun writeNullableInt48sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16402u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt56sAttribute(): NullableInt56sAttribute {
    val ATTRIBUTE_ID: UInt = 16403u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint56s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        tlvReader.getLong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt56sAttribute(decodedValue)
  }

  suspend fun writeNullableInt56sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16403u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableInt64sAttribute(): NullableInt64sAttribute {
    val ATTRIBUTE_ID: UInt = 16404u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableint64s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Long? =
      if (!tlvReader.isNull()) {
        tlvReader.getLong(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableInt64sAttribute(decodedValue)
  }

  suspend fun writeNullableInt64sAttribute(value: Long, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16404u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableEnum8Attribute(): NullableEnum8Attribute {
    val ATTRIBUTE_ID: UInt = 16405u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableenum8 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableEnum8Attribute(decodedValue)
  }

  suspend fun writeNullableEnum8Attribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16405u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableEnum16Attribute(): NullableEnum16Attribute {
    val ATTRIBUTE_ID: UInt = 16406u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableenum16 attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (!tlvReader.isNull()) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableEnum16Attribute(decodedValue)
  }

  suspend fun writeNullableEnum16Attribute(value: UShort, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16406u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableFloatSingleAttribute(): NullableFloatSingleAttribute {
    val ATTRIBUTE_ID: UInt = 16407u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablefloatsingle attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Float? =
      if (!tlvReader.isNull()) {
        tlvReader.getFloat(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableFloatSingleAttribute(decodedValue)
  }

  suspend fun writeNullableFloatSingleAttribute(value: Float, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16407u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableFloatDoubleAttribute(): NullableFloatDoubleAttribute {
    val ATTRIBUTE_ID: UInt = 16408u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablefloatdouble attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Double? =
      if (!tlvReader.isNull()) {
        tlvReader.getDouble(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableFloatDoubleAttribute(decodedValue)
  }

  suspend fun writeNullableFloatDoubleAttribute(
    value: Double,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16408u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableOctetStringAttribute(): NullableOctetStringAttribute {
    val ATTRIBUTE_ID: UInt = 16409u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableoctetstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: ByteArray? =
      if (!tlvReader.isNull()) {
        tlvReader.getByteArray(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableOctetStringAttribute(decodedValue)
  }

  suspend fun writeNullableOctetStringAttribute(
    value: ByteArray,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16409u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableCharStringAttribute(): NullableCharStringAttribute {
    val ATTRIBUTE_ID: UInt = 16414u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablecharstring attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: String? =
      if (!tlvReader.isNull()) {
        tlvReader.getString(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableCharStringAttribute(decodedValue)
  }

  suspend fun writeNullableCharStringAttribute(value: String, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16414u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableEnumAttrAttribute(): NullableEnumAttrAttribute {
    val ATTRIBUTE_ID: UInt = 16420u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullableenumattr attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableEnumAttrAttribute(decodedValue)
  }

  suspend fun writeNullableEnumAttrAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16420u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableStructAttribute(): NullableStructAttribute {
    val ATTRIBUTE_ID: UInt = 16421u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablestruct attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UnitTestingClusterSimpleStruct? =
      if (!tlvReader.isNull()) {
        UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableStructAttribute(decodedValue)
  }

  suspend fun writeNullableStructAttribute(
    value: UnitTestingClusterSimpleStruct,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16421u

    val tlvWriter = TlvWriter()
    value.toTlv(AnonymousTag, tlvWriter)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableRangeRestrictedInt8uAttribute(): NullableRangeRestrictedInt8uAttribute {
    val ATTRIBUTE_ID: UInt = 16422u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablerangerestrictedint8u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (!tlvReader.isNull()) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableRangeRestrictedInt8uAttribute(decodedValue)
  }

  suspend fun writeNullableRangeRestrictedInt8uAttribute(
    value: UByte,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16422u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableRangeRestrictedInt8sAttribute(): NullableRangeRestrictedInt8sAttribute {
    val ATTRIBUTE_ID: UInt = 16423u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Nullablerangerestrictedint8s attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Byte? =
      if (!tlvReader.isNull()) {
        tlvReader.getByte(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableRangeRestrictedInt8sAttribute(decodedValue)
  }

  suspend fun writeNullableRangeRestrictedInt8sAttribute(
    value: Byte,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16423u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableRangeRestrictedInt16uAttribute(): NullableRangeRestrictedInt16uAttribute {
    val ATTRIBUTE_ID: UInt = 16424u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) {
      "Nullablerangerestrictedint16u attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort? =
      if (!tlvReader.isNull()) {
        tlvReader.getUShort(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableRangeRestrictedInt16uAttribute(decodedValue)
  }

  suspend fun writeNullableRangeRestrictedInt16uAttribute(
    value: UShort,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16424u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readNullableRangeRestrictedInt16sAttribute(): NullableRangeRestrictedInt16sAttribute {
    val ATTRIBUTE_ID: UInt = 16425u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) {
      "Nullablerangerestrictedint16s attribute not found in response"
    }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: Short? =
      if (!tlvReader.isNull()) {
        tlvReader.getShort(AnonymousTag)
      } else {
        tlvReader.getNull(AnonymousTag)
        null
      }

    return NullableRangeRestrictedInt16sAttribute(decodedValue)
  }

  suspend fun writeNullableRangeRestrictedInt16sAttribute(
    value: Short,
    timedWriteTimeout: Duration? = null
  ) {
    val ATTRIBUTE_ID: UInt = 16425u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readWriteOnlyInt8uAttribute(): UByte? {
    val ATTRIBUTE_ID: UInt = 16426u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Writeonlyint8u attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UByte? =
      if (tlvReader.isNextTag(AnonymousTag)) {
        tlvReader.getUByte(AnonymousTag)
      } else {
        null
      }

    return decodedValue
  }

  suspend fun writeWriteOnlyInt8uAttribute(value: UByte, timedWriteTimeout: Duration? = null) {
    val ATTRIBUTE_ID: UInt = 16426u

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timedWriteTimeout
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.FINE, "Write command succeeded")
      }
      is WriteResponse.PartialWriteFailure -> {
        val aggregatedErrorMessage =
          response.failures.joinToString("\n") { failure ->
            "Error at ${failure.attributePath}: ${failure.ex.message}"
          }

        response.failures.forEach { failure ->
          logger.log(Level.WARNING, "Error at ${failure.attributePath}: ${failure.ex.message}")
        }

        throw IllegalStateException("Write command failed with errors: \n$aggregatedErrorMessage")
      }
    }
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65528u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Generatedcommandlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return GeneratedCommandListAttribute(decodedValue)
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID: UInt = 65529u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Acceptedcommandlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return AcceptedCommandListAttribute(decodedValue)
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID: UInt = 65530u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Eventlist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return EventListAttribute(decodedValue)
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID: UInt = 65531u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Attributelist attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: List<UInt> =
      buildList<UInt> {
        tlvReader.enterArray(AnonymousTag)
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUInt(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    return AttributeListAttribute(decodedValue)
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID: UInt = 65532u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Featuremap attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

    return decodedValue
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID: UInt = 65533u

    val attributePath =
      AttributePath(endpointId = endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID)

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isEmpty()) {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }

    logger.log(Level.FINE, "Read command succeeded")

    val attributeData =
      response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
        it.path.attributeId == ATTRIBUTE_ID
      }

    requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

    // Decode the TLV data into the appropriate type
    val tlvReader = TlvReader(attributeData.data)
    val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

    return decodedValue
  }

  companion object {
    private val logger = Logger.getLogger(UnitTestingCluster::class.java.name)
    const val CLUSTER_ID: UInt = 4294048773u
  }
}
