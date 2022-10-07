#include "GeneratedClusters.h"

#include "Device.h"

CommonCluster::CommonCluster(std::unique_ptr<GeneratedCluster> cluster) :
    mAttributes(cluster->GetAttributes()), mStorage(std::move(cluster))
{}
CommonCluster::~CommonCluster() = default;

chip::ClusterId CommonCluster::GetClusterId()
{
    return mStorage->GetClusterId();
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

CHIP_ERROR CommonCluster::WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath,
                                          chip::app::AttributeValueDecoder & aDecoder)
{
    AttributeInterface * a = FindAttribute(aPath.mAttributeId);
    if (!a)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    return a->Write(aPath, aDecoder);
}

chip::Span<AttributeInterface *> CommonCluster::GetAllAttributes()
{
    return chip::Span<AttributeInterface *>(mAttributes.data(), mAttributes.size());
}

AttributeInterface * CommonCluster::FindAttribute(chip::AttributeId id)
{
    for (auto * attr : mAttributes)
        if (attr->GetId() == id)
            return attr;
    return nullptr;
}

AttributeInterface * CommonCluster::FindAttributeByName(chip::CharSpan name)
{
    for (auto * attr : mAttributes)
        if (name.data_equal(attr->GetName()))
            return attr;
    return nullptr;
}

void CommonCluster::OnUpdated(chip::AttributeId attr)
{
    if (active())
        MatterReportingAttributeChangeCallback(GetEndpointId(), GetClusterId(), attr);
}

bool CommonCluster::Write(chip::AttributeId attr, chip::TLV::TLVReader & reader)
{
    chip::app::AttributeValueDecoder decoder(reader, chip::Access::SubjectDescriptor());
    if (WriteFromBridge(chip::app::ConcreteDataAttributePath(GetEndpointId(), GetClusterId(), attr), decoder) != CHIP_NO_ERROR)
        return false;
    OnUpdated(attr);
    return true;
}
