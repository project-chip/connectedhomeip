#pragma once

#include <optional>
#include <string>
#include <cstdint>

namespace chip::app::Clusters {

class BridgedDeviceBasicInformationDelegate {
public:
    virtual ~BridgedDeviceBasicInformationDelegate() = default;

    // Called when the NodeLabel attribute is changed.
    virtual void OnNodeLabelChanged(const std::string& newNodeLabel) {}
};

} // namespace chip::app::Clusters
