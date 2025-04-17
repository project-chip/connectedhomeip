#pragma once

namespace chip {
namespace app {
namespace Meta {

template <auto... objects>
struct ObjectList
{
};

template <auto object>
struct Object
{
    static constexpr auto value() { return object; }
};

} // namespace Meta
} // namespace app
} // namespace chip
