#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>

#include <new>

namespace clusters {

struct LabelStruct
{
  CHIP_ERROR Decode(chip::TLV::TLVReader & reader)
  {
    chip::app::Clusters::detail::Structs::LabelStruct::DecodableType t;
    CHIP_ERROR err = t.Decode(reader);
    if(err == CHIP_NO_ERROR) {
      label = t.label;
      value = t.value;
    }
    return err;
  }

  CHIP_ERROR Encode(chip::TLV::TLVWriter & writer, chip::TLV::Tag tag) const
  {
    chip::app::Clusters::detail::Structs::LabelStruct::Type t;
    t.label = label;
    t.value = value;
    return t.Encode(writer, tag);
  }
  OctetString<16, ZCL_CHAR_STRING_ATTRIBUTE_TYPE> label;
  OctetString<16, ZCL_CHAR_STRING_ATTRIBUTE_TYPE> value;
};
struct DemoClusterCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = ZCL_DEMO_CLUSTER_CLUSTER_ID;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    switch(aPath.mAttributeId)
    {
    case 32:
      return mSingleLabel.Write(aPath, aDecoder);
    case 33:
      return mSomeLabels.Write(aPath, aDecoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSingleLabel);
    list->Add(mSomeLabels);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
      { 32, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(LabelStruct), ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
      { 33, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(LabelStruct) * 1 + 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    };
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<32, ATTRIBUTE_MASK_WRITABLE, StructType<LabelStruct>> mSingleLabel;
  Attribute<33, ATTRIBUTE_MASK_WRITABLE, ArrayType<1, StructType<LabelStruct>>> mSomeLabels;
};

struct DemoClusterAccess : public CommonAttributeAccessInterface
{
  DemoClusterAccess() : CommonAttributeAccessInterface(chip::Optional<chip::EndpointId>(), DemoClusterCluster::kClusterId) {}

  DemoClusterCluster* GetCluster(const chip::app::ConcreteClusterPath & aPath)
  {
    CommonCluster * cluster = FindCluster(aPath);
    return cluster ? static_cast<DemoClusterCluster*>(cluster) : nullptr;
  }

  CHIP_ERROR Read(const chip::app::ConcreteReadAttributePath & aPath, chip::app::AttributeValueEncoder & aEncoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;

    switch(aPath.mAttributeId) {
    case 32:
      return c->mSingleLabel.Read(aPath, aEncoder);
    case 33:
      return c->mSomeLabels.Read(aPath, aEncoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  CHIP_ERROR Write(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return CHIP_ERROR_NOT_IMPLEMENTED;
    return c->ForwardWriteToBridge(aPath, aDecoder);
  }

  void OnListWriteBegin(const chip::app::ConcreteAttributePath & aPath) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return;

    switch(aPath.mAttributeId) {
    case 33:
      mSomeLabels.ListWriteBegin(aPath);
      return;
    }
  }

  void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return;

    switch(aPath.mAttributeId) {
    case 33:
      mSomeLabels.ListWriteEnd(aPath, aWriteWasSuccessful);
      return;
    }
  }
};

struct ClusterInfo
{
  chip::ClusterId id;
  const char *name;
  uint16_t size;
  CommonCluster* (*ctor)(void*);
} static const kKnownClusters[] = {

  {
    ZCL_DEMO_CLUSTER_CLUSTER_ID,
    "DemoCluster",
    sizeof(DemoClusterCluster),
    [](void *mem) -> CommonCluster* {
      return new(mem) DemoClusterCluster();
    },
  },
};

inline void BridgeRegisterAllAttributeOverrides()
{

  static DemoClusterAccess DemoCluster;
  registerAttributeAccessOverride(&DemoCluster);
}

struct AttrInfo
{
  chip::ClusterId cluster;
  chip::AttributeId attr;
  const char *name;
} static const kKnownAttributes[] = {

  {ZCL_DEMO_CLUSTER_CLUSTER_ID, 32, "SingleLabel" },
  {ZCL_DEMO_CLUSTER_CLUSTER_ID, 33, "SomeLabels" },
  
};

}