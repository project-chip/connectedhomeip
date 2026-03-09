/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <lib/support/BytesToHex.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceConfig.h>

#include <string.h>

#if SILABS_LOG_OUT_UART
#include "uart.h"
#else
#include "SEGGER_RTT.h"
#endif

// TODO: The FreeRTOS CMSIS OS2 wrapper does not implement osKernelSuspend().
// Using vTaskSuspendAll() directly until the SDK provides the wrapper.
#include "FreeRTOS.h"
#include "task.h"

using chip::Encoding::HexFlags;
using chip::Encoding::Uint32ToHex;

namespace {
// PrintAddr formatting constants
constexpr size_t kAddrHexDigits = 8; // Hex digits for 32-bit address
constexpr size_t kAddrSuffixLen = 2; // \r\n
constexpr size_t kAddrNullTerm  = 1; // \0

// Stack walking constants
constexpr int kMaxStackScan = 64; // Max stack entries to scan
constexpr int kMaxAddrPrint = 10; // Max addresses to print
} // namespace

// Linker-provided symbols for code region detection
extern "C" {
extern char __etext[]; // End of text section (common to all platforms)
#if !defined(SLI_SI91X_MCU_INTERFACE)
extern char linker_vectors_begin[]; // Start of code (EFR32 Series 2/3 only)
extern char __ramfuncs_start__[];   // Start of RAM functions (EFR32 Series 2/3 only)
extern char __ramfuncs_end__[];     // End of RAM functions (EFR32 Series 2/3 only)
#endif
}

/**
 * @brief Prints a prefix string followed by an 8-character uppercase hex address and \r\n.
 *
 * Output format: "<prefix>XXXXXXXX\r\n"
 * Example: PrintAddr("SP=0x", 0x20001000) outputs "SP=0x20001000\r\n"
 *
 * @param prefix String prefix to print before the hex address (max 32 chars)
 * @param addr   Address value to format as 8-digit uppercase hex
 */
template <size_t N>
static void PrintAddr(const char (&prefix)[N], uintptr_t addr)
{
    static_assert(N > 0, "Prefix must not be empty");
    constexpr size_t prefixLen = N - 1; // Exclude null terminator
    constexpr size_t bufSize   = prefixLen + kAddrHexDigits + kAddrSuffixLen + kAddrNullTerm;

    char msg[bufSize];

    // Calculate positions - sizes are compile-time verified
    constexpr size_t hexPos    = prefixLen;
    constexpr size_t suffixPos = hexPos + kAddrHexDigits;
    constexpr size_t totalLen  = suffixPos + kAddrSuffixLen;

    memcpy(msg, prefix, prefixLen);
    RETURN_SAFELY_IGNORED(Uint32ToHex(addr, &msg[hexPos], kAddrHexDigits, HexFlags::kUppercase));
    msg[suffixPos]     = '\r';
    msg[suffixPos + 1] = '\n';
    msg[totalLen]      = '\0';

#if SILABS_LOG_OUT_UART
    uartForceTransmit(msg, totalLen);
#else
    SEGGER_RTT_WriteNoLock(0, msg, totalLen);
#endif
}

extern "C" void chipDie(void)
{
    // Get stack pointer and print callstack
    uintptr_t sp;
    __asm volatile("mov %0, sp" : "=r"(sp));

    PrintAddr("chipDie SP=0x", sp);
    PrintAddr("  LR: 0x", reinterpret_cast<uintptr_t>(__builtin_return_address(0)));

    // Walk stack - print potential return addresses
    // Use linker symbols to determine valid code address ranges
#if defined(SLI_SI91X_MCU_INTERFACE)
    // SiWx917: Flash is at 0x08202000, use heuristic for start since linker_vectors_begin unavailable
    uintptr_t codeStart = 0x08000000;
#else
    // EFR32 Series 2/3: Use linker symbols for code and ram functions
    uintptr_t codeStart    = reinterpret_cast<uintptr_t>(linker_vectors_begin);
    uintptr_t ramCodeStart = reinterpret_cast<uintptr_t>(__ramfuncs_start__);
    uintptr_t ramCodeEnd   = reinterpret_cast<uintptr_t>(__ramfuncs_end__);
#endif
    uintptr_t codeEnd = reinterpret_cast<uintptr_t>(__etext);

    auto * stackPtr = reinterpret_cast<uintptr_t *>(sp);
    int count       = 0;
    for (int i = 0; i < kMaxStackScan && count < kMaxAddrPrint; i++)
    {
        uintptr_t val = stackPtr[i];
        // Valid code address: within flash code region and odd (Thumb mode)
        bool inFlash = (val >= codeStart) && (val < codeEnd) && (val & 1);
#if !defined(SLI_SI91X_MCU_INTERFACE)
        // Also check RAM functions region (SL_CODE_RAM)
        bool inRam = (val >= ramCodeStart) && (val < ramCodeEnd) && (val & 1);
#else
        bool inRam = false;
#endif
        if (inFlash || inRam)
        {
            PrintAddr("  [?] 0x", val);
            count++;
        }
    }

    vTaskSuspendAll();
    chipAbort();
}
