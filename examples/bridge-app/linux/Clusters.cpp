#include "GeneratedClusters.h"

#include "Device.h"

CommonCluster * CommonAttributeAccessInterface::FindCluster(const chip::app::ConcreteClusterPath & path)
{
    Device * dev = FindDeviceEndpoint(path.mEndpointId);
    if (dev)
    {
        for (auto c : dev->clusters())
        {
            if (c->GetClusterId() == path.mClusterId)
                return static_cast<CommonCluster *>(c);
        }
    }
    return nullptr;
}

void CommonCluster::SetEndpointId(chip::EndpointId id)
{
    mEndpoint = id;
}
chip::EndpointId CommonCluster::GetEndpointId() const
{
    return mEndpoint;
}

void CommonCluster::SetCallback(PropagateWriteCB * cb)
{
    mCallback = cb;
}

bool CommonCluster::active() const
{
    return mEndpoint < 0xFFFF;
}

CHIP_ERROR CommonCluster::ForwardWriteToBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                               chip::app::AttributeValueDecoder & aDecoder)
{
    if (mCallback)
        return (*mCallback)(this, aPath, aDecoder);
    return WriteFromBridge(aPath, aDecoder);
}

void CommonCluster::OnUpdated(chip::AttributeId attr)
{
    if (active())
        MatterReportingAttributeChangeCallback(GetEndpointId(), GetClusterId(), attr);
}

bool CommonCluster::Push(chip::AttributeId attr, chip::TLV::TLVReader & reader)
{
    chip::app::AttributeValueDecoder decoder(reader, chip::Access::SubjectDescriptor());
    if (WriteFromBridge(chip::app::ConcreteDataAttributePath(GetEndpointId(), GetClusterId(), attr), decoder) != CHIP_NO_ERROR)
        return false;
    OnUpdated(attr);
    return true;
}

namespace clusters {
BridgedDeviceBasicCluster::BridgedDeviceBasicCluster() {}

CHIP_ERROR BridgedDeviceBasicCluster::WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                                      chip::app::AttributeValueDecoder & aDecoder)
{
    switch (aPath.mAttributeId)
    {
    case ZCL_REACHABLE_ATTRIBUTE_ID:
        return mReachable.Write(aPath, aDecoder);
    }
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace clusters
