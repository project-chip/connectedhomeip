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
  companion object {
    const val CLUSTER_ID: UInt = 4294048773u
  }

  fun test(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun test(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun testNotHandled(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun testNotHandled(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun testSpecific(callback: TestSpecificResponseCallback) {
    // Implementation needs to be added here
  }

  fun testSpecific(callback: TestSpecificResponseCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun testUnknownCommand(callback: DefaultClusterCallback) {
    // Implementation needs to be added here
  }

  fun testUnknownCommand(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun testAddArguments(callback: TestAddArgumentsResponseCallback, arg1: Integer, arg2: Integer) {
    // Implementation needs to be added here
  }

  fun testAddArguments(
    callback: TestAddArgumentsResponseCallback,
    arg1: Integer,
    arg2: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testSimpleArgumentRequest(callback: TestSimpleArgumentResponseCallback, arg1: Boolean) {
    // Implementation needs to be added here
  }

  fun testSimpleArgumentRequest(
    callback: TestSimpleArgumentResponseCallback,
    arg1: Boolean,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testStructArrayArgumentRequest(
    callback: TestStructArrayArgumentResponseCallback,
    arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
    arg2: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
    arg3: ArrayList<Integer>,
    arg4: ArrayList<Boolean>,
    arg5: Integer,
    arg6: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun testStructArrayArgumentRequest(
    callback: TestStructArrayArgumentResponseCallback,
    arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
    arg2: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
    arg3: ArrayList<Integer>,
    arg4: ArrayList<Boolean>,
    arg5: Integer,
    arg6: Boolean,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testStructArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ChipStructs.UnitTestingClusterSimpleStruct
  ) {
    // Implementation needs to be added here
  }

  fun testStructArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ChipStructs.UnitTestingClusterSimpleStruct,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testNestedStructArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ChipStructs.UnitTestingClusterNestedStruct
  ) {
    // Implementation needs to be added here
  }

  fun testNestedStructArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ChipStructs.UnitTestingClusterNestedStruct,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testListStructArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>
  ) {
    // Implementation needs to be added here
  }

  fun testListStructArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testListInt8UArgumentRequest(callback: BooleanResponseCallback, arg1: ArrayList<Integer>) {
    // Implementation needs to be added here
  }

  fun testListInt8UArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ArrayList<Integer>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testNestedStructListArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ChipStructs.UnitTestingClusterNestedStructList
  ) {
    // Implementation needs to be added here
  }

  fun testNestedStructListArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ChipStructs.UnitTestingClusterNestedStructList,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testListNestedStructListArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>
  ) {
    // Implementation needs to be added here
  }

  fun testListNestedStructListArgumentRequest(
    callback: BooleanResponseCallback,
    arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testListInt8UReverseRequest(
    callback: TestListInt8UReverseResponseCallback,
    arg1: ArrayList<Integer>
  ) {
    // Implementation needs to be added here
  }

  fun testListInt8UReverseRequest(
    callback: TestListInt8UReverseResponseCallback,
    arg1: ArrayList<Integer>,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testEnumsRequest(callback: TestEnumsResponseCallback, arg1: Integer, arg2: Integer) {
    // Implementation needs to be added here
  }

  fun testEnumsRequest(
    callback: TestEnumsResponseCallback,
    arg1: Integer,
    arg2: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testNullableOptionalRequest(callback: TestNullableOptionalResponseCallback, arg1: Integer?) {
    // Implementation needs to be added here
  }

  fun testNullableOptionalRequest(
    callback: TestNullableOptionalResponseCallback,
    arg1: Integer?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testComplexNullableOptionalRequest(
    callback: TestComplexNullableOptionalResponseCallback,
    nullableInt: Integer?,
    optionalInt: Integer?,
    nullableOptionalInt: Integer?,
    nullableString: String?,
    optionalString: String?,
    nullableOptionalString: String?,
    nullableStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    optionalStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    nullableOptionalStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    nullableList: ArrayList<Integer>?,
    optionalList: ArrayList<Integer>?,
    nullableOptionalList: ArrayList<Integer>?
  ) {
    // Implementation needs to be added here
  }

  fun testComplexNullableOptionalRequest(
    callback: TestComplexNullableOptionalResponseCallback,
    nullableInt: Integer?,
    optionalInt: Integer?,
    nullableOptionalInt: Integer?,
    nullableString: String?,
    optionalString: String?,
    nullableOptionalString: String?,
    nullableStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    optionalStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    nullableOptionalStruct: ChipStructs.UnitTestingClusterSimpleStruct?,
    nullableList: ArrayList<Integer>?,
    optionalList: ArrayList<Integer>?,
    nullableOptionalList: ArrayList<Integer>?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun simpleStructEchoRequest(
    callback: SimpleStructResponseCallback,
    arg1: ChipStructs.UnitTestingClusterSimpleStruct
  ) {
    // Implementation needs to be added here
  }

  fun simpleStructEchoRequest(
    callback: SimpleStructResponseCallback,
    arg1: ChipStructs.UnitTestingClusterSimpleStruct,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun timedInvokeRequest(callback: DefaultClusterCallback, timedInvokeTimeoutMs: Int) {
    // Implementation needs to be added here
  }

  fun testSimpleOptionalArgumentRequest(callback: DefaultClusterCallback, arg1: Boolean?) {
    // Implementation needs to be added here
  }

  fun testSimpleOptionalArgumentRequest(
    callback: DefaultClusterCallback,
    arg1: Boolean?,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testEmitTestEventRequest(
    callback: TestEmitTestEventResponseCallback,
    arg1: Integer,
    arg2: Integer,
    arg3: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun testEmitTestEventRequest(
    callback: TestEmitTestEventResponseCallback,
    arg1: Integer,
    arg2: Integer,
    arg3: Boolean,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun testEmitTestFabricScopedEventRequest(
    callback: TestEmitTestFabricScopedEventResponseCallback,
    arg1: Integer
  ) {
    // Implementation needs to be added here
  }

  fun testEmitTestFabricScopedEventRequest(
    callback: TestEmitTestFabricScopedEventResponseCallback,
    arg1: Integer,
    timedInvokeTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  interface TestSpecificResponseCallback {
    fun onSuccess(returnValue: Integer)

    fun onError(error: Exception)
  }

  interface TestAddArgumentsResponseCallback {
    fun onSuccess(returnValue: Integer)

    fun onError(error: Exception)
  }

  interface TestSimpleArgumentResponseCallback {
    fun onSuccess(returnValue: Boolean)

    fun onError(error: Exception)
  }

  interface TestStructArrayArgumentResponseCallback {
    fun onSuccess(
      arg1: ArrayList<ChipStructs.UnitTestingClusterNestedStructList>,
      arg2: ArrayList<ChipStructs.UnitTestingClusterSimpleStruct>,
      arg3: ArrayList<Integer>,
      arg4: ArrayList<Boolean>,
      arg5: Integer,
      arg6: Boolean
    )

    fun onError(error: Exception)
  }

  interface BooleanResponseCallback {
    fun onSuccess(value: Boolean)

    fun onError(error: Exception)
  }

  interface TestListInt8UReverseResponseCallback {
    fun onSuccess(arg1: ArrayList<Integer>)

    fun onError(error: Exception)
  }

  interface TestEnumsResponseCallback {
    fun onSuccess(arg1: Integer, arg2: Integer)

    fun onError(error: Exception)
  }

  interface TestNullableOptionalResponseCallback {
    fun onSuccess(wasPresent: Boolean, wasNull: Boolean?, value: Integer?, originalValue: Integer?)

    fun onError(error: Exception)
  }

  interface TestComplexNullableOptionalResponseCallback {
    fun onSuccess(
      nullableIntWasNull: Boolean,
      nullableIntValue: Integer?,
      optionalIntWasPresent: Boolean,
      optionalIntValue: Integer?,
      nullableOptionalIntWasPresent: Boolean,
      nullableOptionalIntWasNull: Boolean?,
      nullableOptionalIntValue: Integer?,
      nullableStringWasNull: Boolean,
      nullableStringValue: String?,
      optionalStringWasPresent: Boolean,
      optionalStringValue: String?,
      nullableOptionalStringWasPresent: Boolean,
      nullableOptionalStringWasNull: Boolean?,
      nullableOptionalStringValue: String?,
      nullableStructWasNull: Boolean,
      nullableStructValue: ChipStructs.UnitTestingClusterSimpleStruct?,
      optionalStructWasPresent: Boolean,
      optionalStructValue: ChipStructs.UnitTestingClusterSimpleStruct?,
      nullableOptionalStructWasPresent: Boolean,
      nullableOptionalStructWasNull: Boolean?,
      nullableOptionalStructValue: ChipStructs.UnitTestingClusterSimpleStruct?,
      nullableListWasNull: Boolean,
      nullableListValue: ArrayList<Integer>?,
      optionalListWasPresent: Boolean,
      optionalListValue: ArrayList<Integer>?,
      nullableOptionalListWasPresent: Boolean,
      nullableOptionalListWasNull: Boolean?,
      nullableOptionalListValue: ArrayList<Integer>?
    )

    fun onError(error: Exception)
  }

  interface SimpleStructResponseCallback {
    fun onSuccess(arg1: ChipStructs.UnitTestingClusterSimpleStruct)

    fun onError(error: Exception)
  }

  interface TestEmitTestEventResponseCallback {
    fun onSuccess(value: Long)

    fun onError(error: Exception)
  }

  interface TestEmitTestFabricScopedEventResponseCallback {
    fun onSuccess(value: Long)

    fun onError(error: Exception)
  }

  interface ListInt8uAttributeCallback {
    fun onSuccess(value: ArrayList<Integer>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ListOctetStringAttributeCallback {
    fun onSuccess(value: ArrayList<ByteArray>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ListStructOctetStringAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.UnitTestingClusterTestListStructOctet>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ListNullablesAndOptionalsStructAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.UnitTestingClusterNullablesAndOptionalsStruct>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface StructAttrAttributeCallback {
    fun onSuccess(value: ChipStructs.UnitTestingClusterSimpleStruct)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ListLongOctetStringAttributeCallback {
    fun onSuccess(value: ArrayList<ByteArray>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface ListFabricScopedAttributeCallback {
    fun onSuccess(value: ArrayList<ChipStructs.UnitTestingClusterTestFabricScoped>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableBooleanAttributeCallback {
    fun onSuccess(value: Boolean?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableBitmap8AttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableBitmap16AttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableBitmap32AttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableBitmap64AttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt8uAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt16uAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt24uAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt32uAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt40uAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt48uAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt56uAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt64uAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt8sAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt16sAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt24sAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt32sAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt40sAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt48sAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt56sAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableInt64sAttributeCallback {
    fun onSuccess(value: Long?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableEnum8AttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableEnum16AttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableFloatSingleAttributeCallback {
    fun onSuccess(value: Float?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableFloatDoubleAttributeCallback {
    fun onSuccess(value: Double?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableOctetStringAttributeCallback {
    fun onSuccess(value: ByteArray?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableCharStringAttributeCallback {
    fun onSuccess(value: String?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableEnumAttrAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableStructAttributeCallback {
    fun onSuccess(value: ChipStructs.UnitTestingClusterSimpleStruct?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableRangeRestrictedInt8uAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableRangeRestrictedInt8sAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableRangeRestrictedInt16uAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface NullableRangeRestrictedInt16sAttributeCallback {
    fun onSuccess(value: Integer?)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface GeneratedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AcceptedCommandListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface EventListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  interface AttributeListAttributeCallback {
    fun onSuccess(value: ArrayList<Long>)

    fun onError(ex: Exception)

    fun onSubscriptionEstablished(subscriptionId: Long)
  }

  fun readBooleanAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBooleanAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeBooleanAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBooleanAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBitmap8Attribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBitmap8Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBitmap8Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBitmap8Attribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBitmap16Attribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBitmap16Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeBitmap16Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBitmap16Attribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBitmap32Attribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBitmap32Attribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeBitmap32Attribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBitmap32Attribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readBitmap64Attribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeBitmap64Attribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeBitmap64Attribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeBitmap64Attribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt8uAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt8uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeInt8uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt8uAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt16uAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt16uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeInt16uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt16uAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt24uAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt24uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt24uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt24uAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt32uAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt32uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt32uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt32uAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt40uAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt40uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt40uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt40uAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt48uAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt48uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt48uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt48uAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt56uAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt56uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt56uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt56uAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt64uAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt64uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt64uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt64uAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt8sAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt8sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeInt8sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt8sAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt16sAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt16sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeInt16sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt16sAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt24sAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt24sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt24sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt24sAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt32sAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt32sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt32sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt32sAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt40sAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt40sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt40sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt40sAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt48sAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt48sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt48sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt48sAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt56sAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt56sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt56sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt56sAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readInt64sAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeInt64sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeInt64sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeInt64sAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnum8Attribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnum8Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeEnum8Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnum8Attribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnum16Attribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnum16Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeEnum16Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnum16Attribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFloatSingleAttribute(callback: FloatAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeFloatSingleAttribute(callback: DefaultClusterCallback, value: Float) {
    // Implementation needs to be added here
  }

  fun writeFloatSingleAttribute(
    callback: DefaultClusterCallback,
    value: Float,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeFloatSingleAttribute(
    callback: FloatAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFloatDoubleAttribute(callback: DoubleAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeFloatDoubleAttribute(callback: DefaultClusterCallback, value: Double) {
    // Implementation needs to be added here
  }

  fun writeFloatDoubleAttribute(
    callback: DefaultClusterCallback,
    value: Double,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeFloatDoubleAttribute(
    callback: DoubleAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readOctetStringAttribute(callback: OctetStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeOctetStringAttribute(callback: DefaultClusterCallback, value: ByteArray) {
    // Implementation needs to be added here
  }

  fun writeOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ByteArray,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeOctetStringAttribute(
    callback: OctetStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readListInt8uAttribute(callback: ListInt8uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeListInt8uAttribute(callback: DefaultClusterCallback, value: ArrayList<Integer>) {
    // Implementation needs to be added here
  }

  fun writeListInt8uAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<Integer>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeListInt8uAttribute(
    callback: ListInt8uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readListOctetStringAttribute(callback: ListOctetStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeListOctetStringAttribute(callback: DefaultClusterCallback, value: ArrayList<ByteArray>) {
    // Implementation needs to be added here
  }

  fun writeListOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ByteArray>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeListOctetStringAttribute(
    callback: ListOctetStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readListStructOctetStringAttribute(callback: ListStructOctetStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeListStructOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.UnitTestingClusterTestListStructOctet>
  ) {
    // Implementation needs to be added here
  }

  fun writeListStructOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.UnitTestingClusterTestListStructOctet>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeListStructOctetStringAttribute(
    callback: ListStructOctetStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLongOctetStringAttribute(callback: OctetStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLongOctetStringAttribute(callback: DefaultClusterCallback, value: ByteArray) {
    // Implementation needs to be added here
  }

  fun writeLongOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ByteArray,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLongOctetStringAttribute(
    callback: OctetStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readCharStringAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeCharStringAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeCharStringAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeCharStringAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readLongCharStringAttribute(callback: CharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeLongCharStringAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeLongCharStringAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeLongCharStringAttribute(
    callback: CharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEpochUsAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEpochUsAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeEpochUsAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEpochUsAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEpochSAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEpochSAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeEpochSAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEpochSAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readVendorIdAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeVendorIdAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeVendorIdAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeVendorIdAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readListNullablesAndOptionalsStructAttribute(
    callback: ListNullablesAndOptionalsStructAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeListNullablesAndOptionalsStructAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.UnitTestingClusterNullablesAndOptionalsStruct>
  ) {
    // Implementation needs to be added here
  }

  fun writeListNullablesAndOptionalsStructAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.UnitTestingClusterNullablesAndOptionalsStruct>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeListNullablesAndOptionalsStructAttribute(
    callback: ListNullablesAndOptionalsStructAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEnumAttrAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeEnumAttrAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeEnumAttrAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeEnumAttrAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readStructAttrAttribute(callback: StructAttrAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeStructAttrAttribute(
    callback: DefaultClusterCallback,
    value: ChipStructs.UnitTestingClusterSimpleStruct
  ) {
    // Implementation needs to be added here
  }

  fun writeStructAttrAttribute(
    callback: DefaultClusterCallback,
    value: ChipStructs.UnitTestingClusterSimpleStruct,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeStructAttrAttribute(
    callback: StructAttrAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRangeRestrictedInt8uAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt8uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt8uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRangeRestrictedInt8uAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRangeRestrictedInt8sAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt8sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt8sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRangeRestrictedInt8sAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRangeRestrictedInt16uAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt16uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt16uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRangeRestrictedInt16uAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readRangeRestrictedInt16sAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt16sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeRangeRestrictedInt16sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeRangeRestrictedInt16sAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readListLongOctetStringAttribute(callback: ListLongOctetStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeListLongOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ByteArray>
  ) {
    // Implementation needs to be added here
  }

  fun writeListLongOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ByteArray>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeListLongOctetStringAttribute(
    callback: ListLongOctetStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readListFabricScopedAttribute(callback: ListFabricScopedAttributeCallback) {
    // Implementation needs to be added here
  }

  fun readListFabricScopedAttributeWithFabricFilter(
    callback: ListFabricScopedAttributeCallback,
    isFabricFiltered: Boolean
  ) {
    // Implementation needs to be added here
  }

  fun writeListFabricScopedAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.UnitTestingClusterTestFabricScoped>
  ) {
    // Implementation needs to be added here
  }

  fun writeListFabricScopedAttribute(
    callback: DefaultClusterCallback,
    value: ArrayList<ChipStructs.UnitTestingClusterTestFabricScoped>,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeListFabricScopedAttribute(
    callback: ListFabricScopedAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readTimedWriteBooleanAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeTimedWriteBooleanAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeTimedWriteBooleanAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneralErrorBooleanAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeGeneralErrorBooleanAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeGeneralErrorBooleanAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeGeneralErrorBooleanAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterErrorBooleanAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeClusterErrorBooleanAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeClusterErrorBooleanAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeClusterErrorBooleanAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readUnsupportedAttribute(callback: BooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeUnsupportedAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeUnsupportedAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeUnsupportedAttribute(
    callback: BooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableBooleanAttribute(callback: NullableBooleanAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableBooleanAttribute(callback: DefaultClusterCallback, value: Boolean) {
    // Implementation needs to be added here
  }

  fun writeNullableBooleanAttribute(
    callback: DefaultClusterCallback,
    value: Boolean,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableBooleanAttribute(
    callback: NullableBooleanAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableBitmap8Attribute(callback: NullableBitmap8AttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap8Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap8Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableBitmap8Attribute(
    callback: NullableBitmap8AttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableBitmap16Attribute(callback: NullableBitmap16AttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap16Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap16Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableBitmap16Attribute(
    callback: NullableBitmap16AttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableBitmap32Attribute(callback: NullableBitmap32AttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap32Attribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap32Attribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableBitmap32Attribute(
    callback: NullableBitmap32AttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableBitmap64Attribute(callback: NullableBitmap64AttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap64Attribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableBitmap64Attribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableBitmap64Attribute(
    callback: NullableBitmap64AttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt8uAttribute(callback: NullableInt8uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt8uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableInt8uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt8uAttribute(
    callback: NullableInt8uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt16uAttribute(callback: NullableInt16uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt16uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableInt16uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt16uAttribute(
    callback: NullableInt16uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt24uAttribute(callback: NullableInt24uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt24uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt24uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt24uAttribute(
    callback: NullableInt24uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt32uAttribute(callback: NullableInt32uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt32uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt32uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt32uAttribute(
    callback: NullableInt32uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt40uAttribute(callback: NullableInt40uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt40uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt40uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt40uAttribute(
    callback: NullableInt40uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt48uAttribute(callback: NullableInt48uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt48uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt48uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt48uAttribute(
    callback: NullableInt48uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt56uAttribute(callback: NullableInt56uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt56uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt56uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt56uAttribute(
    callback: NullableInt56uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt64uAttribute(callback: NullableInt64uAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt64uAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt64uAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt64uAttribute(
    callback: NullableInt64uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt8sAttribute(callback: NullableInt8sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt8sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableInt8sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt8sAttribute(
    callback: NullableInt8sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt16sAttribute(callback: NullableInt16sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt16sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableInt16sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt16sAttribute(
    callback: NullableInt16sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt24sAttribute(callback: NullableInt24sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt24sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt24sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt24sAttribute(
    callback: NullableInt24sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt32sAttribute(callback: NullableInt32sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt32sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt32sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt32sAttribute(
    callback: NullableInt32sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt40sAttribute(callback: NullableInt40sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt40sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt40sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt40sAttribute(
    callback: NullableInt40sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt48sAttribute(callback: NullableInt48sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt48sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt48sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt48sAttribute(
    callback: NullableInt48sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt56sAttribute(callback: NullableInt56sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt56sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt56sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt56sAttribute(
    callback: NullableInt56sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableInt64sAttribute(callback: NullableInt64sAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableInt64sAttribute(callback: DefaultClusterCallback, value: Long) {
    // Implementation needs to be added here
  }

  fun writeNullableInt64sAttribute(
    callback: DefaultClusterCallback,
    value: Long,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableInt64sAttribute(
    callback: NullableInt64sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableEnum8Attribute(callback: NullableEnum8AttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableEnum8Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableEnum8Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableEnum8Attribute(
    callback: NullableEnum8AttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableEnum16Attribute(callback: NullableEnum16AttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableEnum16Attribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableEnum16Attribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableEnum16Attribute(
    callback: NullableEnum16AttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableFloatSingleAttribute(callback: NullableFloatSingleAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableFloatSingleAttribute(callback: DefaultClusterCallback, value: Float) {
    // Implementation needs to be added here
  }

  fun writeNullableFloatSingleAttribute(
    callback: DefaultClusterCallback,
    value: Float,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableFloatSingleAttribute(
    callback: NullableFloatSingleAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableFloatDoubleAttribute(callback: NullableFloatDoubleAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableFloatDoubleAttribute(callback: DefaultClusterCallback, value: Double) {
    // Implementation needs to be added here
  }

  fun writeNullableFloatDoubleAttribute(
    callback: DefaultClusterCallback,
    value: Double,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableFloatDoubleAttribute(
    callback: NullableFloatDoubleAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableOctetStringAttribute(callback: NullableOctetStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableOctetStringAttribute(callback: DefaultClusterCallback, value: ByteArray) {
    // Implementation needs to be added here
  }

  fun writeNullableOctetStringAttribute(
    callback: DefaultClusterCallback,
    value: ByteArray,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableOctetStringAttribute(
    callback: NullableOctetStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableCharStringAttribute(callback: NullableCharStringAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableCharStringAttribute(callback: DefaultClusterCallback, value: String) {
    // Implementation needs to be added here
  }

  fun writeNullableCharStringAttribute(
    callback: DefaultClusterCallback,
    value: String,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableCharStringAttribute(
    callback: NullableCharStringAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableEnumAttrAttribute(callback: NullableEnumAttrAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableEnumAttrAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableEnumAttrAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableEnumAttrAttribute(
    callback: NullableEnumAttrAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableStructAttribute(callback: NullableStructAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeNullableStructAttribute(
    callback: DefaultClusterCallback,
    value: ChipStructs.UnitTestingClusterSimpleStruct
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableStructAttribute(
    callback: DefaultClusterCallback,
    value: ChipStructs.UnitTestingClusterSimpleStruct,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableStructAttribute(
    callback: NullableStructAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableRangeRestrictedInt8uAttribute(
    callback: NullableRangeRestrictedInt8uAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt8uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt8uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableRangeRestrictedInt8uAttribute(
    callback: NullableRangeRestrictedInt8uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableRangeRestrictedInt8sAttribute(
    callback: NullableRangeRestrictedInt8sAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt8sAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt8sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableRangeRestrictedInt8sAttribute(
    callback: NullableRangeRestrictedInt8sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableRangeRestrictedInt16uAttribute(
    callback: NullableRangeRestrictedInt16uAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt16uAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt16uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableRangeRestrictedInt16uAttribute(
    callback: NullableRangeRestrictedInt16uAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readNullableRangeRestrictedInt16sAttribute(
    callback: NullableRangeRestrictedInt16sAttributeCallback
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt16sAttribute(
    callback: DefaultClusterCallback,
    value: Integer
  ) {
    // Implementation needs to be added here
  }

  fun writeNullableRangeRestrictedInt16sAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeNullableRangeRestrictedInt16sAttribute(
    callback: NullableRangeRestrictedInt16sAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readWriteOnlyInt8uAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun writeWriteOnlyInt8uAttribute(callback: DefaultClusterCallback, value: Integer) {
    // Implementation needs to be added here
  }

  fun writeWriteOnlyInt8uAttribute(
    callback: DefaultClusterCallback,
    value: Integer,
    timedWriteTimeoutMs: Int
  ) {
    // Implementation needs to be added here
  }

  fun subscribeWriteOnlyInt8uAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readGeneratedCommandListAttribute(callback: GeneratedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeGeneratedCommandListAttribute(
    callback: GeneratedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAcceptedCommandListAttribute(callback: AcceptedCommandListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAcceptedCommandListAttribute(
    callback: AcceptedCommandListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readEventListAttribute(callback: EventListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeEventListAttribute(
    callback: EventListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readAttributeListAttribute(callback: AttributeListAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeAttributeListAttribute(
    callback: AttributeListAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readFeatureMapAttribute(callback: LongAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeFeatureMapAttribute(
    callback: LongAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }

  fun readClusterRevisionAttribute(callback: IntegerAttributeCallback) {
    // Implementation needs to be added here
  }

  fun subscribeClusterRevisionAttribute(
    callback: IntegerAttributeCallback,
    minInterval: Int,
    maxInterval: Int
  ) {
    // Implementation needs to be added here
  }
}
