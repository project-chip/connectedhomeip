#include <app-common/zap-generated/cluster-id.h>
#include <app-common/zap-generated/attribute-id.h>

#include <new>

namespace clusters {

struct DemoClusterCluster : public CommonCluster
{


  static constexpr chip::ClusterId kClusterId = 10;

  chip::ClusterId GetClusterId() override { return kClusterId; }

  CHIP_ERROR WriteFromBridge(const chip::app::ConcreteDataAttributePath & aPath, chip::app::AttributeValueDecoder & aDecoder) override
  {
    switch(aPath.mAttributeId)
    {
    case 5:
      return mSingleFailSafe.Write(aPath, aDecoder);
    case 100:
      return mArmFailsafes.Write(aPath, aDecoder);
    default:
      return CHIP_ERROR_NOT_IMPLEMENTED;
    }
  }

  template<typename T>
  void AddAllAttributes(T *list)
  {
    list->Add(mSingleFailSafe);
    list->Add(mArmFailsafes);
  }

  chip::Span<const EmberAfAttributeMetadata> GetAllAttributes() override
  {
    static constexpr const EmberAfAttributeMetadata kAllAttributes[] = {
      { 5, ZCL_STRUCT_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest), ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
      { 100, ZCL_ARRAY_ATTRIBUTE_TYPE, sizeof(ArmFailSafeRequest) * 1 + 2, ATTRIBUTE_MASK_WRITABLE | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE), ZAP_EMPTY_DEFAULT() },
    };
    return chip::Span<const EmberAfAttributeMetadata>(kAllAttributes);
  }


  Attribute<5, ATTRIBUTE_MASK_WRITABLE, StructType<ArmFailSafeRequest>> mSingleFailSafe;
  Attribute<100, ATTRIBUTE_MASK_WRITABLE, ArrayType<1, StructType<ArmFailSafeRequest>>> mArmFailsafes;
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
    case 5:
      return c->mSingleFailSafe.Read(aPath, aEncoder);
    case 100:
      return c->mArmFailsafes.Read(aPath, aEncoder);
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
    case 100:
      mArmFailsafes.ListWriteBegin(aPath);
      return;
    }
  }

  void OnListWriteEnd(const chip::app::ConcreteAttributePath & aPath, bool aWriteWasSuccessful) override
  {
    auto * c = GetCluster(aPath);
    if (!c)
      return;

    switch(aPath.mAttributeId) {
    case 100:
      mArmFailsafes.ListWriteEnd(aPath, aWriteWasSuccessful);
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
    10,
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

  { 10, 5, "SingleFailSafe" },
  { 10, 100, "ArmFailsafes" },
  
};

}