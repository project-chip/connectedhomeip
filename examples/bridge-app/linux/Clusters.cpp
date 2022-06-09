#include "Clusters.h"

EmberAfStatus CommonCluster::WriteCallback(chip::AttributeId id, uint8_t * buffer)
{
    // If there is a callback then the implementation is responsible for propagating writes.
    if (mCallback)
        return (*mCallback)(this, mEndpoint, GetClusterId(), id, buffer);
    WriteFromBridge(id, buffer);
    return EMBER_ZCL_STATUS_SUCCESS;
}

namespace clusters {
BridgedDeviceBasicCluster::BridgedDeviceBasicCluster() {}

EmberAfStatus BridgedDeviceBasicCluster::Read(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                              uint16_t maxReadLength)
{
    if (mReachable.IsAttribute(attributeMetadata))
        return mReachable.Read(attributeMetadata, buffer, maxReadLength);
    return EMBER_ZCL_STATUS_FAILURE;
}

EmberAfStatus BridgedDeviceBasicCluster::Write(const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    if (mReachable.IsAttribute(attributeMetadata))
        return mReachable.WriteFromMatter(attributeMetadata, buffer, this);
    return EMBER_ZCL_STATUS_FAILURE;
}

void BridgedDeviceBasicCluster::WriteFromBridge(chip::AttributeId attributeId, const uint8_t * buffer)
{
    if (mReachable.IsAttribute(attributeId))
        return mReachable.WriteFromBridge(buffer, this);
}

} // namespace clusters
