/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2015-2017 Nest Labs, Inc.
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

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <lib/core/CHIPError.h>
#include <lib/core/CHIPTLV.h>

namespace chip {

namespace TLV {

namespace Debug {

typedef void (*PrinterFunc)(const char * aFormat, ...);

/*
 * @brief
 *
 * A default implementation of the PrinterFunc that prints
 * to stdout using vprintf.
 */
void StdoutPrinter(const char * aFormat, ...);

/*
 * @brief
 *
 * A function that can be used to print out TLV contents into a textual representation
 * as specified in the Matter TLV Text specification.
 */
CHIP_ERROR Print(TLV::TLVReader & reader, PrinterFunc printer = StdoutPrinter);

} // namespace Debug

} // namespace TLV

} // namespace chip
