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

import java.util.ArrayList

class UnitTestingCluster(private val endpointId: UShort) {
  class TestSpecificResponse(val returnValue: UByte)

  class TestAddArgumentsResponse(val returnValue: UByte)

  class TestSimpleArgumentResponse(val returnValue: Boolean)

  class TestStructArrayArgumentResponse(
    val arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
    val arg2: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
    val arg3: ArrayList<UInt>,
    val arg4: ArrayList<Boolean>,
    val arg5: UInt,
    val arg6: Boolean
  )

  class BooleanResponse(val value: Boolean)

  class TestListInt8UReverseResponse(val arg1: ArrayList<UByte>)

  class TestEnumsResponse(val arg1: UShort, val arg2: UInt)

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
    val nullableStructValue: ChipStructs.UnitTestingClusterSimpleStruct?,
    val optionalStructWasPresent: Boolean,
    val optionalStructValue: ChipStructs.UnitTestingClusterSimpleStruct?,
    val nullableOptionalStructWasPresent: Boolean,
    val nullableOptionalStructWasNull: Boolean?,
    val nullableOptionalStructValue: ChipStructs.UnitTestingClusterSimpleStruct?,
    val nullableListWasNull: Boolean,
    val nullableListValue: ArrayList<UInt>?,
    val optionalListWasPresent: Boolean,
    val optionalListValue: ArrayList<UInt>?,
    val nullableOptionalListWasPresent: Boolean,
    val nullableOptionalListWasNull: Boolean?,
    val nullableOptionalListValue: ArrayList<UInt>?
  )

  class SimpleStructResponse(val arg1: ChipStructs.UnitTestingClusterSimpleStruct)

  class TestEmitTestEventResponse(val value: ULong)

  class TestEmitTestFabricScopedEventResponse(val value: ULong)

  class ListInt8uAttribute(val value: ArrayList<UByte>)

  class ListOctetStringAttribute(val value: ArrayList<ByteArray>)

  class ListStructOctetStringAttribute(
    val value: ArrayList<ChipStructs.UnitTestingClusterTestListStructOctet>
  )

  class ListNullablesAndOptionalsStructAttribute(
    val value: ArrayList<ChipStructs.UnitTestingClusterNullablesAndOptionalsStruct>
  )

  class StructAttrAttribute(val value: ChipStructs.UnitTestingClusterSimpleStruct)

  class ListLongOctetStringAttribute(val value: ArrayList<ByteArray>)

  class ListFabricScopedAttribute(
    val value: ArrayList<ChipStructs.UnitTestingClusterTestFabricScoped>
  )

  class NullableBooleanAttribute(val value: Boolean?)

  class NullableBitmap8Attribute(val value: UInt?)

  class NullableBitmap16Attribute(val value: UInt?)

  class NullableBitmap32Attribute(val value: ULong?)

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

  class NullableEnum8Attribute(val value: UInt?)

  class NullableEnum16Attribute(val value: UInt?)

  class NullableFloatSingleAttribute(val value: Float?)

  class NullableFloatDoubleAttribute(val value: Double?)

  class NullableOctetStringAttribute(val value: ByteArray?)

  class NullableCharStringAttribute(val value: String?)

  class NullableEnumAttrAttribute(val value: UInt?)

  class NullableStructAttribute(val value: ChipStructs.UnitTestingClusterSimpleStruct?)

  class NullableRangeRestrictedInt8uAttribute(val value: UByte?)

  class NullableRangeRestrictedInt8sAttribute(val value: Byte?)

  class NullableRangeRestrictedInt16uAttribute(val value: UShort?)

  class NullableRangeRestrictedInt16sAttribute(val value: Short?)

  class GeneratedCommandListAttribute(val value: ArrayList<UInt>)

  class AcceptedCommandListAttribute(val value: ArrayList<UInt>)

  class EventListAttribute(val value: ArrayList<UInt>)

  class AttributeListAttribute(val value: ArrayList<UInt>)

  suspend fun test(timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testNotHandled(timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testSpecific(timedInvokeTimeoutMs: Int? = null): TestSpecificResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testUnknownCommand(timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testAddArguments(
    arg1: UByte,
    arg2: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): TestAddArgumentsResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testSimpleArgumentRequest(
    arg1: Boolean,
    timedInvokeTimeoutMs: Int? = null
  ): TestSimpleArgumentResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testStructArrayArgumentRequest(
    arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
    arg2: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
    arg3: ArrayList<UInt>,
    arg4: ArrayList<Boolean>,
    arg5: UInt,
    arg6: Boolean,
    timedInvokeTimeoutMs: Int? = null
  ): TestStructArrayArgumentResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testStructArgumentRequest(
    arg1: ChipStructs.UnitTestingClusterSimpleStruct,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testNestedStructArgumentRequest(
    arg1: ChipStructs.UnitTestingClusterNestedStruct,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testListStructArgumentRequest(
    arg1: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testListInt8UArgumentRequest(
    arg1: ArrayList<UByte>,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testNestedStructListArgumentRequest(
    arg1: ChipStructs.UnitTestingClusterNestedStructList,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testListNestedStructListArgumentRequest(
    arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
    timedInvokeTimeoutMs: Int? = null
  ): BooleanResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testListInt8UReverseRequest(
    arg1: ArrayList<UByte>,
    timedInvokeTimeoutMs: Int? = null
  ): TestListInt8UReverseResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testEnumsRequest(
    arg1: UShort,
    arg2: UInt,
    timedInvokeTimeoutMs: Int? = null
  ): TestEnumsResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testNullableOptionalRequest(
    arg1: UByte?,
    timedInvokeTimeoutMs: Int? = null
  ): TestNullableOptionalResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testComplexNullableOptionalRequest(
    nullableInt: UShort?,
    optionalInt: UShort?,
    nullableOptionalInt: UShort?,
    nullableString: String?,
    optionalString: String?,
    nullableOptionalString: String?,
    nullableStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    optionalStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    nullableOptionalStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    nullableList: ArrayList<UInt>?,
    optionalList: ArrayList<UInt>?,
    nullableOptionalList: ArrayList<UInt>?,
    timedInvokeTimeoutMs: Int? = null
  ): TestComplexNullableOptionalResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun simpleStructEchoRequest(
    arg1: ChipStructs.UnitTestingClusterSimpleStruct,
    timedInvokeTimeoutMs: Int? = null
  ): SimpleStructResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun timedInvokeRequest(timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testSimpleOptionalArgumentRequest(arg1: Boolean?, timedInvokeTimeoutMs: Int? = null) {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testEmitTestEventRequest(
    arg1: UByte,
    arg2: UInt,
    arg3: Boolean,
    timedInvokeTimeoutMs: Int? = null
  ): TestEmitTestEventResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun testEmitTestFabricScopedEventRequest(
    arg1: UByte,
    timedInvokeTimeoutMs: Int? = null
  ): TestEmitTestFabricScopedEventResponse {
    if (timedInvokeTimeoutMs != null) {
      // Do the action with timedInvokeTimeoutMs
    } else {
      // Do the action without timedInvokeTimeoutMs
    }
  }

  suspend fun readBooleanAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeBooleanAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBooleanAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readBitmap8Attribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap8Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap8Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBitmap8Attribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readBitmap16Attribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap16Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap16Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBitmap16Attribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readBitmap32Attribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap32Attribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap32Attribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBitmap32Attribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readBitmap64Attribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap64Attribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeBitmap64Attribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeBitmap64Attribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readInt8uAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeInt8uAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt8uAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readInt16uAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeInt16uAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt16uAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readInt24uAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeInt24uAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeInt24uAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt24uAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readInt32uAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeInt32uAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeInt32uAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt32uAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readInt40uAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeInt40uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeInt40uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt40uAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readInt48uAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeInt48uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeInt48uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt48uAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readInt56uAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeInt56uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeInt56uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt56uAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readInt64uAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeInt64uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeInt64uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt64uAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readInt8sAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun writeInt8sAttribute(value: Byte) {
    // Implementation needs to be added here
  }

  suspend fun writeInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt8sAttribute(minInterval: Int, maxInterval: Int): Byte {
    // Implementation needs to be added here
  }

  suspend fun readInt16sAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeInt16sAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeInt16sAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt16sAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readInt24sAttribute(): Int {
    // Implementation needs to be added here
  }

  suspend fun writeInt24sAttribute(value: Int) {
    // Implementation needs to be added here
  }

  suspend fun writeInt24sAttribute(value: Int, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt24sAttribute(minInterval: Int, maxInterval: Int): Int {
    // Implementation needs to be added here
  }

  suspend fun readInt32sAttribute(): Int {
    // Implementation needs to be added here
  }

  suspend fun writeInt32sAttribute(value: Int) {
    // Implementation needs to be added here
  }

  suspend fun writeInt32sAttribute(value: Int, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt32sAttribute(minInterval: Int, maxInterval: Int): Int {
    // Implementation needs to be added here
  }

  suspend fun readInt40sAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeInt40sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeInt40sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt40sAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readInt48sAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeInt48sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeInt48sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt48sAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readInt56sAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeInt56sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeInt56sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt56sAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readInt64sAttribute(): Long {
    // Implementation needs to be added here
  }

  suspend fun writeInt64sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeInt64sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeInt64sAttribute(minInterval: Int, maxInterval: Int): Long {
    // Implementation needs to be added here
  }

  suspend fun readEnum8Attribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeEnum8Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeEnum8Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnum8Attribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readEnum16Attribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeEnum16Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeEnum16Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnum16Attribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readFloatSingleAttribute(): Float {
    // Implementation needs to be added here
  }

  suspend fun writeFloatSingleAttribute(value: Float) {
    // Implementation needs to be added here
  }

  suspend fun writeFloatSingleAttribute(value: Float, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeFloatSingleAttribute(minInterval: Int, maxInterval: Int): Float {
    // Implementation needs to be added here
  }

  suspend fun readFloatDoubleAttribute(): Double {
    // Implementation needs to be added here
  }

  suspend fun writeFloatDoubleAttribute(value: Double) {
    // Implementation needs to be added here
  }

  suspend fun writeFloatDoubleAttribute(value: Double, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeFloatDoubleAttribute(minInterval: Int, maxInterval: Int): Double {
    // Implementation needs to be added here
  }

  suspend fun readOctetStringAttribute(): OctetString {
    // Implementation needs to be added here
  }

  suspend fun writeOctetStringAttribute(value: ByteArray) {
    // Implementation needs to be added here
  }

  suspend fun writeOctetStringAttribute(value: ByteArray, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeOctetStringAttribute(minInterval: Int, maxInterval: Int): OctetString {
    // Implementation needs to be added here
  }

  suspend fun readListInt8uAttribute(): ListInt8uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeListInt8uAttribute(value: ArrayList<UByte>) {
    // Implementation needs to be added here
  }

  suspend fun writeListInt8uAttribute(value: ArrayList<UByte>, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeListInt8uAttribute(minInterval: Int, maxInterval: Int): ListInt8uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readListOctetStringAttribute(): ListOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeListOctetStringAttribute(value: ArrayList<ByteArray>) {
    // Implementation needs to be added here
  }

  suspend fun writeListOctetStringAttribute(value: ArrayList<ByteArray>, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeListOctetStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ListOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun readListStructOctetStringAttribute(): ListStructOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeListStructOctetStringAttribute(
    value: ArrayList<ChipStructs.UnitTestingClusterTestListStructOctet>
  ) {
    // Implementation needs to be added here
  }

  suspend fun writeListStructOctetStringAttribute(
    value: ArrayList<ChipStructs.UnitTestingClusterTestListStructOctet>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeListStructOctetStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ListStructOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun readLongOctetStringAttribute(): OctetString {
    // Implementation needs to be added here
  }

  suspend fun writeLongOctetStringAttribute(value: ByteArray) {
    // Implementation needs to be added here
  }

  suspend fun writeLongOctetStringAttribute(value: ByteArray, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeLongOctetStringAttribute(minInterval: Int, maxInterval: Int): OctetString {
    // Implementation needs to be added here
  }

  suspend fun readCharStringAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun writeCharStringAttribute(value: String) {
    // Implementation needs to be added here
  }

  suspend fun writeCharStringAttribute(value: String, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeCharStringAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readLongCharStringAttribute(): CharString {
    // Implementation needs to be added here
  }

  suspend fun writeLongCharStringAttribute(value: String) {
    // Implementation needs to be added here
  }

  suspend fun writeLongCharStringAttribute(value: String, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeLongCharStringAttribute(minInterval: Int, maxInterval: Int): CharString {
    // Implementation needs to be added here
  }

  suspend fun readEpochUsAttribute(): ULong {
    // Implementation needs to be added here
  }

  suspend fun writeEpochUsAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeEpochUsAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEpochUsAttribute(minInterval: Int, maxInterval: Int): ULong {
    // Implementation needs to be added here
  }

  suspend fun readEpochSAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun writeEpochSAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeEpochSAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEpochSAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readVendorIdAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeVendorIdAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeVendorIdAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeVendorIdAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readListNullablesAndOptionalsStructAttribute():
    ListNullablesAndOptionalsStructAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeListNullablesAndOptionalsStructAttribute(
    value: ArrayList<ChipStructs.UnitTestingClusterNullablesAndOptionalsStruct>
  ) {
    // Implementation needs to be added here
  }

  suspend fun writeListNullablesAndOptionalsStructAttribute(
    value: ArrayList<ChipStructs.UnitTestingClusterNullablesAndOptionalsStruct>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeListNullablesAndOptionalsStructAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ListNullablesAndOptionalsStructAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEnumAttrAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeEnumAttrAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeEnumAttrAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeEnumAttrAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readStructAttrAttribute(): StructAttrAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeStructAttrAttribute(value: ChipStructs.UnitTestingClusterSimpleStruct) {
    // Implementation needs to be added here
  }

  suspend fun writeStructAttrAttribute(
    value: ChipStructs.UnitTestingClusterSimpleStruct,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeStructAttrAttribute(
    minInterval: Int,
    maxInterval: Int
  ): StructAttrAttribute {
    // Implementation needs to be added here
  }

  suspend fun readRangeRestrictedInt8uAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt8uAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRangeRestrictedInt8uAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readRangeRestrictedInt8sAttribute(): Byte {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt8sAttribute(value: Byte) {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRangeRestrictedInt8sAttribute(minInterval: Int, maxInterval: Int): Byte {
    // Implementation needs to be added here
  }

  suspend fun readRangeRestrictedInt16uAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt16uAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRangeRestrictedInt16uAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  suspend fun readRangeRestrictedInt16sAttribute(): Short {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt16sAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeRangeRestrictedInt16sAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeRangeRestrictedInt16sAttribute(minInterval: Int, maxInterval: Int): Short {
    // Implementation needs to be added here
  }

  suspend fun readListLongOctetStringAttribute(): ListLongOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeListLongOctetStringAttribute(value: ArrayList<ByteArray>) {
    // Implementation needs to be added here
  }

  suspend fun writeListLongOctetStringAttribute(
    value: ArrayList<ByteArray>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeListLongOctetStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ListLongOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun readListFabricScopedAttribute(): ListFabricScopedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readListFabricScopedAttributeWithFabricFilter(
    isFabricFiltered: Boolean
  ): ListFabricScopedAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeListFabricScopedAttribute(
    value: ArrayList<ChipStructs.UnitTestingClusterTestFabricScoped>
  ) {
    // Implementation needs to be added here
  }

  suspend fun writeListFabricScopedAttribute(
    value: ArrayList<ChipStructs.UnitTestingClusterTestFabricScoped>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeListFabricScopedAttribute(
    minInterval: Int,
    maxInterval: Int
  ): ListFabricScopedAttribute {
    // Implementation needs to be added here
  }

  suspend fun readTimedWriteBooleanAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeTimedWriteBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeTimedWriteBooleanAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readGeneralErrorBooleanAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeGeneralErrorBooleanAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeGeneralErrorBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneralErrorBooleanAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readClusterErrorBooleanAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeClusterErrorBooleanAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeClusterErrorBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterErrorBooleanAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readUnsupportedAttribute(): Boolean {
    // Implementation needs to be added here
  }

  suspend fun writeUnsupportedAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeUnsupportedAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeUnsupportedAttribute(minInterval: Int, maxInterval: Int): Boolean {
    // Implementation needs to be added here
  }

  suspend fun readNullableBooleanAttribute(): NullableBooleanAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBooleanAttribute(value: Boolean) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBooleanAttribute(value: Boolean, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableBooleanAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableBooleanAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableBitmap8Attribute(): NullableBitmap8Attribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap8Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap8Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableBitmap8Attribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableBitmap8Attribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableBitmap16Attribute(): NullableBitmap16Attribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap16Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap16Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableBitmap16Attribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableBitmap16Attribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableBitmap32Attribute(): NullableBitmap32Attribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap32Attribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap32Attribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableBitmap32Attribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableBitmap32Attribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableBitmap64Attribute(): NullableBitmap64Attribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap64Attribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableBitmap64Attribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableBitmap64Attribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableBitmap64Attribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt8uAttribute(): NullableInt8uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt8uAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt8uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt8uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt16uAttribute(): NullableInt16uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt16uAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt16uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt16uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt24uAttribute(): NullableInt24uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt24uAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt24uAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt24uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt24uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt32uAttribute(): NullableInt32uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt32uAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt32uAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt32uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt32uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt40uAttribute(): NullableInt40uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt40uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt40uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt40uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt40uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt48uAttribute(): NullableInt48uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt48uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt48uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt48uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt48uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt56uAttribute(): NullableInt56uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt56uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt56uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt56uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt56uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt64uAttribute(): NullableInt64uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt64uAttribute(value: ULong) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt64uAttribute(value: ULong, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt64uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt64uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt8sAttribute(): NullableInt8sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt8sAttribute(value: Byte) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt8sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt8sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt16sAttribute(): NullableInt16sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt16sAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt16sAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt16sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt16sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt24sAttribute(): NullableInt24sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt24sAttribute(value: Int) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt24sAttribute(value: Int, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt24sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt24sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt32sAttribute(): NullableInt32sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt32sAttribute(value: Int) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt32sAttribute(value: Int, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt32sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt32sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt40sAttribute(): NullableInt40sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt40sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt40sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt40sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt40sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt48sAttribute(): NullableInt48sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt48sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt48sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt48sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt48sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt56sAttribute(): NullableInt56sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt56sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt56sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt56sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt56sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableInt64sAttribute(): NullableInt64sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt64sAttribute(value: Long) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableInt64sAttribute(value: Long, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableInt64sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableInt64sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableEnum8Attribute(): NullableEnum8Attribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableEnum8Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableEnum8Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableEnum8Attribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableEnum8Attribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableEnum16Attribute(): NullableEnum16Attribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableEnum16Attribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableEnum16Attribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableEnum16Attribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableEnum16Attribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableFloatSingleAttribute(): NullableFloatSingleAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableFloatSingleAttribute(value: Float) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableFloatSingleAttribute(value: Float, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableFloatSingleAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableFloatSingleAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableFloatDoubleAttribute(): NullableFloatDoubleAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableFloatDoubleAttribute(value: Double) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableFloatDoubleAttribute(value: Double, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableFloatDoubleAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableFloatDoubleAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableOctetStringAttribute(): NullableOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableOctetStringAttribute(value: ByteArray) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableOctetStringAttribute(value: ByteArray, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableOctetStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableOctetStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableCharStringAttribute(): NullableCharStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableCharStringAttribute(value: String) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableCharStringAttribute(value: String, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableCharStringAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableCharStringAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableEnumAttrAttribute(): NullableEnumAttrAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableEnumAttrAttribute(value: UInt) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableEnumAttrAttribute(value: UInt, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableEnumAttrAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableEnumAttrAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableStructAttribute(): NullableStructAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableStructAttribute(value: ChipStructs.UnitTestingClusterSimpleStruct) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableStructAttribute(
    value: ChipStructs.UnitTestingClusterSimpleStruct,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableStructAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableStructAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableRangeRestrictedInt8uAttribute(): NullableRangeRestrictedInt8uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt8uAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableRangeRestrictedInt8uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableRangeRestrictedInt8uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableRangeRestrictedInt8sAttribute(): NullableRangeRestrictedInt8sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt8sAttribute(value: Byte) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt8sAttribute(value: Byte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableRangeRestrictedInt8sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableRangeRestrictedInt8sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableRangeRestrictedInt16uAttribute(): NullableRangeRestrictedInt16uAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt16uAttribute(value: UShort) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt16uAttribute(value: UShort, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableRangeRestrictedInt16uAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableRangeRestrictedInt16uAttribute {
    // Implementation needs to be added here
  }

  suspend fun readNullableRangeRestrictedInt16sAttribute(): NullableRangeRestrictedInt16sAttribute {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt16sAttribute(value: Short) {
    // Implementation needs to be added here
  }

  suspend fun writeNullableRangeRestrictedInt16sAttribute(value: Short, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeNullableRangeRestrictedInt16sAttribute(
    minInterval: Int,
    maxInterval: Int
  ): NullableRangeRestrictedInt16sAttribute {
    // Implementation needs to be added here
  }

  suspend fun readWriteOnlyInt8uAttribute(): UByte {
    // Implementation needs to be added here
  }

  suspend fun writeWriteOnlyInt8uAttribute(value: UByte) {
    // Implementation needs to be added here
  }

  suspend fun writeWriteOnlyInt8uAttribute(value: UByte, timedWriteTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  suspend fun subscribeWriteOnlyInt8uAttribute(minInterval: Int, maxInterval: Int): UByte {
    // Implementation needs to be added here
  }

  suspend fun readGeneratedCommandListAttribute(): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeGeneratedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): GeneratedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAcceptedCommandListAttribute(): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAcceptedCommandListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AcceptedCommandListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readEventListAttribute(): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeEventListAttribute(minInterval: Int, maxInterval: Int): EventListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readAttributeListAttribute(): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun subscribeAttributeListAttribute(
    minInterval: Int,
    maxInterval: Int
  ): AttributeListAttribute {
    // Implementation needs to be added here
  }

  suspend fun readFeatureMapAttribute(): UInt {
    // Implementation needs to be added here
  }

  suspend fun subscribeFeatureMapAttribute(minInterval: Int, maxInterval: Int): UInt {
    // Implementation needs to be added here
  }

  suspend fun readClusterRevisionAttribute(): UShort {
    // Implementation needs to be added here
  }

  suspend fun subscribeClusterRevisionAttribute(minInterval: Int, maxInterval: Int): UShort {
    // Implementation needs to be added here
  }

  companion object {
    const val CLUSTER_ID: UInt = 4294048773u
  }
}
