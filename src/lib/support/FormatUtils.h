#pragma once

#include "StringBuilder.h"

namespace chip {

/**
 * @def INLINE_STRING(BUFSIZE, LEN, DATA)
 *
 * @brief
 *   A null-terminated string that can be used inplace.
 *
 */
#define INLINE_STRING(BUFSIZE, LEN, DATA) chip::StringBuilder<BUFSIZE>().AddFormat("%.*s", LEN, DATA).c_str()

/**
 * @def SPAN_TO_TRUNCATED_CSTR(LEN, DATA)
 *
 * @brief
 *   A null-terminated string that can be used inplace with buffer size 256.
 *   This is used to represent a non null-terminated string as null-terminated string that can be used inplace.
 *   If the buffer size is not enough the value will be truncated.
 *
 */
#define SPAN_TO_TRUNCATED_CSTR(LEN, DATA) INLINE_STRING(256, LEN, DATA)

} // namespace chip
