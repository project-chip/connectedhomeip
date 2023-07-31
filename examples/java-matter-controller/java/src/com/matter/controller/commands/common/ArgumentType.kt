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
