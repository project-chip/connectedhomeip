/*
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */
package com.matter.controller.commands.common

enum class ArgumentType {
  NUMBER_INT8,
  NUMBER_INT16,
  NUMBER_INT32,
  NUMBER_INT64,
  FLOAT,
  DOUBLE,
  BOOL,
  STRING,
  CHARSTRING,
  OCTETSTRING,
  ATTRIBUTE,
  ADDRESS,
  COMPLEX,
  CUSTOM,
  VECTOR_BOOL,
  VECTOR16,
  VECTOR32,
  VECTOR_CUSTOM
}
