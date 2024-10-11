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
package matter.controller

sealed class BooleanSubscriptionState {
  data class Success(val value: Boolean) : BooleanSubscriptionState()

  data class Error(val exception: Exception) : BooleanSubscriptionState()

  object SubscriptionEstablished : BooleanSubscriptionState()
}

sealed class ByteSubscriptionState {
  data class Success(val value: Byte) : ByteSubscriptionState()

  data class Error(val exception: Exception) : ByteSubscriptionState()

  object SubscriptionEstablished : ByteSubscriptionState()
}

sealed class UByteSubscriptionState {
  data class Success(val value: UByte) : UByteSubscriptionState()

  data class Error(val exception: Exception) : UByteSubscriptionState()

  object SubscriptionEstablished : UByteSubscriptionState()
}

sealed class ShortSubscriptionState {
  data class Success(val value: Short) : ShortSubscriptionState()

  data class Error(val exception: Exception) : ShortSubscriptionState()

  object SubscriptionEstablished : ShortSubscriptionState()
}

sealed class UShortSubscriptionState {
  data class Success(val value: UShort) : UShortSubscriptionState()

  data class Error(val exception: Exception) : UShortSubscriptionState()

  object SubscriptionEstablished : UShortSubscriptionState()
}

sealed class IntSubscriptionState {
  data class Success(val value: Int) : IntSubscriptionState()

  data class Error(val exception: Exception) : IntSubscriptionState()

  object SubscriptionEstablished : IntSubscriptionState()
}

sealed class UIntSubscriptionState {
  data class Success(val value: UInt) : UIntSubscriptionState()

  data class Error(val exception: Exception) : UIntSubscriptionState()

  object SubscriptionEstablished : UIntSubscriptionState()
}

sealed class LongSubscriptionState {
  data class Success(val value: Long) : LongSubscriptionState()

  data class Error(val exception: Exception) : LongSubscriptionState()

  object SubscriptionEstablished : LongSubscriptionState()
}

sealed class ULongSubscriptionState {
  data class Success(val value: ULong) : ULongSubscriptionState()

  data class Error(val exception: Exception) : ULongSubscriptionState()

  object SubscriptionEstablished : ULongSubscriptionState()
}

sealed class FloatSubscriptionState {
  data class Success(val value: Float) : FloatSubscriptionState()

  data class Error(val exception: Exception) : FloatSubscriptionState()

  object SubscriptionEstablished : FloatSubscriptionState()
}

sealed class DoubleSubscriptionState {
  data class Success(val value: Double) : DoubleSubscriptionState()

  data class Error(val exception: Exception) : DoubleSubscriptionState()

  object SubscriptionEstablished : DoubleSubscriptionState()
}

sealed class CharSubscriptionState {
  data class Success(val value: Char) : CharSubscriptionState()

  data class Error(val exception: Exception) : CharSubscriptionState()

  object SubscriptionEstablished : CharSubscriptionState()
}

sealed class StringSubscriptionState {
  data class Success(val value: String) : StringSubscriptionState()

  data class Error(val exception: Exception) : StringSubscriptionState()

  object SubscriptionEstablished : StringSubscriptionState()
}

sealed class ByteArraySubscriptionState {
  data class Success(val value: ByteArray) : ByteArraySubscriptionState()

  data class Error(val exception: Exception) : ByteArraySubscriptionState()

  object SubscriptionEstablished : ByteArraySubscriptionState()
}
