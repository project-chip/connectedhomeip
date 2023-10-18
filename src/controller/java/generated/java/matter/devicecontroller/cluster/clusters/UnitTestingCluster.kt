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

package matter.devicecontroller.cluster.clusters

import java.time.Duration
import java.util.logging.Level
import java.util.logging.Logger
import matter.controller.*
import matter.controller.model.AttributePath
import matter.controller.model.CommandPath
import matter.devicecontroller.cluster.structs.*
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

  suspend fun test(timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 0u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun testNotHandled(timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 1u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun testSpecific(timedInvokeTimeoutMs: Int? = null): TestSpecificResponse {
    val commandId: UInt = 2u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_RETURN_VALUE: Int = 0
    val returnValue_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_RETURN_VALUE))
    tlvReader.exitContainer()

    return TestSpecificResponse(returnValue_decoded)
  }

  suspend fun testUnknownCommand(timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 3u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun testAddArguments(
    arg1: UByte,
    arg2: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): TestAddArgumentsResponse {
    val commandId: UInt = 4u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_RETURN_VALUE: Int = 0
    val returnValue_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_RETURN_VALUE))
    tlvReader.exitContainer()

    return TestAddArgumentsResponse(returnValue_decoded)
  }

  suspend fun testSimpleArgumentRequest(
    arg1: Boolean,
    timedInvokeTimeoutMs: Int? = null
  ): TestSimpleArgumentResponse {
    val commandId: UInt = 5u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_RETURN_VALUE: Int = 0
    val returnValue_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_RETURN_VALUE))
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
    timedInvokeTimeoutMs: Int? = null
  ): TestStructArrayArgumentResponse {
    val commandId: UInt = 6u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    val arg1_decoded =
      buildList<UnitTestingClusterNestedStructList> {
        tlvReader.enterArray(ContextSpecificTag(TAG_ARG1))
        while (!tlvReader.isEndOfContainer()) {
          add(UnitTestingClusterNestedStructList.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    val TAG_ARG2: Int = 1
    val arg2_decoded =
      buildList<UnitTestingClusterSimpleStruct> {
        tlvReader.enterArray(ContextSpecificTag(TAG_ARG2))
        while (!tlvReader.isEndOfContainer()) {
          add(UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader))
        }
        tlvReader.exitContainer()
      }

    val TAG_ARG3: Int = 2
    val arg3_decoded =
      buildList<UByte> {
        tlvReader.enterArray(ContextSpecificTag(TAG_ARG3))
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUByte(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    val TAG_ARG4: Int = 3
    val arg4_decoded =
      buildList<Boolean> {
        tlvReader.enterArray(ContextSpecificTag(TAG_ARG4))
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getBoolean(AnonymousTag))
        }
        tlvReader.exitContainer()
      }

    val TAG_ARG5: Int = 4
    val arg5_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_ARG5))

    val TAG_ARG6: Int = 5
    val arg6_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_ARG6))
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
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    val commandId: UInt = 7u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testNestedStructArgumentRequest(
    arg1: UnitTestingClusterNestedStruct,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    val commandId: UInt = 8u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListStructArgumentRequest(
    arg1: List<UnitTestingClusterSimpleStruct>,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    val commandId: UInt = 9u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListInt8UArgumentRequest(
    arg1: List<UByte>,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    val commandId: UInt = 10u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testNestedStructListArgumentRequest(
    arg1: UnitTestingClusterNestedStructList,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    val commandId: UInt = 11u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListNestedStructListArgumentRequest(
    arg1: List<UnitTestingClusterNestedStructList>,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    val commandId: UInt = 12u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return BooleanResponse(value_decoded)
  }

  suspend fun testListInt8UReverseRequest(
    arg1: List<UByte>,
    timedInvokeTimeoutMs: Int? = null
  ): TestListInt8UReverseResponse {
    val commandId: UInt = 13u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    val arg1_decoded =
      buildList<UByte> {
        tlvReader.enterArray(ContextSpecificTag(TAG_ARG1))
        while (!tlvReader.isEndOfContainer()) {
          add(tlvReader.getUByte(AnonymousTag))
        }
        tlvReader.exitContainer()
      }
    tlvReader.exitContainer()

    return TestListInt8UReverseResponse(arg1_decoded)
  }

  suspend fun testEnumsRequest(
    arg1: UShort,
    arg2: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): TestEnumsResponse {
    val commandId: UInt = 14u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    val arg1_decoded = tlvReader.getUShort(ContextSpecificTag(TAG_ARG1))

    val TAG_ARG2: Int = 1
    val arg2_decoded = tlvReader.getUByte(ContextSpecificTag(TAG_ARG2))
    tlvReader.exitContainer()

    return TestEnumsResponse(arg1_decoded, arg2_decoded)
  }

  suspend fun testNullableOptionalRequest(
    arg1: UByte?,
    timedInvokeTimeoutMs: Int? = null
  ): TestNullableOptionalResponse {
    val commandId: UInt = 15u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1?.let { tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_WAS_PRESENT: Int = 0
    val wasPresent_decoded = tlvReader.getBoolean(ContextSpecificTag(TAG_WAS_PRESENT))

    val TAG_WAS_NULL: Int = 1
    val wasNull_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_WAS_NULL))) {
        tlvReader.getBoolean(ContextSpecificTag(TAG_WAS_NULL))
      } else {
        null
      }

    val TAG_VALUE: Int = 2
    val value_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_VALUE))) {
        tlvReader.getUByte(ContextSpecificTag(TAG_VALUE))
      } else {
        null
      }

    val TAG_ORIGINAL_VALUE: Int = 3
    val originalValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_ORIGINAL_VALUE))) {
        if (!tlvReader.isNull()) {
          if (tlvReader.isNextTag(ContextSpecificTag(TAG_ORIGINAL_VALUE))) {
            tlvReader.getUByte(ContextSpecificTag(TAG_ORIGINAL_VALUE))
          } else {
            null
          }
        } else {
          tlvReader.getNull(ContextSpecificTag(TAG_ORIGINAL_VALUE))
          null
        }
      } else {
        null
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
    timedInvokeTimeoutMs: Int? = null
  ): TestComplexNullableOptionalResponse {
    val commandId: UInt = 16u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_NULLABLE_INT_WAS_NULL: Int = 0
    val nullableIntWasNull_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_INT_WAS_NULL))

    val TAG_NULLABLE_INT_VALUE: Int = 1
    val nullableIntValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_INT_VALUE))) {
        tlvReader.getUShort(ContextSpecificTag(TAG_NULLABLE_INT_VALUE))
      } else {
        null
      }

    val TAG_OPTIONAL_INT_WAS_PRESENT: Int = 2
    val optionalIntWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_OPTIONAL_INT_WAS_PRESENT))

    val TAG_OPTIONAL_INT_VALUE: Int = 3
    val optionalIntValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_INT_VALUE))) {
        tlvReader.getUShort(ContextSpecificTag(TAG_OPTIONAL_INT_VALUE))
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_INT_WAS_PRESENT: Int = 4
    val nullableOptionalIntWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_WAS_PRESENT))

    val TAG_NULLABLE_OPTIONAL_INT_WAS_NULL: Int = 5
    val nullableOptionalIntWasNull_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_WAS_NULL))) {
        tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_WAS_NULL))
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_INT_VALUE: Int = 6
    val nullableOptionalIntValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_VALUE))) {
        tlvReader.getUShort(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_INT_VALUE))
      } else {
        null
      }

    val TAG_NULLABLE_STRING_WAS_NULL: Int = 7
    val nullableStringWasNull_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_STRING_WAS_NULL))

    val TAG_NULLABLE_STRING_VALUE: Int = 8
    val nullableStringValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_STRING_VALUE))) {
        tlvReader.getString(ContextSpecificTag(TAG_NULLABLE_STRING_VALUE))
      } else {
        null
      }

    val TAG_OPTIONAL_STRING_WAS_PRESENT: Int = 9
    val optionalStringWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_OPTIONAL_STRING_WAS_PRESENT))

    val TAG_OPTIONAL_STRING_VALUE: Int = 10
    val optionalStringValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_STRING_VALUE))) {
        tlvReader.getString(ContextSpecificTag(TAG_OPTIONAL_STRING_VALUE))
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_STRING_WAS_PRESENT: Int = 11
    val nullableOptionalStringWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_WAS_PRESENT))

    val TAG_NULLABLE_OPTIONAL_STRING_WAS_NULL: Int = 12
    val nullableOptionalStringWasNull_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_WAS_NULL))) {
        tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_WAS_NULL))
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_STRING_VALUE: Int = 13
    val nullableOptionalStringValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_VALUE))) {
        tlvReader.getString(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRING_VALUE))
      } else {
        null
      }

    val TAG_NULLABLE_STRUCT_WAS_NULL: Int = 14
    val nullableStructWasNull_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_STRUCT_WAS_NULL))

    val TAG_NULLABLE_STRUCT_VALUE: Int = 15
    val nullableStructValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_STRUCT_VALUE))) {
        UnitTestingClusterSimpleStruct.fromTlv(
          ContextSpecificTag(TAG_NULLABLE_STRUCT_VALUE),
          tlvReader
        )
      } else {
        null
      }

    val TAG_OPTIONAL_STRUCT_WAS_PRESENT: Int = 16
    val optionalStructWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_OPTIONAL_STRUCT_WAS_PRESENT))

    val TAG_OPTIONAL_STRUCT_VALUE: Int = 17
    val optionalStructValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_STRUCT_VALUE))) {
        UnitTestingClusterSimpleStruct.fromTlv(
          ContextSpecificTag(TAG_OPTIONAL_STRUCT_VALUE),
          tlvReader
        )
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_STRUCT_WAS_PRESENT: Int = 18
    val nullableOptionalStructWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_WAS_PRESENT))

    val TAG_NULLABLE_OPTIONAL_STRUCT_WAS_NULL: Int = 19
    val nullableOptionalStructWasNull_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_WAS_NULL))) {
        tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_WAS_NULL))
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_STRUCT_VALUE: Int = 20
    val nullableOptionalStructValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_VALUE))) {
        UnitTestingClusterSimpleStruct.fromTlv(
          ContextSpecificTag(TAG_NULLABLE_OPTIONAL_STRUCT_VALUE),
          tlvReader
        )
      } else {
        null
      }

    val TAG_NULLABLE_LIST_WAS_NULL: Int = 21
    val nullableListWasNull_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_LIST_WAS_NULL))

    val TAG_NULLABLE_LIST_VALUE: Int = 22
    val nullableListValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_LIST_VALUE))) {
        buildList<UByte> {
          tlvReader.enterArray(ContextSpecificTag(TAG_NULLABLE_LIST_VALUE))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUByte(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }

    val TAG_OPTIONAL_LIST_WAS_PRESENT: Int = 23
    val optionalListWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_OPTIONAL_LIST_WAS_PRESENT))

    val TAG_OPTIONAL_LIST_VALUE: Int = 24
    val optionalListValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_OPTIONAL_LIST_VALUE))) {
        buildList<UByte> {
          tlvReader.enterArray(ContextSpecificTag(TAG_OPTIONAL_LIST_VALUE))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUByte(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_LIST_WAS_PRESENT: Int = 25
    val nullableOptionalListWasPresent_decoded =
      tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_WAS_PRESENT))

    val TAG_NULLABLE_OPTIONAL_LIST_WAS_NULL: Int = 26
    val nullableOptionalListWasNull_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_WAS_NULL))) {
        tlvReader.getBoolean(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_WAS_NULL))
      } else {
        null
      }

    val TAG_NULLABLE_OPTIONAL_LIST_VALUE: Int = 27
    val nullableOptionalListValue_decoded =
      if (tlvReader.isNextTag(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_VALUE))) {
        buildList<UByte> {
          tlvReader.enterArray(ContextSpecificTag(TAG_NULLABLE_OPTIONAL_LIST_VALUE))
          while (!tlvReader.isEndOfContainer()) {
            add(tlvReader.getUByte(AnonymousTag))
          }
          tlvReader.exitContainer()
        }
      } else {
        null
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
    timedInvokeTimeoutMs: Int? = null
  ): SimpleStructResponse {
    val commandId: UInt = 17u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1.toTlv(ContextSpecificTag(TAG_ARG1_REQ), tlvWriter)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_ARG1: Int = 0
    val arg1_decoded =
      UnitTestingClusterSimpleStruct.fromTlv(ContextSpecificTag(TAG_ARG1), tlvReader)
    tlvReader.exitContainer()

    return SimpleStructResponse(arg1_decoded)
  }

  suspend fun timedInvokeRequest(timedInvokeTimeoutMs: Int) {
    val commandId: UInt = 18u
    val timeoutMs: Duration = Duration.ofMillis(timedInvokeTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun testSimpleOptionalArgumentRequest(arg1: Boolean?, timedInvokeTimeoutMs: Int? = null) {
    val commandId: UInt = 19u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    arg1?.let { tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1) }
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")
  }

  suspend fun testEmitTestEventRequest(
    arg1: UByte,
    arg2: UByte,
    arg3: Boolean,
    timedInvokeTimeoutMs: Int? = null
  ): TestEmitTestEventResponse {
    val commandId: UInt = 20u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getULong(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return TestEmitTestEventResponse(value_decoded)
  }

  suspend fun testEmitTestFabricScopedEventRequest(
    arg1: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): TestEmitTestFabricScopedEventResponse {
    val commandId: UInt = 21u
    val timeoutMs: Duration =
      timedInvokeTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.startStructure(AnonymousTag)

    val TAG_ARG1_REQ: Int = 0
    tlvWriter.put(ContextSpecificTag(TAG_ARG1_REQ), arg1)
    tlvWriter.endStructure()

    val request: InvokeRequest =
      InvokeRequest(
        CommandPath(endpointId, clusterId = CLUSTER_ID, commandId),
        tlvPayload = tlvWriter.getEncoded(),
        timedRequest = timeoutMs
      )

    val response: InvokeResponse = controller.invoke(request)
    logger.log(Level.INFO, "Invoke command succeeded: ${response}")

    val tlvReader = TlvReader(response.payload)
    tlvReader.enterStructure(AnonymousTag)
    val TAG_VALUE: Int = 0
    val value_decoded = tlvReader.getULong(ContextSpecificTag(TAG_VALUE))
    tlvReader.exitContainer()

    return TestEmitTestFabricScopedEventResponse(value_decoded)
  }

  suspend fun readBooleanAttribute(): Boolean {
    val ATTRIBUTE_ID_BOOLEAN: UInt = 0u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_BOOLEAN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_BOOLEAN
        }

      requireNotNull(attributeData) { "Boolean attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_BOOLEAN: UInt = 0u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_BOOLEAN
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_BITMAP8: UInt = 1u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_BITMAP8
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_BITMAP8
        }

      requireNotNull(attributeData) { "Bitmap8 attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeBitmap8Attribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_BITMAP8: UInt = 1u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_BITMAP8
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_BITMAP16: UInt = 2u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_BITMAP16
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_BITMAP16
        }

      requireNotNull(attributeData) { "Bitmap16 attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeBitmap16Attribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_BITMAP16: UInt = 2u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_BITMAP16
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_BITMAP32: UInt = 3u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_BITMAP32
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_BITMAP32
        }

      requireNotNull(attributeData) { "Bitmap32 attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeBitmap32Attribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_BITMAP32: UInt = 3u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_BITMAP32
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_BITMAP64: UInt = 4u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_BITMAP64
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_BITMAP64
        }

      requireNotNull(attributeData) { "Bitmap64 attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeBitmap64Attribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_BITMAP64: UInt = 4u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_BITMAP64
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT8U: UInt = 5u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT8U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT8U
        }

      requireNotNull(attributeData) { "Int8u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT8U: UInt = 5u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID_INT8U),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT16U: UInt = 6u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT16U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT16U
        }

      requireNotNull(attributeData) { "Int16u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT16U: UInt = 6u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT16U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT24U: UInt = 7u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT24U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT24U
        }

      requireNotNull(attributeData) { "Int24u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt24uAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT24U: UInt = 7u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT24U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT32U: UInt = 8u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT32U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT32U
        }

      requireNotNull(attributeData) { "Int32u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt32uAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT32U: UInt = 8u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT32U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT40U: UInt = 9u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT40U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT40U
        }

      requireNotNull(attributeData) { "Int40u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt40uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT40U: UInt = 9u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT40U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT48U: UInt = 10u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT48U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT48U
        }

      requireNotNull(attributeData) { "Int48u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt48uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT48U: UInt = 10u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT48U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT56U: UInt = 11u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT56U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT56U
        }

      requireNotNull(attributeData) { "Int56u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt56uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT56U: UInt = 11u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT56U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT64U: UInt = 12u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT64U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT64U
        }

      requireNotNull(attributeData) { "Int64u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt64uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT64U: UInt = 12u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT64U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT8S: UInt = 13u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT8S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT8S
        }

      requireNotNull(attributeData) { "Int8s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte = tlvReader.getByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT8S: UInt = 13u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID_INT8S),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT16S: UInt = 14u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT16S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT16S
        }

      requireNotNull(attributeData) { "Int16s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short = tlvReader.getShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt16sAttribute(value: Short, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT16S: UInt = 14u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT16S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT24S: UInt = 15u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT24S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT24S
        }

      requireNotNull(attributeData) { "Int24s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Int = tlvReader.getInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt24sAttribute(value: Int, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT24S: UInt = 15u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT24S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT32S: UInt = 16u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT32S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT32S
        }

      requireNotNull(attributeData) { "Int32s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Int = tlvReader.getInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt32sAttribute(value: Int, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT32S: UInt = 16u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT32S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT40S: UInt = 17u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT40S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT40S
        }

      requireNotNull(attributeData) { "Int40s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Long = tlvReader.getLong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt40sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT40S: UInt = 17u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT40S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT48S: UInt = 18u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT48S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT48S
        }

      requireNotNull(attributeData) { "Int48s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Long = tlvReader.getLong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt48sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT48S: UInt = 18u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT48S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT56S: UInt = 19u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT56S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT56S
        }

      requireNotNull(attributeData) { "Int56s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Long = tlvReader.getLong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt56sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT56S: UInt = 19u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT56S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_INT64S: UInt = 20u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_INT64S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_INT64S
        }

      requireNotNull(attributeData) { "Int64s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Long = tlvReader.getLong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeInt64sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_INT64S: UInt = 20u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_INT64S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_ENUM8: UInt = 21u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENUM8
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENUM8
        }

      requireNotNull(attributeData) { "Enum8 attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnum8Attribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_ENUM8: UInt = 21u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(endpointId, clusterId = CLUSTER_ID, attributeId = ATTRIBUTE_ID_ENUM8),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_ENUM16: UInt = 22u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENUM16
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENUM16
        }

      requireNotNull(attributeData) { "Enum16 attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnum16Attribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_ENUM16: UInt = 22u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ENUM16
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_FLOATSINGLE: UInt = 23u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FLOATSINGLE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FLOATSINGLE
        }

      requireNotNull(attributeData) { "Floatsingle attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Float = tlvReader.getFloat(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeFloatSingleAttribute(value: Float, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_FLOATSINGLE: UInt = 23u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_FLOATSINGLE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_FLOATDOUBLE: UInt = 24u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FLOATDOUBLE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FLOATDOUBLE
        }

      requireNotNull(attributeData) { "Floatdouble attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Double = tlvReader.getDouble(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeFloatDoubleAttribute(value: Double, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_FLOATDOUBLE: UInt = 24u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_FLOATDOUBLE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_OCTETSTRING: UInt = 25u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_OCTETSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_OCTETSTRING
        }

      requireNotNull(attributeData) { "Octetstring attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ByteArray = tlvReader.getByteArray(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeOctetStringAttribute(value: ByteArray, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_OCTETSTRING: UInt = 25u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_OCTETSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LISTINT8U: UInt = 26u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LISTINT8U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LISTINT8U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeListInt8uAttribute(value: List<UByte>, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_LISTINT8U: UInt = 26u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LISTINT8U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LISTOCTETSTRING: UInt = 27u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LISTOCTETSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LISTOCTETSTRING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeListOctetStringAttribute(
    value: List<ByteArray>,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_LISTOCTETSTRING: UInt = 27u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LISTOCTETSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LISTSTRUCTOCTETSTRING: UInt = 28u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LISTSTRUCTOCTETSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LISTSTRUCTOCTETSTRING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeListStructOctetStringAttribute(
    value: List<UnitTestingClusterTestListStructOctet>,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_LISTSTRUCTOCTETSTRING: UInt = 28u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LISTSTRUCTOCTETSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LONGOCTETSTRING: UInt = 29u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LONGOCTETSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LONGOCTETSTRING
        }

      requireNotNull(attributeData) { "Longoctetstring attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ByteArray = tlvReader.getByteArray(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeLongOctetStringAttribute(value: ByteArray, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_LONGOCTETSTRING: UInt = 29u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LONGOCTETSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_CHARSTRING: UInt = 30u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CHARSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CHARSTRING
        }

      requireNotNull(attributeData) { "Charstring attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: String = tlvReader.getString(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeCharStringAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_CHARSTRING: UInt = 30u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_CHARSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LONGCHARSTRING: UInt = 31u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LONGCHARSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LONGCHARSTRING
        }

      requireNotNull(attributeData) { "Longcharstring attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: String = tlvReader.getString(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeLongCharStringAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_LONGCHARSTRING: UInt = 31u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LONGCHARSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_EPOCHUS: UInt = 32u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EPOCHUS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EPOCHUS
        }

      requireNotNull(attributeData) { "Epochus attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: ULong = tlvReader.getULong(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEpochUsAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_EPOCHUS: UInt = 32u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_EPOCHUS
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_EPOCHS: UInt = 33u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EPOCHS
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EPOCHS
        }

      requireNotNull(attributeData) { "Epochs attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEpochSAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_EPOCHS: UInt = 33u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_EPOCHS
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_VENDORID: UInt = 34u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_VENDORID
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_VENDORID
        }

      requireNotNull(attributeData) { "Vendorid attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeVendorIdAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_VENDORID: UInt = 34u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_VENDORID
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LISTNULLABLESANDOPTIONALSSTRUCT: UInt = 35u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LISTNULLABLESANDOPTIONALSSTRUCT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LISTNULLABLESANDOPTIONALSSTRUCT
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeListNullablesAndOptionalsStructAttribute(
    value: List<UnitTestingClusterNullablesAndOptionalsStruct>,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_LISTNULLABLESANDOPTIONALSSTRUCT: UInt = 35u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LISTNULLABLESANDOPTIONALSSTRUCT
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_ENUMATTR: UInt = 36u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ENUMATTR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ENUMATTR
        }

      requireNotNull(attributeData) { "Enumattr attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeEnumAttrAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_ENUMATTR: UInt = 36u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_ENUMATTR
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_STRUCTATTR: UInt = 37u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_STRUCTATTR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_STRUCTATTR
        }

      requireNotNull(attributeData) { "Structattr attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UnitTestingClusterSimpleStruct =
        UnitTestingClusterSimpleStruct.fromTlv(AnonymousTag, tlvReader)

      return StructAttrAttribute(decodedValue)
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeStructAttrAttribute(
    value: UnitTestingClusterSimpleStruct,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_STRUCTATTR: UInt = 37u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    value.toTlv(AnonymousTag, tlvWriter)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_STRUCTATTR
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_RANGERESTRICTEDINT8U: UInt = 38u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT8U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RANGERESTRICTEDINT8U
        }

      requireNotNull(attributeData) { "Rangerestrictedint8u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UByte = tlvReader.getUByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeRangeRestrictedInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RANGERESTRICTEDINT8U: UInt = 38u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT8U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_RANGERESTRICTEDINT8S: UInt = 39u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT8S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RANGERESTRICTEDINT8S
        }

      requireNotNull(attributeData) { "Rangerestrictedint8s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Byte = tlvReader.getByte(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeRangeRestrictedInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RANGERESTRICTEDINT8S: UInt = 39u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT8S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_RANGERESTRICTEDINT16U: UInt = 40u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT16U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RANGERESTRICTEDINT16U
        }

      requireNotNull(attributeData) { "Rangerestrictedint16u attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeRangeRestrictedInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RANGERESTRICTEDINT16U: UInt = 40u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT16U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_RANGERESTRICTEDINT16S: UInt = 41u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT16S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_RANGERESTRICTEDINT16S
        }

      requireNotNull(attributeData) { "Rangerestrictedint16s attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Short = tlvReader.getShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeRangeRestrictedInt16sAttribute(value: Short, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_RANGERESTRICTEDINT16S: UInt = 41u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_RANGERESTRICTEDINT16S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LISTLONGOCTETSTRING: UInt = 42u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LISTLONGOCTETSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LISTLONGOCTETSTRING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeListLongOctetStringAttribute(
    value: List<ByteArray>,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_LISTLONGOCTETSTRING: UInt = 42u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LISTLONGOCTETSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_LISTFABRICSCOPED: UInt = 43u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_LISTFABRICSCOPED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_LISTFABRICSCOPED
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeListFabricScopedAttribute(
    value: List<UnitTestingClusterTestFabricScoped>,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_LISTFABRICSCOPED: UInt = 43u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

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
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_LISTFABRICSCOPED
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_TIMEDWRITEBOOLEAN: UInt = 48u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_TIMEDWRITEBOOLEAN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_TIMEDWRITEBOOLEAN
        }

      requireNotNull(attributeData) { "Timedwriteboolean attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeTimedWriteBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    val ATTRIBUTE_ID_TIMEDWRITEBOOLEAN: UInt = 48u
    val timeoutMs: Duration = Duration.ofMillis(timedWriteTimeoutMs.toLong())

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_TIMEDWRITEBOOLEAN
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_GENERALERRORBOOLEAN: UInt = 49u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_GENERALERRORBOOLEAN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_GENERALERRORBOOLEAN
        }

      requireNotNull(attributeData) { "Generalerrorboolean attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeGeneralErrorBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_GENERALERRORBOOLEAN: UInt = 49u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_GENERALERRORBOOLEAN
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_CLUSTERERRORBOOLEAN: UInt = 50u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CLUSTERERRORBOOLEAN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CLUSTERERRORBOOLEAN
        }

      requireNotNull(attributeData) { "Clustererrorboolean attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: Boolean = tlvReader.getBoolean(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeClusterErrorBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_CLUSTERERRORBOOLEAN: UInt = 50u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_CLUSTERERRORBOOLEAN
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_UNSUPPORTED: UInt = 255u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_UNSUPPORTED
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_UNSUPPORTED
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeUnsupportedAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_UNSUPPORTED: UInt = 255u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_UNSUPPORTED
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEBOOLEAN: UInt = 16384u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEBOOLEAN
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEBOOLEAN
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEBOOLEAN: UInt = 16384u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEBOOLEAN
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEBITMAP8: UInt = 16385u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEBITMAP8
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEBITMAP8
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableBitmap8Attribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEBITMAP8: UInt = 16385u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEBITMAP8
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEBITMAP16: UInt = 16386u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEBITMAP16
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEBITMAP16
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableBitmap16Attribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEBITMAP16: UInt = 16386u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEBITMAP16
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEBITMAP32: UInt = 16387u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEBITMAP32
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEBITMAP32
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableBitmap32Attribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEBITMAP32: UInt = 16387u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEBITMAP32
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEBITMAP64: UInt = 16388u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEBITMAP64
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEBITMAP64
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableBitmap64Attribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEBITMAP64: UInt = 16388u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEBITMAP64
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT8U: UInt = 16389u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT8U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT8U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT8U: UInt = 16389u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT8U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT16U: UInt = 16390u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT16U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT16U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT16U: UInt = 16390u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT16U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT24U: UInt = 16391u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT24U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT24U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt24uAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT24U: UInt = 16391u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT24U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT32U: UInt = 16392u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT32U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT32U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt32uAttribute(value: UInt, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT32U: UInt = 16392u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT32U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT40U: UInt = 16393u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT40U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT40U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt40uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT40U: UInt = 16393u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT40U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT48U: UInt = 16394u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT48U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT48U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt48uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT48U: UInt = 16394u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT48U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT56U: UInt = 16395u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT56U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT56U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt56uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT56U: UInt = 16395u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT56U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT64U: UInt = 16396u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT64U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT64U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt64uAttribute(value: ULong, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT64U: UInt = 16396u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT64U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT8S: UInt = 16397u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT8S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT8S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT8S: UInt = 16397u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT8S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT16S: UInt = 16398u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT16S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT16S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt16sAttribute(value: Short, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT16S: UInt = 16398u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT16S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT24S: UInt = 16399u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT24S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT24S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt24sAttribute(value: Int, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT24S: UInt = 16399u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT24S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT32S: UInt = 16400u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT32S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT32S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt32sAttribute(value: Int, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT32S: UInt = 16400u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT32S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT40S: UInt = 16401u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT40S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT40S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt40sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT40S: UInt = 16401u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT40S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT48S: UInt = 16402u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT48S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT48S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt48sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT48S: UInt = 16402u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT48S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT56S: UInt = 16403u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT56S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT56S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt56sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT56S: UInt = 16403u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT56S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEINT64S: UInt = 16404u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEINT64S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEINT64S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableInt64sAttribute(value: Long, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEINT64S: UInt = 16404u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEINT64S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEENUM8: UInt = 16405u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEENUM8
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEENUM8
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableEnum8Attribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEENUM8: UInt = 16405u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEENUM8
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEENUM16: UInt = 16406u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEENUM16
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEENUM16
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableEnum16Attribute(value: UShort, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEENUM16: UInt = 16406u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEENUM16
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEFLOATSINGLE: UInt = 16407u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEFLOATSINGLE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEFLOATSINGLE
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableFloatSingleAttribute(value: Float, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEFLOATSINGLE: UInt = 16407u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEFLOATSINGLE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEFLOATDOUBLE: UInt = 16408u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEFLOATDOUBLE
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEFLOATDOUBLE
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableFloatDoubleAttribute(value: Double, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEFLOATDOUBLE: UInt = 16408u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEFLOATDOUBLE
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEOCTETSTRING: UInt = 16409u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEOCTETSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEOCTETSTRING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableOctetStringAttribute(
    value: ByteArray,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_NULLABLEOCTETSTRING: UInt = 16409u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEOCTETSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLECHARSTRING: UInt = 16414u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLECHARSTRING
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLECHARSTRING
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableCharStringAttribute(value: String, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLECHARSTRING: UInt = 16414u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLECHARSTRING
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLEENUMATTR: UInt = 16420u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLEENUMATTR
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLEENUMATTR
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableEnumAttrAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_NULLABLEENUMATTR: UInt = 16420u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLEENUMATTR
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLESTRUCT: UInt = 16421u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLESTRUCT
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLESTRUCT
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableStructAttribute(
    value: UnitTestingClusterSimpleStruct,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_NULLABLESTRUCT: UInt = 16421u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    value.toTlv(AnonymousTag, tlvWriter)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLESTRUCT
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8U: UInt = 16422u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8U
        }

      requireNotNull(attributeData) {
        "Nullablerangerestrictedint8u attribute not found in response"
      }

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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableRangeRestrictedInt8uAttribute(
    value: UByte,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8U: UInt = 16422u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8S: UInt = 16423u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8S
        }

      requireNotNull(attributeData) {
        "Nullablerangerestrictedint8s attribute not found in response"
      }

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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableRangeRestrictedInt8sAttribute(
    value: Byte,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8S: UInt = 16423u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT8S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16U: UInt = 16424u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableRangeRestrictedInt16uAttribute(
    value: UShort,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16U: UInt = 16424u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16S: UInt = 16425u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16S
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16S
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeNullableRangeRestrictedInt16sAttribute(
    value: Short,
    timedWriteTimeoutMs: Int? = null
  ) {
    val ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16S: UInt = 16425u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_NULLABLERANGERESTRICTEDINT16S
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_WRITEONLYINT8U: UInt = 16426u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_WRITEONLYINT8U
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_WRITEONLYINT8U
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun writeWriteOnlyInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int? = null) {
    val ATTRIBUTE_ID_WRITEONLYINT8U: UInt = 16426u
    val timeoutMs: Duration =
      timedWriteTimeoutMs?.let { Duration.ofMillis(it.toLong()) } ?: Duration.ZERO

    val tlvWriter = TlvWriter()
    tlvWriter.put(AnonymousTag, value)

    val writeRequests: WriteRequests =
      WriteRequests(
        requests =
          listOf(
            WriteRequest(
              attributePath =
                AttributePath(
                  endpointId,
                  clusterId = CLUSTER_ID,
                  attributeId = ATTRIBUTE_ID_WRITEONLYINT8U
                ),
              tlvPayload = tlvWriter.getEncoded()
            )
          ),
        timedRequest = timeoutMs
      )

    val response: WriteResponse = controller.write(writeRequests)

    when (response) {
      is WriteResponse.Success -> {
        logger.log(Level.INFO, "Write command succeeded")
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
    val ATTRIBUTE_ID_GENERATEDCOMMANDLIST: UInt = 65528u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_GENERATEDCOMMANDLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_GENERATEDCOMMANDLIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    val ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST: UInt = 65529u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ACCEPTEDCOMMANDLIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    val ATTRIBUTE_ID_EVENTLIST: UInt = 65530u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_EVENTLIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_EVENTLIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    val ATTRIBUTE_ID_ATTRIBUTELIST: UInt = 65531u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_ATTRIBUTELIST
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_ATTRIBUTELIST
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
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readFeatureMapAttribute(): UInt {
    val ATTRIBUTE_ID_FEATUREMAP: UInt = 65532u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_FEATUREMAP
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_FEATUREMAP
        }

      requireNotNull(attributeData) { "Featuremap attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UInt = tlvReader.getUInt(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    val ATTRIBUTE_ID_CLUSTERREVISION: UInt = 65533u

    val attributePath =
      AttributePath(
        endpointId = endpointId,
        clusterId = CLUSTER_ID,
        attributeId = ATTRIBUTE_ID_CLUSTERREVISION
      )

    val readRequest = ReadRequest(eventPaths = emptyList(), attributePaths = listOf(attributePath))

    val response = controller.read(readRequest)

    if (response.successes.isNotEmpty()) {
      logger.log(Level.INFO, "Read command succeeded")

      val attributeData =
        response.successes.filterIsInstance<ReadData.Attribute>().firstOrNull {
          it.path.attributeId == ATTRIBUTE_ID_CLUSTERREVISION
        }

      requireNotNull(attributeData) { "Clusterrevision attribute not found in response" }

      // Decode the TLV data into the appropriate type
      val tlvReader = TlvReader(attributeData.data)
      val decodedValue: UShort = tlvReader.getUShort(AnonymousTag)

      return decodedValue
    } else {
      logger.log(Level.WARNING, "Read command failed")
      throw IllegalStateException("Read command failed with failures: ${response.failures}")
    }
  }

  companion object {
    private val logger = Logger.getLogger(UnitTestingCluster::class.java.name)
    const val CLUSTER_ID: UInt = 4294048773u
  }
}
