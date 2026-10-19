#pragma once
#include <variant>

namespace chip::app {
using NoDependency = std::monostate;
class DeviceInterface;

// More types should be added here as needed.
using OOBAccessorDependency = std::variant<NoDependency, DeviceInterface *>;
} // namespace chip::app
