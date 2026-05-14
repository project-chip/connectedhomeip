
#pragma once

#include <cstddef>

namespace chip {
namespace app {

// The maximum size required for TLV encoding of any non-list/non-struct attribute
// in this application, plus conservative TLV overhead.
// Computed by application_generator.py
constexpr size_t kMaxAttributeTLVSize = 1064;

} // namespace app
} // namespace chip
