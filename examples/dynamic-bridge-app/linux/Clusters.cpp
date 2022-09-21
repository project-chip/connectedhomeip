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

CHIP_ERROR CommonAttributeAccessInterface::Read(const chip::app::ConcreteReadAttributePath & aPath,
                                                chip::app::AttributeValueEncoder & aEncoder)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
    if (!a)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    return a->Read(aPath, aEncoder);
}

CHIP_ERROR CommonAttributeAccessInterface::Write(const chip::app::ConcreteDataAttributePath & aPath,
                                                 chip::app::AttributeValueDecoder & aDecoder)
{
    CommonCluster * c = FindCluster(aPath);
    if (!c)
        return CHIP_ERROR_NOT_IMPLEMENTED;
    return c->ForwardWriteToBridge(aPath, aDecoder);
}

void CommonAttributeAccessInterface::OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath)
{
    CommonCluster * c = FindCluster(aPath);
    if (c)
    {
        AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
        if (a)
            a->ListWriteBegin(aPath);
    }
}

void CommonAttributeAccessInterface::OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful)
{
    CommonCluster * c = FindCluster(aPath);
    if (c)
    {
        AttributeInterface * a = c->FindAttribute(aPath.mAttributeId);
        if (a)
            a->ListWriteEnd(aPath, aWriteWasSuccessful);
    }
}

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

bool CommonCluster::Push(chip::AttributeId attr, chip::TLV::TLVReader & reader)
{
    chip::app::AttributeValueDecoder decoder(reader, chip::Access::SubjectDescriptor());
    if (WriteFromBridge(chip::app::ConcreteDataAttributePath(GetEndpointId(), GetClusterId(), attr), decoder) != CHIP_NO_ERROR)
        return false;
    OnUpdated(attr);
    return true;
}
